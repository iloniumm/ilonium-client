// =====================================================================
// MODERN MODULAR HUD & EDIT MODE SYSTEM
// RetroCycles Client Mod | "ilonium"
// =====================================================================
#ifndef DEDICATED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "thirdparty/imgui/imgui_internal.h"
#include "HudManager.h"
#include "MediaWidget.h"
#include "ePlayer.h"
#include "eTimer.h"
#include "gCycle.h"
#include "gWinZone.h"
#include "eGrid.h"
#include "eTeam.h"
#include <SDL3/SDL.h>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>
#include "tools/tConfiguration.h"
#include "tron/DemoRecorder.h"
#include "tron/DemoPlayer.h"
#include "rConsole.h"
#include "engine/eRectangle.h"
#include "engine/eAdvWall.h"
#include "tron/gWall.h"

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
bool sg_modChatWidgetEnabled = true;
REAL sg_modChatWidgetPosX = 20.0f;
REAL sg_modChatWidgetPosY = 400.0f;
REAL sg_modChatWidth = 400.0f;
REAL sg_modChatHeight = 220.0f;
REAL sg_modChatFontSize = 14.0f;
REAL sg_modChatTimeout = 8.0f;
bool sg_modChatShowTime = true;

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
DEFINE_WIDGET_VARS(sg_hudChat)
DEFINE_WIDGET_VARS(sg_hudMinimap)
DEFINE_WIDGET_VARS(sg_hudFortressAlerts)
DEFINE_WIDGET_VARS(sg_hudCutoffPredictor)
DEFINE_WIDGET_VARS(sg_hudProximityWarning)
DEFINE_WIDGET_VARS(sg_hudTeammateDeath)
DEFINE_WIDGET_VARS(sg_hudWallTimer)
DEFINE_WIDGET_VARS(sg_hudKeystroke1)
DEFINE_WIDGET_VARS(sg_hudKeystroke2)

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
static tConfItem<bool> sg_modChatWidgetEnabledConf("MOD_CHAT_WIDGET_ENABLED", sg_modChatWidgetEnabled);
static tConfItem<REAL> sg_modChatWidgetPosXConf("MOD_CHAT_POS_X", sg_modChatWidgetPosX);
static tConfItem<REAL> sg_modChatWidgetPosYConf("MOD_CHAT_POS_Y", sg_modChatWidgetPosY);
static tConfItem<REAL> sg_modChatWidthConf("MOD_CHAT_WIDTH", sg_modChatWidth);
static tConfItem<REAL> sg_modChatHeightConf("MOD_CHAT_HEIGHT", sg_modChatHeight);
static tConfItem<REAL> sg_modChatFontSizeConf("MOD_CHAT_FONTSIZE", sg_modChatFontSize);
static tConfItem<REAL> sg_modChatTimeoutConf("MOD_CHAT_TIMEOUT", sg_modChatTimeout);
static tConfItem<bool> sg_modChatShowTimeConf("MOD_CHAT_SHOWTIME", sg_modChatShowTime);

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
REGISTER_WIDGET_CONFIGS("CUTOFFPREDICTOR", sg_hudCutoffPredictor)
REGISTER_WIDGET_CONFIGS("PROXIMITYWARNING", sg_hudProximityWarning)
REGISTER_WIDGET_CONFIGS("TEAMMATEDEATH", sg_hudTeammateDeath)
REGISTER_WIDGET_CONFIGS("WALLTIMER", sg_hudWallTimer)
REGISTER_WIDGET_CONFIGS("KEYSTROKE1", sg_hudKeystroke1)
REGISTER_WIDGET_CONFIGS("KEYSTROKE2", sg_hudKeystroke2)

REAL sg_modFortressAlertsPosX = 20.0f;
REAL sg_modFortressAlertsPosY = 140.0f;
REAL sg_modCutoffPredictorPosX = 20.0f;
REAL sg_modCutoffPredictorPosY = 210.0f;
REAL sg_modProximityWarningPosX = 20.0f;
REAL sg_modProximityWarningPosY = 280.0f;
REAL sg_modTeammateDeathPosX = 20.0f;
REAL sg_modTeammateDeathPosY = 350.0f;

static tConfItem<REAL> sg_modFortressAlertsPosXConf("MOD_FORTRESS_ALERTS_POS_X", sg_modFortressAlertsPosX);
static tConfItem<REAL> sg_modFortressAlertsPosYConf("MOD_FORTRESS_ALERTS_POS_Y", sg_modFortressAlertsPosY);
static tConfItem<REAL> sg_modCutoffPredictorPosXConf("MOD_CUTOFF_PREDICTOR_POS_X", sg_modCutoffPredictorPosX);
static tConfItem<REAL> sg_modCutoffPredictorPosYConf("MOD_CUTOFF_PREDICTOR_POS_Y", sg_modCutoffPredictorPosY);
static tConfItem<REAL> sg_modProximityWarningPosXConf("MOD_PROXIMITY_WARNING_POS_X", sg_modProximityWarningPosX);
static tConfItem<REAL> sg_modProximityWarningPosYConf("MOD_PROXIMITY_WARNING_POS_Y", sg_modProximityWarningPosY);
static tConfItem<REAL> sg_modTeammateDeathPosXConf("MOD_TEAMMATE_DEATH_POS_X", sg_modTeammateDeathPosX);
static tConfItem<REAL> sg_modTeammateDeathPosYConf("MOD_TEAMMATE_DEATH_POS_Y", sg_modTeammateDeathPosY);

