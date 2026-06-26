

#ifndef gMenus_H
#define gMenus_H

#include "uMenu.h"

extern void  sg_PlayerMenu();
extern uMenu sg_screenMenu;
void         sg_ConsoleInput(); 
void         sg_ModSettingsMenu(); 


extern bool sg_modWaypointsEnabled;
extern bool sg_modWaypointsEnemyWalls;
extern REAL sg_modWaypointsMinRubber;
extern REAL sg_modWaypointsCooldown;
extern bool sg_modRubberGaugeEnabled;
extern bool sg_modMinimapEnabled;
extern bool sg_modRubberBatteryEnabled;
extern bool sg_modClassicRubberBatteryEnabled;
extern REAL sg_modMinimapScale;
extern REAL sg_modMinimapZoom;
extern bool sg_modMinimapRotate;
extern bool sg_modProximityWarning;
extern bool sg_modZoneTimers;
extern bool sg_modCutoffAimbot;
extern REAL sg_modWaypointsLifetime;
extern bool sg_modPathLineEnabled;
extern bool sg_modTeammateDeathWarning;
extern bool sg_modFortressAlerts;
extern int  sg_modPathDepth;
extern int  sg_modPathRange;
extern bool sg_modScoreboardEnabled;
extern bool sg_modAnti360LockEnabled;
extern bool sg_modChatCalcEnabled;
extern bool sg_modTrashTalkEnabled;
extern bool sg_modKDEnabled;
extern bool sg_modKDResetFlag;
extern int  sg_kdKills;
extern int  sg_kdDeaths;
extern bool sg_modAutoGreet;
extern bool sg_modFriendlyChat;
extern bool sg_modColorPicker;
extern bool sg_modKillSoundsEnabled;
extern int  sg_modKillAnnouncerPack;
extern tString sg_activeCameraConfig;
extern tString sg_activeTexturePack;


extern bool sg_modParticleSystemEnabled;
extern int  sg_modDeathParticlesCount;
extern bool sg_modScreenShakeEnabled;
extern REAL sg_modScreenShakeIntensity;
extern bool sg_modAmbientParticlesEnabled;
extern bool sg_modTrailGradientEnabled;
extern REAL sg_modTrailAlphaTop;
extern REAL sg_modTrailAlphaBottom;


extern bool sg_modClientNameEnabled;
extern bool sg_modFpsEnabled;
extern bool sg_modPingEnabled;
extern bool sg_modTimeEnabled;
extern bool sg_modKeybindsEnabled;
extern bool isHudEditing;
extern REAL sg_modClientNamePosX;
extern REAL sg_modClientNamePosY;
extern REAL sg_modFpsPosX;
extern REAL sg_modFpsPosY;
extern REAL sg_modPingPosX;
extern REAL sg_modPingPosY;
extern REAL sg_modTimePosX;
extern REAL sg_modTimePosY;
extern REAL sg_modKeybindsPosX;
extern REAL sg_modKeybindsPosY;
extern bool sg_modAliveWidgetEnabled;
extern REAL sg_modAliveWidgetPosX;
extern REAL sg_modAliveWidgetPosY;

#ifndef DEDICATED
#include <SDL3/SDL.h>

class cVisualMenu {
public:
    static void Init();
    static void Toggle();
    static void Render();
    static bool HandleEvent(const SDL_Event& event);

    static bool isOpen;
    static float menuAlpha;
};
#endif

#endif
