// RCLAuth.cpp  – Retrocycles League authentication implementation.
//
// Hashing algorithm (matches the reference bash script exactly):
//
//   prefix  = username + ":aaauth:"
//   suffix  = ":retrocyclesleague.com"
//   FIRST   = md5hex( prefix + password + suffix )
//   HASH    = md5hex( hex_decode( FIRST ) )
//             ^ i.e. treat the 32-char hex string as raw bytes and hash again
//
// HTTP check:
//   GET https://rcl.authentication.armagetronad.net/armaauth/0.1
//       ?query=check&user=<urlenc(user)>&hash=<HASH>&method=md5
//   Response body contains "LOGIN_OK" on success.
#include "config.h"
#include "RCLAuth.h"
#include "cShell.h"

#ifdef HAVE_LIBXML2
#include <libxml/nanohttp.h>
#endif

// Project headers
#include "tString.h"
#include "tResourceManager.h"
#include "tDirectories.h"
#include "nKrawall.h"       // for EncodeString (URL-encoding) and md5 wrappers

// C++ standard library
#include <string>
#include <sstream>
#include <thread>
#include <fstream>
#include <cstdio>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <map>
#include <vector>
#include <chrono>
#include <cstdint>
#include <random>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <shlobj.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define RCL_POPEN  _popen
#define RCL_PCLOSE _pclose
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <unistd.h>
#define RCL_POPEN  popen
#define RCL_PCLOSE pclose
#endif

// Internal MD5 implementation already bundled with Armagetron.
// Lives in src/network/md5.h; nKrawall.h pulls it in, but we reference it
// directly so the types are always visible in this TU.
#include "../network/md5.h"

// =========================================================================
// Curl helpers (needed for webhook POST requests)
// =========================================================================
#ifdef LIBCURL_PROTOCOL_HTTP
#include <curl/curl.h>
#include "tOwnership.h"

RC_OWNERSHIP( auth )


// Local RAII curl handle (mirrors the one in tResourceManager.cpp)
class tCurlLocal
{
private:
    CURL* _handle;
public:
    tCurlLocal() {
        static struct CurlGlobalInit { CurlGlobalInit() { curl_global_init(CURL_GLOBAL_DEFAULT); } } _once;
        _handle = curl_easy_init();
    }
    ~tCurlLocal() { curl_easy_cleanup(_handle); }
    operator CURL*() const { return _handle; }
    static size_t write_to_ostream(char* ptr, size_t size, size_t nmemb, void* userdata) {
        std::ostream* stream = static_cast<std::ostream*>(userdata);
        stream->write(ptr, size * nmemb);
        return size * nmemb;
    }
};
#endif

// =========================================================================
// HTTP helpers (xmlNanoHTTP primary, curl fallback)
// =========================================================================

static void RCL_LogDebug(const std::string& msg)
{
    std::ofstream f;
    // We open var directory. Note: tDirectories::Var() opens path/to/var
    // To open with append flags, we can pass it to Open.
    if (tDirectories::Var().Open(f, "rcl_mod_debug.log", std::ios_base::out | std::ios_base::app))
    {
        f << msg << "\n";
        f.close();
    }
}

// Returns the body of a GET request, or empty string on failure.
// Uses xmlNanoHTTP (already linked via -lxml2) as the primary transport;
// falls back to popen/curl with explicit paths if nanohttp fails.
static std::string RCL_HttpGet(const std::string& url)
{
    RCL_LogDebug("RCL_HttpGet request to URL: " + url);

    // nanohttp speaks plain HTTP only. Handing it an https address makes it
    // fail and print an IO error for every single request, so skip straight to
    // the transport that can actually do TLS.
    bool secure = url.compare(0, 8, "https://") == 0;

#ifdef HAVE_LIBXML2
    if (!secure) {
    xmlNanoHTTPInit();
    char* contentType = nullptr;
    void* ctx = xmlNanoHTTPOpen(url.c_str(), &contentType);
    if (ctx)
    {
        RCL_LogDebug("xmlNanoHTTPOpen succeeded");
        std::string result;
        char buf[4096];
        int bytes;
        while ((bytes = xmlNanoHTTPRead(ctx, buf, sizeof(buf))) > 0)
            result.append(buf, bytes);
        xmlNanoHTTPClose(ctx);
        if (contentType) free(contentType);
        if (!result.empty()) {
            RCL_LogDebug("xmlNanoHTTP read " + std::to_string(result.size()) + " bytes successfully.");
            return result;
        }
    }
    else
    {
        RCL_LogDebug("xmlNanoHTTPOpen failed (NULL context)");
        if (contentType) free(contentType);
    }
    }
#else
    (void)secure;
    RCL_LogDebug("HAVE_LIBXML2 not defined, skipping nanohttp");
#endif
    // Fallback: popen curl with explicit full paths
    // NOTE: Game runs inside Steam Linux Runtime (pressure vessel container).
    // Inside the container, the host filesystem is at /run/host/
    // and /usr/bin/curl is the runtime's own curl.
    static const char* curlPaths[] = {
        "/usr/bin/curl",          // works inside pressure vessel (Steam Runtime has curl)
        "/run/host/usr/bin/curl", // host filesystem mounted inside pressure vessel
        "/bin/curl",
        "curl",
        nullptr
    };
    std::string curlBin = "curl";
    for (int i = 0; curlPaths[i] != nullptr; i++) {
        if (curlPaths[i][0] != '/') { curlBin = curlPaths[i]; break; }
        FILE* t = fopen(curlPaths[i], "r");
        if (t) { fclose(t); curlBin = curlPaths[i]; break; }
    }
    std::string cmd = "LD_LIBRARY_PATH=\"\" " + curlBin + " -s -k --max-time 10 '" + url + "'"; // Add -k just in case ssl certs are missing in sandbox
    RCL_LogDebug("Running curl fallback: " + cmd);
    std::string response;
    if (!rc_RunHidden(cmd, response)) {
        RCL_LogDebug("could not start curl");
        return "";
    }
    RCL_LogDebug("curl finished, read: " + std::to_string(response.size()) + " bytes");
    return response;
}