bool sg_modWallTimerEnabled = true;
REAL sg_modWallTimerPosX = 800.0f;
REAL sg_modWallTimerPosY = 200.0f;

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
extern bool g_CameraLock;
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
extern int g_CameraLockKeybind;
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
        pos = ImVec2(sg_modChatWidgetPosX, sg_modChatWidgetPosY);
    } else if (m_Name == "Minimap") {
        pos = ImVec2(sg_modMinimapPosX, sg_modMinimapPosY);
    } else if (m_Name == "FortressAlerts") {
        pos = ImVec2(sg_modFortressAlertsPosX, sg_modFortressAlertsPosY);
    } else if (m_Name == "CutoffPredictor") {
        pos = ImVec2(sg_modCutoffPredictorPosX, sg_modCutoffPredictorPosY);
    } else if (m_Name == "ProximityWarning") {
        pos = ImVec2(sg_modProximityWarningPosX, sg_modProximityWarningPosY);
    } else if (m_Name == "TeammateDeath") {
        pos = ImVec2(sg_modTeammateDeathPosX, sg_modTeammateDeathPosY);
    } else if (m_Name == "WallTimer") {
        pos = ImVec2(sg_modWallTimerPosX, sg_modWallTimerPosY);
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
        sg_modChatWidgetPosX = pos.x;
        sg_modChatWidgetPosY = pos.y;
    } else if (m_Name == "Minimap") {
        sg_modMinimapPosX = pos.x;
        sg_modMinimapPosY = pos.y;
    } else if (m_Name == "FortressAlerts") {
        sg_modFortressAlertsPosX = pos.x;
        sg_modFortressAlertsPosY = pos.y;
    } else if (m_Name == "CutoffPredictor") {
        sg_modCutoffPredictorPosX = pos.x;
        sg_modCutoffPredictorPosY = pos.y;
    } else if (m_Name == "ProximityWarning") {
        sg_modProximityWarningPosX = pos.x;
        sg_modProximityWarningPosY = pos.y;
    } else if (m_Name == "TeammateDeath") {
        sg_modTeammateDeathPosX = pos.x;
        sg_modTeammateDeathPosY = pos.y;
    } else if (m_Name == "WallTimer") {
        sg_modWallTimerPosX = pos.x;
        sg_modWallTimerPosY = pos.y;
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
        return sg_modChatWidgetEnabled;
    } else if (m_Name == "Minimap") {
        return sg_modMinimapEnabled;
    } else if (m_Name == "FortressAlerts") {
        extern bool sg_modFortressAlerts;
        return sg_modFortressAlerts;
    } else if (m_Name == "CutoffPredictor") {
        extern bool sg_modCutoffAimbot;
        return sg_modCutoffAimbot;
    } else if (m_Name == "ProximityWarning") {
        extern bool sg_modProximityWarning;
        return sg_modProximityWarning;
    } else if (m_Name == "TeammateDeath") {
        extern bool sg_modTeammateDeathWarning;
        return sg_modTeammateDeathWarning;
    } else if (m_Name == "WallTimer") {
        return sg_modWallTimerEnabled;
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
        sg_modChatWidgetEnabled = visible;
    } else if (m_Name == "Minimap") {
        sg_modMinimapEnabled = visible;
    } else if (m_Name == "FortressAlerts") {
        extern bool sg_modFortressAlerts;
        sg_modFortressAlerts = visible;
    } else if (m_Name == "CutoffPredictor") {
        extern bool sg_modCutoffAimbot;
        sg_modCutoffAimbot = visible;
    } else if (m_Name == "ProximityWarning") {
        extern bool sg_modProximityWarning;
        sg_modProximityWarning = visible;
    } else if (m_Name == "TeammateDeath") {
        extern bool sg_modTeammateDeathWarning;
        sg_modTeammateDeathWarning = visible;
    } else if (m_Name == "WallTimer") {
        sg_modWallTimerEnabled = visible;
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
    addIfActive("Camera Lock", g_CameraLock, g_CameraLockKeybind);
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
extern bool sg_demoRecorderOverlayEnabled;   // defined below
bool        sg_demoRecorderOverlayEnabled = true;
static tConfItem<bool> sg_demoRecorderOverlayEnabledConf(
    "MOD_DEMO_OVERLAY_ENABLED", sg_demoRecorderOverlayEnabled);

static void RenderRecordingOverlay(ImDrawList* bgDl, const ImGuiIO& io) {
    if (!sg_demoRecorderOverlayEnabled) return;

    DemoRecorder& rec = DemoRecorder::Instance();
    if (!rec.IsRecording()) return;

    const DemoRecorderStats& st = rec.Stats();

    // ── Timings ──────────────────────────────────────────────────────────
    float t        = (float)ImGui::GetTime();
    double dur     = st.duration.load(std::memory_order_relaxed);
    int dur_min    = (int)(dur / 60.0);
    int dur_sec    = (int)(dur) % 60;
    uint64_t bytes = st.bytesWritten.load(std::memory_order_relaxed);
    double mb      = (double)bytes / (1024.0 * 1024.0);

    // ── Blinking REC dot alpha  (0.35 – 1.0, period ~1.4 s) ─────────────
    float dotAlpha = 0.35f + 0.65f * ((sinf(t * 4.6f) + 1.0f) * 0.5f);

    // ── Build text strings ───────────────────────────────────────────────
    char timerBuf[16];
    snprintf(timerBuf, sizeof(timerBuf), "%02d:%02d", dur_min, dur_sec);

    char sizeBuf[16];
    if (mb >= 1.0)
        snprintf(sizeBuf, sizeof(sizeBuf), "%.1f MB", mb);
    else
        snprintf(sizeBuf, sizeof(sizeBuf), "%llu KB", (unsigned long long)(bytes / 1024));

    // ── Geometry ─────────────────────────────────────────────────────────
    // Use a fixed font size regardless of HUD scale setting
    ImFont*  font     = g_FontHeader ? g_FontHeader : ImGui::GetFont();
    float    fontSize = ImGui::GetFontSize() * 0.90f;

    const char* recLabel = " REC";
    float recLabelW  = font->CalcTextSizeA(fontSize, FLT_MAX, -1.f, recLabel).x;
    float timerW     = font->CalcTextSizeA(fontSize, FLT_MAX, -1.f, timerBuf).x;
    float sizeW      = font->CalcTextSizeA(fontSize, FLT_MAX, -1.f, sizeBuf).x;

    float dotRadius  = fontSize * 0.35f;
    float padX       = 10.0f;
    float padY       = 7.0f;
    float sepW       = 10.0f;  // separator between sections

    // Total pill width: dotR*2 + recLabelW + sep + timerW + sep + sizeW
    float pillW = dotRadius * 2.0f + recLabelW + sepW + timerW + sepW + sizeW + padX * 2.0f;
    float pillH = fontSize + padY * 2.0f;

    // Position: top-right, 14 px from edge
    float margin = 14.0f;
    float px     = io.DisplaySize.x - pillW - margin;
    float py     = margin;

    ImVec2 pillMin(px, py);
    ImVec2 pillMax(px + pillW, py + pillH);

    // ── Draw pill background ─────────────────────────────────────────────
    // Semi-transparent dark capsule
    bgDl->AddRectFilled(pillMin, pillMax, IM_COL32(8, 8, 10, 210), pillH * 0.5f);
    // Subtle border that pulses slightly with the dot
    uint8_t borderA = (uint8_t)(60 + (int)(dotAlpha * 60));
    bgDl->AddRect(pillMin, pillMax, IM_COL32(200, 40, 40, borderA), pillH * 0.5f, 0, 1.0f);

    // ── Draw blinking dot ─────────────────────────────────────────────────
    float curX = px + padX + dotRadius;
    float midY = py + padY + fontSize * 0.5f;
    uint8_t dotA = (uint8_t)(dotAlpha * 255.0f);

    // Outer glow ring
    bgDl->AddCircleFilled(
        ImVec2(curX, midY),
        dotRadius * 1.9f,
        IM_COL32(220, 30, 30, (uint8_t)(dotAlpha * 55.0f)));
    // Inner solid dot
    bgDl->AddCircleFilled(
        ImVec2(curX, midY),
        dotRadius,
        IM_COL32(255, 45, 45, dotA));

    curX += dotRadius + 2.0f;

    // ── "REC" label ───────────────────────────────────────────────────────
    bgDl->AddText(font, fontSize,
        ImVec2(curX, py + padY),
        IM_COL32(230, 230, 235, (uint8_t)(dotAlpha * 0.75f * 255.0f + 60)),
        recLabel);
    curX += recLabelW + sepW;

    // ── Divider ───────────────────────────────────────────────────────────
    bgDl->AddLine(
        ImVec2(curX - sepW * 0.5f, py + padY + 2.0f),
        ImVec2(curX - sepW * 0.5f, py + pillH - padY - 2.0f),
        IM_COL32(80, 80, 90, 100));

    // ── Timer ─────────────────────────────────────────────────────────────
    bgDl->AddText(font, fontSize,
        ImVec2(curX, py + padY),
        IM_COL32(255, 255, 255, 245),
        timerBuf);
    curX += timerW + sepW;

    // ── Divider ───────────────────────────────────────────────────────────
    bgDl->AddLine(
        ImVec2(curX - sepW * 0.5f, py + padY + 2.0f),
        ImVec2(curX - sepW * 0.5f, py + pillH - padY - 2.0f),
        IM_COL32(80, 80, 90, 100));

    // ── File size ─────────────────────────────────────────────────────────
    bgDl->AddText(font, fontSize,
        ImVec2(curX, py + padY),
        IM_COL32(140, 210, 255, 220),
        sizeBuf);
}

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
    s_Widgets.push_back(new ChatWidget());
    s_Widgets.push_back(new MinimapWidget());
    s_Widgets.push_back(new FortressAlertsWidget());
    s_Widgets.push_back(new CutoffPredictorWidget());
    s_Widgets.push_back(new ProximityWarningWidget());
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
    if (!s_Initialized) return;

    static bool checkedEnv = false;
    if (!checkedEnv) {
        checkedEnv = true;
        if (getenv("RETRO_TEST_HUD_EDIT")) {
            isHudEditing = true;
        }
    }

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

void HudManager::Render() {
    if (!s_Initialized) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImDrawList* bgDl = ImGui::GetBackgroundDrawList();

    // 1. Draw each widget
    for (auto* widget : s_Widgets) {
        widget->Draw();
    }

    // 1b. Recording overlay (drawn after widgets, always on top of them)
    RenderRecordingOverlay(bgDl, io);

    // 1c. Demo Player HUD — only shown while the viewer loop is active
    {
        auto& dp = DemoPlayerManager::Instance();
        if (dp.IsViewerActive() && dp.IsLoaded()) {
            dp.RenderHUD();
        }
    }

    // 2. HUD Editor Mode interactions
    if (isHudEditing) {
        static HudWidget* selectedWidget = nullptr;
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

            if (hovered) {
                if (mouseClicked) {
                    selectedWidget = widget;
                } else if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
                    selectedWidget = widget;
                }
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
            if (mouseClicked && hovered) {
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
                }

                if (mouseReleased) {
                    widget->SetDragging(false);
                }
            }
        }

        // --- FLOATING INTERACTIVE CUSTOMIZER PANEL ---
        static bool isDirty = false;
        static float lastSaveTime = 0.0f;

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

            // Combo box to select active widget
            std::vector<const char*> widgetNames;
            int currentIdx = -1;
            for (int i = 0; i < (int)s_Widgets.size(); i++) {
                if (s_Widgets[i]->IsVisible()) {
                    widgetNames.push_back(s_Widgets[i]->GetName().c_str());
                    if (s_Widgets[i] == selectedWidget) {
                        currentIdx = (int)widgetNames.size() - 1;
                    }
                }
            }
            if (!widgetNames.empty()) {
                if (ImGui::Combo("Select Widget", &currentIdx, widgetNames.data(), (int)widgetNames.size())) {
                    std::string selName = widgetNames[currentIdx];
                    for (auto* widget : s_Widgets) {
                        if (widget->GetName() == selName) {
                            selectedWidget = widget;
                            break;
                        }
                    }
                }
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }

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
            if (p->Object() && p->Object()->Alive()) {
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
                se.alive = p->Object() && p->Object()->Alive();
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
                se.alive = p->Object() && p->Object()->Alive();
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
// ChatWidget Implementation
// =====================================================================

struct CachedMessage {
    std::string text;
    float timestamp;
    float alpha;
    float currentAlpha;
    float slideY;
};
static std::vector<CachedMessage> g_chatMessages;
static int lastCheckedLineCount = 0;

static bool IsTypingActive() {
    extern int sn_myNetID;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID* p = se_PlayerNetIDs(i);
        if (p && p->Owner() == sn_myNetID && p->IsChatting()) {
            return true;
        }
    }
    return false;
}

ChatWidget::ChatWidget()
    : HudWidget("Chat", ImVec2(20.0f, 400.0f), ImVec2(400.0f, 220.0f), WIDGET_CONFIG_PASS(sg_hudChat)) {
}

void ChatWidget::Update(float dt) {
    HudWidget::Update(dt);
    for (auto& msg : g_chatMessages) {
        msg.currentAlpha = ImLerp(msg.currentAlpha, 1.0f, dt * 8.0f);
        msg.slideY = ImLerp(msg.slideY, 0.0f, dt * 8.0f);
    }
}

static void RenderColoredText(const char* text, ImU32 defaultColor, float alpha, float wrapWidth, bool endWithNewLine = true) {
    struct Token {
        bool isColor;
        std::string text;
        ImU32 color;
    };
    std::vector<Token> tokens;
    
    ImU32 currentColor = defaultColor;
    const char* p = text;
    const char* start = p;
    
    auto toUpperChar = [](char c) {
        return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
    };
    
    while (*p) {
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            // Check for 0xRESETT (case-insensitive, 8 chars)
            bool isResetT = true;
            const char* resetTStr = "RESETT";
            for (int i = 0; i < 6; i++) {
                if (p[2 + i] == '\0' || toUpperChar(p[2 + i]) != resetTStr[i]) {
                    isResetT = false;
                    break;
                }
            }
            if (isResetT) {
                if (p > start) {
                    Token t;
                    t.isColor = false;
                    t.text = std::string(start, p - start);
                    tokens.push_back(t);
                }
                Token t;
                t.isColor = true;
                t.color = defaultColor;
                tokens.push_back(t);
                p += 8;
                start = p;
                continue;
            }
            
            // Check for 0xRESET (case-insensitive, 7 chars)
            bool isReset = true;
            const char* resetStr = "RESET";
            for (int i = 0; i < 5; i++) {
                if (p[2 + i] == '\0' || toUpperChar(p[2 + i]) != resetStr[i]) {
                    isReset = false;
                    break;
                }
            }
            if (isReset) {
                if (p > start) {
                    Token t;
                    t.isColor = false;
                    t.text = std::string(start, p - start);
                    tokens.push_back(t);
                }
                Token t;
                t.isColor = true;
                t.color = defaultColor;
                tokens.push_back(t);
                p += 7;
                start = p;
                continue;
            }

            // Check for 0xRRGGBB
            bool isHex = true;
            for (int i = 2; i < 8; i++) {
                char c = p[i];
                if (c == '\0' || !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                    isHex = false;
                    break;
                }
            }
            if (isHex) {
                if (p > start) {
                    Token t;
                    t.isColor = false;
                    t.text = std::string(start, p - start);
                    tokens.push_back(t);
                }
                
                unsigned int r, g, b;
                char hexStr[7];
                memcpy(hexStr, p + 2, 6);
                hexStr[6] = '\0';
                sscanf(hexStr, "%02x%02x%02x", &r, &g, &b);
                
                Token t;
                t.isColor = true;
                t.color = IM_COL32(r, g, b, 255);
                tokens.push_back(t);
                
                p += 8;
                start = p;
                continue;
            }
        }
        p++;
    }
    if (p > start) {
        Token t;
        t.isColor = false;
        t.text = std::string(start, p - start);
        tokens.push_back(t);
    }
    
    float startX = ImGui::GetCursorPosX();
    currentColor = defaultColor;
    
    auto isSpaceChar = [](char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };
    
    for (const auto& token : tokens) {
        if (token.isColor) {
            currentColor = token.color;
        } else {
            const std::string& textStr = token.text;
            size_t idx = 0;
            while (idx < textStr.size()) {
                size_t endIdx = idx;
                if (isSpaceChar(textStr[idx])) {
                    while (endIdx < textStr.size() && isSpaceChar(textStr[endIdx])) {
                        endIdx++;
                    }
                } else {
                    while (endIdx < textStr.size() && !isSpaceChar(textStr[endIdx])) {
                        endIdx++;
                    }
                }
                
                std::string word = textStr.substr(idx, endIdx - idx);
                idx = endIdx;
                
                float wordWidth = ImGui::CalcTextSize(word.c_str()).x;
                float cursorX = ImGui::GetCursorPosX();
                float availWidth = wrapWidth - (cursorX - startX);
                
                if (cursorX > startX && wordWidth > availWidth) {
                    ImGui::NewLine();
                    if (word.size() > 0 && isSpaceChar(word[0])) {
                        continue;
                    }
                }
                
                ImVec4 colVec = ImGui::ColorConvertU32ToFloat4(currentColor);
                colVec.w *= alpha;
                
                ImGui::PushStyleColor(ImGuiCol_Text, colVec);
                ImGui::TextUnformatted(word.c_str());
                ImGui::PopStyleColor();
                ImGui::SameLine(0.0f, 0.0f);
            }
        }
    }
    if (endWithNewLine) {
        ImGui::NewLine();
    }
}

void ChatWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImVec2 pos = GetPosition();
    float scale = GetScale();
    
    extern REAL sg_modChatWidth;
    extern REAL sg_modChatHeight;
    extern REAL sg_modChatFontSize;
    extern REAL sg_modChatTimeout;
    extern bool sg_modChatShowTime;

    m_Size = ImVec2(sg_modChatWidth * scale, sg_modChatHeight * scale);
    ImVec2 size = m_Size;
    ImVec2 drawPos = pos + m_SlideOffset;

    bool typing = IsTypingActive();
    bool editing = isHudEditing;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    
    float bgOpacity = GetBgOpacitySetting();
    if (typing || editing) {
        bgOpacity = std::max(bgOpacity, 0.6f);
    }

    if (bgOpacity > 0.001f) {
        ImU32 bgCol = GetBgCol();
        ImVec4 bgColVec = ImGui::ColorConvertU32ToFloat4(bgCol);
        bgColVec.w *= bgOpacity;
        dl->AddRectFilled(drawPos, drawPos + size, ImGui::ColorConvertFloat4ToU32(bgColVec), 8.0f * scale);
        
        ImU32 borderCol = GetBorderCol();
        ImVec4 borderColVec = ImGui::ColorConvertU32ToFloat4(borderCol);
        borderColVec.w *= (typing || editing) ? 1.0f : m_Alpha;
        dl->AddRect(drawPos, drawPos + size, ImGui::ColorConvertFloat4ToU32(borderColVec), 8.0f * scale, 0, 1.0f);
    }

    // Capture console messages in real time
    int currentLineCount = sr_con.GetLineCount();
    if (currentLineCount > lastCheckedLineCount) {
        if (currentLineCount < lastCheckedLineCount) {
            lastCheckedLineCount = 0;
        }
        for (int i = lastCheckedLineCount; i < currentLineCount; i++) {
            tString const& rawLine = sr_con.GetLine(i);
            std::string lineStr = static_cast<const char*>(rawLine);
            
            CachedMessage msg;
            msg.text = lineStr;
            msg.timestamp = (float)ImGui::GetTime();
            msg.alpha = 1.0f;
            msg.currentAlpha = 0.0f;
            msg.slideY = 15.0f;
            g_chatMessages.push_back(msg);
        }
        lastCheckedLineCount = currentLineCount;
    }

    if (g_chatMessages.size() > 1000) {
        g_chatMessages.erase(g_chatMessages.begin(), g_chatMessages.end() - 1000);
    }

    // Set up window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
    
    if (!typing) {
        flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing;
    } else {
        flags &= ~ImGuiWindowFlags_NoScrollbar;
    }

    ImGui::SetNextWindowPos(drawPos + ImVec2(6.0f * scale, 6.0f * scale));
    ImGui::SetNextWindowSize(size - ImVec2(12.0f * scale, 12.0f * scale));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    char winName[64];
    snprintf(winName, sizeof(winName), "##ChatWindow_%p", this);

    if (ImGui::Begin(winName, nullptr, flags)) {
        std::vector<CachedMessage> visibleMsgs;
        float curTime = (float)ImGui::GetTime();
        
        for (const auto& msg : g_chatMessages) {
            float elapsed = curTime - msg.timestamp;
            float msgAlpha = 1.0f;
            if (!typing && !editing) {
                if (elapsed > sg_modChatTimeout) {
                    msgAlpha = 1.0f - (elapsed - sg_modChatTimeout) / 1.0f;
                    if (msgAlpha < 0.0f) msgAlpha = 0.0f;
                }
            }
            if (msgAlpha > 0.01f || typing || editing) {
                CachedMessage displayMsg = msg;
                displayMsg.alpha = msgAlpha;
                visibleMsgs.push_back(displayMsg);
            }
        }

        if (visibleMsgs.size() > 1000) {
            visibleMsgs.erase(visibleMsgs.begin(), visibleMsgs.end() - 1000);
        }

        if (editing && visibleMsgs.empty()) {
            CachedMessage p1 = { "System: Welcome to retrocycles mod menu!", curTime, 1.0f, 1.0f, 0.0f };
            CachedMessage p2 = { "Player1: Good luck!", curTime, 1.0f, 1.0f, 0.0f };
            CachedMessage p3 = { "Player2: Have fun!", curTime, 1.0f, 1.0f, 0.0f };
            visibleMsgs.push_back(p1);
            visibleMsgs.push_back(p2);
            visibleMsgs.push_back(p3);
        }

        float originalFontSize = ImGui::GetFontSize();
        float targetFontSize = sg_modChatFontSize * scale;
        ImGui::SetWindowFontScale(targetFontSize / originalFontSize);

        for (const auto& msg : visibleMsgs) {
            std::string converted = ConvertNonUtf8ToUtf8(msg.text);
            
            float finalAlpha = msg.alpha * msg.currentAlpha * m_Alpha;
            if (finalAlpha <= 0.001f) continue;
            
            // Push vertical slide
            if (msg.slideY > 0.1f) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + msg.slideY);
            }
            
            ImU32 textCol = GetTextCol();
            
            if (sg_modChatShowTime) {
                time_t rawtime;
                struct tm * timeinfo;
                char timeBuf[12];
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(timeBuf, sizeof(timeBuf), "[%H:%M:%S] ", timeinfo);
                
                ImVec4 timeCol(0.5f, 0.5f, 0.5f, finalAlpha);
                ImGui::TextColored(timeCol, "%s", timeBuf);
                ImGui::SameLine(0.0f, 0.0f);
            }
            
            ImU32 msgColor = textCol;
            bool isSystem = (converted.rfind("* ", 0) == 0);
            bool isDeath = (converted.find("crashed into") != std::string::npos ||
                            converted.find("has been exterminated") != std::string::npos ||
                            converted.find("died") != std::string::npos ||
                            converted.find("killed") != std::string::npos ||
                            converted.find("suicide") != std::string::npos);
            
            if (isSystem) {
                msgColor = IM_COL32(255, 200, 50, 255); // Gold/Yellow
            } else if (isDeath) {
                msgColor = IM_COL32(255, 90, 90, 255);   // Red/Orange
            }
            
            size_t colonPos = converted.find(": ");
            if (!isSystem && !isDeath && colonPos != std::string::npos && colonPos > 0) {
                std::string nickname = converted.substr(0, colonPos);
                std::string messageBody = converted.substr(colonPos);
                
                RenderColoredText(nickname.c_str(), IM_COL32(0, 210, 255, 255), finalAlpha, ImGui::GetContentRegionAvail().x, false);
                RenderColoredText(messageBody.c_str(), textCol, finalAlpha, ImGui::GetContentRegionAvail().x, true);
            } else {
                RenderColoredText(converted.c_str(), msgColor, finalAlpha, ImGui::GetContentRegionAvail().x, true);
            }
        }

        // Handle manual scrolling via PgUp / PgDn keys
        float scrollStep = 150.0f * scale;
        if (ImGui::IsKeyPressed(ImGuiKey_PageUp, true)) {
            ImGui::SetScrollY(ImGui::GetScrollY() - scrollStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown, true)) {
            ImGui::SetScrollY(ImGui::GetScrollY() + scrollStep);
        }

        static int prevMsgCount = 0;
        static bool lastTyping = false;
        int curMsgCount = g_chatMessages.size();
        if (curMsgCount > prevMsgCount || (typing && !lastTyping)) {
            ImGui::SetScrollHereY(1.0f);
        }
        prevMsgCount = curMsgCount;
        lastTyping = typing;
        
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

