/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
***************************************************************************

*/

#ifndef gMenus_H
#define gMenus_H

#include "uMenu.h"

extern void  sg_PlayerMenu();
extern uMenu sg_screenMenu;
void         sg_ConsoleInput(); // let the player enter one line of console input
void         sg_ModSettingsMenu(); // mod settings menu

// [MOD] Settings variables (defined in gMenus.cpp)
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
extern REAL sg_modMinimapRotateSpeed;
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

// [MOD] Visual Overhaul Settings
extern bool sg_modParticleSystemEnabled;
extern int  sg_modDeathParticlesCount;
extern bool sg_modScreenShakeEnabled;
extern REAL sg_modScreenShakeIntensity;
extern bool sg_modAmbientParticlesEnabled;
extern int sg_modAmbientParticlesMode;
extern int sg_modAmbientParticlesMin;
extern int sg_modAmbientParticlesMax;
extern bool sg_modTrailGradientEnabled;
extern REAL sg_modTrailAlphaTop;
extern REAL sg_modTrailAlphaBottom;
extern REAL sg_modWallHeightMultiplier;

// [MOD] HUD and Editing Settings
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
