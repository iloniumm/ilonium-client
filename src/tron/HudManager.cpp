// =====================================================================
// MODERN MODULAR HUD & EDIT MODE SYSTEM
// RetroCycles Client Mod | "ilonium"
// =====================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef DEDICATED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "thirdparty/imgui/imgui_internal.h"
#include "HudManager.h"
#include "cDiscord.h"
#include "MediaWidget.h"
#include "gLogins.h"
#include "ModMenu.h"

#include "ePlayer.h"
#include "eTimer.h"
#include "gCycle.h"
#include "gWinZone.h"
#include "eGrid.h"
#include "eTeam.h"
#include "ModTheme.h"
#include <SDL3/SDL.h>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include "tools/tConfiguration.h"
#include "tools/tRecorder.h"
#include "rConsole.h"
#include "engine/eRectangle.h"
#include "engine/eAdvWall.h"
#include "tron/gWall.h"
#include "tSysTime.h"
#include "tOwnership.h"

RC_OWNERSHIP( hud )


// Expose standard client configuration variables
extern bool sg_modClientNameEnabled;
extern bool sg_modFpsEnabled;
extern bool sg_modPingEnabled;
extern bool sg_modTimeEnabled;
extern bool sg_modKeybindsEnabled;
extern bool isHudEditing;
extern bool sg_modMinimapEnabled;

// Configuration positions
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

// Global settings
// Global settings
bool sg_modClientNameEnabled = true;
bool sg_modFpsEnabled = true;
bool sg_modPingEnabled = true;
bool sg_modTimeEnabled = true;
bool sg_modKeybindsEnabled = true;
bool sg_modKDWidgetEnabled = true;
bool sg_modSpeedometerEnabled = true;
bool sg_modRubberMeterEnabled = true;
bool sg_modBrakeMeterEnabled = true;
bool sg_modScoreboardWidgetEnabled = true;
bool isHudEditing = false;

REAL sg_modClientNamePosX = 20.0f;
REAL sg_modClientNamePosY = 20.0f;
REAL sg_modFpsPosX = 140.0f;
REAL sg_modFpsPosY = 20.0f;
REAL sg_modPingPosX = 220.0f;
REAL sg_modPingPosY = 20.0f;
REAL sg_modTimePosX = 300.0f;
REAL sg_modTimePosY = 20.0f;
REAL sg_modKeybindsPosX = 20.0f;
REAL sg_modKeybindsPosY = 100.0f;
REAL sg_modKDWidgetPosX = 20.0f;
REAL sg_modKDWidgetPosY = 290.0f;
REAL sg_modSpeedometerPosX = 250.0f;
REAL sg_modSpeedometerPosY = 320.0f;
REAL sg_modRubberMeterPosX = 370.0f;
REAL sg_modRubberMeterPosY = 320.0f;
REAL sg_modBrakeMeterPosX = 490.0f;
REAL sg_modBrakeMeterPosY = 320.0f;
REAL sg_modScoreboardWidgetPosX = 500.0f;
REAL sg_modScoreboardWidgetPosY = 20.0f;
bool sg_modAliveWidgetEnabled = true;
REAL sg_modAliveWidgetPosX = 680.0f;
REAL sg_modAliveWidgetPosY = 20.0f;

bool sg_modNetHealthEnabled = true;
REAL sg_modNetHealthPosX = 425.0f;
REAL sg_modNetHealthPosY = 20.0f;

bool sg_modLiveScoreboardEnabled = true;
bool sg_modZoneTimerEnabled = true;
bool sg_modScoreboardShowTeams = true;
bool sg_modScoreboardShowPlayers = true;
int sg_modScoreboardMaxPlayers = 8;
bool sg_modScoreboardShowPing = false;

REAL sg_modLiveScoreboardPosX = 500.0f;
REAL sg_modLiveScoreboardPosY = 80.0f;
REAL sg_modZoneTimerPosX = 350.0f;
REAL sg_modZoneTimerPosY = 100.0f;

extern bool sg_modRubberBatteryEnabled;
REAL sg_modRubberBatteryPosX = 300.0f;
REAL sg_modRubberBatteryPosY = 600.0f;

extern bool sg_modClassicRubberBatteryEnabled;
REAL sg_modClassicRubberBatteryPosX = 200.0f;
REAL sg_modClassicRubberBatteryPosY = 520.0f;

// [MOD] New Gen Chat & Minimap Widget Variables


REAL sg_modMinimapPosX = 1000.0f;
REAL sg_modMinimapPosY = 500.0f;
int sg_modMinimapShape = 0; // 0: Circle, 1: Square
bool sg_modMinimapAutoZoom = true;
bool sg_modMinimapDeadEffect = true;

// --- MULTI-WIDGET CUSTOMIZATION VARIABLES ---
#define DEFINE_WIDGET_VARS(varPrefix) \
    REAL varPrefix##_Scale = 1.0f; \
    REAL varPrefix##_Opacity = 1.0f; \
    REAL varPrefix##_BgOpacity = 1.0f; \
    bool varPrefix##_UseCustomColors = false; \
    REAL varPrefix##_TextColorR = 1.0f; \
    REAL varPrefix##_TextColorG = 1.0f; \
    REAL varPrefix##_TextColorB = 1.0f; \
    REAL varPrefix##_TextColorA = 1.0f; \
    REAL varPrefix##_BgColorR = 10.0f/255.0f; \
    REAL varPrefix##_BgColorG = 10.0f/255.0f; \
    REAL varPrefix##_BgColorB = 12.0f/255.0f; \
    REAL varPrefix##_BgColorA = 195.0f/255.0f; \
    REAL varPrefix##_BorderColorR = 80.0f/255.0f; \
    REAL varPrefix##_BorderColorG = 80.0f/255.0f; \
    REAL varPrefix##_BorderColorB = 95.0f/255.0f; \
    REAL varPrefix##_BorderColorA = 60.0f/255.0f; \
    REAL varPrefix##_AccentColorR = 0.0f/255.0f; \
    REAL varPrefix##_AccentColorG = 190.0f/255.0f; \
    REAL varPrefix##_AccentColorB = 255.0f/255.0f; \
    REAL varPrefix##_AccentColorA = 1.0f; \
    bool varPrefix##_RgbMode = false; \
    REAL varPrefix##_RgbSpeed = 1.0f;

#define WIDGET_CONFIG_PASS(varPrefix) \
    &varPrefix##_Scale, &varPrefix##_Opacity, &varPrefix##_BgOpacity, &varPrefix##_UseCustomColors, \
    &varPrefix##_TextColorR, &varPrefix##_TextColorG, &varPrefix##_TextColorB, &varPrefix##_TextColorA, \
    &varPrefix##_BgColorR, &varPrefix##_BgColorG, &varPrefix##_BgColorB, &varPrefix##_BgColorA, \
    &varPrefix##_BorderColorR, &varPrefix##_BorderColorG, &varPrefix##_BorderColorB, &varPrefix##_BorderColorA, \
    &varPrefix##_AccentColorR, &varPrefix##_AccentColorG, &varPrefix##_AccentColorB, &varPrefix##_AccentColorA, \
    &varPrefix##_RgbMode, &varPrefix##_RgbSpeed

DEFINE_WIDGET_VARS(sg_hudClientName)
DEFINE_WIDGET_VARS(sg_hudFps)
DEFINE_WIDGET_VARS(sg_hudPing)
DEFINE_WIDGET_VARS(sg_hudTime)
DEFINE_WIDGET_VARS(sg_hudKeybinds)
DEFINE_WIDGET_VARS(sg_hudKD)
DEFINE_WIDGET_VARS(sg_hudSpeedometer)
DEFINE_WIDGET_VARS(sg_hudRubber)
DEFINE_WIDGET_VARS(sg_hudBrake)
DEFINE_WIDGET_VARS(sg_hudScoreboard)
DEFINE_WIDGET_VARS(sg_hudLiveScoreboard)
DEFINE_WIDGET_VARS(sg_hudZoneTimer)
DEFINE_WIDGET_VARS(sg_hudAlive)
DEFINE_WIDGET_VARS(sg_hudNetHealth)
DEFINE_WIDGET_VARS(sg_hudRubberBattery)
DEFINE_WIDGET_VARS(sg_hudClassicRubberBattery)
// The restyled console, defined further down beside its drawing.
extern bool sg_chatOurFont;
extern bool sg_chatStamp;
extern REAL sg_chatSize;
extern REAL sg_chatWide;
extern REAL sg_chatTall;
extern REAL sg_chatLeft;
extern REAL sg_chatTop;
extern REAL sg_chatShade;

DEFINE_WIDGET_VARS(sg_hudChat)
DEFINE_WIDGET_VARS(sg_hudMinimap)
DEFINE_WIDGET_VARS(sg_hudFortressAlerts)
DEFINE_WIDGET_VARS(sg_hudTeammateDeath)
DEFINE_WIDGET_VARS(sg_hudWallTimer)
DEFINE_WIDGET_VARS(sg_hudKeystroke1)
DEFINE_WIDGET_VARS(sg_hudKeystroke2)
DEFINE_WIDGET_VARS(sg_hudCenterMessage)

// --- CONFIG PERSISTENCE ---
#define REGISTER_WIDGET_CONFIGS(nameStr, varPrefix) \
    static tConfItem<REAL> varPrefix##_ScaleConf("MOD_HUD_" nameStr "_SCALE", varPrefix##_Scale); \
    static tConfItem<REAL> varPrefix##_OpacityConf("MOD_HUD_" nameStr "_OPACITY", varPrefix##_Opacity); \
    static tConfItem<REAL> varPrefix##_BgOpacityConf("MOD_HUD_" nameStr "_BGOPACITY", varPrefix##_BgOpacity); \
    static tConfItem<bool> varPrefix##_UseCustomColorsConf("MOD_HUD_" nameStr "_CUSTOMCOLORS", varPrefix##_UseCustomColors); \
    static tConfItem<REAL> varPrefix##_TextColorRConf("MOD_HUD_" nameStr "_TEXTCOLOR_R", varPrefix##_TextColorR); \
    static tConfItem<REAL> varPrefix##_TextColorGConf("MOD_HUD_" nameStr "_TEXTCOLOR_G", varPrefix##_TextColorG); \
    static tConfItem<REAL> varPrefix##_TextColorBConf("MOD_HUD_" nameStr "_TEXTCOLOR_B", varPrefix##_TextColorB); \
    static tConfItem<REAL> varPrefix##_TextColorAConf("MOD_HUD_" nameStr "_TEXTCOLOR_A", varPrefix##_TextColorA); \
    static tConfItem<REAL> varPrefix##_BgColorRConf("MOD_HUD_" nameStr "_BGCOLOR_R", varPrefix##_BgColorR); \
    static tConfItem<REAL> varPrefix##_BgColorGConf("MOD_HUD_" nameStr "_BGCOLOR_G", varPrefix##_BgColorG); \
    static tConfItem<REAL> varPrefix##_BgColorBConf("MOD_HUD_" nameStr "_BGCOLOR_B", varPrefix##_BgColorB); \
    static tConfItem<REAL> varPrefix##_BgColorAConf("MOD_HUD_" nameStr "_BGCOLOR_A", varPrefix##_BgColorA); \
    static tConfItem<REAL> varPrefix##_BorderColorRConf("MOD_HUD_" nameStr "_BORDERCOLOR_R", varPrefix##_BorderColorR); \
    static tConfItem<REAL> varPrefix##_BorderColorGConf("MOD_HUD_" nameStr "_BORDERCOLOR_G", varPrefix##_BorderColorG); \
    static tConfItem<REAL> varPrefix##_BorderColorBConf("MOD_HUD_" nameStr "_BORDERCOLOR_B", varPrefix##_BorderColorB); \
    static tConfItem<REAL> varPrefix##_BorderColorAConf("MOD_HUD_" nameStr "_BORDERCOLOR_A", varPrefix##_BorderColorA); \
    static tConfItem<REAL> varPrefix##_AccentColorRConf("MOD_HUD_" nameStr "_ACCENTCOLOR_R", varPrefix##_AccentColorR); \
    static tConfItem<REAL> varPrefix##_AccentColorGConf("MOD_HUD_" nameStr "_ACCENTCOLOR_G", varPrefix##_AccentColorG); \
    static tConfItem<REAL> varPrefix##_AccentColorBConf("MOD_HUD_" nameStr "_ACCENTCOLOR_B", varPrefix##_AccentColorB); \
    static tConfItem<REAL> varPrefix##_AccentColorAConf("MOD_HUD_" nameStr "_ACCENTCOLOR_A", varPrefix##_AccentColorA); \
    static tConfItem<bool> varPrefix##_RgbModeConf("MOD_HUD_" nameStr "_RGBMODE", varPrefix##_RgbMode); \
    static tConfItem<REAL> varPrefix##_RgbSpeedConf("MOD_HUD_" nameStr "_RGBSPEED", varPrefix##_RgbSpeed);

// Configuration items for persistent storage
static tConfItem<bool> sg_modClientNameEnabledConf("MOD_CLIENTNAME_ENABLED", sg_modClientNameEnabled);
static tConfItem<bool> sg_modFpsEnabledConf("MOD_FPS_ENABLED", sg_modFpsEnabled);
static tConfItem<bool> sg_modPingEnabledConf("MOD_PING_ENABLED", sg_modPingEnabled);
static tConfItem<bool> sg_modTimeEnabledConf("MOD_TIME_ENABLED", sg_modTimeEnabled);
static tConfItem<bool> sg_modKeybindsEnabledConf("MOD_KEYBINDS_ENABLED", sg_modKeybindsEnabled);
static tConfItem<bool> sg_modKDWidgetEnabledConf("MOD_KD_WIDGET_ENABLED", sg_modKDWidgetEnabled);
static tConfItem<bool> sg_modSpeedometerEnabledConf("MOD_SPEEDOMETER_ENABLED", sg_modSpeedometerEnabled);
static tConfItem<bool> sg_modRubberMeterEnabledConf("MOD_RUBBER_METER_ENABLED", sg_modRubberMeterEnabled);
static tConfItem<bool> sg_modBrakeMeterEnabledConf("MOD_BRAKE_METER_ENABLED", sg_modBrakeMeterEnabled);
static tConfItem<bool> sg_modScoreboardWidgetEnabledConf("MOD_SCOREBOARD_WIDGET_ENABLED", sg_modScoreboardWidgetEnabled);
static tConfItem<bool> sg_modAliveWidgetEnabledConf("MOD_ALIVE_WIDGET_ENABLED", sg_modAliveWidgetEnabled);
static tConfItem<bool> sg_modNetHealthEnabledConf("MOD_NETHEALTH_ENABLED", sg_modNetHealthEnabled);

static tConfItem<REAL> sg_modClientNamePosXConf("MOD_CLIENTNAME_POS_X", sg_modClientNamePosX);
static tConfItem<REAL> sg_modClientNamePosYConf("MOD_CLIENTNAME_POS_Y", sg_modClientNamePosY);
static tConfItem<REAL> sg_modFpsPosXConf("MOD_FPS_POS_X", sg_modFpsPosX);
static tConfItem<REAL> sg_modFpsPosYConf("MOD_FPS_POS_Y", sg_modFpsPosY);
static tConfItem<REAL> sg_modPingPosXConf("MOD_PING_POS_X", sg_modPingPosX);
static tConfItem<REAL> sg_modPingPosYConf("MOD_PING_POS_Y", sg_modPingPosY);
static tConfItem<REAL> sg_modTimePosXConf("MOD_TIME_POS_X", sg_modTimePosX);
static tConfItem<REAL> sg_modTimePosYConf("MOD_TIME_POS_Y", sg_modTimePosY);
static tConfItem<REAL> sg_modKeybindsPosXConf("MOD_KEYBINDS_POS_X", sg_modKeybindsPosX);
static tConfItem<REAL> sg_modKeybindsPosYConf("MOD_KEYBINDS_POS_Y", sg_modKeybindsPosY);
static tConfItem<REAL> sg_modKDWidgetPosXConf("MOD_KD_WIDGET_POS_X", sg_modKDWidgetPosX);
static tConfItem<REAL> sg_modKDWidgetPosYConf("MOD_KD_WIDGET_POS_Y", sg_modKDWidgetPosY);
static tConfItem<REAL> sg_modSpeedometerPosXConf("MOD_SPEEDOMETER_POS_X", sg_modSpeedometerPosX);
static tConfItem<REAL> sg_modSpeedometerPosYConf("MOD_SPEEDOMETER_POS_Y", sg_modSpeedometerPosY);
static tConfItem<REAL> sg_modRubberMeterPosXConf("MOD_RUBBER_METER_POS_X", sg_modRubberMeterPosX);
static tConfItem<REAL> sg_modRubberMeterPosYConf("MOD_RUBBER_METER_POS_Y", sg_modRubberMeterPosY);
static tConfItem<REAL> sg_modBrakeMeterPosXConf("MOD_BRAKE_METER_POS_X", sg_modBrakeMeterPosX);
static tConfItem<REAL> sg_modBrakeMeterPosYConf("MOD_BRAKE_METER_POS_Y", sg_modBrakeMeterPosY);
static tConfItem<REAL> sg_modScoreboardWidgetPosXConf("MOD_SCOREBOARD_WIDGET_POS_X", sg_modScoreboardWidgetPosX);
static tConfItem<REAL> sg_modScoreboardWidgetPosYConf("MOD_SCOREBOARD_WIDGET_POS_Y", sg_modScoreboardWidgetPosY);
static tConfItem<REAL> sg_modAliveWidgetPosXConf("MOD_ALIVE_WIDGET_POS_X", sg_modAliveWidgetPosX);
static tConfItem<REAL> sg_modAliveWidgetPosYConf("MOD_ALIVE_WIDGET_POS_Y", sg_modAliveWidgetPosY);
static tConfItem<REAL> sg_modNetHealthPosXConf("MOD_NETHEALTH_POS_X", sg_modNetHealthPosX);
static tConfItem<REAL> sg_modNetHealthPosYConf("MOD_NETHEALTH_POS_Y", sg_modNetHealthPosY);

static tConfItem<bool> sg_modLiveScoreboardEnabledConf("MOD_LIVESCOREBOARD_ENABLED", sg_modLiveScoreboardEnabled);
static tConfItem<REAL> sg_modLiveScoreboardPosXConf("MOD_LIVESCOREBOARD_POS_X", sg_modLiveScoreboardPosX);
static tConfItem<REAL> sg_modLiveScoreboardPosYConf("MOD_LIVESCOREBOARD_POS_Y", sg_modLiveScoreboardPosY);

static tConfItem<bool> sg_modZoneTimerEnabledConf("MOD_ZONETIMER_ENABLED", sg_modZoneTimerEnabled);
static tConfItem<REAL> sg_modZoneTimerPosXConf("MOD_ZONETIMER_POS_X", sg_modZoneTimerPosX);
static tConfItem<REAL> sg_modZoneTimerPosYConf("MOD_ZONETIMER_POS_Y", sg_modZoneTimerPosY);

static tConfItem<bool> sg_modScoreboardShowTeamsConf("MOD_SCOREBOARD_SHOW_TEAMS", sg_modScoreboardShowTeams);
static tConfItem<bool> sg_modScoreboardShowPlayersConf("MOD_SCOREBOARD_SHOW_PLAYERS", sg_modScoreboardShowPlayers);
static tConfItem<int> sg_modScoreboardMaxPlayersConf("MOD_SCOREBOARD_MAX_PLAYERS", sg_modScoreboardMaxPlayers);
static tConfItem<bool> sg_modScoreboardShowPingConf("MOD_SCOREBOARD_SHOW_PING", sg_modScoreboardShowPing);

static tConfItem<REAL> sg_modRubberBatteryPosXConf("MOD_RUBBER_BATTERY_POS_X", sg_modRubberBatteryPosX);
static tConfItem<REAL> sg_modRubberBatteryPosYConf("MOD_RUBBER_BATTERY_POS_Y", sg_modRubberBatteryPosY);

static tConfItem<REAL> sg_modClassicRubberBatteryPosXConf("MOD_CLASSIC_RUBBER_BATTERY_POS_X", sg_modClassicRubberBatteryPosX);
static tConfItem<REAL> sg_modClassicRubberBatteryPosYConf("MOD_CLASSIC_RUBBER_BATTERY_POS_Y", sg_modClassicRubberBatteryPosY);

// [MOD] New Gen Chat & Minimap Configuration

static tConfItem<REAL> sg_modMinimapPosXConf("MOD_MINIMAP_POS_X", sg_modMinimapPosX);
static tConfItem<REAL> sg_modMinimapPosYConf("MOD_MINIMAP_POS_Y", sg_modMinimapPosY);
static tConfItem<int> sg_modMinimapShapeConf("MOD_MINIMAP_SHAPE", sg_modMinimapShape);
static tConfItem<bool> sg_modMinimapAutoZoomConf("MOD_MINIMAP_AUTOZOOM", sg_modMinimapAutoZoom);
static tConfItem<bool> sg_modMinimapDeadEffectConf("MOD_MINIMAP_DEADEFFECT", sg_modMinimapDeadEffect);

REGISTER_WIDGET_CONFIGS("CLIENTNAME", sg_hudClientName)
REGISTER_WIDGET_CONFIGS("FPS", sg_hudFps)
REGISTER_WIDGET_CONFIGS("PING", sg_hudPing)
REGISTER_WIDGET_CONFIGS("TIME", sg_hudTime)
REGISTER_WIDGET_CONFIGS("KEYBINDS", sg_hudKeybinds)
REGISTER_WIDGET_CONFIGS("KD", sg_hudKD)
REGISTER_WIDGET_CONFIGS("SPEEDOMETER", sg_hudSpeedometer)
REGISTER_WIDGET_CONFIGS("RUBBER", sg_hudRubber)
REGISTER_WIDGET_CONFIGS("BRAKE", sg_hudBrake)
REGISTER_WIDGET_CONFIGS("SCOREBOARD", sg_hudScoreboard)
REGISTER_WIDGET_CONFIGS("LIVESCOREBOARD", sg_hudLiveScoreboard)
REGISTER_WIDGET_CONFIGS("ZONETIMER", sg_hudZoneTimer)
REGISTER_WIDGET_CONFIGS("ALIVE", sg_hudAlive)
REGISTER_WIDGET_CONFIGS("NETHEALTH", sg_hudNetHealth)
REGISTER_WIDGET_CONFIGS("RUBBERBATTERY", sg_hudRubberBattery)
REGISTER_WIDGET_CONFIGS("CLASSICRUBBERBATTERY", sg_hudClassicRubberBattery)
REGISTER_WIDGET_CONFIGS("CHAT", sg_hudChat)
REGISTER_WIDGET_CONFIGS("MINIMAP", sg_hudMinimap)
REGISTER_WIDGET_CONFIGS("FORTRESSALERTS", sg_hudFortressAlerts)
REGISTER_WIDGET_CONFIGS("TEAMMATEDEATH", sg_hudTeammateDeath)
REGISTER_WIDGET_CONFIGS("WALLTIMER", sg_hudWallTimer)
REGISTER_WIDGET_CONFIGS("KEYSTROKE1", sg_hudKeystroke1)
REGISTER_WIDGET_CONFIGS("KEYSTROKE2", sg_hudKeystroke2)
REGISTER_WIDGET_CONFIGS("CENTERMESSAGE", sg_hudCenterMessage)

REAL sg_modFortressAlertsPosX = 20.0f;
REAL sg_modFortressAlertsPosY = 140.0f;
REAL sg_modTeammateDeathPosX = 20.0f;
REAL sg_modTeammateDeathPosY = 350.0f;

static tConfItem<REAL> sg_modFortressAlertsPosXConf("MOD_FORTRESS_ALERTS_POS_X", sg_modFortressAlertsPosX);
static tConfItem<REAL> sg_modFortressAlertsPosYConf("MOD_FORTRESS_ALERTS_POS_Y", sg_modFortressAlertsPosY);
static tConfItem<REAL> sg_modTeammateDeathPosXConf("MOD_TEAMMATE_DEATH_POS_X", sg_modTeammateDeathPosX);
static tConfItem<REAL> sg_modTeammateDeathPosYConf("MOD_TEAMMATE_DEATH_POS_Y", sg_modTeammateDeathPosY);

bool sg_modWallTimerEnabled = true;
REAL sg_modWallTimerPosX = 800.0f;
REAL sg_modWallTimerPosY = 200.0f;

static tConfItem<bool> sg_modWallTimerEnabledConf("MOD_WALLTIMER_ENABLED", sg_modWallTimerEnabled);
static tConfItem<REAL> sg_modWallTimerPosXConf("MOD_WALLTIMER_POS_X", sg_modWallTimerPosX);
static tConfItem<REAL> sg_modWallTimerPosYConf("MOD_WALLTIMER_POS_Y", sg_modWallTimerPosY);

bool sg_modCenterMessageEnabled = true;
REAL sg_modCenterMessagePosX = 512.0f;
REAL sg_modCenterMessagePosY = 384.0f;

static tConfItem<bool> sg_modCenterMessageEnabledConf("MOD_CENTERMESSAGE_ENABLED", sg_modCenterMessageEnabled);
static tConfItem<REAL> sg_modCenterMessagePosXConf("MOD_CENTERMESSAGE_POS_X", sg_modCenterMessagePosX);
static tConfItem<REAL> sg_modCenterMessagePosYConf("MOD_CENTERMESSAGE_POS_Y", sg_modCenterMessagePosY);

bool sg_modKeystroke1_Enabled = false;
REAL sg_modKeystroke1_PosX = 500.0f;
REAL sg_modKeystroke1_PosY = 600.0f;
int sg_modKeystroke1_Preset = 3;
int sg_modKeystroke1_Mask0 = 536870912;
int sg_modKeystroke1_Mask1 = 7168;
int sg_modKeystroke1_Mask2 = 32;

bool sg_modKeystroke1_RgbWave = true;
REAL sg_modKeystroke1_RgbSpeed = 1.5f;
REAL sg_modKeystroke1_GlowIntensity = 0.8f;

bool sg_modKeystroke1_SeparateKeys = false;
REAL sg_modKeystroke1_Spacing = 6.0f;
REAL sg_modKeystroke1_Radius = 6.0f;
bool sg_modKeystroke1_ShowCps = true;

bool sg_modKeystroke2_Enabled = false;
REAL sg_modKeystroke2_PosX = 550.0f;
REAL sg_modKeystroke2_PosY = 600.0f;
int sg_modKeystroke2_Preset = 3;
int sg_modKeystroke2_Mask0 = 0;
int sg_modKeystroke2_Mask1 = 0;
int sg_modKeystroke2_Mask2 = 25165824;

bool sg_modKeystroke2_RgbWave = true;
REAL sg_modKeystroke2_RgbSpeed = 1.5f;
REAL sg_modKeystroke2_GlowIntensity = 0.8f;

bool sg_modKeystroke2_SeparateKeys = false;
REAL sg_modKeystroke2_Spacing = 6.0f;
REAL sg_modKeystroke2_Radius = 6.0f;
bool sg_modKeystroke2_ShowCps = true;

static tConfItem<bool> sg_modKeystroke1_EnabledConf("MOD_KEYSTROKE1_ENABLED", sg_modKeystroke1_Enabled);
static tConfItem<REAL> sg_modKeystroke1_PosXConf("MOD_KEYSTROKE1_POS_X", sg_modKeystroke1_PosX);
static tConfItem<REAL> sg_modKeystroke1_PosYConf("MOD_KEYSTROKE1_POS_Y", sg_modKeystroke1_PosY);
static tConfItem<int> sg_modKeystroke1_PresetConf("MOD_KEYSTROKE1_PRESET", sg_modKeystroke1_Preset);
static tConfItem<int> sg_modKeystroke1_Mask0Conf("MOD_KEYSTROKE1_MASK0", sg_modKeystroke1_Mask0);
static tConfItem<int> sg_modKeystroke1_Mask1Conf("MOD_KEYSTROKE1_MASK1", sg_modKeystroke1_Mask1);
static tConfItem<int> sg_modKeystroke1_Mask2Conf("MOD_KEYSTROKE1_MASK2", sg_modKeystroke1_Mask2);

static tConfItem<bool> sg_modKeystroke1_RgbWaveConf("MOD_KEYSTROKE1_RGBWAVE", sg_modKeystroke1_RgbWave);
static tConfItem<REAL> sg_modKeystroke1_RgbSpeedConf("MOD_KEYSTROKE1_RGBSPEED", sg_modKeystroke1_RgbSpeed);
static tConfItem<REAL> sg_modKeystroke1_GlowIntensityConf("MOD_KEYSTROKE1_GLOWINTENSITY", sg_modKeystroke1_GlowIntensity);

static tConfItem<bool> sg_modKeystroke1_SeparateKeysConf("MOD_KEYSTROKE1_SEPARATEKEYS", sg_modKeystroke1_SeparateKeys);
static tConfItem<REAL> sg_modKeystroke1_SpacingConf("MOD_KEYSTROKE1_SPACING", sg_modKeystroke1_Spacing);
static tConfItem<REAL> sg_modKeystroke1_RadiusConf("MOD_KEYSTROKE1_RADIUS", sg_modKeystroke1_Radius);
static tConfItem<bool> sg_modKeystroke1_ShowCpsConf("MOD_KEYSTROKE1_SHOWCPS", sg_modKeystroke1_ShowCps);

static tConfItem<bool> sg_modKeystroke2_EnabledConf("MOD_KEYSTROKE2_ENABLED", sg_modKeystroke2_Enabled);
static tConfItem<REAL> sg_modKeystroke2_PosXConf("MOD_KEYSTROKE2_POS_X", sg_modKeystroke2_PosX);
static tConfItem<REAL> sg_modKeystroke2_PosYConf("MOD_KEYSTROKE2_POS_Y", sg_modKeystroke2_PosY);
static tConfItem<int> sg_modKeystroke2_PresetConf("MOD_KEYSTROKE2_PRESET", sg_modKeystroke2_Preset);
static tConfItem<int> sg_modKeystroke2_Mask0Conf("MOD_KEYSTROKE2_MASK0", sg_modKeystroke2_Mask0);
static tConfItem<int> sg_modKeystroke2_Mask1Conf("MOD_KEYSTROKE2_MASK1", sg_modKeystroke2_Mask1);
static tConfItem<int> sg_modKeystroke2_Mask2Conf("MOD_KEYSTROKE2_MASK2", sg_modKeystroke2_Mask2);

static tConfItem<bool> sg_modKeystroke2_RgbWaveConf("MOD_KEYSTROKE2_RGBWAVE", sg_modKeystroke2_RgbWave);
static tConfItem<REAL> sg_modKeystroke2_RgbSpeedConf("MOD_KEYSTROKE2_RGBSPEED", sg_modKeystroke2_RgbSpeed);
static tConfItem<REAL> sg_modKeystroke2_GlowIntensityConf("MOD_KEYSTROKE2_GLOWINTENSITY", sg_modKeystroke2_GlowIntensity);

static tConfItem<bool> sg_modKeystroke2_SeparateKeysConf("MOD_KEYSTROKE2_SEPARATEKEYS", sg_modKeystroke2_SeparateKeys);
static tConfItem<REAL> sg_modKeystroke2_SpacingConf("MOD_KEYSTROKE2_SPACING", sg_modKeystroke2_Spacing);
static tConfItem<REAL> sg_modKeystroke2_RadiusConf("MOD_KEYSTROKE2_RADIUS", sg_modKeystroke2_Radius);
static tConfItem<bool> sg_modKeystroke2_ShowCpsConf("MOD_KEYSTROKE2_SHOWCPS", sg_modKeystroke2_ShowCps);

// Extern modules from ModMenu
extern bool g_NoclipMode;
extern bool g_CleanScreen;
extern bool g_CustomHitbox;
extern bool g_CustomFog;
extern bool g_ShowHUD;
extern bool g_RubberGauge;
extern bool g_SpeedMeter;
extern bool g_BrakeMeter;
extern bool g_ShowScores;
extern bool g_ShowPing;
extern bool g_AliveCounter;
extern bool g_ShowFastest;
extern bool g_ShowTime;
extern bool g_AutoPacketRefresh;

extern int g_NoclipKeybind;
extern int g_CleanScreenKeybind;
extern int g_CustomHitboxKeybind;
extern int g_CustomFogKeybind;
extern int g_ShowHUDKeybind;
extern int g_RubberGaugeKeybind;
extern int g_SpeedMeterKeybind;
extern int g_BrakeMeterKeybind;
extern int g_ShowScoresKeybind;
extern int g_ShowPingKeybind;
extern int g_AliveCounterKeybind;
extern int g_ShowFastestKeybind;
extern int g_ShowTimeKeybind;
extern int g_PacketRefreshKeybind;

// Helper to get key names via SDL
static const char* GetSDLKeyName(int key) {
    if (key == 0) return "NONE";
    const char* name = SDL_GetKeyName((SDL_Keycode)key);
    if (name) return name;
    return "UNK";
}