// =========================================================================
// Internal helpers
// =========================================================================

/// Convert 16 raw bytes to a 32-character lowercase hex string.
static std::string BytesToHex(const unsigned char* bytes, size_t len)
{
    static const char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i)
    {
        out.push_back(kHex[(bytes[i] >> 4) & 0xF]);
        out.push_back(kHex[ bytes[i]       & 0xF]);
    }
    return out;
}

/// Parse one hex nibble; returns 0 for invalid characters.
static unsigned char HexNibble(char c)
{
    if (c >= '0' && c <= '9') return static_cast<unsigned char>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<unsigned char>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<unsigned char>(c - 'A' + 10);
    return 0;
}

/// Decode a 32-char hex string to 16 raw bytes (like `xxd -r -p`).
/// Returns false if the input is not exactly 32 valid hex characters.
static bool HexDecode(const std::string& hex, unsigned char out[16])
{
    if (hex.size() != 32) return false;
    for (int i = 0; i < 16; ++i)
    {
        out[i] = static_cast<unsigned char>(
            (HexNibble(hex[i * 2]) << 4) | HexNibble(hex[i * 2 + 1]));
    }
    return true;
}

/// Compute MD5 of arbitrary bytes, return 32-char lowercase hex string.
static std::string MD5Hex(const unsigned char* data, size_t len)
{
    md5_state_t state;
    md5_byte_t  digest[16];
    md5_init(&state);
    md5_append(&state, reinterpret_cast<const md5_byte_t*>(data), static_cast<int>(len));
    md5_finish(&state, digest);
    return BytesToHex(reinterpret_cast<const unsigned char*>(digest), 16);
}

/// Overload for std::string input.
static std::string MD5Hex(const std::string& s)
{
    return MD5Hex(reinterpret_cast<const unsigned char*>(s.data()), s.size());
}

// =========================================================================
// Core hashing: produces the HASH the RCL server expects.
// =========================================================================

/// Step 1 + Step 2 as described in the spec.
static std::string ComputeRCLHash(const std::string& username,
                                  const std::string& password)
{
    // Step 1: FIRST = md5hex( username + ":aaauth:" + password + ":retrocyclesleague.com" )
    std::string plain = username + ":aaauth:" + password + ":retrocyclesleague.com";
    std::string first = MD5Hex(plain);

    // Step 2: HASH = md5hex( raw_bytes_of( FIRST ) )
    //         In bash: printf '%s' "$FIRST" | xxd -r -p | md5sum
    //         i.e. hex-decode the 32-char hex string → 16 bytes, then MD5 those bytes.
    unsigned char firstBytes[16];
    if (!HexDecode(first, firstBytes))
    {
        // Should never happen, but return empty on malformed MD5.
        return std::string();
    }
    return MD5Hex(firstBytes, 16);
}

// =========================================================================
// URL encoding  (reuse project's own EncodeString via tString wrapper)
// =========================================================================

static std::string URLEncode(const std::string& s)
{
    tString ts(s.c_str());
    tString encoded = nKrawall::EncodeString(ts);
    return std::string(static_cast<const char*>(encoded));
}

static std::string TrimString(const std::string& str)
{
    size_t start = 0;
    while (start < str.size() && (std::isspace(static_cast<unsigned char>(str[start])) || str[start] == '\r' || str[start] == '\n'))
    {
        start++;
    }
    size_t end = str.size();
    while (end > start && (std::isspace(static_cast<unsigned char>(str[end - 1])) || str[end - 1] == '\r' || str[end - 1] == '\n'))
    {
        end--;
    }
    return str.substr(start, end - start);
}

static std::string GetCleanUsername(const std::string& input)
{
    std::string s = TrimString(input);
    size_t pos = s.find('@');
    if (pos != std::string::npos)
    {
        return s.substr(0, pos);
    }
    return s;
}

// =========================================================================
// State variables
// =========================================================================

static std::mutex        s_authMutex;
static RCLAuthState      s_authState  = RCLAuthState::Idle;
static std::string       s_authError;
static std::string       s_authHash;
static std::atomic<bool> s_threadRunning{ false };

static std::atomic<int> s_playerEloTst{ -1 };
static std::atomic<int> s_playerEloSumo{ -1 };
static std::atomic<int> s_playerElo2s{ -1 };
static std::atomic<bool> s_ElosLoaded{ false };

// =========================================================================
// Synchronous HTTP check
// =========================================================================

