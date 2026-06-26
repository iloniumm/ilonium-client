



#pragma once

#ifndef DEDICATED
#include <SDL3/SDL.h>
#include <functional>

namespace ModMenu {
    void Init();
    void Shutdown();
    void Render();      
    void RenderInner(); 
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

    
    void InitStyle();

    
    bool AnimatedToggle(const char* label, bool* v);
    bool AnimatedSlider(const char* label, float* v, float v_min, float v_max);
}
#endif