// ---------------------------------------------------------------------
// HudWidget Base Implementation
// ---------------------------------------------------------------------
HudWidget::HudWidget(const std::string& name, const ImVec2& defaultPos, const ImVec2& defaultSize,
                     REAL* pScale, REAL* pOpacity, REAL* pBgOpacity, bool* pUseCustomColors,
                     REAL* pTextColorR, REAL* pTextColorG, REAL* pTextColorB, REAL* pTextColorA,
                     REAL* pBgColorR, REAL* pBgColorG, REAL* pBgColorB, REAL* pBgColorA,
                     REAL* pBorderColorR, REAL* pBorderColorG, REAL* pBorderColorB, REAL* pBorderColorA,
                     REAL* pAccentColorR, REAL* pAccentColorG, REAL* pAccentColorB, REAL* pAccentColorA,
                     bool* pRgbMode, REAL* pRgbSpeed)
    : m_Name(name), m_Size(defaultSize), m_Alpha(0.0f), m_IsVisible(true),
      m_TargetAlpha(1.0f), m_SlideOffset(0.0f, 0.0f), m_TargetSlideOffset(0.0f, 0.0f),
      m_IsDragging(false), m_DragMouseOffset(0.0f, 0.0f),
      m_pScale(pScale), m_pOpacity(pOpacity), m_pBgOpacity(pBgOpacity), m_pUseCustomColors(pUseCustomColors),
      m_pTextColorR(pTextColorR), m_pTextColorG(pTextColorG), m_pTextColorB(pTextColorB), m_pTextColorA(pTextColorA),
      m_pBgColorR(pBgColorR), m_pBgColorG(pBgColorG), m_pBgColorB(pBgColorB), m_pBgColorA(pBgColorA),
      m_pBorderColorR(pBorderColorR), m_pBorderColorG(pBorderColorG), m_pBorderColorB(pBorderColorB), m_pBorderColorA(pBorderColorA),
      m_pAccentColorR(pAccentColorR), m_pAccentColorG(pAccentColorG), m_pAccentColorB(pAccentColorB), m_pAccentColorA(pAccentColorA),
      m_pRgbMode(pRgbMode), m_pRgbSpeed(pRgbSpeed) {
}

float HudWidget::GetScale() const { return m_pScale ? *m_pScale : 1.0f; }
float HudWidget::GetOpacitySetting() const { return m_pOpacity ? *m_pOpacity : 1.0f; }
float HudWidget::GetBgOpacitySetting() const { return m_pBgOpacity ? *m_pBgOpacity : 1.0f; }
bool HudWidget::UseCustomColors() const { return m_pUseCustomColors ? *m_pUseCustomColors : false; }
bool HudWidget::GetRgbMode() const { return m_pRgbMode ? *m_pRgbMode : false; }
float HudWidget::GetRgbSpeed() const { return m_pRgbSpeed ? *m_pRgbSpeed : 1.0f; }

static ImVec4 GetRgbColor(float speed, float offset = 0.0f) {
    float hue = fmodf((float)ImGui::GetTime() * 0.2f * speed + offset, 1.0f);
    ImVec4 col;
    ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.95f, col.x, col.y, col.z);
    col.w = 1.0f;
    return col;
}

ImU32 HudWidget::GetTextCol(ImU32 defaultColor) const {
    if (UseCustomColors() || GetRgbMode()) {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        if (GetRgbMode()) {
            ImVec4 rgb = GetRgbColor(GetRgbSpeed());
            r = rgb.x; g = rgb.y; b = rgb.z; a = rgb.w;
        } else if (m_pTextColorR) {
            r = *m_pTextColorR;
            g = *m_pTextColorG;
            b = *m_pTextColorB;
            a = *m_pTextColorA;
        }
        float finalAlpha = a * m_Alpha * GetOpacitySetting();
        return IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(finalAlpha * 255.0f));
    }
    unsigned int a = (defaultColor >> 24) & 0xFF;
    unsigned int b = (defaultColor >> 16) & 0xFF;
    unsigned int g = (defaultColor >> 8) & 0xFF;
    unsigned int r = defaultColor & 0xFF;
    a = (unsigned int)(a * m_Alpha * GetOpacitySetting());
    return IM_COL32(r, g, b, a);
}

ImU32 HudWidget::GetBgCol() const {
    float r = 10.0f/255.0f, g = 10.0f/255.0f, b = 12.0f/255.0f, a = 195.0f/255.0f;
    if (UseCustomColors() && m_pBgColorR) {
        r = *m_pBgColorR;
        g = *m_pBgColorG;
        b = *m_pBgColorB;
        a = *m_pBgColorA;
    }
    float finalAlpha = a * m_Alpha * GetBgOpacitySetting();
    return IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(finalAlpha * 255.0f));
}

ImU32 HudWidget::GetBorderCol() const {
    float r = 80.0f/255.0f, g = 80.0f/255.0f, b = 95.0f/255.0f, a = 60.0f/255.0f;
    if (UseCustomColors() && m_pBorderColorR) {
        r = *m_pBorderColorR;
        g = *m_pBorderColorG;
        b = *m_pBorderColorB;
        a = *m_pBorderColorA;
    }
    float finalAlpha = a * m_Alpha * GetBgOpacitySetting();
    return IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(finalAlpha * 255.0f));
}

ImU32 HudWidget::GetAccentCol(ImU32 defaultColor) const {
    if (UseCustomColors() || GetRgbMode()) {
        float r = 0.0f, g = 190.0f/255.0f, b = 1.0f, a = 1.0f;
        if (GetRgbMode()) {
            ImVec4 rgb = GetRgbColor(GetRgbSpeed(), 0.25f);
            r = rgb.x; g = rgb.y; b = rgb.z; a = rgb.w;
        } else if (m_pAccentColorR) {
            r = *m_pAccentColorR;
            g = *m_pAccentColorG;
            b = *m_pAccentColorB;
            a = *m_pAccentColorA;
        }
        float finalAlpha = a * m_Alpha * GetOpacitySetting();
        return IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(finalAlpha * 255.0f));
    }
    unsigned int a = (defaultColor >> 24) & 0xFF;
    unsigned int b = (defaultColor >> 16) & 0xFF;
    unsigned int g = (defaultColor >> 8) & 0xFF;
    unsigned int r = defaultColor & 0xFF;
    a = (unsigned int)(a * m_Alpha * GetOpacitySetting());
    return IM_COL32(r, g, b, a);
}

ImVec2 HudWidget::CalcTextSize(const char* text) const {
    float scale = GetScale();
    ImFont* font = ImGui::GetFont();
    return font->CalcTextSizeA(ImGui::GetFontSize() * scale, FLT_MAX, -1.0f, text);
}

ImVec2 HudWidget::GetPosition() const {
    ImVec2 pos(20.0f, 20.0f);
    if (m_Name == "ClientName") {
        pos = ImVec2(sg_modClientNamePosX, sg_modClientNamePosY);
    } else if (m_Name == "FPS") {
        pos = ImVec2(sg_modFpsPosX, sg_modFpsPosY);
    } else if (m_Name == "Ping") {
        pos = ImVec2(sg_modPingPosX, sg_modPingPosY);
    } else if (m_Name == "Time") {
        pos = ImVec2(sg_modTimePosX, sg_modTimePosY);
    } else if (m_Name == "Keybinds") {
        pos = ImVec2(sg_modKeybindsPosX, sg_modKeybindsPosY);
    } else if (m_Name == "KD") {
        pos = ImVec2(sg_modKDWidgetPosX, sg_modKDWidgetPosY);
    } else if (m_Name == "Speedometer") {
        pos = ImVec2(sg_modSpeedometerPosX, sg_modSpeedometerPosY);
    } else if (m_Name == "Rubber") {
        pos = ImVec2(sg_modRubberMeterPosX, sg_modRubberMeterPosY);
    } else if (m_Name == "Brakes") {
        pos = ImVec2(sg_modBrakeMeterPosX, sg_modBrakeMeterPosY);
    } else if (m_Name == "Scoreboard") {
        pos = ImVec2(sg_modScoreboardWidgetPosX, sg_modScoreboardWidgetPosY);
    } else if (m_Name == "Alive") {
        pos = ImVec2(sg_modAliveWidgetPosX, sg_modAliveWidgetPosY);
    } else if (m_Name == "NetworkHealth") {
        pos = ImVec2(sg_modNetHealthPosX, sg_modNetHealthPosY);
    } else if (m_Name == "LiveScoreboard") {
        pos = ImVec2(sg_modLiveScoreboardPosX, sg_modLiveScoreboardPosY);
    } else if (m_Name == "ZoneTimer") {
        pos = ImVec2(sg_modZoneTimerPosX, sg_modZoneTimerPosY);
    } else if (m_Name == "RubberBattery") {
        pos = ImVec2(sg_modRubberBatteryPosX, sg_modRubberBatteryPosY);
    } else if (m_Name == "ClassicRubberBattery") {
        pos = ImVec2(sg_modClassicRubberBatteryPosX, sg_modClassicRubberBatteryPosY);
    } else if (m_Name == "Chat") {
        pos = ImVec2(sg_chatLeft, sg_chatTop);
    } else if (m_Name == "Minimap") {
        pos = ImVec2(sg_modMinimapPosX, sg_modMinimapPosY);
    } else if (m_Name == "FortressAlerts") {
        pos = ImVec2(sg_modFortressAlertsPosX, sg_modFortressAlertsPosY);
        } else if (m_Name == "TeammateDeath") {
        pos = ImVec2(sg_modTeammateDeathPosX, sg_modTeammateDeathPosY);
    } else if (m_Name == "WallTimer") {
        pos = ImVec2(sg_modWallTimerPosX, sg_modWallTimerPosY);
    } else if (m_Name == "CenterMessage") {
        pos = ImVec2(sg_modCenterMessagePosX, sg_modCenterMessagePosY);
    } else if (m_Name == "Keystroke 1") {
        pos = ImVec2(sg_modKeystroke1_PosX, sg_modKeystroke1_PosY);
    } else if (m_Name == "Keystroke 2") {
        pos = ImVec2(sg_modKeystroke2_PosX, sg_modKeystroke2_PosY);
    }

    // Clamp to screen boundaries safely
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x > 50.0f && displaySize.y > 50.0f) {
        pos.x = std::max(0.0f, std::min(pos.x, displaySize.x - m_Size.x));
        pos.y = std::max(0.0f, std::min(pos.y, displaySize.y - m_Size.y));
    }
    return pos;
}

void HudWidget::SetPosition(const ImVec2& rawPos) {
    ImVec2 pos = rawPos;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x > 50.0f && displaySize.y > 50.0f) {
        pos.x = std::max(0.0f, std::min(pos.x, displaySize.x - m_Size.x));
        pos.y = std::max(0.0f, std::min(pos.y, displaySize.y - m_Size.y));
    }
    if (m_Name == "ClientName") {
        sg_modClientNamePosX = pos.x;
        sg_modClientNamePosY = pos.y;
    } else if (m_Name == "FPS") {
        sg_modFpsPosX = pos.x;
        sg_modFpsPosY = pos.y;
    } else if (m_Name == "Ping") {
        sg_modPingPosX = pos.x;
        sg_modPingPosY = pos.y;
    } else if (m_Name == "Time") {
        sg_modTimePosX = pos.x;
        sg_modTimePosY = pos.y;
    } else if (m_Name == "Keybinds") {
        sg_modKeybindsPosX = pos.x;
        sg_modKeybindsPosY = pos.y;
    } else if (m_Name == "KD") {
        sg_modKDWidgetPosX = pos.x;
        sg_modKDWidgetPosY = pos.y;
    } else if (m_Name == "Speedometer") {
        sg_modSpeedometerPosX = pos.x;
        sg_modSpeedometerPosY = pos.y;
    } else if (m_Name == "Rubber") {
        sg_modRubberMeterPosX = pos.x;
        sg_modRubberMeterPosY = pos.y;
    } else if (m_Name == "Brakes") {
        sg_modBrakeMeterPosX = pos.x;
        sg_modBrakeMeterPosY = pos.y;
    } else if (m_Name == "Scoreboard") {
        sg_modScoreboardWidgetPosX = pos.x;
        sg_modScoreboardWidgetPosY = pos.y;
    } else if (m_Name == "Alive") {
        sg_modAliveWidgetPosX = pos.x;
        sg_modAliveWidgetPosY = pos.y;
    } else if (m_Name == "NetworkHealth") {
        sg_modNetHealthPosX = pos.x;
        sg_modNetHealthPosY = pos.y;
    } else if (m_Name == "LiveScoreboard") {
        sg_modLiveScoreboardPosX = pos.x;
        sg_modLiveScoreboardPosY = pos.y;
    } else if (m_Name == "ZoneTimer") {
        sg_modZoneTimerPosX = pos.x;
        sg_modZoneTimerPosY = pos.y;
    } else if (m_Name == "RubberBattery") {
        sg_modRubberBatteryPosX = pos.x;
        sg_modRubberBatteryPosY = pos.y;
    } else if (m_Name == "ClassicRubberBattery") {
        sg_modClassicRubberBatteryPosX = pos.x;
        sg_modClassicRubberBatteryPosY = pos.y;
    } else if (m_Name == "Chat") {
        sg_chatLeft = pos.x;
        sg_chatTop = pos.y;
    } else if (m_Name == "Minimap") {
        sg_modMinimapPosX = pos.x;
        sg_modMinimapPosY = pos.y;
    } else if (m_Name == "FortressAlerts") {
        sg_modFortressAlertsPosX = pos.x;
        sg_modFortressAlertsPosY = pos.y;
        } else if (m_Name == "TeammateDeath") {
        sg_modTeammateDeathPosX = pos.x;
        sg_modTeammateDeathPosY = pos.y;
    } else if (m_Name == "WallTimer") {
        sg_modWallTimerPosX = pos.x;
        sg_modWallTimerPosY = pos.y;
    } else if (m_Name == "CenterMessage") {
        sg_modCenterMessagePosX = pos.x;
        sg_modCenterMessagePosY = pos.y;
    } else if (m_Name == "Keystroke 1") {
        sg_modKeystroke1_PosX = pos.x;
        sg_modKeystroke1_PosY = pos.y;
    } else if (m_Name == "Keystroke 2") {
        sg_modKeystroke2_PosX = pos.x;
        sg_modKeystroke2_PosY = pos.y;
    }
}

bool HudWidget::IsVisible() const {
    if (m_Name == "ClientName") {
        return sg_modClientNameEnabled;
    } else if (m_Name == "FPS") {
        return sg_modFpsEnabled;
    } else if (m_Name == "Ping") {
        return sg_modPingEnabled;
    } else if (m_Name == "Time") {
        return sg_modTimeEnabled;
    } else if (m_Name == "Keybinds") {
        return sg_modKeybindsEnabled;
    } else if (m_Name == "KD") {
        return sg_modKDWidgetEnabled;
    } else if (m_Name == "Speedometer") {
        return sg_modSpeedometerEnabled;
    } else if (m_Name == "Rubber") {
        return sg_modRubberMeterEnabled;
    } else if (m_Name == "Brakes") {
        return sg_modBrakeMeterEnabled;
    } else if (m_Name == "Scoreboard") {
        return sg_modScoreboardWidgetEnabled;
    } else if (m_Name == "Alive") {
        return sg_modAliveWidgetEnabled;
    } else if (m_Name == "NetworkHealth") {
        return sg_modNetHealthEnabled;
    } else if (m_Name == "LiveScoreboard") {
        return sg_modLiveScoreboardEnabled;
    } else if (m_Name == "ZoneTimer") {
        return sg_modZoneTimerEnabled;
    } else if (m_Name == "RubberBattery") {
        return sg_modRubberBatteryEnabled;
    } else if (m_Name == "ClassicRubberBattery") {
        return sg_modClassicRubberBatteryEnabled;
    } else if (m_Name == "Chat") {
        return sg_chatSkin;
    } else if (m_Name == "Minimap") {
        return sg_modMinimapEnabled;
    } else if (m_Name == "FortressAlerts") {
        extern bool sg_modFortressAlerts;
        return sg_modFortressAlerts;
    } else if (m_Name == "TeammateDeath") {
        extern bool sg_modTeammateDeathWarning;
        return sg_modTeammateDeathWarning;
    } else if (m_Name == "WallTimer") {
        return sg_modWallTimerEnabled;
    } else if (m_Name == "CenterMessage") {
        return sg_modCenterMessageEnabled;
    } else if (m_Name == "Keystroke 1") {
        return sg_modKeystroke1_Enabled;
    } else if (m_Name == "Keystroke 2") {
        return sg_modKeystroke2_Enabled;
    }
    return m_IsVisible;
}

void HudWidget::SetVisible(bool visible) {
    if (m_Name == "ClientName") {
        sg_modClientNameEnabled = visible;
    } else if (m_Name == "FPS") {
        sg_modFpsEnabled = visible;
    } else if (m_Name == "Ping") {
        sg_modPingEnabled = visible;
    } else if (m_Name == "Time") {
        sg_modTimeEnabled = visible;
    } else if (m_Name == "Keybinds") {
        sg_modKeybindsEnabled = visible;
    } else if (m_Name == "KD") {
        sg_modKDWidgetEnabled = visible;
    } else if (m_Name == "Speedometer") {
        sg_modSpeedometerEnabled = visible;
    } else if (m_Name == "Rubber") {
        sg_modRubberMeterEnabled = visible;
    } else if (m_Name == "Brakes") {
        sg_modBrakeMeterEnabled = visible;
    } else if (m_Name == "Scoreboard") {
        sg_modScoreboardWidgetEnabled = visible;
    } else if (m_Name == "Alive") {
        sg_modAliveWidgetEnabled = visible;
    } else if (m_Name == "NetworkHealth") {
        sg_modNetHealthEnabled = visible;
    } else if (m_Name == "LiveScoreboard") {
        sg_modLiveScoreboardEnabled = visible;
    } else if (m_Name == "ZoneTimer") {
        sg_modZoneTimerEnabled = visible;
    } else if (m_Name == "RubberBattery") {
        sg_modRubberBatteryEnabled = visible;
    } else if (m_Name == "ClassicRubberBattery") {
        sg_modClassicRubberBatteryEnabled = visible;
    } else if (m_Name == "Chat") {
        sg_chatSkin = visible;
    } else if (m_Name == "Minimap") {
        sg_modMinimapEnabled = visible;
    } else if (m_Name == "FortressAlerts") {
        extern bool sg_modFortressAlerts;
        sg_modFortressAlerts = visible;
    } else if (m_Name == "TeammateDeath") {
        extern bool sg_modTeammateDeathWarning;
        sg_modTeammateDeathWarning = visible;
    } else if (m_Name == "WallTimer") {
        sg_modWallTimerEnabled = visible;
    } else if (m_Name == "CenterMessage") {
        sg_modCenterMessageEnabled = visible;
    } else if (m_Name == "Keystroke 1") {
        sg_modKeystroke1_Enabled = visible;
    } else if (m_Name == "Keystroke 2") {
        sg_modKeystroke2_Enabled = visible;
    }
    m_IsVisible = visible;
}

void HudWidget::Update(float dt) {
    bool visible = IsVisible();
    m_TargetAlpha = visible ? 1.0f : 0.0f;
    
    // Smooth LERP for alpha
    m_Alpha = ImLerp(m_Alpha, m_TargetAlpha, dt * 10.0f);
    if (m_Alpha < 0.001f) m_Alpha = 0.0f;
    if (m_Alpha > 0.999f) m_Alpha = 1.0f;

    // Determine sliding offsets
    if (visible) {
        m_TargetSlideOffset = ImVec2(0.0f, 0.0f);
    } else {
        ImVec2 pos = GetPosition();
        float centerX = pos.x + m_Size.x * 0.5f;
        float screenW = ImGui::GetIO().DisplaySize.x;
        if (centerX < screenW * 0.5f) {
            m_TargetSlideOffset = ImVec2(-60.0f, 0.0f); // Slide left
        } else {
            m_TargetSlideOffset = ImVec2(60.0f, 0.0f);  // Slide right
        }
    }

    m_SlideOffset.x = ImLerp(m_SlideOffset.x, m_TargetSlideOffset.x, dt * 10.0f);
    m_SlideOffset.y = ImLerp(m_SlideOffset.y, m_TargetSlideOffset.y, dt * 10.0f);
}

ImU32 HudWidget::GetColorWithAlpha(ImU32 baseColor, float alphaMultiplier) const {
    unsigned int a = (baseColor >> 24) & 0xFF;
    unsigned int b = (baseColor >> 16) & 0xFF;
    unsigned int g = (baseColor >> 8) & 0xFF;
    unsigned int r = baseColor & 0xFF;
    a = (unsigned int)(a * m_Alpha * alphaMultiplier);
    return IM_COL32(r, g, b, a);
}

ImU32 HudWidget::GetColorWithAlpha(float r, float g, float b, float a) const {
    return IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(a * m_Alpha * 255.0f));
}

// ---------------------------------------------------------------------
// ClientNameWidget Implementation
// ---------------------------------------------------------------------
ClientNameWidget::ClientNameWidget()
    : HudWidget("ClientName", ImVec2(20.0f, 20.0f), ImVec2(100.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudClientName)) {
}

void ClientNameWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    
    const char* logoText = "ilonium";
    float logoW = CalcTextSize(logoText).x;
    float padding = 14.0f * scale;
    m_Size.x = logoW + padding * 2.0f;
    m_Size.y = 38.0f * scale;

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (m_Size.y - ImGui::GetFontSize() * scale) * 0.5f;
    float curLogoX = drawPos.x + padding;
    for (int i = 0; logoText[i] != '\0'; ++i) {
        float charOffset = i * 0.35f;
        float colorFactor = (std::sin((float)ImGui::GetTime() * 4.0f - charOffset) + 1.0f) * 0.5f;
        
        ImU32 charCol;
        if (UseCustomColors() || GetRgbMode()) {
            charCol = GetTextCol();
        } else {
            int r = 255;
            int g = (int)(30 + colorFactor * 75);   // Red (30) to Pink (105)
            int b = (int)(80 + colorFactor * 100);  // Red (80) to Pink (180)
            charCol = GetColorWithAlpha(IM_COL32(r, g, b, 255));
        }

        char letter[2] = { logoText[i], '\0' };
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curLogoX, startY), charCol, letter);
        curLogoX += CalcTextSize(letter).x;
    }

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// FPSWidget Implementation
// ---------------------------------------------------------------------
FPSWidget::FPSWidget()
    : HudWidget("FPS", ImVec2(160.0f, 20.0f), ImVec2(85.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudFps)) {
}

void FPSWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    int fps = (int)ImGui::GetIO().Framerate;
    char fpsBuf[32];
    snprintf(fpsBuf, sizeof(fpsBuf), "%d", fps);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 12.0f * scale;
    float valW = CalcTextSize(fpsBuf).x;
    float lblW = CalcTextSize("fps").x;
    m_Size.x = valW + lblW + 4.0f * scale + padding * 2.0f;
    m_Size.y = 38.0f * scale;

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (m_Size.y - ImGui::GetFontSize() * scale) * 0.5f;
    float curX = drawPos.x + padding;

    ImU32 valCol = IM_COL32(100, 240, 100, 255);
    if (fps < 30) valCol = IM_COL32(240, 100, 100, 255);
    else if (fps < 60) valCol = IM_COL32(240, 180, 80, 255);

    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(valCol), fpsBuf);
    curX += valW + 4.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "fps");

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// PingWidget Implementation
// ---------------------------------------------------------------------
PingWidget::PingWidget()
    : HudWidget("Ping", ImVec2(255.0f, 20.0f), ImVec2(80.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudPing)) {
}

void PingWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    int pingVal = 0;
    ePlayer* localPlayer = ePlayer::PlayerConfig(0);
    if (localPlayer && localPlayer->netPlayer) {
        pingVal = (int)(localPlayer->netPlayer->ping * 1000.0f);
    }
    char pingBuf[32];
    snprintf(pingBuf, sizeof(pingBuf), "%d", pingVal);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 12.0f * scale;
    float valW = CalcTextSize(pingBuf).x;
    float lblW = CalcTextSize("ms").x;
    m_Size.x = valW + lblW + 4.0f * scale + padding * 2.0f;
    m_Size.y = 38.0f * scale;

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (m_Size.y - ImGui::GetFontSize() * scale) * 0.5f;
    float curX = drawPos.x + padding;

    ImU32 pCol = IM_COL32(100, 240, 100, 255);
    if (pingVal > 150) pCol = IM_COL32(240, 100, 100, 255);
    else if (pingVal > 80) pCol = IM_COL32(240, 180, 80, 255);

    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(pCol), pingBuf);
    curX += valW + 4.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "ms");

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// TimeWidget Implementation
// ---------------------------------------------------------------------
TimeWidget::TimeWidget()
    : HudWidget("Time", ImVec2(345.0f, 20.0f), ImVec2(95.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudTime)) {
}

void TimeWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    std::time_t rawtime = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&rawtime);
    char timeStr[12];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 12.0f * scale;
    float timeW = CalcTextSize(timeStr).x;
    m_Size.x = timeW + padding * 2.0f;
    m_Size.y = 38.0f * scale;

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (m_Size.y - ImGui::GetFontSize() * scale) * 0.5f;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, startY), GetTextCol(IM_COL32(210, 210, 220, 255)), timeStr);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// KeybindsWidget Implementation
// ---------------------------------------------------------------------
KeybindsWidget::KeybindsWidget()
    : HudWidget("Keybinds", ImVec2(20.0f, 100.0f), ImVec2(220.0f, 180.0f), WIDGET_CONFIG_PASS(sg_hudKeybinds)) {
}

void KeybindsWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    float scale = GetScale();

    struct KeybindInfo {
        const char* displayName;
        bool state;
        int key;
    };
    std::vector<KeybindInfo> activeItems;
    auto addIfActive = [&](const char* name, bool state, int key) {
        if (state) {
            activeItems.push_back({name, state, key});
        }
    };

    addIfActive("Noclip Mode", g_NoclipMode, g_NoclipKeybind);
    addIfActive("Clean Screen", g_CleanScreen, g_CleanScreenKeybind);
    addIfActive("Custom Hitbox", g_CustomHitbox, g_CustomHitboxKeybind);
    addIfActive("Custom Fog", g_CustomFog, g_CustomFogKeybind);
    addIfActive("Show HUD", g_ShowHUD, g_ShowHUDKeybind);
    addIfActive("Rubber Gauge", g_RubberGauge, g_RubberGaugeKeybind);
    addIfActive("Speed Meter", g_SpeedMeter, g_SpeedMeterKeybind);
    addIfActive("Brake Meter", g_BrakeMeter, g_BrakeMeterKeybind);
    addIfActive("Scoreboard", g_ShowScores, g_ShowScoresKeybind);
    addIfActive("Show Ping", g_ShowPing, g_ShowPingKeybind);
    addIfActive("Alive Counter", g_AliveCounter, g_AliveCounterKeybind);
    addIfActive("Show Fastest", g_ShowFastest, g_ShowFastestKeybind);
    addIfActive("Show Time", g_ShowTime, g_ShowTimeKeybind);
    addIfActive("Auto Refresh", g_AutoPacketRefresh, g_PacketRefreshKeybind);

    float rowH = 22.0f * scale;
    float paddingHeader = 40.0f * scale;
    float minH = 75.0f * scale;
    float targetHeight = activeItems.empty() ? minH : paddingHeader + (activeItems.size() * rowH) + 10.0f * scale;

    m_Size.y = ImLerp(m_Size.y, targetHeight, ImGui::GetIO().DeltaTime * 8.0f);
    m_Size.x = 220.0f * scale;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition() + m_SlideOffset;
    ImVec2 size = GetSize();

    dl->AddRectFilled(pos, pos + size, GetBgCol(), 12.0f * scale);
    dl->AddRect(pos, pos + size, GetBorderCol(), 12.0f * scale, 0, 1.0f);

    float headerY = pos.y + 12.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + 15.0f * scale, headerY), GetTextCol(IM_COL32(255, 255, 255, 255)), "Keybinds");

    ImU32 accentColLeft = GetAccentCol(IM_COL32(0, 190, 255, 255));
    ImU32 accentColRight = GetAccentCol(IM_COL32(130, 0, 255, 0));
    dl->AddRectFilledMultiColor(
        ImVec2(pos.x + 12.0f * scale, pos.y + 30.0f * scale),
        ImVec2(pos.x + size.x - 12.0f * scale, pos.y + 32.0f * scale),
        accentColLeft, accentColRight, accentColRight, accentColLeft
    );

    float curY = pos.y + paddingHeader;
    if (activeItems.empty()) {
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + 15.0f * scale, curY + 5.0f * scale), GetTextCol(IM_COL32(110, 110, 125, 180)), "No active modules");
    } else {
        for (const auto& item : activeItems) {
            if (curY + rowH > pos.y + size.y) break;

            dl->AddCircleFilled(ImVec2(pos.x + 20.0f * scale, curY + rowH * 0.5f), 3.0f * scale, GetAccentCol(IM_COL32(0, 190, 255, 255)));

            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + 30.0f * scale, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(IM_COL32(230, 230, 235, 240)), item.displayName);

            const char* keyName = GetSDLKeyName(item.key);
            char keyBuf[32];
            snprintf(keyBuf, sizeof(keyBuf), "[%s]", keyName);
            ImVec2 kSize = CalcTextSize(keyBuf);

            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + size.x - 15.0f * scale - kSize.x, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(IM_COL32(140, 140, 160, 220)), keyBuf);

            curY += rowH;
        }
    }
}

// ---------------------------------------------------------------------
// HudManager Implementation
// ---------------------------------------------------------------------
std::vector<HudWidget*> HudManager::s_Widgets;
bool HudManager::s_Initialized = false;

// -----------------------------------------------------------------------
// Demo Recording HUD Overlay
// -----------------------------------------------------------------------
// Called from HudManager::Render().  Draws directly onto bgDl (no window,
// no background) so it is rendered cleanly on top of the game.
//
// Layout (top-right corner, default position):
//
//   ● REC  03:47  1.2 MB
//
// The red dot blinks via sin(time).  All dimensions scale with screen size.
// -----------------------------------------------------------------------


void HudManager::Init() {
    if (s_Initialized) return;
    s_Widgets.push_back(new ClientNameWidget());
    s_Widgets.push_back(new FPSWidget());
    s_Widgets.push_back(new PingWidget());
    s_Widgets.push_back(new TimeWidget());
    s_Widgets.push_back(new KeybindsWidget());
    s_Widgets.push_back(new KDWidget());
    s_Widgets.push_back(new SpeedometerWidget());
    s_Widgets.push_back(new RubberMeterWidget());
    s_Widgets.push_back(new BrakeMeterWidget());
    s_Widgets.push_back(new ScoreboardWidget());
    s_Widgets.push_back(new LiveScoreboardWidget());
    s_Widgets.push_back(new ZoneTimerWidget());
    s_Widgets.push_back(new AliveWidget());
    s_Widgets.push_back(new NetworkHealthWidget());
    s_Widgets.push_back(new RubberBatteryWidget());
    s_Widgets.push_back(new ClassicRubberBatteryWidget());
    s_Widgets.push_back(new MediaWidget());          // OS media / Spotify overlay
    s_Widgets.push_back(new ChatSkinWidget());
    s_Widgets.push_back(new MinimapWidget());
    s_Widgets.push_back(new FortressAlertsWidget());
    s_Widgets.push_back(new TeammateDeathWarningWidget());
    s_Widgets.push_back(new WallTimerWidget());
    s_Widgets.push_back(new KeystrokeVisualizerWidget(1));
    s_Widgets.push_back(new KeystrokeVisualizerWidget(2));
    s_Initialized = true;
}

void HudManager::Shutdown() {
    for (auto* widget : s_Widgets) {
        delete widget;
    }
    s_Widgets.clear();
    s_Initialized = false;
}

void HudManager::Update(float dt) {
    // Read whatever the server has said since the last frame. Logins are
    // announced in the console and nowhere else a client can reach.
    rc_ScanLogins();

    if ( sn_GetNetState() == nCLIENT )
    {
        rc_FollowPreferredTeam();
        rc_TakeFreeSlot();
        rc_LoginsFromServer();
    }

    rc_DiscordTick();

    if (!s_Initialized) return;

    for (auto* widget : s_Widgets) {
        widget->Update(dt);
    }
}

// Helper to draw a dashed border in Edit Mode
static void DrawDashedRect(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 color, float thickness, float dash_length) {
    // Top
    for (float x = min.x; x < max.x; x += dash_length * 2.0f) {
        dl->AddLine(ImVec2(x, min.y), ImVec2(std::min(x + dash_length, max.x), min.y), color, thickness);
    }
    // Bottom
    for (float x = min.x; x < max.x; x += dash_length * 2.0f) {
        dl->AddLine(ImVec2(x, max.y), ImVec2(std::min(x + dash_length, max.x), max.y), color, thickness);
    }
    // Left
    for (float y = min.y; y < max.y; y += dash_length * 2.0f) {
        dl->AddLine(ImVec2(min.x, y), ImVec2(min.x, std::min(y + dash_length, max.y)), color, thickness);
    }
    // Right
    for (float y = min.y; y < max.y; y += dash_length * 2.0f) {
        dl->AddLine(ImVec2(max.x, y), ImVec2(max.x, std::min(y + dash_length, max.y)), color, thickness);
    }
}




// ---------------------------------------------------------------------
// The table behind Tab
// ---------------------------------------------------------------------

//! The same text without its colour codes.
// How solid the score table is. It is glanced at mid-round, so the default
// leans towards seeing past it.
REAL sg_scoreboardOpacity = 0.55f;
static tConfItem< REAL > sg_scoreboardOpacityConf( "MOD_SCOREBOARD_OPACITY", sg_scoreboardOpacity );

static std::string StripColors( char const * in )
{
    std::string out;
    if ( !in ) return out;

    std::string src( in );
    for ( size_t i = 0; i < src.size(); )
    {
        if ( src[i] == '0' && i + 1 < src.size() && src[i+1] == 'x' && i + 8 <= src.size() ) { i += 8; continue; }
        out += src[i]; ++i;
    }
    return out;
}

//! Everyone in the round, ranked, with their teams.
//!
//! Drawn over the game rather than into it, so it gets the client's own type
//! and spacing instead of a bitmap font with columns counted out in
//! characters. Teams appear only when there is more than one to compare.
bool rc_PlayerIsAlive( ePlayerNetID const * p )
{
    return p && p->Object() && p->Object()->Alive();
}