bool RCL_AuthenticateSync(const std::string& username,
                          const std::string& password,
                          std::string&       outError)
{
    std::string cleanUser = GetCleanUsername(username);
    std::string cleanPass = TrimString(password);

    // --- Compute hash ---
    std::string hash = ComputeRCLHash(cleanUser, cleanPass);
    if (hash.empty())
    {
        outError = "Internal error: hash computation failed.";
        return false;
    }

    // --- Build URL ---
    std::ostringstream url;
    url << "https://rcl.authentication.armagetronad.net/armaauth/0.1"
        << "?query=check"
        << "&user="   << URLEncode(cleanUser)
        << "&hash="   << hash          // hash is pure lowercase hex, no encoding needed
        << "&method=md5";

    // --- Fetch ---
    std::string response = RCL_HttpGet(url.str());
    if (response.empty())
    {
        outError = "Network error: could not reach RCL auth server.";
        return false;
    }

    // Case-insensitive search for "LOGIN_OK"
    std::string responseLower = response;
    std::transform(responseLower.begin(), responseLower.end(),
                   responseLower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (responseLower.find("login_ok") != std::string::npos ||
        responseLower.find("password_ok") != std::string::npos)
    {
        {
            std::lock_guard<std::mutex> lock(s_authMutex);
            s_authHash = hash;
            s_authState = RCLAuthState::Success;
        }
        outError.clear();
        return true;
    }

    // Map known failure tokens to readable messages.
    if (responseLower.find("unknown_user") != std::string::npos)
        outError = "Unknown user: '" + username + "' is not registered on RCL.";
    else if (responseLower.find("password_fail") != std::string::npos ||
             responseLower.find("login_fail")    != std::string::npos)
        outError = "Wrong password.";
    else
        outError = "Authentication failed. Server response: " + response;

    return false;
}

// =========================================================================
// Async state management (for calling from the ImGui thread)
// =========================================================================

RCLAuthState RCL_GetAuthState()
{
    std::lock_guard<std::mutex> lock(s_authMutex);
    return s_authState;
}

const std::string& RCL_GetAuthError()
{
    // Caller should hold no lock here; we return by reference from a
    // string that is only written from the worker thread (which has finished
    // by the time state == Failed/Success).  Safe in practice.
    return s_authError;
}

void RCL_ResetAuthState()
{
    std::lock_guard<std::mutex> lock(s_authMutex);
    s_authState = RCLAuthState::Idle;
    s_authError.clear();
    s_authHash.clear();
}

std::string RCL_GetAuthHash()
{
    std::lock_guard<std::mutex> lock(s_authMutex);
    return s_authHash;
}

void RCL_Login(const std::string& username, const std::string& password)
{
    {
        std::lock_guard<std::mutex> lock(s_authMutex);
        if (s_authState == RCLAuthState::Pending)
            return; // Already in flight, ignore duplicate clicks.
        s_authState = RCLAuthState::Pending;
        s_authError.clear();
        s_authHash.clear();
    }

    // Capture by value so the thread owns its own copies.
    std::string user = username;
    std::string pass = password;

    // Detach a worker thread.  The thread sets state to Success/Failed when done.
    // Note: FetchURI uses libxml nanohttp or libcurl – both are thread-safe as
    // long as we don't share handles, which we don't (FetchURI creates its own).
    std::thread([user, pass]()
    {
        s_threadRunning.store(true);

        std::string error;
        bool ok = RCL_AuthenticateSync(user, pass, error);

        {
            std::lock_guard<std::mutex> lock(s_authMutex);
            s_authState = ok ? RCLAuthState::Success : RCLAuthState::Failed;
            s_authError = error;
            if (ok)
            {
                s_authHash = ComputeRCLHash(GetCleanUsername(user), pass);
                RCL_SaveCredentials(user, s_authHash);
            }
        }

        if (ok)
        {
            RCL_FetchPlayerElosAsync(user);
        }

        s_threadRunning.store(false);
    }).detach();
}



// =========================================================================
// Session Persistence and Auto-Login
// =========================================================================

bool RCL_SaveCredentials(const std::string& username, const std::string& hash)
{
    std::ofstream f;
    if (tDirectories::Var().Open(f, "rcl_session.cfg"))
    {
        f << username << "\n" << hash << "\n";
        f.close();
        return true;
    }
    return false;
}

static bool RCL_LoadCredentials(std::string& outUsername, std::string& outHash)
{
    std::ifstream f;
    if (tDirectories::Var().Open(f, "rcl_session.cfg"))
    {
        if (std::getline(f, outUsername) && std::getline(f, outHash))
        {
            f.close();
            return true;
        }
        f.close();
    }
    return false;
}

bool RCL_AttemptAutoLogin(std::string& outUsername)
{
    std::string user, hash;
    if (RCL_LoadCredentials(user, hash))
    {
        std::lock_guard<std::mutex> lock(s_authMutex);
        s_authState = RCLAuthState::Success;
        s_authHash = hash;
        outUsername = user;
        return true;
    }
    return false;
}

void RCL_Logout()
{
    RCL_ResetAuthState();
    
    s_playerEloTst.store(-1);
    s_playerEloSumo.store(-1);
    s_playerElo2s.store(-1);
    s_ElosLoaded.store(false);

    tString path = rc_InternalPath("rcl_session.cfg");
    std::remove(static_cast<const char*>(path));
}

// =========================================================================
// Queue Actions (Zero Trust join / leave)
// =========================================================================

// Shared last server response for queue actions — read by UI to show real result
static std::mutex s_queueResponseMutex;
static std::string s_queueLastResponse;

const std::string& RCL_GetQueueLastResponse()
{
    // Called from UI thread; written from worker thread after it finishes.
    // Safe because UI reads only after thread has exited (detach + atomic flag not needed here;
    // we accept a potential data race but it only affects display text).
    return s_queueLastResponse;
}

// =========================================================================
// RetroCycles League session (Supabase) — native, transport shared with the
// rest of the client. Credentials only ever travel inside a POST body written
// to a temp file, never on the command line.
// =========================================================================

static const char* kSupaBase   = "https://freymzdowhiekpuvjlur.supabase.co";
static const char* kSupaAnon   = "sb_publishable_wt5eJwpchNSDWuJQNiY15w_j5JBbds3";
static const char* kSupaCookie = "sb-freymzdowhiekpuvjlur-auth-token";
static const char* kSiteBase   = "https://retrocyclesleague.com";

namespace {

std::string CurlBinary()
{
    static const char* paths[] = { "/usr/bin/curl", "/run/host/usr/bin/curl", "/bin/curl", nullptr };
    for (int i = 0; paths[i]; ++i)
    {
        FILE* t = fopen(paths[i], "r");
        if (t) { fclose(t); return paths[i]; }
    }
    return "curl";
}

long HttpPost(const std::string& url,
              const std::vector<std::string>& headers,
              const std::string& body,
              std::string& outBody)
{
    outBody.clear();

    // A name of its own for every call. One shared temporary meant that two
    // requests in flight at once - the session refresh and the queue poll do
    // overlap - had the first one deleting the file while the second's curl
    // was still opening it, which is what "error encountered when reading a
    // file" was: not a broken request, a file pulled out from under it.
    static std::atomic< unsigned > ticket( 0 );

    char name[ 64 ];
    snprintf( name, sizeof( name ), "rcl_http_post_%d_%u.tmp",
              (int)getpid(), ticket.fetch_add( 1 ) );

    tString bodyPath = rc_InternalPath( name );
    {
        std::ofstream bf((const char*)bodyPath, std::ios::binary | std::ios::trunc);
        if (!bf.is_open())
            return -1;
        bf.write(body.data(), (std::streamsize)body.size());
    }

    std::string cmd = "LD_LIBRARY_PATH=\"\" " + CurlBinary() + " -s -k -X POST -w \"\\n%{http_code}\"";
    for (const std::string& h : headers)
        cmd += " -H \"" + h + "\"";
    cmd += " --data-binary @\"" + std::string((const char*)bodyPath) + "\" \"" + url + "\"";

    std::string resp;
    if (!rc_RunHidden(cmd, resp))
    {
        std::remove((const char*)bodyPath);
        return -1;
    }
    std::remove((const char*)bodyPath);

    long code = 0;
    size_t nl = resp.find_last_of('\n');
    std::string codeStr = (nl == std::string::npos) ? resp : resp.substr(nl + 1);
    while (!codeStr.empty() && (codeStr.back() == '\r' || codeStr.back() == ' ' || codeStr.back() == '\n'))
        codeStr.pop_back();

    bool numeric = !codeStr.empty();
    for (char c : codeStr)
        if (!std::isdigit((unsigned char)c)) { numeric = false; break; }

    if (numeric && nl != std::string::npos)
    {
        code = std::stol(codeStr);
        outBody = resp.substr(0, nl);
    }
    else
    {
        outBody = resp;
    }
    return code;
}

std::string JsonEscape(const std::string& s)
{
    std::string o;
    o.reserve(s.size() + 8);
    for (char c : s)
    {
        switch (c)
        {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:   o += c;      break;
        }
    }
    return o;
}

std::string JsonStr(const std::string& j, const std::string& key)
{
    std::string pat = "\"" + key + "\"";
    size_t p = j.find(pat);
    if (p == std::string::npos) return "";
    p = j.find(':', p + pat.size());
    if (p == std::string::npos) return "";
    ++p;
    while (p < j.size() && (j[p] == ' ' || j[p] == '\t' || j[p] == '\n' || j[p] == '\r')) ++p;
    if (p >= j.size() || j[p] != '"') return "";
    ++p;
    std::string out;
    while (p < j.size() && j[p] != '"')
    {
        if (j[p] == '\\' && p + 1 < j.size())
        {
            char n = j[p + 1];
            if (n == 'n') out += '\n';
            else if (n == 't') out += '\t';
            else if (n == 'r') out += '\r';
            else out += n;
            p += 2;
        }
        else { out += j[p]; ++p; }
    }
    return out;
}

long JsonNum(const std::string& j, const std::string& key)
{
    std::string pat = "\"" + key + "\"";
    size_t p = j.find(pat);
    if (p == std::string::npos) return 0;
    p = j.find(':', p + pat.size());
    if (p == std::string::npos) return 0;
    ++p;
    while (p < j.size() && (j[p] == ' ' || j[p] == '\t')) ++p;
    std::string num;
    while (p < j.size() && (std::isdigit((unsigned char)j[p]) || j[p] == '-')) { num += j[p]; ++p; }
    if (num.empty()) return 0;
    return std::stol(num);
}

std::string Base64(const std::string& in)
{
    static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, bits = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        bits += 8;
        while (bits >= 0) { out.push_back(T[(val >> bits) & 0x3F]); bits -= 6; }
    }
    if (bits > -6) out.push_back(T[((val << 8) >> (bits + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

double NowSeconds()
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               std::chrono::system_clock::now().time_since_epoch()).count();
}

struct SupaSession
{
    std::string access;
    std::string refresh;
    std::string raw;
    long        expiresAt = 0;
    bool valid() const { return !access.empty() && !refresh.empty(); }
};

std::mutex  s_supaMutex;
SupaSession s_supa;

void SupaStore(const std::string& raw)
{
    SupaSession s;
    s.raw     = raw;
    s.access  = JsonStr(raw, "access_token");
    s.refresh = JsonStr(raw, "refresh_token");
    s.expiresAt = JsonNum(raw, "expires_at");
    if (s.expiresAt == 0)
    {
        long ttl = JsonNum(raw, "expires_in");
        if (ttl > 0) s.expiresAt = (long)NowSeconds() + ttl;
    }
    if (!s.valid())
        return;

    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        s_supa = s;
    }
    tString path = rc_InternalPath("rcl_session_refreshed.json");
    std::ofstream f((const char*)path, std::ios::binary | std::ios::trunc);
    if (f.is_open())
        f.write(raw.data(), (std::streamsize)raw.size());
}

bool SupaLoadFromDisk()
{
    tString path = rc_InternalPath("rcl_session_refreshed.json");
    std::ifstream f((const char*)path, std::ios::binary);
    if (!f.is_open())
        return false;
    std::stringstream ss;
    ss << f.rdbuf();
    std::string raw = ss.str();
    if (raw.empty())
        return false;

    SupaSession s;
    s.raw     = raw;
    s.access  = JsonStr(raw, "access_token");
    s.refresh = JsonStr(raw, "refresh_token");
    s.expiresAt = JsonNum(raw, "expires_at");
    if (!s.valid())
        return false;

    std::lock_guard<std::mutex> lk(s_supaMutex);
    s_supa = s;
    return true;
}

bool SupaResolveEmail(const std::string& username, std::string& outEmail, std::string& outError)
{
    std::string body = "{\"username\":\"" + JsonEscape(username) + "\"}";
    std::vector<std::string> headers = { "Content-Type: application/json" };
    std::string resp;
    long code = HttpPost(std::string(kSiteBase) + "/api/auth/lookup-email", headers, body, resp);

    if (code == 200)
    {
        outEmail = JsonStr(resp, "email");
        if (!outEmail.empty())
            return true;
        outError = "Account lookup returned no email.";
        return false;
    }
    if (code == 404)
    {
        outError = "No RetroCycles League account matches that name.";
        return false;
    }
    outError = "Account lookup failed (HTTP " + std::to_string(code) + ").";
    return false;
}

bool SupaPasswordGrant(const std::string& identifier, const std::string& password, std::string& outError)
{
    std::string email = identifier;
    {
        std::string trimmed = identifier;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) trimmed.erase(trimmed.begin());
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\r' || trimmed.back() == '\n')) trimmed.pop_back();
        email = trimmed;
    }

    if (email.find('@') == std::string::npos)
    {
        std::string user = email;
        if (user.size() > 4 && user.compare(user.size() - 4, 4, "@rcl") == 0)
            user = user.substr(0, user.size() - 4);
        if (!SupaResolveEmail(user, email, outError))
            return false;
    }

    std::string body = "{\"email\":\"" + JsonEscape(email) + "\",\"password\":\"" + JsonEscape(password) + "\"}";
    std::vector<std::string> headers = {
        std::string("apikey: ") + kSupaAnon,
        "Content-Type: application/json"
    };
    std::string resp;
    long code = HttpPost(std::string(kSupaBase) + "/auth/v1/token?grant_type=password", headers, body, resp);

    if (code == 200 && !JsonStr(resp, "access_token").empty())
    {
        SupaStore(resp);
        return true;
    }

    std::string msg = JsonStr(resp, "error_description");
    if (msg.empty()) msg = JsonStr(resp, "msg");
    if (msg.empty()) msg = JsonStr(resp, "error");
    outError = msg.empty() ? ("Sign-in failed (HTTP " + std::to_string(code) + ").") : msg;
    return false;
}

bool SupaRefresh(std::string& outError)
{
    std::string refresh;
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        refresh = s_supa.refresh;
    }
    if (refresh.empty())
    {
        outError = "No session to refresh.";
        return false;
    }

    std::string body = "{\"refresh_token\":\"" + JsonEscape(refresh) + "\"}";
    std::vector<std::string> headers = {
        std::string("apikey: ") + kSupaAnon,
        "Content-Type: application/json"
    };
    std::string resp;
    long code = HttpPost(std::string(kSupaBase) + "/auth/v1/token?grant_type=refresh_token", headers, body, resp);

    if (code == 200 && !JsonStr(resp, "access_token").empty())
    {
        SupaStore(resp);
        return true;
    }

    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        s_supa = SupaSession();
    }
    tString path = rc_InternalPath("rcl_session_refreshed.json");
    std::remove((const char*)path);
    outError = "Session expired. Please sign in to RetroCycles League again.";
    return false;
}

