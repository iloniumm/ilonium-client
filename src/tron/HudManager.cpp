// =====================================================================
// MODERN MODULAR HUD & EDIT MODE SYSTEM
// RetroCycles Client Mod | "ilonium"
// =====================================================================
#ifndef DEDICATED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "thirdparty/imgui/imgui_internal.h"
#include "HudManager.h"
#include "ePlayer.h"
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
#include "tools/tConfiguration.h"
#include "tron/DemoRecorder.h"
#include "tron/DemoPlayer.h"

// Expose standard client configuration variables
extern bool sg_modClientNameEnabled;
extern bool sg_modFpsEnabled;
extern bool sg_modPingEnabled;
extern bool sg_modTimeEnabled;
extern bool sg_modKeybindsEnabled;
extern bool isHudEditing;

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

#endif
