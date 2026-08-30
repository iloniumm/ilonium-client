#pragma once
#ifndef RCL_AUTH_H
#define RCL_AUTH_H

#include <string>

// RCL (Retrocycles League) Authentication
// =========================================
// Uses double-MD5 with the armagetronad authority format, then queries
// https://rcl.authentication.armagetronad.net/armaauth/0.1
//
// Hashing steps (matches the reference bash script):
//   FIRST = md5hex( username + ":aaauth:" + password + ":retrocyclesleague.com" )
//   HASH  = md5hex( raw_bytes_of_FIRST )   // i.e. md5( hex_decode( FIRST ) )
//
// HTTP check:
//   GET https://rcl.authentication.armagetronad.net/armaauth/0.1
//       ?query=check&user=<user>&hash=<HASH>&method=md5
//   Response "LOGIN_OK" → success, anything else → failure.

// -----------------------------------------------------------------
// Possible states of an ongoing or completed login attempt.
// -----------------------------------------------------------------
enum class RCLAuthState
{
    Idle,       ///< No request in flight.
    Pending,    ///< Request dispatched, waiting for result.
    Success,    ///< Server returned LOGIN_OK.
    Failed,     ///< Wrong credentials or network error.
};

// -----------------------------------------------------------------
// Kick off a non-blocking login attempt.
// Safe to call from the ImGui render/button handler.
// The result is polled via RCL_GetAuthState() / RCL_GetAuthError().
// -----------------------------------------------------------------
void RCL_Login(const std::string& username, const std::string& password);

// -----------------------------------------------------------------
// Query current state (call every frame from ImGui).
// -----------------------------------------------------------------
RCLAuthState RCL_GetAuthState();

// -----------------------------------------------------------------
// Human-readable error message, valid when state == Failed.
// -----------------------------------------------------------------
const std::string& RCL_GetAuthError();

// -----------------------------------------------------------------
// Reset to Idle so the user can try again.
// -----------------------------------------------------------------
void RCL_ResetAuthState();

// -----------------------------------------------------------------
// Get the double-MD5 hash of the successfully authenticated session.
// -----------------------------------------------------------------
std::string RCL_GetAuthHash();

// -----------------------------------------------------------------
// Synchronous version (blocks the calling thread).
// Useful for testing or calling from a background thread you spawn
// yourself.  Returns true on LOGIN_OK.
// -----------------------------------------------------------------
bool RCL_AuthenticateSync(const std::string& username,
                          const std::string& password,
                          std::string&       outError);

// -----------------------------------------------------------------
// Session Persistence and Auto-Login
// -----------------------------------------------------------------
bool RCL_AttemptAutoLogin(std::string& outUsername);
void RCL_Logout();
bool RCL_SaveCredentials(const std::string& username, const std::string& hash);

// -----------------------------------------------------------------
// RetroCycles League website session (Supabase). Established from the
// same credentials used for the in-game login; no browser required.
// -----------------------------------------------------------------
bool RCL_SupabaseLoginSync(const std::string& identifier, const std::string& password, std::string& outError);
void RCL_SupabaseLoginAsync(const std::string& identifier, const std::string& password);
bool RCL_SupabaseIsLinked();
void RCL_SupabaseLogout();
bool RCL_SupabaseImportSession(const std::string& sessionJson, std::string& outError);

// Discord / Google sign-in via a one-time browser consent (PKCE loopback).
// provider is "discord" or "google". Poll state: 0 idle, 1 waiting, 2 ok, 3 failed.
void        RCL_SupabaseOAuthBegin(const std::string& provider);
int         RCL_SupabaseOAuthState();
std::string RCL_SupabaseOAuthMessage();

// -----------------------------------------------------------------
// Queue Actions (Zero Trust join / leave)
// -----------------------------------------------------------------
void RCL_SendQueueAction(const std::string& username, const std::string& action, const std::string& queueId);
bool RCL_SendQueueActionSync(const std::string& username, const std::string& action, const std::string& queueId, std::string& outError);
// Returns the last raw server response from a join/leave action (for debug display in UI)
const std::string& RCL_GetQueueLastResponse();

// -----------------------------------------------------------------
// Queue Summary Polling (Dashboard live data)
// -----------------------------------------------------------------
bool RCL_GetQueueCount(const std::string& laneKey, int& outCurrent, int& outRequired);
void RCL_PollQueueSummaryTick(bool dashboardVisible);

// -----------------------------------------------------------------
// Player ELO Retrieval (Live leaderboard fetching)
// -----------------------------------------------------------------
int RCL_GetPlayerEloTst();
int RCL_GetPlayerEloSumobar();
int RCL_GetPlayerElo2s();
void RCL_FetchPlayerElosAsync(const std::string& username);
bool RCL_ElosLoaded();

#endif // RCL_AUTH_H