void ChatWidget::DrawCustomSettings(bool& isDirty) {
    ImGui::TextColored(ImVec4(0.0f, 0.75f, 1.0f, 1.0f), "Chat Settings");
    
    extern REAL sg_modChatWidth;
    extern REAL sg_modChatHeight;
    extern REAL sg_modChatFontSize;
    extern REAL sg_modChatTimeout;
    extern bool sg_modChatShowTime;

    float widthVal = (float)sg_modChatWidth;
    if (ImGui::SliderFloat("Width", &widthVal, 200.0f, 800.0f, "%.0f px")) {
        sg_modChatWidth = (REAL)widthVal;
        isDirty = true;
    }
    
    float heightVal = (float)sg_modChatHeight;
    if (ImGui::SliderFloat("Height", &heightVal, 100.0f, 600.0f, "%.0f px")) {
        sg_modChatHeight = (REAL)heightVal;
        isDirty = true;
    }
    
    float fontVal = (float)sg_modChatFontSize;
    if (ImGui::SliderFloat("Font Size", &fontVal, 8.0f, 32.0f, "%.0f px")) {
        sg_modChatFontSize = (REAL)fontVal;
        isDirty = true;
    }
    
    float timeoutVal = (float)sg_modChatTimeout;
    if (ImGui::SliderFloat("Message Timeout", &timeoutVal, 1.0f, 30.0f, "%.1fs")) {
        sg_modChatTimeout = (REAL)timeoutVal;
        isDirty = true;
    }
    
    bool showTime = sg_modChatShowTime;
    if (ImGui::Checkbox("Show Timestamps", &showTime)) {
        sg_modChatShowTime = showTime;
        isDirty = true;
    }
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
                bool isAlive = p->Object() && p->Object()->Alive();
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
        if (p && p->Object() && p->Object()->Alive()) {
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
// CutoffPredictorWidget Implementation
// ---------------------------------------------------------------------
CutoffPredictorWidget::CutoffPredictorWidget()
    : HudWidget("CutoffPredictor", ImVec2(20.0f, 210.0f), ImVec2(240.0f, 65.0f), WIDGET_CONFIG_PASS(sg_hudCutoffPredictor)) {
}

void CutoffPredictorWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    float w = 240.0f * scale;
    float h = 65.0f * scale;
    m_Size = ImVec2(w, h);

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 8.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 8.0f * scale, 0, 1.0f);

    gCycle* target = nullptr;
    ePlayerNetID* targetPlayer = nullptr;
    bool canCut = false;
    bool isRight = false;
    float marginVal = 0.0f;
    std::string enemyName = "None";

    ePlayerNetID* me = nullptr;
    ePlayer* lp = ePlayer::PlayerConfig(0);
    if (lp) me = lp->netPlayer;

    if (me && me->Object() && me->Object()->Alive()) {
        gCycle* hCycle = dynamic_cast<gCycle*>(me->Object());
        if (hCycle) {
            float mySpeed = hCycle->Speed();
            if (mySpeed > 0.1f) {
                eCoord myPos = hCycle->Position();
                eCoord myDir = hCycle->Direction();
                eCoord myRight(myDir.y, -myDir.x);

                float bestScore = 99999.0f;
                for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
                    ePlayerNetID *p = se_PlayerNetIDs(i);
                    if (!p || p == me || !p->Object() || !p->Object()->Alive())
                        continue;
                    if (p->CurrentTeam() == me->CurrentTeam())
                        continue;
                    gCycle *enemy = dynamic_cast<gCycle*>(p->Object());
                    if (!enemy) continue;

                    float dot = myDir * enemy->Direction();
                    if (dot > 0.7f) {
                        eCoord delta = enemy->Position() - myPos;
                        float ahead = delta * myDir;
                        float sideDot = delta * myRight;
                        float sideDist = fabs(sideDot);

                        if (ahead < 5.0f && ahead > -25.0f && sideDist > 0.5f && sideDist < 15.0f) {
                            float score = sideDist * sideDist + ahead * ahead;
                            if (score < bestScore) {
                                bestScore = score;
                                target = enemy;
                                targetPlayer = p;
                            }
                        }
                    }
                }

                if (target && targetPlayer) {
                    enemyName = ConvertNonUtf8ToUtf8((const char*)targetPlayer->GetName());
                    eCoord delta = target->Position() - myPos;
                    float ahead = delta * myDir;
                    float sideDot = delta * myRight;
                    float sideDist = fabs(sideDot);

                    float t_cross = sideDist / mySpeed;
                    float t_enemy = (ahead < 0.0f) ? ((-ahead) / (target->Speed() + 0.001f)) : -1.0f;
                    float margin = hCycle->Lag() + target->Lag() + 0.06f;

                    isRight = (sideDot > 0.0f);
                    marginVal = t_enemy - (t_cross + margin);
                    if (ahead < 0.0f && t_enemy > t_cross + margin) {
                        canCut = true;
                    }
                }
            }
        }
    }

    if (isHudEditing && !target) {
        enemyName = "Opponent";
        canCut = true;
        isRight = true;
        marginVal = 0.45f;
    }

    float padding = 12.0f * scale;
    float textY = drawPos.y + 10.0f * scale;
    float detailY = drawPos.y + 36.0f * scale;

    if (enemyName == "None") {
        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY + 10.0f * scale),
                    IM_COL32(120, 120, 130, 180), "RADAR: NO TARGET");
        if (g_FontHeader) ImGui::PopFont();
        return;
    }

    std::string titleStr = "TARGET: " + enemyName;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), GetTextCol(), titleStr.c_str());

    char cutText[64];
    ImU32 indicatorCol;
    if (canCut) {
        snprintf(cutText, sizeof(cutText), isRight ? "CUT RIGHT >>>" : "<<< CUT LEFT");
        indicatorCol = GetColorWithAlpha(IM_COL32(50, 255, 100, 255));
    } else {
        snprintf(cutText, sizeof(cutText), isRight ? "HOLD RIGHT >>>" : "<<< HOLD LEFT");
        indicatorCol = GetColorWithAlpha(IM_COL32(255, 60, 60, 255));
    }

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, detailY), indicatorCol, cutText);
    if (g_FontHeader) ImGui::PopFont();

    char margBuf[32];
    if (marginVal > -10.0f && marginVal < 10.0f) {
        snprintf(margBuf, sizeof(margBuf), "+%.2fs", marginVal);
    } else {
        snprintf(margBuf, sizeof(margBuf), "N/A");
    }
    float margW = CalcTextSize(margBuf).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.9f * scale, ImVec2(drawPos.x + w - padding - margW, detailY + 2.0f * scale),
                canCut ? IM_COL32(50, 255, 100, 220) : IM_COL32(180, 180, 190, 180), margBuf);
}