static void DrawScoreTable()
{
    if ( !sg_customScoreboard || !se_ScoresWanted() )
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    rcTheme const & th = rc_Theme();

    ePlayerNetID::SortByScore();

    // The engine keeps its own order for the teams and hands it out already
    // ranked; asking for it is what puts the leader at the top here too,
    // instead of whatever order the teams happened to be created in.
    eTeam::SortByScore();

    // teams are worth a block of their own only when they mean something
    bool teamsMatter = false;
    for ( int i = eTeam::teams.Len() - 1; i >= 0; --i )
        if ( eTeam::teams[i] && eTeam::teams[i]->NumPlayers() > 1 )
        { teamsMatter = true; break; }

    int rows = se_PlayerNetIDs.Len();
    if ( rows < 1 ) return;
    if ( rows > 16 ) rows = 16;

    float scale = io.DisplaySize.y / 1080.0f;
    if ( scale < 0.7f ) scale = 0.7f;

    float rowH = 34.0f * scale;
    float headH = 64.0f * scale;
    float teamH = teamsMatter ? ( 26.0f * scale * eTeam::teams.Len() + 34.0f * scale ) : 0.0f;
    float w = 720.0f * scale;
    float h = headH + teamH + rowH * ( rows + 1 ) + 26.0f * scale;

    ImVec2 at( ( io.DisplaySize.x - w ) * 0.5f, ( io.DisplaySize.y - h ) * 0.42f );
    ImVec2 to( at.x + w, at.y + h );

    // Opened for a second in the middle of a round to check a number, so it
    // cannot be a wall. Faint enough to read the arena through, dark enough to
    // read the numbers on.
    float dim = sg_scoreboardOpacity;
    if ( dim < 0.05f ) dim = 0.05f;
    if ( dim > 1.0f ) dim = 1.0f;

    dl->AddRectFilled( at, to, rc_Fade( th.page, dim ), th.radiusLarge );
    dl->AddRect( at, to, th.line, th.radiusLarge, 0, 1.0f );
    dl->AddRectFilled( at, ImVec2( to.x, at.y + 3.0f ), rc_Fade( th.primary.solid, 0.85f ), th.radiusLarge );

    float pad = 30.0f * scale;
    float y = at.y + 24.0f * scale;

    // Drawn straight to the list: anything that moves ImGui's cursor needs a
    // window, and without one it conjures its own and labels it "Debug".
    {
        char const * head = "S C O R E S";
        dl->AddText( ImVec2( at.x + pad, y ), th.textMute, head );
    }
    y += 34.0f * scale;

    // ---- teams
    if ( teamsMatter )
    {
        for ( int i = 0; i < eTeam::teams.Len(); ++i )
        {
            eTeam * team = eTeam::teams[i];
            if ( !team ) continue;

            // Team colours are kept in sixteenths, not as a fraction of one.
            // Multiplying them straight up by 255 ran every channel far past
            // the top of a byte and it wrapped, which is how a team could come
            // out wearing another team's colour or none at all.
            ImU32 col = IM_COL32( (int)( team->R() * 255 / 15 ),
                                  (int)( team->G() * 255 / 15 ),
                                  (int)( team->B() * 255 / 15 ), 235 );
            dl->AddText( ImVec2( at.x + pad, y ), col, StripColors( (char const *)team->Name() ).c_str() );

            char pts[ 24 ];
            snprintf( pts, sizeof( pts ), "%d", team->Score() );
            ImVec2 sz = ImGui::CalcTextSize( pts );
            dl->AddText( ImVec2( to.x - pad - sz.x, y ), th.text, pts );

            y += 26.0f * scale;
        }

        y += 8.0f * scale;
        dl->AddLine( ImVec2( at.x + pad, y ), ImVec2( to.x - pad, y ), th.line, 1.0f );
        y += 18.0f * scale;
    }

    // ---- the column heads
    float colAlive = at.x + w * 0.52f;
    float colPing  = at.x + w * 0.64f;
    float colTeam  = at.x + w * 0.74f;
    float colScore = to.x - pad;

    dl->AddText( ImVec2( at.x + pad, y ), th.textMute, "PLAYER" );
    dl->AddText( ImVec2( colAlive, y ), th.textMute, "ALIVE" );
    dl->AddText( ImVec2( colPing, y ), th.textMute, "PING" );
    dl->AddText( ImVec2( colTeam, y ), th.textMute, "TEAM" );
    {
        ImVec2 sz = ImGui::CalcTextSize( "SCORE" );
        dl->AddText( ImVec2( colScore - sz.x, y ), th.textMute, "SCORE" );
    }
    y += 22.0f * scale;
    dl->AddLine( ImVec2( at.x + pad, y ), ImVec2( to.x - pad, y ), th.line, 1.0f );
    y += 10.0f * scale;

    // ---- the players
    for ( int i = 0; i < rows; ++i )
    {
        ePlayerNetID * p = se_PlayerNetIDs( i );
        if ( !p ) continue;

        bool mine = false;
        for ( int seat = 0; seat < MAX_PLAYERS; ++seat )
        {
            ePlayer * cfg = ePlayer::PlayerConfig( seat );
            if ( cfg && cfg->netPlayer == p ) { mine = true; break; }
        }

        // your own row, marked, because finding yourself in a list of sixteen
        // while the round is still going is not a thing anybody should have to
        // do by reading
        if ( mine )
            dl->AddRectFilled( ImVec2( at.x + pad * 0.4f, y - 4.0f * scale ),
                               ImVec2( to.x - pad * 0.4f, y + rowH - 8.0f * scale ),
                               rc_Fade( th.primary.solid, 0.14f ), th.radiusSmall );

        {
            REAL pr, pg, pb;
            p->Color( pr, pg, pb );
            ImU32 nameCol = IM_COL32( (int)(pr*255), (int)(pg*255), (int)(pb*255), 245 );
            dl->AddText( ImVec2( at.x + pad, y ), nameCol, StripColors( (char const *)p->GetName() ).c_str() );
        }

        bool alive = rc_PlayerIsAlive( p );
        dl->AddText( ImVec2( colAlive, y ), alive ? th.good : rc_Fade( th.textMute, 0.7f ),
                     alive ? "yes" : "no" );

        char buf[ 32 ];
        snprintf( buf, sizeof( buf ), "%d", (int)( p->ping * 1000 ) );
        dl->AddText( ImVec2( colPing, y ), th.textDim, buf );

        if ( p->CurrentTeam() )
        {
            eTeam * t = p->CurrentTeam();
            ImU32 teamCol = IM_COL32( (int)( t->R() * 255 / 15 ),
                                      (int)( t->G() * 255 / 15 ),
                                      (int)( t->B() * 255 / 15 ), 220 );
            dl->AddText( ImVec2( colTeam, y ), teamCol, StripColors( (char const *)t->Name() ).c_str() );
        }

        snprintf( buf, sizeof( buf ), "%d", p->TotalScore() );
        ImVec2 sz = ImGui::CalcTextSize( buf );
        dl->AddText( ImVec2( colScore - sz.x, y ), mine ? th.primary.bright : th.text, buf );

        y += rowH;
    }
}

extern bool sg_noclipCinematic;
// A line that appears in the middle of the screen for a moment and goes away by
// itself. The free camera uses it to say what it just did: the engine's own
// centre display is no longer drawn by this client, which is why the zoom
// readout had quietly stopped appearing at all.
namespace
{
std::string s_flashText;
double      s_flashUntil = 0.0;
}

void rc_HudFlash( char const * text, double seconds )
{
    if ( !text )
        return;
    s_flashText = text;
    s_flashUntil = tSysTimeFloat() + seconds;
}

static void DrawHudFlash()
{
    if ( s_flashText.empty() )
        return;

    double const left = s_flashUntil - tSysTimeFloat();
    if ( left <= 0.0 )
    {
        s_flashText.clear();
        return;
    }

    ImGuiIO & io = ImGui::GetIO();
    if ( io.DisplaySize.x < 1 || io.DisplaySize.y < 1 )
        return;

    float const fade = left < 0.4 ? (float)( left / 0.4 ) : 1.0f;
    float const scale = io.DisplaySize.y / 1080.0f;

    ImFont * font = GetBestFont( 26.0f * scale );
    float const size = font->FontSize > 0 ? font->FontSize : 26.0f * scale;
    ImVec2 const measured = font->CalcTextSizeA( size, FLT_MAX, 0.0f, s_flashText.c_str() );

    ImVec2 const at( io.DisplaySize.x * 0.5f - measured.x * 0.5f, io.DisplaySize.y * 0.72f );

    ImDrawList * dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled( ImVec2( at.x - 14.0f * scale, at.y - 8.0f * scale ),
                       ImVec2( at.x + measured.x + 14.0f * scale, at.y + measured.y + 8.0f * scale ),
                       IM_COL32( 8, 9, 12, (int)( 180 * fade ) ), 10.0f * scale );
    dl->AddText( font, size, at, IM_COL32( 235, 240, 250, (int)( 255 * fade ) ), s_flashText.c_str() );
}

void HudManager::Render() {
    // Before the early exits: the names were queued by the arena pass that has
    // already run, and dropping them here would leave them queued for ever.
    rc_DrawCycleNames();

    if (!s_Initialized) return;

    extern bool sg_noclipCinematic;
    if (sg_noclipCinematic && !isHudEditing) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImDrawList* bgDl = ImGui::GetBackgroundDrawList();

    DrawScoreTable();
    rc_DrawCenterMessage();
    DrawHudFlash();
    rc_DrawChatInput();
    rc_DrawStartLights();

    // 1. Draw each widget
    for (auto* widget : s_Widgets) {
        widget->Draw();
    }

    // 2. HUD Editor Mode interactions
    if (isHudEditing) {
        static HudWidget* selectedWidget = nullptr;
        static bool isDirty = false;
        static float lastSaveTime = 0.0f;
        // Overlay screen overlay showing we are editing
        bgDl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 10, 20, 25));
        
        // Show status bar at the top or bottom
        char statusText[] = "HUD EDITOR MODE | Drag widgets to position. Edge Snapping is active. Press ESC or close menu to exit.";
        ImVec2 textSz = ImGui::CalcTextSize(statusText);
        ImVec2 statusPos((io.DisplaySize.x - textSz.x) * 0.5f, io.DisplaySize.y - 45.0f);
        
        // Dark bar background
        bgDl->AddRectFilled(ImVec2(statusPos.x - 20.0f, statusPos.y - 8.0f), ImVec2(statusPos.x + textSz.x + 20.0f, statusPos.y + textSz.y + 8.0f), IM_COL32(10, 10, 15, 235), 8.0f);
        bgDl->AddRect(ImVec2(statusPos.x - 20.0f, statusPos.y - 8.0f), ImVec2(statusPos.x + textSz.x + 20.0f, statusPos.y + textSz.y + 8.0f), IM_COL32(0, 190, 255, 150), 8.0f);
        bgDl->AddText(statusPos, IM_COL32(0, 200, 255, 255), statusText);

        bool mouseClicked = ImGui::IsMouseClicked(0);
        bool mouseReleased = ImGui::IsMouseReleased(0);
        bool mouseDown = ImGui::IsMouseDown(0);

        // A click that a panel is already taking belongs to the panel. Asking
        // the windows one by one from out here does not answer that reliably;
        // this does, and it is what keeps a widget lying under the settings
        // panel from grabbing what was aimed at a slider.
        bool panelHasMouse = ImGui::GetIO().WantCaptureMouse;

        for (auto* widget : s_Widgets) {
            // Widget must be visible to be edited
            if (!widget->IsVisible()) continue;

            ImVec2 pos = widget->GetPosition();
            ImVec2 size = widget->GetSize();
            
            // Bounding box
            ImVec2 minPos = pos;
            ImVec2 maxPos = pos + size;

            bool hovered = (mousePos.x >= minPos.x && mousePos.x <= maxPos.x &&
                            mousePos.y >= minPos.y && mousePos.y <= maxPos.y);

            // Selection follows a click, not the cursor. Following the cursor
            // meant that carrying the mouse across to the panel picked up
            // every widget on the way and left the wrong one selected.
            if (hovered && mouseClicked && !panelHasMouse) {
                selectedWidget = widget;
            }

            // Dotted border color based on hover / dragging
            ImU32 outlineCol = IM_COL32(0, 190, 255, 120);
            if (widget->IsDragging()) {
                outlineCol = IM_COL32(0, 255, 100, 255);
            } else if (hovered) {
                outlineCol = IM_COL32(0, 190, 255, 240);
            }

            // Draw beautiful dotted outline around the widget
            DrawDashedRect(bgDl, minPos - ImVec2(2, 2), maxPos + ImVec2(2, 2), outlineCol, 1.5f, 6.0f);
            
            // Hover overlay card highlight
            if (hovered || widget->IsDragging()) {
                bgDl->AddRectFilled(minPos, maxPos, IM_COL32(0, 190, 255, 15), 12.0f);
            }

            // Label widget names in the editor
            char labelBuf[64];
            snprintf(labelBuf, sizeof(labelBuf), "%s (Drag)", widget->GetName().c_str());
            bgDl->AddText(ImVec2(minPos.x + 4.0f, minPos.y - 16.0f), outlineCol, labelBuf);

            // Input handling
            if (mouseClicked && hovered && !panelHasMouse) {
                // Find if any other widget is already dragging, to avoid dragging multiple
                bool otherDragging = false;
                for (auto* other : s_Widgets) {
                    if (other->IsDragging()) {
                        otherDragging = true;
                        break;
                    }
                }
                if (!otherDragging) {
                    widget->SetDragging(true);
                    widget->SetDragMouseOffset(mousePos - pos);
                }
            }

            if (widget->IsDragging()) {
                if (mouseDown) {
                    ImVec2 newPos = mousePos - widget->GetDragMouseOffset();
                    
                    // Edge Snapping logic (15.0f threshold)
                    float snapThreshold = 15.0f;
                    ImVec2 displaySize = io.DisplaySize;
                    
                    // Snap Left
                    if (std::abs(newPos.x) < snapThreshold) {
                        newPos.x = 0.0f;
                    }
                    // Snap Right
                    else if (std::abs(newPos.x + size.x - displaySize.x) < snapThreshold) {
                        newPos.x = displaySize.x - size.x;
                    }

                    // Snap Top
                    if (std::abs(newPos.y) < snapThreshold) {
                        newPos.y = 0.0f;
                    }
                    // Snap Bottom
                    else if (std::abs(newPos.y + size.y - displaySize.y) < snapThreshold) {
                        newPos.y = displaySize.y - size.y;
                    }

                    // Clamp to screen bounds to prevent widget loss
                    newPos.x = std::max(0.0f, std::min(newPos.x, displaySize.x - size.x));
                    newPos.y = std::max(0.0f, std::min(newPos.y, displaySize.y - size.y));

                    widget->SetPosition(newPos);
                    isDirty = true;
                }

                if (mouseReleased) {
                    widget->SetDragging(false);
                }
            }
        }

        // --- FLOATING INTERACTIVE CUSTOMIZER PANEL ---

        // If currently selected widget is not visible or null, find the first visible widget
        if (selectedWidget && !selectedWidget->IsVisible()) {
            selectedWidget = nullptr;
        }
        if (!selectedWidget) {
            for (auto* widget : s_Widgets) {
                if (widget->IsVisible()) {
                    selectedWidget = widget;
                    break;
                }
            }
        }

        // Highlight selected widget on screen
        if (selectedWidget) {
            ImVec2 pos = selectedWidget->GetPosition();
            ImVec2 size = selectedWidget->GetSize();
            float time = (float)ImGui::GetTime();
            ImU32 selectedBorderCol = ImGui::GetColorU32(ImLerp(ImVec4(1.0f, 0.6f, 0.0f, 0.9f), ImVec4(0.0f, 0.75f, 1.0f, 0.9f), (sinf(time * 6.0f) + 1.0f) * 0.5f));
            bgDl->AddRect(pos - ImVec2(4, 4), pos + size + ImVec2(4, 4), selectedBorderCol, 12.0f, 0, 2.5f);
        }

        // Customizer Panel Window Setup
        ImGui::SetNextWindowPos(ImVec2(30, 80), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340, 560), ImGuiCond_FirstUseEver);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.25f, 0.5f));

        if (ImGui::Begin("HUD CUSTOMIZER", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "Click/Hover a widget to edit properties");
            ImGui::Separator();
            ImGui::Spacing();

            // Every widget, switched on or off, in one place. The list used to
            // hold only the ones already showing, so a widget that was off
            // could not be found here to turn on, and picking one meant
            // hunting for it on the screen.
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.75f, 1.0f), "Widgets");
            if (ImGui::BeginChild("##widgetlist", ImVec2(0.0f, 170.0f), true)) {
                for (auto* widget : s_Widgets) {
                    if (!widget) continue;
                    ImGui::PushID(widget);

                    bool shown = widget->IsVisible();
                    if (ImGui::Checkbox("##shown", &shown)) {
                        widget->SetVisible(shown);
                        isDirty = true;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Show this one on the hud");

                    ImGui::SameLine();

                    bool chosen = (widget == selectedWidget);
                    if (ImGui::Selectable(widget->GetName().c_str(), chosen)) {
                        // picking something to edit implies wanting to see it
                        if (!widget->IsVisible()) {
                            widget->SetVisible(true);
                            isDirty = true;
                        }
                        selectedWidget = widget;
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (selectedWidget) {
                ImGui::Text("Active Widget: %s", selectedWidget->GetName().c_str());
                ImGui::Spacing();

                // Custom slider for Scale
                if (selectedWidget->m_pScale) {
                    float scaleVal = (float)(*selectedWidget->m_pScale);
                    if (ImGui::SliderFloat("Scale", &scaleVal, 0.5f, 2.5f, "%.2f")) {
                        *selectedWidget->m_pScale = (REAL)scaleVal;
                        isDirty = true;
                    }
                }

                // Custom slider for Opacity
                if (selectedWidget->m_pOpacity) {
                    float opacityVal = (float)(*selectedWidget->m_pOpacity);
                    if (ImGui::SliderFloat("Opacity", &opacityVal, 0.0f, 1.0f, "%.2f")) {
                        *selectedWidget->m_pOpacity = (REAL)opacityVal;
                        isDirty = true;
                    }
                }

                // Custom slider for Background Opacity
                if (selectedWidget->m_pBgOpacity) {
                    float bgOpacityVal = (float)(*selectedWidget->m_pBgOpacity);
                    if (ImGui::SliderFloat("Bg Opacity", &bgOpacityVal, 0.0f, 1.0f, "%.2f")) {
                        *selectedWidget->m_pBgOpacity = (REAL)bgOpacityVal;
                        isDirty = true;
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Custom Colors Toggle
                if (selectedWidget->m_pUseCustomColors) {
                    bool useCustom = *selectedWidget->m_pUseCustomColors;
                    if (ImGui::Checkbox("Use Custom Colors", &useCustom)) {
                        *selectedWidget->m_pUseCustomColors = useCustom;
                        isDirty = true;
                    }

                    if (useCustom) {
                        ImGui::Indent(10.0f);
                        
                        // Text Color
                        if (selectedWidget->m_pTextColorR) {
                            float col[4] = {
                                (float)(*selectedWidget->m_pTextColorR),
                                (float)(*selectedWidget->m_pTextColorG),
                                (float)(*selectedWidget->m_pTextColorB),
                                (float)(*selectedWidget->m_pTextColorA)
                            };
                            if (ImGui::ColorEdit4("Text Color", col)) {
                                *selectedWidget->m_pTextColorR = (REAL)col[0];
                                *selectedWidget->m_pTextColorG = (REAL)col[1];
                                *selectedWidget->m_pTextColorB = (REAL)col[2];
                                *selectedWidget->m_pTextColorA = (REAL)col[3];
                                isDirty = true;
                            }
                        }

                        // Background Color
                        if (selectedWidget->m_pBgColorR) {
                            float col[4] = {
                                (float)(*selectedWidget->m_pBgColorR),
                                (float)(*selectedWidget->m_pBgColorG),
                                (float)(*selectedWidget->m_pBgColorB),
                                (float)(*selectedWidget->m_pBgColorA)
                            };
                            if (ImGui::ColorEdit4("Bg Color", col)) {
                                *selectedWidget->m_pBgColorR = (REAL)col[0];
                                *selectedWidget->m_pBgColorG = (REAL)col[1];
                                *selectedWidget->m_pBgColorB = (REAL)col[2];
                                *selectedWidget->m_pBgColorA = (REAL)col[3];
                                isDirty = true;
                            }
                        }

                        // Border Color
                        if (selectedWidget->m_pBorderColorR) {
                            float col[4] = {
                                (float)(*selectedWidget->m_pBorderColorR),
                                (float)(*selectedWidget->m_pBorderColorG),
                                (float)(*selectedWidget->m_pBorderColorB),
                                (float)(*selectedWidget->m_pBorderColorA)
                            };
                            if (ImGui::ColorEdit4("Border Color", col)) {
                                *selectedWidget->m_pBorderColorR = (REAL)col[0];
                                *selectedWidget->m_pBorderColorG = (REAL)col[1];
                                *selectedWidget->m_pBorderColorB = (REAL)col[2];
                                *selectedWidget->m_pBorderColorA = (REAL)col[3];
                                isDirty = true;
                            }
                        }

                        // Accent Color
                        if (selectedWidget->m_pAccentColorR) {
                            float col[4] = {
                                (float)(*selectedWidget->m_pAccentColorR),
                                (float)(*selectedWidget->m_pAccentColorG),
                                (float)(*selectedWidget->m_pAccentColorB),
                                (float)(*selectedWidget->m_pAccentColorA)
                            };
                            if (ImGui::ColorEdit4("Accent Color", col)) {
                                *selectedWidget->m_pAccentColorR = (REAL)col[0];
                                *selectedWidget->m_pAccentColorG = (REAL)col[1];
                                *selectedWidget->m_pAccentColorB = (REAL)col[2];
                                *selectedWidget->m_pAccentColorA = (REAL)col[3];
                                isDirty = true;
                            }
                        }

                        ImGui::Unindent(10.0f);
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Chroma RGB Mode Toggle
                if (selectedWidget->m_pRgbMode) {
                    bool rgbMode = *selectedWidget->m_pRgbMode;
                    if (ImGui::Checkbox("Chroma RGB Mode", &rgbMode)) {
                        *selectedWidget->m_pRgbMode = rgbMode;
                        isDirty = true;
                    }

                    if (rgbMode && selectedWidget->m_pRgbSpeed) {
                        float rgbSpeed = (float)(*selectedWidget->m_pRgbSpeed);
                        if (ImGui::SliderFloat("RGB Speed", &rgbSpeed, 0.1f, 5.0f, "%.1fx")) {
                            *selectedWidget->m_pRgbSpeed = (REAL)rgbSpeed;
                            isDirty = true;
                        }
                    }
                }

                // Draw custom widget specific settings if any
                selectedWidget->DrawCustomSettings(isDirty);
            } else {
                ImGui::Text("No active widget visible or selected.");
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);

        // Real-time auto-saving with throttle (every 0.5s)
        if (isDirty) {
            float currentTime = (float)ImGui::GetTime();
            if (currentTime - lastSaveTime > 0.5f) {
                extern void st_SaveConfig();
                st_SaveConfig();
                lastSaveTime = currentTime;
                isDirty = false;
            }
        }

        // On-the-fly save on close detection
        static bool lastHudEditing = false;
        if (!isHudEditing && lastHudEditing) {
            extern void st_SaveConfig();
            st_SaveConfig();
        }
        lastHudEditing = isHudEditing;
    }
}

HudWidget* HudManager::FindWidget(const std::string& name) {
    if (!s_Initialized) return nullptr;
    for (auto* widget : s_Widgets) {
        if (widget->GetName() == name) {
            return widget;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------
// KDWidget Implementation
// ---------------------------------------------------------------------
KDWidget::KDWidget()
    : HudWidget("KD", ImVec2(20.0f, 290.0f), ImVec2(180.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudKD)) {
}

void KDWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    float height = 38.0f * scale;

    // Retrieve K/D values
    extern int sg_kdKills;
    extern int sg_kdDeaths;
    float kd = (sg_kdDeaths > 0) ? (float)sg_kdKills / (float)sg_kdDeaths : (float)sg_kdKills;

    char kBuf[32], dBuf[32], kdBuf[32];
    snprintf(kBuf, sizeof(kBuf), "%d", sg_kdKills);
    snprintf(dBuf, sizeof(dBuf), "%d", sg_kdDeaths);
    if (sg_kdDeaths > 0)
        snprintf(kdBuf, sizeof(kdBuf), "%.2f", kd);
    else
        snprintf(kdBuf, sizeof(kdBuf), "%.0f", kd);

    // Color code K/D ratio
    ImU32 kdValColor = IM_COL32(100, 240, 100, 240); // Green
    if (sg_kdDeaths > 0) {
        if (kd < 1.0f) kdValColor = IM_COL32(240, 100, 100, 240); // Red
        else if (kd < 2.0f) kdValColor = IM_COL32(240, 180, 80, 240); // Yellow/Orange
    }

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 12.0f * scale;
    float spacing = 8.0f * scale;

    float kLabelW = CalcTextSize("K:").x;
    float kValW = CalcTextSize(kBuf).x;
    float dLabelW = CalcTextSize("D:").x;
    float dValW = CalcTextSize(dBuf).x;
    float kdLabelW = CalcTextSize("K/D:").x;
    float kdValW = CalcTextSize(kdBuf).x;

    float calculatedW = padding 
        + kLabelW + kValW + spacing + spacing // Divider 1
        + dLabelW + dValW + spacing + spacing // Divider 2
        + kdLabelW + kdValW + padding;

    m_Size.x = calculatedW;
    m_Size.y = height;

    ImVec2 drawPos = pos + m_SlideOffset;

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (height - ImGui::GetFontSize() * scale) * 0.5f;
    float curX = drawPos.x + padding;

    // Kills
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "K:");
    curX += kLabelW + 2.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(0, 190, 255, 255)), kBuf);
    curX += kValW + spacing;

    // Divider 1
    ImU32 divCol = GetColorWithAlpha(IM_COL32(255, 255, 255, 20));
    dl->AddLine(ImVec2(curX, drawPos.y + 10.0f * scale), ImVec2(curX, drawPos.y + height - 10.0f * scale), divCol, 1.0f);
    curX += spacing;

    // Deaths
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "D:");
    curX += dLabelW + 2.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(255, 70, 70, 255)), dBuf);
    curX += dValW + spacing;

    // Divider 2
    dl->AddLine(ImVec2(curX, drawPos.y + 10.0f * scale), ImVec2(curX, drawPos.y + height - 10.0f * scale), divCol, 1.0f);
    curX += spacing;

    // K/D Ratio
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "K/D:");
    curX += kdLabelW + 2.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(kdValColor), kdBuf);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// SpeedometerWidget Implementation
// ---------------------------------------------------------------------
SpeedometerWidget::SpeedometerWidget()
    : HudWidget("Speedometer", ImVec2(250.0f, 320.0f), ImVec2(110.0f, 110.0f), WIDGET_CONFIG_PASS(sg_hudSpeedometer)) {
}

void SpeedometerWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(110.0f * scale, 110.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    // Fetch speed
    ePlayer* player = ePlayer::PlayerConfig(0);
    gCycle* h = (player && player->netPlayer) ? dynamic_cast<gCycle*>(player->netPlayer->Object()) : nullptr;
    float speed = h ? (float)h->Speed() : 0.0f;

    // Determine max speed dynamically
    static float maxMeterSpeed = 50.0f;
    if (speed > maxMeterSpeed) {
        maxMeterSpeed = speed + 10.0f;
    } else if (maxMeterSpeed > 50.0f && speed < maxMeterSpeed - 20.0f) {
        maxMeterSpeed -= ImGui::GetIO().DeltaTime * 2.0f;
        if (maxMeterSpeed < 50.0f) maxMeterSpeed = 50.0f;
    }

    float pct = speed / maxMeterSpeed;
    if (pct > 1.0f) pct = 1.0f;
    if (pct < 0.0f) pct = 0.0f;

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + size, GetBgCol(), 12.0f * scale);
    dl->AddRect(drawPos, drawPos + size, GetBorderCol(), 12.0f * scale, 0, 1.0f);

    // Circular gauge center & radius
    ImVec2 center = drawPos + ImVec2(size.x * 0.5f, 42.0f * scale);
    float radius = 30.0f * scale;
    float startAngle = 3.0f * M_PI / 4.0f; // Bottom-left
    float endAngle = 9.0f * M_PI / 4.0f;   // Bottom-right

    // Draw background track
    dl->PathArcTo(center, radius, startAngle, endAngle, 32);
    dl->PathStroke(GetColorWithAlpha(IM_COL32(40, 40, 45, 180)), 0, 3.5f * scale);

    // Draw filled active track
    float currentAngle = startAngle + (endAngle - startAngle) * pct;
    if (pct > 0.001f) {
        dl->PathArcTo(center, radius, startAngle, currentAngle, 32);
        dl->PathStroke(GetAccentCol(IM_COL32(0, 190, 255, 255)), 0, 3.5f * scale);
    }

    // Draw needle
    ImVec2 needleDir = ImVec2(cosf(currentAngle), sinf(currentAngle));
    ImVec2 needleStart = center + needleDir * 6.0f * scale;
    ImVec2 needleEnd = center + needleDir * (radius - 2.0f * scale);
    dl->AddLine(needleStart, needleEnd, GetColorWithAlpha(IM_COL32(255, 255, 255, 255)), 2.0f * scale);
    dl->AddCircleFilled(center, 4.0f * scale, GetAccentCol(IM_COL32(0, 190, 255, 255)));

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    // Speed value text lower down
    char speedStr[32];
    snprintf(speedStr, sizeof(speedStr), "%.0f", speed);
    float valW = CalcTextSize(speedStr).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - valW * 0.5f, drawPos.y + 74.0f * scale), GetTextCol(IM_COL32(255, 255, 255, 255)), speedStr);

    // Widget title at the very bottom
    float titleW = CalcTextSize("SPEED").x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - titleW * 0.5f, drawPos.y + 92.0f * scale), GetTextCol(IM_COL32(140, 140, 150, 220)), "SPEED");

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// RubberMeterWidget Implementation
// ---------------------------------------------------------------------
RubberMeterWidget::RubberMeterWidget()
    : HudWidget("Rubber", ImVec2(370.0f, 320.0f), ImVec2(110.0f, 110.0f), WIDGET_CONFIG_PASS(sg_hudRubber)) {
}

void RubberMeterWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(110.0f * scale, 110.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    // Fetch rubber
    ePlayer* player = ePlayer::PlayerConfig(0);
    gCycle* h = (player && player->netPlayer) ? dynamic_cast<gCycle*>(player->netPlayer->Object()) : nullptr;
    
    float rubber = h ? (float)h->GetRubber() : 0.0f;
    float maxRubber = sg_rubberCycle;
    float pct = (maxRubber > 0.01f) ? rubber / maxRubber : 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    if (pct < 0.0f) pct = 0.0f;

    bool malusActive = gCycle::RubberMalusActive() && h && h->GetRubberMalus() > 0.01f;

    // Draw capsule body
    ImU32 bgCol = GetBgCol();
    ImU32 borderCol;
    if (malusActive) {
        float pulse = (sinf((float)ImGui::GetTime() * 12.0f) + 1.0f) * 0.5f;
        borderCol = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(120 + pulse * 135)));
    } else {
        borderCol = GetBorderCol();
    }
    dl->AddRectFilled(drawPos, drawPos + size, bgCol, 12.0f * scale);
    dl->AddRect(drawPos, drawPos + size, borderCol, 12.0f * scale, 0, malusActive ? 1.5f * scale : 1.0f * scale);

    // Circular gauge center & radius
    ImVec2 center = drawPos + ImVec2(size.x * 0.5f, 42.0f * scale);
    float radius = 30.0f * scale;
    float startAngle = 3.0f * M_PI / 4.0f; // Bottom-left
    float endAngle = 9.0f * M_PI / 4.0f;   // Bottom-right

    // Draw background track
    dl->PathArcTo(center, radius, startAngle, endAngle, 32);
    dl->PathStroke(GetColorWithAlpha(IM_COL32(40, 40, 45, 180)), 0, 3.5f * scale);

    // Draw filled active track
    float currentAngle = startAngle + (endAngle - startAngle) * pct;
    ImU32 fillCol;
    if (malusActive) {
        fillCol = IM_COL32(255, 30, 30, 255); // Red when breaking
    } else {
        if (UseCustomColors() || GetRgbMode()) {
            fillCol = GetAccentCol();
        } else {
            int r = (int)(pct < 0.5f ? (pct * 2.0f * 255.0f) : 255.0f);
            int g = (int)(pct > 0.5f ? ((1.0f - pct) * 2.0f * 255.0f) : 255.0f);
            fillCol = IM_COL32(r, g, 0, 255);
        }
    }

    if (pct > 0.001f) {
        dl->PathArcTo(center, radius, startAngle, currentAngle, 32);
        dl->PathStroke(GetColorWithAlpha(fillCol), 0, 3.5f * scale);
    }

    // Draw needle
    ImVec2 needleDir = ImVec2(cosf(currentAngle), sinf(currentAngle));
    ImVec2 needleStart = center + needleDir * 6.0f * scale;
    ImVec2 needleEnd = center + needleDir * (radius - 2.0f * scale);
    dl->AddLine(needleStart, needleEnd, GetColorWithAlpha(IM_COL32(255, 255, 255, 255)), 2.0f * scale);
    dl->AddCircleFilled(center, 4.0f * scale, GetColorWithAlpha(fillCol));

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    // Rubber value text lower down
    char rubStr[32];
    snprintf(rubStr, sizeof(rubStr), "%.1f", rubber);
    float valW = CalcTextSize(rubStr).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - valW * 0.5f, drawPos.y + 74.0f * scale), GetTextCol(malusActive ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 255, 255)), rubStr);

    // Widget title at the very bottom
    const char* titleText = malusActive ? "BREAKING" : "RUBBER";
    float titleW = CalcTextSize(titleText).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - titleW * 0.5f, drawPos.y + 92.0f * scale), GetTextCol(malusActive ? IM_COL32(255, 100, 100, 220) : IM_COL32(140, 140, 150, 220)), titleText);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// BrakeMeterWidget Implementation
// ---------------------------------------------------------------------
BrakeMeterWidget::BrakeMeterWidget()
    : HudWidget("Brakes", ImVec2(490.0f, 320.0f), ImVec2(110.0f, 110.0f), WIDGET_CONFIG_PASS(sg_hudBrake)) {
}

void BrakeMeterWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(110.0f * scale, 110.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    // Fetch brakes
    ePlayer* player = ePlayer::PlayerConfig(0);
    gCycle* h = (player && player->netPlayer) ? dynamic_cast<gCycle*>(player->netPlayer->Object()) : nullptr;
    
    float brakes = h ? (float)h->GetBrakingReservoir() : 1.0f;
    float pct = brakes;
    if (pct > 1.0f) pct = 1.0f;
    if (pct < 0.0f) pct = 0.0f;

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + size, GetBgCol(), 12.0f * scale);
    dl->AddRect(drawPos, drawPos + size, GetBorderCol(), 12.0f * scale, 0, 1.0f);

    // Circular gauge center & radius
    ImVec2 center = drawPos + ImVec2(size.x * 0.5f, 42.0f * scale);
    float radius = 30.0f * scale;
    float startAngle = 3.0f * M_PI / 4.0f; // Bottom-left
    float endAngle = 9.0f * M_PI / 4.0f;   // Bottom-right

    // Draw background track
    dl->PathArcTo(center, radius, startAngle, endAngle, 32);
    dl->PathStroke(GetColorWithAlpha(IM_COL32(40, 40, 45, 180)), 0, 3.5f * scale);

    // Draw filled active track
    float currentAngle = startAngle + (endAngle - startAngle) * pct;
    ImU32 fillCol = GetAccentCol(IM_COL32(255, 140, 0, 255)); // Amber
    if (pct > 0.001f) {
        dl->PathArcTo(center, radius, startAngle, currentAngle, 32);
        dl->PathStroke(GetColorWithAlpha(fillCol), 0, 3.5f * scale);
    }

    // Draw needle
    ImVec2 needleDir = ImVec2(cosf(currentAngle), sinf(currentAngle));
    ImVec2 needleStart = center + needleDir * 6.0f * scale;
    ImVec2 needleEnd = center + needleDir * (radius - 2.0f * scale);
    dl->AddLine(needleStart, needleEnd, GetColorWithAlpha(IM_COL32(255, 255, 255, 255)), 2.0f * scale);
    dl->AddCircleFilled(center, 4.0f * scale, GetColorWithAlpha(fillCol));

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    // Brakes value text lower down
    char brkStr[32];
    snprintf(brkStr, sizeof(brkStr), "%d%%", (int)(pct * 100.0f));
    float valW = CalcTextSize(brkStr).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - valW * 0.5f, drawPos.y + 74.0f * scale), GetTextCol(IM_COL32(255, 255, 255, 255)), brkStr);

    // Widget title at the very bottom
    float titleW = CalcTextSize("BRAKES").x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + 55.0f * scale - titleW * 0.5f, drawPos.y + 92.0f * scale), GetTextCol(IM_COL32(140, 140, 150, 220)), "BRAKES");

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// ScoreboardWidget Implementation
// ---------------------------------------------------------------------
ScoreboardWidget::ScoreboardWidget()
    : HudWidget("Scoreboard", ImVec2(500.0f, 20.0f), ImVec2(180.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudScoreboard)) {
}

void ScoreboardWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    float height = 38.0f * scale;

    // Fetch scores
    int myScore = 0;
    ePlayer* player = ePlayer::PlayerConfig(0);
    if (player && player->netPlayer) {
        myScore = player->netPlayer->TotalScore();
    }

    int topscore = 0;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID* p = se_PlayerNetIDs[i];
        if (p) {
            int score = p->TotalScore();
            if (score > topscore) {
                topscore = score;
            }
        }
    }

    char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "%d", myScore);

    char topBuf[32];
    snprintf(topBuf, sizeof(topBuf), "%d", topscore);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 14.0f * scale;
    float spacing = 8.0f * scale;

    float scoreLblW = CalcTextSize("SCORE:").x;
    float scoreValW = CalcTextSize(scoreBuf).x;
    float topLblW = CalcTextSize("TOP:").x;
    float topValW = CalcTextSize(topBuf).x;

    float calculatedW = padding 
        + scoreLblW + 2.0f * scale + scoreValW + spacing + spacing // Divider
        + topLblW + 2.0f * scale + topValW + padding;

    m_Size.x = calculatedW;
    m_Size.y = height;

    ImVec2 drawPos = pos + m_SlideOffset;

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    float startY = drawPos.y + (height - ImGui::GetFontSize() * scale) * 0.5f;
    float curX = drawPos.x + padding;

    // Score
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "SCORE:");
    curX += scoreLblW + 2.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(255, 215, 0, 255)), scoreBuf);
    curX += scoreValW + spacing;

    // Divider
    ImU32 divCol = GetColorWithAlpha(IM_COL32(255, 255, 255, 20));
    dl->AddLine(ImVec2(curX, drawPos.y + 10.0f * scale), ImVec2(curX, drawPos.y + height - 10.0f * scale), divCol, 1.0f);
    curX += spacing;

    // Top Score
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "TOP:");
    curX += topLblW + 2.0f * scale;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(IM_COL32(180, 100, 255, 255)), topBuf);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// AliveWidget Implementation
// ---------------------------------------------------------------------
AliveWidget::AliveWidget()
    : HudWidget("Alive", ImVec2(680.0f, 20.0f), ImVec2(110.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudAlive)) {
}

void AliveWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(110.0f * scale, 38.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    int totalPlayers = 0;
    int alivePlayers = 0;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID* p = se_PlayerNetIDs[i];
        if (p) {
            totalPlayers++;
            if (rc_PlayerIsAlive( p )) {
                alivePlayers++;
            }
        }
    }

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    char aliveBuf[32];
    snprintf(aliveBuf, sizeof(aliveBuf), "%d/%d", alivePlayers, totalPlayers);
    
    float lblW = CalcTextSize("ALIVE:").x;
    float valW = CalcTextSize(aliveBuf).x;
    float totalW = lblW + 4.0f * scale + valW;
    
    float startX = drawPos.x + (size.x - totalW) * 0.5f;
    float startY = drawPos.y + (size.y - ImGui::GetFontSize() * scale) * 0.5f;

    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(startX, startY), GetTextCol(IM_COL32(130, 130, 140, 240)), "ALIVE:");
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(startX + lblW + 4.0f * scale, startY), GetAccentCol(IM_COL32(0, 190, 255, 255)), aliveBuf);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// NetworkHealthWidget Implementation
// ---------------------------------------------------------------------
NetworkHealthWidget::NetworkHealthWidget()
    : HudWidget("NetworkHealth", ImVec2(425.0f, 20.0f), ImVec2(160.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudNetHealth)) {
}

void NetworkHealthWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    float lossVal = 0.0f;
    int pingVal = 0;
    
    // Read from sn_Connections[0]
    lossVal = sn_Connections[0].PacketLoss() * 100.0f;
    pingVal = (int)(sn_Connections[0].ping.GetPing() * 1000.0f);
    if (pingVal < 0) pingVal = 0;
    if (pingVal > 999) pingVal = 999;
    if (lossVal < 0.0f) lossVal = 0.0f;
    if (lossVal > 100.0f) lossVal = 100.0f;

    char lossBuf[32];
    snprintf(lossBuf, sizeof(lossBuf), "%.1f%%", lossVal);
    char pingBuf[32];
    snprintf(pingBuf, sizeof(pingBuf), "%d ms", pingVal);

    m_Size = ImVec2(160.0f * scale, 38.0f * scale);
    ImVec2 size = m_Size;

    // Draw capsule body
    dl->AddRectFilled(drawPos, drawPos + size, GetBgCol(), 19.0f * scale);
    dl->AddRect(drawPos, drawPos + size, GetBorderCol(), 19.0f * scale, 0, 1.0f);

    // Determine connection quality and colors
    ImU32 qualityColor = IM_COL32(100, 240, 100, 255); // Green
    int numBars = 4;
    
    if (lossVal > 5.0f || pingVal > 200) {
        qualityColor = IM_COL32(240, 70, 70, 255); // Red (Dying)
        numBars = 1;
        if (int(ImGui::GetTime() * 4.0f) % 2 == 0) {
            qualityColor = IM_COL32(240, 70, 70, 100);
        }
    } else if (lossVal > 2.0f || pingVal > 120) {
        qualityColor = IM_COL32(240, 150, 50, 255); // Orange (Poor)
        numBars = 2;
    } else if (lossVal > 0.5f || pingVal > 70) {
        qualityColor = IM_COL32(240, 210, 50, 255); // Yellow (Fair)
        numBars = 3;
    }

    // Draw signal bars on the left
    float barStartX = drawPos.x + 18.0f * scale;
    float barBaseY = drawPos.y + 26.0f * scale;
    float barSpacing = 4.0f * scale;
    float barWidth = 3.0f * scale;
    
    for (int i = 0; i < 4; i++) {
        float barHeight = (4.0f + i * 4.0f) * scale;
        ImVec2 barMin(barStartX + i * (barWidth + barSpacing), barBaseY - barHeight);
        ImVec2 barMax(barMin.x + barWidth, barBaseY);
        
        ImU32 barCol = (i < numBars) ? qualityColor : IM_COL32(60, 60, 70, 100);
        dl->AddRectFilled(barMin, barMax, GetColorWithAlpha(barCol), 1.0f * scale);
    }

    // Draw Ping and Loss info on the right
    float textStartX = barStartX + 4 * (barWidth + barSpacing) + 8.0f * scale;
    float textY1 = drawPos.y + 4.0f * scale;
    float textY2 = drawPos.y + 19.0f * scale;

    ImGui::PushStyleColor(ImGuiCol_Text, GetColorWithAlpha(IM_COL32(200, 200, 210, 255)));
    
    // Draw ping row
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textStartX, textY1), GetTextCol(IM_COL32(140, 140, 150, 255)), "Ping:");
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textStartX + 42.0f * scale, textY1), GetTextCol(pingVal > 120 ? IM_COL32(240, 120, 120, 255) : IM_COL32(230, 230, 235, 255)), pingBuf);

    // Draw loss row
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textStartX, textY2), GetTextCol(IM_COL32(140, 140, 150, 255)), "Loss:");
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textStartX + 42.0f * scale, textY2), GetColorWithAlpha(qualityColor), lossBuf);

    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------
// LiveScoreboardWidget and ZoneTimerWidget Implementation
// ---------------------------------------------------------------------

static inline bool IsHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline unsigned char HexToVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return 0;
}

static void RenderArmaText(ImDrawList* dl, ImVec2 pos, ImU32 defaultColor, const char* text, float scale, float alpha = 1.0f) {
    ImU32 curCol = defaultColor;
    float curX = pos.x;
    const char* p = text;
    std::string currentSegment = "";
    
    auto renderSegment = [&]() {
        if (!currentSegment.empty()) {
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, pos.y), curCol, currentSegment.c_str());
            curX += ImGui::CalcTextSize(currentSegment.c_str()).x * scale;
            currentSegment.clear();
        }
    };

    while (*p) {
        if (p[0] == '0' && p[1] == 'x') {
            bool isHex = true;
            for (int i = 2; i < 8; ++i) {
                if (!p[i] || !IsHexChar(p[i])) {
                    isHex = false;
                    break;
                }
            }
            if (isHex) {
                renderSegment();
                unsigned int r = (HexToVal(p[2]) << 4) | HexToVal(p[3]);
                unsigned int g = (HexToVal(p[4]) << 4) | HexToVal(p[5]);
                unsigned int b = (HexToVal(p[6]) << 4) | HexToVal(p[7]);
                curCol = IM_COL32(r, g, b, (int)(255 * alpha));
                p += 8;
                continue;
            } else if (strncmp(p, "0xRESETT", 8) == 0) {
                renderSegment();
                curCol = defaultColor;
                p += 8;
                continue;
            } else if (strncmp(p, "0xRESET", 7) == 0) {
                renderSegment();
                curCol = defaultColor;
                p += 7;
                continue;
            }
        }
        currentSegment += *p;
        p++;
    }
    renderSegment();
}

static ImVec2 CalcArmaTextSize(const char* text, float scale) {
    float curX = 0.0f;
    const char* p = text;
    std::string currentSegment = "";
    
    auto measureSegment = [&]() {
        if (!currentSegment.empty()) {
            curX += ImGui::CalcTextSize(currentSegment.c_str()).x * scale;
            currentSegment.clear();
        }
    };

    while (*p) {
        if (p[0] == '0' && p[1] == 'x') {
            bool isHex = true;
            for (int i = 2; i < 8; ++i) {
                if (!p[i] || !IsHexChar(p[i])) {
                    isHex = false;
                    break;
                }
            }
            if (isHex) {
                measureSegment();
                p += 8;
                continue;
            } else if (strncmp(p, "0xRESETT", 8) == 0) {
                measureSegment();
                p += 8;
                continue;
            } else if (strncmp(p, "0xRESET", 7) == 0) {
                measureSegment();
                p += 7;
                continue;
            }
        }
        currentSegment += *p;
        p++;
    }
    measureSegment();
    return ImVec2(curX, ImGui::GetFontSize() * scale);
}

// ---------------------------------------------------------------------
// LiveScoreboardWidget Implementation
// ---------------------------------------------------------------------
LiveScoreboardWidget::LiveScoreboardWidget()
    : HudWidget("LiveScoreboard", ImVec2(500.0f, 80.0f), ImVec2(260.0f, 150.0f), WIDGET_CONFIG_PASS(sg_hudLiveScoreboard)) {
}

struct ScoreboardEntry {
    ePlayerNetID* playerNetID;
    std::string name;
    int score;
    bool alive;
    int ping;
    ImU32 color;
};

struct TeamEntry {
    eTeam* team;
    std::string name;
    int score;
    ImU32 color;
    std::vector<ScoreboardEntry> players;
};

void LiveScoreboardWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    std::vector<TeamEntry> teams;
    std::vector<ScoreboardEntry> ffaPlayers;
    bool isTeamGame = false;

    int activeTeamsCount = 0;
    int teamsWithMultiplePlayers = 0;
    for (int i = 0; i < eTeam::teams.Len(); i++) {
        eTeam* t = eTeam::teams(i);
        if (t && t->NumPlayers() > 0) {
            activeTeamsCount++;
            if (t->NumPlayers() > 1) {
                teamsWithMultiplePlayers++;
            }
        }
    }
    if (teamsWithMultiplePlayers > 0) {
        isTeamGame = true;
    }

    for (int i = 0; i < eTeam::teams.Len(); i++) {
        eTeam* t = eTeam::teams(i);
        if (!t || t->NumPlayers() == 0) continue;

        if (isTeamGame && sg_modScoreboardShowTeams) {
            TeamEntry te;
            te.team = t;
            te.name = (const char*)(t->Name());
            te.score = t->Score();
            te.color = IM_COL32(t->R() * 17, t->G() * 17, t->B() * 17, 255);

            for (int j = 0; j < t->NumPlayers(); j++) {
                ePlayerNetID* p = t->Player(j);
                if (!p) continue;

                ScoreboardEntry se;
                se.playerNetID = p;
                se.name = p->GetColoredName();
                se.score = p->Score();
                se.alive = rc_PlayerIsAlive( p );
                se.ping = (int)(p->ping * 1000.0f);
                se.color = IM_COL32(p->r * 17, p->g * 17, p->b * 17, 255);

                te.players.push_back(se);
            }

            std::sort(te.players.begin(), te.players.end(), [](const ScoreboardEntry& a, const ScoreboardEntry& b) {
                return a.score > b.score;
            });

            teams.push_back(te);
        } else {
            for (int j = 0; j < t->NumPlayers(); j++) {
                ePlayerNetID* p = t->Player(j);
                if (!p) continue;

                ScoreboardEntry se;
                se.playerNetID = p;
                se.name = p->GetColoredName();
                se.score = p->Score();
                se.alive = rc_PlayerIsAlive( p );
                se.ping = (int)(p->ping * 1000.0f);
                se.color = IM_COL32(p->r * 17, p->g * 17, p->b * 17, 255);

                ffaPlayers.push_back(se);
            }
        }
    }

    if (isTeamGame && sg_modScoreboardShowTeams) {
        std::sort(teams.begin(), teams.end(), [](const TeamEntry& a, const TeamEntry& b) {
            return a.score > b.score;
        });
    } else {
        std::sort(ffaPlayers.begin(), ffaPlayers.end(), [](const ScoreboardEntry& a, const ScoreboardEntry& b) {
            return a.score > b.score;
        });
    }

    if (ffaPlayers.empty() && teams.empty() && isHudEditing) {
        if (sg_modScoreboardShowTeams) {
            TeamEntry t1, t2;
            t1.name = "0xff3333Red Team";
            t1.score = 25;
            t1.color = IM_COL32(255, 50, 50, 255);

            ScoreboardEntry p1, p2;
            p1.name = "0xff9999Player One";
            p1.score = 15;
            p1.alive = true;
            p1.ping = 45;
            p1.color = IM_COL32(255, 150, 150, 255);
            t1.players.push_back(p1);

            p2.name = "0xff5555Player Two";
            p2.score = 10;
            p2.alive = false;
            p2.ping = 60;
            p2.color = IM_COL32(255, 80, 80, 255);
            t1.players.push_back(p2);

            t2.name = "0x3333ffBlue Team";
            t2.score = 18;
            t2.color = IM_COL32(50, 50, 255, 255);

            ScoreboardEntry p3;
            p3.name = "0x9999ffPlayer Three";
            p3.score = 18;
            p3.alive = true;
            p3.ping = 30;
            p3.color = IM_COL32(150, 150, 255, 255);
            t2.players.push_back(p3);

            teams.push_back(t1);
            teams.push_back(t2);
            isTeamGame = true;
        } else {
            ScoreboardEntry p1, p2, p3;
            p1.name = "0xff3333Alpha";
            p1.score = 42;
            p1.alive = true;
            p1.ping = 25;
            p1.color = IM_COL32(255, 50, 50, 255);

            p2.name = "0x33ff33Beta";
            p2.score = 30;
            p2.alive = true;
            p2.ping = 48;
            p2.color = IM_COL32(50, 255, 50, 255);

            p3.name = "0x888888Gamma";
            p3.score = 15;
            p3.alive = false;
            p3.ping = 110;
            p3.color = IM_COL32(120, 120, 120, 255);

            ffaPlayers.push_back(p1);
            ffaPlayers.push_back(p2);
            ffaPlayers.push_back(p3);
        }
    }

    if (ffaPlayers.empty() && teams.empty()) {
        return;
    }

    ePlayer* lp = ePlayer::PlayerConfig(0);
    ePlayerNetID* localNetID = (lp && lp->netPlayer) ? lp->netPlayer : nullptr;

    float scale = GetScale();
    float rowH = 22.0f * scale;
    float paddingHeader = 36.0f * scale;
    int rowsCount = 0;

    if (isTeamGame && sg_modScoreboardShowTeams) {
        for (const auto& t : teams) {
            rowsCount++;
            if (sg_modScoreboardShowPlayers) {
                int pCount = std::min((int)t.players.size(), sg_modScoreboardMaxPlayers);
                rowsCount += pCount;
            }
        }
    } else {
        int pCount = std::min((int)ffaPlayers.size(), sg_modScoreboardMaxPlayers);
        rowsCount += pCount;

        bool localPlayerFound = false;
        if (localNetID) {
            for (int i = 0; i < pCount; i++) {
                if (ffaPlayers[i].playerNetID == localNetID) {
                    localPlayerFound = true;
                    break;
                }
            }
            if (!localPlayerFound && ffaPlayers.size() > (size_t)sg_modScoreboardMaxPlayers) {
                int localIdx = -1;
                for (size_t i = 0; i < ffaPlayers.size(); i++) {
                    if (ffaPlayers[i].playerNetID == localNetID) {
                        localIdx = (int)i;
                        break;
                    }
                }
                if (localIdx != -1) {
                    rowsCount += 2;
                }
            }
        }
    }

    float targetHeight = paddingHeader + (rowsCount * rowH) + 12.0f * scale;
    m_Size.x = 260.0f * scale;
    m_Size.y = ImLerp(m_Size.y, targetHeight, ImGui::GetIO().DeltaTime * 10.0f);

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition() + m_SlideOffset;
    ImVec2 size = GetSize();

    dl->AddRectFilled(pos, pos + size, GetBgCol(), 12.0f * scale);
    dl->AddRect(pos, pos + size, GetBorderCol(), 12.0f * scale, 0, 1.0f);

    float headerY = pos.y + 10.0f * scale;
    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + 15.0f * scale, headerY), GetTextCol(IM_COL32(255, 255, 255, 255)), "Live Scoreboard");
    if (g_FontHeader) ImGui::PopFont();

    ImU32 accentColLeft = GetAccentCol(IM_COL32(0, 190, 255, 255));
    ImU32 accentColRight = GetColorWithAlpha(IM_COL32(130, 0, 255, 0));
    dl->AddRectFilledMultiColor(
        ImVec2(pos.x + 12.0f * scale, pos.y + 28.0f * scale),
        ImVec2(pos.x + size.x - 12.0f * scale, pos.y + 30.0f * scale),
        accentColLeft, accentColRight, accentColRight, accentColLeft
    );

    float curY = pos.y + paddingHeader;

    auto renderPlayerRow = [&](const ScoreboardEntry& p, int rankIndex, bool isLocal = false) {
        if (curY + rowH > pos.y + size.y) return;

        if (isLocal) {
            dl->AddRectFilled(ImVec2(pos.x + 8.0f * scale, curY + 1.0f * scale), ImVec2(pos.x + size.x - 8.0f * scale, curY + rowH - 1.0f * scale), GetColorWithAlpha(IM_COL32(0, 190, 255, 25)), 4.0f * scale);
        }

        float dotRadius = 4.0f * scale;
        ImVec2 dotCenter(pos.x + 20.0f * scale, curY + rowH * 0.5f);
        ImU32 dotCol = p.alive ? p.color : IM_COL32(80, 80, 80, 255);
        dl->AddCircleFilled(dotCenter, dotRadius, GetColorWithAlpha(dotCol));
        
        char rankBuf[16];
        snprintf(rankBuf, sizeof(rankBuf), "%d.", rankIndex + 1);
        ImVec2 rankSz = CalcArmaTextSize(rankBuf, scale);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + 30.0f * scale, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(IM_COL32(150, 150, 160, 255)), rankBuf);

        ImVec2 namePos(pos.x + 35.0f * scale + rankSz.x, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f);
        float alphaMult = p.alive ? 1.0f : 0.5f;
        RenderArmaText(dl, namePos, GetColorWithAlpha(p.color, alphaMult), p.name.c_str(), scale, m_Alpha * alphaMult);

        char scoreBuf[32];
        if (sg_modScoreboardShowPing) {
            snprintf(scoreBuf, sizeof(scoreBuf), "%d (%dms)", p.score, p.ping);
        } else {
            snprintf(scoreBuf, sizeof(scoreBuf), "%d", p.score);
        }
        ImVec2 scoreSz = CalcArmaTextSize(scoreBuf, scale);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + size.x - 15.0f * scale - scoreSz.x, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(p.alive ? IM_COL32(255, 255, 255, 255) : IM_COL32(130, 130, 130, 255)), scoreBuf);

        curY += rowH;
    };

    if (isTeamGame && sg_modScoreboardShowTeams) {
        int globalRank = 0;
        for (const auto& t : teams) {
            if (curY + rowH > pos.y + size.y) break;

            dl->AddRectFilled(ImVec2(pos.x + 10.0f * scale, curY + 2.0f * scale), ImVec2(pos.x + size.x - 10.0f * scale, curY + rowH - 2.0f * scale), GetColorWithAlpha(t.color & 0x00FFFFFF | 0x25000000), 4.0f * scale);
            dl->AddRectFilled(ImVec2(pos.x + 12.0f * scale, curY + 4.0f * scale), ImVec2(pos.x + 15.0f * scale, curY + rowH - 4.0f * scale), GetColorWithAlpha(t.color), 1.5f * scale);

            RenderArmaText(dl, ImVec2(pos.x + 22.0f * scale, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetColorWithAlpha(t.color), t.name.c_str(), scale, m_Alpha);

            char teamScoreBuf[32];
            snprintf(teamScoreBuf, sizeof(teamScoreBuf), "%d pts", t.score);
            ImVec2 tsSz = CalcArmaTextSize(teamScoreBuf, scale);
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + size.x - 15.0f * scale - tsSz.x, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(IM_COL32(230, 230, 240, 255)), teamScoreBuf);

            curY += rowH;

            if (sg_modScoreboardShowPlayers) {
                int pCount = std::min((int)t.players.size(), sg_modScoreboardMaxPlayers);
                for (int j = 0; j < pCount; j++) {
                    const auto& p = t.players[j];
                    bool isLocal = (localNetID && p.playerNetID == localNetID);
                    renderPlayerRow(p, globalRank++, isLocal);
                }
            }
        }
    } else {
        int pCount = std::min((int)ffaPlayers.size(), sg_modScoreboardMaxPlayers);
        for (int i = 0; i < pCount; i++) {
            const auto& p = ffaPlayers[i];
            bool isLocal = (localNetID && p.playerNetID == localNetID);
            renderPlayerRow(p, i, isLocal);
        }

        bool localPlayerFound = false;
        if (localNetID) {
            for (int i = 0; i < pCount; i++) {
                if (ffaPlayers[i].playerNetID == localNetID) {
                    localPlayerFound = true;
                    break;
                }
            }
            if (!localPlayerFound && ffaPlayers.size() > (size_t)sg_modScoreboardMaxPlayers) {
                int localIdx = -1;
                for (size_t i = 0; i < ffaPlayers.size(); i++) {
                    if (ffaPlayers[i].playerNetID == localNetID) {
                        localIdx = (int)i;
                        break;
                    }
                }
                if (localIdx != -1) {
                    if (curY + rowH <= pos.y + size.y) {
                        ImVec2 dotsSz = CalcArmaTextSize("...", scale);
                        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pos.x + size.x * 0.5f - dotsSz.x * 0.5f, curY + (rowH - ImGui::GetFontSize() * scale) * 0.5f), GetTextCol(IM_COL32(110, 110, 125, 255)), "...");
                        curY += rowH;
                    }
                    renderPlayerRow(ffaPlayers[localIdx], localIdx, true);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------
// ZoneTimerWidget Implementation
// ---------------------------------------------------------------------
ZoneTimerWidget::ZoneTimerWidget()
    : HudWidget("ZoneTimer", ImVec2(350.0f, 100.0f), ImVec2(120.0f, 38.0f), WIDGET_CONFIG_PASS(sg_hudZoneTimer)) {
}

void ZoneTimerWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    ImVec2 drawPos = pos + m_SlideOffset;

    float bestTime = 9999.0f;
    float bestRad = 0.0f;
    float bestSpeed = 0.0f;
    int zoneCount = 0;

    eGrid* grid = eGrid::CurrentGrid();
    if (grid) {
        const tList<eGameObject>& gameObjects = grid->GameObjects();
        for (int j = gameObjects.Len() - 1; j >= 0; j--) {
            gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
            if (zone) {
                zoneCount++;
                float zrad = zone->GetRadius();
                float zspeed = zone->GetExpansionSpeed();
                if (zrad > 0.0f) {
                    if (zrad > bestRad) {
                        bestRad = zrad;
                        bestSpeed = zspeed;
                    }
                    if (zspeed < -0.001f) {
                        float timeLeft = zrad / -zspeed;
                        if (timeLeft < bestTime) bestTime = timeLeft;
                    }
                }
            }
        }
    }

    if (zoneCount == 0 && !isHudEditing) {
        return;
    }

    if (zoneCount == 0 && isHudEditing) {
        bestTime = 15.0f;
        bestRad = 150.0f;
        bestSpeed = -10.0f;
        zoneCount = 1;
    }

    // How long before returning to the zone stops being enough to save it.
    //
    // Leaving winds the zone up and it keeps shrinking for a while after you
    // come back - the wind-down is not instant. So there is a moment past
    // which the radius still to be lost during that wind-down is more than
    // the radius left, and after it the zone collapses whatever you do.
    //
    // Nothing tells a client how fast the wind-down is, so it is measured:
    // whenever the shrink slows, that is the recovery happening, and the rate
    // it slows at is the number wanted. Until it has been seen once there is
    // nothing honest to show.
    static float learnedRecovery = 0.0f;
    static float lastSpeed = 0.0f;
    static double lastWhen = 0.0;
    float doomed = -1.0f;

    {
        double now = tSysTimeFloat();
        float dt = (float)(now - lastWhen);

        if (lastWhen > 0.0 && dt > 0.005f && dt < 1.0f && bestSpeed < -0.001f)
        {
            float slowing = (bestSpeed - lastSpeed) / dt;   // positive while recovering
            if (slowing > 0.05f)
                learnedRecovery = learnedRecovery > 0.0f
                                ? learnedRecovery * 0.85f + slowing * 0.15f
                                : slowing;
        }

        lastSpeed = bestSpeed;
        lastWhen = now;

        if (learnedRecovery > 0.01f && bestSpeed < -0.001f && bestRad > 0.0f)
        {
            // what the zone still loses while it winds down from this speed
            float coasting = (bestSpeed * bestSpeed) / (2.0f * learnedRecovery);
            float spare = bestRad - coasting;
            doomed = spare > 0.0f ? spare / -bestSpeed : 0.0f;
        }
    }

    char timeStr[64];
    ImU32 textColor = IM_COL32(50, 255, 100, 255);
    bool critical = false;

    if (bestTime < 9999.0f) {
        snprintf(timeStr, sizeof(timeStr), "Zone: %.1fs", bestTime);
        if (bestTime < 3.0f) {
            textColor = IM_COL32(255, 30, 30, 255);
            critical = true;
        } else if (bestTime < 10.0f) {
            textColor = IM_COL32(255, 220, 0, 255);
        }
    } else if (bestRad > 0.0f) {
        snprintf(timeStr, sizeof(timeStr), "Zone R:%.0f S:%.1f", bestRad, bestSpeed);
    } else {
        snprintf(timeStr, sizeof(timeStr), "Zones: %d", zoneCount);
    }

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);

    float padding = 14.0f * scale;
    float textW = CalcTextSize(timeStr).x;
    m_Size.x = textW + padding * 2.0f + 24.0f * scale;
    m_Size.y = 38.0f * scale;

    ImU32 bgCol = GetBgCol();
    ImU32 borderCol = GetBorderCol();
    
    if (critical) {
        float pulse = (sinf((float)ImGui::GetTime() * 15.0f) + 1.0f) * 0.5f;
        borderCol = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(100 + pulse * 155)));
    }

    dl->AddRectFilled(drawPos, drawPos + m_Size, bgCol, 19.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, borderCol, 19.0f * scale, 0, critical ? 1.5f * scale : 1.0f * scale);

    float startY = drawPos.y + (m_Size.y - ImGui::GetFontSize() * scale) * 0.5f;
    float curX = drawPos.x + padding;
    
    if (critical) {
        float pulse = (sinf((float)ImGui::GetTime() * 15.0f) + 1.0f) * 0.5f;
        ImU32 dotColor = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(150 + pulse * 105)));
        dl->AddCircleFilled(ImVec2(curX + 6.0f * scale, drawPos.y + m_Size.y * 0.5f), 5.0f * scale, dotColor);
        curX += 18.0f * scale;
    } else {
        dl->AddCircleFilled(ImVec2(curX + 6.0f * scale, drawPos.y + m_Size.y * 0.5f), 4.0f * scale, GetAccentCol(IM_COL32(0, 190, 255, 255)));
        curX += 18.0f * scale;
    }

    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(curX, startY), GetTextCol(textColor), timeStr);

    if (doomed >= 0.0f)
    {
        char doomStr[64];
        if (doomed > 0.05f)
            snprintf(doomStr, sizeof(doomStr), "point of no return %.1fs", doomed);
        else
            snprintf(doomStr, sizeof(doomStr), "gone - cannot be saved");

        ImU32 doomCol = doomed > 3.0f ? IM_COL32(160, 170, 190, 220)
                      : ( doomed > 0.05f ? IM_COL32(255, 190, 0, 240)
                                         : IM_COL32(255, 40, 40, 255) );

        float tiny = ImGui::GetFontSize() * scale * 0.72f;
        float lineY = drawPos.y + m_Size.y + 3.0f * scale;
        dl->AddText(ImGui::GetFont(), tiny, ImVec2(drawPos.x + padding, lineY), GetColorWithAlpha(doomCol), doomStr);

        float doomW = CalcTextSize(doomStr).x * 0.72f + padding * 2.0f;
        if (doomW > m_Size.x) m_Size.x = doomW;
        m_Size.y += tiny + 5.0f * scale;
    }

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// RubberBatteryWidget Implementation
// ---------------------------------------------------------------------
RubberBatteryWidget::RubberBatteryWidget()
    : HudWidget("RubberBattery", ImVec2(370.0f, 440.0f), ImVec2(110.0f, 30.0f), WIDGET_CONFIG_PASS(sg_hudRubberBattery)) {
}

void RubberBatteryWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(110.0f * scale, 30.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    // Fetch rubber
    ePlayer* player = ePlayer::PlayerConfig(0);
    gCycle* h = (player && player->netPlayer) ? dynamic_cast<gCycle*>(player->netPlayer->Object()) : nullptr;
    
    float rubber = h ? (float)h->GetRubber() : 0.0f;
    float maxRubber = sg_rubberCycle;
    float pct = (maxRubber > 0.01f) ? rubber / maxRubber : 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    if (pct < 0.0f) pct = 0.0f;

    bool malusActive = gCycle::RubberMalusActive() && h && h->GetRubberMalus() > 0.01f;

    // Draw battery capsule body
    ImU32 bgCol = GetBgCol();
    ImU32 borderCol;
    if (malusActive) {
        float pulse = (sinf((float)ImGui::GetTime() * 12.0f) + 1.0f) * 0.5f;
        borderCol = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(120 + pulse * 135)));
    } else {
        borderCol = GetBorderCol();
    }
    
    // Draw background
    dl->AddRectFilled(drawPos, drawPos + size, bgCol, 6.0f * scale);
    dl->AddRect(drawPos, drawPos + size, borderCol, 6.0f * scale, 0, malusActive ? 1.5f * scale : 1.0f * scale);

    // Draw battery tip
    float tipW = 4.0f * scale;
    float tipH = 10.0f * scale;
    ImVec2 tipMin(drawPos.x + size.x, drawPos.y + (size.y - tipH) * 0.5f);
    ImVec2 tipMax(tipMin.x + tipW, tipMin.y + tipH);
    dl->AddRectFilled(tipMin, tipMax, borderCol, 2.0f * scale);

    // Draw battery charge bar (inner fill)
    float padding = 3.0f * scale;
    float fillMaxW = size.x - padding * 2.0f;
    float fillH = size.y - padding * 2.0f;
    float fillW = fillMaxW * pct;

    ImU32 fillCol;
    if (malusActive) {
        fillCol = IM_COL32(255, 30, 30, 255);
    } else {
        if (UseCustomColors() || GetRgbMode()) {
            fillCol = GetAccentCol();
        } else {
            int r = (int)(pct < 0.5f ? (pct * 2.0f * 255.0f) : 255.0f);
            int g = (int)(pct > 0.5f ? ((1.0f - pct) * 2.0f * 255.0f) : 255.0f);
            fillCol = IM_COL32(r, g, 0, 255);
        }
    }

    if (fillW > 0.5f) {
        ImVec2 fillMin(drawPos.x + padding, drawPos.y + padding);
        ImVec2 fillMax(fillMin.x + fillW, fillMin.y + fillH);
        dl->AddRectFilled(fillMin, fillMax, GetColorWithAlpha(fillCol), 4.0f * scale);
    }

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    
    char batText[32];
    snprintf(batText, sizeof(batText), "%.0f%%", pct * 100.0f);
    float textW = CalcTextSize(batText).x;
    ImVec2 textPos(drawPos.x + (size.x - textW) * 0.5f, drawPos.y + (size.y - ImGui::GetFontSize() * scale) * 0.5f);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, textPos, GetTextCol(IM_COL32(255, 255, 255, 255)), batText);

    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// ClassicRubberBatteryWidget Implementation
// ---------------------------------------------------------------------
ClassicRubberBatteryWidget::ClassicRubberBatteryWidget()
    : HudWidget("ClassicRubberBattery", ImVec2(200.0f, 520.0f), ImVec2(400.0f, 24.0f), WIDGET_CONFIG_PASS(sg_hudClassicRubberBattery)) {
}

void ClassicRubberBatteryWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    m_Size = ImVec2(400.0f * scale, 24.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    // Fetch rubber
    ePlayer* player = ePlayer::PlayerConfig(0);
    gCycle* h = (player && player->netPlayer) ? dynamic_cast<gCycle*>(player->netPlayer->Object()) : nullptr;
    
    float rubber = h ? (float)h->GetRubber() : 0.0f;
    float maxRubber = sg_rubberCycle;
    float pct = (maxRubber > 0.01f) ? rubber / maxRubber : 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    if (pct < 0.0f) pct = 0.0f;

    bool malusActive = gCycle::RubberMalusActive() && h && h->GetRubberMalus() > 0.01f;

    // Draw battery capsule body
    ImU32 bgCol = GetBgCol();
    ImU32 borderCol;
    if (malusActive) {
        float pulse = (sinf((float)ImGui::GetTime() * 12.0f) + 1.0f) * 0.5f;
        borderCol = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(120 + pulse * 135)));
    } else {
        borderCol = GetBorderCol();
    }
    
    // Draw background (classic flat/square)
    dl->AddRectFilled(drawPos, drawPos + size, bgCol, 0.0f);
    dl->AddRect(drawPos, drawPos + size, borderCol, 0.0f, 0, malusActive ? 1.5f * scale : 1.0f * scale);

    // Draw battery charge bar (inner fill)
    float fillW = size.x * pct;

    ImU32 fillCol;
    if (malusActive) {
        fillCol = IM_COL32(255, 30, 30, 255);
    } else {
        if (UseCustomColors() || GetRgbMode()) {
            fillCol = GetAccentCol();
        } else {
            int r = (int)(pct < 0.5f ? (pct * 2.0f * 255.0f) : 255.0f);
            int g = (int)(pct > 0.5f ? ((1.0f - pct) * 2.0f * 255.0f) : 255.0f);
            fillCol = IM_COL32(r, g, 0, 255);
        }
    }

    if (fillW > 0.5f) {
        ImVec2 fillMin(drawPos.x, drawPos.y);
        ImVec2 fillMax(drawPos.x + fillW, drawPos.y + size.y);
        dl->AddRectFilled(fillMin, fillMax, GetColorWithAlpha(fillCol), 0.0f);
    }
}

bool IsValidUtf8(const std::string& str) {
    int i = 0;
    int len = str.length();
    while (i < len) {
        unsigned char c = str[i];
        int bytes = 0;
        if (c < 0x80) { i++; continue; }
        else if ((c & 0xE0) == 0xC0) bytes = 2;
        else if ((c & 0xF0) == 0xE0) bytes = 3;
        else if ((c & 0xF8) == 0xF0) bytes = 4;
        else return false;
        
        if (i + bytes > len) return false;
        for (int j = 1; j < bytes; j++) {
            if (((unsigned char)str[i + j] & 0xC0) != 0x80) return false;
        }
        i += bytes;
    }
    return true;
}

std::string Cp1251ToUtf8(const std::string& cp1251) {
    std::string utf8;
    for (unsigned char c : cp1251) {
        unsigned int codepoint = c;
        if (c >= 0x80) {
            if (c >= 0xC0 && c <= 0xFF) {
                codepoint = 0x0410 + (c - 0xC0);
            } else {
                switch (c) {
                    case 0xA8: codepoint = 0x0401; break; // Ё
                    case 0xB8: codepoint = 0x0451; break; // ё
                    case 0x80: codepoint = 0x0402; break; // Ђ
                    case 0x90: codepoint = 0x0452; break; // ђ
                    case 0x81: codepoint = 0x0403; break; // Ѓ
                    case 0x83: codepoint = 0x0453; break; // ѓ
                    case 0xAA: codepoint = 0x0404; break; // Є
                    case 0xBA: codepoint = 0x0454; break; // є
                    case 0xBD: codepoint = 0x0405; break; // Ѕ
                    case 0xBE: codepoint = 0x0455; break; // ѕ
                    case 0xB2: codepoint = 0x0406; break; // І
                    case 0xB3: codepoint = 0x0456; break; // і
                    case 0xAF: codepoint = 0x0407; break; // Ї
                    case 0xBF: codepoint = 0x0457; break; // ї
                    case 0xA3: codepoint = 0x0408; break; // Ј
                    case 0xBC: codepoint = 0x0458; break; // ј
                    case 0x8A: codepoint = 0x0409; break; // Љ
                    case 0x9A: codepoint = 0x0459; break; // љ
                    case 0x8C: codepoint = 0x040A; break; // Њ
                    case 0x9C: codepoint = 0x045A; break; // њ
                    case 0x8E: codepoint = 0x040B; break; // Ћ
                    case 0x9E: codepoint = 0x045B; break; // ћ
                    case 0x8D: codepoint = 0x040C; break; // Ќ
                    case 0x9D: codepoint = 0x045C; break; // ќ
                    case 0xA1: codepoint = 0x040E; break; // Ў
                    case 0xA2: codepoint = 0x045E; break; // ў
                    case 0x8F: codepoint = 0x040F; break; // Џ
                    case 0x9F: codepoint = 0x045F; break; // џ
                    case 0xA5: codepoint = 0x0490; break; // Ґ
                    case 0xB4: codepoint = 0x0491; break; // ґ
                    case 0x97: codepoint = 0x2014; break;
                    case 0x96: codepoint = 0x2013; break;
                    case 0x84: codepoint = 0x201E; break;
                    case 0x93: codepoint = 0x201C; break;
                    case 0x94: codepoint = 0x201D; break;
                    case 0x91: codepoint = 0x2018; break;
                    case 0x92: codepoint = 0x2019; break;
                    case 0x82: codepoint = 0x201A; break;
                    case 0x8B: codepoint = 0x2039; break;
                    case 0x9B: codepoint = 0x203A; break;
                    case 0x95: codepoint = 0x2022; break;
                    case 0x85: codepoint = 0x2026; break;
                    case 0x86: codepoint = 0x2020; break;
                    case 0x87: codepoint = 0x2021; break;
                    case 0x99: codepoint = 0x2122; break;
                    case 0x89: codepoint = 0x2030; break;
                    case 0x88: codepoint = 0x20AC; break;
                    case 0xB9: codepoint = 0x2116; break;
                    case 0xA0: codepoint = 0x00A0; break;
                    default: codepoint = c; break;
                }
            }
        }
        
        // Encode codepoint to UTF-8
        if (codepoint < 0x80) {
            utf8 += (char)codepoint;
        } else if (codepoint < 0x800) {
            utf8 += (char)(0xC0 | (codepoint >> 6));
            utf8 += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint < 0x10000) {
            utf8 += (char)(0xE0 | (codepoint >> 12));
            utf8 += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            utf8 += (char)(0x80 | (codepoint & 0x3F));
        } else {
            utf8 += (char)(0xF0 | (codepoint >> 18));
            utf8 += (char)(0x80 | ((codepoint >> 12) & 0x3F));
            utf8 += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            utf8 += (char)(0x80 | (codepoint & 0x3F));
        }
    }
    return utf8;
}

std::string ConvertNonUtf8ToUtf8(const std::string& str) {
    if (IsValidUtf8(str)) {
        return str;
    }
    return Cp1251ToUtf8(str);
}

tString Utf8ToCp1251(const char *utf8) {
    tString result;
    if (!utf8) return result;
    int len = strlen(utf8);
    for (int i = 0; i < len; ) {
        unsigned char c = (unsigned char)utf8[i];
        unsigned int codepoint = 0;
        int bytes = 0;
        if (c < 0x80) {
            codepoint = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = c & 0x1F;
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = c & 0x0F;
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            codepoint = c & 0x07;
            bytes = 4;
        } else {
            codepoint = c;
            bytes = 1;
        }
        
        if (i + bytes > len) {
            result << (char)c;
            i++;
            continue;
        }
        
        for (int j = 1; j < bytes; j++) {
            unsigned char next = (unsigned char)utf8[i + j];
            if ((next & 0xC0) == 0x80) {
                codepoint = (codepoint << 6) | (next & 0x3F);
            } else {
                codepoint = c;
                bytes = 1;
                break;
            }
        }
        
        i += bytes;
        
        if (codepoint < 128) {
            result << (char)codepoint;
        } else if (codepoint >= 0x0410 && codepoint <= 0x044F) {
            result << (char)(codepoint - 0x0410 + 0xC0);
        } else if (codepoint == 0x0401) {
            result << (char)0xA8;
        } else if (codepoint == 0x0451) {
            result << (char)0xB8;
        } else if (codepoint == 0x0402) {
            result << (char)0x80;
        } else if (codepoint == 0x0452) {
            result << (char)0x90;
        } else if (codepoint == 0x0403) {
            result << (char)0x81;
        } else if (codepoint == 0x0453) {
            result << (char)0x83;
        } else if (codepoint == 0x0404) {
            result << (char)0xAA;
        } else if (codepoint == 0x0454) {
            result << (char)0xBA;
        } else if (codepoint == 0x0405) {
            result << (char)0xBD;
        } else if (codepoint == 0x0455) {
            result << (char)0xBE;
        } else if (codepoint == 0x0406) {
            result << (char)0xB2;
        } else if (codepoint == 0x0456) {
            result << (char)0xB3;
        } else if (codepoint == 0x0407) {
            result << (char)0xAF;
        } else if (codepoint == 0x0457) {
            result << (char)0xBF;
        } else if (codepoint == 0x0408) {
            result << (char)0xA3;
        } else if (codepoint == 0x0458) {
            result << (char)0xBC;
        } else if (codepoint == 0x0409) {
            result << (char)0x8A;
        } else if (codepoint == 0x0459) {
            result << (char)0x9A;
        } else if (codepoint == 0x040A) {
            result << (char)0x8C;
        } else if (codepoint == 0x045A) {
            result << (char)0x9C;
        } else if (codepoint == 0x040B) {
            result << (char)0x8E;
        } else if (codepoint == 0x045B) {
            result << (char)0x9E;
        } else if (codepoint == 0x040C) {
            result << (char)0x8D;
        } else if (codepoint == 0x045C) {
            result << (char)0x9D;
        } else if (codepoint == 0x040E) {
            result << (char)0xA1;
        } else if (codepoint == 0x045E) {
            result << (char)0xA2;
        } else if (codepoint == 0x040F) {
            result << (char)0x8F;
        } else if (codepoint == 0x045F) {
            result << (char)0x9F;
        } else if (codepoint == 0x0490) {
            result << (char)0xA5;
        } else if (codepoint == 0x0491) {
            result << (char)0xB4;
        } else if (codepoint == 0x2014) {
            result << (char)0x97;
        } else if (codepoint == 0x2013) {
            result << (char)0x96;
        } else if (codepoint == 0x201E) {
            result << (char)0x84;
        } else if (codepoint == 0x201C) {
            result << (char)0x93;
        } else if (codepoint == 0x201D) {
            result << (char)0x94;
        } else if (codepoint == 0x2018) {
            result << (char)0x91;
        } else if (codepoint == 0x2019) {
            result << (char)0x92;
        } else if (codepoint == 0x201A) {
            result << (char)0x82;
        } else if (codepoint == 0x2039) {
            result << (char)0x8B;
        } else if (codepoint == 0x203A) {
            result << (char)0x9B;
        } else if (codepoint == 0x2022) {
            result << (char)0x95;
        } else if (codepoint == 0x2026) {
            result << (char)0x85;
        } else if (codepoint == 0x2020) {
            result << (char)0x86;
        } else if (codepoint == 0x2021) {
            result << (char)0x87;
        } else if (codepoint == 0x2122) {
            result << (char)0x99;
        } else if (codepoint == 0x2030) {
            result << (char)0x89;
        } else if (codepoint == 0x20AC) {
            result << (char)0x88;
        } else if (codepoint == 0x2116) {
            result << (char)0xB9;
        } else if (codepoint == 0x00A0) {
            result << (char)0xA0;
        } else if (codepoint >= 128 && codepoint < 256) {
            result << (char)codepoint;
        } else {
            result << '?';
        }
    }
    return result;
}


// =====================================================================
// MinimapWidget Implementation
// =====================================================================

struct DeathEffect {
    eCoord position;
    float timeOfDeath;
    ImU32 color;
};
static std::map<unsigned short, DeathEffect> g_deathEffects;
static std::map<unsigned short, bool> g_prevAliveStates;

static bool ClipLineToCircle(ImVec2& p0, ImVec2& p1, float R) {
    ImVec2 d = ImVec2(p1.x - p0.x, p1.y - p0.y);
    float A = d.x * d.x + d.y * d.y;
    if (A < 0.0001f) {
        return (p0.x * p0.x + p0.y * p0.y <= R * R);
    }
    float B = 2.0f * (p0.x * d.x + p0.y * d.y);
    float C = p0.x * p0.x + p0.y * p0.y - R * R;
    float disc = B * B - 4.0f * A * C;
    if (disc < 0) return false;
    float t0 = (-B - sqrtf(disc)) / (2.0f * A);
    float t1 = (-B + sqrtf(disc)) / (2.0f * A);
    if (t0 > 1.0f || t1 < 0.0f) return false;
    float t_start = std::max(0.0f, t0);
    float t_end = std::min(1.0f, t1);
    if (t_start > t_end) return false;
    ImVec2 np0 = ImVec2(p0.x + d.x * t_start, p0.y + d.y * t_start);
    ImVec2 np1 = ImVec2(p0.x + d.x * t_end, p0.y + d.y * t_end);
    p0 = np0;
    p1 = np1;
    return true;
}

MinimapWidget::MinimapWidget()
    : HudWidget("Minimap", ImVec2(1000.0f, 500.0f), ImVec2(200.0f, 200.0f), WIDGET_CONFIG_PASS(sg_hudMinimap)), m_CurrentAngle(0.0f) {
}

void MinimapWidget::Update(float dt) {
    HudWidget::Update(dt);
}

void MinimapWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    float scale = GetScale();
    
    extern REAL sg_modMinimapZoom;
    extern bool sg_modMinimapRotate;
    extern int sg_modMinimapShape;
    extern bool sg_modMinimapAutoZoom;
    extern bool sg_modMinimapDeadEffect;

    m_Size = ImVec2(200.0f * scale, 200.0f * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;
    ImVec2 mapCenter = drawPos + size * 0.5f;
    float R_map = size.x * 0.5f;

    const eRectangle &bounds = eWallRim::GetBounds();
    eCoord arenaCenter;
    arenaCenter.x = (bounds.GetLow().x + bounds.GetHigh().x) * 0.5f;
    arenaCenter.y = (bounds.GetLow().y + bounds.GetHigh().y) * 0.5f;
    float arena_w = bounds.GetHigh().x - bounds.GetLow().x;
    float arena_h = bounds.GetHigh().y - bounds.GetLow().y;
    float maxArenaRadius = std::max(arena_w, arena_h) * 0.5f;
    if (maxArenaRadius < 10.0f) maxArenaRadius = 10.0f;

    gCycle* localPlayerCycle = nullptr;
    extern int sn_myNetID;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID* p = se_PlayerNetIDs(i);
        if (p && p->Owner() == sn_myNetID && p->Object()) {
            localPlayerCycle = dynamic_cast<gCycle*>(p->Object());
            break;
        }
    }

    if (sg_modMinimapDeadEffect) {
        for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
            ePlayerNetID *p = se_PlayerNetIDs(i);
            if (p) {
                unsigned short pid = p->pID;
                bool isAlive = rc_PlayerIsAlive( p );
                bool wasAlive = g_prevAliveStates[pid];
                if (wasAlive && !isAlive) {
                    DeathEffect effect;
                    effect.position = p->Object() ? p->Object()->Position() : eCoord(0.0f, 0.0f);
                    effect.timeOfDeath = (float)ImGui::GetTime();
                    
                    gRealColor col;
                    col.r = 1.0f; col.g = 0.1f; col.b = 0.1f;
                    gCycle* cycle = dynamic_cast<gCycle*>(p->Object());
                    if (cycle) {
                        col = cycle->color_;
                    }
                    effect.color = ImGui::ColorConvertFloat4ToU32(ImVec4(col.r, col.g, col.b, 1.0f));
                    g_deathEffects[pid] = effect;
                }
                g_prevAliveStates[pid] = isAlive;
            }
        }
    }

    float minZoneRadius = 999999.0f;
    eCoord zoneCenter(0.0f, 0.0f);
    bool foundSumoZone = false;
    
    if (localPlayerCycle && localPlayerCycle->Grid()) {
        const tList<eGameObject>& gameObjects = localPlayerCycle->Grid()->GameObjects();
        for (int j = gameObjects.Len() - 1; j >= 0; j--) {
            gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
            if (zone && zone->GetRadius() > 0.0f) {
                float zrad = zone->GetRadius();
                if (zrad < minZoneRadius) {
                    minZoneRadius = zrad;
                    zoneCenter = zone->GetPosition();
                    foundSumoZone = true;
                }
            }
        }
    }

    eCoord focusPoint = arenaCenter;
    if (sg_modMinimapAutoZoom && foundSumoZone) {
        focusPoint = zoneCenter;
    } else if (localPlayerCycle) {
        focusPoint = localPlayerCycle->Position();
    }

    float innerZoom = (float)sg_modMinimapZoom;
    if (innerZoom < 1.0f) innerZoom = 1.0f;

    float scaleFactor = 1.0f;
    if (sg_modMinimapAutoZoom && foundSumoZone) {
        scaleFactor = (R_map * 0.95f) / minZoneRadius;
        float baseScaleFactor = (R_map * 0.95f) / maxArenaRadius;
        if (scaleFactor < baseScaleFactor) {
            scaleFactor = baseScaleFactor;
        }
    } else {
        scaleFactor = ((R_map * 0.95f) / maxArenaRadius) * innerZoom;
    }

    extern REAL sg_modMinimapRotateSpeed;
    float rotCos = 1.0f, rotSin = 0.0f;
    if (sg_modMinimapRotate && localPlayerCycle) {
        eCoord dir = localPlayerCycle->Direction();
        float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.001f) {
            float targetAngle = atan2f(-dir.x, dir.y);
            float diff = targetAngle - m_CurrentAngle;
            while (diff < -3.14159265f) diff += 2.0f * 3.14159265f;
            while (diff > 3.14159265f) diff -= 2.0f * 3.14159265f;
            
            float dt = ImGui::GetIO().DeltaTime;
            float speed = (float)sg_modMinimapRotateSpeed;
            if (speed >= 30.0f) {
                m_CurrentAngle = targetAngle;
            } else {
                m_CurrentAngle += diff * dt * speed;
            }
            rotCos = cosf(m_CurrentAngle);
            rotSin = sinf(m_CurrentAngle);
        }
    } else {
        m_CurrentAngle = 0.0f;
    }

    auto WorldToScreen = [&](const eCoord& w) -> ImVec2 {
        float dx = w.x - focusPoint.x;
        float dy = w.y - focusPoint.y;
        float rx = dx * rotCos - dy * rotSin;
        float ry = dx * rotSin + dy * rotCos;
        float sx = mapCenter.x + rx * scaleFactor;
        float sy = mapCenter.y - ry * scaleFactor;
        return ImVec2(sx, sy);
    };

    if (sg_modMinimapShape == 0) {
        dl->AddCircleFilled(mapCenter, R_map, GetBgCol());
    } else {
        dl->AddRectFilled(drawPos, drawPos + size, GetBgCol(), 8.0f * scale);
    }

    dl->PushClipRect(drawPos, drawPos + size, true);

    // Draw Sci-Fi radar sweep & grid inside the clip rect
    if (sg_modMinimapShape == 0) {
        // Grid lines (3 concentric circles at 25%, 50%, 75% radius)
        dl->AddCircle(mapCenter, R_map * 0.25f, GetColorWithAlpha(IM_COL32(255, 255, 255, 18)), 32, 1.0f * scale);
        dl->AddCircle(mapCenter, R_map * 0.50f, GetColorWithAlpha(IM_COL32(255, 255, 255, 18)), 32, 1.0f * scale);
        dl->AddCircle(mapCenter, R_map * 0.75f, GetColorWithAlpha(IM_COL32(255, 255, 255, 18)), 32, 1.0f * scale);
        
        // Grid crosshairs
        dl->AddLine(mapCenter - ImVec2(R_map, 0.0f), mapCenter + ImVec2(R_map, 0.0f), GetColorWithAlpha(IM_COL32(255, 255, 255, 12)), 1.0f * scale);
        dl->AddLine(mapCenter - ImVec2(0.0f, R_map), mapCenter + ImVec2(0.0f, R_map), GetColorWithAlpha(IM_COL32(255, 255, 255, 12)), 1.0f * scale);
        
        // Radar Sweep rotation animation (1.8 rad/s)
        float sweepTime = (float)ImGui::GetTime();
        float sweepAngle = sweepTime * 1.8f;
        ImVec2 sweepPt = mapCenter + ImVec2(cosf(sweepAngle), sinf(sweepAngle)) * R_map;
        dl->AddLine(mapCenter, sweepPt, GetColorWithAlpha(IM_COL32(0, 190, 255, 100)), 1.5f * scale);
        
        // Draw trailing fade sectors
        int sectors = 12;
        for (int s = 0; s < sectors; s++) {
            float tailAngle = sweepAngle - (s + 1) * 0.06f;
            float alphaFactor = (1.0f - (float)s / sectors) * 0.12f;
            ImVec2 tailPt1 = mapCenter + ImVec2(cosf(tailAngle), sinf(tailAngle)) * R_map;
            ImVec2 tailPt2 = mapCenter + ImVec2(cosf(tailAngle - 0.06f), sinf(tailAngle - 0.06f)) * R_map;
            
            dl->AddTriangleFilled(mapCenter, tailPt1, tailPt2, GetColorWithAlpha(IM_COL32(0, 190, 255, (int)(alphaFactor * 255))));
        }
    } else {
        // Square grid lines (horizontal/vertical helper grids)
        float step = R_map * 0.5f;
        for (float dx = -R_map + step; dx < R_map; dx += step) {
            dl->AddLine(mapCenter + ImVec2(dx, -R_map), mapCenter + ImVec2(dx, R_map), GetColorWithAlpha(IM_COL32(255, 255, 255, 8)), 1.0f * scale);
            dl->AddLine(mapCenter + ImVec2(-R_map, dx), mapCenter + ImVec2(R_map, dx), GetColorWithAlpha(IM_COL32(255, 255, 255, 8)), 1.0f * scale);
        }
    }

    auto DrawClippedLine = [&](const ImVec2& p0_in, const ImVec2& p1_in, ImU32 col, float thickness) {
        ImVec2 p0 = p0_in;
        ImVec2 p1 = p1_in;
        if (sg_modMinimapShape == 0) {
            ImVec2 p0_rel = p0 - mapCenter;
            ImVec2 p1_rel = p1 - mapCenter;
            if (ClipLineToCircle(p0_rel, p1_rel, R_map)) {
                dl->AddLine(p0_rel + mapCenter, p1_rel + mapCenter, col, thickness);
            }
        } else {
            dl->AddLine(p0, p1, col, thickness);
        }
    };

    for (int i = se_rimWalls.Len() - 1; i >= 0; --i) {
        eWallRim *wall = se_rimWalls[i];
        if (wall) {
            ImVec2 p0 = WorldToScreen(wall->EndPoint(0));
            ImVec2 p1 = WorldToScreen(wall->EndPoint(1));
            DrawClippedLine(p0, p1, GetColorWithAlpha(IM_COL32(255, 255, 255, 180)), 2.0f * scale);
        }
    }

    if (localPlayerCycle && localPlayerCycle->Grid()) {
        const tList<eGameObject>& gameObjects = localPlayerCycle->Grid()->GameObjects();
        for (int j = gameObjects.Len() - 1; j >= 0; j--) {
            gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
            if (zone && zone->GetRadius() > 0.0f) {
                eCoord zpos = zone->GetPosition();
                float zrad = zone->GetRadius();
                gRealColor zcol = zone->GetColor();
                ImU32 borderCol = GetColorWithAlpha(IM_COL32((int)(zcol.r * 255), (int)(zcol.g * 255), (int)(zcol.b * 255), 200));
                
                int numSegments = 32;
                ImVec2 prevPt;
                for (int i = 0; i <= numSegments; i++) {
                    float ang = i * 2.0f * M_PI / numSegments;
                    eCoord wPt(zpos.x + cosf(ang) * zrad, zpos.y + sinf(ang) * zrad);
                    ImVec2 sPt = WorldToScreen(wPt);
                    if (i > 0) {
                        DrawClippedLine(prevPt, sPt, borderCol, 1.5f * scale);
                    }
                    prevPt = sPt;
                }
            }
        }
    }

    auto drawWall = [&](gNetPlayerWall *wall) {
        if (!wall) return;
        gCycle *cycle = wall->Cycle();
        if (!cycle) return;

        eCoord p0 = wall->EndPoint(0);
        eCoord p1 = wall->EndPoint(1);

        REAL wallBeg = wall->BegPos();
        REAL wallEnd = wall->EndPos();

        if (wallBeg > cycle->GetDistance()) return;

        float margin = 50.0f;
        if (p0.x < bounds.GetLow().x - margin || p0.x > bounds.GetHigh().x + margin ||
            p0.y < bounds.GetLow().y - margin || p0.y > bounds.GetHigh().y + margin ||
            p1.x < bounds.GetLow().x - margin || p1.x > bounds.GetHigh().x + margin ||
            p1.y < bounds.GetLow().y - margin || p1.y > bounds.GetHigh().y + margin) {
            return;
        }

        REAL maxLen = cycle->MaxWallsLength();
        if (maxLen > 0) {
            REAL visibleLen = cycle->ThisWallsLength();
            if (visibleLen <= 0.0f) return;
            REAL minVisibleDist = cycle->GetDistance() - visibleLen;
            if (wallEnd < minVisibleDist) return;

            if (wallBeg < minVisibleDist && wallEnd > wallBeg) {
                REAL t = (minVisibleDist - wallBeg) / (wallEnd - wallBeg);
                if (t > 0.0f && t < 1.0f) {
                    p0.x = p0.x + (p1.x - p0.x) * t;
                    p0.y = p0.y + (p1.y - p0.y) * t;
                }
            }
        }

        float alpha = cycle->Alive() ? 0.8f : 0.3f;
        ImU32 trailCol = GetColorWithAlpha(IM_COL32((int)(cycle->color_.r * 255), (int)(cycle->color_.g * 255), (int)(cycle->color_.b * 255), (int)(alpha * 255)));

        DrawClippedLine(WorldToScreen(p0), WorldToScreen(p1), trailCol, 2.0f * scale);
    };

    for (int i = sg_netPlayerWalls.Len() - 1; i >= 0; --i) {
        drawWall(sg_netPlayerWalls[i]);
    }
    for (int i = sg_netPlayerWallsGridded.Len() - 1; i >= 0; --i) {
        drawWall(sg_netPlayerWallsGridded[i]);
    }

    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i) {
        ePlayerNetID *p = se_PlayerNetIDs(i);
        if (rc_PlayerIsAlive( p )) {
            gCycle *cycle = dynamic_cast<gCycle*>(p->Object());
            if (cycle) {
                ImVec2 sPos = WorldToScreen(cycle->Position());
                
                if (sg_modMinimapShape == 0) {
                    float dist = sqrtf((sPos.x - mapCenter.x)*(sPos.x - mapCenter.x) + (sPos.y - mapCenter.y)*(sPos.y - mapCenter.y));
                    if (dist > R_map - 2.0f) {
                        continue;
                    }
                }
                
                ImU32 pCol = GetColorWithAlpha(IM_COL32((int)(cycle->color_.r * 255), (int)(cycle->color_.g * 255), (int)(cycle->color_.b * 255), 255));
                
                if (cycle == localPlayerCycle) {
                    eCoord dir = cycle->Direction();
                    float dirLen = sqrtf(dir.x*dir.x + dir.y*dir.y);
                    ImVec2 heading(0.0f, -1.0f);
                    if (!sg_modMinimapRotate && dirLen > 0.001f) {
                        heading = ImVec2(dir.x / dirLen, -dir.y / dirLen);
                    }
                    
                    float arrowSize = 6.0f * scale;
                    ImVec2 left(-heading.y, heading.x);
                    
                    ImVec2 pA = sPos + heading * arrowSize;
                    ImVec2 pB = sPos - heading * arrowSize * 0.6f + left * arrowSize * 0.5f;
                    ImVec2 pC = sPos - heading * arrowSize * 0.6f - left * arrowSize * 0.5f;
                    
                    dl->AddTriangleFilled(pA, pB, pC, IM_COL32(255, 255, 255, 255));
                    dl->AddTriangle(pA, pB, pC, IM_COL32(0, 0, 0, 200), 1.0f);
                } else {
                    dl->AddCircleFilled(sPos, 4.0f * scale, pCol);
                    dl->AddCircle(sPos, 4.0f * scale, IM_COL32(0, 0, 0, 220), 1.0f * scale);
                }
            }
        }
    }

    if (sg_modMinimapDeadEffect) {
        float curTime = (float)ImGui::GetTime();
        for (auto it = g_deathEffects.begin(); it != g_deathEffects.end(); ) {
            float elapsed = curTime - it->second.timeOfDeath;
            if (elapsed > 2.0f) {
                it = g_deathEffects.erase(it);
            } else {
                float progress = elapsed / 2.0f;
                float alpha = 1.0f - progress;
                float pulseRadius = (5.0f + progress * 25.0f) * scale;
                
                ImVec2 sPos = WorldToScreen(it->second.position);
                
                bool shouldDraw = true;
                if (sg_modMinimapShape == 0) {
                    float dist = sqrtf((sPos.x - mapCenter.x)*(sPos.x - mapCenter.x) + (sPos.y - mapCenter.y)*(sPos.y - mapCenter.y));
                    if (dist > R_map) {
                        shouldDraw = false;
                    }
                }
                
                if (shouldDraw) {
                    ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(it->second.color);
                    colorVec.w *= alpha * m_Alpha;
                    ImU32 col = ImGui::ColorConvertFloat4ToU32(colorVec);
                    
                    dl->AddCircle(sPos, pulseRadius, col, 24, 2.0f * scale);
                    
                    float crossSize = 6.0f * scale;
                    dl->AddLine(sPos - ImVec2(crossSize, crossSize), sPos + ImVec2(crossSize, crossSize), col, 2.0f);
                    dl->AddLine(sPos - ImVec2(-crossSize, crossSize), sPos + ImVec2(-crossSize, crossSize), col, 2.0f);
                    
                    // Draw the blinking & fading player dot at the death position
                    bool drawDot = true;
                    if (elapsed < 1.2f) {
                        drawDot = ((int)(elapsed * 12.0f) % 2 == 0);
                    }
                    if (drawDot) {
                        dl->AddCircleFilled(sPos, 4.0f * scale, col);
                        dl->AddCircle(sPos, 4.0f * scale, IM_COL32(0, 0, 0, (int)(alpha * m_Alpha * 220)), 1.0f * scale);
                    }
                }
                ++it;
            }
        }
    }

    dl->PopClipRect();

    if (sg_modMinimapShape == 0) {
        dl->AddCircle(mapCenter, R_map, GetBorderCol(), 2.0f * scale);
    } else {
        dl->AddRect(drawPos, drawPos + size, GetBorderCol(), 8.0f * scale, 0, 2.0f * scale);
    }
}

