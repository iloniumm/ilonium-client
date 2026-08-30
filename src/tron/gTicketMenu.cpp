#include "config.h"
#include "cBackend.h"
#include "gTicketMenu.h"
#include "cShell.h"
#include "ePlayer.h"
#include "tDirectories.h"
#include "rConsole.h"
#include "rGL.h"
#include "rScreen.h"
#include "rFont.h"
#include "cIRCChat.h"
#include "RCLAuth.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <cctype>

#if defined(_WIN32) || defined(WIN32)
#define RC_POPEN  _popen
#define RC_PCLOSE _pclose
#else
#define RC_POPEN  popen
#define RC_PCLOSE pclose
#endif

// Cloudflare Worker URL endpoint for ticket submission
#ifndef WORKER_TICKET_URL
#define WORKER_TICKET_URL RC_URL_TICKET
#endif

// ============================================================================
// uMenuItemPlaceholderString Implementation
// ============================================================================

uMenuItemPlaceholderString::uMenuItemPlaceholderString(uMenu *M, const tOutput& desc, const tOutput& help, tString &c, const tOutput& placeholder, int maxLength)
    : uMenuItemString(M, desc, help, c, maxLength), placeholder_(placeholder)
{
}

void uMenuItemPlaceholderString::Render(REAL x, REAL y, REAL alpha, bool selected) {
#ifndef DEDICATED
    // Check if the input content is empty (tString length <= 1 considering null terminator)
    int len = content ? content->Len() - 1 : 0;

    if (len <= 0) {
        // 1. Render Description Label on the left
        DisplayText(x - 0.02f, y, description, selected, alpha, 1);

        // 2. Render Placeholder text in Gray on the right
        rTextField::SetDefaultColor(tColor(0.5f, 0.5f, 0.5f, alpha));
        rTextField::SetBlendColor(tColor(0.5f, 0.5f, 0.5f, alpha));

        int cmode = selected ? 1 : 0;
        DisplayText(x + 0.02f, y, placeholder_, selected, alpha, -1, cmode, 0, rTextField::COLOR_USE);
    } else {
        // Content typed by user -> render normal white user text
        uMenuItemString::Render(x, y, alpha, selected);
    }
#endif
}

// ============================================================================
// Helper Menu Item Classes for Send Button, Cooldown & Ban Notices
// ============================================================================

class uMenuItemSendTicket : public uMenuItemAction {
protected:
    gMenuTicketSystem* owner_;

public:
    uMenuItemSendTicket(uMenu* M, gMenuTicketSystem* owner)
        : uMenuItemAction(M, "Send Ticket", "Submit ticket payload to Cloudflare support worker"), owner_(owner)
    {
    }

    virtual void Enter() override {
        if (owner_) {
            owner_->SendTicket();
        }
    }
};

class uMenuItemBannedNotice : public uMenuItem {
public:
    uMenuItemBannedNotice(uMenu* M)
        : uMenuItem(M, "ACCOUNT / DEVICE BANNED FROM SUPPORT SYSTEM")
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1.0f, bool selected = false) override {
#ifndef DEDICATED
        rTextField::SetDefaultColor(tColor(1.0f, 0.2f, 0.2f, alpha));
        rTextField::SetBlendColor(tColor(1.0f, 0.2f, 0.2f, alpha));
        DisplayText(x, y, "ACCOUNT / HWID BANNED FROM TICKET SYSTEM", selected, alpha, 0);
#endif
    }
};

class uMenuItemRefreshUnban : public uMenuItemAction {
protected:
    gMenuTicketSystem* owner_;

public:
    uMenuItemRefreshUnban(uMenu* M, gMenuTicketSystem* owner)
        : uMenuItemAction(M, "Refresh Unban Status", "Clear local ban cache and attempt reconnect"), owner_(owner)
    {
    }

    virtual void Enter() override {
        tString bPath = rc_InternalPath("ticket_banned.cfg");
        std::remove((const char*)bPath);
        tConsole::Message("Ticket System", "Ban status refreshed! Try re-entering the ticket menu.", 5.0f);
        if (owner_) {
            owner_->Exit();
        }
    }
};