// ---------------------------------------------------------------------
// ProximityWarningWidget Implementation
// ---------------------------------------------------------------------
ProximityWarningWidget::ProximityWarningWidget()
    : HudWidget("ProximityWarning", ImVec2(20.0f, 280.0f), ImVec2(220.0f, 60.0f), WIDGET_CONFIG_PASS(sg_hudProximityWarning)) {
}

void ProximityWarningWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    float nearestDistSqr = 99999.0f;
    std::string enemyName = "";
    ePlayerNetID* threatPlayer = nullptr;

    ePlayerNetID* me = nullptr;
    ePlayer* lp = ePlayer::PlayerConfig(0);
    if (lp) me = lp->netPlayer;

    if (me && me->Object() && me->Object()->Alive()) {
        gCycle* hCycle = dynamic_cast<gCycle*>(me->Object());
        if (hCycle) {
            eCoord myPos = hCycle->Position();
            for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
                ePlayerNetID *p = se_PlayerNetIDs(i);
                if (p && p != me && p->Object() && p->Object()->Alive()) {
                    gCycle *enemy = dynamic_cast<gCycle*>(p->Object());
                    if (enemy) {
                        eCoord diff = enemy->Position() - myPos;
                        float distSqr = diff.NormSquared();
                        if (distSqr < 400.0f) {
                            float dot = enemy->Direction() * diff;
                            if (dot < 0.0f) {
                                if (distSqr < nearestDistSqr) {
                                    nearestDistSqr = distSqr;
                                    threatPlayer = p;
                                }
                            }
                        }
                    }
                }
            }
            if (threatPlayer) {
                enemyName = ConvertNonUtf8ToUtf8((const char*)threatPlayer->GetName());
            }
        }
    }

    if (isHudEditing && enemyName.empty()) {
        enemyName = "RivalPlayer";
        nearestDistSqr = 100.0f;
    }

    if (enemyName.empty()) return;

    float distance = sqrtf(nearestDistSqr);
    float intensity = 1.0f - (nearestDistSqr / 400.0f);
    if (intensity > 1.0f) intensity = 1.0f;
    if (intensity < 0.0f) intensity = 0.0f;

    float w = 220.0f * scale;
    float h = 60.0f * scale;
    m_Size = ImVec2(w, h);

    dl->AddRectFilled(drawPos, drawPos + m_Size, GetBgCol(), 8.0f * scale);
    dl->AddRect(drawPos, drawPos + m_Size, GetBorderCol(), 8.0f * scale, 0, 1.0f);

    float padding = 12.0f * scale;
    float textY = drawPos.y + 10.0f * scale;
    float barY = drawPos.y + 36.0f * scale;
    float barH = 8.0f * scale;
    float barW = w - padding * 2.0f;

    float pulse = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 10.0f);
    ImU32 warnCol = GetColorWithAlpha(IM_COL32(255, 30, 30, (int)(180 + pulse * 75)));

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), warnCol, "PROXIMITY WARNING");
    if (g_FontHeader) ImGui::PopFont();

    char distBuf[32];
    snprintf(distBuf, sizeof(distBuf), "%.1fm", distance);
    float distW = CalcTextSize(distBuf).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + w - padding - distW, textY + 2.0f * scale), GetTextCol(), distBuf);

    dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW, barY + barH), IM_COL32(50, 10, 10, 150), 4.0f * scale);
    dl->AddRectFilled(ImVec2(drawPos.x + padding, barY), ImVec2(drawPos.x + padding + barW * intensity, barY + barH), warnCol, 4.0f * scale);

    if (intensity > 0.1f) {
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        float borderThickness = 12.0f * scale * intensity;
        ImU32 borderGlowCol = IM_COL32(255, 0, 0, (int)(intensity * 40.0f + pulse * 20.0f));
        dl->AddRect(ImVec2(0, 0), displaySize, borderGlowCol, 0.0f, 0, borderThickness);
    }
}