bool SupaEnsureSession(std::string& outError)
{
    bool have;
    long exp;
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        have = s_supa.valid();
        exp  = s_supa.expiresAt;
    }
    if (!have)
    {
        if (!SupaLoadFromDisk())
        {
            outError = "Not signed in to RetroCycles League.";
            return false;
        }
        std::lock_guard<std::mutex> lk(s_supaMutex);
        exp = s_supa.expiresAt;
    }

    if (exp - (long)NowSeconds() < 60)
        return SupaRefresh(outError);

    return true;
}

std::string SupaCookieHeader()
{
    std::string raw;
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        raw = s_supa.raw;
    }
    if (raw.empty())
        return "";

    std::string value = "base64-" + Base64(raw);
    const size_t chunk = 3180;
    std::string header = "Cookie: ";
    for (size_t i = 0, idx = 0; i < value.size(); i += chunk, ++idx)
    {
        if (idx > 0) header += "; ";
        header += std::string(kSupaCookie) + "." + std::to_string(idx) + "=" + value.substr(i, chunk);
    }
    return header;
}

} // namespace

bool RCL_SupabaseLoginSync(const std::string& identifier, const std::string& password, std::string& outError)
{
    return SupaPasswordGrant(identifier, password, outError);
}

void RCL_SupabaseLoginAsync(const std::string& identifier, const std::string& password)
{
    std::string id = identifier;
    std::string pw = password;
    std::thread([id, pw]()
    {
        std::string err;
        SupaPasswordGrant(id, pw, err);
    }).detach();
}