class uMenuItemCooldownNotice : public uMenuItem {
protected:
    gMenuTicketSystem* owner_;

public:
    uMenuItemCooldownNotice(uMenu* M, gMenuTicketSystem* owner)
        : uMenuItem(M, "Ticket submission cooldown active"), owner_(owner)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1.0f, bool selected = false) override {
#ifndef DEDICATED
        if (owner_) {
            tString text;
            text << "Cooldown: " << owner_->GetCooldownMinutesLeft() << " min left";

            rTextField::SetDefaultColor(tColor(1.0f, 0.7f, 0.2f, alpha));
            rTextField::SetBlendColor(tColor(1.0f, 0.7f, 0.2f, alpha));

            DisplayText(x, y, text, selected, alpha, 0);
        }
#endif
    }
};

// ============================================================================
// gMenuTicketSystem Implementation
// ============================================================================

gMenuTicketSystem::gMenuTicketSystem()
    : uMenu("Support & Bug Reports", false),
      itemLogin_(nullptr), itemTitle_(nullptr), itemDesc_(nullptr),
      itemSend_(nullptr), itemCooldownNotice_(nullptr), itemExit_(nullptr),
      lastSendTime_(0), isCooldownActive_(false), cooldownMinutesLeft_(0)
{
}

gMenuTicketSystem::~gMenuTicketSystem() {
}

void gMenuTicketSystem::CheckCooldown() {
    lastSendTime_ = 0;
    isCooldownActive_ = false;
    cooldownMinutesLeft_ = 0;

    std::ifstream file;
    if (tDirectories::Var().Open(file, "ticket_timer.cfg")) {
        std::time_t timestamp = 0;
        if (file >> timestamp) {
            lastSendTime_ = timestamp;
        }
        file.close();
    }

    std::time_t now = std::time(nullptr);
    if (lastSendTime_ > 0 && (now - lastSendTime_) < 3600) {
        isCooldownActive_ = true;
        std::time_t diff = 3600 - (now - lastSendTime_);
        cooldownMinutesLeft_ = static_cast<int>((diff + 59) / 60);
    }
}

void gMenuTicketSystem::SaveCooldownTimestamp() {
    std::time_t now = std::time(nullptr);
    tString path = rc_InternalPath("ticket_timer.cfg");
    std::ofstream file((const char*)path, std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        file << now << "\n";
        file.close();
    }
}

void gMenuTicketSystem::OnEnter() {
    CheckCooldown();

    while (items.Len() > 0) {
        delete items[items.Len() - 1];
    }

    // Check if client has ticket_banned.cfg on disk
    tString bannedCfgPath = rc_InternalPath("ticket_banned.cfg");
    std::ifstream bCheck((const char*)bannedCfgPath);
    bool isBanned = bCheck.is_open();
    if (bCheck.is_open()) bCheck.close();

    if (isBanned) {
        // Display Banned warning and Refresh Unban status action
        tNEW(uMenuItemBannedNotice)(this);
        tNEW(uMenuItemRefreshUnban)(this, this);
    } else if (isCooldownActive_) {
        itemCooldownNotice_ = tNEW(uMenuItemCooldownNotice)(this, this);
    } else {
        // Pre-fill login if logged into RCL
        std::string curLogin = ePlayer::PlayerConfig(0) ? (char const *)ePlayer::PlayerConfig(0)->Name() : std::string();
        if (!curLogin.empty() && (loginStr_.Len() <= 1)) {
            loginStr_ = curLogin.c_str();
        }

        itemLogin_ = tNEW(uMenuItemPlaceholderString)(this, "RCL Login", "Enter your RCL username", loginStr_, "Login here...", 128);
        itemTitle_ = tNEW(uMenuItemPlaceholderString)(this, "Ticket Title", "Brief summary of your issue", titleStr_, "Brief title...", 256);
        itemDesc_  = tNEW(uMenuItemPlaceholderString)(this, "Description", "Detailed description of the issue", descStr_, "Describe your problem...", 1024);

        itemSend_  = tNEW(uMenuItemSendTicket)(this, this);
    }

    itemExit_ = tNEW(uMenuItemExit)(this);

    uMenu::OnEnter();
}

void gMenuTicketSystem::OnRender() {
    uMenu::OnRender();
}

