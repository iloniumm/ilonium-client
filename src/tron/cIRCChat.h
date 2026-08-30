#ifndef C_IRC_CHAT_H
#define C_IRC_CHAT_H

#include <string>
#include <vector>
#include <mutex>

// The chat as the client sees it. Nobody signs in: the key made on first run
// says who is speaking, and the tag beside a name is whatever the server says
// it is. There is no password here to store, leak or forget.

struct IRCMessage {
    int id = 0;
    std::string login;
    std::string in_game_name;
    std::string message;
    long long timestamp = 0;
    std::string role_tag;
    std::string role_color;
};

class cIRCChat {
public:
    static void Init();
    static std::string GetHWID();

    //! The first characters of our public key. Shown so a person can tell the
    //! staff which one they are without a name to go by.
    static std::string GetIdentity();

    //! What the server last said we carry. Empty for almost everyone.
    static std::string GetRoleTag();
    static std::string GetRoleColor();

    static void UpdatePolling(bool tabActive);
    static void PollMessagesAsync();
    static bool SendChatMessage(const std::string& text, std::string& outError);
    static double GetCooldownRemaining();

    static std::vector<IRCMessage> GetMessages();
    static std::string GetStatusMessage();
    static void SetStatusMessage(const std::string& status);

private:
    static std::string s_HWID;
    static bool s_Initialized;

    static std::mutex s_MessageMutex;
    static std::vector<IRCMessage> s_Messages;
    static std::string s_StatusMessage;

    static double s_LastPollTime;
    static double s_LastSendTime;
    static bool s_IsPolling;
    static bool s_IsSending;

    static std::string ComputeHWID();
    static std::string MD5String(const std::string& input);
    static void PollMessagesSync();
};

#endif // C_IRC_CHAT_H