// ---------------------------------------------------------------------
// TeammateDeathWarningWidget Implementation
// ---------------------------------------------------------------------
TeammateDeathWarningWidget::TeammateDeathWarningWidget()
    : HudWidget("TeammateDeath", ImVec2(20.0f, 350.0f), ImVec2(240.0f, 60.0f), WIDGET_CONFIG_PASS(sg_hudTeammateDeath)) {
}

extern REAL sg_teammateDeathFlashTime;
extern tString sg_deadTeammateName;

void TeammateDeathWarningWidget::Draw() {
    if (m_Alpha <= 0.0f) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 pos = GetPosition();
    ImVec2 drawPos = pos + m_SlideOffset;
    float scale = GetScale();

    REAL gameTime = se_GameTime();
    REAL elapsed = gameTime - sg_teammateDeathFlashTime;
    bool active = (elapsed >= 0.0f && elapsed < 3.0f);
    std::string deadName = "";

    if (active) {
        deadName = ConvertNonUtf8ToUtf8((const char*)sg_deadTeammateName);
    }

    if (isHudEditing && !active) {
        active = true;
        deadName = "TeammateName";
    }

    if (!active || deadName.empty()) return;

    float alertAlpha = 1.0f;
    if (elapsed > 2.0f) {
        alertAlpha = 1.0f - (elapsed - 2.0f);
    }
    if (isHudEditing) alertAlpha = 1.0f;

    float w = 240.0f * scale;
    float h = 60.0f * scale;
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
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, textY), headerCol, "TEAMMATE ELIMINATED");
    if (g_FontHeader) ImGui::PopFont();

    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawPos.x + padding, nameY), textCol, deadName.c_str());
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

#endif