void gMenuTicketSystem::SendTicket() {
    int loginLen = loginStr_.Len() - 1;
    int titleLen = titleStr_.Len() - 1;
    int descLen  = descStr_.Len() - 1;

    if (loginLen <= 0 || titleLen <= 0 || descLen <= 0) {
        tConsole::Message("Ticket System", "Please fill in all fields (Login, Title, Description)!", 10.0f);
        return;
    }

    std::string login = (const char*)loginStr_;
    std::string title = (const char*)titleStr_;
    std::string msg   = (const char*)descStr_;
    std::string hwid  = cIRCChat::GetHWID();
    std::string rclHash = RCL_GetAuthHash();

    std::string gpu = "Unknown GPU";
#ifndef DEDICATED
    if (sr_glOut) {
        const char* renderer = (const char*)glGetString(GL_RENDERER);
        if (renderer) {
            gpu = renderer;
        }
    }
#endif

    std::string osName = "Linux";
#ifdef _WIN32
    osName = "Windows";
#elif defined(__APPLE__)
    osName = "macOS";
#endif

    auto jsonEscape = [](const std::string& input) -> std::string {
        std::string out;
        for (char c : input) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else out += c;
        }
        return out;
    };

    std::string payload = "{\n"
        "  \"rcl_username\": \"" + jsonEscape(login) + "\",\n"
        "  \"rcl_hash\": \"" + jsonEscape(rclHash) + "\",\n"
        "  \"hwid\": \"" + jsonEscape(hwid) + "\",\n"
        "  \"title\": \"" + jsonEscape(title) + "\",\n"
        "  \"message\": \"" + jsonEscape(msg) + "\",\n"
        "  \"gpu\": \"" + jsonEscape(gpu) + "\",\n"
        "  \"os\": \"" + jsonEscape(osName) + "\"\n"
        "}";

    std::thread([payload]() {
        tString payloadPath = rc_InternalPath("ticket_payload.json");
        std::ofstream pfile((const char*)payloadPath, std::ios::out | std::ios::trunc);
        if (pfile.is_open()) {
            pfile << payload;
            pfile.close();
        }

#if defined(_WIN32) || defined(WIN32)
        std::string cmd = "curl -s -k -w \"\\n%{http_code}\" -X POST -H \"Content-Type: application/json\" -d @\"" + std::string((const char*)payloadPath) + "\" \"" WORKER_TICKET_URL "\"";
#else
        static const char* curlPaths[] = {
            "/usr/bin/curl",
            "/run/host/usr/bin/curl",
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
        std::string cmd = "LD_LIBRARY_PATH=\"\" " + curlBin + " -s -k -w \"\\n%{http_code}\" -X POST -H \"Content-Type: application/json\" -d @\"" + std::string((const char*)payloadPath) + "\" \"" WORKER_TICKET_URL "\"";
#endif

        std::string response;
        if (rc_RunHidden(cmd, response)) {

            while (!response.empty() && (response.back() == '\n' || response.back() == '\r' || response.back() == ' ')) {
                response.pop_back();
            }

            int httpCode = 0;
            size_t lastNewline = response.rfind('\n');
            std::string codeStr = (lastNewline != std::string::npos) ? response.substr(lastNewline + 1) : response;
            codeStr.erase(std::remove_if(codeStr.begin(), codeStr.end(), ::isspace), codeStr.end());
            httpCode = std::atoi(codeStr.c_str());

            if (httpCode == 403) {
                tString bPath = rc_InternalPath("ticket_banned.cfg");
                std::ofstream bOut((const char*)bPath, std::ios::out | std::ios::trunc);
                if (bOut.is_open()) {
                    bOut << "BANNED\n";
                    bOut.close();
                }
            } else if (httpCode == 200 || httpCode == 201) {
                tString bPath = rc_InternalPath("ticket_banned.cfg");
                std::remove((const char*)bPath);
            }
        }
    }).detach();

    SaveCooldownTimestamp();
    tConsole::Message("Ticket System", "Ticket sent successfully!", 15.0f);
    Exit();
}

void sg_TicketMenu() {
    gMenuTicketSystem menu;
    menu.Enter();
}