void MinimapWidget::DrawCustomSettings(bool& isDirty) {
    ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "Minimap Settings");
    
    extern REAL sg_modMinimapZoom;
    extern bool sg_modMinimapRotate;
    extern int sg_modMinimapShape;
    extern bool sg_modMinimapAutoZoom;
    extern bool sg_modMinimapDeadEffect;

    float zoomVal = (float)sg_modMinimapZoom;
    if (ImGui::SliderFloat("Zoom Level", &zoomVal, 1.0f, 10.0f, "%.1fx")) {
        sg_modMinimapZoom = (REAL)zoomVal;
        isDirty = true;
    }
    
    bool rotate = sg_modMinimapRotate;
    if (ImGui::Checkbox("Rotate Map", &rotate)) {
        sg_modMinimapRotate = rotate;
        isDirty = true;
    }
    
    const char* shapes[] = { "Circle", "Square" };
    int shape = sg_modMinimapShape;
    if (ImGui::Combo("Map Shape", &shape, shapes, 2)) {
        sg_modMinimapShape = shape;
        isDirty = true;
    }
    
    bool autoZoom = sg_modMinimapAutoZoom;
    if (ImGui::Checkbox("Sumo Auto-Zoom", &autoZoom)) {
        sg_modMinimapAutoZoom = autoZoom;
        isDirty = true;
    }
    
    bool deadEffect = sg_modMinimapDeadEffect;
    if (ImGui::Checkbox("Death Pulse Effect", &deadEffect)) {
        sg_modMinimapDeadEffect = deadEffect;
        isDirty = true;
    }
}

// ---------------------------------------------------------------------
// FortressAlertsWidget Implementation
// ---------------------------------------------------------------------
FortressAlertsWidget::FortressAlertsWidget()
    : HudWidget("FortressAlerts", ImVec2(20.0f, 140.0f), ImVec2(220.0f, 60.0f), WIDGET_CONFIG_PASS(sg_hudFortressAlerts)) {
}

void FortressAlertsWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    bool ourZoneAttacked = false;
    bool enemyZoneCapturing = false;
    float maxDefenseConquest = 0.0f;
    float maxOffenseConquest = 0.0f;

    ePlayerNetID* me = nullptr;
    ePlayer* lp = ePlayer::PlayerConfig(0);
    if (lp) me = lp->netPlayer;

    if (me && me->CurrentTeam() && me->Object() && me->Object()->Grid()) {
        eTeam* myTeam = me->CurrentTeam();
        eGrid* grid = me->Object()->Grid();
        const tList<eGameObject>& gameObjects = grid->GameObjects();

        float myR = myTeam->R() / 15.0f;
        float myG = myTeam->G() / 15.0f;
        float myB = myTeam->B() / 15.0f;

        for (int j = gameObjects.Len() - 1; j >= 0; j--) {
            gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
            if (!zone) continue;

            REAL rotSpeed = zone->GetRotationSpeed();
            REAL baseRot = 0.3f;
            if (rotSpeed <= baseRot + 0.05f) continue;

            REAL maxSpeed = 10.0f * (2.0f * M_PI) / 11.0f;
            REAL val = (rotSpeed - baseRot) / maxSpeed;
            if (val < 0.0f) val = 0.0f;
            REAL conquest = sqrtf(val);
            if (conquest > 1.0f) conquest = 1.0f;

            gRealColor const &zc = zone->GetColor();
            float dr = zc.r - myR, dg = zc.g - myG, db = zc.b - myB;
            float colorDist = dr*dr + dg*dg + db*db;

            if (colorDist < 0.3f) {
                ourZoneAttacked = true;
                if (conquest > maxDefenseConquest) maxDefenseConquest = conquest;
            } else {
                enemyZoneCapturing = true;
                if (conquest > maxOffenseConquest) maxOffenseConquest = conquest;
            }
        }
    }

    if (isHudEditing && !ourZoneAttacked && !enemyZoneCapturing) {
        ourZoneAttacked = true;
        maxDefenseConquest = 0.65f;
    }

    if (!ourZoneAttacked && !enemyZoneCapturing) return;

    float w = 280.0f * scale;
    float h = 60.0f * scale;
    m_Size = ImVec2(w, h);

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 8.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 8.0f * scale, 0, 1.0f);

    float padding = 12.0f * scale;
    float textY = drawPos.y + 10.0f * scale;
    float barY = drawPos.y + 36.0f * scale;
    float barH = 8.0f * scale;
    float barW = w - padding * 2.0f;

    float pulse = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 8.0f);

    if (ourZoneAttacked) {
        ImU32 textCol = GetColorWithAlpha(IM_COL32(255, 60, 60, 255));
        
        char pctBuf[32];
        snprintf(pctBuf, sizeof(pctBuf), "%d%%", (int)(maxDefenseConquest * 100.0f));

        bool pushedHeader = false;
        if (g_FontHeader) {
            ImGui::PushFont(g_FontHeader);
            pushedHeader = true;
        }
        float pctW = CalcTextSize(pctBuf).x;
        dl->PushClipRect(ImVec2(drawPos.x + padding, textY), ImVec2(drawPos.x + w - padding - pctW - 8.0f * scale, textY + 24.0f * scale), true);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), textCol, "DEFENDING ZONE");
        dl->PopClipRect();
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + w - padding - pctW, textY), GetTextCol(), pctBuf);
        if (pushedHeader) {
            ImGui::PopFont();
        }

        dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW, barY + barH), IM_COL32(50, 10, 10, 150), 4.0f * scale);
        ImU32 fillCol = GetColorWithAlpha(IM_COL32(255, 50, 50, (int)(180 + pulse * 75)));
        dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW * maxDefenseConquest, barY + barH), fillCol, 4.0f * scale);
    } else if (enemyZoneCapturing) {
        ImU32 textCol = GetColorWithAlpha(IM_COL32(50, 220, 100, 255));
        
        char pctBuf[32];
        snprintf(pctBuf, sizeof(pctBuf), "%d%%", (int)(maxOffenseConquest * 100.0f));

        bool pushedHeader = false;
        if (g_FontHeader) {
            ImGui::PushFont(g_FontHeader);
            pushedHeader = true;
        }
        float pctW = CalcTextSize(pctBuf).x;
        dl->PushClipRect(ImVec2(drawPos.x + padding, textY), ImVec2(drawPos.x + w - padding - pctW - 8.0f * scale, textY + 24.0f * scale), true);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), textCol, "CONQUERING ZONE");
        dl->PopClipRect();
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + w - padding - pctW, textY), GetTextCol(), pctBuf);
        if (pushedHeader) {
            ImGui::PopFont();
        }

        dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW, barY + barH), IM_COL32(10, 50, 20, 150), 4.0f * scale);
        ImU32 fillCol = GetColorWithAlpha(IM_COL32(50, 255, 100, (int)(180 + pulse * 75)));
        dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW * maxOffenseConquest, barY + barH), fillCol, 4.0f * scale);
    }
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------




// ---------------------------------------------------------------------
// ---------------------------------------------------------------------




// ---------------------------------------------------------------------
// TeammateDeathWarningWidget Implementation
// ---------------------------------------------------------------------
TeammateDeathWarningWidget::TeammateDeathWarningWidget()
    : HudWidget("TeammateDeath", ImVec2(20.0f, 350.0f), ImVec2(240.0f, 60.0f), WIDGET_CONFIG_PASS(sg_hudTeammateDeath)) {
}

extern REAL sg_teammateDeathFlashTime;
extern tString sg_deadTeammateName;
extern std::vector< tString > sg_teammateDown;

void TeammateDeathWarningWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    REAL elapsed = (REAL)tRealSysTimeFloat() - sg_teammateDeathFlashTime;
    bool active = (elapsed >= 0.0f && elapsed < 3.0f);

    // Everyone lost in the last three seconds, oldest first. Two going down
    // together is ordinary in fortress, and one line meant the second wrote
    // over the first before it had been read.
    std::vector< std::string > lost;
    if (active) {
        for (size_t i = 0; i < sg_teammateDown.size(); ++i) {
            std::string one = ConvertNonUtf8ToUtf8((const char*)sg_teammateDown[i]);
            if (!one.empty())
                lost.push_back(one);
        }
    }

    if (isHudEditing && lost.empty()) {
        active = true;
        lost.push_back("TeammateName");
    }

    if (!active || lost.empty()) return;

    std::string const & deadName = lost[0];

    float alertAlpha = 1.0f;
    if (elapsed > 2.0f) {
        alertAlpha = 1.0f - (elapsed - 2.0f);
    }
    if (isHudEditing) alertAlpha = 1.0f;

    float w = 240.0f * scale;
    float h = ( 36.0f + 24.0f * (float)lost.size() ) * scale;
    m_Size = ImVec2(w, h);

    ImU32 bgCol = GetColorWithAlpha(GetBgCol(), alertAlpha);
    ImU32 borderCol = GetColorWithAlpha(GetBorderCol(), alertAlpha);
    ImU32 textCol = GetColorWithAlpha(GetTextCol(), alertAlpha);

    dl->AddRectFilled(drawPos, drawPos + m_Size, bgCol, 8.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, borderCol, 8.0f * scale, 0, 1.0f);

    float padding = 12.0f * scale;
    float textY = drawPos.y + 10.0f * scale;
    float nameY = drawPos.y + 34.0f * scale;

    ImU32 headerCol = GetColorWithAlpha(IM_COL32(255, 60, 60, 255), alertAlpha);
    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), headerCol,
                lost.size() > 1 ? "TEAMMATES ELIMINATED" : "TEAMMATE ELIMINATED");
    if (g_FontHeader) ImGui::PopFont();

    for (size_t i = 0; i < lost.size(); ++i)
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale,
                    ImVec2(drawPos.x + padding, nameY + 24.0f * scale * (float)i),
                    textCol, lost[i].c_str());
}

// ---------------------------------------------------------------------
// WallTimerWidget Implementation
// ---------------------------------------------------------------------
WallTimerWidget::WallTimerWidget()
    : HudWidget("WallTimer", ImVec2(800.0f, 200.0f), ImVec2(180.0f, 40.0f), WIDGET_CONFIG_PASS(sg_hudWallTimer)) {
}

extern bool sg_corpseTimerOverride;
extern float sg_corpseTimerDuration;

void WallTimerWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    struct WallTimerEntry {
        std::string name;
        float timeLeft;
        ImU32 color;
    };
    std::vector<WallTimerEntry> activeTimers;

    REAL totalDelay = gCycle::WallsStayUpDelay();
    if (totalDelay < 0.0f || sg_corpseTimerOverride) {
        totalDelay = sg_corpseTimerDuration;
    }

    if (totalDelay >= 0.0f) {
        REAL gameTime = se_GameTime();
        for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
            ePlayerNetID *p = se_PlayerNetIDs(i);
            if (p) {
                gCycle *cycle = dynamic_cast<gCycle*>(p->Object());
                if (cycle && !cycle->Alive() && cycle->DeathTime() > 0.0f) {
                    REAL timeSinceDeath = gameTime - cycle->DeathTime();
                    REAL timeLeft = totalDelay - timeSinceDeath;
                    if (timeLeft > 0.0f) {
                        WallTimerEntry entry;
                        entry.name = ConvertNonUtf8ToUtf8(static_cast<const char*>(p->GetName()));
                        entry.timeLeft = timeLeft;
                        float r, g, b;
                        p->Color(r, g, b);
                        entry.color = IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), 255);
                        activeTimers.push_back(entry);
                    }
                }
            }
        }
    }

    if (activeTimers.empty() && isHudEditing) {
        WallTimerEntry dummy;
        dummy.name = "Player_Name";
        dummy.timeLeft = 5.4f;
        dummy.color = IM_COL32(0, 190, 255, 255);
        activeTimers.push_back(dummy);
    }

    if (activeTimers.empty()) {
        return;
    }

    // Sort active timers by time left ascending (closest to dissolve first)
    std::sort(activeTimers.begin(), activeTimers.end(), [](const WallTimerEntry& a, const WallTimerEntry& b) {
        return a.timeLeft < b.timeLeft;
    });

    float width = 250.0f * scale;
    float rowHeight = 24.0f * scale;
    float padding = 8.0f * scale;
    m_Size.x = width;
    m_Size.y = activeTimers.size() * rowHeight + padding * 2.0f;

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 8.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 8.0f * scale, 0, 1.5f * scale);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    for (size_t i = 0; i < activeTimers.size(); i++) {
        const auto& entry = activeTimers[i];
        ImVec2 rowPos = drawPos + ImVec2(padding, padding + i * rowHeight);

        // Timer bar
        float barWidth = 50.0f * scale;
        float barHeight = 5.0f * scale;
        ImVec2 barPos = drawPos + ImVec2(width - barWidth - padding, padding + i * rowHeight + (rowHeight - barHeight) * 0.5f);
        
        dl->AddRectFilled(barPos, barPos + ImVec2(barWidth, barHeight), IM_COL32(40, 40, 45, 255), 2.0f * scale);
        
        float pct = entry.timeLeft / totalDelay;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;
        
        ImU32 barCol = GetAccentCol();
        if (entry.timeLeft <= 1.5f) {
            float blink = sinf((float)ImGui::GetTime() * 15.0f);
            if (blink > 0.0f) {
                barCol = IM_COL32(255, 50, 50, 255);
            }
        }
        
        dl->AddRectFilled(barPos, barPos + ImVec2(barWidth * pct, barHeight), barCol, 2.0f * scale);

        // Timer text
        char timeText[16];
        snprintf(timeText, sizeof(timeText), "%.1fs", entry.timeLeft);
        ImVec2 txtSize = CalcTextSize(timeText);
        ImVec2 txtPos = ImVec2(barPos.x - txtSize.x - 6.0f * scale, rowPos.y);

        // Player Name (Clipped to prevent overlapping with timer/bar)
        dl->PushClipRect(rowPos - ImVec2(2.0f * scale, 2.0f * scale), ImVec2(txtPos.x - 6.0f * scale, rowPos.y + rowHeight), true);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, rowPos, entry.color, entry.name.c_str());
        dl->PopClipRect();

        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, txtPos, GetTextCol(), timeText);
    }
    if (g_FontHeader) ImGui::PopFont();
}

// ---------------------------------------------------------------------
// KeystrokeVisualizerWidget Implementation
// ---------------------------------------------------------------------
static ImU32 LerpCol(ImU32 c1, ImU32 c2, float t) {
    int r1 = (c1 >> 0) & 0xFF;
    int g1 = (c1 >> 8) & 0xFF;
    int b1 = (c1 >> 16) & 0xFF;
    int a1 = (c1 >> 24) & 0xFF;
    
    int r2 = (c2 >> 0) & 0xFF;
    int g2 = (c2 >> 8) & 0xFF;
    int b2 = (c2 >> 16) & 0xFF;
    int a2 = (c2 >> 24) & 0xFF;
    
    return IM_COL32(
        r1 + (int)((r2 - r1) * t),
        g1 + (int)((g2 - g1) * t),
        b1 + (int)((b2 - b1) * t),
        a1 + (int)((a2 - a1) * t)
    );
}



KeystrokeVisualizerWidget::KeystrokeVisualizerWidget(int id)
    : HudWidget(id == 1 ? "Keystroke 1" : "Keystroke 2",
                id == 1 ? ImVec2(500.0f, 600.0f) : ImVec2(550.0f, 600.0f),
                ImVec2(160.0f, 120.0f),
                id == 1 ? WIDGET_CONFIG_PASS(sg_hudKeystroke1) : WIDGET_CONFIG_PASS(sg_hudKeystroke2)),
      m_WidgetId(id) {
}

void KeystrokeVisualizerWidget::Update(float dt) {
    HudWidget::Update(dt);

    std::vector<ImGuiKey> keysToTrack = {
        ImGuiKey_Escape, ImGuiKey_F1, ImGuiKey_F2, ImGuiKey_F3, ImGuiKey_F4, ImGuiKey_F5, ImGuiKey_F6, ImGuiKey_F7, ImGuiKey_F8, ImGuiKey_F9, ImGuiKey_F10, ImGuiKey_F11, ImGuiKey_F12,
        ImGuiKey_GraveAccent, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9, ImGuiKey_0, ImGuiKey_Minus, ImGuiKey_Equal, ImGuiKey_Backspace,
        ImGuiKey_Tab, ImGuiKey_Q, ImGuiKey_W, ImGuiKey_E, ImGuiKey_R, ImGuiKey_T, ImGuiKey_Y, ImGuiKey_U, ImGuiKey_I, ImGuiKey_O, ImGuiKey_P, ImGuiKey_LeftBracket, ImGuiKey_RightBracket, ImGuiKey_Backslash,
        ImGuiKey_CapsLock, ImGuiKey_A, ImGuiKey_S, ImGuiKey_D, ImGuiKey_F, ImGuiKey_G, ImGuiKey_H, ImGuiKey_J, ImGuiKey_K, ImGuiKey_L, ImGuiKey_Semicolon, ImGuiKey_Apostrophe, ImGuiKey_Enter,
        ImGuiKey_LeftShift, ImGuiKey_Z, ImGuiKey_X, ImGuiKey_C, ImGuiKey_V, ImGuiKey_B, ImGuiKey_N, ImGuiKey_M, ImGuiKey_Comma, ImGuiKey_Period, ImGuiKey_Slash, ImGuiKey_RightShift,
        ImGuiKey_LeftCtrl, ImGuiKey_LeftSuper, ImGuiKey_LeftAlt, ImGuiKey_Space, ImGuiKey_RightAlt, ImGuiKey_RightSuper, ImGuiKey_Menu, ImGuiKey_RightCtrl,
        ImGuiKey_PrintScreen, ImGuiKey_ScrollLock, ImGuiKey_Pause, ImGuiKey_Insert, ImGuiKey_Home, ImGuiKey_PageUp, ImGuiKey_Delete, ImGuiKey_End, ImGuiKey_PageDown,
        ImGuiKey_UpArrow, ImGuiKey_LeftArrow, ImGuiKey_DownArrow, ImGuiKey_RightArrow,
        ImGuiKey_MouseLeft, ImGuiKey_MouseRight, ImGuiKey_MouseMiddle
    };

    for (ImGuiKey key : keysToTrack) {
        bool isDown = false;
        if (key == ImGuiKey_MouseLeft) {
            isDown = ImGui::IsMouseDown(0);
        } else if (key == ImGuiKey_MouseRight) {
            isDown = ImGui::IsMouseDown(1);
        } else if (key == ImGuiKey_MouseMiddle) {
            isDown = ImGui::IsMouseDown(2);
        } else {
            isDown = ImGui::IsKeyDown(key);
        }

        float& progress = m_KeyPressStates[key];
        if (isDown) {
            progress += dt * 15.0f;
            if (progress > 1.0f) progress = 1.0f;
        } else {
            progress -= dt * 8.0f;
            if (progress < 0.0f) progress = 0.0f;
        }
    }

    // CPS Tracker
    double currentTime = ImGui::GetTime();
    if (ImGui::IsMouseClicked(0)) {
        m_LmbClicks.push_back(currentTime);
    }
    if (ImGui::IsMouseClicked(1)) {
        m_RmbClicks.push_back(currentTime);
    }

    // Clean up old clicks
    std::vector<double> freshLmb;
    for (double t : m_LmbClicks) {
        if (currentTime - t <= 1.0) freshLmb.push_back(t);
    }
    m_LmbClicks = freshLmb;

    std::vector<double> freshRmb;
    for (double t : m_RmbClicks) {
        if (currentTime - t <= 1.0) freshRmb.push_back(t);
    }
    m_RmbClicks = freshRmb;
}

void KeystrokeVisualizerWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    struct KeyDef {
        std::string label;
        ImGuiKey key;
        float x, y;
        float w;
    };

    std::vector<KeyDef> keys;
    int preset = (m_WidgetId == 1) ? sg_modKeystroke1_Preset : sg_modKeystroke2_Preset;
    int mask0 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask0 : sg_modKeystroke2_Mask0;
    int mask1 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask1 : sg_modKeystroke2_Mask1;
    int mask2 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask2 : sg_modKeystroke2_Mask2;
    bool rgbWave = (m_WidgetId == 1) ? sg_modKeystroke1_RgbWave : sg_modKeystroke2_RgbWave;
    float rgbSpeed = (m_WidgetId == 1) ? sg_modKeystroke1_RgbSpeed : sg_modKeystroke2_RgbSpeed;
    float glowIntensityVal = (m_WidgetId == 1) ? sg_modKeystroke1_GlowIntensity : sg_modKeystroke2_GlowIntensity;
    bool separateKeys = (m_WidgetId == 1) ? sg_modKeystroke1_SeparateKeys : sg_modKeystroke2_SeparateKeys;
    float spacing = (m_WidgetId == 1) ? sg_modKeystroke1_Spacing : sg_modKeystroke2_Spacing;
    float radius = (m_WidgetId == 1) ? sg_modKeystroke1_Radius : sg_modKeystroke2_Radius;
    bool showCps = (m_WidgetId == 1) ? sg_modKeystroke1_ShowCps : sg_modKeystroke2_ShowCps;

    struct LayoutKey {
        int index;
        std::string label;
        ImGuiKey key;
        float gridX, gridY;
        float width;
    };

    std::vector<LayoutKey> masterLayout = {
        // Row 0: Esc, F1-F12, Print, Scroll, Pause
        {0, "Esc", ImGuiKey_Escape, 0.0f, 0.0f, 1.0f},
        {1, "F1", ImGuiKey_F1, 2.0f, 0.0f, 1.0f},
        {2, "F2", ImGuiKey_F2, 3.0f, 0.0f, 1.0f},
        {3, "F3", ImGuiKey_F3, 4.0f, 0.0f, 1.0f},
        {4, "F4", ImGuiKey_F4, 5.0f, 0.0f, 1.0f},
        {5, "F5", ImGuiKey_F5, 6.5f, 0.0f, 1.0f},
        {6, "F6", ImGuiKey_F6, 7.5f, 0.0f, 1.0f},
        {7, "F7", ImGuiKey_F7, 8.5f, 0.0f, 1.0f},
        {8, "F8", ImGuiKey_F8, 9.5f, 0.0f, 1.0f},
        {9, "F9", ImGuiKey_F9, 11.0f, 0.0f, 1.0f},
        {10, "F10", ImGuiKey_F10, 12.0f, 0.0f, 1.0f},
        {11, "F11", ImGuiKey_F11, 13.0f, 0.0f, 1.0f},
        {12, "F12", ImGuiKey_F12, 14.0f, 0.0f, 1.0f},
        {74, "Prt", ImGuiKey_PrintScreen, 15.5f, 0.0f, 1.0f},
        {75, "Scr", ImGuiKey_ScrollLock, 16.5f, 0.0f, 1.0f},
        {76, "Pau", ImGuiKey_Pause, 17.5f, 0.0f, 1.0f},

        // Row 1: Tilde, 1-0, -, =, Backspace, Insert, Home, PgUp
        {13, "`", ImGuiKey_GraveAccent, 0.0f, 1.0f, 1.0f},
        {14, "1", ImGuiKey_1, 1.0f, 1.0f, 1.0f},
        {15, "2", ImGuiKey_2, 2.0f, 1.0f, 1.0f},
        {16, "3", ImGuiKey_3, 3.0f, 1.0f, 1.0f},
        {17, "4", ImGuiKey_4, 4.0f, 1.0f, 1.0f},
        {18, "5", ImGuiKey_5, 5.0f, 1.0f, 1.0f},
        {19, "6", ImGuiKey_6, 6.0f, 1.0f, 1.0f},
        {20, "7", ImGuiKey_7, 7.0f, 1.0f, 1.0f},
        {21, "8", ImGuiKey_8, 8.0f, 1.0f, 1.0f},
        {22, "9", ImGuiKey_9, 9.0f, 1.0f, 1.0f},
        {23, "0", ImGuiKey_0, 10.0f, 1.0f, 1.0f},
        {24, "-", ImGuiKey_Minus, 11.0f, 1.0f, 1.0f},
        {25, "=", ImGuiKey_Equal, 12.0f, 1.0f, 1.0f},
        {26, "Bsp", ImGuiKey_Backspace, 13.0f, 1.0f, 2.0f},
        {77, "Ins", ImGuiKey_Insert, 15.5f, 1.0f, 1.0f},
        {78, "Hm", ImGuiKey_Home, 16.5f, 1.0f, 1.0f},
        {79, "Pup", ImGuiKey_PageUp, 17.5f, 1.0f, 1.0f},

        // Row 2: Tab, Q-P, [, ], \, Delete, End, PgDn
        {27, "Tab", ImGuiKey_Tab, 0.0f, 2.0f, 1.5f},
        {28, "Q", ImGuiKey_Q, 1.5f, 2.0f, 1.0f},
        {29, "W", ImGuiKey_W, 2.5f, 2.0f, 1.0f},
        {30, "E", ImGuiKey_E, 3.5f, 2.0f, 1.0f},
        {31, "R", ImGuiKey_R, 4.5f, 2.0f, 1.0f},
        {32, "T", ImGuiKey_T, 5.5f, 2.0f, 1.0f},
        {33, "Y", ImGuiKey_Y, 6.5f, 2.0f, 1.0f},
        {34, "U", ImGuiKey_U, 7.5f, 2.0f, 1.0f},
        {35, "I", ImGuiKey_I, 8.5f, 2.0f, 1.0f},
        {36, "O", ImGuiKey_O, 9.5f, 2.0f, 1.0f},
        {37, "P", ImGuiKey_P, 10.5f, 2.0f, 1.0f},
        {38, "[", ImGuiKey_LeftBracket, 11.5f, 2.0f, 1.0f},
        {39, "]", ImGuiKey_RightBracket, 12.5f, 2.0f, 1.0f},
        {40, "\\", ImGuiKey_Backslash, 13.5f, 2.0f, 1.5f},
        {80, "Del", ImGuiKey_Delete, 15.5f, 2.0f, 1.0f},
        {81, "End", ImGuiKey_End, 16.5f, 2.0f, 1.0f},
        {82, "Pdn", ImGuiKey_PageDown, 17.5f, 2.0f, 1.0f},

        // Row 3: Caps, A-L, ;, ', Enter
        {41, "Caps", ImGuiKey_CapsLock, 0.0f, 3.0f, 1.75f},
        {42, "A", ImGuiKey_A, 1.75f, 3.0f, 1.0f},
        {43, "S", ImGuiKey_S, 2.75f, 3.0f, 1.0f},
        {44, "D", ImGuiKey_D, 3.75f, 3.0f, 1.0f},
        {45, "F", ImGuiKey_F, 4.75f, 3.0f, 1.0f},
        {46, "G", ImGuiKey_G, 5.75f, 3.0f, 1.0f},
        {47, "H", ImGuiKey_H, 6.75f, 3.0f, 1.0f},
        {48, "J", ImGuiKey_J, 7.75f, 3.0f, 1.0f},
        {49, "K", ImGuiKey_K, 8.75f, 3.0f, 1.0f},
        {50, "L", ImGuiKey_L, 9.75f, 3.0f, 1.0f},
        {51, ";", ImGuiKey_Semicolon, 10.75f, 3.0f, 1.0f},
        {52, "'", ImGuiKey_Apostrophe, 11.75f, 3.0f, 1.0f},
        {53, "Ent", ImGuiKey_Enter, 12.75f, 3.0f, 2.25f},

        // Row 4: Shift L, Z-/, Shift R, Up
        {54, "ShfL", ImGuiKey_LeftShift, 0.0f, 4.0f, 2.25f},
        {55, "Z", ImGuiKey_Z, 2.25f, 4.0f, 1.0f},
        {56, "X", ImGuiKey_X, 3.25f, 4.0f, 1.0f},
        {57, "C", ImGuiKey_C, 4.25f, 4.0f, 1.0f},
        {58, "V", ImGuiKey_V, 5.25f, 4.0f, 1.0f},
        {59, "B", ImGuiKey_B, 6.25f, 4.0f, 1.0f},
        {60, "N", ImGuiKey_N, 7.25f, 4.0f, 1.0f},
        {61, "M", ImGuiKey_M, 8.25f, 4.0f, 1.0f},
        {62, ",", ImGuiKey_Comma, 9.25f, 4.0f, 1.0f},
        {63, ".", ImGuiKey_Period, 10.25f, 4.0f, 1.0f},
        {64, "/", ImGuiKey_Slash, 11.25f, 4.0f, 1.0f},
        {65, "ShfR", ImGuiKey_RightShift, 12.25f, 4.0f, 2.75f},
        {83, "^", ImGuiKey_UpArrow, 16.5f, 4.0f, 1.0f},

        // Row 5: Ctrl L, Win L, Alt L, Space, Alt R, Win R, Menu, Ctrl R, Left, Down, Right
        {66, "CtlL", ImGuiKey_LeftCtrl, 0.0f, 5.0f, 1.25f},
        {67, "WinL", ImGuiKey_LeftSuper, 1.25f, 5.0f, 1.25f},
        {68, "AltL", ImGuiKey_LeftAlt, 2.5f, 5.0f, 1.25f},
        {69, "Space", ImGuiKey_Space, 3.75f, 5.0f, 6.25f},
        {70, "AltR", ImGuiKey_RightAlt, 10.0f, 5.0f, 1.25f},
        {71, "WinR", ImGuiKey_RightSuper, 11.25f, 5.0f, 1.25f},
        {72, "Men", ImGuiKey_Menu, 12.5f, 5.0f, 1.25f},
        {73, "CtlR", ImGuiKey_RightCtrl, 13.75f, 5.0f, 1.25f},
        {84, "<", ImGuiKey_LeftArrow, 15.5f, 5.0f, 1.0f},
        {85, "v", ImGuiKey_DownArrow, 16.5f, 5.0f, 1.0f},
        {86, ">", ImGuiKey_RightArrow, 17.5f, 5.0f, 1.0f},

        // Row 6: LMB, MMB, RMB
        {87, "LMB", ImGuiKey_MouseLeft, 3.75f, 6.0f, 2.0f},
        {89, "MMB", ImGuiKey_MouseMiddle, 5.75f, 6.0f, 2.25f},
        {88, "RMB", ImGuiKey_MouseRight, 8.0f, 6.0f, 2.0f}
    };

    for (const auto& lk : masterLayout) {
        bool isEnabled = false;
        if (preset == 0) { // WASD
            isEnabled = (lk.key == ImGuiKey_W || lk.key == ImGuiKey_A || lk.key == ImGuiKey_S || lk.key == ImGuiKey_D);
        } else if (preset == 1) { // WASD + Space
            isEnabled = (lk.key == ImGuiKey_W || lk.key == ImGuiKey_A || lk.key == ImGuiKey_S || lk.key == ImGuiKey_D || lk.key == ImGuiKey_Space);
        } else if (preset == 2) { // WASD + Space + Shift
            isEnabled = (lk.key == ImGuiKey_W || lk.key == ImGuiKey_A || lk.key == ImGuiKey_S || lk.key == ImGuiKey_D || lk.key == ImGuiKey_Space || lk.key == ImGuiKey_LeftShift);
        } else if (preset == 3) { // WASD + Mouse
            isEnabled = (lk.key == ImGuiKey_W || lk.key == ImGuiKey_A || lk.key == ImGuiKey_S || lk.key == ImGuiKey_D || lk.key == ImGuiKey_Space || lk.key == ImGuiKey_LeftShift || lk.key == ImGuiKey_MouseLeft || lk.key == ImGuiKey_MouseRight);
        } else if (preset == 4) { // Arrows
            isEnabled = (lk.key == ImGuiKey_UpArrow || lk.key == ImGuiKey_LeftArrow || lk.key == ImGuiKey_DownArrow || lk.key == ImGuiKey_RightArrow);
        } else if (preset == 5) { // Arrows + Space
            isEnabled = (lk.key == ImGuiKey_UpArrow || lk.key == ImGuiKey_LeftArrow || lk.key == ImGuiKey_DownArrow || lk.key == ImGuiKey_RightArrow || lk.key == ImGuiKey_Space);
        } else { // Custom
            if (lk.index < 32) {
                isEnabled = (mask0 & (1 << lk.index)) != 0;
            } else if (lk.index < 64) {
                isEnabled = (mask1 & (1 << (lk.index - 32))) != 0;
            } else {
                isEnabled = (mask2 & (1 << (lk.index - 64))) != 0;
            }
        }

        if (isEnabled) {
            keys.push_back({lk.label, lk.key, lk.gridX, lk.gridY, lk.width});
        }
    }

    // Shift active rows to prevent vertical gaps
    std::set<float> activeYRows;
    for (const auto& k : keys) {
        activeYRows.insert(k.y);
    }
    std::map<float, float> yMapping;
    float compactY = 0.0f;
    for (float originalY : activeYRows) {
        yMapping[originalY] = compactY;
        compactY += 1.1f;
    }
    for (auto& k : keys) {
        k.y = yMapping[k.y];
    }

    // Shift active columns if the leftmost key starts after X=0
    if (!keys.empty()) {
        float minX = 999.0f;
        for (const auto& k : keys) {
            if (k.x < minX) minX = k.x;
        }
        if (minX > 0.0f) {
            for (auto& k : keys) {
                k.x -= minX;
            }
        }
    }

    if (keys.empty()) {
        if (isHudEditing) {
            keys = {
                {"W", ImGuiKey_W, 1.0f, 0.0f, 1.0f},
                {"A", ImGuiKey_A, 0.0f, 1.0f, 1.0f},
                {"S", ImGuiKey_S, 1.0f, 1.0f, 1.0f},
                {"D", ImGuiKey_D, 2.0f, 1.0f, 1.0f}
            };
        } else {
            return;
        }
    }

    float maxGridX = 0.0f;
    float maxGridY = 0.0f;
    for (const auto& key : keys) {
        if (key.x + key.w > maxGridX) maxGridX = key.x + key.w;
        if (key.y + 1.0f > maxGridY) maxGridY = key.y + 1.0f;
    }

    float keySize = 40.0f * scale;
    float actualSpacing = spacing * scale;
    float padding = 10.0f * scale;
    float shadowMax = 5.0f * scale;
    float actualRadius = radius * scale;

    m_Size.x = maxGridX * keySize + std::max(0.0f, maxGridX - 1.0f) * actualSpacing + padding * 2.0f;
    m_Size.y = maxGridY * keySize + std::max(0.0f, maxGridY - 1.0f) * actualSpacing + padding * 2.0f + shadowMax;

    // Draw keyboard base plate if separate keys is disabled
    if (!separateKeys) {
        dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 12.0f * scale);
        dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 12.0f * scale, 0, 1.5f * scale);
    } else if (isHudEditing) {
        // Draw dotted boundary when editing so they can position and resize the floating keys
        dl->AddRect(drawPos, drawPos + m_Size, IM_COL32(0, 190, 255, 120), 12.0f * scale, 0, 1.0f * scale);
    }

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    for (const auto& key : keys) {
        float progress = m_KeyPressStates[key.key];

        float kx = drawPos.x + padding + key.x * (keySize + actualSpacing);
        float ky = drawPos.y + padding + key.y * (keySize + actualSpacing);
        float kw = key.w * keySize + std::max(0.0f, key.w - 1.0f) * actualSpacing;
        float kh = keySize;

        ImVec2 keyCenter(kx + kw * 0.5f, ky + kh * 0.5f);

        // Determine glow / underglow color
        ImU32 glowColor;
        if (rgbWave) {
            ImVec4 rgb = GetRgbColor(rgbSpeed, key.x * 0.2f + key.y * 0.1f);
            glowColor = IM_COL32((int)(rgb.x * 255), (int)(rgb.y * 255), (int)(rgb.z * 255), 255);
        } else {
            if (key.label == "Space") {
                glowColor = IM_COL32(200, 50, 255, 255); // Purple
            } else if (key.label == "ShfL" || key.label == "ShfR" || key.label == "CtlL" || key.label == "CtlR" || key.label == "AltL" || key.label == "AltR" || key.label == "Tab" || key.label == "Caps" || key.label == "Esc" || key.label == "Ent") {
                glowColor = IM_COL32(255, 60, 140, 255); // Neon Pink
            } else if (key.label == "LMB" || key.label == "RMB" || key.label == "MMB") {
                glowColor = IM_COL32(0, 255, 200, 255); // Turquoise
            } else {
                glowColor = GetAccentCol();
            }
        }

        // Draw Switch Underglow (Glow bloom under keycap)
        float glowIntensity = glowIntensityVal * (0.2f + 0.8f * progress);
        if (glowIntensity > 0.01f) {
            int glowAlpha = (int)(glowIntensity * 100);
            ImU32 bloomColor = (glowColor & 0x00FFFFFF) | ((glowAlpha & 0xFF) << 24);
            
            dl->AddCircleFilled(keyCenter, (14.0f + 16.0f * progress) * scale, bloomColor);
            dl->AddCircleFilled(keyCenter, (8.0f + 8.0f * progress) * scale, (glowColor & 0x00FFFFFF) | (((int)(glowIntensity * 180) & 0xFF) << 24));
        }

        // 3D Keycap Physical Body Rendering
        // Shadow/Switch housing base
        ImVec2 shadowMin(kx, ky + shadowMax);
        ImVec2 shadowMaxPos(kx + kw, ky + kh + shadowMax);
        dl->AddRectFilled(shadowMin, shadowMaxPos, IM_COL32(18, 18, 22, 255), actualRadius);

        // Calculate travel offset
        float currentOffset = shadowMax * (1.0f - progress);
        ImVec2 capMin(kx, ky + currentOffset);
        ImVec2 capMax(kx + kw, ky + kh + currentOffset);

        // Draw keycap side extrusion (for 3D appearance)
        ImVec2 sideMin(kx, ky + currentOffset + 2.0f * scale);
        ImVec2 sideMax(kx + kw, ky + kh + shadowMax);
        dl->AddRectFilled(sideMin, sideMax, IM_COL32(28, 28, 33, 255), actualRadius);

        // Draw keycap top face
        ImU32 capBgCol = LerpCol(GetBgCol(), (glowColor & 0x00FFFFFF) | 0x22000000, progress);
        dl->AddRectFilled(capMin, capMax, capBgCol, actualRadius);

        // Top face inner highlight border
        ImU32 capBorderCol = LerpCol(GetBorderCol(), glowColor, progress);
        dl->AddRect(capMin, capMax, capBorderCol, actualRadius, 0, 1.2f * scale);

        // Illuminated keycap legends (letters/words/mouse clicks)
        ImU32 textCol = LerpCol(GetTextCol(), glowColor, progress);
        
        if ((key.key == ImGuiKey_MouseLeft || key.key == ImGuiKey_MouseRight) && showCps) {
            int cps = (key.key == ImGuiKey_MouseLeft) ? m_LmbClicks.size() : m_RmbClicks.size();
            char labelBuf[64];
            if (cps > 0) {
                snprintf(labelBuf, sizeof(labelBuf), "%s\n%d CPS", key.label.c_str(), cps);
            } else {
                snprintf(labelBuf, sizeof(labelBuf), "%s", key.label.c_str());
            }
            ImVec2 labelSize = CalcTextSize(labelBuf);
            ImVec2 labelPos = capMin + ImVec2((kw - labelSize.x) * 0.5f, (kh - labelSize.y) * 0.5f);
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, labelPos, textCol, labelBuf);
        } else {
            ImVec2 labelSize = CalcTextSize(key.label.c_str());
            ImVec2 labelPos = capMin + ImVec2((kw - labelSize.x) * 0.5f, (kh - labelSize.y) * 0.5f);
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, labelPos, textCol, key.label.c_str());
        }
    }
    if (g_FontHeader) ImGui::PopFont();
}