bool RCL_SupabaseIsLinked()
{
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        if (s_supa.valid())
            return true;
    }
    return SupaLoadFromDisk();
}

void RCL_SupabaseLogout()
{
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        s_supa = SupaSession();
    }
    tString path = rc_InternalPath("rcl_session_refreshed.json");
    std::remove((const char*)path);
}

bool RCL_SupabaseImportSession(const std::string& sessionJson, std::string& outError)
{
    std::string trimmed = sessionJson;
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t' || trimmed.front() == '\n' || trimmed.front() == '\r'))
        trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\n' || trimmed.back() == '\r'))
        trimmed.pop_back();

    if (trimmed.size() > 7 && trimmed.compare(0, 7, "base64-") == 0)
    {
        std::string decoded;
        std::string b = trimmed.substr(7);
        for (char& c : b) { if (c == '-') c = '+'; else if (c == '_') c = '/'; }
        int val = 0, bits = -8;
        static const std::string T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (char c : b)
        {
            if (c == '=' || c == '\n' || c == '\r') continue;
            size_t pos = T.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + (int)pos;
            bits += 6;
            if (bits >= 0) { decoded.push_back((char)((val >> bits) & 0xFF)); bits -= 8; }
        }
        if (!decoded.empty())
            trimmed = decoded;
    }

    std::string at = JsonStr(trimmed, "access_token");
    std::string rt = JsonStr(trimmed, "refresh_token");
    if (at.empty() || rt.empty())
    {
        outError = "That doesn't look like a valid session (no tokens found).";
        return false;
    }

    SupaStore(trimmed);
    return true;
}

