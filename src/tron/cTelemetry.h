// How many people are running the client.
//
// The client says hello every few minutes and says nothing else: no counts, no
// invented identifier, no shared secret. The request is signed with the key
// this installation made on first run - see cIlonium.h - so the far end knows
// it is the same machine as yesterday and can tell one player from a thousand
// copies of a forged one. Every figure on the stats page is counted there.

#ifndef C_TELEMETRY_H
#define C_TELEMETRY_H

#include <string>

class cTelemetry {
public:
    //! Makes sure the key exists. Safe to call more than once.
    static void Init();

    //! One hello now.
    static void Say();

    //! Says hello every five minutes, on a thread of its own.
    static void StartHeartbeat();

    //! The public half of this installation's key.
    static std::string GetClientId();

private:
    static bool s_HeartbeatStarted;
};

//! Whether the backend is handing out a build newer than this one.
bool rc_UpdateWaiting();

//! What that build is called, what changed, and where it lives.
std::string rc_UpdateVersion();
std::string rc_UpdateNotes();
std::string rc_UpdateUrl();

#endif // C_TELEMETRY_H