void KeystrokeVisualizerWidget::DrawCustomSettings(bool& isDirty) {
    ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "Keystroke Visualizer %d Settings", m_WidgetId);

    const char* presets[] = { "WASD", "WASD + Space", "WASD + Space + Shift", "WASD + Mouse", "Arrows", "Arrows + Space", "Custom" };
    int preset = (m_WidgetId == 1) ? sg_modKeystroke1_Preset : sg_modKeystroke2_Preset;
    if (ImGui::Combo("Layout Preset", &preset, presets, 7)) {
        if (m_WidgetId == 1) sg_modKeystroke1_Preset = preset;
        else sg_modKeystroke2_Preset = preset;
        isDirty = true;
    }

    ImGui::Separator();
    ImGui::Text("Aesthetic Customization:");
    
    bool separateKeys = (m_WidgetId == 1) ? sg_modKeystroke1_SeparateKeys : sg_modKeystroke2_SeparateKeys;
    if (ImGui::Checkbox("Separate Keys Mode (No Base Plate)", &separateKeys)) {
        if (m_WidgetId == 1) sg_modKeystroke1_SeparateKeys = separateKeys;
        else sg_modKeystroke2_SeparateKeys = separateKeys;
        isDirty = true;
    }

    float customSpacing = (m_WidgetId == 1) ? sg_modKeystroke1_Spacing : sg_modKeystroke2_Spacing;
    if (ImGui::SliderFloat("Key Spacing", &customSpacing, 0.0f, 25.0f, "%.1f")) {
        if (m_WidgetId == 1) sg_modKeystroke1_Spacing = customSpacing;
        else sg_modKeystroke2_Spacing = customSpacing;
        isDirty = true;
    }

    float customRadius = (m_WidgetId == 1) ? sg_modKeystroke1_Radius : sg_modKeystroke2_Radius;
    if (ImGui::SliderFloat("Key Border Radius", &customRadius, 0.0f, 20.0f, "%.1f")) {
        if (m_WidgetId == 1) sg_modKeystroke1_Radius = customRadius;
        else sg_modKeystroke2_Radius = customRadius;
        isDirty = true;
    }

    bool showCps = (m_WidgetId == 1) ? sg_modKeystroke1_ShowCps : sg_modKeystroke2_ShowCps;
    if (ImGui::Checkbox("Show Mouse Clicks-Per-Second (CPS)", &showCps)) {
        if (m_WidgetId == 1) sg_modKeystroke1_ShowCps = showCps;
        else sg_modKeystroke2_ShowCps = showCps;
        isDirty = true;
    }

    bool rgbWave = (m_WidgetId == 1) ? sg_modKeystroke1_RgbWave : sg_modKeystroke2_RgbWave;
    if (ImGui::Checkbox("RGB Rainbow Wave", &rgbWave)) {
        if (m_WidgetId == 1) sg_modKeystroke1_RgbWave = rgbWave;
        else sg_modKeystroke2_RgbWave = rgbWave;
        isDirty = true;
    }
    
    if (rgbWave) {
        float rgbSpeed = (m_WidgetId == 1) ? sg_modKeystroke1_RgbSpeed : sg_modKeystroke2_RgbSpeed;
        if (ImGui::SliderFloat("RGB Wave Speed", &rgbSpeed, 0.2f, 5.0f, "%.1f")) {
            if (m_WidgetId == 1) sg_modKeystroke1_RgbSpeed = rgbSpeed;
            else sg_modKeystroke2_RgbSpeed = rgbSpeed;
            isDirty = true;
        }
    }

    float intensity = (m_WidgetId == 1) ? sg_modKeystroke1_GlowIntensity : sg_modKeystroke2_GlowIntensity;
    if (ImGui::SliderFloat("Glow Intensity", &intensity, 0.0f, 1.5f, "%.1f")) {
        if (m_WidgetId == 1) sg_modKeystroke1_GlowIntensity = intensity;
        else sg_modKeystroke2_GlowIntensity = intensity;
        isDirty = true;
    }

    if (preset == 6) {
        ImGui::Separator();
        ImGui::Text("Toggle Custom Keys (Keyboard Matrix):");
        
        int& mask0 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask0 : sg_modKeystroke2_Mask0;
        int& mask1 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask1 : sg_modKeystroke2_Mask1;
        int& mask2 = (m_WidgetId == 1) ? sg_modKeystroke1_Mask2 : sg_modKeystroke2_Mask2;

        auto DrawKeyCheckbox = [&](const char* chk_label, int idx) {
            bool val = false;
            if (idx < 32) val = (mask0 & (1 << idx)) != 0;
            else if (idx < 64) val = (mask1 & (1 << (idx - 32))) != 0;
            else val = (mask2 & (1 << (idx - 64))) != 0;

            if (ImGui::Checkbox(chk_label, &val)) {
                if (idx < 32) {
                    if (val) mask0 |= (1 << idx);
                    else mask0 &= ~(1 << idx);
                } else if (idx < 64) {
                    if (val) mask1 |= (1 << (idx - 32));
                    else mask1 &= ~(1 << (idx - 32));
                } else {
                    if (val) mask2 |= (1 << (idx - 64));
                    else mask2 &= ~(1 << (idx - 64));
                }
                isDirty = true;
            }
        };

        // Render Row 0
        DrawKeyCheckbox("Esc", 0); ImGui::SameLine();
        DrawKeyCheckbox("F1", 1); ImGui::SameLine();
        DrawKeyCheckbox("F2", 2); ImGui::SameLine();
        DrawKeyCheckbox("F3", 3); ImGui::SameLine();
        DrawKeyCheckbox("F4", 4); ImGui::SameLine();
        DrawKeyCheckbox("F5", 5); ImGui::SameLine();
        DrawKeyCheckbox("F6", 6); ImGui::SameLine();
        DrawKeyCheckbox("F7", 7); ImGui::SameLine();
        DrawKeyCheckbox("F8", 8); ImGui::SameLine();
        DrawKeyCheckbox("F9", 9); ImGui::SameLine();
        DrawKeyCheckbox("F10", 10); ImGui::SameLine();
        DrawKeyCheckbox("F11", 11); ImGui::SameLine();
        DrawKeyCheckbox("F12", 12);

        // Render Row 1
        DrawKeyCheckbox("~", 13); ImGui::SameLine();
        DrawKeyCheckbox("1", 14); ImGui::SameLine();
        DrawKeyCheckbox("2", 15); ImGui::SameLine();
        DrawKeyCheckbox("3", 16); ImGui::SameLine();
        DrawKeyCheckbox("4", 17); ImGui::SameLine();
        DrawKeyCheckbox("5", 18); ImGui::SameLine();
        DrawKeyCheckbox("6", 19); ImGui::SameLine();
        DrawKeyCheckbox("7", 20); ImGui::SameLine();
        DrawKeyCheckbox("8", 21); ImGui::SameLine();
        DrawKeyCheckbox("9", 22); ImGui::SameLine();
        DrawKeyCheckbox("0", 23); ImGui::SameLine();
        DrawKeyCheckbox("-", 24); ImGui::SameLine();
        DrawKeyCheckbox("=", 25); ImGui::SameLine();
        DrawKeyCheckbox("Bsp", 26);

        // Render Row 2
        DrawKeyCheckbox("Tab", 27); ImGui::SameLine();
        DrawKeyCheckbox("Q", 28); ImGui::SameLine();
        DrawKeyCheckbox("W", 29); ImGui::SameLine();
        DrawKeyCheckbox("E", 30); ImGui::SameLine();
        DrawKeyCheckbox("R", 31); ImGui::SameLine();
        DrawKeyCheckbox("T", 32); ImGui::SameLine();
        DrawKeyCheckbox("Y", 33); ImGui::SameLine();
        DrawKeyCheckbox("U", 34); ImGui::SameLine();
        DrawKeyCheckbox("I", 35); ImGui::SameLine();
        DrawKeyCheckbox("O", 36); ImGui::SameLine();
        DrawKeyCheckbox("P", 37); ImGui::SameLine();
        DrawKeyCheckbox("[", 38); ImGui::SameLine();
        DrawKeyCheckbox("]", 39); ImGui::SameLine();
        DrawKeyCheckbox("\\", 40);

        // Render Row 3
        DrawKeyCheckbox("Caps", 41); ImGui::SameLine();
        DrawKeyCheckbox("A", 42); ImGui::SameLine();
        DrawKeyCheckbox("S", 43); ImGui::SameLine();
        DrawKeyCheckbox("D", 44); ImGui::SameLine();
        DrawKeyCheckbox("F", 45); ImGui::SameLine();
        DrawKeyCheckbox("G", 46); ImGui::SameLine();
        DrawKeyCheckbox("H", 47); ImGui::SameLine();
        DrawKeyCheckbox("J", 48); ImGui::SameLine();
        DrawKeyCheckbox("K", 49); ImGui::SameLine();
        DrawKeyCheckbox("L", 50); ImGui::SameLine();
        DrawKeyCheckbox(";", 51); ImGui::SameLine();
        DrawKeyCheckbox("'", 52); ImGui::SameLine();
        DrawKeyCheckbox("Enter", 53);

        // Render Row 4
        DrawKeyCheckbox("Shift L", 54); ImGui::SameLine();
        DrawKeyCheckbox("Z", 55); ImGui::SameLine();
        DrawKeyCheckbox("X", 56); ImGui::SameLine();
        DrawKeyCheckbox("C", 57); ImGui::SameLine();
        DrawKeyCheckbox("V", 58); ImGui::SameLine();
        DrawKeyCheckbox("B", 59); ImGui::SameLine();
        DrawKeyCheckbox("N", 60); ImGui::SameLine();
        DrawKeyCheckbox("M", 61); ImGui::SameLine();
        DrawKeyCheckbox(",", 62); ImGui::SameLine();
        DrawKeyCheckbox(".", 63); ImGui::SameLine();
        DrawKeyCheckbox("/", 64); ImGui::SameLine();
        DrawKeyCheckbox("Shift R", 65);

        // Render Row 5
        DrawKeyCheckbox("Ctrl L", 66); ImGui::SameLine();
        DrawKeyCheckbox("Win L", 67); ImGui::SameLine();
        DrawKeyCheckbox("Alt L", 68); ImGui::SameLine();
        DrawKeyCheckbox("Space", 69); ImGui::SameLine();
        DrawKeyCheckbox("Alt R", 70); ImGui::SameLine();
        DrawKeyCheckbox("Win R", 71); ImGui::SameLine();
        DrawKeyCheckbox("Menu", 72); ImGui::SameLine();
        DrawKeyCheckbox("Ctrl R", 73);

        // System / Navigation Row
        ImGui::Text("System/Navigation:");
        DrawKeyCheckbox("PrtScr", 74); ImGui::SameLine();
        DrawKeyCheckbox("Scroll", 75); ImGui::SameLine();
        DrawKeyCheckbox("Pause", 76); ImGui::SameLine();
        DrawKeyCheckbox("Ins", 77); ImGui::SameLine();
        DrawKeyCheckbox("Home", 78); ImGui::SameLine();
        DrawKeyCheckbox("PgUp", 79); ImGui::SameLine();
        DrawKeyCheckbox("Del", 80); ImGui::SameLine();
        DrawKeyCheckbox("End", 81); ImGui::SameLine();
        DrawKeyCheckbox("PgDn", 82);

        // Arrow Keys
        ImGui::Text("Arrows:");
        DrawKeyCheckbox("Up", 83); ImGui::SameLine();
        DrawKeyCheckbox("Left", 84); ImGui::SameLine();
        DrawKeyCheckbox("Down", 85); ImGui::SameLine();
        DrawKeyCheckbox("Right", 86);

        // Mouse Buttons
        ImGui::Text("Mouse:");
        DrawKeyCheckbox("LMB", 87); ImGui::SameLine();
        DrawKeyCheckbox("RMB", 88); ImGui::SameLine();
        DrawKeyCheckbox("MMB", 89);
    }
}


// ---------------------------------------------------------------------------

namespace
{

struct rcCycleName
{
    float x, y;          //!< where, in pixels
    float height;        //!< how tall, in pixels
    float alpha;
    ImU32 colour;
    std::string text;
};

std::vector< rcCycleName > s_cycleNames;

//! The colour a painted name should come out as.
//!
//! The engine writes colour changes into the string itself. One name can carry
//! several, and honouring every one would mean drawing it letter by letter for
//! no gain over a cycle - so the first colour it names wins, and the codes
//! themselves are dropped.
ImU32 FirstColour( std::string const & in, float alpha, std::string & plain )
{
    float r = 1, g = 1, b = 1;
    bool found = false;

    plain.clear();
    plain.reserve( in.size() );

    for ( size_t i = 0; i < in.size(); )
    {
        if ( in[i] == '0' && i + 1 < in.size() && in[i+1] == 'x' && i + 8 <= in.size() )
        {
            if ( !found )
            {
                char buf[3] = { 0, 0, 0 };
                buf[0] = in[i+2]; buf[1] = in[i+3]; r = (float)strtol( buf, 0, 16 ) / 255.0f;
                buf[0] = in[i+4]; buf[1] = in[i+5]; g = (float)strtol( buf, 0, 16 ) / 255.0f;
                buf[0] = in[i+6]; buf[1] = in[i+7]; b = (float)strtol( buf, 0, 16 ) / 255.0f;
                found = true;
            }

            i += 8;
            continue;
        }

        plain += in[i];
        ++i;
    }

    // a name in a colour nobody can read against the floor is worse than none
    float const lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    if ( lum < 0.25f )
    {
        float const lift = 0.25f / ( lum > 0.01f ? lum : 0.01f );
        r *= lift; g *= lift; b *= lift;
        if ( r > 1 ) r = 1;
        if ( g > 1 ) g = 1;
        if ( b > 1 ) b = 1;
    }

    return ImGui::GetColorU32( ImVec4( r, g, b, alpha ) );
}

}

bool sg_sharpNames = true;
static tConfItem<bool> conf_sharpNames( "MOD_SHARP_NAMES", sg_sharpNames );

bool rc_SharpNames()
{
    return sg_sharpNames && sr_glOut;
}

void rc_QueueCycleName( REAL ndcX, REAL ndcY, tString const & text,
                        REAL ndcHeight, REAL alpha )
{
    if ( !ImGui::GetCurrentContext() )
        return;

    ImGuiIO & io = ImGui::GetIO();
    if ( io.DisplaySize.x < 1 || io.DisplaySize.y < 1 )
        return;

    rcCycleName one;
    one.x = (float)( ( ndcX * 0.5 + 0.5 ) * io.DisplaySize.x );
    one.y = (float)( ( 1.0 - ( ndcY * 0.5 + 0.5 ) ) * io.DisplaySize.y );
    one.height = (float)( ndcHeight * io.DisplaySize.y );
    one.alpha = (float)alpha;

    std::string plain;
    one.colour = FirstColour( std::string( (char const *)text ), (float)alpha, plain );
    one.text = plain;

    if ( one.text.empty() || one.height < 4.0f )
        return;

    s_cycleNames.push_back( one );
}

void rc_DrawCycleNames()
{
    if ( s_cycleNames.empty() )
        return;

    ImDrawList * dl = ImGui::GetBackgroundDrawList();

    for ( size_t i = 0; i < s_cycleNames.size(); ++i )
    {
        rcCycleName const & one = s_cycleNames[i];

        // The nearest size that was actually baked, and drawn at that size.
        // Asking for an arbitrary height stretches one rastered face, which is
        // exactly what made these labels ragged.
        ImFont * font = GetBestNameFont( one.height );
        float const drawn = font->FontSize > 0 ? font->FontSize : one.height;

        ImVec2 const size = font->CalcTextSizeA( drawn, FLT_MAX, 0.0f, one.text.c_str() );
        ImVec2 const at( one.x - size.x * 0.5f, one.y - size.y );

        // a dark pass underneath, so a pale name still reads over the floor
        ImU32 const shade = ImGui::GetColorU32( ImVec4( 0, 0, 0, one.alpha * 0.7f ) );
        dl->AddText( font, drawn, ImVec2( at.x + 1.0f, at.y + 1.0f ), shade, one.text.c_str() );
        dl->AddText( font, drawn, at, one.colour, one.text.c_str() );
    }

    s_cycleNames.clear();
}

// ---------------------------------------------------------------------------
// The console, in our own hand.
//
// Nothing here decides what is shown or for how long - the engine's console
// does all of that, exactly as it always has, and hands the lines over. This
// only draws them: our face, a frame that can be moved and sized like any
// other gauge, the time a line arrived, and a mark on anybody speaking from
// the grave.

bool sg_chatSkin = true;
static tConfItem<bool> sg_chatSkinConf( "MOD_CHAT_SKIN", sg_chatSkin );

bool sg_chatOurFont = true;
static tConfItem<bool> sg_chatOurFontConf( "MOD_CHAT_FACE", sg_chatOurFont );

bool sg_chatStamp = true;
static tConfItem<bool> sg_chatStampConf( "MOD_CHAT_STAMP", sg_chatStamp );

REAL sg_chatSize = 15.0f;
static tConfItem<REAL> sg_chatSizeConf( "MOD_CHAT_SIZE", sg_chatSize );

REAL sg_chatWide = 620.0f;
static tConfItem<REAL> sg_chatWideConf( "MOD_CHAT_WIDE", sg_chatWide );

REAL sg_chatLeft = 24.0f;
static tConfItem<REAL> sg_chatLeftConf( "MOD_CHAT_LEFT", sg_chatLeft );

REAL sg_chatTop = 620.0f;
static tConfItem<REAL> sg_chatTopConf( "MOD_CHAT_TOP", sg_chatTop );

REAL sg_chatTall = 220.0f;
static tConfItem<REAL> sg_chatTallConf( "MOD_CHAT_TALL", sg_chatTall );

REAL sg_chatShade = 0.45f;
static tConfItem<REAL> sg_chatShadeConf( "MOD_CHAT_SHADE", sg_chatShade );

namespace
{

//! When each console line arrived, kept by the line's own number.
std::map< int, time_t > s_when;

//! Text without the colour codes.
std::string Bare( std::string const & in )
{
    std::string out;
    out.reserve( in.size() );

    for ( size_t i = 0; i < in.size(); )
    {
        if ( in[i] == '0' && i + 1 < in.size() && in[i+1] == 'x' && i + 8 <= in.size() )
        {
            i += 8;
            continue;
        }

        out += in[i];
        ++i;
    }

    return out;
}

//! Whether the player who said this had no cycle at the time.
bool Ghost( std::string const & line )
{
    size_t const colon = line.find( ": " );
    if ( colon == std::string::npos || colon == 0 )
        return false;

    std::string said = Bare( line.substr( 0, colon ) );

    while ( !said.empty() && said[0] == ' ' )
        said.erase( said.begin() );
    while ( !said.empty() && said[ said.size() - 1 ] == ' ' )
        said.erase( said.end() - 1 );

    if ( said.empty() )
        return false;

    for ( int i = 0; i < se_PlayerNetIDs.Len(); ++i )
    {
        ePlayerNetID * p = se_PlayerNetIDs( i );
        if ( !p )
            continue;

        tString plain = tColoredString::RemoveColors( p->GetName() );
        if ( strcmp( (char const *)plain, said.c_str() ) != 0 )
            continue;

        return !rc_PlayerIsAlive( p );
    }

    return false;
}

//! One stretch of text in one colour.
struct Piece
{
    std::string text;
    ImU32 colour;
};

//! Turns a painted line into pieces, and lifts anything too dark to read.
void Paint( std::string const & in, ImU32 base, float alpha, std::vector< Piece > & out )
{
    Piece run;
    run.colour = base;

    for ( size_t i = 0; i < in.size(); )
    {
        bool const code = ( in[i] == '0' && i + 1 < in.size() && in[i+1] == 'x' && i + 8 <= in.size() );

        if ( !code )
        {
            run.text += in[i];
            ++i;
            continue;
        }

        if ( !run.text.empty() )
        {
            out.push_back( run );
            run.text.clear();
        }

        if ( in.compare( i, 8, "0xRESETT" ) == 0 )
        {
            run.colour = base;
            i += 8;
            continue;
        }

        char pair[3] = { 0, 0, 0 };
        pair[0] = in[i+2]; pair[1] = in[i+3]; float r = (float)strtol( pair, 0, 16 ) / 255.0f;
        pair[0] = in[i+4]; pair[1] = in[i+5]; float g = (float)strtol( pair, 0, 16 ) / 255.0f;
        pair[0] = in[i+6]; pair[1] = in[i+7]; float b = (float)strtol( pair, 0, 16 ) / 255.0f;

        float const lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        if ( lum < 0.45f )
        {
            float const lift = 0.45f / ( lum > 0.01f ? lum : 0.01f );
            r *= lift; g *= lift; b *= lift;
            if ( r > 1 ) r = 1;
            if ( g > 1 ) g = 1;
            if ( b > 1 ) b = 1;
        }

        run.colour = ImGui::GetColorU32( ImVec4( r, g, b, alpha ) );
        i += 8;
    }

    if ( !run.text.empty() )
        out.push_back( run );
}

//! One display line, after wrapping.
struct Row { std::vector< Piece > pieces; };

//! What the last layout produced, waiting to be drawn.
struct Ready
{
    std::vector< Row > rows;
    ImFont * font = 0;
    float size = 0, step = 0, wide = 0, pad = 0;
    bool have = false;
};

Ready s_ready;

}

bool rc_ConsoleSkinned()
{
    return sg_chatSkin && sr_glOut && ImGui::GetCurrentContext() != 0;
}

bool rc_ConsoleSkin( tArray<tString> & lines, int from, int to, int cap, int & used )
{
    used = 0;

    if ( !rc_ConsoleSkinned() )
        return false;

    ImGuiIO & io = ImGui::GetIO();
    if ( io.DisplaySize.x < 1 || io.DisplaySize.y < 1 )
        return false;

    float const wanted = sg_chatSize;
    ImFont * font = sg_chatOurFont ? GetBestNameFont( wanted ) : GetBestFont( wanted );
    float const size = font->FontSize > 0 ? font->FontSize : wanted;
    float const step = size * 1.2f;

    float const wide = sg_chatWide;
    float const pad = 8.0f;
    float const room = wide - pad * 2.0f;

    if ( room < 40.0f )
        return false;

    ImU32 const plain = IM_COL32( 236, 240, 248, 255 );

    std::vector< Row > rows;

    for ( int i = from; i <= to && i <= cap; ++i )
    {
        if ( lines[i].Len() <= 1 )
            continue;

        std::string const raw = (char const *)lines[i];

        // A line that is only colour codes and spaces carries nothing; giving
        // it a timestamp would put a bare clock in the middle of the chat.
        {
            std::string const bare = Bare( raw );
            if ( bare.find_first_not_of( " \t\r\n" ) == std::string::npos )
                continue;
        }

        if ( s_when.find( i ) == s_when.end() )
            s_when[ i ] = time( 0 );

        std::vector< Piece > pieces;

        if ( sg_chatStamp )
        {
            struct tm * when = localtime( &s_when[ i ] );
            char stamp[ 16 ];
            strftime( stamp, sizeof( stamp ), "%H:%M ", when );

            Piece p;
            p.text = stamp;
            p.colour = IM_COL32( 120, 126, 138, 255 );
            pieces.push_back( p );
        }

        if ( Ghost( raw ) )
        {
            Piece p;
            p.text = "*DEAD* ";
            p.colour = IM_COL32( 236, 92, 92, 255 );
            pieces.push_back( p );
        }

        Paint( raw, plain, 1.0f, pieces );

        // wrapped at word boundaries, so a long line keeps its colours
        Row row;
        float wideSoFar = 0.0f;

        for ( size_t p = 0; p < pieces.size(); ++p )
        {
            std::string word;
            std::string const & text = pieces[p].text;

            for ( size_t c = 0; c <= text.size(); ++c )
            {
                bool const end = ( c == text.size() );

                if ( !end && text[c] != ' ' )
                {
                    word += text[c];
                    continue;
                }

                if ( !end )
                    word += ' ';
                if ( word.empty() )
                    continue;

                float const w = font->CalcTextSizeA( size, FLT_MAX, 0.0f, word.c_str() ).x;

                if ( wideSoFar + w > room && wideSoFar > 0.0f )
                {
                    rows.push_back( row );
                    row.pieces.clear();
                    wideSoFar = 0.0f;
                }

                Piece piece;
                piece.text = word;
                piece.colour = pieces[p].colour;
                row.pieces.push_back( piece );
                wideSoFar += w;

                word.clear();
            }
        }

        if ( !row.pieces.empty() )
            rows.push_back( row );
    }

    used = (int)rows.size();

    // Older arrivals are of no further interest.
    while ( s_when.size() > 400 )
        s_when.erase( s_when.begin() );

    // Laid out here, drawn later. The console is rendered outside the
    // interface's frame - by the time this runs, that frame has already been
    // handed to the card, and anything added to it now would simply never be
    // sent. So the finished rows wait for the next pass over the interface.
    s_ready.rows.swap( rows );
    s_ready.font = font;
    s_ready.size = size;
    s_ready.step = step;
    s_ready.wide = wide;
    s_ready.pad = pad;
    s_ready.have = true;

    return true;
}

// ---------------------------------------------------------------------------
// The start.
//
// The engine counts a round in with three numbers thrown at the middle of the
// screen, one a second, each one landing on top of whatever you were looking
// at. It tells you the time but not the rhythm, and the last thing it says is
// a digit that looks exactly like the two before it - so the moment that
// actually matters, the one you push off on, is the one that reads worst.
//
// A gantry says it better. Lamps fill from the left while you wait, the whole
// row is lit a moment before the off, and then it all goes dark at once: the
// signal is the change, not a word, and peripheral vision catches it without
// being looked at. Underneath, a line that empties in real time, because
// knowing the rhythm is not the same as knowing the fraction of a second, and
// a start worth practising deserves both.

int   sg_startLights = 1;      //!< 0 numbers, 1 gantry, 2 traffic light
REAL  sg_startLightsSize = 1.0f;
REAL  sg_startLightsY = 0.22f;

static tConfItem<int>  sg_startLightsConf( "MOD_START_LIGHTS", sg_startLights );
static tConfItem<REAL> sg_startLightsSizeConf( "MOD_START_LIGHTS_SIZE", sg_startLightsSize );
static tConfItem<REAL> sg_startLightsYConf( "MOD_START_LIGHTS_Y", sg_startLightsY );

namespace
{
//! Seconds until the round starts, or a large number when that is not a thing
//! that is about to happen.
REAL TimeToStart()
{
    if ( sg_startLights <= 0 )
        return 1000.0f;

    if ( !se_mainGameTimer || !se_mainGameTimer->IsSynced() )
        return 1000.0f;

    REAL const now = se_GameTime();

    static bool counting = false;

    // The count in is four seconds at most, and a round that has been running
    // for a while is not about to start again.
    if ( now < -6.0f || now > 1.1f )
    {
        counting = false;
        return 1000.0f;
    }

    // Between two matches the clock is put back to zero and held there while
    // the grid is built again, which from here looks exactly like the last
    // moment of a count in. A clock that has never been below zero is not
    // counting anything in, and the lights have no business showing up for a
    // reload nobody is waiting on.
    if ( now < -0.05f )
        counting = true;

    if ( !counting )
        return 1000.0f;

    return -now;
}

//! One lamp: housing, lens, and the light it throws when it is on.
void Lamp( ImDrawList * dl, ImVec2 at, float radius, ImU32 colour, float lit, float scale )
{
    // the housing stays visible unlit, so the row reads as a thing before it
    // does anything
    dl->AddCircleFilled( at, radius * 1.18f, IM_COL32( 10, 11, 14, 230 ), 32 );
    dl->AddCircle( at, radius * 1.18f, IM_COL32( 48, 52, 62, 220 ), 32, 1.5f * scale );

    if ( lit <= 0.01f )
    {
        dl->AddCircleFilled( at, radius, IM_COL32( 24, 26, 32, 210 ), 32 );
        return;
    }

    int const r = (int)( ( colour >> IM_COL32_R_SHIFT ) & 0xFF );
    int const g = (int)( ( colour >> IM_COL32_G_SHIFT ) & 0xFF );
    int const b = (int)( ( colour >> IM_COL32_B_SHIFT ) & 0xFF );

    // spill first, so the lens sits on top of its own glow
    for ( int i = 3; i >= 1; --i )
    {
        float const spread = radius * ( 1.0f + 0.55f * i );
        int const a = (int)( 40 * lit / i );
        dl->AddCircleFilled( at, spread, IM_COL32( r, g, b, a ), 32 );
    }

    dl->AddCircleFilled( at, radius, IM_COL32( r, g, b, (int)( 255 * lit ) ), 32 );

    // the hot centre of a real lamp
    dl->AddCircleFilled( ImVec2( at.x, at.y - radius * 0.18f ), radius * 0.45f,
                         IM_COL32( 255, 255, 255, (int)( 180 * lit ) ), 24 );
}
}

