// =====================================================================
// PREMIUM MOD MENU - Dear ImGui based
// RetroyCycles Client Mod | Inspired by Osiris, Foxyz, Raticks
// =====================================================================
#pragma once

// Set to 1 for public release builds.
#ifndef PUBLIC_BUILD
#define PUBLIC_BUILD 0
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DEDICATED
#include <SDL3/SDL.h>
#include <functional>
#include <string>

class ePlayerNetID;

extern bool g_NoclipMode;

namespace ModMenu {
    void Init();
    void Shutdown();
    void Render();      // Main render loop - called every frame via overlay
    void RenderInner(); // Inner layout rendering - decoupled from overlay NewFrame/Render
    void Toggle();
    void OpenTab(int tabId);
    bool ProcessEvent(const SDL_Event* event);
    bool IsOpen();
    void SetOpen(bool open);
    void ApplySettingsToEngine();

    extern bool g_MainMenuActive;
    extern bool g_CustomMainMenuTempDisabled;
    extern bool g_InGameMenuOpen;

    //! Whether one of our own blocking overlays owns the screen.
    //!
    //! The login box and the server notices run their own loop, outside the
    //! menu flags everything else keys off. Without this the input filter
    //! decides no menu is open, turns relative mouse mode back on every frame
    //! while the overlay turns it off again, and the pointer flickers and
    //! cannot be aimed.
    extern bool g_ModalOverlayOpen;
    extern std::function<void()> g_PendingLegacyMenuAction;
    void RunCustomMainMenu();
    void RunCustomInGameMenu();

    //! The server's greeting, in the client's own clothes. Blocks until the
    //! reader dismisses it or the server's timeout runs out, the same way the
    //! screen it replaces did.
    void ShowServerMessage( char const * title, char const * body, float timeout );

    //! The screen shown when a game ends against our will.
    //!
    //! kind: 0 terminated by the server, 1 login denied, 2 no answer,
    //! 3 connection lost, 4 gamestate never arrived.
    void ShowDisconnect( int kind, char const * title, char const * body,
                         char const * reason, char const * redirect, float timeout );

    //! The login a server asks for. Blocks; returns false when the reader
    //! walked away. The password is handed straight back and kept nowhere.
    bool AskLogin( char const * heading, std::string & username,
                   std::string & password, int & storeMode );
    void RenderInGameTeams(float width);
    void RenderInGameSettings(float width);
    void RenderInGameVotingAndPolice(float width);

    // Style initialization
    void InitStyle();

    // Custom widgets
    bool AnimatedToggle(const char* label, bool* v);
    bool AnimatedSlider(const char* label, float* v, float v_min, float v_max);

    // Queue and Chat Command Helpers
    bool JoinQueue(const std::string& laneKey);
    bool LeaveQueue(const std::string& laneKey);
    bool IsInQueue(const std::string& laneKey);
    bool CheckEligibility(const std::string& laneKey, std::string& outError);
    bool HandleChatCommand(const std::string& msg);
    void SendLocalChat(const char* text);
    void HandleIncomingChat(ePlayerNetID* sender, const std::string& msg);
    void SendPublicChat(const std::string& text);
}
//! Moves the rider onto the team they asked for, once it exists.
void rc_FollowPreferredTeam();

//! Keeps asking for a place while a full server has none, and takes the first
//! one that opens.
void rc_TakeFreeSlot();
extern bool g_TakeFreeSlot;

#endif