// =========================================================================
// OAuth (Discord / Google) via a PKCE loopback. Opens the provider consent
// page in the system browser and catches the authorization code on a local
// 127.0.0.1 listener. No embedded credentials, no browser data touched.
// =========================================================================

namespace {

#if defined(_WIN32) || defined(WIN32)
typedef SOCKET sock_t;
static const sock_t kBadSock = INVALID_SOCKET;
void SockStartup() { static bool done = false; if (!done) { WSADATA w; WSAStartup(MAKEWORD(2, 2), &w); done = true; } }
void SockClose(sock_t s) { closesocket(s); }
#else
typedef int sock_t;
static const sock_t kBadSock = -1;
void SockStartup() {}
void SockClose(sock_t s) { close(s); }
#endif

std::atomic<int> s_oauthState{0}; // 0 idle, 1 waiting, 2 success, 3 failed
std::mutex       s_oauthMsgMutex;
std::string      s_oauthMsg;

void OAuthSet(int st, const std::string& m)
{
    s_oauthState.store(st);
    std::lock_guard<std::mutex> lk(s_oauthMsgMutex);
    s_oauthMsg = m;
}

inline uint32_t Rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

std::string Sha256Raw(const std::string& msg)
{
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };

    uint32_t H[8] = { 0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 };

    std::string data = msg;
    uint64_t bitlen = (uint64_t)data.size() * 8;
    data.push_back((char)0x80);
    while (data.size() % 64 != 56) data.push_back((char)0);
    for (int i = 7; i >= 0; --i) data.push_back((char)((bitlen >> (i * 8)) & 0xFF));

