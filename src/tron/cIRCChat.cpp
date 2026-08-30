#include "cIRCChat.h"
#include "cShell.h"
#include "cBackend.h"
#include "cIlonium.h"
#include "tDirectories.h"
#include "tString.h"
#include "tSysTime.h"
#include "ePlayer.h"
#include "../network/md5.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#define RC_POPEN  _popen
#define RC_PCLOSE _pclose
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "tOwnership.h"

RC_OWNERSHIP( chat )

#define RC_POPEN  popen
#define RC_PCLOSE pclose
#endif

std::string cIRCChat::s_HWID = "";
bool cIRCChat::s_Initialized = false;

std::mutex cIRCChat::s_MessageMutex;
std::vector<IRCMessage> cIRCChat::s_Messages;
std::string cIRCChat::s_StatusMessage = "";

double cIRCChat::s_LastPollTime = 0.0;
double cIRCChat::s_LastSendTime = 0.0;
static double s_LastSendTime = 0.0;
bool cIRCChat::s_IsPolling = false;
bool cIRCChat::s_IsSending = false;

static std::string BytesToHexStr(const unsigned char* bytes, size_t len) {
    static const char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(kHex[(bytes[i] >> 4) & 0xF]);
        out.push_back(kHex[ bytes[i]       & 0xF]);
    }
    return out;
}

std::string cIRCChat::MD5String(const std::string& input) {
    md5_state_t state;
    md5_byte_t digest[16];
    md5_init(&state);
    md5_append(&state, reinterpret_cast<const md5_byte_t*>(input.data()), static_cast<int>(input.size()));
    md5_finish(&state, digest);
    return BytesToHexStr(reinterpret_cast<const unsigned char*>(digest), 16);
}

std::string cIRCChat::ComputeHWID() {
    std::string rawHWID = "";

#if defined(_WIN32) || defined(WIN32)
    DWORD serialNum = 0;
    GetVolumeInformationA("C:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0);

    char regGuid[256] = {0};
    DWORD bufSize = sizeof(regGuid);
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)regGuid, &bufSize);
        RegCloseKey(hKey);
    }
    rawHWID = std::string(regGuid) + "_" + std::to_string(serialNum);
#else
    static const char* machineIdPaths[] = {
        "/run/host/etc/machine-id",
        "/etc/machine-id",
        "/var/lib/dbus/machine-id",
        nullptr
    };

    for (int i = 0; machineIdPaths[i] != nullptr; ++i) {
        std::ifstream f(machineIdPaths[i]);
        if (f.is_open()) {
            std::string line;
            if (std::getline(f, line) && !line.empty()) {
                rawHWID = line;
                f.close();
                break;
            }
            f.close();
        }
    }

    if (rawHWID.empty()) {
        char host[256] = {0};
        gethostname(host, sizeof(host) - 1);
        struct passwd* pw = getpwuid(getuid());
        std::string user = (pw && pw->pw_name) ? pw->pw_name : "user";
        rawHWID = std::string(host) + "_" + user;
    }
#endif

    return MD5String(rawHWID);
}

static std::string TrimString(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && (std::isspace(static_cast<unsigned char>(str[start])) || str[start] == '\r' || str[start] == '\n')) {
        start++;
    }
    size_t end = str.size();
    while (end > start && (std::isspace(static_cast<unsigned char>(str[end - 1])) || str[end - 1] == '\r' || str[end - 1] == '\n')) {
        end--;
    }
    return str.substr(start, end - start);
}

void cIRCChat::Init() {
    if (s_Initialized) return;
    s_Initialized = true;

    // The key is made here rather than at the first message, so a machine that
    // has run the game once already has an identity when it needs one.
    rc_IloniumReady();
    s_HWID = ComputeHWID();
}

std::string cIRCChat::GetHWID() {
    if (s_HWID.empty()) s_HWID = ComputeHWID();
    return s_HWID;
}

std::string cIRCChat::GetIdentity() {
    std::string key = rc_IloniumPublicKey();
    return key.size() > 10 ? key.substr(0, 10) : key;
}

std::string cIRCChat::GetRoleTag() {
    return rc_IloniumMe().roleTag;
}

std::string cIRCChat::GetRoleColor() {
    return rc_IloniumMe().roleColor;
}

std::vector<IRCMessage> cIRCChat::GetMessages() {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    return s_Messages;
}

std::string cIRCChat::GetStatusMessage() {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    return s_StatusMessage;
}

void cIRCChat::SetStatusMessage(const std::string& status) {
    std::lock_guard<std::mutex> lock(s_MessageMutex);
    s_StatusMessage = status;
}

// The gap between two messages is the server's to enforce; this is only so the
// send button can grey out instead of firing into a refusal.
double cIRCChat::GetCooldownRemaining() {
    double const left = 3.0 - (tSysTimeFloat() - s_LastSendTime);
    return left > 0.0 ? left : 0.0;
}

void cIRCChat::PollMessagesSync() {
    std::vector<IloniumLine> lines;
    std::string error;

    if (!rc_IloniumFetch(lines, error)) {
        SetStatusMessage(error.empty() ? "[System] The chat is not answering." : ("[System] " + error));
        s_IsPolling = false;
        return;
    }

    std::vector<IRCMessage> fresh;
    fresh.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); ++i) {
        IRCMessage m;
        m.id = (int)lines[i].id;
        m.login = lines[i].name;
        m.in_game_name = lines[i].name;
        m.message = lines[i].message;
        m.timestamp = lines[i].stamp;
        m.role_tag = lines[i].roleTag;
        m.role_color = lines[i].roleColor;
        fresh.push_back(m);
    }

    {
        std::lock_guard<std::mutex> lock(s_MessageMutex);
        s_Messages.swap(fresh);
        if (s_StatusMessage.rfind("[System] ", 0) == 0)
            s_StatusMessage.clear();
    }

    s_IsPolling = false;
}

void cIRCChat::PollMessagesAsync() {
    if (s_IsPolling) return;
    s_IsPolling = true;

    std::thread([]() {
        PollMessagesSync();
    }).detach();
}

void cIRCChat::UpdatePolling(bool tabActive) {
    if (!tabActive) return;

    double const now = tSysTimeFloat();
    if (now - s_LastPollTime < 4.0) return;

    s_LastPollTime = now;
    PollMessagesAsync();
}

bool cIRCChat::SendChatMessage(const std::string& text, std::string& outError) {
    std::string const body = TrimString(text);
    if (body.empty()) {
        outError = "nothing to say";
        return false;
    }
    if (s_IsSending) {
        outError = "still sending the last one";
        return false;
    }

    // The name over the bike is the name in the chat: one person, one label,
    // and nothing to fill in twice.
    std::string name = "player";
    if (se_PlayerNetIDs.Len() > 0 && se_PlayerNetIDs(0))
        name = (char const *)se_PlayerNetIDs(0)->GetName();
    else if (ePlayer::PlayerConfig(0))
        name = (char const *)ePlayer::PlayerConfig(0)->Name();

    s_IsSending = true;
    bool const sent = rc_IloniumSay(name, body, outError);
    s_IsSending = false;

    if (sent) {
        s_LastSendTime = tSysTimeFloat();
        s_LastPollTime = 0.0;      // show it immediately rather than in four seconds
        PollMessagesAsync();
    }

    return sent;
}