//! Whether the count in is being shown as lights, so the numbers can stay off.
bool rc_StartLightsUp()
{
    REAL const left = TimeToStart();
    return sg_startLights > 0 && left < 900.0f;
}

void rc_DrawStartLights()
{
    REAL const left = TimeToStart();
    if ( left > 900.0f )
        return;

    ImGuiIO & io = ImGui::GetIO();
    if ( io.DisplaySize.x < 1 || io.DisplaySize.y < 1 )
        return;

    ImDrawList * dl = ImGui::GetForegroundDrawList();

    float const scale = ( io.DisplaySize.y / 1080.0f ) * ( sg_startLightsSize > 0.05f ? sg_startLightsSize : 1.0f );

    // After the off the whole thing goes out and the green flash takes over,
    // then it leaves. Nothing lingers over the arena once the round is live.
    bool const away = left <= 0.0f;
    float const since = away ? -left : 0.0f;
    float const bye = away ? ( 1.0f - since / 1.1f ) : 1.0f;
    if ( bye <= 0.0f )
        return;

    float const cx = io.DisplaySize.x * 0.5f;
    float const cy = io.DisplaySize.y * ( sg_startLightsY > 0.02f ? sg_startLightsY : 0.22f );

    if ( sg_startLights == 2 )
    {
        // The other kind: one head, three lamps, the way a road does it.
        float const radius = 26.0f * scale;
        float const step = radius * 2.6f;
        float const w = radius * 3.4f;
        float const h = step * 2.0f + radius * 3.4f;

        ImVec2 const top( cx - w * 0.5f, cy - h * 0.5f );
        ImVec2 const bot( cx + w * 0.5f, cy + h * 0.5f );

        dl->AddRectFilled( top, bot, IM_COL32( 8, 9, 12, (int)( 220 * bye ) ), 14.0f * scale );
        dl->AddRect( top, bot, IM_COL32( 44, 48, 58, (int)( 210 * bye ) ), 14.0f * scale );

        float const red = left > 2.0f ? 1.0f : 0.0f;
        float const amber = ( left <= 2.0f && left > 0.0f ) ? 1.0f : 0.0f;
        float const green = away ? bye : 0.0f;

        Lamp( dl, ImVec2( cx, cy - step ), radius, IM_COL32( 240, 60, 55, 255 ), red * bye, scale );
        Lamp( dl, ImVec2( cx, cy ),        radius, IM_COL32( 250, 190, 40, 255 ), amber * bye, scale );
        Lamp( dl, ImVec2( cx, cy + step ), radius, IM_COL32( 60, 230, 110, 255 ), green, scale );
    }
    else
    {
        // Five lamps, filling left to right, one every six tenths. All five
        // are lit for the last stretch, and the off is all of them going dark
        // together - which is the part you actually react to.
        int const pods = 5;
        float const radius = 22.0f * scale;
        float const step = radius * 3.1f;
        float const w = step * ( pods - 1 ) + radius * 3.6f;
        float const h = radius * 3.6f;

        ImVec2 const top( cx - w * 0.5f, cy - h * 0.5f );
        ImVec2 const bot( cx + w * 0.5f, cy + h * 0.5f );

        dl->AddRectFilled( top, bot, IM_COL32( 8, 9, 12, (int)( 215 * bye ) ), 16.0f * scale );
        dl->AddRect( top, bot, IM_COL32( 44, 48, 58, (int)( 200 * bye ) ), 16.0f * scale );

        // the gantry's own shadow, so it hangs rather than floats
        dl->AddRectFilled( ImVec2( top.x + 6.0f * scale, bot.y ),
                           ImVec2( bot.x - 6.0f * scale, bot.y + 4.0f * scale ),
                           IM_COL32( 0, 0, 0, (int)( 90 * bye ) ), 3.0f * scale );

        for ( int i = 0; i < pods; ++i )
        {
            float const at = 3.0f - i * 0.6f;      // when this one comes on
            float lit = 0.0f;

            if ( !away && left <= at )
            {
                // a lamp does not snap on; it takes a moment to come up
                float const age = at - left;
                lit = age / 0.12f;
                if ( lit > 1.0f ) lit = 1.0f;
            }

            Lamp( dl, ImVec2( cx - w * 0.5f + radius * 1.8f + i * step, cy ),
                  radius, IM_COL32( 240, 55, 50, 255 ), lit * bye, scale );
        }

        // the off: one green wash across the row
        if ( away )
        {
            float const flash = bye * bye;
            dl->AddRectFilled( top, bot, IM_COL32( 60, 230, 110, (int)( 70 * flash ) ), 16.0f * scale );

            ImFont * font = GetBestNameFont( 44.0f * scale );
            float const size = font->FontSize > 0 ? font->FontSize : 44.0f * scale;
            ImVec2 const sz = font->CalcTextSizeA( size, FLT_MAX, 0.0f, "GO" );
            dl->AddText( font, size, ImVec2( cx - sz.x * 0.5f, bot.y + 14.0f * scale ),
                         IM_COL32( 120, 245, 150, (int)( 255 * flash ) ), "GO" );
        }
    }

    // The fraction, for anyone who wants to practise the push. Empties in real
    // time and disappears with the rest.
    if ( !away && left < 4.0f )
    {
        float const barW = 160.0f * scale;
        float const y = cy + 44.0f * scale;
        float const part = left / 3.0f > 1.0f ? 1.0f : left / 3.0f;

        dl->AddRectFilled( ImVec2( cx - barW * 0.5f, y ), ImVec2( cx + barW * 0.5f, y + 2.0f * scale ),
                           IM_COL32( 40, 44, 54, 200 ) );
        dl->AddRectFilled( ImVec2( cx - barW * 0.5f, y ),
                           ImVec2( cx - barW * 0.5f + barW * ( 1.0f - part ), y + 2.0f * scale ),
                           IM_COL32( 200, 210, 230, 220 ) );
    }
}

//! Whether the hud has the centre of the screen, so the engine leaves it be.
//!
//! Deliberately not tied to the widget being switched on. Turning the widget
//! off means wanting no centre messages, not wanting them in the old letters -
//! and while there is a window at all, ours is the only renderer that should
//! be putting words there.
bool rc_CenterSkinned()
{
    return sr_glOut && ImGui::GetCurrentContext() != 0;
}

// Fading a colour without a widget to ask.
static ImU32 rc_WithAlpha( ImU32 colour, float alpha )
{
    if ( alpha < 0.0f ) alpha = 0.0f;
    if ( alpha > 1.0f ) alpha = 1.0f;

    unsigned int const a = (unsigned int)( ( ( colour >> IM_COL32_A_SHIFT ) & 0xFF ) * alpha );
    return ( colour & ~IM_COL32_A_MASK ) | ( a << IM_COL32_A_SHIFT );
}

//! Whether a centre message is naming a winner.
static bool IsWinnerMessage( std::string const & msg )
{
    std::string const bare = StripColors( msg.c_str() );
    return bare.find( "Winner" ) != std::string::npos ||
           bare.find( "winner" ) != std::string::npos;
}

//! Whether a centre message is one tick of the count in.
static bool IsCountdownMessage( std::string const & msg )
{
    return ( msg.size() == 1 && msg[0] >= '0' && msg[0] <= '9' ) ||
           msg == "GO" || msg == "Go" || msg == "go" || msg == "GO!" || msg == "Go!";
}

// The word in the middle of the screen: the count in, the winner, whatever the
// server has to say. Drawn straight rather than as a widget - it is not
// furniture to be arranged, it belongs in the centre and nowhere else.
void rc_DrawCenterMessage() {
    if (!sg_modCenterMessageEnabled) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImGuiIO & io = ImGui::GetIO();
    if (io.DisplaySize.x < 1 || io.DisplaySize.y < 1) return;

    float const scale = io.DisplaySize.y / 1080.0f;

    ImVec2 m_Size = ImVec2(450.0f * scale, 120.0f * scale);

    // Get current message from rConsoleGraph
    std::string msg = g_modCenterText;
    double expiry = g_modCenterTextExpiry;
    double now = tSysTimeFloat();
    double timeLeft = expiry - now;

    // The engine names the champion and opens the next match in the same tick,
    // so the "new match" notice lands on top of the announcement and nobody
    // ever sees who won; the end of a round is wiped the same way when the
    // grid goes. A winner keeps the middle of the screen for a few seconds of
    // its own, and only the count in or another winner may cut in.
    static std::string heldMsg;
    static std::string lastMsg;
    static double heldUntil = 0.0;

    if ( msg != lastMsg )
    {
        lastMsg = msg;

        if ( IsCountdownMessage( msg ) )
            heldUntil = 0.0;
        else if ( timeLeft > 0.0 && IsWinnerMessage( msg ) )
        {
            heldMsg = msg;
            heldUntil = now + 4.0;
        }
    }

    if ( now < heldUntil && msg != heldMsg )
    {
        msg = heldMsg;
        expiry = heldUntil;
        timeLeft = heldUntil - now;
    }

    // Use dummy message if editing
    bool editing = isHudEditing;
    if (editing && (msg.empty() || timeLeft <= 0.0)) {
        msg = "Match Winner: player";
        timeLeft = 2.5;
        expiry = now + 2.5;
    }

    if (msg.empty() || timeLeft <= 0.0) {
        return;
    }

    // Centering point
    ImVec2 center( io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.42f );

    // Fade out alpha
    float alpha = 1.0f;
    if (timeLeft < 0.4) {
        alpha = (float)(timeLeft / 0.4);
    }

    bool isCountdown = IsCountdownMessage( msg );

    // Two things counting the same round in is one too many.
    if ( isCountdown && rc_StartLightsUp() && !editing )
        return;

    if (isCountdown) {
        // === PREMIUM COUNTDOWN STYLE ===
        // Determine phase of the tick (phase goes from 1.0 down to 0.0)
        float duration = 1.0f;
        float phase = (float)(timeLeft / duration);
        if (phase > 1.0f) phase = 1.0f;
        if (phase < 0.0f) phase = 0.0f;

        // Custom premium entry pop-in animation
        float scaleFactor = 1.0f;
        float textAlpha = alpha;
        if (phase > 0.85f) {
            // Pop in: scale down from 2.0x to 1.0x, fade in
            float t = (phase - 0.85f) / 0.15f; // 1.0 down to 0.0
            scaleFactor = 1.0f + 1.0f * t;
            textAlpha *= (1.0f - t);
        } else if (phase < 0.2f) {
            // Fade out/scale up slightly
            float t = phase / 0.2f; // 1.0 down to 0.0
            scaleFactor = 1.0f + 0.1f * (1.0f - t);
            textAlpha *= t;
        }

        float numSize = 84.0f * scale * scaleFactor;
        ImFont* font = GetBestFont(numSize);
        if (!font) font = g_FontHeader ? g_FontHeader : ImGui::GetFont();

        ImVec2 textSz = font->CalcTextSizeA(numSize, FLT_MAX, 0.0f, msg.c_str());
        ImVec2 textPos(center.x - textSz.x * 0.5f, center.y - textSz.y * 0.5f);

        // Glassmorphic circular backplate
        float circleRadius = 60.0f * scale;
        dl->AddCircleFilled(center, circleRadius, IM_COL32(10, 12, 18, (int)(160 * textAlpha)));

        // Neon outer glow outline
        ImU32 glowCol = (msg == "GO" || msg == "Go" || msg == "go" || msg == "GO!" || msg == "Go!" || msg == "0") 
                        ? IM_COL32(0, 255, 128, 255) 
                        : IM_COL32(255, 180, 0, 255);
        
        // Expanding pulse ripple ring
        float rippleProgress = 1.0f - phase; // 0.0 to 1.0
        float rippleRadius = circleRadius * (1.0f + rippleProgress * 0.6f);
        dl->AddCircle(center, rippleRadius, rc_WithAlpha(glowCol, textAlpha * (1.0f - rippleProgress) * 0.4f), 64, 2.0f * scale);

        // Main outer glow ring
        dl->AddCircle(center, circleRadius, rc_WithAlpha(glowCol, textAlpha * 0.85f), 64, 2.5f * scale);

        // Multi-layered text glow
        dl->AddText(font, numSize, textPos + ImVec2(-1.0f * scale, -1.0f * scale), rc_WithAlpha(glowCol, textAlpha * 0.15f), msg.c_str());
        dl->AddText(font, numSize, textPos + ImVec2(1.0f * scale, -1.0f * scale), rc_WithAlpha(glowCol, textAlpha * 0.15f), msg.c_str());
        dl->AddText(font, numSize, textPos + ImVec2(-1.0f * scale, 1.0f * scale), rc_WithAlpha(glowCol, textAlpha * 0.15f), msg.c_str());
        dl->AddText(font, numSize, textPos + ImVec2(1.0f * scale, 1.0f * scale), rc_WithAlpha(glowCol, textAlpha * 0.15f), msg.c_str());
        dl->AddText(font, numSize, textPos, rc_WithAlpha(glowCol, textAlpha * 0.3f), msg.c_str());

        // Shadow text
        dl->AddText(font, numSize, textPos + ImVec2(3.0f * scale, 3.0f * scale), IM_COL32(0, 0, 0, (int)(200 * textAlpha)), msg.c_str());

        // Main text
        ImU32 textCol = IM_COL32(255, 255, 255, (int)(255 * textAlpha));
        dl->AddText(font, numSize, textPos, textCol, msg.c_str());
    }
    else if ( IsWinnerMessage( msg ) ) {
        // === THE WINNER ===
        //
        // The end of a match said in the same grey box as "ten seconds left"
        // is a poor way to end anything. The label goes small and spaced, the
        // name goes large, and the two rules do the work a box was doing.
        std::string const bare = StripColors( msg.c_str() );
        size_t const colon = bare.find( ':' );

        std::string label = colon == std::string::npos ? std::string( "WINNER" ) : bare.substr( 0, colon );
        std::string who   = colon == std::string::npos ? bare : bare.substr( colon + 1 );

        while ( !who.empty() && who[0] == ' ' ) who.erase( 0, 1 );
        while ( !who.empty() && ( who[who.size()-1] == ' ' || who[who.size()-1] == '\n' ) ) who.erase( who.size()-1 );
        for ( size_t i = 0; i < label.size(); ++i ) label[i] = (char)toupper( (unsigned char)label[i] );

        bool const match = label.find( "MATCH" ) != std::string::npos;

        // gold for the match, the house colour for a round
        ImU32 const accent = match ? IM_COL32( 236, 196, 92, 255 ) : IM_COL32( 96, 200, 255, 255 );

        float const arrive = 1.0f - ( timeLeft > 2.6 ? (float)( ( timeLeft - 2.6 ) / 0.4 ) : 0.0f );
        float const ease = arrive < 0.0f ? 0.0f : ( arrive > 1.0f ? 1.0f : arrive );

        // sized against the screen as well as the widget: a match ending is
        // not a hud element, it is the moment
        float const room = ImGui::GetIO().DisplaySize.y / 1080.0f;
        float const nameSize = ( match ? 84.0f : 56.0f ) * scale * room;
        ImFont * nameFont = GetBestNameFont( nameSize );
        float const drawn = nameFont->FontSize > 0 ? nameFont->FontSize : nameSize;

        ImFont * tinyFont = GetBestFont( 21.0f * scale * room );
        float const smallSize = tinyFont->FontSize > 0 ? tinyFont->FontSize : 21.0f * scale * room;

        ImVec2 const nameSz = nameFont->CalcTextSizeA( drawn, FLT_MAX, 0.0f, who.c_str() );
        ImVec2 const labelSz = tinyFont->CalcTextSizeA( smallSize, FLT_MAX, 0.0f, label.c_str() );

        float const wide = ( nameSz.x > labelSz.x ? nameSz.x : labelSz.x ) + 96.0f * scale;
        float const tall = drawn + smallSize + 58.0f * scale;
        m_Size = ImVec2( wide, tall );

        float const y0 = center.y - tall * 0.5f + ( 1.0f - ease ) * 10.0f;
        float const fade = alpha * ease;

        // a wash behind it rather than a card, so the arena stays visible
        dl->AddRectFilled( ImVec2( center.x - wide * 0.5f, y0 ),
                           ImVec2( center.x + wide * 0.5f, y0 + tall ),
                           IM_COL32( 6, 7, 10, (int)( 170 * fade ) ), 10.0f * scale );

        dl->AddRectFilled( ImVec2( center.x - wide * 0.5f, y0 ),
                           ImVec2( center.x + wide * 0.5f, y0 + 2.0f * scale ),
                           rc_WithAlpha( accent, fade ) );
        dl->AddRectFilled( ImVec2( center.x - wide * 0.5f, y0 + tall - 2.0f * scale ),
                           ImVec2( center.x + wide * 0.5f, y0 + tall ),
                           rc_WithAlpha( accent, fade * 0.5f ) );

        dl->AddText( tinyFont, smallSize,
                     ImVec2( center.x - labelSz.x * 0.5f, y0 + 16.0f * scale ),
                     rc_WithAlpha( accent, fade * 0.9f ), label.c_str() );

        ImVec2 const namePos( center.x - nameSz.x * 0.5f, y0 + smallSize + 30.0f * scale );
        dl->AddText( nameFont, drawn, namePos + ImVec2( 2.0f * scale, 3.0f * scale ),
                     IM_COL32( 0, 0, 0, (int)( 190 * fade ) ), who.c_str() );
        dl->AddText( nameFont, drawn, namePos, rc_WithAlpha( IM_COL32( 255, 255, 255, 255 ), fade ), who.c_str() );
    }
    else if ( StripColors( msg.c_str() ).find( "Waiting" ) != std::string::npos ) {
        // === WAITING ===
        //
        // This one repeats for as long as nobody arrives, so it must not
        // announce itself each time. A quiet strip, out of the way, with a
        // breath in it to show the client has not simply frozen.
        std::string const bare = StripColors( msg.c_str() );
        std::string text = bare;
        while ( !text.empty() && ( text[text.size()-1] == '\n' || text[text.size()-1] == ' ' ) )
            text.erase( text.size()-1 );

        ImFont * font = GetBestFont( 18.0f * scale );
        float const size = font->FontSize > 0 ? font->FontSize : 18.0f * scale;
        ImVec2 const sz = font->CalcTextSizeA( size, FLT_MAX, 0.0f, text.c_str() );

        float const breathe = 0.72f + 0.28f * (float)( 0.5 + 0.5 * sin( now * 2.0 ) );

        float const padX = 26.0f * scale, padY = 12.0f * scale;
        m_Size = ImVec2( sz.x + padX * 2.0f, sz.y + padY * 2.0f );

        ImVec2 const at( center.x - m_Size.x * 0.5f, center.y - m_Size.y * 0.5f );
        ImVec2 const to( at.x + m_Size.x, at.y + m_Size.y );

        dl->AddRectFilled( at, to, IM_COL32( 8, 9, 12, (int)( 150 * alpha ) ), m_Size.y * 0.5f );
        dl->AddRect( at, to, IM_COL32( 60, 66, 78, (int)( 130 * alpha ) ), m_Size.y * 0.5f );

        dl->AddCircleFilled( ImVec2( at.x + padX * 0.62f, center.y ), 4.0f * scale,
                             rc_WithAlpha( IM_COL32( 120, 190, 255, 255 ), alpha * breathe ), 16 );

        dl->AddText( font, size, ImVec2( at.x + padX + 6.0f * scale, center.y - sz.y * 0.5f ),
                     rc_WithAlpha( IM_COL32( 214, 220, 232, 255 ), alpha * 0.92f ), text.c_str() );
    }
    else {
        // === BANNER/CARD STYLE ===
        std::string displayText = ConvertNonUtf8ToUtf8(msg);
        const char* cstr = displayText.c_str();

        float fontSize = 24.0f * scale;
        ImFont* font = GetBestFont(fontSize);
        if (!font) font = g_FontHeader ? g_FontHeader : ImGui::GetFont();

        ImVec2 textSz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, cstr);

        float padX = 40.0f * scale, padY = 20.0f * scale;
        float cardW = textSz.x + padX * 2.0f;
        float cardH = textSz.y + padY * 2.0f;

        // Make sure card meets minimum widget width
        if (cardW < m_Size.x) cardW = m_Size.x;
        m_Size.x = cardW;
        m_Size.y = cardH;

        ImVec2 cardMin(center.x - cardW * 0.5f, center.y - cardH * 0.5f);
        ImVec2 cardMax(center.x + cardW * 0.5f, center.y + cardH * 0.5f);

        // Draw shadow
        dl->AddRectFilled(cardMin + ImVec2(5.0f * scale, 5.0f * scale), cardMax + ImVec2(5.0f * scale, 5.0f * scale), IM_COL32(0, 0, 0, (int)(130 * alpha)), 12.0f * scale);

        // Draw background
        dl->AddRectFilled(cardMin, cardMax, IM_COL32(10, 10, 12, (int)(195 * alpha)), 12.0f * scale);

        // Draw RGB/Gradient border
        ImU32 borderL, borderR;
        if (false) {
            float hue = (float)now * 1.0f * 0.2f;
            float r1, g1, b1;
            float r2, g2, b2;
            ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 1.0f, r1, g1, b1);
            ImGui::ColorConvertHSVtoRGB(hue + 0.25f, 0.8f, 1.0f, r2, g2, b2);
            borderL = IM_COL32((int)(r1 * 255), (int)(g1 * 255), (int)(b1 * 255), (int)(alpha * 255));
            borderR = IM_COL32((int)(r2 * 255), (int)(g2 * 255), (int)(b2 * 255), (int)(alpha * 255));
        } else {
            borderL = IM_COL32(0, 190, 255, (int)(255 * alpha));
            borderR = IM_COL32(60, 66, 78, (int)(220 * alpha));
        }
        
        // Render top/bottom border lines
        dl->AddRect(cardMin, cardMax, borderL, 12.0f * scale, 0, 2.0f * scale);

        // Progress bar at the bottom showing lifetime left
        float lifePct = (float)(timeLeft / 3.0); // center display messages typically last 3 seconds max, adjust accordingly
        if (lifePct < 0.0f) lifePct = 0.0f;
        if (lifePct > 1.0f) lifePct = 1.0f;
        
        float progressW = cardW - 24.0f * scale;
        ImVec2 barMin(cardMin.x + 12.0f * scale, cardMax.y - 8.0f * scale);
        ImVec2 barMax(barMin.x + progressW * lifePct, cardMax.y - 4.0f * scale);
        dl->AddRectFilled(barMin, ImVec2(cardMin.x + 12.0f * scale + progressW, cardMax.y - 4.0f * scale), IM_COL32(40, 40, 50, (int)(100 * alpha)), 2.0f * scale);
        dl->AddRectFilledMultiColor(barMin, barMax, borderL, borderR, borderR, borderL);

        // Draw text
        ImVec2 textPos(center.x - textSz.x * 0.5f, center.y - textSz.y * 0.5f - 2.0f * scale);
        // Shadow text
        dl->AddText(font, fontSize, textPos + ImVec2(2.0f * scale, 2.0f * scale), IM_COL32(0, 0, 0, (int)(200 * alpha)), cstr);
        // Main text
        dl->AddText(font, fontSize, textPos, IM_COL32(240, 242, 248, (int)(255 * alpha)), cstr);
    }
}


// ---------------------------------------------------------------------------
// The line you type into.

//
// The old one was a menu: a label and a field drawn in the engine's bitmap
// font, sitting wherever the menu happened to put it. Everything about how it
// behaves is worth keeping - the history, the tab completion, the word delete
// - and none of how it looked. So the engine's own item still does the
// typing and simply hands over what it holds; the drawing happens here, in
// the same face and on the same footing as the chat above it.

bool sg_chatBar = true;
static tConfItem<bool> sg_chatBarConf( "MOD_CHAT_BAR", sg_chatBar );

namespace
{
struct Typing
{
    std::string text;
    int         cursor;
    bool        team;
    bool        live;
    double      since;
};

Typing s_typing = { "", 0, false, false, 0.0 };
}

bool rc_ChatInputSkinned()
{
    return sg_chatBar && sg_chatSkin && sr_glOut && ImGui::GetCurrentContext() != 0;
}

void rc_QueueChatInput( char const * text, int cursor, bool team )
{
    if ( !rc_ChatInputSkinned() )
        return;

    if ( !s_typing.live )
        s_typing.since = tRealSysTimeFloat();

    s_typing.text   = text ? text : "";
    s_typing.cursor = cursor;
    s_typing.team   = team;
    s_typing.live   = true;
}

//! Draws whatever was handed over this frame, then forgets it. The engine
//! calls in every frame the line is up, so a frame without a call means the
//! line is gone.
void rc_DrawChatInput()
{
    if ( !s_typing.live )
        return;

    s_typing.live = false;

    ImGuiIO & io = ImGui::GetIO();
    if ( io.DisplaySize.x < 1 || io.DisplaySize.y < 1 )
        return;

    ImDrawList * dl = ImGui::GetForegroundDrawList();

    float const wanted = sg_chatSize > 8 ? sg_chatSize : 15.0f;
    ImFont * font = sg_chatOurFont ? GetBestNameFont( wanted ) : GetBestFont( wanted );
    float const size = font->FontSize > 0 ? font->FontSize : wanted;

    float const pad = 14.0f;
    float const gap = 12.0f;

    char const * label = s_typing.team ? "TEAM" : "SAY";
    ImU32 const accent = s_typing.team ? IM_COL32( 120, 220, 150, 255 )
                                       : IM_COL32( 90, 190, 255, 255 );

    ImVec2 const labelSz = font->CalcTextSizeA( size, FLT_MAX, 0.0f, label );

    // Sitting on the chat, not somewhere else on the screen: what you are
    // writing belongs with what has been said.
    float const wide = sg_chatWide > 200 ? sg_chatWide : 620.0f;
    float const tall = size + pad * 2.0f;
    float const left = sg_chatLeft;
    float const top  = sg_chatTop + sg_chatTall + 10.0f;

    ImVec2 at( left, top );
    ImVec2 to( left + wide, top + tall );

    // a hair of drop-in, so it does not simply appear
    float const age = (float)( tRealSysTimeFloat() - s_typing.since );
    float const in = age < 0.12f ? age / 0.12f : 1.0f;
    at.y += ( 1.0f - in ) * 6.0f;
    to.y += ( 1.0f - in ) * 6.0f;

    dl->AddRectFilled( at, to, IM_COL32( 8, 10, 14, (int)( 232 * in ) ), 8.0f );
    dl->AddRect( at, to, IM_COL32( 42, 48, 60, (int)( 220 * in ) ), 8.0f );
    dl->AddRectFilled( at, ImVec2( at.x + 3.0f, to.y ), accent, 8.0f );

    float x = at.x + pad;
    float const y = at.y + pad;

    dl->AddText( font, size, ImVec2( x, y ), accent, label );
    x += labelSz.x + gap;

    dl->AddText( font, size, ImVec2( x, y ), IM_COL32( 236, 240, 248, 255 ), s_typing.text.c_str() );

    // the caret, where the engine says it is
    {
        int cursor = s_typing.cursor;
        if ( cursor < 0 ) cursor = 0;
        if ( cursor > (int)s_typing.text.size() ) cursor = (int)s_typing.text.size();

        std::string const before = s_typing.text.substr( 0, cursor );
        float const off = font->CalcTextSizeA( size, FLT_MAX, 0.0f, before.c_str() ).x;

        bool const on = fmod( tRealSysTimeFloat() - s_typing.since, 1.0 ) < 0.55;
        if ( on )
            dl->AddRectFilled( ImVec2( x + off, y ), ImVec2( x + off + 2.0f, y + size ),
                               IM_COL32( 236, 240, 248, 255 ) );
    }

    // how long it can still get, once it starts to matter
    {
        int const room = se_SpamMaxLen - (int)s_typing.text.size();
        if ( room <= 24 )
        {
            char left[ 16 ];
            snprintf( left, sizeof( left ), "%d", room );
            ImVec2 const sz = font->CalcTextSizeA( size, FLT_MAX, 0.0f, left );
            dl->AddText( font, size, ImVec2( to.x - pad - sz.x, y ),
                         room <= 8 ? IM_COL32( 236, 110, 110, 255 ) : IM_COL32( 150, 156, 168, 255 ),
                         left );
        }
    }
}

ChatSkinWidget::ChatSkinWidget()
    : HudWidget( "Chat", ImVec2( 24.0f, 620.0f ), ImVec2( 620.0f, 220.0f ),
                 WIDGET_CONFIG_PASS( sg_hudChat ) )
{
}

void ChatSkinWidget::Draw()
{
    float const scale = GetScale();
    ImVec2 const at = GetPosition() + m_SlideOffset;
    ImDrawList * dl = ImGui::GetBackgroundDrawList();

    bool const editing = isHudEditing;

    float const wide = sg_chatWide * scale;
    float const pad = 8.0f;

    // Something to aim at while the chat is quiet. Sizing a box against thin
    // air is guesswork, so the editor always has a few lines in it.
    static Row sampleRows[ 3 ];
    static bool sampleMade = false;

    std::vector< Row > * rows = 0;
    std::vector< Row > sample;

    ImFont * font = s_ready.font;
    float size = s_ready.size;
    float step = s_ready.step;

    if ( s_ready.have && !s_ready.rows.empty() && s_ready.font )
    {
        s_ready.have = false;
        rows = &s_ready.rows;
    }
    else if ( editing )
    {
        float const wanted = sg_chatSize;
        font = sg_chatOurFont ? GetBestNameFont( wanted ) : GetBestFont( wanted );
        size = font->FontSize > 0 ? font->FontSize : wanted;
        step = size * 1.2f;

        char const * lines[ 3 ] =
        {
            "12:00 somebody entered the game.",
            "12:00 *DEAD* somebody: watch the left wall",
            "12:00 Go (round 1 of 10)!"
        };

        for ( int i = 0; i < 3; ++i )
        {
            Row row;
            Piece piece;
            piece.text = lines[i];
            piece.colour = IM_COL32( 236, 240, 248, 255 );
            row.pieces.push_back( piece );
            sample.push_back( row );
        }

        rows = &sample;
        (void)sampleRows; (void)sampleMade;
    }

    if ( !rows || rows->empty() || !font )
        return;

    // How many lines the box is allowed to be, and never any taller than what
    // there is to show: the old chat sat in its corner with the first line
    // right against it, and an empty strip between the corner and the text is
    // exactly what it never had.
    int const cap = (int)( ( sg_chatTall * scale - pad * 2.0f ) / step );
    int const room = cap > 1 ? cap : 1;

    int first = (int)rows->size() - room;
    if ( first < 0 )
        first = 0;

    int const shown = (int)rows->size() - first;
    float const tall = shown * step + pad * 2.0f;

    m_Size = ImVec2( wide, tall );

    float shade = sg_chatShade;
    if ( editing && shade < 0.25f )
        shade = 0.25f;

    if ( shade > 0.001f )
    {
        dl->AddRectFilled( at, at + m_Size,
                           ImGui::GetColorU32( ImVec4( 0.03f, 0.035f, 0.05f, shade ) ), 7.0f );
        dl->AddRect( at, at + m_Size,
                     ImGui::GetColorU32( ImVec4( 0.22f, 0.24f, 0.30f, shade ) ), 7.0f );
    }

    // From the top down, oldest first, exactly the order the console prints in.
    float y = at.y + pad;

    for ( int r = first; r < (int)rows->size(); ++r )
    {
        float x = at.x + pad;

        for ( size_t p = 0; p < (*rows)[r].pieces.size(); ++p )
        {
            Piece const & piece = (*rows)[r].pieces[p];
            dl->AddText( font, size, ImVec2( x, y ), piece.colour, piece.text.c_str() );
            x += font->CalcTextSizeA( size, FLT_MAX, 0.0f, piece.text.c_str() ).x;
        }

        y += step;
    }
}

void ChatSkinWidget::DrawCustomSettings( bool & isDirty )
{
    ImGui::TextColored( ImVec4( 0.0f, 0.75f, 1.0f, 1.0f ), "Chat" );

    float wide = (float)sg_chatWide;
    if ( ImGui::SliderFloat( "Width", &wide, 260.0f, 1200.0f, "%.0f px" ) )
    {
        sg_chatWide = (REAL)wide;
        isDirty = true;
    }

    float tall = (float)sg_chatTall;
    if ( ImGui::SliderFloat( "Height limit", &tall, 60.0f, 700.0f, "%.0f px" ) )
    {
        sg_chatTall = (REAL)tall;
        isDirty = true;
    }

    float size = (float)sg_chatSize;
    if ( ImGui::SliderFloat( "Font Size", &size, 10.0f, 28.0f, "%.0f px" ) )
    {
        sg_chatSize = (REAL)size;
        isDirty = true;
    }

    float shade = (float)sg_chatShade;
    if ( ImGui::SliderFloat( "Background", &shade, 0.0f, 1.0f, "%.2f" ) )
    {
        sg_chatShade = (REAL)shade;
        isDirty = true;
    }

    bool ours = sg_chatOurFont;
    if ( ImGui::Checkbox( "Our font", &ours ) )
    {
        sg_chatOurFont = ours;
        isDirty = true;
    }
    if ( ImGui::IsItemHovered() )
        ImGui::SetTooltip( "Expressway, the face the names over the cycles use.\nOff falls back to the interface font." );

    bool stamp = sg_chatStamp;
    if ( ImGui::Checkbox( "Time before each line", &stamp ) )
    {
        sg_chatStamp = stamp;
        isDirty = true;
    }
}


#endif