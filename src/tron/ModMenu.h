// =====================================================================
// PREMIUM MOD MENU - Dear ImGui based
// RetroyCycles Client Mod | Inspired by Osiris, Foxyz, Raticks
// =====================================================================
#pragma once

// Set to 1 for public release build, 0 for private user build with cheats (ai trash talker, auto escape, perfect turn)
#ifndef PUBLIC_BUILD
#define PUBLIC_BUILD 0
#endif

#ifndef DEDICATED
#include <SDL3/SDL.h>
#include <functional>

namespace ModMenu {
    void Init();
    void Shutdown();
    void Render();      // Main render loop - called every frame via overlay
    void RenderInner(); // Inner layout rendering - decoupled from overlay NewFrame/Render
    void Toggle();
    bool ProcessEvent(const SDL_Event* event);
    bool IsOpen();
    void SetOpen(bool open);
    void ApplySettingsToEngine();

    extern bool g_MainMenuActive;
    extern bool g_CustomMainMenuTempDisabled;
    extern bool g_InGameMenuOpen;
    extern std::function<void()> g_PendingLegacyMenuAction;
    void RunCustomMainMenu();
    void RunCustomInGameMenu();
    void RenderInGameTeams(float width);
    void RenderInGameSettings(float width);
    void RenderInGameVotingAndPolice(float width);

    // Style initialization
    void InitStyle();

    // Custom widgets
    bool AnimatedToggle(const char* label, bool* v);
    bool AnimatedSlider(const char* label, float* v, float v_min, float v_max);
}
#endif