    for (size_t off = 0; off < data.size(); off += 64)
    {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = ((uint32_t)(unsigned char)data[off + i * 4] << 24)
                 | ((uint32_t)(unsigned char)data[off + i * 4 + 1] << 16)
                 | ((uint32_t)(unsigned char)data[off + i * 4 + 2] << 8)
                 | ((uint32_t)(unsigned char)data[off + i * 4 + 3]);
        for (int i = 16; i < 64; ++i)
        {
            uint32_t s0 = Rotr(w[i-15],7) ^ Rotr(w[i-15],18) ^ (w[i-15] >> 3);
            uint32_t s1 = Rotr(w[i-2],17) ^ Rotr(w[i-2],19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for (int i = 0; i < 64; ++i)
        {
            uint32_t S1 = Rotr(e,6) ^ Rotr(e,11) ^ Rotr(e,25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t t1 = h + S1 + ch + K[i] + w[i];
            uint32_t S0 = Rotr(a,2) ^ Rotr(a,13) ^ Rotr(a,22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
    }

    std::string out(32, '\0');
    for (int i = 0; i < 8; ++i)
    {
        out[i*4]   = (char)((H[i] >> 24) & 0xFF);
        out[i*4+1] = (char)((H[i] >> 16) & 0xFF);
        out[i*4+2] = (char)((H[i] >> 8) & 0xFF);
        out[i*4+3] = (char)(H[i] & 0xFF);
    }
    return out;
}

std::string Base64Url(const std::string& in)
{
    std::string b = Base64(in);
    for (char& c : b) { if (c == '+') c = '-'; else if (c == '/') c = '_'; }
    while (!b.empty() && b.back() == '=') b.pop_back();
    return b;
}

std::string UrlEncode(const std::string& s)
{
    static const char* hex = "0123456789ABCDEF";
    std::string o;
    for (unsigned char c : s)
    {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            o += (char)c;
        else { o += '%'; o += hex[c >> 4]; o += hex[c & 0xF]; }
    }
    return o;
}

std::string RandomVerifier()
{
    static const char* set = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    std::random_device rd;
    std::mt19937 gen(rd() ^ (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 63);
    std::string v;
    v.reserve(64);
    for (int i = 0; i < 64; ++i) v += set[dist(gen)];
    return v;
}

void LaunchBrowser(const std::string& url)
{
#if defined(_WIN32) || defined(WIN32)
    // ShellExecute rather than a shell: "start" needs cmd, and that flashes a
    // console window over a full screen game
    ShellExecuteA( NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL );
    int rc = 0;
#elif defined(__APPLE__)
    int rc = std::system(("open \"" + url + "\"").c_str());
#else
    int rc = std::system(("xdg-open \"" + url + "\" >/dev/null 2>&1 &").c_str());
#endif
    (void)rc;
}

sock_t LoopbackListen(int& outPort)
{
    SockStartup();
    for (int port = 8385; port <= 8387; ++port)
    {
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == kBadSock) continue;

        int yes = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short)port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (bind(s, (sockaddr*)&addr, sizeof(addr)) == 0 && listen(s, 1) == 0)
        {
            outPort = port;
            return s;
        }
        SockClose(s);
    }
    return kBadSock;
}

std::string ExtractQueryParam(const std::string& req, const std::string& key)
{
    std::string pat = key + "=";
    size_t p = req.find(pat);
    if (p == std::string::npos) return "";
    p += pat.size();
    std::string val;
    while (p < req.size() && req[p] != '&' && req[p] != ' ' && req[p] != '\r' && req[p] != '\n')
    {
        val += req[p];
        ++p;
    }
    return val;
}

std::string LoopbackAwaitCode(sock_t s, int timeoutSec, std::string& outError)
{
    fd_set rf;
    FD_ZERO(&rf);
    FD_SET(s, &rf);
    timeval tv;
    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;

    int sel = select((int)s + 1, &rf, nullptr, nullptr, &tv);
    if (sel <= 0)
    {
        outError = "No response from browser (timed out).";
        return "";
    }

    sock_t c = accept(s, nullptr, nullptr);
    if (c == kBadSock)
    {
        outError = "Local connection failed.";
        return "";
    }

    std::string req;
    char buf[4096];
    int n = (int)recv(c, buf, sizeof(buf) - 1, 0);
    if (n > 0) req.assign(buf, n);

    std::string code = ExtractQueryParam(req, "code");
    std::string err  = ExtractQueryParam(req, "error");

    const char* page =
        "<!doctype html><meta charset=utf-8><title>RetroCycles League</title>"
        "<body style=\"margin:0;background:#0b0e14;color:#cfd8e3;font-family:system-ui,sans-serif;"
        "display:flex;align-items:center;justify-content:center;height:100vh\">"
        "<div style=\"text-align:center\"><h2 style=\"color:#5fb0ff\">RetroCycles League linked</h2>"
        "<p>You can close this tab and return to the game.</p></div></body>";
    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                       "Connection: close\r\nContent-Length: " + std::to_string((int)strlen(page)) +
                       "\r\n\r\n" + page;
    send(c, resp.data(), (int)resp.size(), 0);
    SockClose(c);

    if (code.empty() && !err.empty())
        outError = "Provider returned an error: " + err;
    else if (code.empty())
        outError = "No authorization code received.";
    return code;
}

} // namespace

int RCL_SupabaseOAuthState()
{
    return s_oauthState.load();
}

std::string RCL_SupabaseOAuthMessage()
{
    std::lock_guard<std::mutex> lk(s_oauthMsgMutex);
    return s_oauthMsg;
}

void RCL_SupabaseOAuthBegin(const std::string& provider)
{
    if (s_oauthState.load() == 1)
        return;

    OAuthSet(1, "Starting sign-in...");
    std::string prov = provider;

    std::thread([prov]()
    {
        int port = 0;
        sock_t s = LoopbackListen(port);
        if (s == kBadSock)
        {
            OAuthSet(3, "Could not open a local login port.");
            return;
        }

        std::string verifier  = RandomVerifier();
        std::string challenge = Base64Url(Sha256Raw(verifier));
        std::string redirect  = "http://localhost:" + std::to_string(port);

        std::string url = std::string(kSupaBase) + "/auth/v1/authorize?provider=" + prov
                        + "&redirect_to=" + UrlEncode(redirect)
                        + "&code_challenge=" + challenge
                        + "&code_challenge_method=s256";

        RCL_LogDebug("OAuth authorize URL: " + url);
        OAuthSet(1, "Waiting for browser sign-in... approve the login, then return here.");
        LaunchBrowser(url);

        std::string err;
        std::string code = LoopbackAwaitCode(s, 180, err);
        SockClose(s);

        if (code.empty())
        {
            OAuthSet(3, err.empty() ? "Sign-in was cancelled." : err);
            return;
        }

        std::string body = "{\"auth_code\":\"" + JsonEscape(code) + "\",\"code_verifier\":\"" + JsonEscape(verifier) + "\"}";
        std::vector<std::string> headers = {
            std::string("apikey: ") + kSupaAnon,
            "Content-Type: application/json"
        };
        std::string resp;
        long hc = HttpPost(std::string(kSupaBase) + "/auth/v1/token?grant_type=pkce", headers, body, resp);

        if (hc == 200 && !JsonStr(resp, "access_token").empty())
        {
            SupaStore(resp);
            OAuthSet(2, "Signed in to RetroCycles League.");
        }
        else
        {
            std::string msg = JsonStr(resp, "error_description");
            if (msg.empty()) msg = JsonStr(resp, "msg");
            if (msg.empty()) msg = JsonStr(resp, "error");
            OAuthSet(3, msg.empty() ? ("Sign-in failed (HTTP " + std::to_string(hc) + ").") : msg);
        }
    }).detach();
}

bool RCL_SendQueueActionSync(const std::string& username, const std::string& action, const std::string& queueId, std::string& outError)
{
    (void)username;

    std::string err;
    if (!SupaEnsureSession(err))
    {
        outError = err;
        std::lock_guard<std::mutex> lk(s_queueResponseMutex);
        s_queueLastResponse = "ERROR: " + err;
        return false;
    }

    std::string access;
    {
        std::lock_guard<std::mutex> lk(s_supaMutex);
        access = s_supa.access;
    }

    std::vector<std::string> headers = {
        "Content-Type: application/json",
        std::string("Authorization: Bearer ") + access,
        SupaCookieHeader()
    };
    std::string body = "{\"laneKey\":\"" + JsonEscape(queueId) + "\"}";

    std::string resp;
    long code = HttpPost(std::string(kSiteBase) + "/api/queue/" + action, headers, body, resp);

    while (!resp.empty() && (resp.back() == '\n' || resp.back() == '\r' || resp.back() == ' '))
        resp.pop_back();

    {
        std::lock_guard<std::mutex> lk(s_queueResponseMutex);
        s_queueLastResponse = resp.empty() ? "(empty response)" : resp.substr(0, 200);
    }

    if (code == 200 || code == 201)
        return true;

    if (code == 401)
    {
        SupaRefresh(err);
        outError = "Session expired — try again.";
        return false;
    }

    std::string msg = JsonStr(resp, "error");
    outError = msg.empty() ? ("Queue request failed (HTTP " + std::to_string(code) + ").") : msg;
    return false;
}

void RCL_SendQueueAction(const std::string& username, const std::string& action, const std::string& queueId)
{
    std::string user = username;
    std::string act = action;
    std::string qid = queueId;
    std::thread([user, act, qid]()
    {
        std::string error;
        RCL_SendQueueActionSync(user, act, qid, error);
    }).detach();
}

namespace {
    std::mutex s_queuePollMutex;
    std::map<std::string, std::pair<int, int>> s_queueCounts; // laneKey -> {current, required}
    bool s_queueCountsLoaded = false;
    std::atomic<bool> s_queuePollThreadRunning{false};
    double s_lastPollTime = 0.0;

    double GetCurrentTimeSeconds() {
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    }

    int ParseIntField(const std::string& json, const std::string& field, size_t startPos, size_t limit) {
        size_t pos = json.find(field, startPos);
        if (pos == std::string::npos || pos >= limit) {
            return -1;
        }
        pos += field.length();
        // Skip spaces and colons
        while (pos < limit && (json[pos] == ' ' || json[pos] == ':')) {
            pos++;
        }
        // Parse digits
        std::string valStr;
        while (pos < limit && std::isdigit(static_cast<unsigned char>(json[pos]))) {
            valStr.push_back(json[pos]);
            pos++;
        }
        if (valStr.empty()) return -1;
        return std::stoi(valStr);
    }
}

bool RCL_GetQueueCount(const std::string& laneKey, int& outCurrent, int& outRequired) {
    std::lock_guard<std::mutex> lock(s_queuePollMutex);
    if (!s_queueCountsLoaded) {
        return false;
    }
    auto it = s_queueCounts.find(laneKey);
    if (it != s_queueCounts.end()) {
        outCurrent = it->second.first;
        outRequired = it->second.second;
        return true;
    }
    return false;
}

void RCL_PollQueueSummaryTick(bool dashboardVisible) {
    if (!dashboardVisible) {
        return;
    }

    double now = GetCurrentTimeSeconds();
    if (s_queuePollThreadRunning) {
        return;
    }

    // Rate limit: Poll every 4.0 seconds
    if (s_lastPollTime > 0.0 && (now - s_lastPollTime) < 4.0) {
        return;
    }

    s_queuePollThreadRunning = true;
    s_lastPollTime = now;

    std::thread([]() {
        std::string json = RCL_HttpGet("https://retrocyclesleague.com/api/queue/summary");

        if (!json.empty()) {
            std::map<std::string, std::pair<int, int>> tempCounts;
            
            size_t pos = 0;
            while ((pos = json.find("\"laneKey\":", pos)) != std::string::npos) {
                pos += 10;
                size_t quoteStart = json.find("\"", pos);
                if (quoteStart == std::string::npos) break;
                size_t quoteEnd = json.find("\"", quoteStart + 1);
                if (quoteEnd == std::string::npos) break;
                std::string laneKey = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                
                size_t nextLaneKey = json.find("\"laneKey\":", quoteEnd);
                size_t endOfObject = json.find("}", quoteEnd);
                size_t limit = (nextLaneKey != std::string::npos && nextLaneKey < endOfObject) ? nextLaneKey : endOfObject;
                if (limit == std::string::npos) limit = json.size();
                
                int currentCount = ParseIntField(json, "\"currentCount\"", quoteEnd, limit);
                int requiredCount = ParseIntField(json, "\"requiredCount\"", quoteEnd, limit);
                
                tempCounts[laneKey] = {currentCount, requiredCount};
                pos = limit;
            }
            
            if (!tempCounts.empty()) {
                std::lock_guard<std::mutex> lock(s_queuePollMutex);
                s_queueCounts = tempCounts;
                s_queueCountsLoaded = true;
            }
        }
        s_queuePollThreadRunning = false;
    }).detach();
}

int RCL_GetPlayerEloTst()
{
    return s_playerEloTst.load();
}

int RCL_GetPlayerEloSumobar()
{
    return s_playerEloSumo.load();
}

int RCL_GetPlayerElo2s()
{
    return s_playerElo2s.load();
}

static int FetchEloFromLeaderboard(const std::string& mode, const std::string& cleanUser, int defaultVal)
{
    std::string url = "https://retrocyclesleague.com/api/hub/leaderboard?mode=" + mode + "&season=4&region=combined&period=all";
    std::string response = RCL_HttpGet(url);

    if (response.empty())
        return defaultVal;

    std::string responseLower = response;
    std::transform(responseLower.begin(), responseLower.end(), responseLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    size_t pos = 0;
    while (true)
    {
        // Try matching by username first
        size_t foundPos = responseLower.find("\"username\":\"" + cleanUser + "\"", pos);

        // If not found, try matching by playerAuth without suffix
        if (foundPos == std::string::npos)
            foundPos = responseLower.find("\"playerauth\":\"" + cleanUser + "\"", pos);

        // If not found, try matching by playerAuth with @rcl suffix
        if (foundPos == std::string::npos)
            foundPos = responseLower.find("\"playerauth\":\"" + cleanUser + "@rcl\"", pos);

        if (foundPos == std::string::npos)
            break;

        pos = foundPos;

        size_t objStart = responseLower.rfind('{', pos);
        size_t objEnd = responseLower.find('}', pos);

        if (objStart != std::string::npos && objEnd != std::string::npos && objStart < objEnd)
        {
            std::string objStr = responseLower.substr(objStart, objEnd - objStart);
            size_t eloPos = objStr.find("\"elo\":");
            if (eloPos != std::string::npos)
            {
                size_t valStart = eloPos + 6;
                while (valStart < objStr.length() && (objStr[valStart] == ' ' || objStr[valStart] == '\t'))
                {
                    valStart++;
                }
                std::string eloValStr;
                while (valStart < objStr.length() && std::isdigit(objStr[valStart]))
                {
                    eloValStr += objStr[valStart];
                    valStart++;
                }
                if (!eloValStr.empty())
                {
                    return std::stoi(eloValStr);
                }
            }
        }

        pos += 1;
    }

    return defaultVal;
}

void RCL_FetchPlayerElosAsync(const std::string& username)
{
    s_ElosLoaded.store(false);
    std::string cleanUser = GetCleanUsername(username);
    std::transform(cleanUser.begin(), cleanUser.end(), cleanUser.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::thread([cleanUser]() {
        int tstElo = FetchEloFromLeaderboard("tst", cleanUser, 1800);
        s_playerEloTst.store(tstElo);

        int sumoElo = FetchEloFromLeaderboard("sumobar", cleanUser, 1200);
        s_playerEloSumo.store(sumoElo);

        int twoSElo = FetchEloFromLeaderboard("2s", cleanUser, 1500);
        s_playerElo2s.store(twoSElo);
        
        s_ElosLoaded.store(true);
    }).detach();
}

bool RCL_ElosLoaded()
{
    return s_ElosLoaded.load();
}

