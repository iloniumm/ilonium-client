#include "ModMenu.h"
#include <iostream>
#include "rTexture.h"
#include "rUpscale.h"
#include "eFloodlights.h"
#include "gLogins.h"
#include "eVoter.h"
#include "ModTheme.h"
#include "ModPalette.h"
#include "ModTools.h"
#include "gStats.h"
#include "eWaypoints.h"
#include "cTelemetry.h"

// Kept where the game reads them, shown from here.
extern bool sg_hideOtherCycles;
extern bool sg_hideOtherTrails;

// The world's own colours, kept where the drawing reads them.
extern REAL sg_floorR, sg_floorG, sg_floorB;
extern REAL sg_gridR,  sg_gridG,  sg_gridB;
extern REAL sg_majorR, sg_majorG, sg_majorB;

extern bool sg_heartDeaths;
extern bool sg_heartSparks;
extern int  sg_heartDeathCount;
extern int  sg_heartSparkCount;
extern REAL sg_sparkRate;
extern bool sg_heartTrail;
extern int  sg_heartTrailCount;
extern bool sg_neonFloor;
extern bool sg_neonWalls;

extern bool sg_rimPainted;
extern int  sg_rimColours;
extern REAL sg_rim1R, sg_rim1G, sg_rim1B;
extern REAL sg_rim2R, sg_rim2G, sg_rim2B;
extern REAL sg_rim3R, sg_rim3G, sg_rim3B;

extern bool sg_trailPainted;
extern int  sg_trailColours;
extern REAL sg_trail1R, sg_trail1G, sg_trail1B;
extern REAL sg_trail2R, sg_trail2G, sg_trail2B;
extern REAL sg_trail3R, sg_trail3G, sg_trail3B;
#include "cNowPlaying.h"
#include "cBackend.h"

#ifndef DEDICATED
#include "HudManager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl2.h"
#include "rSDL.h"
#include "render/rConsole.h"
#include "tools/tSysTime.h"
#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#else
#include <sys/time.h>
#include <sys/statvfs.h>
#include <unistd.h>
#endif
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "tools/tRecorder.h"
#include "tron/MediaWidget.h"
#include "cPostProcessor.h"
#include "cDiscord.h"
#include "eWorldPaint.h"
#include "RCLAuth.h"
#include "gSocialMenu.h"
#include "gTicketMenu.h"
#include "cIRCChat.h"
#include "cIlonium.h"


#include "render/rSysdep.h"
#include "tron/gMenus.h"

static void RenderOriginalDashboard(ImDrawList* dl, ImGuiIO& io, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void RenderEsportsDashboard(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void RenderSupportTicketsTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void RenderIRCChatTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void RenderSocialsTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void RenderMiscTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH);
static void rc_MiscToolPick( int what );

extern REAL sg_modNameSizeScale;
extern bool sg_modPlainNames;

extern void (*st_PostLoadConfigCallback)();

// Auto-register the visual overlay renderer at static initialization time
struct ModMenuAutoRegister {
    ModMenuAutoRegister() {
        rSysDep::SetOverlayRenderer(&cVisualMenu::Render);
    }
};
static ModMenuAutoRegister g_ModMenuAutoRegister;

// Retrocycles / Armagetron Headers
#include "tDirectories.h"
#include "rTexture.h"
#include "ePlayer.h"
#include "eTimer.h"
#include "nNetwork.h"
#include "tools/tConfiguration.h"

// Custom main menu additions
#include "tron/gGame.h"
#include "tron/gServerBrowser.h"
#include "tron/gServerFavorites.h"
#include "tron/gFriends.h"
#include "tron/gLogo.h"
#include "tron/gStuff.h"
#include "engine/eSound.h"
#include "gCycle.h"
#include "gWinZone.h"
#include "ui/uInput.h"
#include "ui/uInputQueue.h"
#include "ui/uMenu.h"
#include "tools/tToDo.h"
#include "tools/tSysTime.h"
#include "tron/gLanguageMenu.h"
#include "network/nServerInfo.h"
#include "render/rScreen.h"
#include "render/rViewport.h"
#include "engine/eCamera.h"
#include "engine/eTeam.h"
#include "tron/gTeam.h"
#include "engine/eVoter.h"
static bool CaseInsensitiveSubstringSearch(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2));
        }
    );
    return it != haystack.end();
}

extern tString gl_vendor;
extern tString gl_renderer;
extern void sg_StartupPlayerMenu();
extern void GameSettingsSP();
extern void GameSettingsCurrent();
extern void sg_PlayerMenu();
extern void ret_to_MainMenu();
extern bool sg_RequestedDisconnection;
extern void sg_DisplayVersionInfo();
extern gGameSettings singlePlayer;

extern uMenu *sg_IngameMenu;
extern void gameloop_idle();
extern void sg_Receive();

#include "network/nNetObject.h"
#include "tOwnership.h"

RC_OWNERSHIP( menu )


// Real HUD / engine variables (in global namespace)
extern tString sg_lastServerName;
extern tString sg_lastServerIP;
extern unsigned int sg_lastServerPort;
extern bool subby_ShowHUD;
extern bool subby_ShowSpeedFastest;
extern bool subby_ShowScore;
extern bool subby_ShowAlivePeople;
extern bool subby_ShowPing;
extern bool subby_ShowSpeedMeter;
extern bool subby_ShowBrakeMeter;
extern bool subby_ShowRubberMeter;
extern bool showTime;
extern bool show24hour;

extern REAL CUSTOM_FOG_R;
extern REAL CUSTOM_FOG_G;
extern REAL CUSTOM_FOG_B;
extern REAL CUSTOM_FOG_DENSITY;

extern bool sg_noclipCinematic;
extern bool sg_noclipHideConsole;
extern bool sg_noclipHideNames;
extern bool sg_IsNoclipActive();
extern bool cfg_EnableInstancing;
extern bool cfg_MSAA;

// Client HUD Layout variables
extern REAL subby_SpeedGaugeSize;
extern REAL subby_SpeedGaugeLocX;
extern REAL subby_SpeedGaugeLocY;
extern REAL subby_BrakeGaugeSize;
extern REAL subby_BrakeGaugeLocX;
extern REAL subby_BrakeGaugeLocY;
extern REAL subby_RubberGaugeSize;
extern REAL subby_RubberGaugeLocX;
extern REAL subby_RubberGaugeLocY;
extern REAL subby_ScoreSize;
extern REAL subby_ScoreLocX;
extern REAL subby_ScoreLocY;
extern REAL subby_FastestSize;
extern REAL subby_FastestLocX;
extern REAL subby_FastestLocY;
extern REAL subby_AlivePeopleSize;
extern REAL subby_AlivePeopleLocX;
extern REAL subby_AlivePeopleLocY;
extern REAL subby_PingSize;
extern REAL subby_PingLocX;
extern REAL subby_PingLocY;

// Graphics & physics details
extern int sr_floorDetail;
extern bool sr_alphaBlend;
extern bool sr_smoothShading;
extern bool crash_sparks;
extern bool white_sparks;
extern bool sg_crashExplosion;
extern bool sg_explosionSingleLineUp;
extern bool sr_textOut;
extern bool sr_FPSOut;
extern bool sr_RecordingTimeOut;
extern int sr_floorMirror;
extern bool sr_highRim;
extern bool sr_dither;
extern bool sr_upperSky;
extern bool sr_lowerSky;
extern bool sr_skyWobble;
extern bool sr_infinityPlane;
extern bool sr_laggometer;
extern bool sg_axesIndicator;
extern bool sr_predictObjects;
extern bool sr_texturesTruecolor;
extern bool sr_keepWindowActive;
extern int sound_quality;
extern int buffer_shift;
extern int sound_sources;


// Globals
bool g_NoclipMode = false;
bool g_CleanScreen = false;
bool g_SmartGlance = false;
bool g_CustomHitbox = false;
bool g_CustomFog = true;
float g_FogDensity = 0.0f;
float g_FogR = 1.0f, g_FogG = 1.0f, g_FogB = 1.0f;
bool g_ShowHUD = false;
bool g_RubberGauge = false;
bool g_SpeedMeter = false;
bool g_BrakeMeter = false;
bool g_ShowScores = false;
bool g_ShowPing = false;
bool g_AliveCounter = false;
bool g_ShowFastest = false;
bool g_ShowTime = false;
bool g_24hFormat = false;

// Client variables
float g_FOV = 90.0f;
float g_SpeedGaugeSize = 1.0f;
float g_SpeedGaugeX = 0.0f;
float g_SpeedGaugeY = 0.0f;
float g_BrakeGaugeSize = 1.0f;
float g_BrakeGaugeX = 0.0f;
float g_BrakeGaugeY = 0.0f;
float g_RubberGaugeSize = 1.0f;
float g_RubberGaugeX = 0.0f;
float g_RubberGaugeY = 0.0f;

// Graphics & Util variables
bool g_Sparks = true;
bool g_WhiteSparks = false;
bool g_Explosions = true;
bool g_AlphaBlend = true;
bool g_SmoothShading = true;
float g_FloorDetail = 3.0f;

// Theme custom globals
bool g_RGBTopBar = true;
bool g_RGBAccent = false;
float g_RGBSpeed = 0.15f;
bool g_ShowParticles = true;
float g_ParticleType = 0.0f; // 0 = Dust, 1 = Rain, 2 = Stars
bool g_GradientAccent = true;
ImVec4 g_AccentColor = ImVec4(0.478f, 0.345f, 1.0f, 1.0f);
// Taken from the theme rather than written out again: everything that asks
// GetThemeColor for a colour follows these two, so this is where the whole
// interface changes hue at once.
ImVec4 g_AccentColor1 = ImGui::ColorConvertU32ToFloat4( rc_Theme().primary.solid );
ImVec4 g_AccentColor2 = ImGui::ColorConvertU32ToFloat4( rc_Theme().second.solid );
ImVec4 g_MenuBgColor = ImVec4(0.07f, 0.07f, 0.08f, 1.0f);
float g_MenuBgAlpha = 0.85f;
bool g_InteractiveParticles = true;
bool g_ConstellationWeb = true;
bool g_ParallaxEffect = true;

static std::vector<std::string> s_DashboardConfigs;
static bool s_DashboardConfigsLoaded = false;
static void LoadDashboardConfigs();


// Global Keybind Variables
bool g_ModMenuKeybindEnabled = true;
int g_ModMenuKeybind = 277;    // 277 is the legacy keycode for SDLK_INSERT
int g_NoclipKeybind = 110;       // SDLK_N ('n')
int g_CleanScreenKeybind = 104;   // SDLK_H ('h')
int g_SmartGlanceKeybind = 0;
int g_CustomHitboxKeybind = 0;
int g_DashboardActiveCol = 0;
int g_DashboardLeftSelected = 0;
int g_DashboardMiddleSelected = 0;
int g_DashboardRightSelected = 0;
bool g_DashboardActionTriggered = false;
int g_CustomFogKeybind = 0;
int g_ShowHUDKeybind = 0;
int g_RubberGaugeKeybind = 0;
int g_SpeedMeterKeybind = 0;
int g_BrakeMeterKeybind = 0;
int g_ShowScoresKeybind = 0;
int g_ShowPingKeybind = 0;
int g_AliveCounterKeybind = 0;
int g_ShowFastestKeybind = 0;
int g_ShowTimeKeybind = 0;
int g_24hFormatKeybind = 0;
int g_SparksKeybind = 0;
int g_WhiteSparksKeybind = 0;
int g_ExplosionsKeybind = 0;
int g_AlphaBlendKeybind = 0;
int g_SmoothShadingKeybind = 0;
int g_RGBTopBarKeybind = 0;
int g_RGBAccentKeybind = 0;
int g_ShowParticlesKeybind = 0;
int g_InteractiveParticlesKeybind = 0;
int g_ConstellationWebKeybind = 0;
int g_ParallaxEffectKeybind = 0;
int g_GradientAccentKeybind = 0;
extern bool g_EnhancedGraphicsMode;
int g_EnhancedGraphicsKeybind = 288; // F9

// Auto Packet Refresh configuration
bool g_AutoPacketRefresh = true;
int g_PacketRefreshKeybind = 0;

// K/D Reset keybind
int g_ResetKDKeybind = 0;
extern bool sg_modKDResetFlag;
extern bool sg_modCenterMessageEnabled;
extern bool g_modChatManualScroll;
extern float g_modChatPendingScrollDelta;
extern float g_modChatLastScrollTime;

// Media Player Keybinds
int g_MediaPlayPauseKeybind = 0;
int g_MediaNextKeybind = 0;
int g_MediaPrevKeybind = 0;

// Player Color Overrides
extern bool sg_overrideLocalColor;
extern REAL sg_localColorR;
extern REAL sg_localColorG;
extern REAL sg_localColorB;
extern bool sg_distributeEnemyColors;
extern bool sg_overrideEnemyUnifiedColor;
extern REAL sg_enemyUnifiedColorR;
extern REAL sg_enemyUnifiedColorG;
extern REAL sg_enemyUnifiedColorB;

extern bool sg_drawZoneCenter;
extern bool sg_corpseTimerEnabled;
extern bool sg_corpseTimerOverride;
extern float sg_corpseTimerDuration;
extern int sg_corpseTrailStyle;
extern bool sg_hasLastServer;
extern tString sg_lastServerIP;
extern unsigned int sg_lastServerPort;
extern void ReconnectToServer();

// Network Anti-Lag packet arrival tracking
extern double sn_LastPacketTime;



// Spectator / Noclip configuration variables
extern REAL sg_noclipSpeed;
extern REAL sg_noclipSlowFactor;
extern REAL sg_noclipMouseSens;
extern int sg_noclipKeyForward;
extern int sg_noclipKeyBack;
extern int sg_noclipKeyLeft;
extern int sg_noclipKeyRight;
extern int sg_noclipKeyUp;
extern int sg_noclipKeyDown;
extern int sg_noclipKeyZoomIn;
extern int sg_noclipKeyZoomOut;
extern int sg_noclipKeyCenter;
extern int sg_noclipKeyLookAt;
extern int sg_noclipKeyLock;
extern int sg_noclipKeyFollow;
extern int sg_noclipKeyNextPl;
extern int sg_noclipKeyPrevPl;
extern int sg_noclipKeySlow;
extern int sg_noclipKeyFast;
extern int sg_noclipKeyLevel;
extern int sg_noclipKeySphere;
extern REAL sg_noclipFastFactor;
extern REAL sg_noclipSphereRadius;
extern bool sg_noclipLevel;
extern bool sg_noclipSphere;
extern bool sg_noclipCinematic;
extern int sg_noclipKeyToggle;
extern int sg_noclipKeyCinematic;
extern int sg_noclipKeyOrbit;
extern int sg_noclipKeySmooth;
extern int sg_noclipKeyZoneFocus;
extern int sg_noclipKeyMapCenter;
extern REAL sg_noclipOrbitSpeed;
extern REAL sg_noclipOrbitRadius;
extern REAL sg_noclipOrbitHeight;
extern REAL sg_noclipFollowDist;
extern REAL sg_noclipFollowHeight;
extern REAL sg_noclipSmoothFactor;
extern bool sg_noclipHideConsole;
extern bool sg_noclipHideNames;

// Configurations for our custom variables
static tConfItem<int> conf_noclipKeybind("MOD_NOCLIP_KEYBIND", g_NoclipKeybind);
static tConfItem<int> conf_cleanScreenKeybind("MOD_CLEAN_SCREEN_KEYBIND", g_CleanScreenKeybind);

// These switches offered a key and then did nothing with it: the slot was a
// local that no key check ever read and no config file ever saw. Same shape as
// every other bind now - one place, saved, and answered below.
int g_NeonFloorKeybind = 0;
int g_NeonWallsKeybind = 0;
int g_ZoneGlowKeybind = 0;
int g_ZoneGlowOwnKeybind = 0;
int g_FloodlightsKeybind = 0;
int g_FloodlightBeamsKeybind = 0;
int g_HideCyclesKeybind = 0;
int g_HideTrailsKeybind = 0;
int g_HeartSparksKeybind = 0;
int g_HeartTrailKeybind = 0;
int g_HeartDeathsKeybind = 0;

static tConfItem<int> conf_neonFloorKeybind("MOD_NEON_FLOOR_KEYBIND", g_NeonFloorKeybind);
static tConfItem<int> conf_neonWallsKeybind("MOD_NEON_WALLS_KEYBIND", g_NeonWallsKeybind);
static tConfItem<int> conf_zoneGlowKeybind("MOD_ZONE_GLOW_KEYBIND", g_ZoneGlowKeybind);
static tConfItem<int> conf_zoneGlowOwnKeybind("MOD_ZONE_GLOW_OWN_KEYBIND", g_ZoneGlowOwnKeybind);
static tConfItem<int> conf_floodlightsKeybind("MOD_FLOODLIGHTS_KEYBIND", g_FloodlightsKeybind);
static tConfItem<int> conf_floodlightBeamsKeybind("MOD_FLOODLIGHT_BEAMS_KEYBIND", g_FloodlightBeamsKeybind);
static tConfItem<int> conf_hideCyclesKeybind("MOD_HIDE_CYCLES_KEYBIND", g_HideCyclesKeybind);
static tConfItem<int> conf_hideTrailsKeybind("MOD_HIDE_TRAILS_KEYBIND", g_HideTrailsKeybind);
static tConfItem<int> conf_heartSparksKeybind("MOD_HEART_SPARKS_KEYBIND", g_HeartSparksKeybind);
static tConfItem<int> conf_heartTrailKeybind("MOD_HEART_TRAIL_KEYBIND", g_HeartTrailKeybind);
static tConfItem<int> conf_heartDeathsKeybind("MOD_HEART_DEATHS_KEYBIND", g_HeartDeathsKeybind);

static tConfItem<int> conf_customHitboxKeybind("MOD_CUSTOM_HITBOX_KEYBIND", g_CustomHitboxKeybind);
static tConfItem<int> conf_customFogKeybind("MOD_CUSTOM_FOG_KEYBIND", g_CustomFogKeybind);
static tConfItem<bool> conf_autoPacketRefresh("MOD_AUTO_PACKET_REFRESH", g_AutoPacketRefresh);
static tConfItem<int> conf_packetRefreshKeybind("MOD_PACKET_REFRESH_KEYBIND", g_PacketRefreshKeybind);
static tConfItem<int> conf_resetKDKeybind("MOD_RESET_KD_KEYBIND", g_ResetKDKeybind);
static tConfItem<int> conf_mediaPlayPauseKeybind("MOD_MEDIA_PLAY_PAUSE_KEYBIND", g_MediaPlayPauseKeybind);
static tConfItem<int> conf_mediaNextKeybind("MOD_MEDIA_NEXT_KEYBIND", g_MediaNextKeybind);
static tConfItem<int> conf_mediaPrevKeybind("MOD_MEDIA_PREV_KEYBIND", g_MediaPrevKeybind);
static tConfItem<int> conf_showHudKeybind("MOD_SHOW_HUD_KEYBIND", g_ShowHUDKeybind);
static tConfItem<int> conf_rubberGaugeKeybind("MOD_RUBBER_GAUGE_KEYBIND", g_RubberGaugeKeybind);
static tConfItem<int> conf_speedMeterKeybind("MOD_SPEED_METER_KEYBIND", g_SpeedMeterKeybind);
static tConfItem<int> conf_brakeMeterKeybind("MOD_BRAKE_METER_KEYBIND", g_BrakeMeterKeybind);
static tConfItem<int> conf_scoreboardKeybind("MOD_SCOREBOARD_KEYBIND", g_ShowScoresKeybind);
static tConfItem<int> conf_showPingKeybind("MOD_SHOW_PING_KEYBIND", g_ShowPingKeybind);
static tConfItem<int> conf_aliveCounterKeybind("MOD_ALIVE_COUNTER_KEYBIND", g_AliveCounterKeybind);
static tConfItem<int> conf_showFastestKeybind("MOD_SHOW_FASTEST_KEYBIND", g_ShowFastestKeybind);
static tConfItem<int> conf_showTimeKeybind("MOD_SHOW_TIME_KEYBIND", g_ShowTimeKeybind);
static tConfItem<int> conf_24hFormatKeybind("MOD_24H_FORMAT_KEYBIND", g_24hFormatKeybind);
static tConfItem<int> conf_sparksKeybind("MOD_SPARKS_KEYBIND", g_SparksKeybind);
static tConfItem<int> conf_whiteSparksKeybind("MOD_WHITE_SPARKS_KEYBIND", g_WhiteSparksKeybind);
static tConfItem<int> conf_explosionsKeybind("MOD_EXPLOSIONS_KEYBIND", g_ExplosionsKeybind);
static tConfItem<int> conf_alphaBlendKeybind("MOD_ALPHA_BLEND_KEYBIND", g_AlphaBlendKeybind);
static tConfItem<int> conf_smoothShadingKeybind("MOD_SMOOTH_SHADING_KEYBIND", g_SmoothShadingKeybind);
static tConfItem<int> conf_rgbTopBarKeybind("MOD_RGB_TOP_BAR_KEYBIND", g_RGBTopBarKeybind);
static tConfItem<int> conf_rgbAccentKeybind("MOD_RGB_ACCENT_KEYBIND", g_RGBAccentKeybind);
static tConfItem<int> conf_showParticlesKeybind("MOD_SHOW_PARTICLES_KEYBIND", g_ShowParticlesKeybind);
static tConfItem<int> conf_interactiveParticlesKeybind("MOD_INTERACTIVE_PARTICLES_KEYBIND", g_InteractiveParticlesKeybind);
static tConfItem<int> conf_constellationWebKeybind("MOD_CONSTELLATION_WEB_KEYBIND", g_ConstellationWebKeybind);
static tConfItem<int> conf_parallaxEffectKeybind("MOD_PARALLAX_EFFECT_KEYBIND", g_ParallaxEffectKeybind);
static tConfItem<int> conf_gradientAccentKeybind("MOD_GRADIENT_ACCENT_KEYBIND", g_GradientAccentKeybind);
static tConfItem<int> conf_enhancedGraphicsKeybind("MOD_ENHANCED_GRAPHICS_KEYBIND", g_EnhancedGraphicsKeybind);
static tConfItem<bool> conf_modMenuKeybindEnabled("MOD_MENU_KEYBIND_ENABLED", g_ModMenuKeybindEnabled);
static tConfItem<int> conf_modMenuKeybind("MOD_MENU_KEYBIND", g_ModMenuKeybind);


// Mod modes & variables
static tConfItem<bool> conf_noclipMode("MOD_NOCLIP_MODE", g_NoclipMode);
static tConfItem<bool> conf_cleanScreen("MOD_CLEAN_SCREEN", g_CleanScreen);
static tConfItem<bool> conf_customHitbox("MOD_CUSTOM_HITBOX", g_CustomHitbox);
static tConfItem<bool> conf_customFog("MOD_CUSTOM_FOG", g_CustomFog);
static tConfItem<float> conf_fogDensity("MOD_FOG_DENSITY", g_FogDensity);
static tConfItem<float> conf_fogR("MOD_FOG_R", g_FogR);
static tConfItem<float> conf_fogG("MOD_FOG_G", g_FogG);
static tConfItem<float> conf_fogB("MOD_FOG_B", g_FogB);

static tConfItem<bool> conf_showHUD("MOD_SHOW_HUD", g_ShowHUD);
static tConfItem<bool> conf_rubberGauge("MOD_HUD_RUBBER_GAUGE", g_RubberGauge);
static tConfItem<bool> conf_speedMeter("MOD_SPEED_METER", g_SpeedMeter);
static tConfItem<bool> conf_brakeMeter("MOD_BRAKE_METER", g_BrakeMeter);
static tConfItem<bool> conf_showScores("MOD_SHOW_SCORES", g_ShowScores);
static tConfItem<bool> conf_showPing("MOD_SHOW_PING", g_ShowPing);
static tConfItem<bool> conf_aliveCounter("MOD_ALIVE_COUNTER", g_AliveCounter);
static tConfItem<bool> conf_showFastest("MOD_SHOW_FASTEST", g_ShowFastest);
static tConfItem<bool> conf_showTime("MOD_SHOW_TIME", g_ShowTime);
static tConfItem<bool> conf_24hFormat("MOD_24H_FORMAT", g_24hFormat);

static tConfItem<float> conf_fov("MOD_FOV", g_FOV);
static tConfItem<float> conf_speedGaugeSize("MOD_SPEED_GAUGE_SIZE", g_SpeedGaugeSize);
static tConfItem<float> conf_speedGaugeX("MOD_SPEED_GAUGE_X", g_SpeedGaugeX);
static tConfItem<float> conf_speedGaugeY("MOD_SPEED_GAUGE_Y", g_SpeedGaugeY);
static tConfItem<float> conf_brakeGaugeSize("MOD_BRAKE_GAUGE_SIZE", g_BrakeGaugeSize);
static tConfItem<float> conf_brakeGaugeX("MOD_BRAKE_GAUGE_X", g_BrakeGaugeX);
static tConfItem<float> conf_brakeGaugeY("MOD_BRAKE_GAUGE_Y", g_BrakeGaugeY);
static tConfItem<float> conf_rubberGaugeSize("MOD_RUBBER_GAUGE_SIZE", g_RubberGaugeSize);
static tConfItem<float> conf_rubberGaugeX("MOD_RUBBER_GAUGE_X", g_RubberGaugeX);
static tConfItem<float> conf_rubberGaugeY("MOD_RUBBER_GAUGE_Y", g_RubberGaugeY);

static tConfItem<bool> conf_sparks("MOD_SPARKS", g_Sparks);
static tConfItem<bool> conf_whiteSparks("MOD_WHITE_SPARKS", g_WhiteSparks);
static tConfItem<bool> conf_explosions("MOD_EXPLOSIONS", g_Explosions);
static tConfItem<bool> conf_alphaBlend("MOD_ALPHA_BLEND", g_AlphaBlend);
static tConfItem<bool> conf_smoothShading("MOD_SMOOTH_SHADING", g_SmoothShading);
static tConfItem<float> conf_floorDetail("MOD_FLOOR_DETAIL", g_FloorDetail);

static tConfItem<bool> conf_rgbTopBar("MOD_RGB_TOP_BAR", g_RGBTopBar);
static tConfItem<bool> conf_rgbAccent("MOD_RGB_ACCENT", g_RGBAccent);
static tConfItem<float> conf_rgbSpeed("MOD_RGB_SPEED", g_RGBSpeed);
static tConfItem<bool> conf_showParticles("MOD_SHOW_PARTICLES", g_ShowParticles);
static tConfItem<float> conf_particleType("MOD_PARTICLE_TYPE", g_ParticleType);
static tConfItem<bool> conf_gradientAccent("MOD_GRADIENT_ACCENT", g_GradientAccent);
static tConfItem<bool> conf_interactiveParticles("MOD_INTERACTIVE_PARTICLES", g_InteractiveParticles);
static tConfItem<bool> conf_constellationWeb("MOD_CONSTELLATION_WEB", g_ConstellationWeb);
static tConfItem<bool> conf_parallaxEffect("MOD_PARALLAX_EFFECT", g_ParallaxEffect);


static tConfItem<float> conf_accentR1("MOD_ACCENT_R1", g_AccentColor1.x);
static tConfItem<float> conf_accentG1("MOD_ACCENT_G1", g_AccentColor1.y);
static tConfItem<float> conf_accentB1("MOD_ACCENT_B1", g_AccentColor1.z);
static tConfItem<float> conf_accentR2("MOD_ACCENT_R2", g_AccentColor2.x);
static tConfItem<float> conf_accentG2("MOD_ACCENT_G2", g_AccentColor2.y);
static tConfItem<float> conf_accentB2("MOD_ACCENT_B2", g_AccentColor2.z);
static tConfItem<float> conf_menuBgR("MOD_MENU_BG_R", g_MenuBgColor.x);
static tConfItem<float> conf_menuBgG("MOD_MENU_BG_G", g_MenuBgColor.y);
static tConfItem<float> conf_menuBgB("MOD_MENU_BG_B", g_MenuBgColor.z);
static tConfItem<float> conf_menuBgAlpha("MOD_MENU_BG_ALPHA", g_MenuBgAlpha);

static int* g_BindingKeybindPtr = nullptr;

// Static tracker variables for bidirectional sync
static bool prev_ShowHUD = false;
static bool prev_ShowFastest = false;
static bool prev_ShowScores = false;
static bool prev_AliveCounter = false;
static bool prev_ShowPing = false;
static bool prev_SpeedMeter = false;
static bool prev_BrakeMeter = false;
static bool prev_RubberGauge = false;
static bool prev_ShowTime = false;
static bool prev_24hFormat = false;
static float prev_FogR = 1.0f;
static float prev_FogG = 1.0f;
static float prev_FogB = 1.0f;
static float prev_FogDensity = 0.0f;
static bool prev_NoclipMode = false;
static bool prev_CleanScreen = false;
static bool prev_SmartGlance = false;
static float prev_FOV = 70.0f;
static float prev_SpeedGaugeSize = 1.0f;
static float prev_SpeedGaugeX = 0.0f;
static float prev_SpeedGaugeY = 0.0f;
static float prev_BrakeGaugeSize = 1.0f;
static float prev_BrakeGaugeX = 0.0f;
static float prev_BrakeGaugeY = 0.0f;
static float prev_RubberGaugeSize = 1.0f;
static float prev_RubberGaugeX = 0.0f;
static float prev_RubberGaugeY = 0.0f;
static bool prev_Sparks = true;
static bool prev_WhiteSparks = false;
static bool prev_Explosions = true;
static bool prev_AlphaBlend = true;
static bool prev_SmoothShading = true;
static float prev_FloorDetail = 3.0f;


static int TranslateLegacyKeycodeToSDL3Keycode(int keysym) {
    if (keysym < 0) return keysym;
    if (keysym < 128) {
        return keysym;
    }
    switch (keysym) {
        case 127: return SDLK_DELETE;
        case 273: return SDLK_UP;
        case 274: return SDLK_DOWN;
        case 275: return SDLK_RIGHT;
        case 276: return SDLK_LEFT;
        case 277: return SDLK_INSERT;
        case 278: return SDLK_HOME;
        case 279: return SDLK_END;
        case 280: return SDLK_PAGEUP;
        case 281: return SDLK_PAGEDOWN;
        case 282: return SDLK_F1;
        case 283: return SDLK_F2;
        case 284: return SDLK_F3;
        case 285: return SDLK_F4;
        case 286: return SDLK_F5;
        case 287: return SDLK_F6;
        case 288: return SDLK_F7;
        case 289: return SDLK_F8;
        case 290: return SDLK_F9;
        case 291: return SDLK_F10;
        case 292: return SDLK_F11;
        case 293: return SDLK_F12;
        case 256: return SDLK_KP_0;
        case 257: return SDLK_KP_1;
        case 258: return SDLK_KP_2;
        case 259: return SDLK_KP_3;
        case 260: return SDLK_KP_4;
        case 261: return SDLK_KP_5;
        case 262: return SDLK_KP_6;
        case 263: return SDLK_KP_7;
        case 264: return SDLK_KP_8;
        case 265: return SDLK_KP_9;
        case 266: return SDLK_KP_PERIOD;
        case 267: return SDLK_KP_DIVIDE;
        case 268: return SDLK_KP_MULTIPLY;
        case 269: return SDLK_KP_MINUS;
        case 270: return SDLK_KP_PLUS;
        case 271: return SDLK_KP_ENTER;
        case 272: return SDLK_KP_EQUALS;
        case 300: return SDLK_NUMLOCKCLEAR;
        case 301: return SDLK_CAPSLOCK;
        case 302: return SDLK_SCROLLLOCK;
        case 303: return SDLK_RSHIFT;
        case 304: return SDLK_LSHIFT;
        case 305: return SDLK_RCTRL;
        case 306: return SDLK_LCTRL;
        case 307: return SDLK_RALT;
        case 308: return SDLK_LALT;
        case 309: return SDLK_RGUI;
        case 310: return SDLK_LGUI;
        case 311: return SDLK_LGUI;
        case 312: return SDLK_RGUI;
        case 316: return SDLK_PRINTSCREEN;
        case 19:  return SDLK_PAUSE;
    }
    return keysym;
}

static void TranslateAllModKeybinds() {
    g_ModMenuKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ModMenuKeybind);
    g_NoclipKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_NoclipKeybind);
    g_CleanScreenKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_CleanScreenKeybind);
    g_CustomHitboxKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_CustomHitboxKeybind);
    g_CustomFogKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_CustomFogKeybind);
    g_ShowHUDKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowHUDKeybind);
    g_RubberGaugeKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_RubberGaugeKeybind);
    g_SpeedMeterKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_SpeedMeterKeybind);
    g_BrakeMeterKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_BrakeMeterKeybind);
    g_ShowScoresKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowScoresKeybind);
    g_ShowPingKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowPingKeybind);
    g_AliveCounterKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_AliveCounterKeybind);
    g_ShowFastestKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowFastestKeybind);
    g_ShowTimeKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowTimeKeybind);
    g_24hFormatKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_24hFormatKeybind);
    g_SparksKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_SparksKeybind);
    g_WhiteSparksKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_WhiteSparksKeybind);
    g_ExplosionsKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ExplosionsKeybind);
    g_AlphaBlendKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_AlphaBlendKeybind);
    g_SmoothShadingKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_SmoothShadingKeybind);
    g_RGBTopBarKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_RGBTopBarKeybind);
    g_RGBAccentKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_RGBAccentKeybind);
    g_ShowParticlesKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ShowParticlesKeybind);
    g_InteractiveParticlesKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_InteractiveParticlesKeybind);
    g_ConstellationWebKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ConstellationWebKeybind);
    g_ParallaxEffectKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ParallaxEffectKeybind);
    g_GradientAccentKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_GradientAccentKeybind);
    g_EnhancedGraphicsKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_EnhancedGraphicsKeybind);
    g_PacketRefreshKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_PacketRefreshKeybind);
    g_ResetKDKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_ResetKDKeybind);
    g_MediaPlayPauseKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_MediaPlayPauseKeybind);
    g_MediaNextKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_MediaNextKeybind);
    g_MediaPrevKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_MediaPrevKeybind);

    sg_noclipKeyForward = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyForward);
    sg_noclipKeyBack = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyBack);
    sg_noclipKeyLeft = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyLeft);
    sg_noclipKeyRight = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyRight);
    sg_noclipKeyUp = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyUp);
    sg_noclipKeyDown = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyDown);
    sg_noclipKeyZoomIn = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyZoomIn);
    sg_noclipKeyZoomOut = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyZoomOut);
    sg_noclipKeyCenter = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyCenter);
    sg_noclipKeyLookAt = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyLookAt);
    sg_noclipKeyLock = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyLock);
    sg_noclipKeyFollow = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyFollow);
    sg_noclipKeyNextPl = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyNextPl);
    sg_noclipKeyPrevPl = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyPrevPl);
    sg_noclipKeySlow = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeySlow);
    sg_noclipKeyToggle = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyToggle);
    sg_noclipKeyCinematic = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyCinematic);
    sg_noclipKeyOrbit = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyOrbit);
    sg_noclipKeySmooth = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeySmooth);
    sg_noclipKeyZoneFocus = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyZoneFocus);
    sg_noclipKeyMapCenter = TranslateLegacyKeycodeToSDL3Keycode(sg_noclipKeyMapCenter);
}

static std::string GetKeyName(int key) {
    if (key == 0) return "NONE";
    const char* name = SDL_GetKeyName((SDL_Keycode)key);
    if (name && name[0] != '\0') return std::string(name);
    return "UNKNOWN";
}

static std::string GetScancodeName(int scancode) {
    if (scancode == 0) return "NONE";
    const char* name = SDL_GetScancodeName((SDL_Scancode)scancode);
    if (name && name[0] != '\0') return std::string(name);
    return "UNKNOWN";
}

static void DrawCycleColorPreview(ePlayer* lp) {
    if (!lp) return;
    
    // 1. Calculate color overflow values for cycle body:
    int rc = lp->rgb[0] & 15;
    int gc = lp->rgb[1] & 15;
    int bc = lp->rgb[2] & 15;

    float rc_f = (float)rc;
    float gc_f = (float)gc;
    float bc_f = (float)bc;

    // Brightness boosting for cycle
    while (rc_f + gc_f + bc_f < 3.0f) {
        rc_f += 0.5f;
        gc_f += 0.5f;
        bc_f += 0.5f;
    }

    ImVec4 cycleColor(rc_f * 17.0f / 255.0f, gc_f * 17.0f / 255.0f, bc_f * 17.0f / 255.0f, 1.0f);

    // 2. Calculate clamped color values for the trail:
    float rt_f = (float)lp->rgb[0];
    float gt_f = (float)lp->rgb[1];
    float bt_f = (float)lp->rgb[2];

    if (rt_f > 15.0f) rt_f = 15.0f;
    if (gt_f > 15.0f) gt_f = 15.0f;
    if (bt_f > 15.0f) bt_f = 15.0f;
    if (rt_f < 0.0f) rt_f = 0.0f;
    if (gt_f < 0.0f) gt_f = 0.0f;
    if (bt_f < 0.0f) bt_f = 0.0f;

    // Brightness boosting for trail
    while (rt_f + gt_f + bt_f < 6.0f) {
        rt_f += 0.5f;
        gt_f += 0.5f;
        bt_f += 0.5f;
    }

    ImVec4 trailColor(rt_f / 15.0f, gt_f / 15.0f, bt_f / 15.0f, 1.0f);

    ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "LIVE CYCLE & TRAIL PREVIEW");
    ImGui::Spacing();
    
    // Begin a child or canvas area
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 110.0f;
    ImVec2 canvas_size(width, height);
    
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    // Draw background box with rich aesthetics
    ImU32 bgCol = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
    dl->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + width, canvas_pos.y + height), bgCol, 8.0f);
    dl->AddRect(canvas_pos, ImVec2(canvas_pos.x + width, canvas_pos.y + height), borderCol, 8.0f);
    
    // Draw grid floor lines to make it feel like the Armagetron grid
    ImU32 gridCol = ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.25f, 0.25f));
    float gridSpacing = 20.0f;
    for (float x = canvas_pos.x + gridSpacing; x < canvas_pos.x + width; x += gridSpacing) {
        dl->AddLine(ImVec2(x, canvas_pos.y), ImVec2(x, canvas_pos.y + height), gridCol);
    }
    for (float y = canvas_pos.y + gridSpacing; y < canvas_pos.y + height; y += gridSpacing) {
        dl->AddLine(ImVec2(canvas_pos.x, y), ImVec2(canvas_pos.x + width, y), gridCol);
    }
    
    // Define positions
    ImVec2 cyclePos(canvas_pos.x + width - 65.0f, canvas_pos.y + height * 0.55f);
    ImVec2 trailLeftTop(canvas_pos.x + 15.0f, canvas_pos.y + height * 0.15f);
    ImVec2 trailLeftBottom(canvas_pos.x + 15.0f, canvas_pos.y + height * 0.85f);
    ImVec2 trailRightTop(cyclePos.x, cyclePos.y - 12.0f);
    ImVec2 trailRightBottom(cyclePos.x, cyclePos.y + 12.0f);
    
    // Draw trail wall polygon with gradient-like transparency
    ImVec2 trailPts[4] = {
        trailLeftTop,
        trailRightTop,
        trailRightBottom,
        trailLeftBottom
    };
    dl->AddConvexPolyFilled(trailPts, 4, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.40f)));
    
    // Draw trail glowing core line (top of the wall)
    dl->AddLine(trailLeftTop, trailRightTop, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.95f)), 2.5f);
    // Bottom border of the wall
    dl->AddLine(trailLeftBottom, trailRightBottom, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.45f)), 1.5f);
    
    // Draw futuristic zig-zag grid patterns on the trail
    ImU32 zigCol = ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.65f));
    int numZigs = 7;
    for (int i = 0; i < numZigs; ++i) {
        float t0 = (float)i / numZigs;
        float t1 = ((float)i + 0.5f) / numZigs;
        float t2 = ((float)i + 1.0f) / numZigs;
        
        ImVec2 p0(trailLeftTop.x + (trailRightTop.x - trailLeftTop.x) * t0, trailLeftTop.y + (trailRightTop.y - trailLeftTop.y) * t0);
        ImVec2 p1(trailLeftBottom.x + (trailRightBottom.x - trailLeftBottom.x) * t1, trailLeftBottom.y + (trailRightBottom.y - trailLeftBottom.y) * t1);
        ImVec2 p2(trailLeftTop.x + (trailRightTop.x - trailLeftTop.x) * t2, trailLeftTop.y + (trailRightTop.y - trailLeftTop.y) * t2);
        
        dl->AddLine(p0, p1, zigCol, 1.5f);
        dl->AddLine(p1, p2, zigCol, 1.5f);
    }
    
    // Draw Lightcycle:
    ImVec2 rearWheel = cyclePos;
    ImVec2 frontWheel(cyclePos.x + 38.0f, cyclePos.y + 2.0f);
    
    // Rear wheel
    dl->AddCircleFilled(rearWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.15f, 1.0f)));
    dl->AddCircleFilled(rearWheel, 5.0f, ImGui::GetColorU32(cycleColor));
    dl->AddCircle(rearWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.75f, 1.0f)), 16, 2.0f);
    
    // Front wheel
    dl->AddCircleFilled(frontWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.15f, 1.0f)));
    dl->AddCircleFilled(frontWheel, 5.0f, ImGui::GetColorU32(cycleColor));
    dl->AddCircle(frontWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.75f, 1.0f)), 16, 2.0f);
    
    // Cycle body chassis polygon
    ImVec2 bodyPts[6] = {
        ImVec2(cyclePos.x - 12.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 5.0f, cyclePos.y - 12.0f),
        ImVec2(cyclePos.x + 20.0f, cyclePos.y - 12.0f),
        ImVec2(cyclePos.x + 40.0f, cyclePos.y - 2.0f),
        ImVec2(cyclePos.x + 36.0f, cyclePos.y + 6.0f),
        ImVec2(cyclePos.x - 2.0f, cyclePos.y + 6.0f)
    };
    dl->AddConvexPolyFilled(bodyPts, 6, ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.24f, 1.0f)));
    dl->AddPolyline(bodyPts, 6, ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.50f, 1.0f)), true, 1.0f);
    
    // Colored main accent body panel
    ImVec2 accentPts[4] = {
        ImVec2(cyclePos.x + 3.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 16.0f, cyclePos.y - 8.0f),
        ImVec2(cyclePos.x + 28.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 22.0f, cyclePos.y + 2.0f)
    };
    dl->AddConvexPolyFilled(accentPts, 4, ImGui::GetColorU32(cycleColor));
    dl->AddPolyline(accentPts, 4, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.35f)), true, 1.0f);
    
    // Glass canopy
    ImVec2 canopyPts[4] = {
        ImVec2(cyclePos.x + 8.0f, cyclePos.y - 11.0f),
        ImVec2(cyclePos.x + 18.0f, cyclePos.y - 11.0f),
        ImVec2(cyclePos.x + 23.0f, cyclePos.y - 6.0f),
        ImVec2(cyclePos.x + 13.0f, cyclePos.y - 6.0f)
    };
    dl->AddConvexPolyFilled(canopyPts, 4, ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.12f, 0.85f)));
    dl->AddPolyline(canopyPts, 4, ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.40f, 1.0f)), true, 1.0f);

    // Text labels for colors
    char colorLabel[128];
    snprintf(colorLabel, sizeof(colorLabel), "Cycle: R:%d G:%d B:%d | Trail: R:%.2f G:%.2f B:%.2f",
             (int)(cycleColor.x * 255.0f), (int)(cycleColor.y * 255.0f), (int)(cycleColor.z * 255.0f), trailColor.x, trailColor.y, trailColor.z);
    
    ImGui::Dummy(canvas_size);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "%s", colorLabel);
}

#include <dirent.h>
#include <sys/stat.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <map>

// Whether the overlay is expecting characters. Anything that switches text
// input off has to ask first, or it silences a field the player is typing in.

bool ModMenu_WantsTextInput()
{
    return ImGui::GetCurrentContext() != NULL && ImGui::GetIO().WantTextInput;
}

struct SavedProfile {
    std::string name;
    std::string globalID;
    bool autoLogin;
};

struct SavedColor {
    std::string name;
    int r, g, b;
};

static std::vector<SavedProfile> g_SavedProfiles;
static std::vector<SavedColor> g_SavedColors;
static std::map<tString, std::string> g_DefaultCameraSettings;
static bool g_ProfilesLoaded = false;
static bool g_ColorsLoaded = false;

static void LoadProfiles() {
    g_SavedProfiles.clear();
    std::string path = (const char*)tDirectories::Var().GetWritePath("saved_profiles.txt");
    std::ifstream f(path);
    if (!f.is_open()) {
        ePlayer* lp = ePlayer::PlayerConfig(0);
        if (lp) {
            SavedProfile def;
            def.name = (const char*)lp->name;
            def.globalID = (const char*)lp->globalID;
            def.autoLogin = lp->autoLogin;
            g_SavedProfiles.push_back(def);
        }
        g_ProfilesLoaded = true;
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        size_t firstTab = line.find('\t');
        if (firstTab == std::string::npos) continue;
        size_t secondTab = line.find('\t', firstTab + 1);
        if (secondTab == std::string::npos) continue;
        
        SavedProfile p;
        p.name = line.substr(0, firstTab);
        p.globalID = line.substr(firstTab + 1, secondTab - firstTab - 1);
        p.autoLogin = (line.substr(secondTab + 1) == "1");
        g_SavedProfiles.push_back(p);
    }
    f.close();
    g_ProfilesLoaded = true;
}

static void SaveProfiles() {
    std::string path = (const char*)tDirectories::Var().GetWritePath("saved_profiles.txt");
    std::ofstream f(path);
    if (!f.is_open()) return;
    for (const auto& p : g_SavedProfiles) {
        f << p.name << "\t" << p.globalID << "\t" << (p.autoLogin ? "1" : "0") << "\n";
    }
    f.close();
}

static void LoadColors() {
    g_SavedColors.clear();
    std::string path = (const char*)tDirectories::Var().GetWritePath("saved_colors.txt");
    std::ifstream f(path);
    if (!f.is_open()) {
        g_SavedColors.push_back({"Default Cyan", 0, 15, 15});
        g_SavedColors.push_back({"Classic Red", 15, 0, 0});
        g_SavedColors.push_back({"Retro Purple", 10, 0, 15});
        g_SavedColors.push_back({"Neon Green", 0, 15, 2});
        g_ColorsLoaded = true;
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        size_t t1 = line.find('\t');
        if (t1 == std::string::npos) continue;
        size_t t2 = line.find('\t', t1 + 1);
        if (t2 == std::string::npos) continue;
        size_t t3 = line.find('\t', t2 + 1);
        if (t3 == std::string::npos) continue;
        
        SavedColor c;
        c.name = line.substr(0, t1);
        try {
            c.r = std::stoi(line.substr(t1 + 1, t2 - t1 - 1));
            c.g = std::stoi(line.substr(t2 + 1, t3 - t2 - 1));
            c.b = std::stoi(line.substr(t3 + 1));
        } catch (...) {
            c.r = 0;
            c.g = 0;
            c.b = 0;
        }
        g_SavedColors.push_back(c);
    }
    f.close();
    g_ColorsLoaded = true;
}

static void SaveColors() {
    std::string path = (const char*)tDirectories::Var().GetWritePath("saved_colors.txt");
    std::ofstream f(path);
    if (!f.is_open()) return;
    for (const auto& c : g_SavedColors) {
        f << c.name << "\t" << c.r << "\t" << c.g << "\t" << c.b << "\n";
    }
    f.close();
}

static void BackupDefaultCameraSettings() {
    if (!g_DefaultCameraSettings.empty()) return;
    
    tConfItemBase::tConfItemMap const & confmap = tConfItemBase::GetConfItemMap();
    for (tConfItemBase::tConfItemMap::const_iterator iter = confmap.begin(); iter != confmap.end(); ++iter) {
        tString title = iter->first;
        if (title.StartsWith("CAMERA_")) {
            std::stringstream ss;
            iter->second->WriteVal(ss);
            g_DefaultCameraSettings[title] = ss.str();
        }
    }
}

static void ResetCameraSettingsToDefault() {
    BackupDefaultCameraSettings();
    
    tConfItemBase::tConfItemMap & confmap = const_cast<tConfItemBase::tConfItemMap&>(tConfItemBase::GetConfItemMap());
    for (const auto& pair : g_DefaultCameraSettings) {
        tString title = pair.first;
        auto iter = confmap.find(title);
        if (iter != confmap.end()) {
            std::stringstream ss(pair.second);
            iter->second->ReadVal(ss);
        }
    }
}

static void ApplyCameraConfigNoSave(const std::string& filename) {
    if (filename.empty()) return;

    // These files come from the player's own config directory and are read the
    // same way the startup configuration is, so they get the same standing.
    // Without this the reader runs at whatever level happens to be current,
    // which outside startup is the lowest one, and every server-side setting
    // the file happens to contain prints a complaint about access rights.
    tCurrentAccessLevel level( tAccessLevel_Owner, true );

    ResetCameraSettingsToDefault();

    std::ifstream fVar;
    if (tDirectories::Var().Open(fVar, filename.c_str())) {
        tConfItemBase::LoadAll(fVar, false);
        fVar.close();
    } else {
        std::ifstream fConfig;
        if (tDirectories::Config().Open(fConfig, filename.c_str())) {
            tConfItemBase::LoadAll(fConfig, false);
            fConfig.close();
        } else {
            std::ifstream fLocal(filename);
            if (fLocal.is_open()) {
                tConfItemBase::LoadAll(fLocal, false);
                fLocal.close();
            }
        }
    }
    
    sg_activeCameraConfig = filename.c_str();
}

static void ApplyCameraConfig(const std::string& filename) {
    ApplyCameraConfigNoSave(filename);
    st_SaveConfig();
}

static void ReapplyCameraConfig() {
    if (sg_activeCameraConfig != "") {
        ApplyCameraConfigNoSave((const char*)sg_activeCameraConfig);
    }
    TranslateAllModKeybinds();
}

static std::vector<std::string> ListCfgFiles(const std::string& dirPath) {
    std::vector<std::string> files;
    DIR* dir = opendir(dirPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.length() > 4 && name.substr(name.length() - 4) == ".cfg") {
                if (name != "user.cfg" && name != "settings.cfg" && name != "master.cfg" && name != "autoexec.cfg" && name != "aiplayers.cfg") {
                    files.push_back(name);
                }
            }
        }
        closedir(dir);
    }
    return files;
}

static std::vector<std::string> GetAvailableCameraConfigs() {
    std::vector<std::string> configs;
    auto localConfigs = ListCfgFiles(".");
    configs.insert(configs.end(), localConfigs.begin(), localConfigs.end());
    
    tString varDir = tDirectories::Var().GetWritePath("x");
    std::string varPath;
    if (varDir.Len() > 2) {
        varPath = (const char*)varDir.SubStr(0, varDir.Len() - 2);
    } else {
        varPath = ".";
    }
    if (!varPath.empty()) {
        auto varConfigs = ListCfgFiles(varPath);
        for (const auto& vc : varConfigs) {
            if (std::find(configs.begin(), configs.end(), vc) == configs.end()) {
                configs.push_back(vc);
            }
        }
    }
    std::sort(configs.begin(), configs.end());
    return configs;
}

static bool CopyFile(const std::string& src, const std::string& dst) {
    std::ifstream srcFile(src, std::ios::binary);
    if (!srcFile.is_open()) return false;
    std::ofstream dstFile(dst, std::ios::binary);
    if (!dstFile.is_open()) return false;
    dstFile << srcFile.rdbuf();
    return true;
}

static void CreateDir(const std::string& path) {
#ifdef WIN32
    mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0777);
#endif
}

static bool IsDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }
    return false;
}

static std::vector<std::string> GetAvailableTexturePacks() {
    std::vector<std::string> packs;
    std::string base = "./custom_packs";
    CreateDir(base);
    CreateDir(base + "/Default");
    CreateDir(base + "/Default/textures");
    CreateDir(base + "/Default/models");
    
    DIR* dir = opendir(base.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                std::string fullPath = base + "/" + name;
                if (IsDirectory(fullPath)) {
                    packs.push_back(name);
                }
            }
        }
        closedir(dir);
    }
    if (std::find(packs.begin(), packs.end(), "Default") == packs.end()) {
        packs.push_back("Default");
    }
    std::sort(packs.begin(), packs.end());
    return packs;
}

static void BackupDefaultPacks() {
    std::string base = "./custom_packs/Default";
    DIR* dir = opendir((base + "/textures").c_str());
    bool empty = true;
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                empty = false;
                break;
            }
        }
        closedir(dir);
    }
    
    if (empty) {
        DIR* srcDir = opendir("./textures");
        if (srcDir) {
            struct dirent* entry;
            while ((entry = readdir(srcDir)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") {
                    CopyFile("./textures/" + name, base + "/textures/" + name);
                }
            }
            closedir(srcDir);
        }
        DIR* srcModels = opendir("./models");
        if (srcModels) {
            struct dirent* entry;
            while ((entry = readdir(srcModels)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") {
                    CopyFile("./models/" + name, base + "/models/" + name);
                }
            }
            closedir(srcModels);
        }
    }
}

static void CopyFolderContents(const std::string& src, const std::string& dst) {
    CreateDir(dst);
    DIR* dir = opendir(src.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                CopyFile(src + "/" + name, dst + "/" + name);
            }
        }
        closedir(dir);
    }
}

static void ApplyTexturePackNoSave(const std::string& packName) {
    BackupDefaultPacks();
    
    CopyFolderContents("./custom_packs/Default/textures", "./textures");
    CopyFolderContents("./custom_packs/Default/models", "./models");
    
    if (packName != "Default") {
        CopyFolderContents("./custom_packs/" + packName + "/textures", "./textures");
        CopyFolderContents("./custom_packs/" + packName + "/models", "./models");
    }
    
    sg_activeTexturePack = packName.c_str();
    
    rITexture::UnloadAll();
    rITexture::LoadAll();
}

static void ApplyTexturePack(const std::string& packName) {
    ApplyTexturePackNoSave(packName);
    st_SaveConfig();
}

static void DrawCycleColorPreviewForRGB(int rgb[3], const char* title) {
    int rc = rgb[0] & 15;
    int gc = rgb[1] & 15;
    int bc = rgb[2] & 15;

    float rc_f = (float)rc;
    float gc_f = (float)gc;
    float bc_f = (float)bc;

    while (rc_f + gc_f + bc_f < 3.0f) {
        rc_f += 0.5f;
        gc_f += 0.5f;
        bc_f += 0.5f;
    }

    ImVec4 cycleColor(rc_f * 17.0f / 255.0f, gc_f * 17.0f / 255.0f, bc_f * 17.0f / 255.0f, 1.0f);

    float rt_f = (float)rgb[0];
    float gt_f = (float)rgb[1];
    float bt_f = (float)rgb[2];

    if (rt_f > 15.0f) rt_f = 15.0f;
    if (gt_f > 15.0f) gt_f = 15.0f;
    if (bt_f > 15.0f) bt_f = 15.0f;
    if (rt_f < 0.0f) rt_f = 0.0f;
    if (gt_f < 0.0f) gt_f = 0.0f;
    if (bt_f < 0.0f) bt_f = 0.0f;

    while (rt_f + gt_f + bt_f < 6.0f) {
        rt_f += 0.5f;
        gt_f += 0.5f;
        bt_f += 0.5f;
    }

    ImVec4 trailColor(rt_f / 15.0f, gt_f / 15.0f, bt_f / 15.0f, 1.0f);

    ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "%s", title);
    ImGui::Spacing();
    
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 110.0f;
    ImVec2 canvas_size(width, height);
    
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    ImU32 bgCol = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
    dl->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + width, canvas_pos.y + height), bgCol, 8.0f);
    dl->AddRect(canvas_pos, ImVec2(canvas_pos.x + width, canvas_pos.y + height), borderCol, 8.0f);
    
    ImU32 gridCol = ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.25f, 0.25f));
    float gridSpacing = 20.0f;
    for (float x = canvas_pos.x + gridSpacing; x < canvas_pos.x + width; x += gridSpacing) {
        dl->AddLine(ImVec2(x, canvas_pos.y), ImVec2(x, canvas_pos.y + height), gridCol);
    }
    for (float y = canvas_pos.y + gridSpacing; y < canvas_pos.y + height; y += gridSpacing) {
        dl->AddLine(ImVec2(canvas_pos.x, y), ImVec2(canvas_pos.x + width, y), gridCol);
    }
    
    ImVec2 cyclePos(canvas_pos.x + width - 65.0f, canvas_pos.y + height * 0.55f);
    ImVec2 trailLeftTop(canvas_pos.x + 15.0f, canvas_pos.y + height * 0.15f);
    ImVec2 trailLeftBottom(canvas_pos.x + 15.0f, canvas_pos.y + height * 0.85f);
    ImVec2 trailRightTop(cyclePos.x, cyclePos.y - 12.0f);
    ImVec2 trailRightBottom(cyclePos.x, cyclePos.y + 12.0f);
    
    ImVec2 trailPts[4] = {
        trailLeftTop,
        trailRightTop,
        trailRightBottom,
        trailLeftBottom
    };
    dl->AddConvexPolyFilled(trailPts, 4, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.40f)));
    
    dl->AddLine(trailLeftTop, trailRightTop, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.95f)), 2.5f);
    dl->AddLine(trailLeftBottom, trailRightBottom, ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.45f)), 1.5f);
    
    ImU32 zigCol = ImGui::GetColorU32(ImVec4(trailColor.x, trailColor.y, trailColor.z, 0.65f));
    int numZigs = 7;
    for (int i = 0; i < numZigs; ++i) {
        float t0 = (float)i / numZigs;
        float t1 = ((float)i + 0.5f) / numZigs;
        float t2 = ((float)i + 1.0f) / numZigs;
        
        ImVec2 p0(trailLeftTop.x + (trailRightTop.x - trailLeftTop.x) * t0, trailLeftTop.y + (trailRightTop.y - trailLeftTop.y) * t0);
        ImVec2 p1(trailLeftBottom.x + (trailRightBottom.x - trailLeftBottom.x) * t1, trailLeftBottom.y + (trailRightBottom.y - trailLeftBottom.y) * t1);
        ImVec2 p2(trailLeftTop.x + (trailRightTop.x - trailLeftTop.x) * t2, trailLeftTop.y + (trailRightTop.y - trailLeftTop.y) * t2);
        
        dl->AddLine(p0, p1, zigCol, 1.5f);
        dl->AddLine(p1, p2, zigCol, 1.5f);
    }
    
    ImVec2 rearWheel = cyclePos;
    ImVec2 frontWheel(cyclePos.x + 38.0f, cyclePos.y + 2.0f);
    
    dl->AddCircleFilled(rearWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.15f, 1.0f)));
    dl->AddCircleFilled(rearWheel, 5.0f, ImGui::GetColorU32(cycleColor));
    dl->AddCircle(rearWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.75f, 1.0f)), 16, 2.0f);
    
    dl->AddCircleFilled(frontWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.15f, 1.0f)));
    dl->AddCircleFilled(frontWheel, 5.0f, ImGui::GetColorU32(cycleColor));
    dl->AddCircle(frontWheel, 8.5f, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.75f, 1.0f)), 16, 2.0f);
    
    ImVec2 bodyPts[6] = {
        ImVec2(cyclePos.x - 12.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 5.0f, cyclePos.y - 12.0f),
        ImVec2(cyclePos.x + 20.0f, cyclePos.y - 12.0f),
        ImVec2(cyclePos.x + 40.0f, cyclePos.y - 2.0f),
        ImVec2(cyclePos.x + 36.0f, cyclePos.y + 6.0f),
        ImVec2(cyclePos.x - 2.0f, cyclePos.y + 6.0f)
    };
    dl->AddConvexPolyFilled(bodyPts, 6, ImGui::GetColorU32(ImVec4(0.20f, 0.20f, 0.24f, 1.0f)));
    dl->AddPolyline(bodyPts, 6, ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.50f, 1.0f)), true, 1.0f);
    
    ImVec2 accentPts[4] = {
        ImVec2(cyclePos.x + 3.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 16.0f, cyclePos.y - 8.0f),
        ImVec2(cyclePos.x + 28.0f, cyclePos.y - 4.0f),
        ImVec2(cyclePos.x + 22.0f, cyclePos.y + 2.0f)
    };
    dl->AddConvexPolyFilled(accentPts, 4, ImGui::GetColorU32(cycleColor));
    dl->AddPolyline(accentPts, 4, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.35f)), true, 1.0f);
    
    ImVec2 canopyPts[4] = {
        ImVec2(cyclePos.x + 8.0f, cyclePos.y - 11.0f),
        ImVec2(cyclePos.x + 18.0f, cyclePos.y - 11.0f),
        ImVec2(cyclePos.x + 23.0f, cyclePos.y - 6.0f),
        ImVec2(cyclePos.x + 13.0f, cyclePos.y - 6.0f)
    };
    dl->AddConvexPolyFilled(canopyPts, 4, ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.12f, 0.85f)));
    dl->AddPolyline(canopyPts, 4, ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.40f, 1.0f)), true, 1.0f);

    char colorLabel[128];
    snprintf(colorLabel, sizeof(colorLabel), "Cycle: R:%d G:%d B:%d | Trail: R:%d G:%d B:%d",
             rgb[0], rgb[1], rgb[2], rgb[0], rgb[1], rgb[2]);
    
    ImGui::Dummy(canvas_size);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "%s", colorLabel);
}

// Two buttons pushed against the right edge of a row, each as wide as its own
// label. A fixed size clips the word on one font and floats on another.
static void rc_RowButtons( float & loadX, float & loadW, float & delX, float & delW )
{
    ImGuiStyle const & style = ImGui::GetStyle();
    loadW = ImGui::CalcTextSize( "Load" ).x + style.FramePadding.x * 2.0f;
    delW  = ImGui::CalcTextSize( "Delete" ).x + style.FramePadding.x * 2.0f;
    float const gap = style.ItemSpacing.x;
    delX  = ImGui::GetContentRegionMax().x - delW;
    loadX = delX - gap - loadW;
}

static void RenderArmagetronColoredText(const char* text) {
    ImVec4 currentColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    const char* p = text;
    std::string currentSegment = "";
    
    while (*p != '\0') {
        if (*p == '0' && *(p + 1) == 'x') {
            bool isHex = true;
            for (int i = 2; i < 8; i++) {
                char c = *(p + i);
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                    isHex = false;
                    break;
                }
            }
            if (isHex) {
                if (!currentSegment.empty()) {
                    ImGui::TextColored(currentColor, "%s", currentSegment.c_str());
                    ImGui::SameLine(0.0f, 0.0f);
                    currentSegment = "";
                }
                char hex[7];
                memcpy(hex, p + 2, 6);
                hex[6] = '\0';
                unsigned int rgbVal = 0;
                std::stringstream ss;
                ss << std::hex << hex;
                ss >> rgbVal;
                
                float r = ((rgbVal >> 16) & 0xFF) / 255.0f;
                float g = ((rgbVal >> 8) & 0xFF) / 255.0f;
                float b = (rgbVal & 0xFF) / 255.0f;
                currentColor = ImVec4(r, g, b, 1.0f);
                
                p += 8;
                continue;
            }
        }
        currentSegment += *p;
        p++;
    }
    if (!currentSegment.empty()) {
        ImGui::TextColored(currentColor, "%s", currentSegment.c_str());
    }
}


#define COL_TEXT ImVec4(0.95f, 0.95f, 0.95f, 1.0f)
#define COL_TEXT_DIM ImVec4(0.50f, 0.50f, 0.50f, 1.0f)

static bool g_Initialized = false;
static bool g_MenuOpen = false;
static ImVec2 g_LastDisplaySize = ImVec2(0, 0);
void CheckDisplaySizeRebuild() {
    ImGuiIO& io = ImGui::GetIO();

    // Applying display settings builds a new GL context and everything the old
    // one held, the font atlas included, dies with it. Watching the size alone
    // missed every change that keeps the resolution - fullscreen, colour
    // depth, vsync - and left the menu drawing text out of a texture that no
    // longer existed. Watching the context itself catches all of them.
    static SDL_GLContext lastContext = NULL;
    SDL_GLContext context = SDL_GL_GetCurrentContext();
    const bool contextChanged = (context != lastContext);
    lastContext = context;

    const bool sizeChanged = (g_LastDisplaySize.x != io.DisplaySize.x ||
                              g_LastDisplaySize.y != io.DisplaySize.y);

    if (contextChanged || sizeChanged) {
        if (g_LastDisplaySize.x != 0 && g_LastDisplaySize.y != 0) {
            ImGui_ImplOpenGL2_DestroyDeviceObjects();
            ImGui_ImplOpenGL2_CreateDeviceObjects();
        }
        g_LastDisplaySize = io.DisplaySize;
    }
}
static float g_MenuAlpha = 0.0f;
static int g_ActiveTab = 0;
static int g_ModMenuTab = 0;
static bool g_CloseInGameMenuRequested = false;
static uAction* s_BindingAction = nullptr;
static int s_BindingSlot = 0;
static GLuint g_AvatarTexture = 0;
static std::vector<std::string> s_JoinedQueues;
static char s_RclUsername[128] = "";
static int s_DashSubTab = 0; // 0: Info, 1: Profiles
static int s_ActiveProfileIndex = -1;
static float s_ScrollProfilesY = 0.0f;

// Font pointers and main menu active flag
ImFont* g_FontDefault = nullptr;
ImFont* g_FontHeader = nullptr;
ImFont* g_FontDisplay = nullptr;
ImFont* g_FontMono = nullptr;
// Multi-size font array (sizes: 10, 12, 14, 16, 18, 20, 24, 28, 32)
ImFont* g_FontSizes[MOD_FONT_SIZE_COUNT] = {};
ImFont* g_FontNames[MOD_FONT_SIZE_COUNT] = {};
bool ModMenu::g_MainMenuActive = true;
bool ModMenu::g_CustomMainMenuTempDisabled = false;
bool ModMenu::g_InGameMenuOpen = false;
bool ModMenu::g_ModalOverlayOpen = false;

//! Which field the typing goes into: 0 the name, 1 the password.
static int  s_loginField = 0;

//! Reachable from the input queue, which cannot see into this namespace.
bool rc_ModalOverlayOpen()
{
    return ModMenu::g_ModalOverlayOpen;
}

// How much the arena is dimmed behind the in-game menu. None by default.
float g_MenuDim = 0.0f;
static tConfItem<float> conf_menuDim( "MOD_MENU_DIM", g_MenuDim );
std::function<void()> ModMenu::g_PendingLegacyMenuAction = nullptr;

// the server browser flips the net state to nCLIENT while it polls the master
// list, so that alone is not proof we joined anything; a net id is only handed
// out once a login actually went through
static bool sg_OnRemoteServer()
{
    return sn_GetNetState() == nCLIENT && sn_myNetID != 0;
}

static nServerInfoBase* s_PendingConnectServer = nullptr;
static nServerInfoRedirect* s_DirectRedirectServer = nullptr;
static bool s_PendingStartLocalGame = false;
// 0 = plain local play, 1 = Mazing Training preset
static int  s_LocalPreset = 0;
static bool s_PendingReconnect = false;

static nServerInfo* s_SelectedServer = nullptr;
static int s_SelectedFavoriteIdx = -1;
static bool s_ServerQueryStarted = false;
static char s_DirectIP[128] = "";
static int s_DirectPort = 4534;


static bool g_OpenResetHUDPopup = false;
static bool g_OpenApplyConfigPopup = false;
static bool g_OpenDeleteConfigPopup = false;
static bool g_OpenCreateConfigPopup = false;
static bool g_OpenUpdateConfigPopup = false;
static bool g_OpenResetAllPopup = false;
static bool g_OpenThemePopup = false;
static bool g_OpenDirectConnectModal = false;
static bool g_OpenServerDetailsModal = false;


// Tab sliding animation states
static float g_TabUnderlineX = 0.0f;
static float g_TabUnderlineWidth = 0.0f;

struct MenuParticle {
    ImVec2 pos;
    ImVec2 vel;
    float radius;
    float alpha;
    float speed;
    int layer;
    float depth;
};
static std::vector<MenuParticle> g_Particles;


void ModMenu::InitStyle() {
    // Everything the look is made of now lives in one place, so a panel can
    // no longer decide on its own what a border or a gap is worth here.
    rc_ApplyStyle( ImGui::GetStyle(), 1.0f );
}

// Shader-based shadow placeholder
static void RenderTextureGlow(ImVec2 pos, ImVec2 size, ImU32 color, float rounding) {
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(pos.x - 4.0f, pos.y - 4.0f), 
        ImVec2(pos.x + size.x + 4.0f, pos.y + size.y + 4.0f), 
        color, rounding
    );
}

// Multi-layered soft shadow glow (CSS-like box-shadow)
static void RenderSoftGlow(ImDrawList* dl, ImVec2 pos, ImVec2 size, ImU32 color, float rounding) {
    float glowRadius[] = { 3.0f, 7.0f, 12.0f };
    float glowAlpha[] = { 0.45f, 0.22f, 0.08f };
    
    unsigned int r = (color >> 0) & 0xFF;
    unsigned int g = (color >> 8) & 0xFF;
    unsigned int b = (color >> 16) & 0xFF;
    
    for (int i = 0; i < 3; i++) {
        ImU32 layerCol = IM_COL32(r, g, b, (int)(glowAlpha[i] * 255));
        float pad = glowRadius[i];
        dl->AddRectFilled(
            ImVec2(pos.x - pad, pos.y - pad),
            ImVec2(pos.x + size.x + pad, pos.y + size.y + pad),
            layerCol,
            rounding + pad
        );
    }
}

// Draw a filled rectangle with rounded corners matching the border perfectly
static void AddRoundedGradientRect(ImDrawList* dl, ImVec2 p_min, ImVec2 p_max, ImU32 col_top, ImU32 col_bot, float rounding) {
    unsigned char r1 = (col_top >> 0) & 0xFF;
    unsigned char g1 = (col_top >> 8) & 0xFF;
    unsigned char b1 = (col_top >> 16) & 0xFF;
    unsigned char a1 = (col_top >> 24) & 0xFF;

    unsigned char r2 = (col_bot >> 0) & 0xFF;
    unsigned char g2 = (col_bot >> 8) & 0xFF;
    unsigned char b2 = (col_bot >> 16) & 0xFF;
    unsigned char a2 = (col_bot >> 24) & 0xFF;

    ImU32 col = IM_COL32((r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2, (a1 + a2) / 2);
    dl->AddRectFilled(p_min, p_max, col, rounding);
}

// Get the theme color mapped to position context (horizontal gradient support)
ImU32 GetThemeColor(float t) {
    float alphaVal = (g_MenuAlpha < 0.0f) ? 0.0f : ((g_MenuAlpha > 1.0f) ? 1.0f : g_MenuAlpha);
    if (g_RGBAccent) {
        float menuTime = (float)ImGui::GetTime();
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(fmodf(menuTime * g_RGBSpeed, 1.0f), 1.0f, 1.0f, r, g, b);
        return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(alphaVal * 255));
    }
    float clampedT = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
    if (g_GradientAccent) {
        ImVec4 col;
        col.x = ImLerp(g_AccentColor1.x, g_AccentColor2.x, clampedT);
        col.y = ImLerp(g_AccentColor1.y, g_AccentColor2.y, clampedT);
        col.z = ImLerp(g_AccentColor1.z, g_AccentColor2.z, clampedT);
        col.w = alphaVal;
        
        col.x = (col.x < 0.0f) ? 0.0f : ((col.x > 1.0f) ? 1.0f : col.x);
        col.y = (col.y < 0.0f) ? 0.0f : ((col.y > 1.0f) ? 1.0f : col.y);
        col.z = (col.z < 0.0f) ? 0.0f : ((col.z > 1.0f) ? 1.0f : col.z);
        
        return ImGui::GetColorU32(col);
    }
    ImVec4 col = ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, alphaVal);
    col.x = (col.x < 0.0f) ? 0.0f : ((col.x > 1.0f) ? 1.0f : col.x);
    col.y = (col.y < 0.0f) ? 0.0f : ((col.y > 1.0f) ? 1.0f : col.y);
    col.z = (col.z < 0.0f) ? 0.0f : ((col.z > 1.0f) ? 1.0f : col.z);
    return ImGui::GetColorU32(col);
}

// Visual-only Animated Toggle
static void RenderToggleVisual(ImVec2 pos, bool v, ImGuiID id, float itemAlpha) {
    float height = 20.0f;
    float width = 38.0f;
    float radius = height * 0.5f;

    ImGuiStorage* storage = ImGui::GetStateStorage();
    float t = storage->GetFloat(id, v ? 1.0f : 0.0f);
    float target = v ? 1.0f : 0.0f;
    t = ImLerp(t, target, ImGui::GetIO().DeltaTime * 15.0f);
    storage->SetFloat(id, t);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    float knobX = ImLerp(pos.x + radius, pos.x + width - radius, t);
    float knobY = pos.y + radius;
    
    // Texture-based glow architecture
    if (t > 0.01f) {
        ImU32 glowColor = GetThemeColor(0.5f);
        glowColor = (glowColor & 0x00FFFFFF) | (((unsigned int)(0.2f * t * 255 * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(knobX - radius, knobY - radius), ImVec2(radius*2, radius*2), glowColor, radius + 5.0f);
    }

    ImU32 bgCol = ImGui::GetColorU32(ImVec4(
        ImLerp(0.16f, (g_AccentColor1.x + g_AccentColor2.x) * 0.5f, t),
        ImLerp(0.16f, (g_AccentColor1.y + g_AccentColor2.y) * 0.5f, t),
        ImLerp(0.20f, (g_AccentColor1.z + g_AccentColor2.z) * 0.5f, t),
        itemAlpha
    ));
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bgCol, radius);

    // Knob Drop Shadow
    dl->AddCircleFilled(ImVec2(knobX, knobY + 1.0f), radius - 2.0f, IM_COL32(0, 0, 0, 100 * itemAlpha));
    // Knob
    dl->AddCircleFilled(ImVec2(knobX, knobY), radius - 2.0f, IM_COL32(255, 255, 255, 255 * itemAlpha));
}

// A description cut to the height it has been given.
//
// The card is a fixed size and the text wraps to its width, so a long one
// simply ran past the bottom edge and was sliced off mid sentence. This walks
// back to the last word that still fits and ends it properly; the whole
// sentence is a hover away, so nothing is actually lost.
static char const * FitDescription( char const * desc, float wrap, float avail, float fontSize, std::string & scratch )
{
    if ( !desc || !*desc )
        return desc;

    ImFont * font = ImGui::GetFont();
    if ( font->CalcTextSizeA( fontSize, FLT_MAX, wrap, desc ).y <= avail )
        return desc;

    size_t const len = strlen( desc );
    size_t good = 0;

    for ( size_t i = 0; i < len; ++i )
    {
        if ( desc[i] != ' ' && i + 1 != len )
            continue;

        scratch.assign( desc, i );
        scratch += "...";

        if ( font->CalcTextSizeA( fontSize, FLT_MAX, wrap, scratch.c_str() ).y > avail )
            break;

        good = i;
    }

    if ( good == 0 )
        return desc;

    scratch.assign( desc, good );
    scratch += "...";
    return scratch.c_str();
}


// Which key row is listening for a press, if any. Held as the address of the
// setting itself, so no two rows can ever both be waiting.
static int s_KeyCaptureFor = 0;

// One key, laid out as a card like every other setting on the page.
//
// The rows used to be drawn straight into the interface's own cursor flow
// while everything around them was placed on an absolute grid; the two had no
// idea about each other, so the keys landed on top of the switches above them.
bool KeyItemAbsolute(ImVec2 pos, ImVec2 size, const char* title, int* key, float alphaMult = 1.0f) {
    ImGui::PushID(title);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);

    bool const waiting = ( s_KeyCaptureFor == (int)(intptr_t)key );

    ImGui::SetCursorScreenPos(pos);
    bool changed = false;
    if (ImGui::InvisibleButton("##keyrow", size)) {
        s_KeyCaptureFor = waiting ? 0 : (int)(intptr_t)key;
        se_PlayUiClick();
    }

    bool hovered = ImGui::IsItemHovered();
    hoverT = ImLerp(hoverT, hovered ? 1.0f : 0.0f, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);

    float itemAlpha = g_MenuAlpha * alphaMult;

    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    if (hoverT > 0.001f) {
        float scaleFactor = 1.0f + 0.02f * hoverT;
        renderPos.x -= (size.x * scaleFactor - size.x) * 0.5f;
        renderPos.y -= (size.y * scaleFactor - size.y) * 0.5f;
        renderSize = ImVec2(size.x * scaleFactor, size.y * scaleFactor);
    }

    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);

    ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);

    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (waiting)
        borderCol = ImGui::GetColorU32(ImVec4(0.95f, 0.55f, 0.25f, itemAlpha));
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f + 0.5f * hoverT);

    dl->AddText(ImVec2(renderPos.x + 18.0f, renderPos.y + 14.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), title);

    std::string caption;
    if (waiting)
        caption = "press a key, escape clears";
    else if (*key > 0)
        caption = SDL_GetKeyName((SDL_Keycode)*key);
    else
        caption = "not set";
    if (caption.empty())
        caption = "unknown";

    ImU32 capCol = waiting ? ImGui::GetColorU32(ImVec4(0.95f, 0.65f, 0.30f, itemAlpha))
                           : ImGui::GetColorU32(ImVec4(0.55f, 0.56f, 0.62f, itemAlpha));
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f,
                ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), capCol, caption.c_str());

    // the key itself, on the right, so a page of them can be read down the edge
    if (!waiting && *key > 0) {
        char const * name = SDL_GetKeyName((SDL_Keycode)*key);
        if (name && *name) {
            ImVec2 sz = ImGui::CalcTextSize(name);
            float const boxW = sz.x + 22.0f;
            ImVec2 boxAt(renderPos.x + renderSize.x - boxW - 16.0f, renderPos.y + (renderSize.y - 26.0f) * 0.5f);
            dl->AddRectFilled(boxAt, ImVec2(boxAt.x + boxW, boxAt.y + 26.0f),
                              ImGui::GetColorU32(ImVec4(0.16f, 0.16f, 0.19f, itemAlpha)), 8.0f);
            dl->AddRect(boxAt, ImVec2(boxAt.x + boxW, boxAt.y + 26.0f),
                        ImGui::GetColorU32(ImVec4(0.26f, 0.27f, 0.31f, itemAlpha)), 8.0f);
            dl->AddText(ImVec2(boxAt.x + 11.0f, boxAt.y + 5.0f),
                        ImGui::GetColorU32(ImVec4(0.88f, 0.90f, 0.94f, itemAlpha)), name);
        }
    }

    if (waiting) {
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
            if (!ImGui::IsKeyPressed((ImGuiKey)k, false))
                continue;

            if (k == ImGuiKey_Escape) {
                *key = 0;
            } else {
                char const * name = ImGui::GetKeyName((ImGuiKey)k);
                SDL_Keycode code = name ? SDL_GetKeyFromName(name) : SDLK_UNKNOWN;
                if (code != SDLK_UNKNOWN)
                    *key = (int)code;
            }

            s_KeyCaptureFor = 0;
            changed = true;
            st_SaveConfig();
            break;
        }
    }

    ImGui::PopID();
    return changed;
}

bool SettingItemAbsolute(ImVec2 pos, ImVec2 size, const char* title, const char* desc, bool* v, int* keybind = nullptr, float alphaMult = 1.0f) {
    ImGui::PushID(title);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    // We register the item at the static position
    ImGui::SetCursorScreenPos(pos);
    bool changed = false;
    if (ImGui::InvisibleButton("##row", size)) {
        *v = !*v;
        changed = true;
        se_PlayUiClick();
    }
    
    bool hovered = ImGui::IsItemHovered();
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    // Check for Right Click to open popup
    bool rightClicked = false;
    if (hovered && ImGui::IsMouseClicked(1)) {
        rightClicked = true;
    }
    
    // Multiplied alpha for transition transitions
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    // Calculate magnetic / offset position
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    if (hoverT > 0.001f) {
        ImVec2 mousePos = io.MousePos;
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        
        // Dynamic shift towards the cursor (magnetic offset)
        float maxOffset = 5.0f; 
        ImVec2 delta(mousePos.x - center.x, mousePos.y - center.y);
        float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
        if (len > 1.0f) {
            renderPos.x += (delta.x / len) * maxOffset * hoverT;
            renderPos.y += (delta.y / len) * maxOffset * hoverT;
        }
        
        // Slight expansion/scale-up
        float scaleFactor = 1.0f + 0.02f * hoverT;
        float newW = size.x * scaleFactor;
        float newH = size.y * scaleFactor;
        renderPos.x -= (newW - size.x) * 0.5f;
        renderPos.y -= (newH - size.y) * 0.5f;
        renderSize = ImVec2(newW, newH);
    }
    
    // Render Panel Background with Vertical Gradient & Rounding (24.0f)
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    if (hoverT > 0.001f) {
        // Soft backdrop glow
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.12f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw a small left indicator stripe
    ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);
    
    // Render Border (Glows dynamically on hover, softer unhovered, 24.0f rounding)
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (hoverT > 0.001f) {
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.12f, ((themeCol >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeCol >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.14f, ((themeCol >> 16) & 0xFF) / 255.0f, hoverT),
            itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f + 0.5f * hoverT);
    
    // Typography aligned & shifted right to account for stripe
    dl->AddText(ImVec2(renderPos.x + 18.0f, renderPos.y + 14.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), title);
    if (strcmp(title, "Anti-Aliasing (MSAA)") == 0) {
        float titleWidth = ImGui::CalcTextSize(title).x;
        dl->AddText(ImVec2(renderPos.x + 18.0f + titleWidth + 8.0f, renderPos.y + 14.0f), IM_COL32(255, 75, 75, (int)(255 * itemAlpha)), "(Requires Restart)");
    }
    {
        std::string fitted;
        float const wrap = renderSize.x - 76.0f;
        char const * shown = FitDescription( desc, wrap, renderSize.y - 40.0f, ImGui::GetFontSize() - 1.0f, fitted );
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), shown, nullptr, wrap);
        if ( hovered && shown != desc )
            ImGui::SetTooltip( "%s", desc );
    }
    
    // Toggle pinned to exact right edge, vertically centered
    RenderToggleVisual(ImVec2(renderPos.x + renderSize.x - 53.0f, renderPos.y + (renderSize.y - 20.0f) * 0.5f), *v, ImGui::GetID("##toggle"), itemAlpha);

    if (rightClicked) {
        ImGui::OpenPopup("CardSettingsPopup");
    }
    
    // Dynamic popup dimensions based on feature needs - spacious & no scroll
    ImVec2 popupSize(300.0f, 130.0f);
    if (strcmp(title, "Custom Fog") == 0) {
        popupSize = ImVec2(380.0f, 250.0f);
    } else if (strcmp(title, "Noclip Mode") == 0) {
        popupSize = ImVec2(340.0f, 175.0f);
    } else if (strcmp(title, "Clean Screen") == 0) {
        popupSize = ImVec2(340.0f, 175.0f);
    }
    
    ImGui::SetNextWindowSize(popupSize, ImGuiCond_Always);
    
    // Style settings popup elements with accent palette
    ImU32 themeColU32 = GetThemeColor(pos.x / 1180.0f);
    ImVec4 themeColFloat = ImGui::ColorConvertU32ToFloat4(themeColU32);
 
    // Clear ImGui's default popup background and border to avoid a dark square behind our rounded corners
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, themeColFloat);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeColFloat);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(themeColFloat.x * 1.1f, themeColFloat.y * 1.1f, themeColFloat.z * 1.1f, themeColFloat.w));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.16f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 24.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    // Draggable settings popup with no scrollbar
    if (ImGui::BeginPopup("CardSettingsPopup", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
        ImDrawList* popDl = ImGui::GetWindowDrawList();
        ImVec2 popPos = ImGui::GetWindowPos();
        ImVec2 popSz = ImGui::GetWindowSize();
        
        // Custom background gradient
        ImU32 popBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, 0.98f));
        ImU32 popBgBot = ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 0.98f));
        AddRoundedGradientRect(popDl, popPos, ImVec2(popPos.x + popSz.x, popPos.y + popSz.y), popBgTop, popBgBot, 24.0f);
        
        // Draw left indicator stripe
        ImU32 popStripeCol = (themeColU32 & 0x00FFFFFF) | (0xE0U << 24);
        popDl->AddRectFilled(ImVec2(popPos.x + 6.0f, popPos.y + 14.0f), ImVec2(popPos.x + 9.0f, popPos.y + popSz.y - 14.0f), popStripeCol, 2.0f);
        
        // Draw custom rounded border to match the background perfectly
        ImU32 popBorderCol = ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.16f, 1.0f));
        popDl->AddRect(ImVec2(popPos.x + 0.5f, popPos.y + 0.5f), ImVec2(popPos.x + popSz.x - 0.5f, popPos.y + popSz.y - 0.5f), popBorderCol, 24.0f, 0, 1.0f);

        // Draw Header
        ImGui::SetCursorPosX(16.0f);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.95f), "%s", title);
        ImGui::SameLine(popSz.x - 36.0f);
        
        // Rounded close button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.25f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.1f, 0.1f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        if (ImGui::Button("x", ImVec2(20, 20))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleVar();
        // Three were pushed for this button, not four. The fourth came off the
        // stack belonging to the popup around it, so the count stopped adding up
        // and right clicking any row at all brought the client down.
        ImGui::PopStyleColor(3);
        
        popDl->AddLine(ImVec2(popPos.x + 16.0f, popPos.y + 42.0f), ImVec2(popPos.x + popSz.x - 16.0f, popPos.y + 42.0f), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f)));
        ImGui::Dummy(ImVec2(0, 5));
        
        // Keybind option
        if (keybind) {
            ImGui::Dummy(ImVec2(0, 3));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Text("Keybind:");
            
            char bindName[64];
            if (g_BindingKeybindPtr == keybind) {
                strcpy(bindName, "Press any key...");
            } else {
                strcpy(bindName, GetKeyName(*keybind).c_str());
            }
            
            ImGui::SetCursorPosX(16.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.4f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(themeColFloat.x, themeColFloat.y, themeColFloat.z, 0.6f));
            if (ImGui::Button(bindName, ImVec2(popSz.x - 40.0f, 32.0f))) {
                g_BindingKeybindPtr = keybind;
            }
            ImGui::PopStyleColor(3);
        }
        
        // Feature-specific custom sub-options
        if (strcmp(title, "Custom Fog") == 0) {
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Text("Density:");
            ImGui::SetCursorPosX(16.0f);
            ImGui::PushItemWidth(popSz.x - 40.0f);
            ImGui::SliderFloat("##density", &g_FogDensity, 0.0f, 0.1f, "%.4f");
            ImGui::PopItemWidth();
            
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Text("Color:");
            ImGui::SetCursorPosX(16.0f);
            ImGui::PushItemWidth(popSz.x - 40.0f);
            ImVec4 fogCol(g_FogR, g_FogG, g_FogB, 1.0f);
            if (ImGui::ColorEdit3("##color", &fogCol.x)) {
                g_FogR = fogCol.x;
                g_FogG = fogCol.y;
                g_FogB = fogCol.z;
            }
            ImGui::PopItemWidth();
        } else if (strcmp(title, "Noclip Mode") == 0) {
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Text("Speed:");
            ImGui::SetCursorPosX(16.0f);
            ImGui::PushItemWidth(popSz.x - 40.0f);
            extern REAL sg_noclipSpeed;
            float speedVal = (float)sg_noclipSpeed;
            if (ImGui::SliderFloat("##noclip_speed", &speedVal, 10.0f, 200.0f, "%.1f")) {
                sg_noclipSpeed = speedVal;
            }
            ImGui::PopItemWidth();
        } else if (strcmp(title, "Clean Screen") == 0) {
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Checkbox("Hide Console", &sg_noclipHideConsole);
            
            ImGui::Dummy(ImVec2(0, 4));
            ImGui::SetCursorPosX(16.0f);
            ImGui::Checkbox("Hide Names", &sg_noclipHideNames);
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(11);

    ImGui::PopID();
    return changed;
}

bool SliderItemAbsolute(ImVec2 pos, ImVec2 size, const char* title, const char* desc, float* v, float v_min, float v_max, const char* format, float alphaMult = 1.0f) {
    ImGui::PushID(title);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    // Slider Math & Interaction
    float sliderWidth = size.x - 30.0f;
    float trackHeight = 6.0f;
    ImVec2 trackPos(pos.x + 15.0f, pos.y + 60.0f);
    
    ImGui::SetCursorScreenPos(ImVec2(trackPos.x, trackPos.y - 10.0f));
    bool changed = false;
    
    // Use InvisibleButton to handle drag events
    ImGui::InvisibleButton("##slider_click", ImVec2(sliderWidth, 26.0f));
    bool active = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    if (active) {
        float mouseX = io.MousePos.x;
        float t = (mouseX - trackPos.x) / sliderWidth;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        *v = v_min + t * (v_max - v_min);
        changed = true;
    }
    
    // Multiplied alpha for transition transitions
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    // Calculate magnetic / offset position
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    if (hoverT > 0.001f) {
        ImVec2 mousePos = io.MousePos;
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        
        float maxOffset = 5.0f;
        ImVec2 delta(mousePos.x - center.x, mousePos.y - center.y);
        float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
        if (len > 1.0f) {
            renderPos.x += (delta.x / len) * maxOffset * hoverT;
            renderPos.y += (delta.y / len) * maxOffset * hoverT;
        }
        
        float scaleFactor = 1.0f + 0.02f * hoverT;
        float newW = size.x * scaleFactor;
        float newH = size.y * scaleFactor;
        renderPos.x -= (newW - size.x) * 0.5f;
        renderPos.y -= (newH - size.y) * 0.5f;
        renderSize = ImVec2(newW, newH);
    }
    
    // Render Panel Background with Vertical Gradient & Rounding (24.0f)
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.12f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw a small left indicator stripe
    ImU32 themeColSlider = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeColSlider & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);
    
    // Render Border (softer unhovered, theme-colored on hover, 24.0f rounding)
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (hoverT > 0.001f) {
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.12f, ((themeColSlider >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeColSlider >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.14f, ((themeColSlider >> 16) & 0xFF) / 255.0f, hoverT),
            itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f + 0.5f * hoverT);
    
    // Typography aligned & shifted right to account for stripe
    dl->AddText(ImVec2(renderPos.x + 18.0f, renderPos.y + 12.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), title);
    {
        std::string fitted;
        float const wrap = renderSize.x - 36.0f;
        char const * shown = FitDescription( desc, wrap, renderSize.y - 56.0f, ImGui::GetFontSize() - 1.0f, fitted );
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 30.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), shown, nullptr, wrap);
        if ( hovered && shown != desc )
            ImGui::SetTooltip( "%s", desc );
    }
    
    // Interpolate value for rendering
    float currentT = (*v - v_min) / (v_max - v_min);
    if (currentT < 0.0f) currentT = 0.0f;
    if (currentT > 1.0f) currentT = 1.0f;
    
    // Animation using state storage
    ImGuiID grabId = ImGui::GetID("##grab");
    float animT = storage->GetFloat(grabId, currentT);
    animT = ImLerp(animT, currentT, io.DeltaTime * 20.0f);
    storage->SetFloat(grabId, animT);
    
    // Hover/Active scale animation for grab circle
    float grabScale = storage->GetFloat(ImGui::GetID("##grabscale"), 1.0f);
    float targetScale = active ? 1.3f : (ImGui::IsItemHovered() ? 1.15f : 1.0f);
    grabScale = ImLerp(grabScale, targetScale, io.DeltaTime * 15.0f);
    storage->SetFloat(ImGui::GetID("##grabscale"), grabScale);
    
    // Draw track background relative to renderPos (shifted for left stripe alignment)
    ImVec2 renderTrackPos(renderPos.x + 18.0f, renderPos.y + (renderSize.y - 25.0f));
    float renderSliderWidth = renderSize.x - 36.0f;
    dl->AddRectFilled(renderTrackPos, ImVec2(renderTrackPos.x + renderSliderWidth, renderTrackPos.y + trackHeight), ImGui::GetColorU32(ImVec4(0.16f, 0.16f, 0.19f, itemAlpha)), 3.0f);
    
    // Draw track fill
    float filledWidth = renderSliderWidth * animT;
    if (filledWidth > 0.0f) {
        dl->AddRectFilledMultiColor(
            renderTrackPos,
            ImVec2(renderTrackPos.x + filledWidth, renderTrackPos.y + trackHeight),
            GetThemeColor(0.0f), GetThemeColor(animT), GetThemeColor(animT), GetThemeColor(0.0f)
        );
    }
    
    // Draw grab circle with glow shadow
    ImVec2 grabPos(renderTrackPos.x + filledWidth, renderTrackPos.y + trackHeight * 0.5f);
    float grabRadius = 6.0f * grabScale;
    
    // Grab Shadow
    dl->AddCircleFilled(ImVec2(grabPos.x, grabPos.y + 1.0f), grabRadius + 1.5f, IM_COL32(0, 0, 0, 60 * itemAlpha));
    
    // Grab Inner Circle
    dl->AddCircleFilled(grabPos, grabRadius, IM_COL32(255, 255, 255, 255 * itemAlpha));
    dl->AddCircle(grabPos, grabRadius, GetThemeColor(animT), 16, 1.2f);
    
    // Render current value text elegantly relative to renderPos
    char valBuf[64];
    if (strcmp(format, "ANNOUNCER_PACK_FORMAT") == 0) {
        int pack = (int)*v;
        if (pack == 0) strcpy(valBuf, "Dota 2 (Male)");
        else if (pack == 1) strcpy(valBuf, "Quake (Female)");
        else strcpy(valBuf, "Neutral (Hitmarker)");
    } else if (strcmp(format, "CORPSE_TRAIL_STYLE_FORMAT") == 0) {
        int style = (int)*v;
        if (style == 0) strcpy(valBuf, "Legacy (Decay after Delay)");
        else if (style == 1) strcpy(valBuf, "Blinking Warning (Competitive)");
        else strcpy(valBuf, "Gradual Fade & Shrink");
    } else if (strcmp(format, "FLOODLIGHT_MODE_FORMAT") == 0) {
        int mode = (int)*v;
        if (mode == 1) strcpy(valBuf, "Aim at the zones");
        else if (mode == 2) strcpy(valBuf, "Two, one per base");
        else strcpy(valBuf, "Light the whole floor");
    } else if (strcmp(format, "UPSCALE_STYLE_FORMAT") == 0) {
        int style = (int)*v;
        if (style == 1) strcpy(valBuf, "Keep hard edges");
        else if (style == 2) strcpy(valBuf, "Smooth everything");
        else strcpy(valBuf, "Per texture");
    } else if (strcmp(format, "UPSCALE_FACTOR_FORMAT") == 0) {
        int factor = (int)*v;
        if (factor <= 1) strcpy(valBuf, "Off");
        else sprintf(valBuf, "%dx", factor);
    } else if (strcmp(format, "AMBIENT_PARTICLES_MODE_FORMAT") == 0) {
        int mode = (int)*v;
        if (mode == 0) strcpy(valBuf, "Uniform (Everywhere)");
        else strcpy(valBuf, "Zone Only (Inside Sumo)");
    } else if (strcmp(format, "DEATH_VFX_STYLE_FORMAT") == 0) {
        int style = (int)*v;
        if (style == 1) strcpy(valBuf, "1: Crater Decal (Ground)");
        else if (style == 2) strcpy(valBuf, "2: Rising Ghost (Billboard)");
        else if (style == 3) strcpy(valBuf, "3: Lightning Strike");
        else if (style == 4) strcpy(valBuf, "4: Fire Phoenix (Billboard)");
        else if (style == 5) strcpy(valBuf, "5: Gothic Tomb Cross");
        else strcpy(valBuf, "Disabled");
    } else {
        sprintf(valBuf, format, *v);
    }
    ImVec2 valSz = ImGui::CalcTextSize(valBuf);
    dl->AddText(ImVec2(renderPos.x + renderSize.x - 15.0f - valSz.x, renderPos.y + 12.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), valBuf);
    
    ImGui::PopID();
    return changed;
}

bool ColorItemAbsolute(ImVec2 pos, ImVec2 size, const char* title, const char* desc, ImVec4* col, float alphaMult = 1.0f) {
    ImGui::PushID(title);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    ImVec2 btnPos = ImVec2(pos.x + size.x - 45.0f, pos.y + (size.y - 20.0f) * 0.5f);
    ImGui::SetCursorScreenPos(pos);
    
    float rawCol[4] = { col->x, col->y, col->z, col->w };
    bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    // Multiplied alpha for transition transitions
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    // Calculate magnetic / offset position
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    if (hoverT > 0.001f) {
        ImVec2 mousePos = io.MousePos;
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        
        float maxOffset = 5.0f;
        ImVec2 delta(mousePos.x - center.x, mousePos.y - center.y);
        float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
        if (len > 1.0f) {
            renderPos.x += (delta.x / len) * maxOffset * hoverT;
            renderPos.y += (delta.y / len) * maxOffset * hoverT;
        }
        
        float scaleFactor = 1.0f + 0.02f * hoverT;
        float newW = size.x * scaleFactor;
        float newH = size.y * scaleFactor;
        renderPos.x -= (newW - size.x) * 0.5f;
        renderPos.y -= (newH - size.y) * 0.5f;
        renderSize = ImVec2(newW, newH);
    }
    
    // Render Panel Background with Vertical Gradient & Rounding (24.0f)
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.12f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw a small left indicator stripe
    ImU32 themeColLeft = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeColLeft & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);
    
    // Render Border (softer unhovered, theme-colored on hover, 24.0f rounding)
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (hoverT > 0.001f) {
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.12f, ((themeColLeft >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeColLeft >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.14f, ((themeColLeft >> 16) & 0xFF) / 255.0f, hoverT),
            itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f + 0.5f * hoverT);
    
    // Typography aligned & shifted right to account for stripe
    dl->AddText(ImVec2(renderPos.x + 18.0f, renderPos.y + 14.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), title);
    {
        std::string fitted;
        float const wrap = renderSize.x - 76.0f;
        char const * shown = FitDescription( desc, wrap, renderSize.y - 40.0f, ImGui::GetFontSize() - 1.0f, fitted );
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), shown, nullptr, wrap);
        if ( hovered && shown != desc )
            ImGui::SetTooltip( "%s", desc );
    }
    
    // Colour swatch, positioned relative to renderPos
    ImVec2 renderBtnPos = ImVec2(renderPos.x + renderSize.x - 45.0f, renderPos.y + (renderSize.y - 20.0f) * 0.5f);
    ImGui::SetCursorScreenPos(renderBtnPos);
    bool changed = ImGui::ColorEdit4("##color", rawCol, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview);
    if (changed) {
        col->x = rawCol[0];
        col->y = rawCol[1];
        col->z = rawCol[2];
        col->w = rawCol[3];
    }
    
    ImGui::PopID();
    return changed;
}

bool ButtonItemAbsolute(ImVec2 pos, ImVec2 size, const char* title, const char* desc, float alphaMult = 1.0f) {
    ImGui::PushID(title);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    ImGui::SetCursorScreenPos(pos);
    bool clicked = ImGui::InvisibleButton("##btn", size);
    
    bool hovered = ImGui::IsItemHovered();
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    if (hoverT > 0.001f) {
        ImVec2 mousePos = io.MousePos;
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        
        float maxOffset = 5.0f; 
        ImVec2 delta(mousePos.x - center.x, mousePos.y - center.y);
        float len = sqrtf(delta.x * delta.x + delta.y * delta.y);
        if (len > 1.0f) {
            renderPos.x += (delta.x / len) * maxOffset * hoverT;
            renderPos.y += (delta.y / len) * maxOffset * hoverT;
        }
        
        float scaleFactor = 1.0f + 0.02f * hoverT;
        float newW = size.x * scaleFactor;
        float newH = size.y * scaleFactor;
        renderPos.x -= (newW - size.x) * 0.5f;
        renderPos.y -= (newH - size.y) * 0.5f;
        renderSize = ImVec2(newW, newH);
    }
    
    // Render Panel Background with Vertical Gradient & Rounding (24.0f)
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.12f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw a small left indicator stripe
    ImU32 themeColLeft = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeColLeft & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);
    
    // Render Border (softer unhovered, theme-colored on hover, 24.0f rounding)
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (hoverT > 0.001f) {
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.12f, ((themeColLeft >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeColLeft >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.14f, ((themeColLeft >> 16) & 0xFF) / 255.0f, hoverT),
            itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f + 0.5f * hoverT);
    
    // Typography aligned & shifted right to account for stripe
    dl->AddText(ImVec2(renderPos.x + 18.0f, renderPos.y + 14.0f), ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, itemAlpha)), title);
    {
        std::string fitted;
        float const wrap = renderSize.x - 56.0f;
        char const * shown = FitDescription( desc, wrap, renderSize.y - 40.0f, ImGui::GetFontSize() - 1.0f, fitted );
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), shown, nullptr, wrap);
        if ( hovered && shown != desc )
            ImGui::SetTooltip( "%s", desc );
    }
    
    // Render an action circle on the right
    ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
    ImVec2 actionPos = ImVec2(renderPos.x + renderSize.x - 30.0f, renderPos.y + renderSize.y * 0.5f);
    dl->AddCircle(actionPos, 8.0f, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.35f, itemAlpha)), 12, 1.0f);
    dl->AddCircleFilled(actionPos, 4.0f, ((themeCol & 0x00FFFFFF) | (((unsigned int)(255.0f * (0.3f + 0.7f * hoverT) * itemAlpha)) << 24)), 12);
    
    ImGui::PopID();
    return clicked;
}

//! Writes a config profile.
//!
//! It used to keep only the settings whose names begin with MOD_, which is
//! the mod menu's own half of the client. Everything the player had actually
//! tuned - camera, keys, sound, detail, colours - stayed behind, so loading a
//! profile on another machine gave them a menu that matched and a game that
//! did not. A profile is meant to be the whole client, so it saves whatever
//! the game itself would write to user.cfg.
//!
//! The two halves are written apart, ours first, because somebody opening the
//! file by hand is nearly always looking for one of ours.
static void SaveModSettings(std::ostream &s) {
    tConfItemBase::tConfItemMap const & confmap = tConfItemBase::GetConfItemMap();

    for (int pass = 0; pass < 2; ++pass) {
        bool wrote = false;

        for (tConfItemBase::tConfItemMap::const_iterator iter = confmap.begin(); iter != confmap.end(); ++iter) {
            tConfItemBase* ci = iter->second;
            if (!ci || !ci->CanSave() || !ci->Save())
                continue;

            bool const ours = ci->GetTitle().StartsWith("MOD_");
            if (ours != (pass == 0))
                continue;

            if (!wrote) {
                s << (pass == 0 ? "# client\n" : "\n# game\n");
                wrote = true;
            }

            s << std::setw(28) << ci->GetTitle() << " ";
            ci->WriteVal(s);
            s << '\n';
        }
    }
}

static char g_SelectedConfig[256] = "";
static char g_NewConfigName[128] = "";

bool ConfigItemAbsolute(ImVec2 pos, ImVec2 size, const char* filename, bool& applyClicked, bool& updateClicked, bool& deleteClicked, bool& folderClicked, float alphaMult = 1.0f) {
    ImGui::PushID(filename);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    ImGui::SetCursorScreenPos(pos);
    
    bool hovered = io.MousePos.x >= pos.x && io.MousePos.x <= pos.x + size.x &&
                   io.MousePos.y >= pos.y && io.MousePos.y <= pos.y + size.y &&
                   ImGui::IsWindowHovered();
                   
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    
    // Render Panel Background
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.12f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw left indicator stripe
    ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
    ImU32 stripeCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(ImLerp(0.3f, 1.0f, hoverT) * 255.0f * itemAlpha)) << 24);
    dl->AddRectFilled(ImVec2(renderPos.x + 6.0f, renderPos.y + 14.0f), ImVec2(renderPos.x + 9.0f, renderPos.y + renderSize.y - 14.0f), stripeCol, 2.0f);
    
    // Render Border
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, itemAlpha * 0.4f));
    if (hoverT > 0.001f) {
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.12f, ((themeCol >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeCol >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.12f, ((themeCol >> 16) & 0xFF) / 255.0f, hoverT),
            0.4f * itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f);
    
    // Config Name Text
    tString displayName(filename);
    if (st_StringEndsWith(displayName, ".cfg")) {
        displayName = displayName.SubStr(0, displayName.Len() - 4);
    }
    
    // Title
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + 18.0f, renderPos.y + 12.0f));
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.95f * itemAlpha), "%s", (const char*)displayName);
    
    // Subtitle
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + 18.0f, renderPos.y + 30.0f));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.52f, 0.85f * itemAlpha), "Profile Config file");
    
    // Action Buttons (4 buttons to fit within ~277px width)
    float btnW = 59.0f;
    float btnH = 26.0f;
    float btnY = renderPos.y + 52.0f;
    float spacing = 5.0f;
    float startX = 13.0f;
    
    // Apply Button
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + startX, btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.2f, 0.6f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.25f, 0.8f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.4f, 0.2f, 0.9f * itemAlpha));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button("Apply", ImVec2(btnW, btnH))) {
        applyClicked = true;
    }
    ImGui::PopStyleColor(3);
    
    // Update Button
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + startX + btnW + spacing, btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.25f, 0.45f, 0.6f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.65f, 0.8f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.25f, 0.5f, 0.9f * itemAlpha));
    if (ImGui::Button("Update", ImVec2(btnW, btnH))) {
        updateClicked = true;
    }
    ImGui::PopStyleColor(3);
    
    // Folder Button
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + startX + 2.0f * (btnW + spacing), btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.22f, 0.26f, 0.6f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.32f, 0.38f, 0.8f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.27f, 0.32f, 0.9f * itemAlpha));
    if (ImGui::Button("Folder", ImVec2(btnW, btnH))) {
        folderClicked = true;
    }
    ImGui::PopStyleColor(3);
    
    // Delete Button
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + startX + 3.0f * (btnW + spacing), btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.15f, 0.15f, 0.6f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.2f, 0.2f, 0.8f * itemAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.15f, 0.15f, 0.9f * itemAlpha));
    if (ImGui::Button("Delete", ImVec2(btnW, btnH))) {
        deleteClicked = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    
    ImGui::PopID();
    return true;
}

bool CreateConfigItemAbsolute(ImVec2 pos, ImVec2 size, float alphaMult = 1.0f) {
    ImGui::PushID("create_new_config_card");
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    // Smooth hover animation state tracking
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID hoverTimeId = ImGui::GetID("##hover_time_create");
    float hoverT = storage->GetFloat(hoverTimeId, 0.0f);
    
    ImGui::SetCursorScreenPos(pos);
    
    bool hovered = io.MousePos.x >= pos.x && io.MousePos.x <= pos.x + size.x &&
                   io.MousePos.y >= pos.y && io.MousePos.y <= pos.y + size.y &&
                   ImGui::IsWindowHovered();
                   
    float targetHover = hovered ? 1.0f : 0.0f;
    hoverT = ImLerp(hoverT, targetHover, io.DeltaTime * 12.0f);
    storage->SetFloat(hoverTimeId, hoverT);
    
    float itemAlpha = g_MenuAlpha * alphaMult;
    
    ImVec2 renderPos = pos;
    ImVec2 renderSize = size;
    
    // Render Panel Background
    ImU32 panelBgTop = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, itemAlpha * 0.5f));
    ImU32 panelBgBot = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, itemAlpha * 0.5f));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        ImU32 shadowCol = (themeCol & 0x00FFFFFF) | (((unsigned int)(0.10f * hoverT * 255.0f * itemAlpha)) << 24);
        RenderTextureGlow(ImVec2(renderPos.x - 8.0f, renderPos.y - 8.0f), ImVec2(renderSize.x + 16.0f, renderSize.y + 16.0f), shadowCol, 28.0f);
    }
    
    AddRoundedGradientRect(dl, renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), panelBgTop, panelBgBot, 24.0f);
    
    // Draw dashed border or thin accent border
    ImU32 borderCol = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.15f * itemAlpha));
    if (hoverT > 0.001f) {
        ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
        borderCol = ImGui::GetColorU32(ImVec4(
            ImLerp(0.3f, ((themeCol >> 0) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.3f, ((themeCol >> 8) & 0xFF) / 255.0f, hoverT),
            ImLerp(0.3f, ((themeCol >> 16) & 0xFF) / 255.0f, hoverT),
            0.5f * itemAlpha
        ));
    }
    dl->AddRect(renderPos, ImVec2(renderPos.x + renderSize.x, renderPos.y + renderSize.y), borderCol, 24.0f, 0, 1.0f);
    
    // Large Plus icon in center or left
    ImVec2 center(renderPos.x + renderSize.x * 0.5f, renderPos.y + renderSize.y * 0.45f);
    ImU32 plusCol = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, ImLerp(0.4f, 0.95f, hoverT) * itemAlpha));
    if (hoverT > 0.001f) {
        plusCol = GetThemeColor(pos.x / 1180.0f);
    }
    float plusSize = 12.0f;
    dl->AddLine(ImVec2(center.x - plusSize, center.y), ImVec2(center.x + plusSize, center.y), plusCol, 3.0f);
    dl->AddLine(ImVec2(center.x, center.y - plusSize), ImVec2(center.x, center.y + plusSize), plusCol, 3.0f);
    
    float textW = ImGui::CalcTextSize("Create New Profile").x;
    ImGui::SetCursorScreenPos(ImVec2(renderPos.x + (renderSize.x - textW) * 0.5f, renderPos.y + renderSize.y - 32.0f));
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, ImLerp(0.6f, 0.95f, hoverT) * itemAlpha), "Create New Profile");
    
    ImGui::SetCursorScreenPos(pos);
    bool clicked = ImGui::InvisibleButton("##create_btn", size);
    ImGui::PopID();
    return clicked;
}

static void LoadAvatarTexture() {
    if (g_AvatarTexture != 0) return;
    // PNG rather than JPEG: the decoder for the latter is a separate
    // library that does not always survive packaging
    rSurface surface("textures/avatar.png");
    SDL_Surface* surf = surface.GetSurface();
    if (!surf) return;
    
    glGenTextures(1, &g_AvatarTexture);
    glBindTexture(GL_TEXTURE_2D, g_AvatarTexture);
    
    GLenum format = surface.GetFormat();
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, surf->w, surf->h, 0, format, GL_UNSIGNED_BYTE, surf->pixels);
}

namespace search {

// Fold to lowercase words so punctuation and spacing never decide a match.
static std::string Normalize(const char* in) {
    std::string out;
    if (!in) return out;
    for (const unsigned char* p = (const unsigned char*)in; *p; ++p) {
        if (std::isalnum(*p))
            out += (char)std::tolower(*p);
        else if (!out.empty() && out.back() != ' ')
            out += ' ';
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

static std::vector<std::string> Tokens(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ' ') { if (!cur.empty()) { out.push_back(cur); cur.clear(); } }
        else cur += c;
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// "fov" finds "field of view", "wlh" finds "wall height".
static bool InitialsMatch(const std::string& needle, const std::vector<std::string>& words) {
    if (needle.size() < 2 || needle.size() > words.size()) return false;
    size_t w = 0;
    for (char c : needle) {
        bool hit = false;
        while (w < words.size()) {
            if (words[w++][0] == c) { hit = true; break; }
        }
        if (!hit) return false;
    }
    return true;
}

// "brk" still finds "brake": letters in order inside one word. Spanning the
// whole row instead matched almost anything, so it stays word bound, and an
// abbreviation that drops its first letter is not an abbreviation.
static bool WordSubsequence(const std::string& needle, const std::string& word) {
    if (needle.size() > word.size()) return false;
    if (needle[0] != word[0]) return false;
    size_t i = 0;
    for (char c : word) if (i < needle.size() && c == needle[i]) ++i;
    return i == needle.size();
}

// Typo tolerance. Bails out as soon as the budget is blown.
static bool WithinEdits(const std::string& a, const std::string& b, int budget) {
    int la = (int)a.size(), lb = (int)b.size();
    if (std::abs(la - lb) > budget) return false;

    std::vector<int> prev(lb + 1), cur(lb + 1);
    for (int j = 0; j <= lb; ++j) prev[j] = j;

    for (int i = 1; i <= la; ++i) {
        cur[0] = i;
        int best = cur[0];
        for (int j = 1; j <= lb; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min(std::min(cur[j - 1] + 1, prev[j] + 1), prev[j - 1] + cost);
            best = std::min(best, cur[j]);
        }
        if (best > budget) return false;
        prev.swap(cur);
    }
    return prev[lb] <= budget;
}

// Words people actually type instead of the ones on screen.
static const char* kAliases[][2] = {
    { "fov",        "field of view camera zoom" },
    { "sound",      "audio volume music" },
    { "audio",      "sound volume music" },
    { "music",      "audio sound volume media" },
    { "key",        "bind keybind hotkey shortcut" },
    { "bind",       "key keybind hotkey shortcut" },
    { "keybind",    "key bind hotkey shortcut" },
    { "colour",     "color rgb theme accent" },
    { "color",      "colour rgb theme accent" },
    { "rgb",        "color colour theme accent" },
    { "fps",        "frame rate performance speed" },
    { "lag",        "ping latency network delay" },
    { "ping",       "lag latency network delay" },
    { "net",        "network ping connection server" },
    { "mouse",      "sensitivity cursor look aim" },
    { "sens",       "sensitivity mouse look" },
    { "wall",       "trail line" },
    { "trail",      "wall line" },
    { "cam",        "camera view glance" },
    { "hud",        "overlay widget interface display" },
    { "ui",         "hud interface menu overlay" },
    { "gauge",      "meter bar indicator" },
    { "meter",      "gauge bar indicator" },
    { "bright",     "brightness gamma light" },
    { "res",        "resolution screen display" },
    { "fullscreen", "screen window display resolution" },
    { "chat",       "message text say" },
    { "name",       "nickname player username" },
};

// Query words plus anything those words commonly stand for.
static void Expand(const std::string& token, std::vector<std::string>& into) {
    into.push_back(token);
    for (const auto& row : kAliases) {
        if (token == row[0]) {
            for (const std::string& extra : Tokens(Normalize(row[1])))
                into.push_back(extra);
        }
    }
}

static bool TokenMatches(const std::string& token,
                         const std::string& hay,
                         const std::vector<std::string>& hayWords,
                         const std::string& title,
                         const std::vector<std::string>& titleWords) {
    // what the user literally typed, anywhere in the row
    if (hay.find(token) != std::string::npos) return true;

    // acronyms: "fov" for "field of view"
    if (token.size() >= 2 && InitialsMatch(token, titleWords)) return true;

    // dropped letters: "brk" for "brake"
    if (token.size() >= 3)
        for (const std::string& w : hayWords)
            if (WordSubsequence(token, w)) return true;

    // a real typo. Short words get no slack at all, otherwise "aim" starts
    // answering for "rim" and every three letter query matches the whole page.
    if (token.size() >= 5) {
        int budget = token.size() >= 8 ? 2 : 1;
        for (const std::string& w : hayWords) {
            if (WithinEdits(token, w, budget)) return true;
            if (w.size() > token.size() &&
                WithinEdits(token, w.substr(0, token.size()), budget)) return true;
        }
    }

    // synonyms stand in for the literal word, against the title only; running
    // them through the fuzzy rules as well is what made "cam" match "cache"
    std::vector<std::string> variants;
    Expand(token, variants);
    for (const std::string& v : variants) {
        if (v == token || v.size() < 3) continue;
        if (title.find(v) != std::string::npos) return true;
    }
    return false;
}

} // namespace search

bool ItemMatchesSearch(const char* title, const char* desc, const char* query) {
    if (!query || query[0] == '\0') return true;

    std::string q = search::Normalize(query);
    if (q.empty()) return true;

    std::string hay = search::Normalize(title);
    std::string descNorm = search::Normalize(desc);
    if (!descNorm.empty()) hay += " " + descNorm;

    std::string titleNorm = search::Normalize(title);
    std::vector<std::string> hayWords = search::Tokens(hay);
    std::vector<std::string> titleWords = search::Tokens(titleNorm);

    // Every word the user typed has to be accounted for, so extra words narrow
    // the list down instead of widening it.
    for (const std::string& token : search::Tokens(q)) {
        if (!search::TokenMatches(token, hay, hayWords, titleNorm, titleWords))
            return false;
    }
    return true;
}

// Floating search field. Drawn as its own window pinned just above the panel:
// close enough to read as part of it, detached enough to keep its own shape.
static void DrawFloatingSearch(ImVec2 menuPos, ImVec2 menuSize, char* buf, size_t bufSize) {
    const float barH = 40.0f;
    const float gap  = 10.0f;
    float barW = menuSize.x * 0.34f;
    if (barW < 260.0f) barW = 260.0f;
    if (barW > 520.0f) barW = 520.0f;

    // The window is padded so the glow has room; clipping happens at the window
    // edge, and without the margin the halo was being sliced off.
    const float pad = 18.0f;

    ImVec2 barPos(menuPos.x + (menuSize.x - barW) * 0.5f, menuPos.y - barH - gap);
    if (barPos.y < pad + 4.0f) barPos.y = pad + 4.0f;

    ImGui::SetNextWindowPos(ImVec2(barPos.x - pad, barPos.y - pad));
    ImGui::SetNextWindowSize(ImVec2(barW + pad * 2.0f, barH + pad * 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    // The window is only a canvas for the pill; its own frame would show as a
    // grey rectangle around the glow.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    if (ImGui::Begin("##ModSearchBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings)) {

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 p0 = barPos;
        ImVec2 p1 = ImVec2(barPos.x + barW, barPos.y + barH);
        float  r  = barH * 0.5f;   // fully rounded ends

        static float focusAnim = 0.0f;
        bool active = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) ||
                      ImGui::IsMouseHoveringRect(p0, p1);
        float target = active ? 1.0f : 0.0f;
        float step = io.DeltaTime > 0.0f ? io.DeltaTime : 1.0f / 60.0f;
        focusAnim += (target - focusAnim) * (1.0f - expf(-12.0f * step));

        float t = (float)ImGui::GetTime();
        float alpha = g_MenuAlpha;

        // Colour sweep along the outline; the same hue drives the glow so the
        // whole control reads as one light source.
        auto hue = [&](float offset) {
            float h = fmodf(t * 0.12f + offset, 1.0f);
            float rr, gg, bb;
            ImGui::ColorConvertHSVtoRGB(h, 0.75f, 1.0f, rr, gg, bb);
            return ImVec4(rr, gg, bb, 1.0f);
        };

        ImVec4 cA = hue(0.0f);

        // Outer glow, stronger while focused.
        int glowLayers = 6;
        for (int i = glowLayers; i > 0; --i) {
            float spread = (float)i * (1.7f + focusAnim * 1.8f);
            float a = (0.10f + focusAnim * 0.09f) * (1.0f - (float)i / (glowLayers + 1)) * alpha;
            dl->AddRect(ImVec2(p0.x - spread, p0.y - spread),
                        ImVec2(p1.x + spread, p1.y + spread),
                        ImGui::GetColorU32(ImVec4(cA.x, cA.y, cA.z, a)),
                        r + spread, 0, 2.0f);
        }

        dl->AddRectFilled(p0, p1, ImGui::GetColorU32(ImVec4(0.07f, 0.075f, 0.105f, 0.99f * alpha)), r);
        // Sheen across the upper half. Rounded on top only: a plain rect here
        // painted square corners over the rounded ends.
        dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + r),
                          ImGui::GetColorU32(ImVec4(1, 1, 1, 0.055f * alpha)),
                          r, ImDrawFlags_RoundCornersTop);

        // Outline drawn as one continuous ring so the colour can travel around
        // it. Walking the perimeter by hand keeps the straight runs and the
        // caps in a single ordered list, which arc calls alone cannot give.
        float border = 2.0f + focusAnim * 1.2f;
        float edgeA = (0.95f + focusAnim * 0.05f) * alpha;

        const int kArcSteps  = 26;
        const int kEdgeSteps = 26;
        float straight = barW - 2.0f * r;
        if (straight < 0.0f) straight = 0.0f;

        ImVec2 ring[(kArcSteps + kEdgeSteps) * 2 + 4];
        int n = 0;
        ImVec2 lc(p0.x + r, p0.y + r);
        ImVec2 rc(p1.x - r, p0.y + r);

        // top edge, left to right
        for (int i = 0; i <= kEdgeSteps; ++i)
            ring[n++] = ImVec2(lc.x + straight * ((float)i / kEdgeSteps), p0.y);
        // right cap, top to bottom
        for (int i = 1; i <= kArcSteps; ++i) {
            float a = -1.5707963f + 3.1415926f * ((float)i / kArcSteps);
            ring[n++] = ImVec2(rc.x + cosf(a) * r, rc.y + sinf(a) * r);
        }
        // bottom edge, right to left
        for (int i = 1; i <= kEdgeSteps; ++i)
            ring[n++] = ImVec2(rc.x - straight * ((float)i / kEdgeSteps), p1.y);
        // left cap, bottom to top
        for (int i = 1; i <= kArcSteps; ++i) {
            float a = 1.5707963f + 3.1415926f * ((float)i / kArcSteps);
            ring[n++] = ImVec2(lc.x + cosf(a) * r, lc.y + sinf(a) * r);
        }

        for (int i = 0; i < n - 1; ++i) {
            float f = (float)i / (float)(n - 1);
            ImVec4 c = hue(f * 0.85f);
            dl->AddLine(ring[i], ring[i + 1],
                        ImGui::GetColorU32(ImVec4(c.x, c.y, c.z, edgeA)), border);
        }

        // Magnifier.
        ImVec2 gc(p0.x + r + 2.0f, p0.y + barH * 0.5f);
        ImU32 iconCol = ImGui::GetColorU32(ImVec4(0.75f, 0.78f, 0.88f, (0.6f + focusAnim * 0.4f) * alpha));
        dl->AddCircle(ImVec2(gc.x - 1.0f, gc.y - 1.0f), 5.2f, iconCol, 16, 1.7f);
        dl->AddLine(ImVec2(gc.x + 2.6f, gc.y + 2.6f), ImVec2(gc.x + 6.4f, gc.y + 6.4f), iconCol, 1.9f);

        float textLeft = gc.x + 14.0f;
        float clearW   = 26.0f;

        ImGui::SetCursorScreenPos(ImVec2(textLeft, p0.y + (barH - ImGui::GetTextLineHeight()) * 0.5f - 3.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.97f, 1.0f, alpha));
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.62f, 0.66f, 0.78f, 0.95f * alpha));
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(cA.x, cA.y, cA.z, 0.35f * alpha));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 4));
        ImGui::SetNextItemWidth(barW - (textLeft - p0.x) - clearW - r);
        ImGui::InputTextWithHint("##modsearch", "Search settings...", buf, bufSize);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        // Clear button, only while there is something to clear.
        if (buf[0] != '\0') {
            ImVec2 xc(p1.x - r - 2.0f, p0.y + barH * 0.5f);
            bool overX = ImGui::IsMouseHoveringRect(ImVec2(xc.x - 9, xc.y - 9), ImVec2(xc.x + 9, xc.y + 9));
            ImU32 xcol = ImGui::GetColorU32(ImVec4(1.0f, 0.45f, 0.5f, (overX ? 1.0f : 0.55f) * alpha));
            dl->AddLine(ImVec2(xc.x - 4, xc.y - 4), ImVec2(xc.x + 4, xc.y + 4), xcol, 1.8f);
            dl->AddLine(ImVec2(xc.x + 4, xc.y - 4), ImVec2(xc.x - 4, xc.y + 4), xcol, 1.8f);
            if (overX && ImGui::IsMouseClicked(0)) buf[0] = '\0';
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

// Function to draw text parsing game color codes (0xRRGGBB and 0xRESETT) in ImGui
static inline bool IsHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

//! The same text without its colour codes, for measuring.
static std::string Uncoloured(std::string const & in)
{
    std::string out;
    for (size_t i = 0; i < in.size(); )
    {
        if (in[i] == '0' && i + 1 < in.size() && in[i+1] == 'x' && i + 8 <= in.size()) { i += 8; continue; }
        out += in[i]; ++i;
    }
    return out;
}

void RenderColoredText(ImDrawList* dl, ImVec2 pos, ImU32 defaultColor, const char* text) {
    std::string convertedText = ConvertNonUtf8ToUtf8(text);
    ImVec2 currentPos = pos;
    ImU32 currentColor = defaultColor;
    
    const char* p = convertedText.c_str();
    std::string currentSegment = "";
    
    while (*p != '\0') {
        if (*p == '0' && *(p+1) == 'x') {
            if (!currentSegment.empty()) {
                dl->AddText(currentPos, currentColor, currentSegment.c_str());
                currentPos.x += ImGui::CalcTextSize(currentSegment.c_str()).x;
                currentSegment = "";
            }
            
            if (strncmp(p, "0xRESETT", 8) == 0) {
                currentColor = defaultColor;
                p += 8;
                continue;
            }
            
            bool isHexColor = true;
            for (int i = 2; i < 8; i++) {
                char c = *(p + i);
                if (c == '\0' || !IsHexChar(c)) {
                    isHexColor = false;
                    break;
                }
            }
            
            if (isHexColor) {
                unsigned int r = 0, g = 0, b = 0;
                char hex[3] = {0};
                
                hex[0] = *(p + 2); hex[1] = *(p + 3);
                sscanf(hex, "%x", &r);
                
                hex[0] = *(p + 4); hex[1] = *(p + 5);
                sscanf(hex, "%x", &g);
                
                hex[0] = *(p + 6); hex[1] = *(p + 7);
                sscanf(hex, "%x", &b);
                
                currentColor = IM_COL32(r, g, b, 255);
                p += 8;
                continue;
            }
        }
        
        currentSegment += *p;
        p++;
    }
    
    if (!currentSegment.empty()) {
        dl->AddText(currentPos, currentColor, currentSegment.c_str());
    }
}

inline int GetUtf8CharLen(const char* p) {
    unsigned char c = (unsigned char)*p;
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

std::string TruncateColoredString(const char* val, int maxLen) {
    if (!val) return "";
    std::string converted = ConvertNonUtf8ToUtf8(val);
    std::string truncatedColorStr = "";
    int strippedLen = 0;
    const char* p = converted.c_str();
    while (*p != '\0') {
        if (*p == '0' && *(p+1) == 'x') {
            if (strncmp(p, "0xRESETT", 8) == 0) {
                truncatedColorStr += "0xRESETT";
                p += 8;
                continue;
            }
            bool isHex = true;
            for (int i = 2; i < 8; i++) {
                if (p[i] == '\0' || !IsHexChar(p[i])) { isHex = false; break; }
            }
            if (isHex) {
                truncatedColorStr.append(p, 8);
                p += 8;
                continue;
            }
        }
        int charLen = GetUtf8CharLen(p);
        if (strippedLen < maxLen) {
            truncatedColorStr.append(p, charLen);
            strippedLen++;
            p += charLen;
        } else {
            // Check if there are actually more printable characters left
            const char* next_p = p;
            bool hasMorePrintable = false;
            while (*next_p != '\0') {
                if (*next_p == '0' && *(next_p+1) == 'x') {
                    if (strncmp(next_p, "0xRESETT", 8) == 0) {
                        next_p += 8;
                        continue;
                    }
                    bool isHex = true;
                    for (int i = 2; i < 8; i++) {
                        if (next_p[i] == '\0' || !IsHexChar(next_p[i])) { isHex = false; break; }
                    }
                    if (isHex) {
                        next_p += 8;
                        continue;
                    }
                }
                hasMorePrintable = true;
                break;
            }
            if (hasMorePrintable) {
                truncatedColorStr += "...";
            }
            break;
        }
    }
    return truncatedColorStr;
}

namespace {

struct BindGroup {
    const char * title;
    uActionScope scope;
    const char * prefix;  // NULL takes everything the prefixed groups reject
    bool         openByDefault;
};

// the engine already orders each list from the everyday keys down to the
// obscure ones; the two bulk families get split off so they cannot bury the
// handful of keys people actually rebind
const BindGroup s_BindGroups[] = {
    { "CYCLE CONTROLS",     uACTION_PLAYER, NULL,           true  },
    { "CAMERA & VIEW",      uACTION_CAMERA, NULL,           true  },
    { "GENERAL",            uACTION_GLOBAL, NULL,           true  },
    { "INSTANT CHAT",       uACTION_PLAYER, "INSTANT_CHAT", false },
    { "SPECTATOR & NOCLIP", uACTION_CAMERA, "NOCLIP",       false },
};

bool InGroup(uAction* act, const BindGroup& group) {
    const char* name = static_cast<const char*>(act->internalName);
    if (group.prefix)
        return strncmp(name, group.prefix, strlen(group.prefix)) == 0;

    for (size_t i = 0; i < sizeof(s_BindGroups) / sizeof(s_BindGroups[0]); ++i) {
        const BindGroup& other = s_BindGroups[i];
        if (other.prefix && other.scope == group.scope &&
            strncmp(name, other.prefix, strlen(other.prefix)) == 0)
            return false;
    }
    return true;
}

// a description that never made it into the language file comes back as its
// own lookup key, which is not something to show a player
bool LooksLikeRawKey(const char* text) {
    if (!text || !*text) return true;
    bool underscore = false;
    for (const char* c = text; *c; ++c) {
        if (*c == ' ') return false;
        if (*c == '_') underscore = true;
    }
    return underscore;
}

std::string BoundKeysOf(uAction* act, int slot) {
    std::string res;
    for (int sym = 0; sym < SDLK_NEWLAST; ++sym) {
        if (keymap[sym] && keymap[sym]->act == act && keymap[sym]->CheckPlayer(slot)) {
            if (!res.empty()) res += " / ";
            res += su_KeyName(sym);
        }
    }
    return res.empty() ? "NOT BOUND" : res;
}

}

namespace {

char s_SettingsFilter[128] = "";
int  s_SettingsMatches = 0;      // counted as rows are drawn
int  s_SettingsMatchesShown = 0; // what the previous frame ended up with

bool SettingsFiltering() { return s_SettingsFilter[0] != '\0'; }

bool SettingsShow(const char* label, const char* help = "") {
    if (!SettingsFiltering()) return true;
    if (!ItemMatchesSearch(label, help, s_SettingsFilter)) return false;
    ++s_SettingsMatches;
    return true;
}

// with a filter up the headers step aside and each row answers for itself,
// so nothing stays trapped inside a section that happens to be folded
bool SettingsSectionOpen(const char* title) {
    if (SettingsFiltering()) return true;
    return ImGui::CollapsingHeader(title);
}

} // namespace

// Rows come in two shapes here: a widget carrying its own caption, and a
// caption drawn separately above a widget whose id is hidden behind "##".
// These keep both shapes filterable without restating every label.
namespace flt {

static std::string s_caption;

static bool Visible(const char* label) {
    if (label && label[0] == '#') return SettingsShow(s_caption.c_str());
    s_caption = label ? label : "";
    return SettingsShow(label);
}

static bool Label(const char* text) {
    s_caption = text ? text : "";
    if (!SettingsShow(text)) return false;
    ImGui::TextUnformatted(text);
    return true;
}

static bool Checkbox(const char* label, bool* v) {
    if (!Visible(label)) return false;
    if (!ImGui::Checkbox(label, v)) return false;
    se_PlayUiClick();
    return true;
}

static bool SliderInt(const char* label, int* v, int lo, int hi, const char* fmt = "%d") {
    return Visible(label) && ImGui::SliderInt(label, v, lo, hi, fmt);
}

static bool SliderFloat(const char* label, float* v, float lo, float hi, const char* fmt = "%.3f") {
    return Visible(label) && ImGui::SliderFloat(label, v, lo, hi, fmt);
}

static bool Combo(const char* label, int* v, const char* const items[], int count) {
    return Visible(label) && ImGui::Combo(label, v, items, count);
}

static bool InputText(const char* label, char* buf, size_t size) {
    return Visible(label) && ImGui::InputText(label, buf, size);
}

static bool ColorEdit3(const char* label, float col[3]) {
    return Visible(label) && ImGui::ColorEdit3(label, col);
}

static bool Button(const char* label, const ImVec2& size = ImVec2(0, 0)) {
    return Visible(label) && ImGui::Button(label, size);
}

}

namespace {

// while a filter is up the headers step aside and every row speaks for itself,
// which keeps empty sections from piling up on screen
void SettingsSection(const char* title, bool openByDefault, void (*body)(float), float width) {
    if (SettingsFiltering()) {
        body(width);
        return;
    }
    ImGui::SetNextItemOpen(openByDefault, ImGuiCond_FirstUseEver);
    if (ImGui::CollapsingHeader(title)) {
        ImGui::Spacing();
        body(width);
        ImGui::Spacing();
    }
}

// resolutions the driver reports, newest query cached until the menu is reopened
const std::vector<rScreenSize>& ScreenModes() {
    static std::vector<rScreenSize> modes;

    // Built once and kept forever, from whatever the driver happened to report
    // at that moment. Ask again when the screen has changed size, or a display
    // the game did not know about at startup never appears in the list.
    int deskW = 0, deskH = 0;
    if ( SDL_DisplayMode const * desk = SDL_GetDesktopDisplayMode( SDL_GetPrimaryDisplay() ) )
    {
        deskW = desk->w;
        deskH = desk->h;
    }

    static int builtFor = -1;
    int now = deskW * 100000 + deskH;
    if ( !modes.empty() && builtFor == now )
        return modes;

    modes.clear();
    builtFor = now;

    modes.push_back(rScreenSize(ArmageTron_Custom));
    if (sr_DesktopScreensizeSupported()) modes.push_back(rScreenSize(ArmageTron_Desktop));

    // The size the desktop is actually running at belongs in the list whether
    // or not the driver lists it as a fullscreen mode - it is the one people
    // look for first.
    if ( deskW > 0 && deskH > 0 )
        modes.push_back( rScreenSize( deskW, deskH ) );

    int count = 0;
    SDL_DisplayMode** found = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &count);
    if (found && count > 0) {
        for (int i = 0; i < count; ++i) {
            rScreenSize size(found[i]->w, found[i]->h);
            bool seen = false;
            for (size_t j = 0; j < modes.size(); ++j)
                if (modes[j] == size) { seen = true; break; }
            if (!seen) modes.push_back(size);
        }
        SDL_free((void*)found);
    } else {
        for (int r = ArmageTron_Min; r < ArmageTron_Custom; ++r)
            modes.push_back(rScreenSize(rResolution(r)));
    }
    return modes;
}

std::string ScreenModeLabel(const rScreenSize& size) {
    if (size.res == ArmageTron_Desktop) return "Desktop";
    if (size.res == ArmageTron_Custom)  return "Custom";
    char buf[32];
    snprintf(buf, sizeof(buf), "%d x %d", size.width, size.height);
    return buf;
}

void ScreenModeCombo(const char* label, rScreenSize& target) {
    const std::vector<rScreenSize>& modes = ScreenModes();
    std::string current = ScreenModeLabel(target);
    if (!ImGui::BeginCombo(label, current.c_str())) return;
    for (size_t i = 0; i < modes.size(); ++i) {
        std::string name = ScreenModeLabel(modes[i]);
        if (ImGui::Selectable(name.c_str(), modes[i] == target)) target = modes[i];
    }
    ImGui::EndCombo();
}

template<class T>
void EnumCombo(const char* label, T& target, const char* const* names, const T* values, int count) {
    int current = 0;
    for (int i = 0; i < count; ++i) if (values[i] == target) current = i;
    if (!ImGui::BeginCombo(label, names[current])) return;
    for (int i = 0; i < count; ++i)
        if (ImGui::Selectable(names[i], i == current)) target = values[i];
    ImGui::EndCombo();
}

}

extern bool sg_simpleTrail;
extern REAL sg_wallTextureStretch;
extern bool sg_cycleBloom;
extern REAL sg_cycleBloomPower;
extern REAL sg_trailBloomPower;

// A widget draws its name to the right of itself, so handing the control the
// whole row pushes that name past the edge and leaves the player looking at a
// single letter. Take the name's width off the control instead.
static void SettingItemWidth(const char* label, float w) {
    const float labelW = ImGui::CalcTextSize(label, NULL, true).x;
    const float room = w - labelW - ImGui::GetStyle().ItemInnerSpacing.x;
    const float floorW = 90.0f;
    ImGui::SetNextItemWidth(room > floorW ? room : floorW);
}

// Each texture group carries its own filtering. The old detail sheet exposed
// all four and this menu exposed none of them, so anyone wanting sharper walls
// or an unfiltered floor had nowhere to say so.
static void TextureModeCombo(const char* label, const char* id, int& mode, bool allowOff, float itemWidth) {
    static const char* const allNames[]  = { "Off", "Nearest", "Bilinear",
                                             "Mipmap Nearest", "Mipmap Bilinear", "Mipmap Trilinear" };
    static const int         allValues[] = { -1, GL_NEAREST, GL_LINEAR,
                                             GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR };

    // the font has to stay drawn and is never mipmapped, so it gets the two
    // plain filters and nothing else
    const int first = allowOff ? 0 : 1;
    const int count = allowOff ? 6 : 2;

    int current = 0;
    for (int i = 0; i < count; ++i)
        if (allValues[first + i] == mode) current = i;

    // a width set for a row the search has filtered away would be picked up by
    // whichever row is drawn next instead
    if (!flt::Label(label)) return;
    if (itemWidth > 0.0f)
        ImGui::SetNextItemWidth(itemWidth);
    if (flt::Combo(id, &current, allNames + first, count))
        mode = allValues[first + current];
}

static void DisplayBody(float w) {
    if (SettingsShow("Fullscreen Resolution", "screen size in fullscreen")) {
        SettingItemWidth("Fullscreen Resolution", w);
        ScreenModeCombo("Fullscreen Resolution", currentScreensetting.res);
    }
    if (SettingsShow("Window Size", "size of the game window")) {
        SettingItemWidth("Window Size", w);
        ScreenModeCombo("Window Size", currentScreensetting.windowSize);
    }
    if (SettingsShow("Fullscreen", "borderless windowed or fullscreen"))
        ImGui::Checkbox("Fullscreen", &currentScreensetting.fullscreen);

    static const char* depthNames[] = { "16 bit", "Desktop", "32 bit" };
    static const rColorDepth depthValues[] = { ArmageTron_ColorDepth_16,
                                               ArmageTron_ColorDepth_Desktop,
                                               ArmageTron_ColorDepth_32 };
    if (SettingsShow("Color Depth", "bits per pixel")) {
        SettingItemWidth("Color Depth", w);
        EnumCombo("Color Depth", currentScreensetting.colorDepth, depthNames, depthValues, 3);
    }
    if (SettingsShow("Z Buffer Depth", "depth buffer precision")) {
        SettingItemWidth("Z Buffer Depth", w);
        EnumCombo("Z Buffer Depth", currentScreensetting.zDepth, depthNames, depthValues, 3);
    }

    static const char* syncNames[] = { "On", "Driver Default", "Off", "Motion Blur" };
    static const rVSync syncValues[] = { ArmageTron_VSync_On, ArmageTron_VSync_Default,
                                         ArmageTron_VSync_Off, ArmageTron_VSync_MotionBlur };
    if (SettingsShow("Vertical Sync", "vsync tearing")) {
        SettingItemWidth("Vertical Sync", w);
        EnumCombo("Vertical Sync", currentScreensetting.vSync, syncNames, syncValues, 4);
    }
    if (SettingsShow("Frame Rate Limit", "max fps cap")) {
        SettingItemWidth("Frame Rate Limit", w);
        ImGui::SliderInt("Frame Rate Limit", &sr_maxFPS, 0, 1200, sr_maxFPS > 0 ? "%d fps" : "unlimited");
    }
    if (SettingsShow("Grab Mouse", "confine the cursor to the window"))
        ImGui::Checkbox("Grab Mouse", &su_mouseGrab);
    if (SettingsShow("Keep Rendering When Unfocused", "keep drawing in the background"))
        ImGui::Checkbox("Keep Rendering When Unfocused", &sr_keepWindowActive);
    if (SettingsShow("Report Driver Errors", "check for opengl errors"))
        ImGui::Checkbox("Report Driver Errors", &currentScreensetting.checkErrors);
    if (SettingsShow("High Pixel Density", "draw at the display's real pixel count")) {
        extern bool sr_highDPI;
        ImGui::Checkbox("High Pixel Density", &sr_highDPI);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Sharper on displays that pack more pixels than they report,\n"
                              "at up to four times the work. Needs applying below.");
    }

    ImGui::Spacing();
    if (ImGui::Button("APPLY DISPLAY CHANGES", ImVec2(w, 30.0f))) {
        sr_ReinitDisplay();
        st_SaveConfig();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Resolution, depth and sync changes only take hold once applied.");
}

static void PerformanceBody(float w) {
    static const char* listNames[] = { "Off", "Compile and Call", "Compile and Execute" };
    static const rDisplayListUsage listValues[] = { rDisplayList_Off, rDisplayList_CAC, rDisplayList_CAE };
    if (SettingsShow("Display Lists", "cache geometry on the gpu")) {
        SettingItemWidth("Display Lists", w);
        EnumCombo("Display Lists", sr_useDisplayLists, listNames, listValues, 3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Caches geometry on the GPU. Turn off if the world renders incorrectly.");
    }

    static const char* swapNames[] = { "Fastest", "glFlush", "glFinish" };
    static const rSysDep::rSwapMode swapValues[] = { rSysDep::rSwap_Fastest,
                                                     rSysDep::rSwap_glFlush,
                                                     rSysDep::rSwap_glFinish };
    if (SettingsShow("Buffer Swap Mode", "frame pacing glfinish")) {
        SettingItemWidth("Buffer Swap Mode", w);
        EnumCombo("Buffer Swap Mode", rSysDep::swapMode_, swapNames, swapValues, 3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("glFinish trades a little throughput for steadier frame pacing.");
    }

    if (SettingsShow("Simple Trails", "cheaper wall rendering"))
        ImGui::Checkbox("Simple Trails", &sg_simpleTrail);
    if (SettingsShow("Cycle Headlight", "light cast in front of the cycle"))
        ImGui::Checkbox("Cycle Headlight", &headlights);
}

static void SplitScreenBody(float w) {
    int& layout = rViewportConfiguration::next_conf_num;
    const int layouts = rViewportConfiguration::s_viewportNumConfigurations;
    if (layout < 0) layout = 0;
    if (layout >= layouts) layout = layouts - 1;

    if (SettingsShow("Layout", "split screen viewport layout")) {
        SettingItemWidth("Layout", w);
        if (ImGui::BeginCombo("Layout", rViewportConfiguration::s_viewportConfigurationNames[layout])) {
            for (int i = 0; i < layouts; ++i)
                if (ImGui::Selectable(rViewportConfiguration::s_viewportConfigurationNames[i], i == layout))
                    layout = i;
            ImGui::EndCombo();
        }
    }

    for (int vp = 0; vp < MAX_VIEWPORTS; ++vp) {
        char label[32];
        snprintf(label, sizeof(label), "Viewport %d", vp + 1);
        if (!SettingsShow(label, "which player owns this split screen view")) continue;

        int& owner = s_newViewportBelongsToPlayer[vp];
        if (owner < 0 || owner >= MAX_PLAYERS) owner = 0;

        ePlayer* holder = ePlayer::PlayerConfig(owner);
        char shown[96];
        snprintf(shown, sizeof(shown), "Player %d  (%s)", owner + 1,
                 holder ? static_cast<const char*>(holder->Name()) : "");

        SettingItemWidth(label, w);
        if (ImGui::BeginCombo(label, shown)) {
            for (int pl = 0; pl < MAX_PLAYERS; ++pl) {
                ePlayer* candidate = ePlayer::PlayerConfig(pl);
                char option[96];
                snprintf(option, sizeof(option), "Player %d  (%s)", pl + 1,
                         candidate ? static_cast<const char*>(candidate->Name()) : "");
                if (ImGui::Selectable(option, pl == owner)) {
                    rViewport::SetDirectionOfCorrection(vp, pl > owner ? 1 : -1);
                    owner = pl;
                    rViewport::CorrectViewport(vp, MAX_PLAYERS);
                }
            }
            ImGui::EndCombo();
        }
    }
}

static void SoundBody(float w) {
    const int wasQuality = sound_quality;
    const int wasShift = buffer_shift;

    static const char* qualityNames[] = { "Off", "Low", "Medium", "High" };
    static const int qualityValues[] = { se_SOUND_OFF, se_SOUND_LOW, se_SOUND_MED, se_SOUND_HIGH };
    if (SettingsShow("Sound Quality", "audio sample rate volume off")) {
        SettingItemWidth("Sound Quality", w);
        EnumCombo("Sound Quality", sound_quality, qualityNames, qualityValues, 4);
    }

    static const char* bufferNames[] = { "Very Small", "Small", "Medium", "Large", "Very Large" };
    static const int bufferValues[] = { -2, -1, 0, 1, 2 };
    if (SettingsShow("Buffer Size", "audio latency crackle")) {
        SettingItemWidth("Buffer Size", w);
        EnumCombo("Buffer Size", buffer_shift, bufferNames, bufferValues, 5);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Smaller buffers cut audio latency but can crackle on slower machines.");
    }

    if (SettingsShow("Simultaneous Sounds", "how many sounds mix at once")) {
        SettingItemWidth("Simultaneous Sounds", w);
        ImGui::SliderInt("Simultaneous Sounds", &sound_sources, 2, 20);
    }

    if (sound_quality != wasQuality || buffer_shift != wasShift) {
        se_SoundExit();
        se_SoundInit();
        st_SaveConfig();
    }
}

static void DrawSettingsSearch(float width) {
    const float w = width - 55.0f;
    const bool had = SettingsFiltering();

    // the tally can only be read once the rows have all had their say, so it
    // trails a frame behind; nobody types fast enough to notice
    s_SettingsMatchesShown = s_SettingsMatches;
    s_SettingsMatches = 0;

    ImGui::SetNextItemWidth(had ? w - 34.0f : w);
    ImGui::InputTextWithHint("##settings_search", "Search settings...",
                             s_SettingsFilter, sizeof(s_SettingsFilter));
    if (had) {
        ImGui::SameLine(0.0f, 6.0f);
        if (ImGui::Button("X##settings_search_clear", ImVec2(28.0f, 0.0f)))
            s_SettingsFilter[0] = '\0';

        if (s_SettingsMatchesShown > 0)
            ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.6f, 1.0f), "%d matching", s_SettingsMatchesShown);
        else
            ImGui::TextColored(ImVec4(0.85f, 0.55f, 0.35f, 1.0f), "nothing matches that");
    }
    ImGui::Spacing();
}

static void DrawDisplaySettings(float width) {
    const float w = width - 55.0f;
    ImGui::Spacing();
    SettingsSection("DISPLAY & VIDEO",    true,  &DisplayBody,     w);
    SettingsSection("PERFORMANCE TWEAKS", false, &PerformanceBody, w);
    SettingsSection("SPLIT SCREEN",       false, &SplitScreenBody, w);
}

static void DrawSoundSettings(float width) {
    ImGui::Spacing();
    SettingsSection("AUDIO", true, &SoundBody, width - 55.0f);
}

static void DrawKeybindList(float width, int localPlayer) {
    const float rowW = width - 55.0f;

    for (size_t g = 0; g < sizeof(s_BindGroups) / sizeof(s_BindGroups[0]); ++g) {
        const BindGroup& group = s_BindGroups[g];
        uAction* head = su_ActionList(group.scope);
        if (!head) continue;

        const int slot = su_BindSlot(group.scope, localPlayer);

        if (!SettingsFiltering()) {
            ImGui::Spacing();
            ImGui::SetNextItemOpen(group.openByDefault, ImGuiCond_FirstUseEver);
            if (!ImGui::CollapsingHeader(group.title)) continue;
            ImGui::Spacing();
        }

        for (uAction* act = head; act; act = act->Next()) {
            if (!InGroup(act, group)) continue;

            tString label;
            label << act->description;
            if (LooksLikeRawKey(static_cast<const char*>(label))) label = act->internalName;

            if (!SettingsShow(static_cast<const char*>(label),
                              static_cast<const char*>(act->internalName))) continue;

            ImGui::TextUnformatted(static_cast<const char*>(label));

            const bool waiting = (s_BindingAction == act && s_BindingSlot == slot);
            std::string caption = waiting ? std::string("PRESS A KEY  (ESC CLEARS)")
                                          : BoundKeysOf(act, slot);
            caption += "##bind_";
            caption += static_cast<const char*>(act->internalName);

            if (ImGui::Button(caption.c_str(), ImVec2(rowW, 28.0f))) {
                s_BindingAction = act;
                s_BindingSlot = slot;
            }
            if (ImGui::IsItemHovered()) {
                tString help;
                help << act->helpText;
                if (help.Len() > 1) ImGui::SetTooltip("%s", static_cast<const char*>(help));
            }
            ImGui::Spacing();
        }
    }
}

// the config browser only ever lists the var directory, so the configuration
// the game ships with is placed there once as an ordinary preset. Recording
// that it happened means a player who deletes it is not handed it back on the
// next launch, and an installation that predates it still receives it.
static bool sg_presetPlaced = false;
static tConfItem<bool> sg_presetPlacedConf("MOD_PRESET_PLACED", sg_presetPlaced);

static void PlaceShippedPreset() {
    if (sg_presetPlaced) return;
    sg_presetPlaced = true;

    tString source = tDirectories::Config().GetReadPath("retrocycles.cfg");
    if (source.Len() <= 1) return;

    std::ifstream in((const char*)source, std::ios::binary);
    if (!in.is_open()) return;

    tString target = tDirectories::Var().GetWritePath("ilonium.cfg");
    std::ofstream out((const char*)target, std::ios::binary);
    if (!out.is_open()) return;

    out << in.rdbuf();
    out.close();
    st_SaveConfig();
}


void ModMenu::Init() {
    PlaceShippedPreset();

    if (g_Initialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Type carries more of how a thing feels than any other single choice, and
    // what was here was whatever the system happened to have - the same face a
    // file manager uses. Two are loaded now and each has a job. Rajdhani is
    // narrow and squared off and belongs on a grid, so it takes the headings
    // and the numbers that are meant to be read across a room. Barlow says
    // nothing about itself and reads cleanly at small sizes, so it takes
    // everything a player actually has to read.
    //
    // Neither covers Cyrillic. Rather than give that up, the face that does is
    // merged in behind them, so a Russian name falls through to it instead of
    // arriving as empty boxes.
    struct FaceWanted { char const * file; float size; ImFont ** into; };

    tString cyrillic = tDirectories::Data().GetReadPath("textures/DejaVuSans.ttf");
    bool haveCyrillic = false;
    if (FILE* probe = fopen((const char*)cyrillic, "rb")) { fclose(probe); haveCyrillic = true; }

    tString bodyFace    = tDirectories::Data().GetReadPath("textures/Barlow-Medium.ttf");
    tString displayFace = tDirectories::Data().GetReadPath("textures/Rajdhani-SemiBold.ttf");
    tString monoFace    = tDirectories::Data().GetReadPath("textures/ShareTechMono-Regular.ttf");

    // The face for names over the cycles. Wide, open and squared off, so a
    // name stays legible at a glance while the arena moves under it. Baked at
    // several sizes below rather than scaled: stretching one rastered size is
    // what made the old labels look ragged.
    tString nameFace    = tDirectories::Data().GetReadPath("textures/expresswayfree.ttf");
    bool haveNameFace = false;
    if (FILE* probe = fopen((const char*)nameFace, "rb")) { fclose(probe); haveNameFace = true; }

    bool haveFaces = false;
    if (FILE* probe = fopen((const char*)bodyFace, "rb")) { fclose(probe); haveFaces = true; }
    if (haveFaces) {
        if (FILE* probe = fopen((const char*)displayFace, "rb")) fclose(probe); else haveFaces = false;
    }

    // A face that is not there is not worth crashing over; the old path stays
    // as the answer for a machine missing the files.
    if (!haveFaces && haveCyrillic) {
        bodyFace = cyrillic;
        displayFace = cyrillic;
        monoFace = cyrillic;
        haveFaces = true;
    }

    if (haveFaces) {
        ImFontConfig fallback;
        fallback.MergeMode = true;
        fallback.PixelSnapH = true;

        // Adds one face and, behind it, whatever it cannot spell.
        struct Loader {
            ImGuiIO & io;
            tString const & cyrillic;
            bool haveCyrillic;
            ImFontConfig & fallback;

            ImFont * Load(char const * file, float size) {
                ImFont * face = io.Fonts->AddFontFromFileTTF(file, size, nullptr,
                                    io.Fonts->GetGlyphRangesDefault());
                if (face && haveCyrillic)
                    io.Fonts->AddFontFromFileTTF((char const *)cyrillic, size, &fallback,
                                    io.Fonts->GetGlyphRangesCyrillic());
                return face;
            }
        } loader = { io, cyrillic, haveCyrillic, fallback };

        g_FontDefault = loader.Load((const char*)bodyFace, 15.0f);
        g_FontHeader  = loader.Load((const char*)displayFace, 24.0f);
        g_FontDisplay = loader.Load((const char*)displayFace, 34.0f);
        g_FontMono    = loader.Load((const char*)monoFace, 15.0f);

        for (int i = 0; i < MOD_FONT_SIZE_COUNT; i++)
            g_FontSizes[i] = loader.Load((const char*)bodyFace, g_FontSizeValues[i]);

        for (int i = 0; i < MOD_FONT_SIZE_COUNT; i++)
            g_FontNames[i] = haveNameFace
                           ? loader.Load((const char*)nameFace, g_FontSizeValues[i])
                           : g_FontSizes[i];
    } else {
        g_FontDefault = io.Fonts->AddFontDefault();
        g_FontHeader  = g_FontDefault;
        g_FontDisplay = g_FontDefault;
        g_FontMono    = g_FontDefault;
        for (int i = 0; i < MOD_FONT_SIZE_COUNT; i++)
            g_FontSizes[i] = g_FontDefault;
    }

    ImGui_ImplSDL3_InitForOpenGL(sr_screen, SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL2_Init();
    InitStyle();
    HudManager::Init();

    ::st_PostLoadConfigCallback = &ReapplyCameraConfig;

    if (!g_ProfilesLoaded) LoadProfiles();
    if (!g_ColorsLoaded) LoadColors();
    BackupDefaultCameraSettings();
    if (sg_activeCameraConfig != "") {
        ApplyCameraConfigNoSave((const char*)sg_activeCameraConfig);
    }
    // Every launch, whichever pack it is. Skipping the default one meant a
    // change to the default pack never reached the game - the files sat in
    // custom_packs and the arena went on using whatever happened to be in
    // textures from the last pack that was applied.
    ApplyTexturePackNoSave( sg_activeTexturePack == "" ? std::string( "Default" )
                                                       : std::string( (char const *)sg_activeTexturePack ) );
    TranslateAllModKeybinds();

    // Initialize trackers with active engine values
    ePlayer* lp_cam = ePlayer::PlayerConfig(0);
    g_ShowHUD = subby_ShowHUD;
    prev_ShowHUD = g_ShowHUD;

    g_ShowFastest = subby_ShowSpeedFastest;
    prev_ShowFastest = g_ShowFastest;

    g_ShowScores = subby_ShowScore;
    prev_ShowScores = g_ShowScores;

    g_AliveCounter = subby_ShowAlivePeople;
    prev_AliveCounter = g_AliveCounter;

    g_ShowPing = subby_ShowPing;
    prev_ShowPing = g_ShowPing;

    g_SpeedMeter = subby_ShowSpeedMeter;
    prev_SpeedMeter = g_SpeedMeter;

    g_BrakeMeter = subby_ShowBrakeMeter;
    prev_BrakeMeter = g_BrakeMeter;

    g_RubberGauge = subby_ShowRubberMeter;
    prev_RubberGauge = g_RubberGauge;

    g_ShowTime = showTime;
    prev_ShowTime = g_ShowTime;

    g_24hFormat = show24hour;
    prev_24hFormat = g_24hFormat;

    g_FogR = CUSTOM_FOG_R;
    prev_FogR = g_FogR;

    g_FogG = CUSTOM_FOG_G;
    prev_FogG = g_FogG;

    g_FogB = CUSTOM_FOG_B;
    prev_FogB = g_FogB;

    g_FogDensity = CUSTOM_FOG_DENSITY;
    prev_FogDensity = g_FogDensity;

    extern bool sg_IsNoclipActive();
    g_NoclipMode = ::sg_IsNoclipActive();
    prev_NoclipMode = g_NoclipMode;

    g_CleanScreen = sg_noclipCinematic;
    prev_CleanScreen = g_CleanScreen;

    g_SmartGlance = lp_cam ? lp_cam->smartCustomGlance : false;
    prev_SmartGlance = g_SmartGlance;

    g_FOV = lp_cam ? lp_cam->startFOV : 70.0f;
    prev_FOV = g_FOV;

    g_SpeedGaugeSize = subby_SpeedGaugeSize;
    prev_SpeedGaugeSize = g_SpeedGaugeSize;
    g_SpeedGaugeX = subby_SpeedGaugeLocX;
    prev_SpeedGaugeX = g_SpeedGaugeX;
    g_SpeedGaugeY = subby_SpeedGaugeLocY;
    prev_SpeedGaugeY = g_SpeedGaugeY;

    g_BrakeGaugeSize = subby_BrakeGaugeSize;
    prev_BrakeGaugeSize = g_BrakeGaugeSize;
    g_BrakeGaugeX = subby_BrakeGaugeLocX;
    prev_BrakeGaugeX = g_BrakeGaugeX;
    g_BrakeGaugeY = subby_BrakeGaugeLocY;
    prev_BrakeGaugeY = g_BrakeGaugeY;

    g_RubberGaugeSize = subby_RubberGaugeSize;
    prev_RubberGaugeSize = g_RubberGaugeSize;
    g_RubberGaugeX = subby_RubberGaugeLocX;
    prev_RubberGaugeX = g_RubberGaugeX;
    g_RubberGaugeY = subby_RubberGaugeLocY;
    prev_RubberGaugeY = g_RubberGaugeY;

    g_Sparks = crash_sparks;
    prev_Sparks = g_Sparks;

    g_WhiteSparks = white_sparks;
    prev_WhiteSparks = g_WhiteSparks;

    g_Explosions = sg_crashExplosion;
    prev_Explosions = g_Explosions;

    g_AlphaBlend = sr_alphaBlend;
    prev_AlphaBlend = g_AlphaBlend;

    g_SmoothShading = sr_smoothShading;
    prev_SmoothShading = g_SmoothShading;

    g_FloorDetail = (float)sr_floorDetail;
    prev_FloorDetail = g_FloorDetail;

    g_MainMenuActive = true;
    sn_programVersion = "ILONIUM";
    g_Initialized = true;
}

void ModMenu::Shutdown() {
    if (!g_Initialized) return;
    if (g_AvatarTexture != 0) {
        glDeleteTextures(1, &g_AvatarTexture);
        g_AvatarTexture = 0;
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    HudManager::Shutdown();
    ImGui::DestroyContext();
    g_Initialized = false;
}

void ModMenu::Toggle() {
    g_MenuOpen = !g_MenuOpen;
    SDL_HideCursor();
    if (!g_MenuOpen) {
        st_SaveConfig();
        if (!ModMenu::g_MainMenuActive && (currentScreensetting.fullscreen || su_mouseGrab)) {
            SDL_WM_GrabInput(SDL_GRAB_ON);
        } else {
            SDL_WM_GrabInput(SDL_GRAB_OFF);
        }
    } else {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }
    SDL_HideCursor();
}


bool ModMenu::IsOpen() { return g_MenuOpen; }

//! The same answer under a plain name, for the engine to call.
bool rc_ModMenuOpen() { return g_MenuOpen; }

void ModMenu::SetOpen(bool open) {
    g_MenuOpen = open;
}

//! Where the finder sends you. Ids below a thousand are the places down the
//! left; above that, a section inside the settings, which also has to open the
//! settings themselves on the way.
static void PaletteGo( int what )
{
    // Doing rather than going. A finder that only moves you somewhere still
    // leaves you to hunt for the control once you arrive, and the things
    // people reach for most are a handful of switches, not places.
    if ( what >= 2000 )
    {
        switch ( what )
        {
        case 2000: case 2001: case 2002: case 2003:
            g_ParticleType = (float)( what - 2000 );
            break;

        case 2010: rc_TellPlayer( -1 ); break;
        case 2011: rc_TellPlayer(  0 ); break;
        case 2012: rc_TellPlayer(  1 ); break;

        case 2020:
            isHudEditing = true;
            g_MenuOpen = false;
            break;

        case 2040:
        case 2041:
        case 2042:
            // Straight to the tool, not merely to the drawer it lives in.
            g_ActiveTab = 11;
            g_DashboardActiveCol = 1;
            g_DashboardLeftSelected = 7;
            rc_MiscToolPick( what - 2040 );
            break;

        case 2030:
            g_InteractiveParticles = !g_InteractiveParticles;
            break;

        case 2031:
            g_ParallaxEffect = !g_ParallaxEffect;
            break;
        }
        return;
    }

    if ( what >= 1000 )
    {
        g_ActiveTab = 4;
        g_ModMenuTab = what - 1000;
        g_DashboardLeftSelected = 8;
    }
    else
    {
        g_ActiveTab = what;
        g_DashboardActiveCol = 1;
    }
}

//! Everywhere worth going, said once at start up.
static void PaletteFill()
{
    static char const * const places[] = {
        "Dashboard", "Local Play", "Server Browser", "Settings", "Mod Menu",
        "Socials", "Pickup", "Support Tickets", "IRC Global Chat"
    };
    static int const ids[] = { 0, 1, 2, 3, 4, 8, 7, 9, 10 };

    for ( int i = 0; i < 9; ++i )
        rc_PaletteAdd( ids[i], places[i], "go" );

    static char const * const sections[] = {
        "Visuals", "HUD", "Client", "Noclip", "Util", "Theme", "Configs", "Player", "Profiles"
    };

    for ( int i = 0; i < 9; ++i )
        rc_PaletteAdd( 1000 + i, sections[i], "settings" );

    // The shapes are worth reaching directly. Somebody who wants hearts wants
    // them now, not after finding out which section they were filed under.
    rc_PaletteAdd( 2000, "Particles: Dust",   "look" );
    rc_PaletteAdd( 2001, "Particles: Rain",   "look" );
    rc_PaletteAdd( 2002, "Particles: Stars",  "look" );
    rc_PaletteAdd( 2003, "Particles: Hearts", "look" );
    rc_PaletteAdd( 2030, "Particles follow the cursor", "look" );
    rc_PaletteAdd( 2031, "Depth as the view moves",     "look" );

    rc_PaletteAdd( 2020, "Edit the HUD", "do" );

    rc_PaletteAdd( 11, "Misc", "go" );
    rc_PaletteAdd( 2040, "Command Finder", "tools" );
    rc_PaletteAdd( 2041, "Gradient Name", "tools" );
    rc_PaletteAdd( 2042, "Camera", "tools" );

    rc_PaletteAdd( 2010, "Previous track", "music" );
    rc_PaletteAdd( 2011, "Play or pause",  "music" );
    rc_PaletteAdd( 2012, "Next track",     "music" );

    rc_PaletteHandler( PaletteGo );
}

//! What this client has watched you do, laid out to be read at a glance.
//!
//! Nothing here comes from anywhere else and nothing is seeded: a fresh copy
//! starts at zero for everybody, and every figure grew out of something that
//! actually happened at this machine. That is the only reason to trust any of
//! it, and trust is the whole point of putting numbers on a screen.
static void DrawStatsBlock(ImDrawList* dl, ImVec2 at, ImVec2 size)
{
    rcTheme const & th = rc_Theme();
    rcStats const & st = rc_Stats();

    // Hours and minutes, because seconds are noise at this scale and a bare
    // count of them is something to be decoded rather than read.
    struct Say {
        static void Span(char* into, size_t room, double seconds) {
            int all = (int)seconds;
            int h = all / 3600;
            int m = ( all % 3600 ) / 60;
            if ( h > 0 ) snprintf( into, room, "%dh %02dm", h, m );
            else         snprintf( into, room, "%dm %02ds", m, all % 60 );
        }
    };

    float x = at.x;
    float y = at.y;

    // ---- the three that matter, said large
    char inClient[ 32 ], riding[ 32 ], rounds[ 32 ];
    Say::Span( inClient, sizeof( inClient ), st.clientSeconds );
    Say::Span( riding, sizeof( riding ), st.matchSeconds );
    snprintf( rounds, sizeof( rounds ), "%d", st.rounds );

    char const * bigName[ 3 ] = { "IN THE CLIENT", "RIDING", "ROUNDS" };
    char const * bigWhat[ 3 ] = { inClient, riding, rounds };

    float third = size.x / 3.0f;

    for ( int i = 0; i < 3; ++i )
    {
        ImVec2 was = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( x + third * i, y ) );
        rc_Tracked( bigName[i], 1.5f, th.textMute );
        ImGui::SetCursorScreenPos( was );

        ImGui::PushFont( g_FontDisplay ? g_FontDisplay : ImGui::GetFont() );
        dl->AddText( ImVec2( x + third * i, y + 18.0f ),
                     i == 2 ? th.primary.bright : th.text, bigWhat[i] );
        ImGui::PopFont();
    }

    y += 88.0f;

    // ---- the calendar
    //
    // A row of bars says how much and nothing else. A calendar says when: the
    // week somebody played every night, the fortnight they did not touch it,
    // the day that went on until four in the morning. That is the part worth
    // looking at, and it is why every service that keeps this kind of history
    // ends up drawing the same grid.
    {
        float cellSize = 13.0f;
        float gap = 4.0f;
        float step = cellSize + gap;

        float labelW = 30.0f;
        int weeks = (int)( ( size.x - labelW ) / step );
        if ( weeks > 20 ) weeks = 20;
        if ( weeks < 4 )  weeks = 4;

        time_t nowStamp = time( NULL );
        struct tm today = *localtime( &nowStamp );
        int todayRow = ( today.tm_wday + 6 ) % 7;      // monday first

        // The busiest day sets the scale, so a quiet history still shows its
        // own shape rather than a uniform dark square.
        double most = 1.0;
        for ( size_t i = 0; i < st.days.size(); ++i )
            if ( st.days[i].seconds > most )
                most = st.days[i].seconds;

        {
            ImVec2 was = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos( ImVec2( x, y ) );
            rc_Tracked( "ACTIVITY", 1.5f, th.textMute );
            ImGui::SetCursorScreenPos( was );

            char summary[ 96 ];
            snprintf( summary, sizeof( summary ), "%d rounds   %d kills   %d deaths",
                      st.rounds, st.kills, st.deaths );

            float w = ImGui::CalcTextSize( summary ).x;
            dl->AddText( ImVec2( x + size.x - w, y ), th.textMute, summary );
        }

        // Room for the heading and then the row of month names under it.
        float gridY = y + 44.0f;
        float gridX = x + labelW;

        // All seven, two letters each. Three of them written out was neither
        // one thing nor the other: the gaps read as rows somebody forgot.
        char const * rowName[ 7 ] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
        for ( int r = 0; r < 7; ++r )
        {
            // Centred against its own row rather than hung from the top of it,
            // which is what made them look a line out.
            float textH = ImGui::GetTextLineHeight();
            dl->AddText( ImVec2( x, gridY + r * step + ( cellSize - textH ) * 0.5f ),
                         rc_Fade( th.textMute, 0.75f ), rowName[r] );
        }

        std::string hovered;
        char hoveredWhat[ 96 ] = "";
        ImVec2 hoveredAt;

        int lastMonth = -1;
        int lastMonthCol = -99;

        for ( int col = 0; col < weeks; ++col )
        {
            for ( int row = 0; row < 7; ++row )
            {
                int back = ( weeks - 1 - col ) * 7 + ( todayRow - row );
                if ( back < 0 )
                    continue;   // later this week, not here yet

                time_t stamp = nowStamp - (time_t)back * 24 * 3600;
                struct tm when = *localtime( &stamp );

                char date[ 16 ];
                strftime( date, sizeof( date ), "%Y-%m-%d", &when );

                // The month's name sits over the column its first week starts,
                // and only when there is room for it: two names three columns
                // apart overlap into something unreadable, which is exactly
                // what they were doing.
                if ( row == 0 && when.tm_mon != lastMonth )
                {
                    lastMonth = when.tm_mon;

                    if ( col - lastMonthCol >= 3 || lastMonthCol < 0 )
                    {
                        lastMonthCol = col;
                        char month[ 8 ];
                        strftime( month, sizeof( month ), "%b", &when );
                        dl->AddText( ImVec2( gridX + col * step, gridY - 18.0f ),
                                     rc_Fade( th.textMute, 0.8f ), month );
                    }
                }

                rcDay const * found = NULL;
                for ( size_t i = 0; i < st.days.size(); ++i )
                    if ( st.days[i].when == date )
                    {
                        found = &st.days[i];
                        break;
                    }

                ImVec2 cellMin( gridX + col * step, gridY + row * step );
                ImVec2 cellMax( cellMin.x + cellSize, cellMin.y + cellSize );

                ImU32 fill = rc_Fade( th.line, 0.9f );

                if ( found && found->seconds > 0.0 )
                {
                    // Squared so a short session still lifts off the floor;
                    // linear and everything but the best day looks empty.
                    float much = (float)sqrt( found->seconds / most );
                    fill = rc_Mix( th.primary.wash, th.primary.solid, much );
                }

                dl->AddRectFilled( cellMin, cellMax, fill, 2.5f );

                if ( back == 0 )
                    dl->AddRect( cellMin, cellMax, rc_Fade( th.text, 0.55f ), 2.5f, 0, 1.0f );

                if ( ImGui::IsMouseHoveringRect( cellMin, cellMax ) )
                {
                    char pretty[ 24 ];
                    strftime( pretty, sizeof( pretty ), "%d %b", &when );

                    int mins = found ? (int)( found->seconds / 60.0 ) : 0;

                    if ( found && found->seconds > 0.0 )
                        snprintf( hoveredWhat, sizeof( hoveredWhat ),
                                  "%s  -  %dm, %d rounds, %d kills, %d deaths",
                                  pretty, mins, found->rounds, found->kills, found->deaths );
                    else
                        snprintf( hoveredWhat, sizeof( hoveredWhat ), "%s  -  nothing", pretty );

                    hovered = pretty;
                    hoveredAt = ImVec2( cellMin.x, cellMin.y );
                }
            }
        }

        // Said last so it lies over the grid rather than under the next cell.
        if ( !hovered.empty() )
        {
            float w = ImGui::CalcTextSize( hoveredWhat ).x + 20.0f;
            float h = ImGui::GetTextLineHeight() + 14.0f;

            ImVec2 tipAt( hoveredAt.x - 6.0f, hoveredAt.y - h - 6.0f );
            if ( tipAt.x + w > x + size.x )
                tipAt.x = x + size.x - w;
            if ( tipAt.x < x )
                tipAt.x = x;

            ImVec2 tipTo( tipAt.x + w, tipAt.y + h );

            dl->AddRectFilled( tipAt, tipTo, th.raised, th.radiusSmall );
            dl->AddRect( tipAt, tipTo, th.lineStrong, th.radiusSmall, 0, 1.0f );
            dl->AddText( ImVec2( tipAt.x + 10.0f, tipAt.y + 7.0f ), th.text, hoveredWhat );
        }

        y = gridY + 7.0f * step + 14.0f;
    }

    // ---- the rest, small and in a row
    char best[ 32 ], dist[ 32 ], deaths[ 32 ], sessions[ 32 ];
    Say::Span( best, sizeof( best ), st.bestLife );
    snprintf( dist, sizeof( dist ), "%.0f", st.distance );
    snprintf( deaths, sizeof( deaths ), "%d", st.deaths );
    snprintf( sessions, sizeof( sessions ), "%d", st.sessions );

    char kills[ 32 ];
    snprintf( kills, sizeof( kills ), "%d", st.kills );

    char const * smallName[ 5 ] = { "LONGEST LIFE", "KILLS", "DEATHS", "DISTANCE", "LAUNCHES" };
    char const * smallWhat[ 5 ] = { best, kills, deaths, dist, sessions };

    float quarter = size.x / 5.0f;

    for ( int i = 0; i < 5; ++i )
    {
        ImVec2 was = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( x + quarter * i, y ) );
        rc_Tracked( smallName[i], 1.3f, th.textMute );
        ImGui::SetCursorScreenPos( was );

        ImGui::PushFont( g_FontMono ? g_FontMono : ImGui::GetFont() );
        dl->AddText( ImVec2( x + quarter * i, y + 17.0f ), th.textDim, smallWhat[i] );
        ImGui::PopFont();
    }

    y += 42.0f;

    // ---- one line that puts the rest in context
    char since[ 96 ];
    char thisRun[ 32 ];
    Say::Span( thisRun, sizeof( thisRun ), rc_StatsSession() );
    int streak = rc_StatsStreak();
    if ( streak > 1 )
        snprintf( since, sizeof( since ), "counting since %s  -  this session %s  -  %d days in a row",
                  st.firstSeen.c_str(), thisRun, streak );
    else
        snprintf( since, sizeof( since ), "counting since %s  -  this session %s",
                  st.firstSeen.c_str(), thisRun );

    dl->AddText( ImVec2( x, y ), rc_Fade( th.textMute, 0.85f ), since );
}

//! The waypoint list, while the free camera is flying.
//!
//! Shown over the game rather than in a menu, because it is used while flying:
//! a list you have to open a menu to read is a list you fly away from. It
//! keeps out of the way otherwise - no free camera, no panel.
bool sg_waypointPanel = true;
static tConfItem<bool> conf_waypointPanel( "MOD_WAYPOINT_PANEL", sg_waypointPanel );

static void DrawWaypointPanel()
{
    extern bool g_NoclipMode;

    extern bool sg_waypointsVisible;

    if ( !sg_waypointPanel || !g_NoclipMode || !sg_waypointsVisible )
        return;

    rcTheme const & th = rc_Theme();
    ImGuiIO & io = ImGui::GetIO();

    std::vector< rcWaypoint > const & marks = rc_Waypoints();
    int here = rc_WaypointCurrent();

    float const rowH = 26.0f;

    // The panel is the pilot's own list: it is dragged where it suits and
    // pulled to the height that shows as many of them as are wanted. It used
    // to be sized from the number of waypoints on every frame, which took the
    // resize grip away and grew the window off the bottom of the screen as
    // soon as there were more marks than fitted - the rows were still being
    // drawn, below the edge, where nobody could see them.
    ImGui::SetNextWindowPos( ImVec2( 24.0f, io.DisplaySize.y * 0.5f - 180.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSize( ImVec2( 264.0f, 360.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSizeConstraints( ImVec2( 210.0f, 150.0f ),
                                         ImVec2( 560.0f, io.DisplaySize.y - 40.0f ) );

    ImGui::PushStyleColor( ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4( rc_Fade( th.surface, 0.94f ) ) );
    ImGui::PushStyleColor( ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4( th.line ) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, th.radiusMedium );

    if ( ImGui::Begin( "Waypoints##rcWaypoints", NULL,
                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoSavedSettings ) )
    {
        rc_Tracked( "WAYPOINTS", 1.5f, th.textMute );
        ImGui::Spacing();

        if ( ImGui::Button( "Save this view", ImVec2( -1.0f, 26.0f ) ) )
            rc_WaypointAdd();

        ImGui::Spacing();

        // Everything below the button scrolls, so a long list runs out of room
        // rather than out of the window.
        ImGui::BeginChild( "##rcWaypointList", ImVec2( 0.0f, 0.0f ), false );

        if ( marks.empty() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textMute ) );
            ImGui::TextUnformatted( "none yet" );
            ImGui::PopStyleColor();
        }

        // Deleting inside the loop would pull the ground out from under it, so
        // the row that was asked for is remembered and dealt with afterwards.
        int drop = -1;

        for ( size_t i = 0; i < marks.size(); ++i )
        {
            ImGui::PushID( (int)i );

            char label[ 64 ];
            if ( marks[i].name.empty() )
                snprintf( label, sizeof( label ), "%d", (int)i + 1 );
            else
                snprintf( label, sizeof( label ), "%d  %s", (int)i + 1, marks[i].name.c_str() );

            bool current = ( (int)i == here );

            // measured against the room there actually is, so the rows stay
            // lined up whatever width the panel has been pulled to
            float const room = ImGui::GetContentRegionAvail().x;

            ImGui::PushStyleColor( ImGuiCol_Text,
                ImGui::ColorConvertU32ToFloat4( current ? th.primary.bright : th.textDim ) );

            if ( ImGui::Selectable( label, current, 0, ImVec2( room - 30.0f, rowH - 6.0f ) ) )
                rc_WaypointGoTo( (int)i );

            ImGui::PopStyleColor();

            ImGui::SameLine( room - 20.0f );
            if ( ImGui::SmallButton( "x" ) )
                drop = (int)i;

            ImGui::PopID();
        }

        ImGui::EndChild();

        if ( drop >= 0 )
            rc_WaypointDelete( drop );
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor( 2 );
}

//! Set while the pointer we draw ourselves is on screen.
//!
//! A dozen places in this file ask for the system's cursor back, most of them
//! every frame, and they were doing it a frame after ours had asked for it to
//! go away. The result was not two pointers so much as one blinking. They all
//! go through here now, and here says no while ours is up.
static bool sg_ourCursor = false;

static void ShowSystemCursor()
{
    if ( !sg_ourCursor )
        SDL_ShowCursor();
}

//! The finder, as an ordinary entry in the mod menu with an ordinary binding,
//! rather than a shortcut written into the code and a settings page of its
//! own. Everything else in there is rebound the same way, and a feature that
//! needs its own corner of the interface to be configured is a feature nobody
//! finds.
bool g_FinderEnabled = true;
int  g_FinderKeybind = 0;
static tConfItem<bool> conf_finderOn( "MOD_FINDER", g_FinderEnabled );
static tConfItem<int>  conf_finderBind( "MOD_FINDER_KEYBIND", g_FinderKeybind );

//! Kept for anybody who had already moved it before this became a binding.
static int  g_FinderKey  = (int)ImGuiKey_K;
static bool g_FinderCtrl = true;
static tConfItem<int>  conf_finderKey( "MOD_FINDER_KEY", g_FinderKey );
static tConfItem<bool> conf_finderCtrl( "MOD_FINDER_CTRL", g_FinderCtrl );

//! True while there is an interface on screen to search through.
//!
//! The finder used to be reachable from anywhere, which meant it also came up
//! in the middle of a round, on top of the game, where there is nothing for it
//! to take you to. It belongs to the menus and nowhere else.
static bool MenuIsUp()
{
    // The older menu counts too. It is still what comes up on escape during a
    // match, and leaving it with the system pointer while every other screen
    // has ours is exactly the kind of seam that gives a client away.
    return ModMenu::g_MainMenuActive || g_MenuOpen || g_MenuAlpha > 0.01f ||
           uMenu::MenuActive();
}

//! The finder and the interface's own voice, on every frame the client has.
//! There are five of them - the main menu draws itself on one, the in game
//! menu on another - and a shortcut that works on some screens and not others
//! is worse than not having it at all.
static void PaletteFrame()
{
    static bool filled = false;
    if ( !filled )
    {
        filled = true;
        PaletteFill();
    }

    // Counted here because this runs on every frame the client has, and the
    // count itself knows to ignore being called more than once in one.
    rc_StatsTick( 0.0f );

    bool up = MenuIsUp();

    // Rather than hanging a sound on every button by hand and missing half of
    // them, ask the interface what the hand is on. Anything that can be
    // hovered or pressed answers, so a control added later is heard without
    // anybody remembering to wire it up.
    if ( up )
    {
        static ImGuiID lastHovered = 0;
        static ImGuiID lastActive  = 0;

        ImGuiID hovered = ImGui::GetHoveredID();
        ImGuiID active  = ImGui::GetActiveID();

        if ( hovered && hovered != lastHovered )
            se_PlayUi( "hover" );

        if ( active && active != lastActive )
            se_PlayUi( "select" );

        lastHovered = hovered;
        lastActive = active;
    }

    if ( up )
    {
        // Asked before anything else gets the keyboard, so a text field
        // somewhere underneath cannot swallow the key that opens it.
        bool wanted = g_FinderEnabled &&
                      ImGui::IsKeyPressed( (ImGuiKey)g_FinderKey, false ) &&
                      ( !g_FinderCtrl || ImGui::GetIO().KeyCtrl );

        if ( wanted )
            rc_PaletteToggle();
    }
    else
    {
        rc_PaletteClose();
    }

    rc_PaletteDraw();

    DrawWaypointPanel();

    // Said once a session, the first time the backend answers with a newer
    // build than this one. The card in the Client tab is where the detail
    // lives; this is what makes anybody go and look at it.
    {
        static bool told = false;
        if ( !told && rc_UpdateWaiting() )
        {
            told = true;
            std::string const say = "Update available - " + rc_UpdateVersion() + " - ilonium.dev";
            rc_Toast( say.c_str() );
        }
    }

    rc_DrawToasts();

    // Last of all, so nothing is ever drawn over the pointer, and after every
    // other place in this file that has an opinion about the cursor - there
    // are a dozen of them, and whoever speaks last wins.
    static bool weHidIt = false;

    bool ours = up && rc_DrawCursor( ImGui::IsMouseDown( ImGuiMouseButton_Left ) );
    sg_ourCursor = ours;

    // The other pointer was never the system's at all: four places ask the
    // interface to draw its own arrow, and that is what was sitting beside
    // ours. Said here, after all of them and before anything is drawn, so it
    // is this that decides.
    if ( ours )
        ImGui::GetIO().MouseDrawCursor = false;

    if ( ours )
    {
        SDL_HideCursor();
        weHidIt = true;
    }
    else if ( weHidIt )
    {
        // Either the menu closed or ours stood down mid stall. Give the
        // system's back and let whoever owns the screen decide from there.
        SDL_ShowCursor();

        weHidIt = false;
    }
}

void ModMenu::OpenTab(int tabId) {
    g_ActiveTab = tabId;
    g_DashboardActiveCol = 1;
    SetOpen(true);
}

void ModMenu::ApplySettingsToEngine() {
    ePlayer* lp_cam = ePlayer::PlayerConfig(0);

    subby_ShowHUD = g_ShowHUD;
    subby_ShowSpeedFastest = g_ShowFastest;
    subby_ShowScore = g_ShowScores;
    subby_ShowAlivePeople = g_AliveCounter;
    subby_ShowPing = g_ShowPing;
    subby_ShowSpeedMeter = g_SpeedMeter;
    subby_ShowBrakeMeter = g_BrakeMeter;
    subby_ShowRubberMeter = g_RubberGauge;
    showTime = g_ShowTime;
    show24hour = g_24hFormat;

    CUSTOM_FOG_R = g_FogR;
    CUSTOM_FOG_G = g_FogG;
    CUSTOM_FOG_B = g_FogB;
    CUSTOM_FOG_DENSITY = g_FogDensity;

    if (lp_cam) {
        lp_cam->smartCustomGlance = g_SmartGlance;
        lp_cam->startFOV = g_FOV;
            rc_CameraFOV( (int)g_FOV );
    }

    subby_SpeedGaugeSize = g_SpeedGaugeSize;
    subby_SpeedGaugeLocX = g_SpeedGaugeX;
    subby_SpeedGaugeLocY = g_SpeedGaugeY;
    subby_BrakeGaugeSize = g_BrakeGaugeSize;
    subby_BrakeGaugeLocX = g_BrakeGaugeX;
    subby_BrakeGaugeLocY = g_BrakeGaugeY;
    subby_RubberGaugeSize = g_RubberGaugeSize;
    subby_RubberGaugeLocX = g_RubberGaugeX;
    subby_RubberGaugeLocY = g_RubberGaugeY;

    crash_sparks = g_Sparks;
    white_sparks = g_WhiteSparks;
    sg_crashExplosion = g_Explosions;
    sr_alphaBlend = g_AlphaBlend;
    sr_smoothShading = g_SmoothShading;
    sr_floorDetail = (int)( g_FloorDetail + 0.5f );   // rounding, or the top setting can never be reached

    // Trackers
    prev_ShowHUD = g_ShowHUD;
    prev_ShowFastest = g_ShowFastest;
    prev_ShowScores = g_ShowScores;
    prev_AliveCounter = g_AliveCounter;
    prev_ShowPing = g_ShowPing;
    prev_SpeedMeter = g_SpeedMeter;
    prev_BrakeMeter = g_BrakeMeter;
    prev_RubberGauge = g_RubberGauge;
    prev_ShowTime = g_ShowTime;
    prev_24hFormat = g_24hFormat;
    prev_FogR = g_FogR;
    prev_FogG = g_FogG;
    prev_FogB = g_FogB;
    prev_FogDensity = g_FogDensity;
    prev_NoclipMode = g_NoclipMode;
    sg_noclipCinematic = g_CleanScreen;
    prev_CleanScreen = g_CleanScreen;
    if (lp_cam) {
        prev_SmartGlance = g_SmartGlance;
        prev_FOV = g_FOV;
    }
    prev_SpeedGaugeSize = g_SpeedGaugeSize;
    prev_SpeedGaugeX = g_SpeedGaugeX;
    prev_SpeedGaugeY = g_SpeedGaugeY;
    prev_BrakeGaugeSize = g_BrakeGaugeSize;
    prev_BrakeGaugeX = g_BrakeGaugeX;
    prev_BrakeGaugeY = g_BrakeGaugeY;
    prev_RubberGaugeSize = g_RubberGaugeSize;
    prev_RubberGaugeX = g_RubberGaugeX;
    prev_RubberGaugeY = g_RubberGaugeY;
    prev_Sparks = g_Sparks;
    prev_WhiteSparks = g_WhiteSparks;
    prev_Explosions = g_Explosions;
    prev_AlphaBlend = g_AlphaBlend;
    prev_SmoothShading = g_SmoothShading;
    prev_FloorDetail = g_FloorDetail;
}

bool ModMenu::ProcessEvent(const SDL_Event* event) {
    if (!g_Initialized) return false;

    // Exit HUD Editor Mode when Escape is pressed
    if (isHudEditing && event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        if (!ImGui::GetIO().WantTextInput) {
            isHudEditing = false;
            return true;
        }
    }

    // One of our own blocking overlays owns the screen: the login box, the
    // server notices. They run their own loop on top of whatever was there,
    // and what was there during a connect is a legacy menu - so the guard
    // below would hand every keystroke back to a menu nobody can see, and the
    // field in front of the player would never receive a letter. Escape still
    // worked, because the legacy layer answers that one itself, which is
    // exactly how this looked from the outside: a blinking caret that takes
    // nothing but cancel.
    if (ModMenu::g_ModalOverlayOpen) {
        ImGui_ImplSDL3_ProcessEvent(event);
        return true;
    }

    // If a legacy menu is active, do not intercept any events
    if (uMenu::IsLegacyMenuActive()) {
        return false;
    }



    // Manual Packet Refresh keybind (Anti-Lag trigger)
    if (event->type == SDL_EVENT_KEY_DOWN && g_PacketRefreshKeybind != 0 && event->key.key == g_PacketRefreshKeybind) {
        if (!ImGui::GetIO().WantTextInput) {
            sn_Receive();
            nNetObject::SyncAll();
            sn_SendPlanned();
            sn_LastPacketTime = tSysTimeFloat();
            return true;
        }
    }
    
    // Capture keybinding if waiting
    if (g_BindingKeybindPtr && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.key;
        if (key == SDLK_ESCAPE) {
            *g_BindingKeybindPtr = 0; // Clear bind
        } else {
            *g_BindingKeybindPtr = key;
        }
        g_BindingKeybindPtr = nullptr;
        st_SaveConfig(); // Save configuration immediately!
        return true;
    }

    // Capture gameplay action keybinding if waiting
    if (s_BindingAction && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.scancode;
        if (key == SDL_SCANCODE_ESCAPE) {
            // ESC clears all existing keymap binds for this action
            for (int sym_i = 0; sym_i < SDLK_NEWLAST; ++sym_i) {
                if (keymap[sym_i] && keymap[sym_i]->act == s_BindingAction &&
                    keymap[sym_i]->CheckPlayer(s_BindingSlot)) {
                    keymap[sym_i] = nullptr;
                }
            }
        } else {
            if (key >= 0 && key < SDLK_NEWLAST) {
                keymap[key] = uBindPlayer::NewBind(s_BindingAction, s_BindingSlot);
            }
        }
        s_BindingAction = nullptr;
        st_SaveConfig();
        return true;
    }
    
    if (event->type == SDL_EVENT_KEY_DOWN && g_ModMenuKeybindEnabled && g_ModMenuKeybind != 0 && event->key.key == g_ModMenuKeybind) {
        if (g_MenuOpen) {
            Toggle();
        } else if (g_MainMenuActive) {
            if (g_ActiveTab == 4) {
                g_ActiveTab = 0;
                g_DashboardLeftSelected = 0;
            } else {
                g_ActiveTab = 4;
                g_DashboardLeftSelected = 4;
                g_DashboardActiveCol = 2;
            }
        } else {
            if (ModMenu::g_InGameMenuOpen) {
                g_CloseInGameMenuRequested = true;
            }
            Toggle();
        }
        return true;
    }
    if (g_MenuOpen && event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        Toggle();
        return true;
    }
    if ((g_MainMenuActive && !g_CustomMainMenuTempDisabled || ModMenu::g_InGameMenuOpen) && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.key;
        if (key == SDLK_ESCAPE) {
            if (g_OpenApplyConfigPopup || g_OpenDeleteConfigPopup || g_OpenCreateConfigPopup || g_OpenUpdateConfigPopup || g_OpenResetHUDPopup) {
                g_OpenApplyConfigPopup = false;
                g_OpenDeleteConfigPopup = false;
                g_OpenCreateConfigPopup = false;
                g_OpenUpdateConfigPopup = false;
                g_OpenResetHUDPopup = false;
                return true;
            }
            if (s_BindingAction) {
                s_BindingAction = nullptr;
                return true;
            }
            if (g_DashboardActiveCol != 0) {
                g_DashboardActiveCol = 0;
                return true;
            }
            if (g_ActiveTab != 0) {
                g_ActiveTab = 0;
                g_DashboardLeftSelected = 0;
                return true;
            }
            if (ModMenu::g_InGameMenuOpen) {
                g_CloseInGameMenuRequested = true;
                return true;
            }
            uMenu::quickexit = uMenu::QuickExit_Total;
            return true;
        }
        
        bool inPopup = g_OpenApplyConfigPopup || g_OpenDeleteConfigPopup || g_OpenCreateConfigPopup || g_OpenUpdateConfigPopup || g_OpenResetHUDPopup;
        if (!inPopup && !ImGui::GetIO().WantTextInput) {
            // Right / Left arrow key column cycling
            if (key == SDLK_RIGHT) {
                if (g_ActiveTab == 0 || g_ActiveTab == 7 || g_DashboardActiveCol == 0) {
                    if (g_DashboardActiveCol < 2) {
                        g_DashboardActiveCol++;
                        return true;
                    }
                }
            }
            if (key == SDLK_LEFT) {
                if (g_ActiveTab == 0 || g_ActiveTab == 7 || g_DashboardActiveCol == 0) {
                    if (g_DashboardActiveCol > 0) {
                        g_DashboardActiveCol--;
                        return true;
                    }
                }
            }

            // Decide if ImGui's native keyboard navigation should handle this key
            // We want ImGui to handle keys if we are on a settings page (g_ActiveTab != 0 && g_ActiveTab != 7) AND focused on settings columns (g_DashboardActiveCol != 0)
            bool letImGuiHandle = (g_ActiveTab != 0 && g_ActiveTab != 7 && g_DashboardActiveCol != 0);

            if (!letImGuiHandle) {
                if (key == SDLK_TAB) {
                    if (event->key.mod & KMOD_SHIFT) {
                        g_DashboardActiveCol = (g_DashboardActiveCol + 2) % 3;
                    } else {
                        g_DashboardActiveCol = (g_DashboardActiveCol + 1) % 3;
                    }
                    return true;
                }
                if (key == SDLK_UP) {
                    se_PlayUi("hover");
                    if (g_DashboardActiveCol == 0) {
                        int numNavItems = 11;
                        if (ModMenu::g_InGameMenuOpen) {
                            numNavItems = sg_hasLastServer ? 8 : 7;
                        }
                        g_DashboardLeftSelected = (g_DashboardLeftSelected + numNavItems - 1) % numNavItems;
                    } else if (g_DashboardActiveCol == 1) {
                        g_DashboardMiddleSelected = (g_DashboardMiddleSelected + 1) % 2;
                    } else if (g_DashboardActiveCol == 2) {
                        int totalRightItems = (int)s_DashboardConfigs.size() + 1;
                        g_DashboardRightSelected = (g_DashboardRightSelected + totalRightItems - 1) % totalRightItems;
                    }
                    return true;
                }
                if (key == SDLK_DOWN) {
                    se_PlayUi("hover");
                    if (g_DashboardActiveCol == 0) {
                        int numNavItems = 11;
                        if (ModMenu::g_InGameMenuOpen) {
                            numNavItems = sg_hasLastServer ? 8 : 7;
                        }
                        g_DashboardLeftSelected = (g_DashboardLeftSelected + 1) % numNavItems;
                    } else if (g_DashboardActiveCol == 1) {
                        g_DashboardMiddleSelected = (g_DashboardMiddleSelected + 1) % 2;
                    } else if (g_DashboardActiveCol == 2) {
                        int totalRightItems = (int)s_DashboardConfigs.size() + 1;
                        g_DashboardRightSelected = (g_DashboardRightSelected + 1) % totalRightItems;
                    }
                    return true;
                }
                if (key == SDLK_RETURN || key == SDLK_SPACE) {
                    // The same voice the mouse gets. Somebody driving this
                    // from the keyboard was moving through a silent client.
                    se_PlayUi("select");
                    g_DashboardActionTriggered = true;
                    return true;
                }
            }
        }
    }
    // Always forward event to ImGui to keep its keyboard/mouse state updated, e.g. for Keystroke visualizer
    ImGui_ImplSDL3_ProcessEvent(event);

    if (g_MenuOpen || ModMenu::g_InGameMenuOpen || (g_MainMenuActive && !g_CustomMainMenuTempDisabled) || isHudEditing) {
        return true;
    }
    
    // Process keybinds when menu is closed and user is not chatting
    bool isChatting = false;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID* p = se_PlayerNetIDs[i];
        if (p && p->Owner() == sn_myNetID && p->IsChatting()) {
            isChatting = true;
            break;
        }
    }

    if (!g_MenuOpen && !ModMenu::g_InGameMenuOpen && !isChatting && !ImGui::GetIO().WantTextInput && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.key;
        if (key != 0) {
            bool matched = false;
            auto isKeyMatch = [&](int bind, int eventKey, SDL_Scancode scancode) -> bool {
                if (bind <= 0) return false;
                if (eventKey == bind) return true;
                if (eventKey >= 'A' && eventKey <= 'Z' && (eventKey + 32) == bind) return true;
                if (bind >= 'A' && bind <= 'Z' && (bind + 32) == eventKey) return true;
                if (scancode > 0 && SDL_GetScancodeFromKey((SDL_Keycode)bind) == scancode) return true;
                return false;
            };

            auto checkAndToggle = [&](int bind, bool& val) {
                if (isKeyMatch(bind, key, event->key.scancode)) {
                    val = !val;
                    matched = true;
                    se_PlayUiClick();
                }
            };
            bool prevNoclip = g_NoclipMode;
            checkAndToggle(g_NoclipKeybind, g_NoclipMode);
            if (g_NoclipMode != prevNoclip) {
                tString m;
                m << "NoClip Flight Mode: " << (g_NoclipMode ? "ENABLED (WASD / Mouse Look)" : "DISABLED");
                sr_con.DoCenterDisplay(m, 2.0, 0.3, 1.0, 0.8);
            }
            checkAndToggle(g_CleanScreenKeybind, g_CleanScreen);
            checkAndToggle(g_CustomHitboxKeybind, g_CustomHitbox);
            checkAndToggle(g_CustomFogKeybind, g_CustomFog);
            checkAndToggle(g_ShowHUDKeybind, g_ShowHUD);
            checkAndToggle(g_RubberGaugeKeybind, g_RubberGauge);
            checkAndToggle(g_SpeedMeterKeybind, g_SpeedMeter);
            checkAndToggle(g_BrakeMeterKeybind, g_BrakeMeter);
            checkAndToggle(g_ShowScoresKeybind, g_ShowScores);
            checkAndToggle(g_ShowPingKeybind, g_ShowPing);
            checkAndToggle(g_AliveCounterKeybind, g_AliveCounter);
            checkAndToggle(g_ShowFastestKeybind, g_ShowFastest);
            checkAndToggle(g_ShowTimeKeybind, g_ShowTime);
            checkAndToggle(g_24hFormatKeybind, g_24hFormat);
            checkAndToggle(g_SparksKeybind, g_Sparks);
            checkAndToggle(g_WhiteSparksKeybind, g_WhiteSparks);
            checkAndToggle(g_ExplosionsKeybind, g_Explosions);
            checkAndToggle(g_RGBTopBarKeybind, g_RGBTopBar);
            checkAndToggle(g_RGBAccentKeybind, g_RGBAccent);

            checkAndToggle(g_ShowParticlesKeybind, g_ShowParticles);
            checkAndToggle(g_InteractiveParticlesKeybind, g_InteractiveParticles);
            checkAndToggle(g_ConstellationWebKeybind, g_ConstellationWeb);
            checkAndToggle(g_ParallaxEffectKeybind, g_ParallaxEffect);
            checkAndToggle(g_GradientAccentKeybind, g_GradientAccent);
            bool prevEnh = g_EnhancedGraphicsMode;
            checkAndToggle(g_EnhancedGraphicsKeybind, g_EnhancedGraphicsMode);
            checkAndToggle(g_NeonFloorKeybind, sg_neonFloor);
            checkAndToggle(g_NeonWallsKeybind, sg_neonWalls);
            checkAndToggle(g_ZoneGlowKeybind, sg_zoneGlow);
            checkAndToggle(g_ZoneGlowOwnKeybind, sg_zoneGlowOwnWhite);
            checkAndToggle(g_FloodlightsKeybind, sg_floodlights);
            checkAndToggle(g_FloodlightBeamsKeybind, sg_floodlightShafts);
            checkAndToggle(g_HideCyclesKeybind, sg_hideOtherCycles);
            checkAndToggle(g_HideTrailsKeybind, sg_hideOtherTrails);
            checkAndToggle(g_HeartSparksKeybind, sg_heartSparks);
            checkAndToggle(g_HeartTrailKeybind, sg_heartTrail);
            checkAndToggle(g_HeartDeathsKeybind, sg_heartDeaths);
            if (g_EnhancedGraphicsMode != prevEnh) {
                tString m;
                m << "Enhanced Visuals: " << (g_EnhancedGraphicsMode ? "ENABLED (Neon/Shaders)" : "DISABLED (Classic Original Game)");
                sr_con.DoCenterDisplay(m, 2.0, 1.0, 0.85, 0.3);
            }
            if (g_ResetKDKeybind != 0 && key == g_ResetKDKeybind) {
                sg_modKDResetFlag = true;
                matched = true;
            }
            if (g_MediaPlayPauseKeybind != 0 && key == g_MediaPlayPauseKeybind) {
                SendMediaCommand(0);
                matched = true;
            }
            if (g_MediaNextKeybind != 0 && key == g_MediaNextKeybind) {
                SendMediaCommand(1);
                matched = true;
            }
            if (g_MediaPrevKeybind != 0 && key == g_MediaPrevKeybind) {
                SendMediaCommand(-1);
                matched = true;
            }
            
            if (matched) {
                ApplySettingsToEngine();
                st_SaveConfig();
                return true;
            }
        }
    }
    return false;
}

static ImVec4 RGBColorAtTime(float time) {
    float r, g, b;
    ImGui::ColorConvertHSVtoRGB(fmodf(time * g_RGBSpeed, 1.0f), 1.0f, 1.0f, r, g, b);
    return ImVec4(r, g, b, 1.0f);
}

void DrawBackgroundParticles(ImDrawList* dl, ImVec2 winPos, ImVec2 winSize, float alphaMultiplier) {
    // The ground the particles drift over. Drawn from here so every screen
    // that had particles has a background now, rather than one of them.
    rc_DrawBackdrop(dl, winPos, winSize, alphaMultiplier);

    if (!g_ShowParticles) return;
    if (winSize.x <= 1.0f || winSize.y <= 1.0f) return;

    static float lastParticleType = -1.0f;
    if (lastParticleType != g_ParticleType) {
        g_Particles.clear();
        lastParticleType = g_ParticleType;
    }
    if (g_Particles.empty()) {
        for (int i = 0; i < 50; i++) {
            MenuParticle p;
            p.pos = ImVec2(winPos.x + (float)(rand() % (int)winSize.x), winPos.y + (float)(rand() % (int)winSize.y));
            
            // Assign to one of three depth layers
            int layer = rand() % 3;
            if (layer == 0) {
                p.depth = 1.0f;
                p.radius = 1.0f + (rand() % 100) * 0.01f;
                p.vel = ImVec2(-8.0f - (rand() % 12), 0.0f);
            } else if (layer == 1) {
                p.depth = 1.8f;
                p.radius = 2.0f + (rand() % 100) * 0.01f;
                p.vel = ImVec2(-18.0f - (rand() % 25), 0.0f);
            } else {
                p.depth = 2.6f;
                p.radius = 3.2f + (rand() % 100) * 0.01f;
                p.vel = ImVec2(-32.0f - (rand() % 40), 0.0f);
            }
            g_Particles.push_back(p);
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    float dt = io.DeltaTime;
    if (dt > 0.1f) dt = 0.1f;
    float menuTime = (float)SDL_GetTicks() * 0.001f;

    // Cycle the gradient colors if requested
    ImVec4 c1 = g_RGBAccent ? RGBColorAtTime(menuTime) : g_AccentColor1;
    ImVec4 c2 = g_RGBAccent ? RGBColorAtTime(menuTime + 1.5f) : g_AccentColor2;

    for (size_t i = 0; i < g_Particles.size(); i++) {
        MenuParticle& p = g_Particles[i];
        
        // Wrap coordinates if screen bounds change
        if (p.pos.x < winPos.x - 20.0f) {
            p.pos.x = winPos.x + winSize.x + 10.0f;
            p.pos.y = winPos.y + (float)(rand() % (int)winSize.y);
        }
        if (p.pos.x > winPos.x + winSize.x + 20.0f) {
            p.pos.x = winPos.x - 10.0f;
            p.pos.y = winPos.y + (float)(rand() % (int)winSize.y);
        }
        if (p.pos.y < winPos.y - 20.0f) p.pos.y = winPos.y + winSize.y + 10.0f;
        if (p.pos.y > winPos.y + winSize.y + 20.0f) p.pos.y = winPos.y - 10.0f;

        // 1. Particle Physics / Movement
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;

        // Apply mouse repulsion if interactive
        if (g_InteractiveParticles) {
            float dx = p.pos.x - io.MousePos.x;
            float dy = p.pos.y - io.MousePos.y;
            float distSq = dx * dx + dy * dy;
            if (distSq > 0.001f && distSq < 150.0f * 150.0f) {
                float dist = sqrtf(distSq);
                float force = (150.0f - dist) / 150.0f; // Scale force 0 to 1
                float push = force * 60.0f / p.depth; // Deeper particles resist push
                p.pos.x += (dx / dist) * push * dt;
                p.pos.y += (dy / dist) * push * dt;
            }
        }

        // 2. Parallax Camera/Mouse offsets
        ImVec2 renderPos = p.pos;
        if (g_ParallaxEffect) {
            float parallaxScaleX = (io.MousePos.x - (winPos.x + winSize.x * 0.5f)) * 0.03f;
            float parallaxScaleY = (io.MousePos.y - (winPos.y + winSize.y * 0.5f)) * 0.03f;
            renderPos.x += parallaxScaleX * (p.depth - 1.0f);
            renderPos.y += parallaxScaleY * (p.depth - 1.0f);
        }

        // 3. Render Web/Lines between close particles (Constellation style)
        if (g_ConstellationWeb) {
            for (size_t j = i + 1; j < g_Particles.size(); j++) {
                MenuParticle& other = g_Particles[j];
                float dx = renderPos.x - other.pos.x;
                float dy = renderPos.y - other.pos.y;
                float distSq = dx * dx + dy * dy;
                if (distSq < 70.0f * 70.0f) {
                    float dist = sqrtf(distSq);
                    float alpha = (70.0f - dist) / 70.0f;
                    float baseAlpha = alpha * 0.12f / (p.depth * other.depth);
                    
                    float tCol = (renderPos.x - winPos.x) / winSize.x;
                    if (tCol < 0.0f) tCol = 0.0f;
                    if (tCol > 1.0f) tCol = 1.0f;
                    
                    ImU32 lineCol;
                    if (g_GradientAccent) {
                        lineCol = ImGui::GetColorU32(ImLerp(c1, c2, tCol));
                    } else {
                        lineCol = ImGui::GetColorU32(g_AccentColor);
                    }
                    lineCol = (lineCol & 0x00FFFFFF) | (((unsigned int)((lineCol >> 24) * baseAlpha * alphaMultiplier)) << 24);
                    
                    dl->AddLine(renderPos, other.pos, lineCol, 1.0f);
                }
            }
        }

        // 4. Render Particle itself
        float tCol = (renderPos.x - winPos.x) / winSize.x;
        if (tCol < 0.0f) tCol = 0.0f;
        if (tCol > 1.0f) tCol = 1.0f;
        
        ImU32 col;
        if (g_GradientAccent) {
            col = ImGui::GetColorU32(ImLerp(c1, c2, tCol));
        } else {
            col = ImGui::GetColorU32(g_AccentColor);
        }
        
        float particleAlpha = 0.45f / p.depth;
        col = (col & 0x00FFFFFF) | (((unsigned int)((col >> 24) * particleAlpha * alphaMultiplier)) << 24);
        
        if ((int)g_ParticleType == 0) { // Dust
            dl->AddCircleFilled(renderPos, p.radius, col);
        } else if ((int)g_ParticleType == 1) { // Rain
            dl->AddLine(renderPos, ImVec2(renderPos.x + p.vel.x * 0.05f, renderPos.y + 8.0f), col, 1.0f);
        } else if ((int)g_ParticleType == 2) { // Stars
            float pulse = 0.5f + 0.5f * sinf(menuTime * 3.5f + renderPos.x * 0.1f);
            float r = p.radius * (1.0f + pulse);
            dl->AddLine(ImVec2(renderPos.x - r, renderPos.y), ImVec2(renderPos.x + r, renderPos.y), col, 1.0f);
            dl->AddLine(ImVec2(renderPos.x, renderPos.y - r), ImVec2(renderPos.x, renderPos.y + r), col, 1.0f);
        } else if ((int)g_ParticleType == 3) { // Hearts
            // Each one keeps its own beat and its own tilt, taken from where it
            // happens to be, so a screenful of them never pulses in unison -
            // which is what makes a field of shapes look like a screensaver.
            float own = renderPos.x * 0.021f + renderPos.y * 0.013f;
            float beat = 1.0f + 0.14f * sinf(menuTime * 2.4f + own * 6.0f);
            float lean = sinf(menuTime * 0.5f + own * 3.0f) * 0.32f;

            // Bigger than a dot of the same radius would be, or they read as
            // smudges rather than as a shape somebody chose.
            rc_DrawHeart(dl, renderPos, p.radius * 3.1f * beat, lean, col);
        }
    }
}

void RenderModMenuModals() {
    ePlayer* lp_cam = ePlayer::PlayerConfig(0);
    // Open popups if requested
    if (g_OpenResetAllPopup) {
        ImGui::OpenPopup("Confirm Reset All");
        g_OpenResetAllPopup = false;
    }
    if (g_OpenResetHUDPopup) {
        ImGui::OpenPopup("Confirm Reset HUD Layout");
        g_OpenResetHUDPopup = false;
    }
    if (g_OpenThemePopup) {
        ImGui::OpenPopup("Custom Theme Creator");
        g_OpenThemePopup = false;
    }
    if (g_OpenApplyConfigPopup) {
        ImGui::OpenPopup("Confirm Apply Config");
        g_OpenApplyConfigPopup = false;
    }
    if (g_OpenDeleteConfigPopup) {
        ImGui::OpenPopup("Confirm Delete Config");
        g_OpenDeleteConfigPopup = false;
    }
    if (g_OpenCreateConfigPopup) {
        ImGui::OpenPopup("Create Config Profile");
        g_OpenCreateConfigPopup = false;
    }
    if (g_OpenUpdateConfigPopup) {
        ImGui::OpenPopup("Confirm Update Config");
        g_OpenUpdateConfigPopup = false;
    }
    if (g_OpenDirectConnectModal) {
        ImGui::OpenPopup("Direct Connect Modal");
        g_OpenDirectConnectModal = false;
    }
    if (g_OpenServerDetailsModal) {
        ImGui::OpenPopup("Server Details Modal");
        g_OpenServerDetailsModal = false;
    }


    // Style settings for popups
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.06f, 0.06f, 0.08f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    
    // Custom Theme Creator popup modal
    ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Custom Theme Creator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Custom Theme Creator");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        static ImVec4 customColor1 = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
        static ImVec4 customColor2 = ImVec4(0.8f, 0.2f, 1.0f, 1.0f);
        
        ImGui::Text("Accent Color 1");
        ImGui::ColorEdit4("##Color1", (float*)&customColor1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
        
        ImGui::Dummy(ImVec2(0, 5));
        
        ImGui::Text("Accent Color 2");
        ImGui::ColorEdit4("##Color2", (float*)&customColor2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
        
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Apply Theme", ImVec2(140, 30))) {
            g_RGBAccent = false;
            g_AccentColor1 = customColor1;
            g_AccentColor2 = customColor2;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    // Confirm Reset All
    ImGui::SetNextWindowSize(ImVec2(340, 180), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Confirm Reset All", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Reset All Settings");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextWrapped("Are you sure you want to restore all settings to default?");
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Reset", ImVec2(140, 30))) {
            g_NoclipMode = false;
            g_SmartGlance = false;
            g_CustomHitbox = false;
            g_CustomFog = false;
            g_FogDensity = 0.03f;
            g_FogR = 0.5f; g_FogG = 0.1f; g_FogB = 0.8f;
            g_ShowHUD = true;
            g_RubberGauge = true;
            g_SpeedMeter = true;
            g_BrakeMeter = true;
            g_ShowScores = true;
            g_ShowPing = true;
            g_AliveCounter = true;
            g_ShowFastest = true;
            g_ShowTime = false;
            g_24hFormat = false;

            g_FOV = 90.0f;
            g_SpeedGaugeSize = 0.175f;
            g_SpeedGaugeX = 0.0f;
            g_SpeedGaugeY = -0.9f;
            g_BrakeGaugeSize = 0.175f;
            g_BrakeGaugeX = 0.48f;
            g_BrakeGaugeY = -0.9f;
            g_RubberGaugeSize = 0.175f;
            g_RubberGaugeX = -0.48f;
            g_RubberGaugeY = -0.9f;

            g_Sparks = true;
            g_WhiteSparks = false;
            g_Explosions = true;
            g_AlphaBlend = true;
            g_SmoothShading = true;
            g_FloorDetail = 3.0f;

            g_RGBTopBar = true;
            g_RGBAccent = false;

            g_RGBSpeed = 0.15f;
            g_ShowParticles = true;
            g_ParticleType = 0.0f;
            g_GradientAccent = true;
            g_AccentColor = ImVec4(0.478f, 0.345f, 1.0f, 1.0f);
            g_AccentColor1 = ImVec4(0.478f, 0.345f, 1.0f, 1.0f);
            g_AccentColor2 = ImVec4(0.0f, 0.75f, 1.0f, 1.0f);
            g_MenuBgColor = ImVec4(0.07f, 0.07f, 0.08f, 1.0f);
            g_MenuBgAlpha = 0.85f;
            g_InteractiveParticles = true;
            g_ConstellationWeb = true;
            g_ParallaxEffect = true;

            g_NoclipKeybind = 0;
            g_CustomHitboxKeybind = 0;
            g_CustomFogKeybind = 0;
            g_ShowHUDKeybind = 0;
            g_RubberGaugeKeybind = 0;
            g_SpeedMeterKeybind = 0;
            g_BrakeMeterKeybind = 0;
            g_ShowScoresKeybind = 0;
            g_ShowPingKeybind = 0;
            g_AliveCounterKeybind = 0;
            g_ShowFastestKeybind = 0;
            g_ShowTimeKeybind = 0;
            g_24hFormatKeybind = 0;
            g_SparksKeybind = 0;
            g_WhiteSparksKeybind = 0;
            g_ExplosionsKeybind = 0;
            g_AlphaBlendKeybind = 0;
            g_SmoothShadingKeybind = 0;
            g_RGBTopBarKeybind = 0;
            g_RGBAccentKeybind = 0;

            g_ShowParticlesKeybind = 0;
            g_InteractiveParticlesKeybind = 0;
            g_ConstellationWebKeybind = 0;
            g_ParallaxEffectKeybind = 0;
            g_GradientAccentKeybind = 0;
            ModMenu::g_MainMenuActive = true;

            subby_ShowHUD = g_ShowHUD;
            subby_ShowSpeedFastest = g_ShowFastest;
            subby_ShowScore = g_ShowScores;
            subby_ShowAlivePeople = g_AliveCounter;
            subby_ShowPing = g_ShowPing;
            subby_ShowSpeedMeter = g_SpeedMeter;
            subby_ShowBrakeMeter = g_BrakeMeter;
            subby_ShowRubberMeter = g_RubberGauge;
            showTime = g_ShowTime;
            show24hour = g_24hFormat;

            CUSTOM_FOG_R = g_FogR;
            CUSTOM_FOG_G = g_FogG;
            CUSTOM_FOG_B = g_FogB;
            CUSTOM_FOG_DENSITY = g_FogDensity;

            subby_SpeedGaugeSize = g_SpeedGaugeSize;
            subby_SpeedGaugeLocX = g_SpeedGaugeX;
            subby_SpeedGaugeLocY = g_SpeedGaugeY;
            subby_BrakeGaugeSize = g_BrakeGaugeSize;
            subby_BrakeGaugeLocX = g_BrakeGaugeX;
            subby_BrakeGaugeLocY = g_BrakeGaugeY;
            subby_RubberGaugeSize = g_RubberGaugeSize;
            subby_RubberGaugeLocX = g_RubberGaugeX;
            subby_RubberGaugeLocY = g_RubberGaugeY;

            crash_sparks = g_Sparks;
            white_sparks = g_WhiteSparks;
            sg_crashExplosion = g_Explosions;
            sr_alphaBlend = g_AlphaBlend;
            sr_smoothShading = g_SmoothShading;
            sr_floorDetail = (int)( g_FloorDetail + 0.5f );   // rounding, or the top setting can never be reached

            if (lp_cam) {
                lp_cam->smartCustomGlance = g_SmartGlance;
                lp_cam->startFOV = g_FOV;
            rc_CameraFOV( (int)g_FOV );
            }

            prev_ShowHUD = g_ShowHUD;
            prev_ShowFastest = g_ShowFastest;
            prev_ShowScores = g_ShowScores;
            prev_AliveCounter = g_AliveCounter;
            prev_ShowPing = g_ShowPing;
            prev_SpeedMeter = g_SpeedMeter;
            prev_BrakeMeter = g_BrakeMeter;
            prev_RubberGauge = g_RubberGauge;
            prev_ShowTime = g_ShowTime;
            prev_24hFormat = g_24hFormat;
            prev_FogR = g_FogR;
            prev_FogG = g_FogG;
            prev_FogB = g_FogB;
            prev_FogDensity = g_FogDensity;
            prev_NoclipMode = g_NoclipMode;
            prev_SmartGlance = g_SmartGlance;
            prev_FOV = g_FOV;
            prev_SpeedGaugeSize = g_SpeedGaugeSize;
            prev_SpeedGaugeX = g_SpeedGaugeX;
            prev_SpeedGaugeY = g_SpeedGaugeY;
            prev_BrakeGaugeSize = g_BrakeGaugeSize;
            prev_BrakeGaugeX = g_BrakeGaugeX;
            prev_BrakeGaugeY = g_BrakeGaugeY;
            prev_RubberGaugeSize = g_RubberGaugeSize;
            prev_RubberGaugeX = g_RubberGaugeX;
            prev_RubberGaugeY = g_RubberGaugeY;
            prev_Sparks = g_Sparks;
            prev_WhiteSparks = g_WhiteSparks;
            prev_Explosions = g_Explosions;
            prev_AlphaBlend = g_AlphaBlend;
            prev_SmoothShading = g_SmoothShading;
            prev_FloorDetail = g_FloorDetail;

            st_SaveConfig();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Confirm Reset HUD Layout
    ImGui::SetNextWindowSize(ImVec2(340, 180), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Confirm Reset HUD Layout", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Reset HUD Layout");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextWrapped("Are you sure you want to restore the HUD layout configuration to default?");
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Reset", ImVec2(140, 30))) {
            g_FOV = 90.0f;
            g_SpeedGaugeSize = 0.175f;
            g_SpeedGaugeX = 0.0f;
            g_SpeedGaugeY = -0.9f;
            
            g_BrakeGaugeSize = 0.175f;
            g_BrakeGaugeX = 0.48f;
            g_BrakeGaugeY = -0.9f;
            
            g_RubberGaugeSize = 0.175f;
            g_RubberGaugeX = -0.48f;
            g_RubberGaugeY = -0.9f;

            // Also reset toggles to defaults
            g_ShowHUD = true;
            g_RubberGauge = true;
            g_SpeedMeter = true;
            g_BrakeMeter = true;
            g_ShowScores = true;
            g_ShowPing = true;
            g_AliveCounter = true;
            g_ShowFastest = true;
            g_ShowTime = false;
            g_24hFormat = false;

            subby_ShowHUD = g_ShowHUD;
            subby_ShowSpeedFastest = g_ShowFastest;
            subby_ShowScore = g_ShowScores;
            subby_ShowAlivePeople = g_AliveCounter;
            subby_ShowPing = g_ShowPing;
            subby_ShowSpeedMeter = g_SpeedMeter;
            subby_ShowBrakeMeter = g_BrakeMeter;
            subby_ShowRubberMeter = g_RubberGauge;
            showTime = g_ShowTime;
            show24hour = g_24hFormat;

            if (lp_cam) {
                lp_cam->smartCustomGlance = g_SmartGlance;
                lp_cam->startFOV = g_FOV;
            rc_CameraFOV( (int)g_FOV );
            }

            prev_ShowHUD = g_ShowHUD;
            prev_ShowFastest = g_ShowFastest;
            prev_ShowScores = g_ShowScores;
            prev_AliveCounter = g_AliveCounter;
            prev_ShowPing = g_ShowPing;
            prev_SpeedMeter = g_SpeedMeter;
            prev_BrakeMeter = g_BrakeMeter;
            prev_RubberGauge = g_RubberGauge;
            prev_ShowTime = g_ShowTime;
            prev_24hFormat = g_24hFormat;
            prev_FOV = g_FOV;
            prev_SpeedGaugeSize = g_SpeedGaugeSize;
            prev_SpeedGaugeX = g_SpeedGaugeX;
            prev_SpeedGaugeY = g_SpeedGaugeY;
            prev_BrakeGaugeSize = g_BrakeGaugeSize;
            prev_BrakeGaugeX = g_BrakeGaugeX;
            prev_BrakeGaugeY = g_BrakeGaugeY;
            prev_RubberGaugeSize = g_RubberGaugeSize;
            prev_RubberGaugeX = g_RubberGaugeX;
            prev_RubberGaugeY = g_RubberGaugeY;

            st_SaveConfig();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Confirm Apply Config
    ImGui::SetNextWindowSize(ImVec2(340, 180), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Confirm Apply Config", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Apply Config Profile");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        char msg[512];
        snprintf(msg, sizeof(msg), "Are you sure you want to apply the configuration profile '%s'?", g_SelectedConfig);
        ImGui::TextWrapped("%s", msg);
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Apply", ImVec2(140, 30))) {
            tCurrentAccessLevel level( tAccessLevel_Owner, true );
            tString path(g_SelectedConfig);
            std::ifstream s;
            if (tDirectories::Var().Open(s, path)) {
                tConfItemBase::LoadAll(s, true);
                st_SaveConfig();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Confirm Update Config
    ImGui::SetNextWindowSize(ImVec2(340, 180), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Confirm Update Config", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Update Config Profile");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        char msg[512];
        snprintf(msg, sizeof(msg), "Are you sure you want to overwrite the configuration profile '%s' with your current settings?", g_SelectedConfig);
        ImGui::TextWrapped("%s", msg);
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Update", ImVec2(140, 30))) {
            tString path(g_SelectedConfig);
            std::ofstream s;
            if (tDirectories::Var().Open(s, path, std::ios::out, true)) {
                SaveModSettings(s);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Confirm Delete Config
    ImGui::SetNextWindowSize(ImVec2(340, 180), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Confirm Delete Config", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Delete Config Profile");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        char msg[512];
        snprintf(msg, sizeof(msg), "Are you sure you want to delete the configuration profile '%s'? This action cannot be undone.", g_SelectedConfig);
        ImGui::TextWrapped("%s", msg);
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Delete", ImVec2(140, 30))) {
            tString path(g_SelectedConfig);
            tString fullPath = tDirectories::Var().GetWritePath(path);
            std::remove((const char*)fullPath);
            LoadDashboardConfigs();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Create Config Profile
    ImGui::SetNextWindowSize(ImVec2(340, 200), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Create Config Profile", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Create Config Profile");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Enter profile name:");
        ImGui::InputText("##profile_name", g_NewConfigName, sizeof(g_NewConfigName));
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Create", ImVec2(140, 30))) {
            std::string nameStr(g_NewConfigName);
            nameStr.erase(0, nameStr.find_first_not_of(" \t\r\n"));
            nameStr.erase(nameStr.find_last_not_of(" \t\r\n") + 1);
            for (char &c : nameStr) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                    c = '_';
                }
            }
            if (!nameStr.empty()) {
                if (nameStr.length() < 4 || nameStr.substr(nameStr.length() - 4) != ".cfg") {
                    nameStr += ".cfg";
                }
                tString path(nameStr.c_str());
                std::ofstream s;
                if (tDirectories::Var().Open(s, path, std::ios::out, true)) {
                    SaveModSettings(s);
                    LoadDashboardConfigs();
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Direct Connect Modal
    ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Direct Connect Modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Direct Connect");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::Text("IP Address:");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("##DirectIPModal", s_DirectIP, sizeof(s_DirectIP));
        
        ImGui::Dummy(ImVec2(0, 5));
        
        ImGui::Text("Port:");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputInt("##DirectPortModal", &s_DirectPort);
        
        ImGui::Dummy(ImVec2(0, 15));
        
        if (ImGui::Button("Connect", ImVec2(140, 30))) {
            if (s_DirectRedirectServer) {
                delete s_DirectRedirectServer;
            }
            s_DirectRedirectServer = new nServerInfoRedirect(tString(s_DirectIP), s_DirectPort);
            s_PendingConnectServer = s_DirectRedirectServer;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 30))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Server Details Modal
    ImGui::SetNextWindowSize(ImVec2(340, 280), ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Server Details Modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), "Server Details");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        if (s_SelectedServer) {
            std::string sName = TruncateColoredString((const char*)s_SelectedServer->GetName(), 24);
            ImGui::Text("Name: %s", sName.c_str());
            ImGui::Text("Latency: %d ms", (int)(s_SelectedServer->Ping() * 1000.0f));
            ImGui::Text("Players: %d / %d", s_SelectedServer->Users(), s_SelectedServer->MaxUsers());
            
            ImGui::Dummy(ImVec2(0, 15));
            
            if (ImGui::Button("Join Server", ImVec2(300, 35))) {
                s_PendingConnectServer = s_SelectedServer;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::Dummy(ImVec2(0, 5));
            
            bool isFav = gServerFavorites::IsFavorite(s_SelectedServer);
            if (isFav) {
                if (ImGui::Button("Remove from Favorites", ImVec2(300, 30))) {
                    gServerFavorites::RemoveFavorite(s_SelectedServer);
                }
            } else {
                if (ImGui::Button("Add to Favorites", ImVec2(300, 30))) {
                    gServerFavorites::AddFavorite(s_SelectedServer);
                }
            }
        } else if (s_SelectedFavoriteIdx != -1) {
            tString favName, favAddress;
            int favPort;
            if (gServerFavorites::GetFavoriteInfo(s_SelectedFavoriteIdx, favName, favAddress, favPort)) {
                std::string sName = TruncateColoredString((const char*)favName, 24);
                ImGui::Text("Name: %s", sName.c_str());
                ImGui::Text("Address: %s:%d", (const char*)favAddress, favPort);
                
                ImGui::Dummy(ImVec2(0, 15));
                
                if (ImGui::Button("Join Server", ImVec2(300, 35))) {
                    if (s_DirectRedirectServer) {
                        delete s_DirectRedirectServer;
                    }
                    s_DirectRedirectServer = new nServerInfoRedirect(favAddress, favPort);
                    s_PendingConnectServer = s_DirectRedirectServer;
                    ImGui::CloseCurrentPopup();
                }
                
                ImGui::Dummy(ImVec2(0, 5));
                
                if (ImGui::Button("Remove from Favorites", ImVec2(300, 30))) {
                    gServerFavorites::RemoveFavoriteByIndex(s_SelectedFavoriteIdx);
                    s_SelectedFavoriteIdx = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        
        ImGui::Dummy(ImVec2(0, 10));
        if (ImGui::Button("Close", ImVec2(300, 30))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }


    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

bool ModMenu::AnimatedToggle(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

    float height = ImGui::GetFrameHeight();
    float width = height * 2.0f;
    float radius = height * 0.5f;

    ImVec2 pos = window->DC.CursorPos;
    ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + height));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    // Animation state
    static std::map<ImGuiID, float> anim_states;
    if (anim_states.find(id) == anim_states.end()) {
        anim_states[id] = *v ? 1.0f : 0.0f;
    }
    float& anim = anim_states[id];
    float target = *v ? 1.0f : 0.0f;
    anim += (target - anim) * 0.15f;

    // Render frame
    ImDrawList* dl = window->DrawList;
    ImU32 col_bg;
    if (hovered) {
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.12f, 0.12f, 0.15f, 1.0f), ImVec4(0.18f, 0.50f, 0.90f, 1.0f), anim));
    } else {
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.08f, 0.08f, 0.10f, 1.0f), ImVec4(0.12f, 0.40f, 0.80f, 1.0f), anim));
    }

    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), col_bg, radius);
    
    // Render circle
    float knob_radius = radius - 2.0f;
    float knob_x = pos.x + radius + anim * (width - radius * 2.0f);
    dl->AddCircleFilled(ImVec2(knob_x, pos.y + radius), knob_radius, IM_COL32(255, 255, 255, 255));

    if (label_size.x > 0.0f) {
        ImGui::RenderText(ImVec2(pos.x + width + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y), label);
    }

    return pressed;
}

bool ModMenu::AnimatedSlider(const char* label, float* v, float v_min, float v_max) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

    float height = ImGui::GetFrameHeight();
    float width = ImGui::GetContentRegionAvail().x - (label_size.x > 0.0f ? label_size.x + style.ItemInnerSpacing.x : 0.0f);
    if (width < 50.0f) width = 50.0f;

    ImVec2 pos = window->DC.CursorPos;
    ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + height));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    // Slider behavior
    ImRect slider_bb(pos, ImVec2(pos.x + width, pos.y + height));
    ImRect grab_bb;
    bool pressed = ImGui::SliderBehavior(slider_bb, id, ImGuiDataType_Float, v, &v_min, &v_max, "%.2f", ImGuiSliderFlags_None, &grab_bb);
    if (pressed) {
        ImGui::MarkItemEdited(id);
    }

    bool hovered = ImGui::ItemHoverable(slider_bb, id, g.LastItemData.ItemFlags);
    bool held = (g.ActiveId == id);

    // Slider active animation
    static std::map<ImGuiID, float> anim_states;
    if (anim_states.find(id) == anim_states.end()) {
        anim_states[id] = 0.0f;
    }
    float& anim = anim_states[id];
    float target = (hovered || held) ? 1.0f : 0.0f;
    anim += (target - anim) * 0.15f;

    // Render track
    ImDrawList* dl = window->DrawList;
    float track_y = pos.y + height * 0.5f - 2.0f;
    dl->AddRectFilled(ImVec2(pos.x, track_y), ImVec2(pos.x + width, track_y + 4.0f), ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.15f, 1.0f)), 2.0f);

    // Render fill
    float percent = (*v - v_min) / (v_max - v_min);
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;
    dl->AddRectFilled(ImVec2(pos.x, track_y), ImVec2(pos.x + width * percent, track_y + 4.0f), ImGui::GetColorU32(ImLerp(ImVec4(0.35f, 0.40f, 0.50f, 1.0f), ImVec4(0.40f, 0.55f, 0.90f, 1.0f), anim)), 2.0f);

    // Render grab
    float grab_radius = 6.0f + anim * 2.0f;
    dl->AddCircleFilled(ImVec2(pos.x + width * percent, pos.y + height * 0.5f), grab_radius, IM_COL32(255, 255, 255, 255));

    if (label_size.x > 0.0f) {
        char val_str[64];
        snprintf(val_str, sizeof(val_str), "%s: %.2f", label, *v);
        ImGui::RenderText(ImVec2(pos.x + width + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y), val_str);
    }

    return pressed;
}

// Where the pointer actually is, asked of the system rather than pieced
// together from arrivals and departures.
//
// The interface backend keeps the cursor position from motion events and, when
// the window is focused, from the desktop. Both are bookkeeping, and when the
// bookkeeping slips - a leave without its enter, focus changing under a
// fullscreen window, the cursor being hidden and shown around a round - it
// parks the pointer at minus infinity and holds it there. Every click then
// lands on nothing at all, which from the outside looks exactly like a menu
// that answers one press in ten. Asking the system each frame cannot slip.
static void FeedMousePosition()
{
    if (!sr_screen)
        return;

    float mx = 0.0f, my = 0.0f;
    SDL_GetMouseState(&mx, &my);
    ImGui::GetIO().AddMousePosEvent(mx, my);
}

void ModMenu::Render() {
    if (sr_screen) {
        Uint32 flags = SDL_GetWindowFlags(sr_screen);
        if (flags & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN)) {
            return;
        }
    }

    if (!g_Initialized) { Init(); if (!g_Initialized) return; }
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f) {
        // Nothing has sized the frame yet. That used to mean giving up, which
        // left the hud - the chat with it - blank for anyone who got into a
        // game without passing through a menu screen first.
        int w = 0, h = 0;
        if (sr_screen)
            SDL_GetWindowSize(sr_screen, &w, &h);
        if (w <= 0 || h <= 0)
            return;
        io.DisplaySize = ImVec2((float)w, (float)h);
    }


    // Automatically turn off HUD editing if the Mod Menu / Dashboard is not actively showing the Mod Menu tab
    if (isHudEditing) {
        bool isModMenuVisible = false;
        if (g_MenuOpen) {
            isModMenuVisible = true;
        } else if ((ModMenu::g_InGameMenuOpen || g_MainMenuActive) && g_ActiveTab == 4) {
            isModMenuVisible = true;
        }
        if (!isModMenuVisible) {
            isHudEditing = false;
        }
    }

    // Auto Packet Refresh / Anti-Lag check (Anti-freeze/lag feature)
    if (g_AutoPacketRefresh && sg_OnRemoteServer()) {
        double curTime = tSysTimeFloat();
        if (sn_LastPacketTime > 0.0 && (curTime - sn_LastPacketTime) > 1.2) {
            sn_Receive();
            nNetObject::SyncAll();
            sn_SendPlanned();
            sn_LastPacketTime = curTime - 0.7; // Prevent spamming, check again after 0.5s of simulated delay
        }
    }

    if (g_InGameMenuOpen) {
        return;
    }

    if (g_MainMenuActive && !g_CustomMainMenuTempDisabled) {
        return;
    }

    if (isHudEditing) {
        ShowSystemCursor();
        io.MouseDrawCursor = true;
    } else if (g_InGameMenuOpen) {
        ShowSystemCursor();
        io.MouseDrawCursor = false;
    } else if (!g_MenuOpen) {
        SDL_HideCursor();
        io.MouseDrawCursor = false;
    } else {
        SDL_HideCursor();
        io.MouseDrawCursor = true;
    }

    float target = g_MenuOpen ? 1.0f : 0.0f;
    {
        float step = io.DeltaTime > 0.0f ? io.DeltaTime : 1.0f / 60.0f;
        if (step > 0.1f) step = 0.1f;
        g_MenuAlpha += (target - g_MenuAlpha) * (1.0f - expf(-9.75f * step));
    }

    bool needsHud = sg_modClientNameEnabled || sg_modFpsEnabled || 
                    sg_modPingEnabled || sg_modTimeEnabled || 
                    sg_modKeybindsEnabled || 
                    sg_modKDWidgetEnabled || sg_modSpeedometerEnabled || 
                    sg_modRubberMeterEnabled || sg_modBrakeMeterEnabled || 
                    sg_modScoreboardWidgetEnabled || sg_modAliveWidgetEnabled || 
                    sg_modLiveScoreboardEnabled || sg_modZoneTimerEnabled || 
                    sg_modRubberBatteryEnabled || sg_modClassicRubberBatteryEnabled ||
                    sg_modNetHealthEnabled || sg_modMediaWidgetEnabled || isHudEditing ||
                    sg_chatSkin;
    if (!g_MenuOpen && g_MenuAlpha < 0.01f && !needsHud) return;

    ePlayer* lp_cam = ePlayer::PlayerConfig(0);

    // Bidirectional synchronization logic (Pull)
    if (subby_ShowHUD != prev_ShowHUD) { g_ShowHUD = subby_ShowHUD; prev_ShowHUD = subby_ShowHUD; }
    if (subby_ShowSpeedFastest != prev_ShowFastest) { g_ShowFastest = subby_ShowSpeedFastest; prev_ShowFastest = subby_ShowSpeedFastest; }
    if (subby_ShowScore != prev_ShowScores) { g_ShowScores = subby_ShowScore; prev_ShowScores = subby_ShowScore; }
    if (subby_ShowAlivePeople != prev_AliveCounter) { g_AliveCounter = subby_ShowAlivePeople; prev_AliveCounter = subby_ShowAlivePeople; }
    if (subby_ShowPing != prev_ShowPing) { g_ShowPing = subby_ShowPing; prev_ShowPing = subby_ShowPing; }
    if (subby_ShowSpeedMeter != prev_SpeedMeter) { g_SpeedMeter = subby_ShowSpeedMeter; prev_SpeedMeter = subby_ShowSpeedMeter; }
    if (subby_ShowBrakeMeter != prev_BrakeMeter) { g_BrakeMeter = subby_ShowBrakeMeter; prev_BrakeMeter = subby_ShowBrakeMeter; }
    if (subby_ShowRubberMeter != prev_RubberGauge) { g_RubberGauge = subby_ShowRubberMeter; prev_RubberGauge = subby_ShowRubberMeter; }
    if (showTime != prev_ShowTime) { g_ShowTime = showTime; prev_ShowTime = showTime; }
    if (show24hour != prev_24hFormat) { g_24hFormat = show24hour; prev_24hFormat = show24hour; }
    if (CUSTOM_FOG_R != prev_FogR) { g_FogR = CUSTOM_FOG_R; prev_FogR = CUSTOM_FOG_R; }
    if (CUSTOM_FOG_G != prev_FogG) { g_FogG = CUSTOM_FOG_G; prev_FogG = CUSTOM_FOG_G; }
    if (CUSTOM_FOG_B != prev_FogB) { g_FogB = CUSTOM_FOG_B; prev_FogB = CUSTOM_FOG_B; }
    bool activeNoclip = ::sg_IsNoclipActive();
    if (activeNoclip != prev_NoclipMode) { g_NoclipMode = activeNoclip; prev_NoclipMode = activeNoclip; }
    if (sg_noclipCinematic != prev_CleanScreen) { g_CleanScreen = sg_noclipCinematic; prev_CleanScreen = sg_noclipCinematic; }
    if (lp_cam) {
        if (lp_cam->smartCustomGlance != prev_SmartGlance) { g_SmartGlance = lp_cam->smartCustomGlance; prev_SmartGlance = lp_cam->smartCustomGlance; }
        if (lp_cam->startFOV != prev_FOV) { g_FOV = lp_cam->startFOV; prev_FOV = lp_cam->startFOV; }
    }
    if (subby_SpeedGaugeSize != prev_SpeedGaugeSize) { g_SpeedGaugeSize = subby_SpeedGaugeSize; prev_SpeedGaugeSize = subby_SpeedGaugeSize; }
    if (subby_SpeedGaugeLocX != prev_SpeedGaugeX) { g_SpeedGaugeX = subby_SpeedGaugeLocX; prev_SpeedGaugeX = subby_SpeedGaugeLocX; }
    if (subby_SpeedGaugeLocY != prev_SpeedGaugeY) { g_SpeedGaugeY = subby_SpeedGaugeLocY; prev_SpeedGaugeY = subby_SpeedGaugeLocY; }
    if (subby_BrakeGaugeSize != prev_BrakeGaugeSize) { g_BrakeGaugeSize = subby_BrakeGaugeSize; prev_BrakeGaugeSize = subby_BrakeGaugeSize; }
    if (subby_BrakeGaugeLocX != prev_BrakeGaugeX) { g_BrakeGaugeX = subby_BrakeGaugeLocX; prev_BrakeGaugeX = subby_BrakeGaugeLocX; }
    if (subby_BrakeGaugeLocY != prev_BrakeGaugeY) { g_BrakeGaugeY = subby_BrakeGaugeLocY; prev_BrakeGaugeY = subby_BrakeGaugeLocY; }
    if (subby_RubberGaugeSize != prev_RubberGaugeSize) { g_RubberGaugeSize = subby_RubberGaugeSize; prev_RubberGaugeSize = subby_RubberGaugeSize; }
    if (subby_RubberGaugeLocX != prev_RubberGaugeX) { g_RubberGaugeX = subby_RubberGaugeLocX; prev_RubberGaugeX = subby_RubberGaugeLocX; }
    if (subby_RubberGaugeLocY != prev_RubberGaugeY) { g_RubberGaugeY = subby_RubberGaugeLocY; prev_RubberGaugeY = subby_RubberGaugeLocY; }
    if (crash_sparks != prev_Sparks) { g_Sparks = crash_sparks; prev_Sparks = crash_sparks; }
    if (white_sparks != prev_WhiteSparks) { g_WhiteSparks = white_sparks; prev_WhiteSparks = white_sparks; }
    if (sg_crashExplosion != prev_Explosions) { g_Explosions = sg_crashExplosion; prev_Explosions = sg_crashExplosion; }
    if (sr_alphaBlend != prev_AlphaBlend) { g_AlphaBlend = sr_alphaBlend; prev_AlphaBlend = sr_alphaBlend; }
    if (sr_smoothShading != prev_SmoothShading) { g_SmoothShading = sr_smoothShading; prev_SmoothShading = sr_smoothShading; }
    if ((float)sr_floorDetail != prev_FloorDetail) { g_FloorDetail = (float)sr_floorDetail; prev_FloorDetail = (float)sr_floorDetail; }
    
    CheckDisplaySizeRebuild();
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();

    static double lastFrameStamp = 0.0;
    const double frameStamp = tRealSysTimeFloat();
    if (lastFrameStamp > 0.0) {
        double dt = frameStamp - lastFrameStamp;
        if (dt > 0.0) {
            if (dt > 0.25) dt = 0.25;
            io.DeltaTime = (float)dt;
            lastFrameStamp = frameStamp;
        }
    } else {
        lastFrameStamp = frameStamp;
    }

    ImGui::NewFrame();

    // Set for the whole frame rather than pushed and popped around each piece:
    // everything drawn while a section is arriving comes up together.
    ImGui::GetStyle().Alpha = 0.88f + 0.12f * rc_PageEase( g_ActiveTab * 100 + g_ModMenuTab );

    // The editor lays its backdrop down first. Drawing it after the widgets -
    // which is what used to happen on this path only - buried every one of
    // them under it, so the editor came up with nothing to arrange.
    bool const layingOutHud = isHudEditing && g_MenuAlpha > 0.01f;
    if (layingOutHud)
        DrawBackgroundParticles(ImGui::GetBackgroundDrawList(), ImVec2(0, 0), io.DisplaySize, 1.0f);

    HudManager::Update(io.DeltaTime);
    HudManager::Render();

    if (g_MenuAlpha > 0.01f) {
        if (isHudEditing) {
            // backdrop already down, above
        } else {
            if (!g_MainMenuActive) {
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(0, 0),
                    io.DisplaySize,
                    ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.85f * g_MenuAlpha))
                );
            }
            RenderInner();
        }
    }

    PaletteFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void DrawColumnSeparator() {
    ImVec2 min = ImGui::GetCursorScreenPos();
    min.y += 2.0f;
    ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y);
    ImGui::GetWindowDrawList()->AddLine(min, max, ImGui::GetColorU32(ImGuiCol_Separator));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
}

namespace
{
//! Gives a panel its own corner of the id space.
//!
//! Two widgets carrying the same label in one window are the same widget as
//! far as the library is concerned, and they spend the frame taking the
//! keyboard off each other - which is what makes a text field blink and
//! refuse to accept a letter. The panels here grew apart over time and share
//! plenty of labels; scoping each one is one line, where renaming every
//! collision is forty and comes back with the next panel.
struct ImIdScope
{
    explicit ImIdScope( char const * name ) { ImGui::PushID( name ); }
    ~ImIdScope() { ImGui::PopID(); }
};
}

void ModMenu::RenderInner() {
    ImIdScope idScope( "modmenu" );
    ImGuiIO& io = ImGui::GetIO();
    ePlayer* lp_cam = ePlayer::PlayerConfig(0);
    static float g_MenuScale = 0.95f;
    g_MenuScale += (g_MenuOpen ? (1.00f - g_MenuScale) : (0.95f - g_MenuScale)) * io.DeltaTime * 14.0f;

    bool isInline = false;

    ImVec2 targetSize(std::min(1180.0f, io.DisplaySize.x - 40.0f), 700.0f);
    ImVec2 currentSize(targetSize.x * g_MenuScale, targetSize.y * g_MenuScale);
    
    float savedMenuAlpha = g_MenuAlpha;
    
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImVec2 wPos(center.x - currentSize.x * 0.5f, center.y - currentSize.y * 0.5f);

    static bool prevMenuOpen = false;
    if (g_MenuOpen && !prevMenuOpen) {
        ImGui::SetNextWindowFocus();
    }
    prevMenuOpen = g_MenuOpen;
    ImGui::SetNextWindowSize(currentSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(wPos, ImGuiCond_Always);
    
    // Dynamically update background color with opacity
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(g_MenuBgColor.x, g_MenuBgColor.y, g_MenuBgColor.z, g_MenuBgAlpha * g_MenuAlpha);

    ImVec4 savedBorderCol = style.Colors[ImGuiCol_Border];
    float savedBorderSize = style.WindowBorderSize;
    float savedRounding = style.WindowRounding;
    
    if (isInline) {
        style.Colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(GetThemeColor(0.5f));
        style.WindowBorderSize = 1.5f;
        style.WindowRounding = 12.0f; // Match dashboard panel rounding!
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
    if (isInline) {
        flags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_MenuAlpha);
    ImGui::Begin("##ModMenu", nullptr, flags);
    
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    // MAIN SHADOW VIA TEXTURE ARCHITECTURE
    RenderTextureGlow(winPos, winSize, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.3f * g_MenuAlpha)), 16.0f);

    // Dynamic color updates for RGB mode
    float menuTime = (float)ImGui::GetTime();
    float r, g, b;
    ImGui::ColorConvertHSVtoRGB(fmodf(menuTime * g_RGBSpeed, 1.0f), 1.0f, 1.0f, r, g, b);
    
    if (g_RGBAccent) {
        g_AccentColor = ImVec4(r, g, b, 1.0f);
    } else {
        g_AccentColor = g_AccentColor1;
    }

    // BACKGROUND PARTICLES FIELD (Premium design detail)
    DrawBackgroundParticles(dl, winPos, winSize, g_MenuAlpha);

    // Window dragging logic (non-blocking - allows clicks on header buttons/inputs)
    if (!isInline && ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0) && io.MousePos.y < winPos.y + 60.0f) {
        if (!ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) {
            ImVec2 delta = io.MouseDelta;
            ImGui::SetWindowPos(ImVec2(winPos.x + delta.x, winPos.y + delta.y));
        }
    }

    // RGB / GRADIENT TOP BAR - Absolute Coordinates
    ImVec2 p_min = winPos;
    ImVec2 p_max = ImVec2(p_min.x + winSize.x, p_min.y + 4.0f);
    ImU32 topBarColLeft = GetThemeColor(0.0f);
    ImU32 topBarColRight = GetThemeColor(1.0f);
    dl->AddRectFilledMultiColor(p_min, p_max, topBarColLeft, topBarColRight, topBarColRight, topBarColLeft);

    // LEFT SEPARATED SIDEBAR (Figma Style)
    float themeBarW = 70.0f;
    ImRect themeBarRect(ImVec2(winPos.x + 10, winPos.y + 10), ImVec2(winPos.x + themeBarW, winPos.y + winSize.y - 10));
    dl->AddRectFilled(themeBarRect.Min, themeBarRect.Max, ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.06f, g_MenuAlpha)), 12.0f);

    // Theme dots on sidebar (Preserving Gradient Accents)
    ImU32 themeColors1[] = {
        IM_COL32(122, 88, 255, 255), // Indigo
        IM_COL32(50, 150, 250, 255), // Blue
        IM_COL32(50, 200, 100, 255), // Green
        IM_COL32(250, 80, 50, 255),  // Red
        IM_COL32(250, 180, 50, 255)  // Gold
    };
    ImVec4 matchColors1[] = {
        ImVec4(122/255.0f, 88/255.0f, 255/255.0f, 1.0f),
        ImVec4(50/255.0f, 150/255.0f, 250/255.0f, 1.0f),
        ImVec4(50/255.0f, 200/255.0f, 100/255.0f, 1.0f),
        ImVec4(250/255.0f, 80/255.0f, 50/255.0f, 1.0f),
        ImVec4(250/255.0f, 180/255.0f, 50/255.0f, 1.0f)
    };
    ImVec4 matchColors2[] = {
        ImVec4(200/255.0f, 80/255.0f, 250/255.0f, 1.0f), // Purple
        ImVec4(50/255.0f, 230/255.0f, 250/255.0f, 1.0f), // Cyan
        ImVec4(50/255.0f, 250/255.0f, 180/255.0f, 1.0f), // Teal
        ImVec4(250/255.0f, 150/255.0f, 50/255.0f, 1.0f), // Orange
        ImVec4(250/255.0f, 240/255.0f, 50/255.0f, 1.0f)  // Yellow
    };

    for (int i = 0; i < 5; i++) {
        float cy = themeBarRect.Min.y + 20 + i * 55;
        ImVec2 dotMin(themeBarRect.Min.x + 10.0f, cy);
        ImVec2 dotMax(themeBarRect.Min.x + 50.0f, cy + 40.0f);
        
        ImGui::SetCursorScreenPos(dotMin);
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##themedot", ImVec2(40, 40))) {
            g_RGBAccent = false;
            g_AccentColor1 = matchColors1[i];
            g_AccentColor2 = matchColors2[i];
        }
        
        bool isActive = !g_RGBAccent && fabsf(g_AccentColor1.x - matchColors1[i].x) < 0.01f && fabsf(g_AccentColor1.y - matchColors1[i].y) < 0.01f && fabsf(g_AccentColor1.z - matchColors1[i].z) < 0.01f;
        float opacity = isActive ? 1.0f : (ImGui::IsItemHovered() ? 0.75f : 0.35f);
        ImGui::PopID();
        
        ImU32 col1 = ImGui::GetColorU32(ImVec4(matchColors1[i].x, matchColors1[i].y, matchColors1[i].z, opacity * g_MenuAlpha));
        ImU32 col2 = ImGui::GetColorU32(ImVec4(matchColors2[i].x, matchColors2[i].y, matchColors2[i].z, opacity * g_MenuAlpha));
        
        if (isActive) {
            // Draw a beautiful soft colored glow around the rounded square
            RenderSoftGlow(dl, dotMin, ImVec2(40.0f, 40.0f), col1, 10.0f);
        }
        
        // Draw the vertical gradient on the square
        AddRoundedGradientRect(dl, dotMin, dotMax, col1, col2, 10.0f);
        
        if (isActive) {
            // Draw the white outline
            dl->AddRect(dotMin, dotMax, IM_COL32(255, 255, 255, 220), 10.0f, 0, 1.5f);
        }
    }
    
    // Add custom theme plus button on sidebar (matching presets)
    ImGui::SetCursorScreenPos(ImVec2(themeBarRect.Min.x + 10.0f, themeBarRect.Min.y + 305.0f));
    ImGui::PushID("custom_plus_btn");
    
    bool plusClicked = ImGui::InvisibleButton("##plus_btn", ImVec2(40, 40));
    bool plusHovered = ImGui::IsItemHovered();
    bool plusActive = ImGui::IsItemActive();
    
    ImU32 plusBgCol = ImGui::GetColorU32(plusActive ? ImVec4(1, 1, 1, 0.12f) : (plusHovered ? ImVec4(1, 1, 1, 0.08f) : ImVec4(1, 1, 1, 0.03f)));
    ImU32 plusBorderCol = ImGui::GetColorU32(plusHovered ? ImVec4(1, 1, 1, 0.25f) : ImVec4(1, 1, 1, 0.10f));
    ImVec2 plusMin(themeBarRect.Min.x + 10.0f, themeBarRect.Min.y + 305.0f);
    ImVec2 plusMax(themeBarRect.Min.x + 50.0f, themeBarRect.Min.y + 345.0f);
    
    dl->AddRectFilled(plusMin, plusMax, plusBgCol, 10.0f);
    dl->AddRect(plusMin, plusMax, plusBorderCol, 10.0f, 0, 1.0f);
    
    ImU32 plusSignCol = ImGui::GetColorU32(plusHovered ? ImVec4(1, 1, 1, 0.9f) : ImVec4(1, 1, 1, 0.5f));
    dl->AddLine(ImVec2(plusMin.x + 14, plusMin.y + 20), ImVec2(plusMin.x + 26, plusMin.y + 20), plusSignCol, 2.0f);
    dl->AddLine(ImVec2(plusMin.x + 20, plusMin.y + 14), ImVec2(plusMin.x + 20, plusMin.y + 26), plusSignCol, 2.0f);
    
    ImGui::PopID();
    
    if (plusClicked) {
        ImGui::OpenPopup("Custom Theme Creator");
    }

    // Add Reset All Settings button on sidebar at the very bottom (spaced dynamically from plus button)
    ImGui::SetCursorScreenPos(ImVec2(themeBarRect.Min.x + 10.0f, themeBarRect.Max.y - 50.0f));
    ImGui::PushID("reset_all_btn");
    
    bool resetClicked = ImGui::InvisibleButton("##reset_btn", ImVec2(40, 40));
    bool resetHovered = ImGui::IsItemHovered();
    bool resetActive = ImGui::IsItemActive();
    
    ImU32 resetBgCol = ImGui::GetColorU32(resetActive ? ImVec4(1, 0, 0, 0.18f) : (resetHovered ? ImVec4(1, 0, 0, 0.12f) : ImVec4(1, 1, 1, 0.03f)));
    ImU32 resetBorderCol = ImGui::GetColorU32(resetHovered ? ImVec4(1, 0.2f, 0.2f, 0.4f) : ImVec4(1, 1, 1, 0.10f));
    ImVec2 resetMin(themeBarRect.Min.x + 10.0f, themeBarRect.Max.y - 50.0f);
    ImVec2 resetMax(themeBarRect.Min.x + 50.0f, themeBarRect.Max.y - 10.0f);
    
    dl->AddRectFilled(resetMin, resetMax, resetBgCol, 10.0f);
    dl->AddRect(resetMin, resetMax, resetBorderCol, 10.0f, 0, 1.0f);
    
    // Draw Reset Arrow / Refresh icon
    ImU32 resetIconCol = ImGui::GetColorU32(resetHovered ? ImVec4(1.0f, 0.3f, 0.3f, 0.95f) : ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
    ImVec2 resetCenter(resetMin.x + 20.0f, resetMin.y + 20.0f);
    float radius = 7.0f;
    int num_segments = 12;
    for (int i = 0; i < num_segments; i++) {
        float a1 = (float)i / 16.0f * 2.0f * M_PI - M_PI/2.0f;
        float a2 = (float)(i + 1) / 16.0f * 2.0f * M_PI - M_PI/2.0f;
        dl->AddLine(
            ImVec2(resetCenter.x + cosf(a1) * radius, resetCenter.y + sinf(a1) * radius),
            ImVec2(resetCenter.x + cosf(a2) * radius, resetCenter.y + sinf(a2) * radius),
            resetIconCol, 2.0f
        );
    }
    // Arrowhead
    float arrowAngle = 12.0f / 16.0f * 2.0f * M_PI - M_PI/2.0f;
    ImVec2 arrowTip(resetCenter.x + cosf(arrowAngle) * radius, resetCenter.y + sinf(arrowAngle) * radius);
    dl->AddLine(arrowTip, ImVec2(arrowTip.x - 3.0f, arrowTip.y - 1.0f), resetIconCol, 2.0f);
    dl->AddLine(arrowTip, ImVec2(arrowTip.x + 1.0f, arrowTip.y - 3.0f), resetIconCol, 2.0f);
    
    ImGui::PopID();
    
    if (resetHovered) {
        ImGui::SetTooltip("Reset All Settings");
    }
    
    if (resetClicked) {
        ImGui::OpenPopup("Confirm Reset All");
    }

    // HEADER TABS (Centered on Main Area, Capsule Style)
    float mainX = winPos.x + themeBarW + 20;
    float mainW = winSize.x - themeBarW - 30;

    const char* tabs[] = { "Visuals", "HUD", "Client", "Noclip", "Util", "Theme", "Configs", "Player", "Profiles" };
    int numTabs = 9;
    bool showSearch = (winSize.x >= 1200.0f);
    float tabSpacing = 95.0f;
    float tabPillWidth = 90.0f;
    float tabButtonWidth = 85.0f;
    if (winSize.x < 1150.0f) {
        tabSpacing = (mainW - (showSearch ? 320.0f : 120.0f)) / (float)numTabs;
        tabPillWidth = tabSpacing - 5.0f;
        tabButtonWidth = tabSpacing - 10.0f;
    }
    float totalTabsW = numTabs * tabSpacing;
    float tabStartX = mainX + (mainW - (showSearch ? 320.0f : 120.0f) - totalTabsW) * 0.5f;
    if (winSize.x < 1150.0f) {
        tabStartX = mainX;
    }

    // Draw Back / Close Button at top right
    float closeBtnX = winPos.x + winSize.x - 120.0f;
    float closeBtnY = winPos.y + 14.0f;
    ImGui::SetCursorScreenPos(ImVec2(closeBtnX, closeBtnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.15f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.25f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.10f, 0.12f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    if (ImGui::Button("CLOSE", ImVec2(100.0f, 30.0f))) {
        isHudEditing = false;
        if (ModMenu::g_InGameMenuOpen) {
            g_CloseInGameMenuRequested = true;
        } else if (g_MainMenuActive) {
            g_ActiveTab = 0;
            g_DashboardLeftSelected = 0;
        } else {
            Toggle();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    // Draw tabs capsule container background
    ImRect capsuleRect(ImVec2(tabStartX - 10.0f, winPos.y + 12.0f), ImVec2(tabStartX + totalTabsW - 5.0f, winPos.y + 44.0f));
    dl->AddRectFilled(capsuleRect.Min, capsuleRect.Max, ImGui::GetColorU32(ImVec4(0.04f, 0.04f, 0.05f, g_MenuAlpha * 0.9f)), 16.0f);

    // Smoothly slide/lerp Tab Selector Pill
    float targetUnderlineX = tabStartX + g_ModMenuTab * tabSpacing - 5.0f;
    float targetUnderlineWidth = tabPillWidth;
    g_TabUnderlineX += (targetUnderlineX - g_TabUnderlineX) * io.DeltaTime * 14.0f;
    g_TabUnderlineWidth += (targetUnderlineWidth - g_TabUnderlineWidth) * io.DeltaTime * 14.0f;
    
    // Draw the sliding pill selector
    ImU32 selectorCol = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f));
    dl->AddRectFilled(ImVec2(g_TabUnderlineX, winPos.y + 15.0f), ImVec2(g_TabUnderlineX + g_TabUnderlineWidth, winPos.y + 41.0f), selectorCol, 13.0f);

    for (int i = 0; i < numTabs; i++) {
        ImVec2 tabPos = ImVec2(tabStartX + i * tabSpacing, winPos.y + 15.0f);
        ImVec2 tabSize(tabButtonWidth, 30);
        
        ImGui::SetCursorScreenPos(tabPos);
        ImGui::PushID(i);
        if (ImGui::InvisibleButton(tabs[i], tabSize)) {
            g_ModMenuTab = i;
        }
        bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();
        
        bool selected = (g_ModMenuTab == i);
        ImVec4 textCol = selected ? COL_TEXT : COL_TEXT_DIM;
        
        // Layout: Icon + Text
        ImVec2 textSz = ImGui::CalcTextSize(tabs[i]);
        float iconW = 14.0f;
        bool showText = (tabButtonWidth >= 65.0f);
        float spacing = showText ? 6.0f : 0.0f;
        float contentW = showText ? (iconW + spacing + textSz.x) : iconW;
        float startX = tabPos.x + (tabSize.x - contentW) * 0.5f;
        
        // Draw programmatic vector icons inline
        ImVec2 iconCenter(startX + 7.0f, tabPos.y + tabSize.y * 0.5f);
        ImU32 iconColor = ImGui::GetColorU32(ImVec4(textCol.x, textCol.y, textCol.z, g_MenuAlpha));
        
        if (i == 0) { // Visuals: Diamond/Spark
            dl->AddTriangleFilled(ImVec2(iconCenter.x, iconCenter.y - 7.0f), ImVec2(iconCenter.x - 5.0f, iconCenter.y), ImVec2(iconCenter.x + 5.0f, iconCenter.y), iconColor);
            dl->AddTriangleFilled(ImVec2(iconCenter.x, iconCenter.y + 7.0f), ImVec2(iconCenter.x - 5.0f, iconCenter.y), ImVec2(iconCenter.x + 5.0f, iconCenter.y), iconColor);
        } else if (i == 1) { // HUD: Mini Layout Grid
            dl->AddRectFilled(ImVec2(iconCenter.x - 6.0f, iconCenter.y - 5.0f), ImVec2(iconCenter.x + 6.0f, iconCenter.y - 2.0f), iconColor, 1.0f);
            dl->AddRectFilled(ImVec2(iconCenter.x - 6.0f, iconCenter.y - 1.0f), ImVec2(iconCenter.x + 0.0f, iconCenter.y + 2.0f), iconColor, 1.0f);
            dl->AddRectFilled(ImVec2(iconCenter.x + 2.0f, iconCenter.y - 1.0f), ImVec2(iconCenter.x + 6.0f, iconCenter.y + 2.0f), iconColor, 1.0f);
            dl->AddRectFilled(ImVec2(iconCenter.x - 6.0f, iconCenter.y + 3.0f), ImVec2(iconCenter.x + 6.0f, iconCenter.y + 5.0f), iconColor, 1.0f);
        } else if (i == 2) { // Client: Monitor / Screen
            dl->AddRect(ImVec2(iconCenter.x - 7.0f, iconCenter.y - 5.0f), ImVec2(iconCenter.x + 7.0f, iconCenter.y + 3.0f), iconColor, 1.5f, 0, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x - 3.0f, iconCenter.y + 4.0f), ImVec2(iconCenter.x - 1.0f, iconCenter.y + 7.0f), iconColor, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x + 3.0f, iconCenter.y + 4.0f), ImVec2(iconCenter.x + 1.0f, iconCenter.y + 7.0f), iconColor, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x - 4.0f, iconCenter.y + 7.0f), ImVec2(iconCenter.x + 4.0f, iconCenter.y + 7.0f), iconColor, 1.0f);
        } else if (i == 3) { // Noclip: Camera body with a lens
            dl->AddRect(ImVec2(iconCenter.x - 7.0f, iconCenter.y - 4.0f), ImVec2(iconCenter.x + 4.0f, iconCenter.y + 5.0f), iconColor, 2.0f, 0, 1.2f);
            dl->AddTriangleFilled(ImVec2(iconCenter.x + 4.0f, iconCenter.y - 1.0f), ImVec2(iconCenter.x + 8.0f, iconCenter.y - 4.0f), ImVec2(iconCenter.x + 8.0f, iconCenter.y + 3.0f), iconColor);
            dl->AddCircle(ImVec2(iconCenter.x - 2.0f, iconCenter.y + 0.5f), 2.5f, iconColor, 10, 1.2f);
        } else if (i == 4) { // Util: Gear / Wrench
            dl->AddCircle(iconCenter, 4.0f, iconColor, 12, 1.5f);
            for (int a = 0; a < 8; a++) {
                float angle = a * (IM_PI / 4.0f);
                dl->AddLine(
                    ImVec2(iconCenter.x + cosf(angle) * 4.0f, iconCenter.y + sinf(angle) * 4.0f),
                    ImVec2(iconCenter.x + cosf(angle) * 7.0f, iconCenter.y + sinf(angle) * 7.0f),
                    iconColor, 1.5f
                );
            }
        } else if (i == 5) { // Theme: Paintbrush / Palette
            dl->AddCircleFilled(iconCenter, 6.0f, iconColor, 12);
            dl->AddCircleFilled(ImVec2(iconCenter.x - 2.0f, iconCenter.y - 2.0f), 1.2f, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.12f, g_MenuAlpha)));
            dl->AddCircleFilled(ImVec2(iconCenter.x + 2.0f, iconCenter.y - 2.0f), 1.2f, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.12f, g_MenuAlpha)));
            dl->AddCircleFilled(ImVec2(iconCenter.x - 2.0f, iconCenter.y + 2.0f), 1.2f, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.12f, g_MenuAlpha)));
            dl->AddCircleFilled(ImVec2(iconCenter.x + 2.0f, iconCenter.y + 2.0f), 1.2f, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.12f, g_MenuAlpha)));
        } else if (i == 6) { // Configs: Folder / Document
            dl->AddRect(ImVec2(iconCenter.x - 7.0f, iconCenter.y - 4.0f), ImVec2(iconCenter.x + 7.0f, iconCenter.y + 5.0f), iconColor, 1.0f, 0, 1.5f);
            dl->AddLine(ImVec2(iconCenter.x - 7.0f, iconCenter.y - 1.0f), ImVec2(iconCenter.x + 7.0f, iconCenter.y - 1.0f), iconColor, 1.0f);
            dl->AddRectFilled(ImVec2(iconCenter.x - 5.0f, iconCenter.y - 6.0f), ImVec2(iconCenter.x - 1.0f, iconCenter.y - 4.0f), iconColor, 1.0f);
        } else if (i == 7) { // Player: User Avatar
            dl->AddCircleFilled(ImVec2(iconCenter.x, iconCenter.y - 3.0f), 3.0f, iconColor, 10);
            dl->AddRectFilled(ImVec2(iconCenter.x - 6.0f, iconCenter.y + 2.0f), ImVec2(iconCenter.x + 6.0f, iconCenter.y + 6.0f), iconColor, 2.0f);
        } else if (i == 8) { // Profiles: Id Card / List
            dl->AddRect(ImVec2(iconCenter.x - 7.0f, iconCenter.y - 5.0f), ImVec2(iconCenter.x + 7.0f, iconCenter.y + 5.0f), iconColor, 1.0f, 0, 1.5f);
            dl->AddLine(ImVec2(iconCenter.x - 4.0f, iconCenter.y - 2.0f), ImVec2(iconCenter.x + 4.0f, iconCenter.y - 2.0f), iconColor, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x - 4.0f, iconCenter.y + 1.0f), ImVec2(iconCenter.x + 4.0f, iconCenter.y + 1.0f), iconColor, 1.0f);
        }
        
        if (showText) {
            ImVec2 textPos(startX + iconW + spacing, tabPos.y + (tabSize.y - textSz.y) * 0.5f);
            dl->AddText(textPos, ImGui::GetColorU32(ImVec4(textCol.x, textCol.y, textCol.z, g_MenuAlpha)), tabs[i]);
        } else if (hovered) {
            ImGui::SetTooltip("%s", tabs[i]);
        }
    }

    // The search field floats just above the panel rather than living in the
    // header, so it reads as its own control and stays put no matter how the
    // tab strip is squeezed.
    static char searchBuf[64] = "";
    
    DrawFloatingSearch(winPos, winSize, searchBuf, sizeof(searchBuf));

    // GRID MATH (Figma Layout) - with margins to prevent card glow clipping
    float startY = winPos.y + 80.0f;
    float gridMarginX = 12.0f;
    float gridMarginY = 12.0f;
    float scrollbarPadding = 12.0f;
    int numCols = 3;
    if (winSize.x < 800.0f) {
        numCols = 1;
    } else if (winSize.x < 1100.0f) {
        numCols = 2;
    }
    float spacingY = 16.0f;
    float spacingX = 16.0f;
    float columnW = (mainW - scrollbarPadding - 2.0f * gridMarginX - (numCols - 1) * spacingX) / (float)numCols;
    float itemH = 90.0f;

    // Tab content transition animation
    static int prevTab = g_ModMenuTab;
    static float tabTransitionT = 1.0f;
    if (prevTab != g_ModMenuTab) {
        tabTransitionT = 0.0f;
        prevTab = g_ModMenuTab;
    }
    tabTransitionT += (1.0f - tabTransitionT) * io.DeltaTime * 12.0f;
    float slideOffset = (1.0f - tabTransitionT) * 40.0f;
    float alphaMultiplier = tabTransitionT;

    // Custom scrollbar style matching active theme
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.25f * g_MenuAlpha));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.45f * g_MenuAlpha));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.65f * g_MenuAlpha));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 10.0f);

    float childH = winSize.y - 170.0f;
    ImGui::SetCursorScreenPos(ImVec2(mainX, startY));
    ImGui::BeginChild("##grid_scroll", ImVec2(mainW, childH), false, ImGuiWindowFlags_NoScrollWithMouse);
    // Keyboard navigation of the grid wants the focus, and takes it back every
    // frame it does not have it. That is fine until something else needs the
    // keyboard: click the search box and this fires on the very next frame,
    // the field loses focus, and nothing can be typed into it. Anything asking
    // for text wins - it is the only thing that could be asking.
    if (isInline && g_DashboardActiveCol == 2 &&
        !ImGui::GetIO().WantTextInput &&
        !ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        ImGui::SetWindowFocus();
    }

    // Smooth scroll interpolation logic
    static float currentScrollY = 0.0f;
    static float targetScrollY = -1.0f;
    static float prevNativeScrollY = -1.0f;
    static int prevActiveTab = -1;

    if (g_ModMenuTab != prevActiveTab) {
        targetScrollY = 0.0f;
        currentScrollY = targetScrollY;
        ImGui::SetScrollY(targetScrollY);
        prevActiveTab = g_ModMenuTab;
    }

    float nativeScrollY = ImGui::GetScrollY();
    if (targetScrollY < 0.0f) {
        targetScrollY = nativeScrollY;
        currentScrollY = nativeScrollY;
        prevNativeScrollY = nativeScrollY;
    }

    if (nativeScrollY != prevNativeScrollY && fabs(nativeScrollY - currentScrollY) > 1.5f) {
        targetScrollY = nativeScrollY;
        currentScrollY = nativeScrollY;
    }

    if (ImGui::IsWindowHovered()) {
        float wheel = io.MouseWheel;
        if (wheel != 0.0f) {
            targetScrollY -= wheel * 140.0f; // Fast, premium scrolling speed
            float maxScroll = ImGui::GetScrollMaxY();
            if (targetScrollY < 0.0f) targetScrollY = 0.0f;
            if (targetScrollY > maxScroll) targetScrollY = maxScroll;
        }
    }

    // Snappy and smooth framerate-independent lerp
    {
        currentScrollY = ImLerp(currentScrollY, targetScrollY, io.DeltaTime * 24.0f);
        if (fabs(currentScrollY - targetScrollY) < 0.2f) {
            currentScrollY = targetScrollY;
        }
    }

    ImGui::SetScrollY(currentScrollY);
    prevNativeScrollY = ImGui::GetScrollY();
    float childScrollY = currentScrollY;

#define GET_CELL_POS(col, row) ImVec2(mainX + gridMarginX + (col) * (columnW + spacingX) + slideOffset, startY + gridMarginY + (row) * (itemH + spacingY) - childScrollY)

    // Grid rendering logic
    int cellIdx = 0;
    auto renderItem = [&](const char* title, const char* desc, bool* v, int* keybind = nullptr) {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, desc, searchBuf)) return;
        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        SettingItemAbsolute(GET_CELL_POS(colIdx, rowIdx), ImVec2(columnW, itemH), title, desc, v, keybind, alphaMultiplier);
        cellIdx++;
    };
    
    auto renderNote = [&](const char* title, const char* line) {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, line, searchBuf)) return;

        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        ImVec2 at = GET_CELL_POS(colIdx, rowIdx);
        float const a = g_MenuAlpha * alphaMultiplier;

        // The list of the scrolling region, not the one of the window around
        // it: only that one is clipped to the region, and everything drawn on
        // the outer list keeps going over the tabs once the page is scrolled.
        ImDrawList* into = ImGui::GetWindowDrawList();

        into->AddRectFilled(at, ImVec2(at.x + columnW, at.y + itemH),
                            ImGui::GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, a)), 24.0f);
        into->AddRect(at, ImVec2(at.x + columnW, at.y + itemH),
                      ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.17f, a)), 24.0f);
        into->AddText(ImVec2(at.x + 18.0f, at.y + 14.0f),
                      ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, a)), title);
        into->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f,
                      ImVec2(at.x + 18.0f, at.y + 36.0f),
                      ImGui::GetColorU32(ImVec4(0.55f, 0.57f, 0.63f, a)), line);

        cellIdx++;
    };

    auto renderKey = [&](const char* title, int* key) {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, "keyboard key", searchBuf)) return;
        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        KeyItemAbsolute(GET_CELL_POS(colIdx, rowIdx), ImVec2(columnW, itemH), title, key, alphaMultiplier);
        cellIdx++;
    };

    auto renderSlider = [&](const char* title, const char* desc, float* v, float v_min, float v_max, const char* format) {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, desc, searchBuf)) return;
        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        SliderItemAbsolute(GET_CELL_POS(colIdx, rowIdx), ImVec2(columnW, itemH), title, desc, v, v_min, v_max, format, alphaMultiplier);
        cellIdx++;
    };
    
    auto renderRealSlider = [&](const char* title, const char* desc, REAL* v, float v_min, float v_max, const char* format) {
        float fVal = (float)*v;
        renderSlider(title, desc, &fVal, v_min, v_max, format);
        *v = (REAL)fVal;
    };

    auto renderIntSlider = [&](const char* title, const char* desc, int* v, float v_min, float v_max, const char* format) {
        float fVal = (float)*v;
        renderSlider(title, desc, &fVal, v_min, v_max, format);
        *v = (int)fVal;
    };
    
    // A heading between runs of cards.
    //
    // The tab had grown to seventy odd entries in one unbroken grid, which is
    // fine for the person who put them in that order and no use to anybody
    // else. This finishes whatever row is open, gives the heading a line of
    // its own, and lets the cards carry on underneath.
    //
    // With the search box in use it draws nothing: the list is then a set of
    // matches from all over, and headings would claim things they do not own.
    auto renderSection = [&](const char* title) {
        if (searchBuf[0] != '\0')
            return;

        if (cellIdx % numCols != 0)
            cellIdx += numCols - (cellIdx % numCols);

        ImVec2 pos = GET_CELL_POS(0, cellIdx / numCols);
        float const y = pos.y + itemH * 0.55f;

        ImU32 const label = ImGui::GetColorU32(ImVec4(0.55f, 0.58f, 0.66f, g_MenuAlpha * alphaMultiplier));
        ImU32 const rule  = ImGui::GetColorU32(ImVec4(0.22f, 0.23f, 0.27f, g_MenuAlpha * alphaMultiplier * 0.8f));

        ImDrawList* into = ImGui::GetWindowDrawList();
        into->AddText(ImVec2(pos.x + 4.0f, y), label, title);

        float const textW = ImGui::CalcTextSize(title).x;
        float const from = pos.x + 4.0f + textW + 12.0f;
        float const to = pos.x + numCols * (columnW + spacingX) - spacingX;
        if (to > from)
            into->AddLine(ImVec2(from, y + ImGui::GetFontSize() * 0.5f), ImVec2(to, y + ImGui::GetFontSize() * 0.5f), rule, 1.0f);

        cellIdx += numCols;
    };

    auto renderColor = [&](const char* title, const char* desc, ImVec4* colVal) {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, desc, searchBuf)) return;
        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        ColorItemAbsolute(GET_CELL_POS(colIdx, rowIdx), ImVec2(columnW, itemH), title, desc, colVal, alphaMultiplier);
        cellIdx++;
    };

    auto renderButton = [&](const char* title, const char* desc) -> bool {
        if (searchBuf[0] != '\0' && !ItemMatchesSearch(title, desc, searchBuf)) return false;
        int colIdx = cellIdx % numCols;
        int rowIdx = cellIdx / numCols;
        bool clicked = ButtonItemAbsolute(GET_CELL_POS(colIdx, rowIdx), ImVec2(columnW, itemH), title, desc, alphaMultiplier);
        cellIdx++;
        return clicked;
    };

        // Tab-specific screens
        // a query reaches across the tabs; without this only the open one
        // is ever built, so most of the menu could not be found at all
        const bool searchingAll = (searchBuf[0] != '\0');

        if (searchingAll || g_ModMenuTab == 0) { // Visuals
            renderItem("Master Enhanced Visuals", "Toggle ALL modern graphics, bloom, neon grid & custom shaders in 1 click (Hotkey: F9)", &g_EnhancedGraphicsMode, &g_EnhancedGraphicsKeybind);

            // The two halves of the enhanced look, asked for on their own.
            // The master switch above still turns both on; these let somebody
            // take one without the other, or without the bloom and the shaders
            // their machine cannot afford.
            renderSection("ARENA");
            renderItem("Neon Floor", "The dark floor and its grid, without the rest of enhanced mode", &sg_neonFloor, &g_NeonFloorKeybind);
            {
                renderItem("Custom Score Table", "Draw the Tab table in the client's own style instead of the built-in one", &sg_customScoreboard);
                if (sg_customScoreboard)
                    renderRealSlider("Score Table Opacity", "How solid it is - it is glanced at mid-round, so leave room to see past it", &sg_scoreboardOpacity, 0.05f, 1.0f, "%.2f");
            }

            renderItem("Neon Arena Walls", "The lit perimeter walls, without the rest of enhanced mode", &sg_neonWalls, &g_NeonWallsKeybind);

            // The sumo and fortress zones, made to look lit
            {
                renderItem("Zone Glow", "Make the turning zone edges look lit instead of merely drawn", &sg_zoneGlow, &g_ZoneGlowKeybind);
                if (sg_zoneGlow) {
                    renderRealSlider("Zone Glow Strength", "How far past white the zone is drawn, which is what the bloom picks up", &sg_zoneGlowStrength, 0.0f, 6.0f, "%.2f");
                    renderItem("Own Zone Burns White", "Your team's zone glows white in the middle, so whose it is needs no reading", &sg_zoneGlowOwnWhite, &g_ZoneGlowOwnKeybind);
                }
            }

            // Stadium masts around the rim. They stand on the arena's own
            // corners and throw far enough to reach the middle, so the rest of
            // these only decide how hard and where.
            {
                renderSection("FLOODLIGHTS");
                renderItem("Floodlights", "Stadium masts at the corners of the arena, throwing light across the floor", &sg_floodlights, &g_FloodlightsKeybind);

                if (sg_floodlights) {
                    renderIntSlider("Floodlight Aim",
                                    "Where the masts point. Zones and bases only exist in the modes that have them",
                                    &sg_floodlightMode, 0.0f, 2.0f, "FLOODLIGHT_MODE_FORMAT");
                    renderRealSlider("Floodlight Power", "How hard they burn", &sg_floodlightBright, 0.0f, 2.5f, "%.2f");
                    renderRealSlider("Floodlight Height", "How tall the masts stand", &sg_floodlightHeight, 20.0f, 160.0f, "%.0f");
                    renderRealSlider("Floodlight Spread", "How wide the light lands where it arrives", &sg_floodlightReach, 20.0f, 160.0f, "%.0f");

                    renderItem("Floodlight Beams", "The shaft of light between the mast and the floor", &sg_floodlightShafts, &g_FloodlightBeamsKeybind);

                    ImVec4 lampCol((float)sg_floodR, (float)sg_floodG, (float)sg_floodB, 1.0f);
                    renderColor("Floodlight Colour", "Warm white by default, so a lamp does not vanish into a cold arena", &lampCol);
                    sg_floodR = lampCol.x; sg_floodG = lampCol.y; sg_floodB = lampCol.z;
                }
            }

            // Only worth showing while the floor they belong to is being
            // drawn. Three colours the arena is built out of, which used to be
            // numbers in the drawing code and are now somebody's choice.
            //
            // The picker works in one colour and the drawing reads three
            // numbers, so each is carried across as it is shown rather than
            // kept in two places at once.
            // Texture upscaling. Changing it has to reach textures that are
            // already on the card, so the whole set is dropped and reloaded -
            // which is a stutter, and the reason this is not applied while the
            // slider is being dragged.
            {
                int factor = rc_UpscaleFactorSetting();
                int wasFactor = factor;
                renderSection("TEXTURES");
                renderIntSlider("Texture Upscaling",
                                "Rebuild the game's art larger before it reaches the card, so close-up "
                                "walls and text stop being staircases. Costs texture memory, not framerate",
                                &factor, 1.0f, 4.0f, "UPSCALE_FACTOR_FORMAT");
                if (factor != wasFactor) rc_UpscaleFactorSet(factor);

                if (factor > 1) {
                    int mode = rc_UpscaleModeSetting();
                    int wasMode = mode;
                    renderIntSlider("Upscaling Style",
                                    "Per texture measures each one and decides. Hard edges suits flat art "
                                    "and text; smooth suits skies, flames and anything painted",
                                    &mode, 0.0f, 2.0f, "UPSCALE_STYLE_FORMAT");
                    if (mode != wasMode) rc_UpscaleModeSet(mode);
                }

                if (!ImGui::IsAnyItemActive() && rc_UpscaleDirty()) {
                    rITexture::UnloadAll();
                    rITexture::LoadAll();
                    rc_UpscaleClean();
                }
            }

            if (g_EnhancedGraphicsMode || sg_neonFloor) {
                ImVec4 floorCol((float)sg_floorR, (float)sg_floorG, (float)sg_floorB, 1.0f);
                renderColor("Floor Color", "The glass everything else sits on", &floorCol);
                sg_floorR = floorCol.x; sg_floorG = floorCol.y; sg_floorB = floorCol.z;

                ImVec4 gridCol((float)sg_gridR, (float)sg_gridG, (float)sg_gridB, 1.0f);
                renderColor("Grid Color", "The grid, and the crosses where it meets itself", &gridCol);
                sg_gridR = gridCol.x; sg_gridG = gridCol.y; sg_gridB = gridCol.z;

                ImVec4 majorCol((float)sg_majorR, (float)sg_majorG, (float)sg_majorB, 1.0f);
                renderColor("Grid Accent Color", "The heavier line every fifth square", &majorCol);
                sg_majorR = majorCol.x; sg_majorG = majorCol.y; sg_majorB = majorCol.z;
            }

            // The arena's own walls. These belong to the map rather than to a
            // player, so painting them is nobody's advantage.
            if (g_EnhancedGraphicsMode || sg_neonWalls) {
                renderSection("COLOURS");
                renderItem("Paint Arena Walls", "Colour the walls that hold the grid in", &sg_rimPainted);

                if (sg_rimPainted) {
                    float howMany = (float)sg_rimColours;
                    renderSlider("Arena Wall Colors", "One colour, or two and three blended along their length", &howMany, 1.0f, 3.0f, "%.0f");
                    sg_rimColours = (int)(howMany + 0.5f);

                    ImVec4 r1((float)sg_rim1R, (float)sg_rim1G, (float)sg_rim1B, 1.0f);
                    renderColor("Arena Wall Color 1", "Where the blend begins", &r1);
                    sg_rim1R = r1.x; sg_rim1G = r1.y; sg_rim1B = r1.z;

                    if (sg_rimColours >= 2) {
                        ImVec4 r2((float)sg_rim2R, (float)sg_rim2G, (float)sg_rim2B, 1.0f);
                        renderColor("Arena Wall Color 2", sg_rimColours >= 3 ? "Through the middle" : "Where it ends", &r2);
                        sg_rim2R = r2.x; sg_rim2G = r2.y; sg_rim2B = r2.z;
                    }

                    if (sg_rimColours >= 3) {
                        ImVec4 r3((float)sg_rim3R, (float)sg_rim3G, (float)sg_rim3B, 1.0f);
                        renderColor("Arena Wall Color 3", "Where it ends", &r3);
                        sg_rim3R = r3.x; sg_rim3G = r3.y; sg_rim3B = r3.z;
                    }
                }
            }

            // Your own wall, and only yours: repainting everybody's would take
            // away the one thing wall colour is for, which is knowing whose
            // wall is in front of you.
            renderItem("Paint My Trail", "Colour your own wall instead of using your player colour", &sg_trailPainted);

            if (sg_trailPainted) {
                float howMany = (float)sg_trailColours;
                renderSlider("Trail Colors", "One colour, or two and three blended along its length", &howMany, 1.0f, 3.0f, "%.0f");
                sg_trailColours = (int)(howMany + 0.5f);

                ImVec4 t1((float)sg_trail1R, (float)sg_trail1G, (float)sg_trail1B, 1.0f);
                renderColor("Trail Color 1", "At the far end of the wall", &t1);
                sg_trail1R = t1.x; sg_trail1G = t1.y; sg_trail1B = t1.z;

                if (sg_trailColours >= 2) {
                    ImVec4 t2((float)sg_trail2R, (float)sg_trail2G, (float)sg_trail2B, 1.0f);
                    renderColor("Trail Color 2", sg_trailColours >= 3 ? "Through the middle" : "Nearest the rider", &t2);
                    sg_trail2R = t2.x; sg_trail2G = t2.y; sg_trail2B = t2.z;
                }

                if (sg_trailColours >= 3) {
                    ImVec4 t3((float)sg_trail3R, (float)sg_trail3G, (float)sg_trail3B, 1.0f);
                    renderColor("Trail Color 3", "Nearest the rider", &t3);
                    sg_trail3R = t3.x; sg_trail3G = t3.y; sg_trail3B = t3.z;
                }
            }
            renderItem("Custom Hitboxes", "Displays wireframe boundaries of objects", &g_CustomHitbox, &g_CustomHitboxKeybind);
            // The gauge draws in the arena over other riders, so it belongs
            // with the rest of what is drawn there.
            renderItem("3D Rubber Gauge", "Show how much rubber each rider has left, in the world above their machine", &sg_modRubberGaugeEnabled);
            renderSection("ATMOSPHERE");
            renderItem("Custom Fog", "Toggle customizable atmospheric world fog", &g_CustomFog, &g_CustomFogKeybind);
            // kept off renderColor because the colour lives in three separate
            // floats, so it has to answer to the filter on its own
            if (searchBuf[0] == '\0' ||
                ItemMatchesSearch("Fog Color", "Customizes the color of atmospheric fog", searchBuf)) {
                ImVec4 fogCol(g_FogR, g_FogG, g_FogB, 1.0f);
                if (ColorItemAbsolute(GET_CELL_POS(cellIdx % numCols, cellIdx / numCols), ImVec2(columnW, itemH), "Fog Color", "Customizes the color of atmospheric fog", &fogCol, alphaMultiplier)) {
                    g_FogR = fogCol.x;
                    g_FogG = fogCol.y;
                    g_FogB = fogCol.z;
                }
                cellIdx++;
            }
            renderSlider("Fog Density", "Sets the thickness of the world fog", &g_FogDensity, 0.0f, 0.1f, "%.4f");

            renderSection("ARENA EXTRAS");
            renderItem("Zone Center Indicator", "Show a soft beacon and crosshair at the zone center", &sg_drawZoneCenter);
            renderItem("Corpse Fade Timer", "Show a timer above dead players until their corpse disappears", &sg_corpseTimerEnabled);
            renderItem("Corpse Timer Override", "Override server corpse delay with custom duration", &sg_corpseTimerOverride);
            renderRealSlider("Corpse Fade Duration", "Custom duration in seconds for corpse fade", &sg_corpseTimerDuration, 1.0f, 30.0f, "%.1f");
            renderRealSlider("Player Name Size", "Scale factor for other players' names displayed above their cycles", &sg_modNameSizeScale, 0.2f, 5.0f, "%.2f");
            renderItem("Plain Cycle Names", "Draw the name above each cycle in plain white instead of the player's colours", &sg_modPlainNames);
            {
                renderItem("Sharp Cycle Names", "Draw names with the interface font instead of the engine's bitmap sheet - same place and size, even letters", &sg_sharpNames);
            }
            renderItem("Name Plate Background", "Draw a bright box behind dark player names instead of brightening the name itself", &sr_darkTextPlate);
            renderIntSlider("Corpse Trail Style", "0: Legacy, 1: Blinking Warning, 2: Gradual Fade & Shrink", &sg_corpseTrailStyle, 0.0f, 2.0f, "CORPSE_TRAIL_STYLE_FORMAT");
            renderItem("Alpha Blending", "Enables smooth transparency blending", &g_AlphaBlend, &g_AlphaBlendKeybind);
            renderItem("Smooth Shading", "Enables smooth vertex normals shading", &g_SmoothShading, &g_SmoothShadingKeybind);
            renderSlider("Floor Detail", "Sets floor grid/texture complexity (0-3)", &g_FloorDetail, 0.0f, 3.0f, "%.0f");
            {
                renderItem("Custom Floor", "Our own panelled ground in the plain look. Master Enhanced keeps its flat floor and neon grid either way", &sg_floorTexture);
                renderRealSlider("Custom Floor Light", "How brightly the panels are lit", &sg_floorTextureLight, 0.0f, 2.0f, "%.2f");
                renderRealSlider("Custom Floor Scale", "How many grid squares one tile of it covers", &sg_floorTextureScale, 1.0f, 40.0f, "%.0f");
            }
            
            

            renderSection("BLOOM AND POST");
            renderItem("FBO Post-Processing", "The master switch for bloom. Works whether or not enhanced graphics are on", &sg_postProcessingEnabled);
            renderRealSlider("Vignette Intensity", "Darkening intensity of cinematic screen edges", &sg_vignetteIntensity, 0.0f, 1.0f, "%.2f");
            renderRealSlider("Vignette Radius", "Inner radius where vignette gradient begins", &sg_vignetteRadius, 0.2f, 1.5f, "%.2f");
            renderRealSlider("Vignette Softness", "Feathering softness of edge vignette blur", &sg_vignetteSoftness, 0.05f, 1.0f, "%.2f");
            renderRealSlider("Bloom Threshold", "How bright a pixel must be before it starts to glow", &sg_bloomThreshold, 0.05f, 1.50f, "%.2f");
            renderRealSlider("Bloom Strength", "How much of the glow is added back over the frame", &sg_bloomStrength, 0.0f, 8.0f, "%.2f");
            renderRealSlider("Bloom Spread", "How far the light bleeds from whatever is glowing", &sg_bloomSpread, 0.25f, 3.0f, "%.2f");
            renderItem("Cycle & Trail Bloom", "One switch for the bikes and their trails together", &sg_cycleBloom);
            if (sg_cycleBloom)
            {
                renderRealSlider("Cycle Bloom Power", "How hard the bikes themselves burn", &sg_cycleBloomPower, 1.0f, 6.0f, "%.2f");
                renderRealSlider("Trail Bloom Power", "How hard the trails burn, kept apart because their shader already brightens them", &sg_trailBloomPower, 1.0f, 4.0f, "%.2f");
            }
            renderSection("PARTICLES");
            renderItem("Particle Engine", "Enable custom cycle/ambient particles", &sg_modParticleSystemEnabled);
            renderIntSlider("Death Burst Count", "Number of particles spawned on cycle death", &sg_modDeathParticlesCount, 0.0f, 500.0f, "%.0f");
            renderItem("Camera Shake", "Enable camera shake when a cycle crashes", &sg_modScreenShakeEnabled);
            renderRealSlider("Shake Intensity", "Intensity offset of screen shaking", &sg_modScreenShakeIntensity, 0.0f, 2.0f, "%.2f");
            renderItem("Ambient Dust", "Enable glowing ambient dust particles rising from floor", &sg_modAmbientParticlesEnabled);
            renderIntSlider("Ambient Particles Spawn Mode", "Spawn mode for ambient floor particles", &sg_modAmbientParticlesMode, 0.0f, 1.0f, "AMBIENT_PARTICLES_MODE_FORMAT");
            renderIntSlider("Ambient Particles Min Limit", "Minimum active ambient floor particles count", &sg_modAmbientParticlesMin, 0.0f, 500.0f, "%.0f");
            renderIntSlider("Ambient Particles Max Limit", "Maximum active ambient floor particles count", &sg_modAmbientParticlesMax, 0.0f, 1000.0f, "%.0f");
            renderSection("TRAILS");
            renderItem("Vanilla Wall Shading", "Draw walls exactly as a plain build does: colour times texture, the texture's own transparency, no added brightness or gradient. For packs made for a normal client", &sg_wallFaithful);
            renderItem("Trail Gradient", "Fades trail opacity from top (opaque) to bottom (transparent)", &sg_modTrailGradientEnabled);
            renderRealSlider("Trail Alpha Top", "Opacity level near the cycle/trail top", &sg_modTrailAlphaTop, 0.0f, 1.0f, "%.2f");
            renderRealSlider("Trail Alpha Bottom", "Opacity level near the grid floor", &sg_modTrailAlphaBottom, 0.0f, 1.0f, "%.2f");
            renderRealSlider("Trail Height", "Height scale of all cycle trails (1.0 = normal, 0.1 = pixel line)", &sg_modWallHeightMultiplier, 0.1f, 1.0f, "%.2f");
            renderRealSlider("Trail Texture Stretch", "How much wall one turn of the texture covers - raise it to read the picture instead of hatching", &sg_wallTextureStretch, 1.0f, 40.0f, "%.1f");
            renderItem("Instanced Trails", "Enable next-gen instanced rendering for trail walls (Safe Fallback if disabled)", &cfg_EnableInstancing);
            bool prevMSAA = cfg_MSAA;
            renderItem("Anti-Aliasing (MSAA)", "Enables 8x hardware multi-sample anti-aliasing", &cfg_MSAA);
            if (cfg_MSAA != prevMSAA) {
                st_SaveConfig();
            }

            {
                renderSection("CYCLE COLOURS");
                renderItem("Cycle Body Color", "Recolour the machine on its own, leaving the wall its own colour", &sg_overrideCycleColor);
                if (sg_overrideCycleColor) {
                    ImVec4 bodyCol((float)sg_cycleColorR, (float)sg_cycleColorG, (float)sg_cycleColorB, 1.0f);
                    renderColor("Body Colour", "What the machine is painted, independent of the wall", &bodyCol);
                    sg_cycleColorR = bodyCol.x; sg_cycleColorG = bodyCol.y; sg_cycleColorB = bodyCol.z;
                }
            }

            renderItem("Override Own Color", "Override your cycle color visually for yourself", &sg_overrideLocalColor);
            if (sg_overrideLocalColor) {
                ImVec4 ownCol(sg_localColorR, sg_localColorG, sg_localColorB, 1.0f);
                renderColor("Own Cycle Color", "Choose your visual cycle color", &ownCol);
                sg_localColorR = ownCol.x;
                sg_localColorG = ownCol.y;
                sg_localColorB = ownCol.z;
            }
            renderItem("Distribute Opponent Colors", "Ensure all opponents have different, clean colors", &sg_distributeEnemyColors);
            if (!sg_distributeEnemyColors) {
                renderItem("Unified Opponent Color", "Override all other players to a single chosen color", &sg_overrideEnemyUnifiedColor);
                if (sg_overrideEnemyUnifiedColor) {
                    ImVec4 enemyCol(sg_enemyUnifiedColorR, sg_enemyUnifiedColorG, sg_enemyUnifiedColorB, 1.0f);
                    renderColor("Opponent Cycle Color", "Choose color for all other players", &enemyCol);
                    sg_enemyUnifiedColorR = enemyCol.x;
                    sg_enemyUnifiedColorG = enemyCol.y;
                    sg_enemyUnifiedColorB = enemyCol.z;
                }
            }
        }
        if (searchingAll || g_ModMenuTab == 1) { // HUD
            renderItem("Show HUD", "Toggle global visibility of HUD overlay", &g_ShowHUD, &g_ShowHUDKeybind);
            renderItem("Rubber Gauge", "Toggle display of the rubber gauge", &g_RubberGauge, &g_RubberGaugeKeybind);
            renderItem("Speed Meter", "Toggle display of the speedometer", &g_SpeedMeter, &g_SpeedMeterKeybind);
            renderItem("Brake Meter", "Toggle display of the brake gauge", &g_BrakeMeter, &g_BrakeMeterKeybind);
            renderItem("Scoreboard", "Toggle display of scoreboard", &g_ShowScores, &g_ShowScoresKeybind);
            renderItem("Show Ping", "Toggle display of latency information", &g_ShowPing, &g_ShowPingKeybind);
            renderItem("Alive Counter", "Toggle player alive counter", &g_AliveCounter, &g_AliveCounterKeybind);
            renderItem("Show Fastest", "Toggle display of peak speed attained", &g_ShowFastest, &g_ShowFastestKeybind);
            renderItem("Show FPS (Legacy)", "Toggle display of legacy FPS counter", &sr_FPSOut);
            
            // Migrated HUD: Minimap & In-Game Alerts
            renderItem("Minimap Enabled", "Show/hide the static minimap radar overlay", &sg_modMinimapEnabled);
            renderItem("Minimap Rotate", "Rotate minimap smoothly to match player direction", &sg_modMinimapRotate);
            if (sg_modMinimapRotate) {
                renderRealSlider("Rotation Speed", "Speed of minimap rotation. 1=Slow/smooth, 30=Instant", &sg_modMinimapRotateSpeed, 1.0f, 30.0f, "%.0f");
            }
            renderRealSlider("Minimap Zoom", "Zoom into minimap content", &sg_modMinimapZoom, 1.0f, 5.0f, "%.1fx");
            renderRealSlider("Minimap Size", "Minimap window scale size", &sg_modMinimapScale, 0.1f, 3.0f, "%.1fx");
            
            renderItem("Modern Live Scoreboard", "Esports-style rank list of teams and players", &sg_modLiveScoreboardEnabled);
            // sub options drawn in normal flow rather than the grid, so they
            // have no place among search results
            if (sg_modLiveScoreboardEnabled && searchBuf[0] == '\0') {
                ImGui::Indent(15.0f);
                ImGui::Checkbox("Show Teams##ModLiveScoreboard", &sg_modScoreboardShowTeams);
                ImGui::Checkbox("Show Players##ModLiveScoreboard", &sg_modScoreboardShowPlayers);
                ImGui::SliderInt("Max Players Shown##ModLiveScoreboard", &sg_modScoreboardMaxPlayers, 2, 24);
                ImGui::Checkbox("Show Ping in List##ModLiveScoreboard", &sg_modScoreboardShowPing);
                ImGui::Unindent(15.0f);
            }
            renderItem("Modern Zone Timer", "Sleek capsule showing remaining zone time with alarms", &sg_modZoneTimerEnabled);
            renderItem("Fortress Zone Alerts", "Show conquest/defense alerts for fortress zones", &sg_modFortressAlerts);
            renderItem("Teammate Death", "Flash red warning when a teammate dies", &sg_modTeammateDeathWarning);

            {
                bool dummyResetKDTab = false;
                renderItem("Reset K/D Stats", "Hotkey or toggle to reset your kills/deaths count", &dummyResetKDTab, &g_ResetKDKeybind);
                if (dummyResetKDTab) {
                    sg_modKDResetFlag = true;
                }
            }
            renderItem("Rubber Battery", "Wide horizontal rubber bar above bottom panels", &sg_modRubberBatteryEnabled);
            renderItem("Classic Rubber Battery", "Classic horizontal rubber bar above bottom panels", &sg_modClassicRubberBatteryEnabled);
            renderItem("Client Name Widget", "Separate widget showing the client logo name", &sg_modClientNameEnabled);
            renderItem("Center Message Widget", "Visual countdown and winner announcer overlay", &sg_modCenterMessageEnabled);
            renderItem("Wall Timer Widget", "Countdown till wall dissolves for dead players", &sg_modWallTimerEnabled);
            renderItem("Keystroke Visualizer 1", "Visual representation of pressed buttons (Widget 1)", &sg_modKeystroke1_Enabled);
            renderItem("Keystroke Visualizer 2", "Visual representation of pressed buttons (Widget 2)", &sg_modKeystroke2_Enabled);
            renderItem("FPS Widget", "Separate widget showing current framerate", &sg_modFpsEnabled);
            renderItem("Ping Widget", "Separate widget showing network latency", &sg_modPingEnabled);
            renderItem("Time Widget", "Separate widget showing current local system time", &sg_modTimeEnabled);
            renderItem("Keybinds Widget", "Display listing of all active modules and hotkeys", &sg_modKeybindsEnabled);
            renderItem("Modern KD Widget", "High-fidelity capsule showing Kills, Deaths, and Ratio", &sg_modKDWidgetEnabled);
            renderItem("Modern Speedometer", "Sleek circular needle gauge for cycle speed", &sg_modSpeedometerEnabled);
            renderItem("Modern Rubber Meter", "Circular needle gauge for rubber used with overheating alerts", &sg_modRubberMeterEnabled);
            renderItem("Modern Brake Reservoir", "Circular needle gauge of remaining braking capacity", &sg_modBrakeMeterEnabled);
            renderItem("Modern Scoreboard", "Top banner showing your score and the highest player score", &sg_modScoreboardWidgetEnabled);
            renderItem("Modern Alive Counter", "High-fidelity widget showing alive player counts", &sg_modAliveWidgetEnabled);
            renderItem("Network Health Widget", "Display connection quality, latency, and packet loss", &sg_modNetHealthEnabled);
            renderItem("Media Player Widget", "OS media / Spotify overlay with EQ bars and controls", &sg_modMediaWidgetEnabled);
            {
                bool dummyPlayPause = false;
                bool dummyNext = false;
                bool dummyPrev = false;
                renderItem("  Media: Play/Pause", "Bind key to play or pause media", &dummyPlayPause, &g_MediaPlayPauseKeybind);
                renderItem("  Media: Next Track", "Bind key to skip to next track", &dummyNext, &g_MediaNextKeybind);
                renderItem("  Media: Prev Track", "Bind key to go back to previous track", &dummyPrev, &g_MediaPrevKeybind);
            }
            renderSection("DISCORD");
            renderItem("Discord Status", "Show what you are playing under your name in Discord. Nothing leaves the machine except through Discord itself", &sg_discord);
            if (sg_discord)
            {
                renderNote("Connection", rc_DiscordStatus());
            }

            renderSection("ROUND START");
            renderIntSlider("Start Signal", "How a round is counted in: 0 plain numbers, 1 the light gantry, 2 a traffic light", &sg_startLights, 0.0f, 2.0f, "%.0f");
            renderRealSlider("Start Signal Size", "How large the lights are", &sg_startLightsSize, 0.4f, 2.5f, "%.2f");
            renderRealSlider("Start Signal Height", "Where they hang, as a fraction down the screen", &sg_startLightsY, 0.05f, 0.6f, "%.2f");
            renderItem("Chat Input Bar", "Draw the line you type into in our own face, under the chat", &sg_chatBar);

            renderItem("Edit HUD Layout", "Enter interactive dragging/snapping design mode", &isHudEditing);
        }
        if (searchingAll || g_ModMenuTab == 2) { // Client HUD Position adjustments
            {
                // What build this is, and whether there is a newer one. The
                // answer comes back with the hello the client sends anyway, so
                // nothing here goes and asks for it.
                char thisBuild[ 96 ];
                snprintf( thisBuild, sizeof( thisBuild ), "Version %s", RC_CLIENT_VERSION );

                if ( rc_UpdateWaiting() )
                {
                    char waiting[ 200 ];
                    std::string const notes = rc_UpdateNotes();
                    snprintf( waiting, sizeof( waiting ), "%s is out%s%s",
                              rc_UpdateVersion().c_str(),
                              notes.empty() ? "" : " - ",
                              notes.empty() ? "" : notes.c_str() );

                    renderNote( "Update available", waiting );
                    renderNote( thisBuild, rc_UpdateUrl().c_str() );
                }
                else
                {
                    renderNote( thisBuild, "This is the current build" );
                }
            }

            renderSlider("Field of View (FOV)", "Adjust the client camera Field of View", &g_FOV, 30.0f, 150.0f, "%.0f");
            renderSlider("Speed Gauge Size", "Scale factor of the speed meter gauge", &g_SpeedGaugeSize, 0.1f, 3.0f, "%.2f");
            renderSlider("Speed Gauge X Offset", "Horizontal position of the speed meter", &g_SpeedGaugeX, -2.0f, 2.0f, "%.2f");
            renderSlider("Speed Gauge Y Offset", "Vertical position of the speed meter", &g_SpeedGaugeY, -2.0f, 2.0f, "%.2f");
            
            renderSlider("Rubber Gauge Size", "Scale factor of the rubber gauge", &g_RubberGaugeSize, 0.1f, 3.0f, "%.2f");
            renderSlider("Rubber Gauge X Offset", "Horizontal position of the rubber gauge", &g_RubberGaugeX, -2.0f, 2.0f, "%.2f");
            renderSlider("Rubber Gauge Y Offset", "Vertical position of the rubber gauge", &g_RubberGaugeY, -2.0f, 2.0f, "%.2f");

            renderSlider("Brake Gauge Size", "Scale factor of the brake gauge", &g_BrakeGaugeSize, 0.1f, 3.0f, "%.2f");
            renderSlider("Brake Gauge X Offset", "Horizontal position of the brake gauge", &g_BrakeGaugeX, -2.0f, 2.0f, "%.2f");
            renderSlider("Brake Gauge Y Offset", "Vertical position of the brake gauge", &g_BrakeGaugeY, -2.0f, 2.0f, "%.2f");

            if (renderButton("Reset HUD Layout", "Restore default positions and sizes for all HUD gauges")) {
                g_OpenResetHUDPopup = true;
            }

        }
        if (searchingAll || g_ModMenuTab == 3) { // Noclip
            // The free camera used to be scattered - the switch on two tabs,
            // the speeds on a third, the keys in a file. All of it lives here
            // now and nowhere else.
            if (!searchingAll) {
                renderItem("Noclip Mode", "Leave the bike and fly the arena freely", &g_NoclipMode, &g_NoclipKeybind);
                renderItem("Clean Screen", "Hide every hud element for a clean shot", &g_CleanScreen, &g_CleanScreenKeybind);
            }

            renderSection("FLIGHT");
            renderRealSlider("Flight Speed", "Base movement speed in noclip mode", &sg_noclipSpeed, 5.0f, 500.0f, "%.0f");
            renderRealSlider("Slow Factor", "Speed multiplier when slow key is held", &sg_noclipSlowFactor, 0.05f, 1.0f, "%.2f");
            renderRealSlider("Fast Factor", "Speed multiplier when fast key is held", &sg_noclipFastFactor, 1.0f, 10.0f, "%.1f");
            renderRealSlider("Mouse Sensitivity", "Mouse look sensitivity in noclip mode", &sg_noclipMouseSens, 0.0005f, 0.02f, "%.4f");
            renderItem("Level Flight", "Forward stays level with the ground however far you look down", &sg_noclipLevel, &sg_noclipKeyLevel);
            renderItem("Lock On", "Ride a sphere around the chosen player and keep them framed", &sg_noclipSphere, &sg_noclipKeySphere);
            renderRealSlider("Lock-on Distance", "How far out the sphere sits from the player", &sg_noclipSphereRadius, 10.0f, 300.0f, "%.0f");
            renderRealSlider("Orbit Speed", "Angular speed of orbit camera", &sg_noclipOrbitSpeed, 0.05f, 2.0f, "%.2f");
            renderRealSlider("Orbit Radius", "Distance from orbit center to camera", &sg_noclipOrbitRadius, 10.0f, 500.0f, "%.0f");
            renderRealSlider("Orbit Height", "Height of orbit camera above target", &sg_noclipOrbitHeight, 5.0f, 300.0f, "%.0f");
            renderRealSlider("Follow Distance", "Camera distance behind followed player", &sg_noclipFollowDist, 5.0f, 200.0f, "%.0f");
            renderRealSlider("Follow Height", "Camera height above followed player", &sg_noclipFollowHeight, 5.0f, 200.0f, "%.0f");
            renderRealSlider("Smooth Factor", "LERP smoothing strength for follow camera", &sg_noclipSmoothFactor, 1.0f, 20.0f, "%.1f");


            renderSection("KEYS");
            renderKey("Forward",            &sg_noclipKeyForward);
            renderKey("Back",               &sg_noclipKeyBack);
            renderKey("Left",               &sg_noclipKeyLeft);
            renderKey("Right",              &sg_noclipKeyRight);
            renderKey("Up",                 &sg_noclipKeyUp);
            renderKey("Down",               &sg_noclipKeyDown);
            renderKey("Zoom in",            &sg_noclipKeyZoomIn);
            renderKey("Zoom out",           &sg_noclipKeyZoomOut);
            renderKey("Slow",               &sg_noclipKeySlow);
            renderKey("Fast",               &sg_noclipKeyFast);
            renderKey("Level flight",       &sg_noclipKeyLevel);
            renderKey("Lock on",            &sg_noclipKeySphere);
            renderKey("Centre",             &sg_noclipKeyCenter);
            renderKey("Look at",            &sg_noclipKeyLookAt);
            renderKey("Lock",               &sg_noclipKeyLock);
            renderKey("Follow",             &sg_noclipKeyFollow);
            renderKey("Next player",        &sg_noclipKeyNextPl);
            renderKey("Previous player",    &sg_noclipKeyPrevPl);

            renderSection("WAYPOINTS");
            renderKey("Drop waypoint",      &sg_noclipKeyMark);
            renderKey("Next waypoint",      &sg_noclipKeyMarkNext);
            renderKey("Previous waypoint",  &sg_noclipKeyMarkPrev);
            renderKey("Delete waypoint",    &sg_noclipKeyMarkDrop);
            renderKey("Show or hide",       &sg_noclipKeyMarkHide);
            renderItem("Waypoints Visible", "Show the waypoint list, or keep it out of the shot", &sg_waypointsVisible);
        }
        if (searchingAll || g_ModMenuTab == 4) { // Util
            {
                // Neither of these makes the game easier: the walls still kill
                // and the riders still take up room. They are here for a busy
                // screen on a tired machine and for a clear look at one's own
                // line, and they never hide the person holding the keyboard.

                renderItem("Hide Other Cycles", "Stop drawing other players' machines, names and all", &sg_hideOtherCycles, &g_HideCyclesKeybind);
                renderItem("Hide Other Trails", "Stop drawing other players' walls - they still kill", &sg_hideOtherTrails, &g_HideTrailsKeybind);
            }

            renderItem("Quick Finder", "Search every screen and switch from one field", &g_FinderEnabled, &g_FinderKeybind);

            {
                // The interface has its own voice and its own volume, separate
                // from the game's: these are heard while everything else is
                // parked, and what suits a match rarely suits a menu.
                // The slider works on a float and the setting is kept wider,
                // so it is copied out, shown, and copied back.
                static float loud = (float)se_uiVolume;
                loud = (float)se_uiVolume;
                renderSlider("Interface Volume", "How loud the menu's own sounds are", &loud, 0.0f, 1.0f, "%.2f");
                se_uiVolume = loud;
            }

            // Kill sounds
            renderItem("Kill Sounds", "Voice announcement pack on kill streaks", &sg_modKillSoundsEnabled);
            renderIntSlider("Announcer Pack", "Choose announcer pack style", &sg_modKillAnnouncerPack, 0.0f, 2.0f, "ANNOUNCER_PACK_FORMAT");

            renderItem("Show Time", "Toggle clock rendering in the HUD", &g_ShowTime, &g_ShowTimeKeybind);
            renderItem("24h Format", "Use 24-hour style for the clock", &g_24hFormat, &g_24hFormatKeybind);
            renderItem("Cycle Sparks", "Toggle sparks generation on collisions", &g_Sparks, &g_SparksKeybind);
            renderItem("White Sparks", "Use clean white particles for sparks", &g_WhiteSparks, &g_WhiteSparksKeybind);

            // Beside the thing they change the look of, and only while it is
            // switched on. Sat on their own they were two switches that did
            // nothing whatsoever, because what they repaint was not being
            // drawn in the first place - and nothing said so.
            if (g_Sparks) {
                {
                    // How often a shower is thrown at all, which matters far
                    // more than how many are in one: grinding a wall asks for
                    // a new one on every step of the simulation.
                    float rate = (float)sg_sparkRate;
                    renderSlider("Spark Rate", "Showers per second while grinding a wall", &rate, 1.0f, 60.0f, "%.0f");
                    sg_sparkRate = rate;
                }

                renderItem("Heart Sparks", "Sparks off a wall come out as hearts", &sg_heartSparks, &g_HeartSparksKeybind);

                if (sg_heartSparks) {
                    float howMany = (float)sg_heartSparkCount;
                    renderSlider("Spark Hearts", "How many a scrape along a wall throws", &howMany, 1.0f, 40.0f, "%.0f");
                    sg_heartSparkCount = (int)(howMany + 0.5f);
                }
            }

            {
                // Yours alone, and it needs no parent switched on: this is not
                // a repaint of something else, it is its own thing.
                renderItem("Heart Trail", "Hearts drift up out of your own wake", &sg_heartTrail, &g_HeartTrailKeybind);

                if (sg_heartTrail) {
                    float howMany = (float)sg_heartTrailCount;
                    renderSlider("Trail Hearts", "How many are in the air behind you", &howMany, 1.0f, 24.0f, "%.0f");
                    sg_heartTrailCount = (int)(howMany + 0.5f);
                }
            }

            renderItem("Explosions", "Toggle particle explosions upon death", &g_Explosions, &g_ExplosionsKeybind);

            if (g_Explosions) {
                renderItem("Heart Explosions", "A crash throws hearts instead of shards", &sg_heartDeaths, &g_HeartDeathsKeybind);

                if (sg_heartDeaths) {
                    float howMany = (float)sg_heartDeathCount;
                    renderSlider("Death Hearts", "How many a crash throws", &howMany, 1.0f, 60.0f, "%.0f");
                    sg_heartDeathCount = (int)(howMany + 0.5f);
                }
            }
            renderItem("Single Line Explosion", "Render legacy crash explosion as a single vertical vector line", &sg_explosionSingleLineUp);
            renderItem("Auto Packet Refresh", "Trigger sync commands when packet delay exceeds 1.2s", &g_AutoPacketRefresh, &g_PacketRefreshKeybind);
            
            // Migrated Util: Chat Bots & Automation
#if !PUBLIC_BUILD
#endif
            renderItem("Mod Menu Toggle Keybind", "Keybind to open the Mod Menu", &g_ModMenuKeybindEnabled, &g_ModMenuKeybind);
        }
        if (searchingAll || g_ModMenuTab == 5) { // Theme
            renderItem("Rainbow Top Bar", "Enables cycling RGB gradient on top bar", &g_RGBTopBar, &g_RGBTopBarKeybind);
            renderItem("Rainbow Accent", "Enables cycling RGB on all elements", &g_RGBAccent, &g_RGBAccentKeybind);
            renderSlider("Rainbow Speed", "Controls velocity of hue rotation", &g_RGBSpeed, 0.0f, 2.0f, "%.2f");

            renderItem("Particle Backdrop", "Toggle floating background particles", &g_ShowParticles, &g_ShowParticlesKeybind);
            renderSlider("Particle Type", "Choose particle shape (0=Dust, 1=Rain, 2=Stars, 3=Hearts)", &g_ParticleType, 0.0f, 3.0f, "%.0f");
            renderItem("Interactive Particles", "Toggle mouse repulsion on particles", &g_InteractiveParticles, &g_InteractiveParticlesKeybind);
            renderItem("Constellation Web", "Toggle connecting lines between particles", &g_ConstellationWeb, &g_ConstellationWebKeybind);
            renderItem("Parallax Effect", "Toggle 3D parallax offsets for particles", &g_ParallaxEffect, &g_ParallaxEffectKeybind);
            renderItem("Gradient Accent", "Use a smooth blend between 2 colors", &g_GradientAccent, &g_GradientAccentKeybind);
            if (!g_RGBAccent) {
                renderColor("Accent Color 1", "Starting color of the theme gradient", &g_AccentColor1);
            }
            if (!g_RGBAccent && g_GradientAccent) {
                renderColor("Accent Color 2", "Ending color of the theme gradient", &g_AccentColor2);
            }
            renderColor("Background Color", "Changes the main window color", &g_MenuBgColor);
            renderSlider("Background Opacity", "Sets the transparency of the menu", &g_MenuBgAlpha, 0.05f, 1.0f, "%.2f");
        }
        if (!searchingAll && g_ModMenuTab == 6) { // Configs
            // First item: "+ Create New Config"
            ImVec2 pos = GET_CELL_POS(cellIdx % numCols, cellIdx / numCols);
            if (CreateConfigItemAbsolute(pos, ImVec2(columnW, itemH), alphaMultiplier)) {
                g_NewConfigName[0] = '\0';
                g_OpenCreateConfigPopup = true;
            }
            cellIdx++;

            // List files in the var directory
            tString varDir = tDirectories::Var().GetWritePath("x");
            if (varDir.Len() > 2) {
                varDir = varDir.SubStr(0, varDir.Len() - 2);
            } else {
                varDir = "./";
            }

            tArray<tString> files;
            tDirectories::GetFiles(varDir, tString("*.cfg"), files, tDirectories::eGetFilesFilesOnly);

            for (int i = 0; i < files.Len(); i++) {
                tString filename = files(i);
                std::string s((const char*)filename);
                for (size_t c = 0; c < s.length(); ++c) {
                    s[c] = std::tolower(s[c]);
                }
                if (s == "user.cfg" || s == "settings.cfg" || s == "master.cfg" || s == "aiplayers.cfg" || s == "autoexec.cfg") {
                    continue;
                }

                bool applyClicked = false;
                bool updateClicked = false;
                bool deleteClicked = false;
                bool folderClicked = false;

                ImVec2 itemPos = GET_CELL_POS(cellIdx % numCols, cellIdx / numCols);
                ConfigItemAbsolute(itemPos, ImVec2(columnW, itemH), (const char*)filename, applyClicked, updateClicked, deleteClicked, folderClicked, alphaMultiplier);

                if (applyClicked) {
                    strncpy(g_SelectedConfig, (const char*)filename, sizeof(g_SelectedConfig) - 1);
                    g_SelectedConfig[sizeof(g_SelectedConfig) - 1] = '\0';
                    g_OpenApplyConfigPopup = true;
                }
                if (updateClicked) {
                    strncpy(g_SelectedConfig, (const char*)filename, sizeof(g_SelectedConfig) - 1);
                    g_SelectedConfig[sizeof(g_SelectedConfig) - 1] = '\0';
                    g_OpenUpdateConfigPopup = true;
                }
                if (deleteClicked) {
                    strncpy(g_SelectedConfig, (const char*)filename, sizeof(g_SelectedConfig) - 1);
                    g_SelectedConfig[sizeof(g_SelectedConfig) - 1] = '\0';
                    g_OpenDeleteConfigPopup = true;
                }
                if (folderClicked) {
                    tString fullPath = tDirectories::Var().GetWritePath(filename);
                    std::string fullPathStr((const char*)fullPath);
#ifdef WIN32
                    for (size_t i = 0; i < fullPathStr.length(); ++i) {
                        if (fullPathStr[i] == '/') {
                            fullPathStr[i] = '\\';
                        }
                    }
                    size_t lastSlash = fullPathStr.find_last_of('\\');
#else
                    size_t lastSlash = fullPathStr.find_last_of('/');
#endif
                    if (lastSlash != std::string::npos) {
                        std::string dirPath = fullPathStr.substr(0, lastSlash);
#ifdef WIN32
                        ::ShellExecuteA(NULL, "open", dirPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
                        tString cmd;
                        cmd << "xdg-open \"" << dirPath.c_str() << "\" &";
                        int res = system((const char*)cmd);
                        (void)res;
#endif
                    }
                }

                cellIdx++;
            }
        } else if (!searchingAll && g_ModMenuTab == 7) { // Player Setup
            ePlayer* lp = ePlayer::PlayerConfig(0);
            if (lp) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.8f));
                bool useColumns = (winSize.x >= 900.0f);
                if (useColumns) {
                    ImGui::Columns(2, "PlayerSetupColumns", false);
                    ImGui::SetColumnWidth(0, (mainW - 30.0f) * 0.5f);
                    ImGui::SetColumnWidth(1, (mainW - 30.0f) * 0.5f);
                } else {
                    ImGui::Columns(1);
                }
                
                // COLUMN 0: General Profile & Colors
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "PROFILE SETTINGS");
                DrawColumnSeparator();
                ImGui::Spacing();
                
                char nameBuf[256];
                std::string nameUtf8 = Cp1251ToUtf8((const char*)lp->name);
                strncpy(nameBuf, nameUtf8.c_str(), sizeof(nameBuf));
                nameBuf[sizeof(nameBuf)-1] = '\0';
                if (ImGui::InputText("Screen Name##profile", nameBuf, sizeof(nameBuf))) {
                    lp->name = Utf8ToCp1251(nameBuf);
                    // request network synchronization
                    static nVersionFeature inGameRenames(5);
                    if (inGameRenames.Supported()) {
                        ePlayerNetID::Update();
                        ePlayer::SendAuthNames();
                    }
                }
                ImGui::Spacing();
                
                // Player Color (R, G, B)
                float col[3] = { (float)lp->rgb[0] / 15.0f, (float)lp->rgb[1] / 15.0f, (float)lp->rgb[2] / 15.0f };
                if (ImGui::ColorEdit3("Cycle Color", col)) {
                    lp->rgb[0] = (int)(col[0] * 15.0f + 0.5f);
                    lp->rgb[1] = (int)(col[1] * 15.0f + 0.5f);
                    lp->rgb[2] = (int)(col[2] * 15.0f + 0.5f);
                }
                int rawR = lp->rgb[0];
                int rawG = lp->rgb[1];
                int rawB = lp->rgb[2];
                if (ImGui::SliderInt("Raw R (Glow)", &rawR, 0, 255)) lp->rgb[0] = rawR;
                if (ImGui::SliderInt("Raw G (Glow)", &rawG, 0, 255)) lp->rgb[1] = rawG;
                if (ImGui::SliderInt("Raw B (Glow)", &rawB, 0, 255)) lp->rgb[2] = rawB;
                ImGui::Spacing();

                DrawCycleColorPreview(lp);
                ImGui::Spacing();
                
                char gidBuf[512];
                std::string gidUtf8 = Cp1251ToUtf8((const char*)lp->globalID);
                strncpy(gidBuf, gidUtf8.c_str(), sizeof(gidBuf));
                gidBuf[sizeof(gidBuf)-1] = '\0';
                if (ImGui::InputText("Global ID##profile", gidBuf, sizeof(gidBuf))) {
                    lp->globalID = Utf8ToCp1251(gidBuf);
                }
                ImGui::Spacing();
                
                ImGui::Checkbox("Auto Login on Connect", &lp->autoLogin);
                ImGui::Checkbox("Stealth Mode (Hide ID)", &lp->stealth);
                ImGui::Checkbox("Spectator Mode", &lp->spectate);
                ImGui::Checkbox("Name Team After Me", &lp->nameTeamAfterMe);
                
                char teamNameBuf[256];
                std::string teamNameUtf8 = Cp1251ToUtf8((const char*)lp->teamName);
                strncpy(teamNameBuf, teamNameUtf8.c_str(), sizeof(teamNameBuf));
                teamNameBuf[sizeof(teamNameBuf)-1] = '\0';
                if (ImGui::InputText("Custom Team Name##profile", teamNameBuf, sizeof(teamNameBuf))) {
                    lp->teamName = Utf8ToCp1251(teamNameBuf);
                    static nVersionFeature inGameRenames(5);
                    if (inGameRenames.Supported()) {
                        ePlayerNetID::Update();
                        ePlayer::SendAuthNames();
                    }
                }
                
                ImGui::Text("Preferred Players Per Team:");
                ImGui::SliderInt("##PlayersPerTeam", &lp->favoriteNumberOfPlayersPerTeam, 1, 16);
                ImGui::Spacing();
                ImGui::Spacing();
                
                // Camera Configurations
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "CAMERA CONFIGURATION");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text("Field of View (FOV):");
                if (ImGui::SliderInt("##FOV_Slider", &lp->startFOV, 30, 160)) rc_CameraFOV(lp->startFOV);
                
                ImGui::Checkbox("Auto-Switch In-Cam", &lp->autoSwitchIncam);
                ImGui::Checkbox("Wobble In-Cam", &lp->wobbleIncam);
                ImGui::Checkbox("Center In-Cam on Turn", &lp->centerIncamOnTurn);
                
                ImGui::Text("Allowed Camera Modes:");
                ImGui::Checkbox("Smart Cam", &lp->allowCam[CAMERA_SMART]); ImGui::SameLine();
                ImGui::Checkbox("Follow Cam", &lp->allowCam[CAMERA_FOLLOW]); ImGui::SameLine();
                ImGui::Checkbox("Free Cam", &lp->allowCam[CAMERA_FREE]);
                ImGui::Checkbox("Custom Cam", &lp->allowCam[CAMERA_CUSTOM]); ImGui::SameLine();
                ImGui::Checkbox("Server Custom", &lp->allowCam[CAMERA_SERVER_CUSTOM]); ImGui::SameLine();
                ImGui::Checkbox("In-Cam", &lp->allowCam[CAMERA_IN]);
                
                const char* camModeNames[] = { "Server Custom", "Custom", "In-Camera", "Smart", "Follow", "Free" };
                eCamMode camModes[] = { CAMERA_SERVER_CUSTOM, CAMERA_CUSTOM, CAMERA_IN, CAMERA_SMART, CAMERA_FOLLOW, CAMERA_FREE };
                int currentCamIdx = 3;
                for (int c = 0; c < 6; c++) {
                    if (lp->startCamera == camModes[c]) { currentCamIdx = c; break; }
                }
                if (ImGui::Combo("Initial Camera Mode", &currentCamIdx, camModeNames, 6)) {
                    lp->startCamera = camModes[currentCamIdx];
                }
                
                if (useColumns) {
                    ImGui::NextColumn();
                } else {
                    ImGui::Spacing(); ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing(); ImGui::Spacing();
                }
                
                // COLUMN 1: Instant Chat Macros
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "INSTANT CHAT MACROS");
                DrawColumnSeparator();
                ImGui::Spacing();
                
                ImGui::BeginChild("##MacrosScroll", ImVec2(useColumns ? 390.0f : (mainW - 30.0f), 420.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                for (int m = 0; m < 20; m++) {
                    char macroBuf[256];
                    std::string macroUtf8 = Cp1251ToUtf8((const char*)lp->instantChatString[m]);
                    strncpy(macroBuf, macroUtf8.c_str(), sizeof(macroBuf));
                    macroBuf[sizeof(macroBuf)-1] = '\0';
                    char label[64];
                    snprintf(label, sizeof(label), "Macro %d##macro_%d", m + 1, m);
                    if (ImGui::InputText(label, macroBuf, sizeof(macroBuf))) {
                        lp->instantChatString[m] = Utf8ToCp1251(macroBuf);
                    }
                    ImGui::Spacing();
                }
                ImGui::EndChild();
                
                if (useColumns) {
                    ImGui::Columns(1);
                }
                ImGui::PopStyleColor();
            }
        } else if (!searchingAll && g_ModMenuTab == 8) { // Profiles & Managers
            ePlayer* lp = ePlayer::PlayerConfig(0);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.8f));
            bool useColumns = (winSize.x >= 900.0f);
            if (useColumns) {
                ImGui::Columns(2, "ProfilesColumns", false);
                ImGui::SetColumnWidth(0, (mainW - 30.0f) * 0.5f);
                ImGui::SetColumnWidth(1, (mainW - 30.0f) * 0.5f);
            } else {
                ImGui::Columns(1);
            }
            
            // ================= COLUMN 0: Profiles & Colors =================
            // 1. Name & Account Manager
            ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "NAME & ACCOUNT MANAGER");
            DrawColumnSeparator();
            ImGui::Spacing();

            if (lp) {
                char nameBuf[256];
                std::string nameUtf8 = Cp1251ToUtf8((const char*)lp->name);
                strncpy(nameBuf, nameUtf8.c_str(), sizeof(nameBuf));
                nameBuf[sizeof(nameBuf)-1] = '\0';
                if (ImGui::InputText("Screen Name##mgr", nameBuf, sizeof(nameBuf))) {
                    lp->name = Utf8ToCp1251(nameBuf);
                    static nVersionFeature inGameRenames(5);
                    if (inGameRenames.Supported()) {
                        ePlayerNetID::Update();
                        ePlayer::SendAuthNames();
                    }
                }
                ImGui::Text("Preview: ");
                ImGui::SameLine();
                RenderArmagetronColoredText((const char*)lp->name);
                ImGui::Spacing();
                
                char gidBuf[512];
                std::string gidUtf8 = Cp1251ToUtf8((const char*)lp->globalID);
                strncpy(gidBuf, gidUtf8.c_str(), sizeof(gidBuf));
                gidBuf[sizeof(gidBuf)-1] = '\0';
                if (ImGui::InputText("Global ID##mgr", gidBuf, sizeof(gidBuf))) {
                    lp->globalID = Utf8ToCp1251(gidBuf);
                }
                
                ImGui::Checkbox("Auto Login##mgr", &lp->autoLogin);
                
                if (ImGui::Button("Add to Favorites##mgr", ImVec2(-1, 28))) {
                    SavedProfile sp;
                    sp.name = (const char*)lp->name;
                    sp.globalID = (const char*)lp->globalID;
                    sp.autoLogin = lp->autoLogin;
                    g_SavedProfiles.push_back(sp);
                    SaveProfiles();
                }
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "Saved Profiles:");
            ImGui::BeginChild("##ProfilesList", ImVec2(-1, 140), true);
            for (size_t i = 0; i < g_SavedProfiles.size(); i++) {
                ImGui::PushID(i);
                RenderArmagetronColoredText(g_SavedProfiles[i].name.c_str());
                
                float loadX, loadW, delX, delW;
                rc_RowButtons(loadX, loadW, delX, delW);
                ImGui::SameLine(loadX);
                if (ImGui::Button("Load", ImVec2(loadW, 0.0f))) {
                    if (lp) {
                        lp->name = g_SavedProfiles[i].name.c_str();
                        lp->globalID = g_SavedProfiles[i].globalID.c_str();
                        lp->autoLogin = g_SavedProfiles[i].autoLogin;
                        static nVersionFeature inGameRenames(5);
                        if (inGameRenames.Supported()) {
                            ePlayerNetID::Update();
                            ePlayer::SendAuthNames();
                        }
                    }
                }
                ImGui::SameLine(delX);
                if (ImGui::Button("Delete", ImVec2(delW, 0.0f))) {
                    g_SavedProfiles.erase(g_SavedProfiles.begin() + i);
                    SaveProfiles();
                    i--;
                }
                ImGui::PopID();
                ImGui::Separator();
            }
            ImGui::EndChild();
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // 2. Color Manager
            ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "COLOR MANAGER");
            DrawColumnSeparator();
            ImGui::Spacing();

            if (lp) {
                float col[3] = { (float)lp->rgb[0] / 15.0f, (float)lp->rgb[1] / 15.0f, (float)lp->rgb[2] / 15.0f };
                if (ImGui::ColorEdit3("Active Color##mgr", col)) {
                    lp->rgb[0] = (int)(col[0] * 15.0f + 0.5f);
                    lp->rgb[1] = (int)(col[1] * 15.0f + 0.5f);
                    lp->rgb[2] = (int)(col[2] * 15.0f + 0.5f);
                }
                
                int rawR = lp->rgb[0];
                int rawG = lp->rgb[1];
                int rawB = lp->rgb[2];
                if (ImGui::SliderInt("Raw R##mgr", &rawR, 0, 255)) lp->rgb[0] = rawR;
                if (ImGui::SliderInt("Raw G##mgr", &rawG, 0, 255)) lp->rgb[1] = rawG;
                if (ImGui::SliderInt("Raw B##mgr", &rawB, 0, 255)) lp->rgb[2] = rawB;
                
                static char newColorName[64] = "My Color Preset";
                ImGui::InputText("Preset Name##mgr", newColorName, sizeof(newColorName));
                
                if (ImGui::Button("Add to Favorites##col_mgr", ImVec2(-1, 28))) {
                    SavedColor sc;
                    sc.name = newColorName;
                    sc.r = lp->rgb[0];
                    sc.g = lp->rgb[1];
                    sc.b = lp->rgb[2];
                    g_SavedColors.push_back(sc);
                    SaveColors();
                }
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "Saved ColorsPreset:");
            ImGui::BeginChild("##ColorsList", ImVec2(-1, 140), true);
            static int selectedColorIdx = 0;
            if (selectedColorIdx >= (int)g_SavedColors.size()) {
                selectedColorIdx = 0;
            }
            for (size_t i = 0; i < g_SavedColors.size(); i++) {
                ImGui::PushID(i);
                bool isSelected = (selectedColorIdx == (int)i);
                
                ImVec2 cursor = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(cursor.x, cursor.y + 2), ImVec2(cursor.x + 15, cursor.y + 17), ImGui::ColorConvertFloat4ToU32(ImVec4(g_SavedColors[i].r/15.f, g_SavedColors[i].g/15.f, g_SavedColors[i].b/15.f, 1.f)));
                ImGui::Dummy(ImVec2(20, 15));
                ImGui::SameLine();
                
                float loadX, loadW, delX, delW;
                rc_RowButtons(loadX, loadW, delX, delW);
                float nameWidth = loadX - ImGui::GetCursorPosX() - 8.0f;
                if (nameWidth < 50.0f) nameWidth = 50.0f;
                if (ImGui::Selectable(g_SavedColors[i].name.c_str(), isSelected, 0, ImVec2(nameWidth, 0))) {
                    selectedColorIdx = i;
                }
                
                ImGui::SameLine(loadX);
                if (ImGui::Button("Load", ImVec2(loadW, 0.0f))) {
                    if (lp) {
                        lp->rgb[0] = g_SavedColors[i].r;
                        lp->rgb[1] = g_SavedColors[i].g;
                        lp->rgb[2] = g_SavedColors[i].b;
                        static nVersionFeature inGameRenames(5);
                        if (inGameRenames.Supported()) {
                            ePlayerNetID::Update();
                            ePlayer::SendAuthNames();
                        }
                    }
                }
                ImGui::SameLine(delX);
                if (ImGui::Button("Delete", ImVec2(delW, 0.0f))) {
                    g_SavedColors.erase(g_SavedColors.begin() + i);
                    SaveColors();
                    i--;
                }
                ImGui::PopID();
            }
            ImGui::EndChild();

            if (!g_SavedColors.empty() && selectedColorIdx < (int)g_SavedColors.size()) {
                int previewRGB[3] = { g_SavedColors[selectedColorIdx].r, g_SavedColors[selectedColorIdx].g, g_SavedColors[selectedColorIdx].b };
                DrawCycleColorPreviewForRGB(previewRGB, "PRESET CYCLE & TRAIL PREVIEW");
            }

            if (useColumns) {
                ImGui::NextColumn();
            } else {
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
            }
            
            // ================= COLUMN 1: Camera & Textures =================
            // 3. Camera Configuration Manager
            ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "CAMERA MANAGER");
            DrawColumnSeparator();
            ImGui::Spacing();

            std::vector<std::string> camConfigs = GetAvailableCameraConfigs();
            static int selectedCamIdx = -1;
            if (selectedCamIdx == -1 && !camConfigs.empty()) {
                for (size_t i = 0; i < camConfigs.size(); i++) {
                    if (camConfigs[i] == (const char*)sg_activeCameraConfig) {
                        selectedCamIdx = i;
                        break;
                    }
                }
                if (selectedCamIdx == -1) selectedCamIdx = 0;
            }

            ImGui::Text("Active Config: ");
            ImGui::SameLine();
            if (sg_activeCameraConfig == "") {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "None (Default)");
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "%s", (const char*)sg_activeCameraConfig);
            }

            ImGui::Spacing();
            ImGui::Text("Select Camera Preset (.cfg):");
            if (ImGui::BeginCombo("##CamPresetCombo", camConfigs.empty() ? "No configs found" : camConfigs[selectedCamIdx].c_str())) {
                for (size_t i = 0; i < camConfigs.size(); i++) {
                    bool isSelected = (selectedCamIdx == (int)i);
                    if (ImGui::Selectable(camConfigs[i].c_str(), isSelected)) {
                        selectedCamIdx = i;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            if (ImGui::Button("Apply Selected Config", ImVec2(-1, 28))) {
                if (!camConfigs.empty() && selectedCamIdx >= 0 && selectedCamIdx < (int)camConfigs.size()) {
                    ApplyCameraConfig(camConfigs[selectedCamIdx]);
                }
            }

            if (ImGui::Button("Reset Camera to Game Default", ImVec2(-1, 28))) {
                ResetCameraSettingsToDefault();
                sg_activeCameraConfig = "";
                st_SaveConfig();
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // 4. Texture & Customization Manager
            ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "TEXTURE MANAGER");
            DrawColumnSeparator();
            ImGui::Spacing();

            std::vector<std::string> texPacks = GetAvailableTexturePacks();
            static int selectedPackIdx = -1;
            if (selectedPackIdx == -1 && !texPacks.empty()) {
                for (size_t i = 0; i < texPacks.size(); i++) {
                    if (texPacks[i] == (const char*)sg_activeTexturePack) {
                        selectedPackIdx = i;
                        break;
                    }
                }
                if (selectedPackIdx == -1) selectedPackIdx = 0;
            }

            ImGui::Text("Active Pack: ");
            ImGui::SameLine();
            if (sg_activeTexturePack == "Default") {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Default");
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "%s", (const char*)sg_activeTexturePack);
            }

            ImGui::Spacing();
            ImGui::Text("Select Custom Theme Pack:");
            if (ImGui::BeginCombo("##TexPackCombo", texPacks.empty() ? "No packs found" : texPacks[selectedPackIdx].c_str())) {
                for (size_t i = 0; i < texPacks.size(); i++) {
                    bool isSelected = (selectedPackIdx == (int)i);
                    if (ImGui::Selectable(texPacks[i].c_str(), isSelected)) {
                        selectedPackIdx = i;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            if (ImGui::Button("Apply Selected Pack", ImVec2(-1, 28))) {
                if (!texPacks.empty() && selectedPackIdx >= 0 && selectedPackIdx < (int)texPacks.size()) {
                    ApplyTexturePack(texPacks[selectedPackIdx]);
                }
            }

            ImGui::Spacing();
            ImGui::TextWrapped("Custom packs directory: ./custom_packs/");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Structure:\n./custom_packs/<PackName>/textures/\n./custom_packs/<PackName>/models/");

            if (useColumns) {
                ImGui::Columns(1);
            }
            ImGui::PopStyleColor();
        }

    // Put a dummy spacing element at the bottom of child content to define total height for scrollbar
    int totalRows = (cellIdx + numCols - 1) / numCols;
    float totalHeight = totalRows * (itemH + spacingY) + 2.0f * gridMarginY;
    if (g_ModMenuTab == 7) {
        totalHeight = 0.0f;
    } else if (g_ModMenuTab == 8) {
        totalHeight = 0.0f;
    }
    ImGui::Dummy(ImVec2(mainW - scrollbarPadding - 2.0f * gridMarginX, totalHeight));
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

#undef GET_CELL_POS

    // Apply configuration back to engine
    subby_ShowHUD = g_ShowHUD;
    subby_ShowSpeedFastest = g_ShowFastest;
    subby_ShowScore = g_ShowScores;
    subby_ShowAlivePeople = g_AliveCounter;
    subby_ShowPing = g_ShowPing;
    subby_ShowSpeedMeter = g_SpeedMeter;
    subby_ShowBrakeMeter = g_BrakeMeter;
    subby_ShowRubberMeter = g_RubberGauge;
    showTime = g_ShowTime;
    show24hour = g_24hFormat;

    CUSTOM_FOG_R = g_FogR;
    CUSTOM_FOG_G = g_FogG;
    CUSTOM_FOG_B = g_FogB;
    CUSTOM_FOG_DENSITY = g_FogDensity;

    if (lp_cam) {
        lp_cam->smartCustomGlance = g_SmartGlance;
        lp_cam->startFOV = g_FOV;
            rc_CameraFOV( (int)g_FOV );
    }

    subby_SpeedGaugeSize = g_SpeedGaugeSize;
    subby_SpeedGaugeLocX = g_SpeedGaugeX;
    subby_SpeedGaugeLocY = g_SpeedGaugeY;
    subby_BrakeGaugeSize = g_BrakeGaugeSize;
    subby_BrakeGaugeLocX = g_BrakeGaugeX;
    subby_BrakeGaugeLocY = g_BrakeGaugeY;
    subby_RubberGaugeSize = g_RubberGaugeSize;
    subby_RubberGaugeLocX = g_RubberGaugeX;
    subby_RubberGaugeLocY = g_RubberGaugeY;

    crash_sparks = g_Sparks;
    white_sparks = g_WhiteSparks;
    sg_crashExplosion = g_Explosions;
    sr_alphaBlend = g_AlphaBlend;
    sr_smoothShading = g_SmoothShading;
    sr_floorDetail = (int)( g_FloorDetail + 0.5f );   // rounding, or the top setting can never be reached
    sg_noclipCinematic = g_CleanScreen;

    // Bidirectional synchronization logic (Push back to trackers)
    prev_ShowHUD = g_ShowHUD;
    prev_ShowFastest = g_ShowFastest;
    prev_ShowScores = g_ShowScores;
    prev_AliveCounter = g_AliveCounter;
    prev_ShowPing = g_ShowPing;
    prev_SpeedMeter = g_SpeedMeter;
    prev_BrakeMeter = g_BrakeMeter;
    prev_RubberGauge = g_RubberGauge;
    prev_ShowTime = g_ShowTime;
    prev_24hFormat = g_24hFormat;
    prev_FogR = g_FogR;
    prev_FogG = g_FogG;
    prev_FogB = g_FogB;
    prev_FogDensity = g_FogDensity;
    prev_NoclipMode = g_NoclipMode;
    prev_CleanScreen = g_CleanScreen;
    if (lp_cam) {
        prev_SmartGlance = g_SmartGlance;
        prev_FOV = g_FOV;
    }
    prev_SpeedGaugeSize = g_SpeedGaugeSize;
    prev_SpeedGaugeX = g_SpeedGaugeX;
    prev_SpeedGaugeY = g_SpeedGaugeY;
    prev_BrakeGaugeSize = g_BrakeGaugeSize;
    prev_BrakeGaugeX = g_BrakeGaugeX;
    prev_BrakeGaugeY = g_BrakeGaugeY;
    prev_RubberGaugeSize = g_RubberGaugeSize;
    prev_RubberGaugeX = g_RubberGaugeX;
    prev_RubberGaugeY = g_RubberGaugeY;
    prev_Sparks = g_Sparks;
    prev_WhiteSparks = g_WhiteSparks;
    prev_Explosions = g_Explosions;
    prev_AlphaBlend = g_AlphaBlend;
    prev_SmoothShading = g_SmoothShading;
    prev_FloorDetail = g_FloorDetail;

    // DIVIDER LINE ABOVE PROFILE AREA
    dl->AddLine(
        ImVec2(mainX, winPos.y + winSize.y - 80.0f),
        ImVec2(mainX + mainW, winPos.y + winSize.y - 80.0f),
        ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, g_MenuAlpha * 0.6f)),
        1.0f
    );

    // PROFILE AREA IN BOTTOM LEFT
    ImVec2 avatarPos = ImVec2(mainX, winPos.y + winSize.y - 70);
    if (g_AvatarTexture == 0) {
        LoadAvatarTexture();
    }
    
    // Draw rounded avatar + border
    float avatarRounding = 10.0f;
    ImVec2 avatarSize(50, 50);
    ImVec2 avatarCenter = ImVec2(avatarPos.x + 25.0f, avatarPos.y + 25.0f);
    
    if (g_AvatarTexture != 0) {
        dl->AddImageRounded(
            (ImTextureID)(intptr_t)g_AvatarTexture,
            avatarPos,
            ImVec2(avatarPos.x + avatarSize.x, avatarPos.y + avatarSize.y),
            ImVec2(0,0), ImVec2(1,1),
            IM_COL32(255,255,255,255),
            avatarRounding
        );
        // Subtle outline border
        dl->AddRect(avatarPos, ImVec2(avatarPos.x + avatarSize.x, avatarPos.y + avatarSize.y), ImGui::GetColorU32(ImVec4(0.16f, 0.16f, 0.18f, g_MenuAlpha)), avatarRounding, 0, 1.5f);
    } else {
        dl->AddRectFilled(avatarPos, ImVec2(avatarPos.x + avatarSize.x, avatarPos.y + avatarSize.y), ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, g_MenuAlpha)), avatarRounding);
        dl->AddText(ImVec2(avatarCenter.x - 5.0f, avatarCenter.y - 8.0f), ImGui::GetColorU32(ImVec4(1,1,1,1)), "R");
        dl->AddRect(avatarPos, ImVec2(avatarPos.x + avatarSize.x, avatarPos.y + avatarSize.y), ImGui::GetColorU32(ImVec4(0.16f, 0.16f, 0.18f, g_MenuAlpha)), avatarRounding, 0, 1.5f);
    }

    // Parse & render colored nickname safely
    const char* rawPlayerName = "Local Player";
    ePlayer* lp = ePlayer::PlayerConfig(0);
    if (lp) {
        rawPlayerName = static_cast<const char*>(lp->Name());
    }

    const char* connState = "Singleplayer";
    if (sn_GetNetState() == nSERVER) {
        connState = "Hosting Server";
    } else if (sg_OnRemoteServer()) {
        connState = "Multiplayer Client";
    } else if (uMenu::MenuActive()) {
        connState = "Main Menu";
    }

    char dateBuf[64];
    time_t rawtime = time(nullptr);
    struct tm* timeinfo = localtime(&rawtime);
    strftime(dateBuf, sizeof(dateBuf), "%d.%m.%Y", timeinfo);

    // Render parsed colored name + status details
    RenderColoredText(dl, ImVec2(avatarPos.x + 60, avatarPos.y + 10), IM_COL32(255, 255, 255, 255), rawPlayerName);
    dl->AddText(ImVec2(avatarPos.x + 60, avatarPos.y + 25), ImGui::GetColorU32(ImVec4(0.5f,0.5f,0.5f,1)), connState);
    dl->AddText(ImVec2(avatarPos.x + 60, avatarPos.y + 40), GetThemeColor(0.0f), dateBuf);

    // Render modals/popups
    RenderModMenuModals();

    // Draw the media player card below the standalone mod menu when it is open (in-game)
    if (g_MenuOpen) {
        MediaWidget* media = MediaWidget::GetInstance();
        if (media) {
            float bottomGap = io.DisplaySize.y - (wPos.y + currentSize.y);
            ImVec2 mediaPos(
                wPos.x + (currentSize.x - 380.0f) * 0.5f,
                wPos.y + currentSize.y + (bottomGap - 125.0f) * 0.5f
            );
            media->Draw(ImGui::GetForegroundDrawList(), mediaPos, g_MenuAlpha);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    
    if (isInline) {
        style.Colors[ImGuiCol_Border] = savedBorderCol;
        style.WindowBorderSize = savedBorderSize;
        style.WindowRounding = savedRounding;
    }
    
    g_MenuAlpha = savedMenuAlpha;
}

// ----------------------------------------------------
// CUSTOM MAIN MENU DASHBOARD
// ----------------------------------------------------
extern uMenu* g_SettingsMenuPtr;
extern void sg_SinglePlayerGame();
extern void net_game();
extern bool sr_FPSOut;

//! Whether a file in the var directory belongs to the client rather than to the
//! player. The list of profiles is a place to keep your own settings, and every
//! session token, hardware id and half-written state file that appears next to
//! them is noise at best - at worst it invites somebody to apply one and wonder
//! why the game behaves oddly afterwards.
//!
//! Matched by prefix as well as by name, so a file the client starts writing
//! next month does not have to be remembered here.
static bool rc_IsPrivateConfig(std::string const & lowerName) {
    static char const * const kExact[] = {
        "user.cfg", "settings.cfg", "master.cfg", "aiplayers.cfg", "autoexec.cfg",
        "check.cfg", "server_settings.cfg", "ilonium.cfg", "default.cfg",
        "keys_cursor.cfg", "keys_wasd.cfg", "keys_zqsd.cfg", "keys_x.cfg",
        "keys_cursor_single.cfg", "keys_twohand.cfg",
        nullptr
    };
    static char const * const kPrefix[] = {
        "telemetry", "irc_", "rcl_", "ticket_", "noclip_", "saved_", "session",
        nullptr
    };

    for (int i = 0; kExact[i]; ++i) {
        if (lowerName == kExact[i]) return true;
    }
    for (int i = 0; kPrefix[i]; ++i) {
        if (lowerName.rfind(kPrefix[i], 0) == 0) return true;
    }
    return false;
}

static void LoadDashboardConfigs() {
    s_DashboardConfigs.clear();
    tString varDir = tDirectories::Var().GetWritePath("x");
    if (varDir.Len() > 2) {
        varDir = varDir.SubStr(0, varDir.Len() - 2);
    } else {
        varDir = "./";
    }

    tArray<tString> files;
    tDirectories::GetFiles(varDir, tString("*.cfg"), files, tDirectories::eGetFilesFilesOnly);

    for (int i = 0; i < files.Len(); i++) {
        tString filename = files(i);
        std::string s((const char*)filename);
        std::string s_lower = s;
        for (size_t c = 0; c < s_lower.length(); ++c) {
            s_lower[c] = std::tolower(s_lower[c]);
        }
        if (rc_IsPrivateConfig(s_lower)) {
            continue;
        }
        s_DashboardConfigs.push_back(s);
    }
    s_DashboardConfigsLoaded = true;
}

void ConnectToServerHelper(nServerInfoBase* server) {
    if (!server) return;
    
    // Close the dashboard and completely stop rendering ImGui
    g_MenuOpen = false;
    g_MenuAlpha = 0.0f;
    ModMenu::g_CustomMainMenuTempDisabled = true;
    ModMenu::g_MainMenuActive = false;
    SDL_HideCursor();
    SDL_WM_GrabInput(SDL_GRAB_ON);
    
    // Set ImGui MouseDrawCursor to false
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    
    // Do the connection
    ConnectToServer(server);
    
    // Restore the custom menu state once we return
    ModMenu::g_MainMenuActive = true;
    ModMenu::g_CustomMainMenuTempDisabled = false;
    g_MenuOpen = true;
    g_MenuAlpha = 1.0f;
    ShowSystemCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    
    // Purge input queue
    SDL_Event pEvent;
    while (su_GetSDLInput(pEvent)) {}
}

// Captures every setting that can be written back, so a preset can be undone
// once the match is over and ordinary local play is not left with its rules.
static std::string sg_SnapshotSettings() {
    std::ostringstream out;
    tConfItemBase::tConfItemMap const & items = tConfItemBase::GetConfItemMap();
    for (tConfItemBase::tConfItemMap::const_iterator i = items.begin(); i != items.end(); ++i) {
        tConfItemBase* item = i->second;
        if (!item || !item->CanSave() || !item->Writable())
            continue;
        out << item->GetTitle() << " ";
        item->WriteVal(out);
        out << "\n";
    }
    return out.str();
}

static void sg_RestoreSettings(const std::string& snapshot) {
    if (snapshot.empty())
        return;
    // the snapshot covers every saveable setting, server-side ones included,
    // so putting it back needs the standing the game had when it read them
    tCurrentAccessLevel level( tAccessLevel_Owner, true );
    std::istringstream in(snapshot);
    tConfItemBase::LoadAll(in);
}

static bool sg_ApplyPresetFile(const char* name) {
    // Settings are rejected when the caller sits below the level they require,
    // which is why loading one from the menu quietly did nothing.
    tCurrentAccessLevel level( tAccessLevel_Owner, true );

    std::ifstream s;
    if (!tConfItemBase::OpenFile(s, tString(name), tConfItemBase::All)) {
        con << "Could not find " << name << "\n";
        return false;
    }
    tConfItemBase::ReadFile(s);
    return true;
}

void StartLocalGameHelper() {
    // Presets rewrite live settings, so remember what they were.
    std::string restorePoint;
    if (s_LocalPreset) {
        restorePoint = sg_SnapshotSettings();
        if (!sg_ApplyPresetFile("training.cfg"))
            restorePoint.clear();
    }

    // Close the dashboard and completely stop rendering ImGui
    g_MenuOpen = false;
    g_MenuAlpha = 0.0f;
    ModMenu::g_CustomMainMenuTempDisabled = true;
    ModMenu::g_MainMenuActive = false;
    SDL_HideCursor();
    SDL_WM_GrabInput(SDL_GRAB_ON);
    
    // Set ImGui MouseDrawCursor to false
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;
    
    // Do local game start
    sg_SinglePlayerGame();
    
    // Restore the custom menu state once we return
    ModMenu::g_MainMenuActive = true;
    ModMenu::g_CustomMainMenuTempDisabled = false;
    g_MenuOpen = true;
    g_MenuAlpha = 1.0f;
    ShowSystemCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    
    // Put the rules back the way the player had them.
    sg_RestoreSettings(restorePoint);

    // Purge input queue
    SDL_Event pEvent;
    while (su_GetSDLInput(pEvent)) {}
}





// Forward declaration; the definition lives further down.
static void drawPanel(ImDrawList* dl, ImVec2 pos, ImVec2 size, bool active, const char* title);

// ── Demo Player Full Tab ─────────────────────────────────────────────────
// Standalone tab (g_ActiveTab == 6) with file browser, transport controls
// and live playback state — occupies all 3 panels, panel0 is NAVIGATION.


// ── Demo Player: file browser + open controls ────────────────────────────


static void drawPanel(ImDrawList* dl, ImVec2 pos, ImVec2 size, bool active, const char* title) {
    if (pos.x < -500.0f) return; // Hidden panel

    rcTheme const & th = rc_Theme();

    // The whole panel arrives from a little below when the section changes.
    // Small enough that nobody could describe it afterwards, large enough
    // that its absence is what they would notice.
    float arrived = rc_PageEase( g_ActiveTab * 100 + g_ModMenuTab );
    pos.y += ( 1.0f - arrived ) * 12.0f;

    ImVec2 bot = ImVec2(pos.x + size.x, pos.y + size.y);
    float r = th.radiusLarge;

    // Which panel has the attention is said by lifting it, not by drawing a
    // bright line around it. An outline you can name the colour of reads as a
    // placeholder for a design rather than as one.
    float lift = rc_Ease(ImGui::GetID(title), active ? 1.0f : 0.0f, th.settle);

    // Nearly black on purpose. A coloured shadow is a glow, and a glow under
    // every panel is most of what made this look like a toy.
    RenderTextureGlow(pos, size, IM_COL32(0, 0, 0, 70), 18.0f);

    dl->AddRectFilled(pos, bot, rc_Mix(th.surface, th.surfaceHi, lift), r);

    // One hairline picked out along the top edge. Real surfaces catch light
    // where they turn, and that single line does more for depth than an
    // outline all the way round.
    dl->AddLine(ImVec2(pos.x + r, pos.y + 0.5f), ImVec2(bot.x - r, pos.y + 0.5f),
                rc_Mix(rc_Fade(th.text, 0.06f), rc_Fade(th.primary.solid, 0.30f), lift), 1.0f);

    dl->AddRect(pos, bot, rc_Mix(th.line, th.primary.line, lift * 0.28f), r, 0, 1.0f);

    // The heading labels the panel; it is not an advertisement for the accent
    // colour. Quiet, spaced out, and lifted only while in use.
    ImGui::PushFont(g_FontHeader ? g_FontHeader : g_FontDefault);
    {
        ImVec2 was = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(pos.x + 22.0f, pos.y + 18.0f));
        rc_Tracked(title, 1.6f, rc_Mix(th.textDim, th.text, lift));
        ImGui::SetCursorScreenPos(was);
    }
    ImGui::PopFont();

    // The accent appears once per panel, as a short rule that grows in when
    // the panel is picked up. A rule running the full width is a divider;
    // this is a marker, so it stays short.
    float ruleW = (size.x - 44.0f) * (0.18f + 0.32f * lift);
    if (ruleW > 1.0f) {
        float y = pos.y + 46.0f;
        dl->AddRectFilledMultiColor(ImVec2(pos.x + 22.0f, y), ImVec2(pos.x + 22.0f + ruleW, y + 1.5f),
                                    rc_Fade(th.primary.solid, 0.15f + 0.85f * lift),
                                    rc_Fade(th.primary.solid, 0.0f),
                                    rc_Fade(th.primary.solid, 0.0f),
                                    rc_Fade(th.primary.solid, 0.15f + 0.85f * lift));
    }
}


void ModMenu::RunCustomMainMenu() {
    if (!g_Initialized) {
        Init();
    }
    ImGuiIO& io = ImGui::GetIO();
    
    g_MenuOpen = false;
    CheckDisplaySizeRebuild();
    
    // Clear stuck keys
    su_ClearKeys();
    
    // Purge pending SDL events to prevent splash screen enter skip leaking into main menu
    SDL_Event purgeEvent;
    while (su_GetSDLInput(purgeEvent)) {
        // Discard
    }
    
    // Reset ImGui's mouse click state on enter
    for (int i = 0; i < 5; i++) {
        io.MouseDown[i] = false;
        io.MouseClicked[i] = false;
    }
    
    if (!s_DashboardConfigsLoaded) {
        LoadDashboardConfigs();
    }
    
    // Smooth scrolling variables
    static float targetScrollY = 0.0f;
    static float currentScrollY = 0.0f;
    
    // Visual diagnostic flash animation timer
    static float diagFlashTimer = 0.0f;
    
    float startupCooldown = 0.3f; // 0.3s cooldown to prevent input leak
    
    while (g_MainMenuActive && !g_CustomMainMenuTempDisabled && !uMenu::quickexit) {
        CheckDisplaySizeRebuild();
        if (s_PendingReconnect) {
            s_PendingReconnect = false;
            if (sg_hasLastServer) {
                nServerInfoRedirect lastServer(sg_lastServerIP, sg_lastServerPort);
                ConnectToServerHelper(&lastServer);
            }
            continue;
        }
        if (s_PendingConnectServer) {
            nServerInfoBase* s = s_PendingConnectServer;
            s_PendingConnectServer = nullptr;
            bool valid = true;
            if (s != s_DirectRedirectServer) {
                valid = false;
                nServerInfo* check = nServerInfo::GetFirstServer();
                while (check) {
                    if (check == s) {
                        valid = true;
                        break;
                    }
                    check = check->Next();
                }
            }
            if (valid) {
                ConnectToServerHelper(s);
            }
            if (s_DirectRedirectServer && s == s_DirectRedirectServer) {
                delete s_DirectRedirectServer;
                s_DirectRedirectServer = nullptr;
            }
            continue;
        }
        if (s_PendingStartLocalGame) {
            s_PendingStartLocalGame = false;
            StartLocalGameHelper();
            continue;
        }
        if (uMenu::exitToMain) {
            uMenu::exitToMain = false;
        }
        ShowSystemCursor();
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        io.MouseDrawCursor = false;
        
        st_DoToDo();
        tAdvanceFrame();
        
        // SDL event polling

        SDL_Event event;
        while (su_GetSDLInput(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                uMenu::quickexit = uMenu::QuickExit_Total;
                break;
            }
            if (ProcessEvent(&event)) {
                continue;
            }
            su_HandleEvent(event, true);
        }
        

        if (!g_MainMenuActive || g_CustomMainMenuTempDisabled || uMenu::quickexit) {
            break;
        }

        // --- HUD EDITOR MODE OVERLAY IN MAIN MENU ---
        if (isHudEditing) {
            // Update delta time
            // engine clock rather than gettimeofday: that one is POSIX only and
            // monotonic here, so a clock adjustment cannot skew the frame time
            static double lastFrameStamp = 0.0;
            const double frameStamp = tRealSysTimeFloat();
            if (lastFrameStamp > 0.0) {
                const double dt = frameStamp - lastFrameStamp;
                if (dt > 0.0) {
                    io.DeltaTime = (float)dt;
                    lastFrameStamp = frameStamp;
                }
            } else {
                lastFrameStamp = frameStamp;
            }

            ShowSystemCursor();
            io.MouseDrawCursor = true;

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
            ImGui::NewFrame();

            // Same arrival as the other frame path: a transition that only
            // happens on some screens is worse than none at all.
            ImGui::GetStyle().Alpha = 0.88f + 0.12f * rc_PageEase( g_ActiveTab * 100 + g_ModMenuTab );

            ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
            bgDl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.65f)));
            DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);

            HudManager::Update(io.DeltaTime);
            HudManager::Render();

            if (sr_glOut) {
                sr_ResetRenderState(true);
                gLogo::Display();
            }
            
            PaletteFrame();
    ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            
            rSysDep::SwapGL();
            rSysDep::ClearGL();
            continue;
        }
        
        // Setup ImGui frames
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
        ImGui::NewFrame();
        
        // engine clock rather than gettimeofday: that one is POSIX only and
        // monotonic here, so a clock adjustment cannot skew the frame time
        static double lastFrameStamp = 0.0;
        const double frameStamp = tRealSysTimeFloat();
        if (lastFrameStamp > 0.0) {
            const double dt = frameStamp - lastFrameStamp;
            if (dt > 0.0) {
                io.DeltaTime = (float)dt;
                lastFrameStamp = frameStamp;
            }
        } else {
            lastFrameStamp = frameStamp;
        }

        HudManager::Update(io.DeltaTime);
        
        if (startupCooldown > 0.0f) {
            startupCooldown -= io.DeltaTime;
            g_DashboardActionTriggered = false;
            // Clear any clicked mouse inputs as well during cooldown
            for (int i = 0; i < 5; i++) {
                io.MouseDown[i] = false;
                io.MouseClicked[i] = false;
            }
        }
        if (diagFlashTimer > 0.0f) {
            diagFlashTimer -= io.DeltaTime;
        }
        
        // Temporarily set g_MenuAlpha to 1.0 to render with full opacity
        float savedAlpha = g_MenuAlpha;
        g_MenuAlpha = 1.0f;
        
        RCL_PollQueueSummaryTick(g_ActiveTab == 0 || g_ActiveTab == 7);
        
        // Fullscreen Dashboard Window
        ImVec2 cardSize = io.DisplaySize;
        ImVec2 cardPos(0.0f, 0.0f);
        ImGui::SetNextWindowPos(cardPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(cardSize, ImGuiCond_Always);
        // the card is a backdrop; letting it rise on click would bury every
        // panel that draws itself as a separate window on top of it
        ImGuiWindowFlags cardFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus;
                                     
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(g_MenuBgColor.x, g_MenuBgColor.y, g_MenuBgColor.z, g_MenuBgAlpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        
        ImGui::Begin("##DashboardCard", nullptr, cardFlags);
        
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        
        // Draw background particles directly on the window's draw list
        DrawBackgroundParticles(dl, ImVec2(0, 0), io.DisplaySize, 1.0f);
        
        // Accent/RGB update
        float menuTime = (float)ImGui::GetTime();
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(fmodf(menuTime * g_RGBSpeed, 1.0f), 1.0f, 1.0f, r, g, b);
        if (g_RGBAccent) {
            g_AccentColor = ImVec4(r, g, b, 1.0f);
        } else {
            g_AccentColor = g_AccentColor1;
        }
        
        // 3. Header and title
        float paddingX = cardSize.x < 1180.0f ? 20.0f : 50.0f;
        float headerY = 70.0f;
        
        // Neon Title Text
        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        dl->AddText(ImVec2(winPos.x + paddingX, winPos.y + 25.0f), IM_COL32(255, 255, 255, 255), "RETROCYCLES COMPETITIVE DASHBOARD");
        if (g_FontHeader) ImGui::PopFont();
        
        // Subtitle or version tag on top-right
        char verBuf[64];
        if (sn_programVersion == "ILONIUM" || sn_programVersion == "ilonium") {
            snprintf(verBuf, sizeof(verBuf), "ILONIUM | COMPETITIVE EDITION");
        } else {
            snprintf(verBuf, sizeof(verBuf), "v%s | COMPETITIVE EDITION", (const char*)sn_programVersion);
        }
        ImVec2 verSize = ImGui::CalcTextSize(verBuf);
        dl->AddText(ImVec2(winPos.x + cardSize.x - paddingX - verSize.x, winPos.y + 35.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.6f, 0.8f)), verBuf);
        
        // A rule under the title, not a stripe across the product. Three
        // pixels of full strength colour edge to edge was the loudest thing
        // on the screen and the first thing the eye went to, which is a poor
        // use of the one place attention is free.
        {
            rcTheme const & th = rc_Theme();
            float ruleY = winPos.y + headerY;
            ImVec2 from( winPos.x + paddingX, ruleY );
            ImVec2 to( winPos.x + cardSize.x - paddingX, ruleY + 1.0f );
            ImVec2 mid( ( from.x + to.x ) * 0.5f, to.y );

            dl->AddRectFilledMultiColor( from, mid,
                rc_Fade( th.primary.solid, 0.0f ), rc_Fade( th.primary.solid, 0.5f ),
                rc_Fade( th.primary.solid, 0.5f ), rc_Fade( th.primary.solid, 0.0f ) );

            dl->AddRectFilledMultiColor( ImVec2( mid.x, from.y ), to,
                rc_Fade( th.primary.solid, 0.5f ), rc_Fade( th.primary.solid, 0.0f ),
                rc_Fade( th.primary.solid, 0.0f ), rc_Fade( th.primary.solid, 0.5f ) );
        }
        
        // 4. Grid Columns layout (Fullscreen Symmetrical)
        float colY = headerY + 30.0f;
        float colH = cardSize.y - colY - 45.0f;
        
        ImVec2 panel0Pos, panel0Size;
        ImVec2 panel1Pos, panel1Size;
        ImVec2 panel2Pos, panel2Size;
        bool hasPanel2 = (cardSize.x >= 950.0f);
        
        if (hasPanel2) {
            float col0W = 230.0f;
            float col2W = 270.0f;
            float col1W = cardSize.x - 2.0f * paddingX - 40.0f - col0W - col2W;
            if (col1W < 300.0f) col1W = 300.0f;
            
            panel0Pos = ImVec2(winPos.x + paddingX, winPos.y + colY);
            panel0Size = ImVec2(col0W, colH);
            
            panel1Pos = ImVec2(winPos.x + paddingX + col0W + 20.0f, winPos.y + colY);
            panel1Size = ImVec2(col1W, colH);
            
            panel2Pos = ImVec2(winPos.x + paddingX + col0W + 40.0f + col1W, winPos.y + colY);
            panel2Size = ImVec2(col2W, colH);
        } else {
            float col0W = 180.0f;
            float col1W = cardSize.x - 2.0f * paddingX - 20.0f - col0W;
            if (col1W < 200.0f) col1W = 200.0f;
            
            panel0Pos = ImVec2(winPos.x + paddingX, winPos.y + colY);
            panel0Size = ImVec2(col0W, colH);
            
            panel1Pos = ImVec2(winPos.x + paddingX + col0W + 20.0f, winPos.y + colY);
            panel1Size = ImVec2(col1W, colH);
            
            panel2Pos = ImVec2(-1000.0f, -1000.0f);
            panel2Size = ImVec2(0.0f, 0.0f);
        }
        
        // Panel Drawer helper lambda
        // The one panel there is, drawn from one place. Three copies of it is
        // how the values drifted apart in the first place.
        auto drawPanel = [&](ImVec2 pos, ImVec2 size, bool active, const char* title) {
            ::drawPanel(dl, pos, size, active, title);
        };
        
        ImVec2 panelSize = panel1Size; // For compatibility

        
        // --- COLUMN 0: NAVIGATION (Left) ---
        drawPanel(panel0Pos, panel0Size, g_DashboardActiveCol == 0, "NAVIGATION");
        
        const char* navLabels[11] = { "Dashboard", "IRC Global Chat", "Pickup", "Local Play", "Server Browser", "Settings", "Socials", "Misc", "Support Tickets", "Mod Menu", "Exit Game" };
        int mainTabMap[11] = { 0, 10, 7, 1, 2, 3, 8, 11, 9, 4, -1 };
        
        ImGui::SetCursorScreenPos(ImVec2(panel0Pos.x + 5.0f, panel0Pos.y + 50.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        bool showNav = ImGui::BeginChild("##NavigationScroll", ImVec2(panel0Size.x - 10.0f, panel0Size.y - 60.0f), false, ImGuiWindowFlags_NavFlattened);
        if (showNav) {
            ImDrawList* navDl = ImGui::GetWindowDrawList();
            ImVec2 navStart = ImGui::GetCursorScreenPos();
            
            float availH = panel0Size.y - 65.0f;
            float stepY = availH / 11.0f;
            if (stepY > 38.0f) stepY = 38.0f;
            if (stepY < 30.0f) stepY = 30.0f;
            float itemH = stepY - 5.0f;

            for (int i = 0; i < 11; i++) {
                float itemY = navStart.y + i * stepY;
                ImVec2 itemMin(panel0Pos.x + 15.0f, itemY);
                ImVec2 itemMax(panel0Pos.x + panel0Size.x - 15.0f, itemY + itemH);

                bool hovered = io.MousePos.x >= itemMin.x && io.MousePos.x <= itemMax.x &&
                               io.MousePos.y >= itemMin.y && io.MousePos.y <= itemMax.y &&
                               ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
                bool selected = (g_DashboardActiveCol == 0) && (g_DashboardLeftSelected == i);
                
                if (hovered && io.MouseClicked[0]) {
                    g_DashboardLeftSelected = i;
                    g_DashboardActiveCol = 0;
                    g_DashboardActionTriggered = true;
                }
                
                // Highlight selected tab visually
                int tabVal = mainTabMap[i];
                bool isActiveTab = (tabVal >= 0 && g_ActiveTab == tabVal);
                
                rcTheme const & th = rc_Theme();
                bool onNow = selected || isActiveTab;

                // Ten boxes in a column is a list of boxes, not a menu. At
                // rest an entry is only its word; the surface appears under
                // the hand and stays under the one that is open.
                float warm = rc_Ease(ImGui::GetID(navLabels[i]), hovered ? 1.0f : 0.0f);
                float open = rc_Ease(ImGui::GetID(navLabels[i]) ^ 0x9E37u, onNow ? 1.0f : 0.0f, th.settle);

                if (warm > 0.01f || open > 0.01f) {
                    navDl->AddRectFilled(itemMin, itemMax,
                        rc_Mix(rc_Fade(th.surfaceHi, warm * 0.9f), th.primary.wash, open),
                        th.radiusSmall);
                }

                // Where it is is said by a mark at the edge rather than by
                // flooding the whole row with the accent. It grows out of
                // nothing, which is also what makes moving between entries
                // read as movement.
                if (open > 0.01f) {
                    float half = itemH * 0.34f * open;
                    float mid = itemMin.y + itemH * 0.5f;
                    navDl->AddRectFilled(ImVec2(itemMin.x, mid - half),
                                         ImVec2(itemMin.x + 2.5f, mid + half),
                                         th.primary.solid, 2.0f);
                }

                // The word itself carries the state: dim when idle, plain
                // white when it is the one you are on.
                navDl->AddText(ImVec2(itemMin.x + 14.0f + 4.0f * open,
                                      itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                            rc_Mix(rc_Mix(th.textDim, th.text, warm), th.text, open), navLabels[i]);
                
                if (g_DashboardActionTriggered && selected) {
                    g_DashboardActionTriggered = false;
                    int targetTab = mainTabMap[i];
                    if (targetTab == 4) { // Mod Menu (toggle)
                        if (g_ActiveTab == 4) {
                            g_ActiveTab = 0;
                            g_DashboardLeftSelected = 0;
                        } else {
                            g_ActiveTab = 4;
                        }
                    } else if (targetTab == 5) { // Demo Recorder
                        g_ActiveTab = 5;
                    } else if (targetTab == 6) { // Demo Player
                        g_ActiveTab = 6;
                    } else if (targetTab == 8) { // Socials
                        g_ActiveTab = 8;
                    } else if (targetTab == 9) { // Support Tickets
                        g_ActiveTab = 9;
                    } else if (targetTab == 10) { // IRC Global Chat
                        g_ActiveTab = 10;
                    } else if (targetTab == -1) { // Exit Game
                        uMenu::quickexit = uMenu::QuickExit_Total;
                    } else {
                        g_ActiveTab = targetTab;
                    }
                }
            }
            ImGui::Dummy(ImVec2(panel0Size.x - 20.0f, 12 * stepY));
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (g_ActiveTab == 0) {
            RenderOriginalDashboard(dl, io, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 7) {
            RenderEsportsDashboard(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 9) {
            RenderSupportTicketsTab(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 10) {
            RenderIRCChatTab(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 11) {
            RenderMiscTab(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 8) {
            RenderSocialsTab(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, panel1Size.x, colH);
        } else if (g_ActiveTab == 1) {
            // --- TAB 1: LOCAL PLAY ---
            float width1 = panel1Size.x;
            float width2 = hasPanel2 ? panel2Size.x : panel1Size.x;
            
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, hasPanel2 ? "GAMEPLAY RULES" : "LOCAL PLAY CONFIG");
            
            // Render settings inside a child scroll area to prevent overflow
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 60.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("LocalPlayRulesScroll", ImVec2(width1 - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            
            // 1. Game Mode. The last entry is a preset rather than an engine
            // game type: picking it loads a rule set when the match starts.
            const char* modeNames[] = { "Free For All", "Duel", "Human vs AI", "Mazing Training" };
            int currentMode = s_LocalPreset ? 3 : (int)singlePlayer.gameType;
            ImGui::Text("Game Mode:");
            ImGui::SetNextItemWidth(width1 - 50.0f);
            if (ImGui::Combo("##SPMode", &currentMode, modeNames, 4)) {
                if (currentMode == 3) {
                    s_LocalPreset = 1;
                } else {
                    s_LocalPreset = 0;
                    singlePlayer.gameType = (gGameType)currentMode;
                }
            }
            if (s_LocalPreset) {
                ImGui::TextColored(ImVec4(0.55f, 0.6f, 0.72f, 1.0f),
                    "Solo, no bots, no zones. Dying costs your wall and nothing else.");
            }
            
            ImGui::Spacing(); ImGui::Spacing();
            
            // 2. Game Speed
            float speedFactVal = (float)singlePlayer.speedFactor;
            float speedMult = exp(speedFactVal);
            ImGui::Text("Game Speed Multiplier (%.2fx):", speedMult);
            ImGui::SetNextItemWidth(width1 - 50.0f);
            if (ImGui::SliderFloat("##SPSpeed", &speedFactVal, -1.0f, 1.0f, "%.2f")) {
                singlePlayer.speedFactor = speedFactVal;
            }
            
            ImGui::Spacing(); ImGui::Spacing();
            
            // 3. Grid Size
            float sizeFactVal = (float)singlePlayer.sizeFactor;
            float sizeMult = exp(sizeFactVal);
            ImGui::Text("Arena Size Multiplier (%.2fx):", sizeMult);
            ImGui::SetNextItemWidth(width1 - 50.0f);
            if (ImGui::SliderFloat("##SPSize", &sizeFactVal, -1.5f, 1.5f, "%.2f")) {
                singlePlayer.sizeFactor = sizeFactVal;
            }
            
            ImGui::Spacing(); ImGui::Spacing();
            
            // 4. Walls Stay Up Delay
            ImGui::Text("Wall Stay-up Delay (sec, <0 for infinite):");
            ImGui::SetNextItemWidth(width1 - 50.0f);
            float wDelay = (float)singlePlayer.wallsStayUpDelay;
            if (ImGui::SliderFloat("##SPWallDelay", &wDelay, -1.0f, 10.0f, "%.1f")) {
                singlePlayer.wallsStayUpDelay = wDelay;
            }
            
            ImGui::Spacing(); ImGui::Spacing();
            
            // 5. Walls Length
            ImGui::Text("Max Wall Length:");
            ImGui::SetNextItemWidth(width1 - 50.0f);
            float wLen = (float)singlePlayer.wallsLength;
            if (ImGui::SliderFloat("##SPWallLen", &wLen, -1.0f, 1000.0f, "%.0f")) {
                singlePlayer.wallsLength = wLen;
            }
            
            if (!hasPanel2) {
                // If single column layout, stack the Match Config and Start Button directly inside the scroll pane
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "MATCH CONFIG & LAUNCH");
                ImGui::Spacing();
                
                // 1. Number of AIs
                ImGui::Text("AI Opponents count:");
                ImGui::SetNextItemWidth(width1 - 50.0f);
                ImGui::SliderInt("##SPAIs", &singlePlayer.numAIs, 0, 15);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 2. AI Intelligence (IQ)
                ImGui::Text("AI Difficulty level:");
                ImGui::SetNextItemWidth(width1 - 50.0f);
                ImGui::SliderInt("##SPAI_IQ", &singlePlayer.AI_IQ, 0, 100);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 3. Auto AI count adjustment
                bool autoAI = singlePlayer.autoNum;
                if (ImGui::Checkbox("Auto-adjust AI count", &autoAI)) {
                    singlePlayer.autoNum = autoAI;
                }
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 4. Score Limit
                ImGui::Text("Match Score Limit:");
                ImGui::SetNextItemWidth(width1 - 50.0f);
                ImGui::InputInt("##SPScoreLimit", &singlePlayer.limitScore);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 5. Round Limit
                ImGui::Text("Match Round Limit:");
                ImGui::SetNextItemWidth(width1 - 50.0f);
                ImGui::InputInt("##SPRoundLimit", &singlePlayer.limitRounds);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                if (ImGui::Button("START LOCAL GAME", ImVec2(width1 - 50.0f, 50.0f))) {
                    s_PendingStartLocalGame = true;
                }
            }
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            if (hasPanel2) {
                // --- COLUMN 2: MATCH LIMITS & START MATCH (Right) ---
                drawPanel(panel2Pos, panel2Size, g_DashboardActiveCol == 2, "MATCH CONFIG & LAUNCH");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 15.0f, panel2Pos.y + 60.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::BeginChild("LocalPlayLimitsScroll", ImVec2(width2 - 30.0f, colH - 150.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
                
                // 1. Number of AIs
                ImGui::Text("AI Opponents count:");
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::SliderInt("##SPAIs", &singlePlayer.numAIs, 0, 15);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 2. AI Intelligence (IQ)
                ImGui::Text("AI Difficulty level:");
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::SliderInt("##SPAI_IQ", &singlePlayer.AI_IQ, 0, 100);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 3. Auto AI count adjustment
                bool autoAI = singlePlayer.autoNum;
                if (ImGui::Checkbox("Auto-adjust AI count", &autoAI)) {
                    singlePlayer.autoNum = autoAI;
                }
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 4. Score Limit
                ImGui::Text("Match Score Limit:");
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputInt("##SPScoreLimitWide", &singlePlayer.limitScore);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 5. Round Limit
                ImGui::Text("Match Round Limit:");
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputInt("##SPRoundLimitWide", &singlePlayer.limitRounds);
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                
                // Big start button
                float startBtnY = panel2Pos.y + colH - 70.0f;
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, startBtnY));
                if (ImGui::Button("START LOCAL GAME", ImVec2(width2 - 40.0f, 50.0f))) {
                    s_PendingStartLocalGame = true;
                }
            }
        } else if (g_ActiveTab == 2) {
            // --- TAB 2: SERVER BROWSER ---
            static nServerInfo* s_SelectedServer = nullptr;
            static int s_SelectedFavoriteIdx = -1;
            static bool s_ServerQueryStarted = false;
            static char s_DirectIP[128] = "";
            static int s_DirectPort = 4534;
            static bool s_ShowFavoritesOnly = false;
            static char s_ServerSearchBuf[128] = "";
            
            // Poll query/network events so the list is updated
            sn_Receive();
            sn_SendPlanned();
            nServerInfo::DoQueryAll(32); // batch of 32 simultaneous UDP pings
            
            // Validate selected server pointer to prevent crash from dangling pointers
            bool selectedServerValid = false;
            if (s_SelectedServer) {
                nServerInfo* check = nServerInfo::GetFirstServer();
                while (check) {
                    if (check == s_SelectedServer) {
                        selectedServerValid = true;
                        break;
                    }
                    check = check->Next();
                }
            }
            if (!selectedServerValid) {
                s_SelectedServer = nullptr;
            }
            
            if (!s_ServerQueryStarted) {
                s_ServerQueryStarted = true;
                nServerInfo::DeleteAll(false);
                nServerInfo::GetFromMaster();
                nServerInfo::GetFromLANContinuously();
                nServerInfo::StartQueryAll(nServerInfo::QUERY_OPTOUT);
            }
            
            float width1 = panel1Size.x;
            float width2 = hasPanel2 ? panel2Size.x : panel1Size.x;
            
            // --- COLUMN 1: SERVER LIST ---
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, "SERVER BROWSER");
            
            // Render Refresh button and optional Direct Connect button in header area of Column 1
            float refreshBtnW = 75.0f;
            float directBtnW = 60.0f;
            float refreshBtnH = 26.0f;
            ImVec2 refreshBtnMin, refreshBtnMax;
            ImVec2 directBtnMin, directBtnMax;
            if (hasPanel2) {
                refreshBtnW = 100.0f;
                refreshBtnMin = ImVec2(panel1Pos.x + width1 - 20.0f - refreshBtnW, panel1Pos.y + 14.0f);
                refreshBtnMax = ImVec2(refreshBtnMin.x + refreshBtnW, refreshBtnMin.y + refreshBtnH);
            } else {
                refreshBtnMin = ImVec2(panel1Pos.x + width1 - 20.0f - refreshBtnW - directBtnW - 10.0f, panel1Pos.y + 14.0f);
                refreshBtnMax = ImVec2(refreshBtnMin.x + refreshBtnW, refreshBtnMin.y + refreshBtnH);
                directBtnMin = ImVec2(panel1Pos.x + width1 - 20.0f - directBtnW, panel1Pos.y + 14.0f);
                directBtnMax = ImVec2(directBtnMin.x + directBtnW, directBtnMin.y + refreshBtnH);
            }
            
            bool refHovered = io.MousePos.x >= refreshBtnMin.x && io.MousePos.x <= refreshBtnMax.x &&
                              io.MousePos.y >= refreshBtnMin.y && io.MousePos.y <= refreshBtnMax.y;
            dl->AddRectFilled(refreshBtnMin, refreshBtnMax, refHovered ? IM_COL32(40, 40, 50, 200) : IM_COL32(20, 20, 25, 150), 4.0f);
            dl->AddRect(refreshBtnMin, refreshBtnMax, IM_COL32(80, 80, 100, 100), 4.0f);
            dl->AddText(ImVec2(refreshBtnMin.x + (refreshBtnW - ImGui::CalcTextSize("Refresh").x)*0.5f, refreshBtnMin.y + (refreshBtnH - ImGui::GetTextLineHeight())*0.5f),
                        IM_COL32(200, 200, 200, 255), "Refresh");
            if (refHovered && io.MouseClicked[0]) {
                nServerInfo::DeleteAll(false);
                nServerInfo::GetFromMaster();
                nServerInfo::GetFromLANContinuously();
                nServerInfo::StartQueryAll(nServerInfo::QUERY_OPTOUT);
                s_SelectedServer = nullptr;
                s_SelectedFavoriteIdx = -1;
            }
            
            if (!hasPanel2) {
                bool dirHovered = io.MousePos.x >= directBtnMin.x && io.MousePos.x <= directBtnMax.x &&
                                  io.MousePos.y >= directBtnMin.y && io.MousePos.y <= directBtnMax.y;
                dl->AddRectFilled(directBtnMin, directBtnMax, dirHovered ? IM_COL32(40, 40, 50, 200) : IM_COL32(20, 20, 25, 150), 4.0f);
                dl->AddRect(directBtnMin, directBtnMax, IM_COL32(80, 80, 100, 100), 4.0f);
                dl->AddText(ImVec2(directBtnMin.x + (directBtnW - ImGui::CalcTextSize("Direct").x)*0.5f, directBtnMin.y + (refreshBtnH - ImGui::GetTextLineHeight())*0.5f),
                            IM_COL32(200, 200, 200, 255), "Direct");
                if (dirHovered && io.MouseClicked[0]) {
                    g_OpenDirectConnectModal = true;
                }
            }
            
            // Render Sort options below header
            // These three sit shoulder to shoulder on the same surface, and
            // with no edge between them the row read as one crooked block with
            // words floating on it. An outline each is all it needs - the
            // colour was never the problem.
            ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4( rc_Theme().lineStrong ));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 20.0f, panel1Pos.y + 50.0f));
            ImGui::PushItemWidth(110.0f);
            const char* sortKeys[] = { "Name", "Ping", "Players", "Score" };
            static int selectedSort = 3; // default to Score
            if (ImGui::Combo("Sort By", &selectedSort, sortKeys, 4)) {
                nServerInfo::PrimaryKey pKey = nServerInfo::KEY_SCORE;
                if (selectedSort == 0) pKey = nServerInfo::KEY_NAME;
                else if (selectedSort == 1) pKey = nServerInfo::KEY_PING;
                else if (selectedSort == 2) pKey = nServerInfo::KEY_USERS;
                else if (selectedSort == 3) pKey = nServerInfo::KEY_SCORE;
                nServerInfo::Sort(pKey);
            }
            ImGui::PopItemWidth();
            
            // Favorites filter checkbox next to sort
            ImGui::SameLine(0.0f, 26.0f);
            bool prevShowFavoritesOnly = s_ShowFavoritesOnly;
            if (ImGui::Checkbox("Favs Only", &s_ShowFavoritesOnly)) {
                if (s_ShowFavoritesOnly != prevShowFavoritesOnly) {
                    s_SelectedServer = nullptr;
                    s_SelectedFavoriteIdx = -1;
                }
            }
            
            // Periodic sorting update
            static int sortCooldown = 0;
            if (sortCooldown++ > 30) {
                sortCooldown = 0;
                nServerInfo::PrimaryKey pKey = nServerInfo::KEY_SCORE;
                if (selectedSort == 0) pKey = nServerInfo::KEY_NAME;
                else if (selectedSort == 1) pKey = nServerInfo::KEY_PING;
                else if (selectedSort == 2) pKey = nServerInfo::KEY_USERS;
                else if (selectedSort == 3) pKey = nServerInfo::KEY_SCORE;
                nServerInfo::Sort(pKey);
            }
            
            // Search input box
            // Far enough below the row above that the two do not touch.
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 20.0f, panel1Pos.y + 92.0f));
            ImGui::PushItemWidth(width1 - 110.0f);
            ImGui::InputTextWithHint("##Search", "Search servers...", s_ServerSearchBuf, sizeof(s_ServerSearchBuf));
            ImGui::PopItemWidth();
            if (s_ServerSearchBuf[0] != '\0') {
                ImGui::SameLine(0.0f, 8.0f);
                if (ImGui::Button("Clear", ImVec2(50.0f, 0.0f))) {
                    s_ServerSearchBuf[0] = '\0';
                }
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            // Server List Scrollable Child Area
            //
            // Moved down with the search field above it. The two were within a
            // few pixels of each other, which is the same complaint as the row
            // above by another name.
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 132.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("ServerListScroll", ImVec2(width1 - 30.0f, colH - 152.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImDrawList* child_dl = ImGui::GetWindowDrawList();
            child_dl->PushClipRect(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 132.0f), ImVec2(panel1Pos.x + width1 - 15.0f, panel1Pos.y + colH - 20.0f), true);
            
            int serverIdx = 0;
            if (s_ShowFavoritesOnly) {
                int numFavs = gServerFavorites::GetNumFavorites();
                for (int i = 0; i < numFavs; i++) {
                    tString favName, favAddress;
                    int favPort;
                    if (!gServerFavorites::GetFavoriteInfo(i, favName, favAddress, favPort)) {
                        continue;
                    }
                    if (s_ServerSearchBuf[0] != '\0') {
                        std::string needle(s_ServerSearchBuf);
                        std::string rawName = static_cast<const char*>(favName);
                        tString tName(favName);
                        std::string strippedName = static_cast<const char*>(tColoredString::RemoveColors(tName));
                        if (!CaseInsensitiveSubstringSearch(rawName, needle) &&
                            !CaseInsensitiveSubstringSearch(strippedName, needle)) {
                            continue;
                        }
                    }
                    
                    ImVec2 curPos = ImGui::GetCursorScreenPos();
                    float itemH = 45.0f;
                    
                    bool itemHovered = io.MousePos.x >= curPos.x && io.MousePos.x <= curPos.x + (width1 - 30.0f) &&
                                       io.MousePos.y >= curPos.y && io.MousePos.y <= curPos.y + itemH;
                    bool isSelected = (i == s_SelectedFavoriteIdx);
                    
                    if (itemHovered && io.MouseClicked[0]) {
                        s_SelectedFavoriteIdx = i;
                        s_SelectedServer = nullptr;
                        if (!hasPanel2) {
                            g_OpenServerDetailsModal = true;
                        }
                    }
                    
                    ImU32 itemBg = isSelected ? GetThemeColor(0.2f) : (itemHovered ? IM_COL32(30, 30, 35, 150) : IM_COL32(15, 15, 18, 100));
                    ImU32 itemBorder = isSelected ? GetThemeColor(0.5f) : IM_COL32(40, 40, 50, 80);
                    
                    child_dl->AddRectFilled(curPos, ImVec2(curPos.x + width1 - 45.0f, curPos.y + itemH - 5.0f), itemBg, 6.0f);
                    child_dl->AddRect(curPos, ImVec2(curPos.x + width1 - 45.0f, curPos.y + itemH - 5.0f), itemBorder, 6.0f);
                    
                    RenderColoredText(child_dl, ImVec2(curPos.x + 10.0f, curPos.y + 6.0f), IM_COL32(255,255,255,255), TruncateColoredString((const char*)favName, 60).c_str());
                    
                    char statsBuf[128];
                    snprintf(statsBuf, sizeof(statsBuf), "%s:%d", (const char*)favAddress, favPort);
                    child_dl->AddText(ImVec2(curPos.x + 10.0f, curPos.y + 22.0f), IM_COL32(150, 150, 160, 255), statsBuf);
                    
                    if (itemHovered && io.MouseDoubleClicked[0]) {
                        s_SelectedFavoriteIdx = i;
                        s_SelectedServer = nullptr;
                        if (s_DirectRedirectServer) {
                            delete s_DirectRedirectServer;
                        }
                        s_DirectRedirectServer = new nServerInfoRedirect(favAddress, favPort);
                        s_PendingConnectServer = s_DirectRedirectServer;
                        break;
                    }
                    
                    ImGui::Dummy(ImVec2(width1 - 30.0f, itemH));
                    serverIdx++;
                }
            } else {
                nServerInfo* s = nServerInfo::GetFirstServer();
                while (s) {
                    if (s->Reachable()) {
                        if (s_ServerSearchBuf[0] != '\0') {
                            std::string needle(s_ServerSearchBuf);
                            std::string rawName = (s->GetName() ? static_cast<const char*>(s->GetName()) : "");
                            tString tName(rawName.c_str());
                            std::string strippedName = static_cast<const char*>(tColoredString::RemoveColors(tName));
                            if (!CaseInsensitiveSubstringSearch(rawName, needle) &&
                                !CaseInsensitiveSubstringSearch(strippedName, needle)) {
                                s = s->Next();
                                continue;
                            }
                        }
                        ImVec2 curPos = ImGui::GetCursorScreenPos();
                        float itemH = 45.0f;
                        
                        bool itemHovered = io.MousePos.x >= curPos.x && io.MousePos.x <= curPos.x + (width1 - 30.0f) &&
                                           io.MousePos.y >= curPos.y && io.MousePos.y <= curPos.y + itemH;
                        bool isSelected = (s == s_SelectedServer);
                        
                        if (itemHovered && io.MouseClicked[0]) {
                            s_SelectedServer = s;
                            s_SelectedFavoriteIdx = -1;
                            if (!hasPanel2) {
                                g_OpenServerDetailsModal = true;
                            }
                        }
                        
                        ImU32 itemBg = isSelected ? GetThemeColor(0.2f) : (itemHovered ? IM_COL32(30, 30, 35, 150) : IM_COL32(15, 15, 18, 100));
                        ImU32 itemBorder = isSelected ? GetThemeColor(0.5f) : IM_COL32(40, 40, 50, 80);
                        
                        child_dl->AddRectFilled(curPos, ImVec2(curPos.x + width1 - 45.0f, curPos.y + itemH - 5.0f), itemBg, 6.0f);
                        child_dl->AddRect(curPos, ImVec2(curPos.x + width1 - 45.0f, curPos.y + itemH - 5.0f), itemBorder, 6.0f);
                        
                        RenderColoredText(child_dl, ImVec2(curPos.x + 10.0f, curPos.y + 6.0f), IM_COL32(255,255,255,255), TruncateColoredString((const char*)s->GetName(), 60).c_str());
                        
                        char statsBuf[128];
                        snprintf(statsBuf, sizeof(statsBuf), "%d/%d  |  %dms", s->Users(), s->MaxUsers(), (int)(s->Ping() * 1000.0f));
                        child_dl->AddText(ImVec2(curPos.x + 10.0f, curPos.y + 22.0f), IM_COL32(150, 150, 160, 255), statsBuf);
                        
                        if (itemHovered && io.MouseDoubleClicked[0]) {
                            s_SelectedServer = s;
                            s_SelectedFavoriteIdx = -1;
                            s_PendingConnectServer = s_SelectedServer;
                            break;
                        }
                        
                        ImGui::Dummy(ImVec2(width1 - 30.0f, itemH));
                        serverIdx++;
                    }
                    s = s->Next();
                }
            }
            child_dl->PopClipRect();
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            if (hasPanel2) {
                // --- COLUMN 2: SELECTED SERVER INFO & DIRECT CONNECT ---
                drawPanel(panel2Pos, panel2Size, g_DashboardActiveCol == 2, "SERVER INFO");
                
                float infoY = panel2Pos.y + 65.0f;
                auto drawInfoRow = [&](const char* label, const char* val) {
                    dl->AddText(ImVec2(panel2Pos.x + 25.0f, infoY), ImGui::GetColorU32(ImVec4(0.55f, 0.55f, 0.60f, 1.0f)), label);
                    
                    tString valStr(val);
                    tString strippedVal = tColoredString::RemoveColors(valStr);
                    std::string valStd = static_cast<const char*>(strippedVal);
                    if (valStd.length() > 22) {
                        valStd = valStd.substr(0, 19) + "...";
                    }
                    
                    float valWidth = ImGui::CalcTextSize(valStd.c_str()).x;
                    float startX = panel2Pos.x + width2 - 25.0f - valWidth;
                    
                    if (strcmp(label, "Name:") == 0) {
                        std::string truncatedColorStr = TruncateColoredString(val, 19);
                        RenderColoredText(dl, ImVec2(startX, infoY), IM_COL32(255, 255, 255, 255), truncatedColorStr.c_str());
                    } else {
                        dl->AddText(ImVec2(startX, infoY), IM_COL32(255, 255, 255, 255), valStd.c_str());
                    }
                    
                    dl->AddLine(ImVec2(panel2Pos.x + 20.0f, infoY + 28.0f), ImVec2(panel2Pos.x + width2 - 20.0f, infoY + 28.0f),
                                ImGui::GetColorU32(ImVec4(0.11f, 0.11f, 0.13f, 0.4f)));
                    infoY += 35.0f;
                };
                
                if (s_SelectedServer) {
                    drawInfoRow("Name:", (const char*)s_SelectedServer->GetName());
                    char pingBuf[32];
                    snprintf(pingBuf, sizeof(pingBuf), "%d ms", (int)(s_SelectedServer->Ping() * 1000.0f));
                    drawInfoRow("Latency:", pingBuf);
                    char playersBuf[32];
                    snprintf(playersBuf, sizeof(playersBuf), "%d / %d", s_SelectedServer->Users(), s_SelectedServer->MaxUsers());
                    drawInfoRow("Players:", playersBuf);
                    
                    ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, infoY + 10.0f));
                    if (ImGui::Button("JOIN SERVER", ImVec2(width2 - 40.0f, 40.0f))) {
                        s_PendingConnectServer = s_SelectedServer;
                    }
                    
                    // Add/Remove Favorite button
                    bool isFav = gServerFavorites::IsFavorite(s_SelectedServer);
                    ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, infoY + 60.0f));
                    if (isFav) {
                        if (ImGui::Button("REMOVE FROM FAVORITES", ImVec2(width2 - 40.0f, 30.0f))) {
                            gServerFavorites::RemoveFavorite(s_SelectedServer);
                        }
                    } else {
                        if (ImGui::Button("ADD TO FAVORITES", ImVec2(width2 - 40.0f, 30.0f))) {
                            gServerFavorites::AddFavorite(s_SelectedServer);
                        }
                    }

                    // Render players online list
                    float pListY = infoY + 100.0f;
                    // Twenty pixels taller than it was, because the room between the
                    // parts had to come from somewhere and the panel's foot is
                    // not it.
                    float dcStartY = panel2Pos.y + colH - 200.0f;
                    if (dcStartY > pListY + 40.0f) {
                        dl->AddLine(ImVec2(panel2Pos.x + 20.0f, pListY), ImVec2(panel2Pos.x + width2 - 20.0f, pListY), ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.25f, 0.6f)));
                        dl->AddText(ImVec2(panel2Pos.x + 25.0f, pListY + 5.0f), GetThemeColor(0.3f), "PLAYERS ONLINE");
                        
                        float listStartY = pListY + 25.0f;
                        float listHeight = dcStartY - 10.0f - listStartY;
                        
                        std::string oneLine = (const char*)s_SelectedServer->UserNamesOneLine();
                        std::vector<std::string> players;
                        std::size_t start = 0;
                        while (true) {
                            std::size_t end = oneLine.find(", ", start);
                            if (end == std::string::npos) {
                                std::string last = oneLine.substr(start);
                                if (!last.empty() && last != "No Info" && last != "UNKNOWN" && last.find("Sever polled over master server") == std::string::npos) {
                                    players.push_back(last);
                                }
                                break;
                            }
                            std::string p = oneLine.substr(start, end - start);
                            if (!p.empty() && p != "No Info" && p != "UNKNOWN" && p.find("Sever polled over master server") == std::string::npos) {
                                players.push_back(p);
                            }
                            start = end + 2;
                        }

                        ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, listStartY));
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                        if (ImGui::BeginChild("##SelectedServerPlayers", ImVec2(width2 - 40.0f, listHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                            if (players.empty()) {
                                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.8f), "No players online");
                            } else {
                                for (const auto& playerStr : players) {
                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.4f));
                                    ImGui::BeginChildFrame(ImGui::GetID(playerStr.c_str()), ImVec2(width2 - 65.0f, 30.0f));
                                    
                                    ImDrawList* cellDl = ImGui::GetWindowDrawList();
                                    ImVec2 framePos = ImGui::GetWindowPos();
                                    RenderColoredText(cellDl, ImVec2(framePos.x + 8.0f, framePos.y + 6.0f), IM_COL32(255, 255, 255, 255), playerStr.c_str());
                                    
                                    ImGui::EndChildFrame();
                                    ImGui::PopStyleColor();
                                    ImGui::PopStyleVar();
                                    ImGui::Spacing();
                                }
                            }
                        }
                        ImGui::EndChild();
                        ImGui::PopStyleColor();
                    }
                } else if (s_SelectedFavoriteIdx != -1) {
                    tString favName, favAddress;
                    int favPort;
                    if (gServerFavorites::GetFavoriteInfo(s_SelectedFavoriteIdx, favName, favAddress, favPort)) {
                        drawInfoRow("Name:", (const char*)favName);
                        char addrBuf[128];
                        snprintf(addrBuf, sizeof(addrBuf), "%s:%d", (const char*)favAddress, favPort);
                        drawInfoRow("Address:", addrBuf);
                        
                        ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, infoY + 10.0f));
                        if (ImGui::Button("JOIN SERVER", ImVec2(width2 - 40.0f, 40.0f))) {
                            if (s_DirectRedirectServer) {
                                delete s_DirectRedirectServer;
                            }
                            s_DirectRedirectServer = new nServerInfoRedirect(favAddress, favPort);
                            s_PendingConnectServer = s_DirectRedirectServer;
                        }
                        
                        ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, infoY + 60.0f));
                        if (ImGui::Button("REMOVE FROM FAVORITES", ImVec2(width2 - 40.0f, 30.0f))) {
                            gServerFavorites::RemoveFavoriteByIndex(s_SelectedFavoriteIdx);
                            s_SelectedFavoriteIdx = -1;
                        }
                    } else {
                        s_SelectedFavoriteIdx = -1;
                    }
                } else {
                    dl->AddText(ImVec2(panel2Pos.x + 25.0f, infoY + 20.0f), IM_COL32(150, 150, 150, 255), "Select a server to view details.");
                }
                
                // Twenty pixels taller than it was, because the room between the
                    // parts had to come from somewhere and the panel's foot is
                    // not it.
                    float dcStartY = panel2Pos.y + colH - 200.0f;
                dl->AddLine(ImVec2(panel2Pos.x + 20.0f, dcStartY), ImVec2(panel2Pos.x + width2 - 20.0f, dcStartY), ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.25f, 0.6f)));
                dl->AddText(ImVec2(panel2Pos.x + 25.0f, dcStartY + 5.0f), GetThemeColor(0.3f), "DIRECT CONNECT");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 25.0f));
                ImGui::Text("IP Address:");
                // The address, the port with its two steppers and the button
                // below all shared one surface and touched each other, so the
                // whole corner read as a single shape. Edges and a little room
                // between them is all it wants.
                ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4( rc_Theme().lineStrong ));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 45.0f));
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputText("##DirectIP", s_DirectIP, sizeof(s_DirectIP));
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 86.0f));
                ImGui::Text("Port:");
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 106.0f));
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputInt("##DirectPort", &s_DirectPort);
                
                // The one thing here that does something keeps its distance
                // from the things that only describe what it will do.
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, dcStartY + 152.0f));
                if (ImGui::Button("CONNECT DIRECTLY", ImVec2(width2 - 40.0f, 35.0f))) {
                    if (s_DirectRedirectServer) {
                        delete s_DirectRedirectServer;
                    }
                    s_DirectRedirectServer = new nServerInfoRedirect(tString(s_DirectIP), s_DirectPort);
                    s_PendingConnectServer = s_DirectRedirectServer;
                }
            }
        } else if (g_ActiveTab == 3) {
            // --- TAB 3: SYSTEM SETTINGS ---
            float width1 = panel1Size.x;
            float width2 = hasPanel2 ? panel2Size.x : panel1Size.x;
            
            
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, hasPanel2 ? "PROFILE & VIDEO" : "PROFILE, VIDEO & KEYS");
            
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 60.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("GeneralSettingsScroll", ImVec2(width1 - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);

            DrawSettingsSearch(width1);

            // --- SECTION 1: PLAYER PROFILE ---
            if (!SettingsFiltering()) {
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "PLAYER PROFILE");
                ImGui::Separator();
                ImGui::Spacing();
            }
            
            static char s_NickBuf[128] = "";
            static char s_GlobalIDBuf[128] = "";
            static bool s_ProfileInit = false;
            
            ePlayer* lp = ePlayer::PlayerConfig(0);
            if (lp && !s_ProfileInit) {
                s_ProfileInit = true;
                std::string nickUtf8 = Cp1251ToUtf8(static_cast<const char*>(lp->name));
                strncpy(s_NickBuf, nickUtf8.c_str(), sizeof(s_NickBuf)-1);
                std::string gidUtf8 = Cp1251ToUtf8(static_cast<const char*>(lp->globalID));
                strncpy(s_GlobalIDBuf, gidUtf8.c_str(), sizeof(s_GlobalIDBuf)-1);
            }
            
            if (SettingsShow("Nickname", "player name")) {
                flt::Label("Nickname:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::InputText("##Nickname", s_NickBuf, sizeof(s_NickBuf))) {
                    if (lp) {
                        lp->name = Utf8ToCp1251(s_NickBuf);
                    }
                }
            }
            
            if (SettingsShow("Global ID", "authentication login name")) {
                flt::Label("Global ID:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::InputText("##GlobalID", s_GlobalIDBuf, sizeof(s_GlobalIDBuf))) {
                    if (lp) {
                        lp->globalID = Utf8ToCp1251(s_GlobalIDBuf);
                    }
                }
            }
            
            if (lp) {
                int colR = lp->rgb[0];
                int colG = lp->rgb[1];
                int colB = lp->rgb[2];
                
                flt::Label("Color Red:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::SliderInt("##ColorRed", &colR, 0, 255)) {
                    lp->rgb[0] = colR;
                }
                
                flt::Label("Color Green:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::SliderInt("##ColorGreen", &colG, 0, 255)) {
                    lp->rgb[1] = colG;
                }
                
                flt::Label("Color Blue:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::SliderInt("##ColorBlue", &colB, 0, 255)) {
                    lp->rgb[2] = colB;
                }
            }
            
            ImGui::Spacing();
            if (ImGui::Button("SAVE PROFILE", ImVec2(width1 - 55.0f, 30.0f))) {
                st_SaveConfig();
                static nVersionFeature inGameRenames(5);
                if (inGameRenames.Supported()) {
                    ePlayerNetID::Update();
                    ePlayer::SendAuthNames();
                }
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 2: VIDEO & GRAPHICS ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "VIDEO & GRAPHICS");
            ImGui::Separator();
            ImGui::Spacing();
            
            flt::Checkbox("Fullscreen##mainmenu", &currentScreensetting.fullscreen);
            
            flt::Checkbox("Grab Mouse##mainmenu", &su_mouseGrab);
            
            const char* vsyncModes[] = { "On", "Default", "Off", "Motion Blur" };
            int vsyncIdx = (int)currentScreensetting.vSync;
            flt::Label("VSync Mode:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (flt::Combo("##VSyncMode", &vsyncIdx, vsyncModes, IM_ARRAYSIZE(vsyncModes))) {
                currentScreensetting.vSync = (rVSync)vsyncIdx;
            }
            
            const char* resNames[] = {
                "Desktop", "320x200", "320x240", "400x300",
                "512x384", "640x480", "800x600", "1024x768",
                "1280x800", "1280x854", "1280x1024", "1600x1200",
                "1680x1050", "2048x1572"
            };
            int resIdx = (int)currentScreensetting.res.res;
            if (resIdx < 0 || resIdx >= IM_ARRAYSIZE(resNames)) resIdx = 0;
            flt::Label("Resolution:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (flt::Combo("##Resolution", &resIdx, resNames, IM_ARRAYSIZE(resNames))) {
                currentScreensetting.res.res = (rResolution)resIdx;
                currentScreensetting.res.UpdateSize();
            }
            
            flt::Label("Max FPS (0 for Off):");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            ImGui::InputInt("##MaxFPS", &sr_maxFPS);
            
            const char* depthNames[] = { "16-bit", "Desktop", "32-bit" };
            int zDepthIdx = (int)currentScreensetting.zDepth;
            flt::Label("Z-Depth:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (flt::Combo("##ZDepth", &zDepthIdx, depthNames, IM_ARRAYSIZE(depthNames))) {
                currentScreensetting.zDepth = (rColorDepth)zDepthIdx;
            }
            
            int colDepthIdx = (int)currentScreensetting.colorDepth;
            flt::Label("Color Depth:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (flt::Combo("##ColorDepth", &colDepthIdx, depthNames, IM_ARRAYSIZE(depthNames))) {
                currentScreensetting.colorDepth = (rColorDepth)colDepthIdx;
            }
            
            ImGui::Spacing();
            if (ImGui::Button("APPLY VIDEO SETTINGS", ImVec2(width1 - 55.0f, 30.0f))) {
                sr_ReinitDisplay();
                st_SaveConfig();
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 3: CAMERA SETTINGS ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "CAMERA SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();
            
            if (lp) {
                flt::Label("Field of View (FOV):");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::SliderInt("##FOV", &lp->startFOV, 30, 160)) rc_CameraFOV(lp->startFOV);
                
                flt::Checkbox("Auto-Switch In-Cam", &lp->autoSwitchIncam);
                flt::Checkbox("Wobble In-Cam", &lp->wobbleIncam);
                flt::Checkbox("Center In-Cam on Turn", &lp->centerIncamOnTurn);
                
                flt::Checkbox("Allow Smart Camera", &lp->allowCam[CAMERA_SMART]);
                flt::Checkbox("Allow Follow Camera", &lp->allowCam[CAMERA_FOLLOW]);
                flt::Checkbox("Allow Free Camera", &lp->allowCam[CAMERA_FREE]);
                flt::Checkbox("Allow Custom Camera", &lp->allowCam[CAMERA_CUSTOM]);
                flt::Checkbox("Allow Server Custom Camera", &lp->allowCam[CAMERA_SERVER_CUSTOM]);
                flt::Checkbox("Allow In-Cam", &lp->allowCam[CAMERA_IN]);
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 4: GRAPHICS QUALITY ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "GRAPHICS QUALITY");
            ImGui::Separator();
            ImGui::Spacing();
            
            flt::Checkbox("Alpha Blending", &sr_alphaBlend);
            flt::Checkbox("Smooth Shading", &sr_smoothShading);
            flt::Checkbox("Dither", &sr_dither);
            flt::Checkbox("High Rim", &sr_highRim);
            flt::Checkbox("Keep Window Active", &sr_keepWindowActive);
            
            {
                const char* floorDetailNames[] = { "Off", "Grid", "Texture", "Double Texture" };
                int floorDetailVal = sr_floorDetail;
                if (floorDetailVal < 0) floorDetailVal = 0;
                if (floorDetailVal > 3) floorDetailVal = 3;
                if (flt::Label("Floor Detail:")) {
                ImGui::SetNextItemWidth(width1 - 55.0f); }
                if (flt::Combo("##FloorDetailCombo", &floorDetailVal, floorDetailNames, 4)) {
                    sr_floorDetail = floorDetailVal;
                    g_FloorDetail = (float)floorDetailVal;
                }
            }
            
            {
                const char* floorMirrorNames[] = { "Off", "Objects", "Walls", "All" };
                int floorMirrorIdx = 0;
                if (sr_floorMirror == rMIRROR_OFF) floorMirrorIdx = 0;
                else if (sr_floorMirror == rMIRROR_OBJECTS) floorMirrorIdx = 1;
                else if (sr_floorMirror == rMIRROR_WALLS) floorMirrorIdx = 2;
                else if (sr_floorMirror == rMIRROR_ALL) floorMirrorIdx = 3;
                if (flt::Label("Floor Mirror:")) {
                ImGui::SetNextItemWidth(width1 - 55.0f); }
                if (flt::Combo("##FloorMirrorCombo", &floorMirrorIdx, floorMirrorNames, 4)) {
                    if (floorMirrorIdx == 0) sr_floorMirror = rMIRROR_OFF;
                    else if (floorMirrorIdx == 1) sr_floorMirror = rMIRROR_OBJECTS;
                    else if (floorMirrorIdx == 2) sr_floorMirror = rMIRROR_WALLS;
                    else if (floorMirrorIdx == 3) sr_floorMirror = rMIRROR_ALL;
                }
            }

            TextureModeCombo("Floor Texture:",  "##TexModeFloor", rTextureGroups::TextureMode[0], true,  width1 - 55.0f);
            TextureModeCombo("Wall Textures:",  "##TexModeWall",  rTextureGroups::TextureMode[1], true,  width1 - 55.0f);
            TextureModeCombo("Object Textures:","##TexModeObject",rTextureGroups::TextureMode[2], true,  width1 - 55.0f);
            TextureModeCombo("Font:",           "##TexModeFont",  rTextureGroups::TextureMode[3], false, width1 - 55.0f);

            ImGui::Spacing();
            ImGui::Spacing();

            // --- SECTION 5: SKY & ENVIRONMENT ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "SKY & ENVIRONMENT");
            ImGui::Separator();
            ImGui::Spacing();
            
            flt::Checkbox("Upper Sky", &sr_upperSky);
            flt::Checkbox("Lower Sky", &sr_lowerSky);
            flt::Checkbox("Sky Wobble", &sr_skyWobble);
            flt::Checkbox("Infinity Plane", &sr_infinityPlane);
            flt::Checkbox("Predict Objects", &sr_predictObjects);
            flt::Checkbox("Truecolor Textures", &sr_texturesTruecolor);
            flt::Checkbox("Lag-O-Meter", &sr_laggometer);
            flt::Checkbox("Axes Indicator", &sg_axesIndicator);
            flt::Checkbox("Use Moviepack (Custom Walls/Sky/Floor)", &sg_moviepackUse);
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 6: SOUND SETTINGS ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "SOUND SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();
            
            {
                const char* soundQualityNames[] = { "Off", "Low", "Medium", "High" };
                int soundQual = sound_quality;
                if (soundQual < 0 || soundQual > 3) soundQual = 2;
                flt::Label("Sound Quality:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::Combo("##SoundQualityCombo", &soundQual, soundQualityNames, 4)) {
                    int oldQual = sound_quality;
                    sound_quality = soundQual;
                    if (oldQual != sound_quality) {
                        se_SoundExit();
                        se_SoundInit();
                    }
                }
            }
            
            {
                const char* bufferShiftNames[] = { "Very Small", "Small", "Medium", "High", "Very High" };
                int bufShiftIdx = buffer_shift + 2;
                if (bufShiftIdx < 0 || bufShiftIdx > 4) bufShiftIdx = 2;
                flt::Label("Audio Buffer Size:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (flt::Combo("##AudioBufferCombo", &bufShiftIdx, bufferShiftNames, 5)) {
                    int oldShift = buffer_shift;
                    buffer_shift = bufShiftIdx - 2;
                    if (oldShift != buffer_shift) {
                        se_SoundExit();
                        se_SoundInit();
                    }
                }
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 7: ADVANCED ENGINE SETTINGS ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "ADVANCED ENGINE SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("GAME RULES & PHYSICS SETTINGS", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { GameSettingsSP(); };
            }
            ImGui::Spacing();
            DrawDisplaySettings(width1);
            ImGui::Spacing();
            DrawSoundSettings(width1);
            ImGui::Spacing();
            
            if (!hasPanel2) {
                // If single column layout, stack the Keybindings directly inside the scroll pane
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "KEYBINDINGS");
                ImGui::Spacing();
                
                DrawKeybindList(width1, 0);
            }
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            if (hasPanel2) {
                // Column 2: Keybindings
                drawPanel(panel2Pos, panel2Size, g_DashboardActiveCol == 2, "KEYBINDINGS");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 15.0f, panel2Pos.y + 60.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::BeginChild("KeybindingsScroll", ImVec2(width2 - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
                
                DrawKeybindList(width2, 0);
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        } else if (g_ActiveTab == 4) {
            bool savedOpen = g_MenuOpen;
            float savedAlphaInner = g_MenuAlpha;
            g_MenuOpen = true;
            g_MenuAlpha = 1.0f;
            RenderInner();
            g_MenuOpen = savedOpen;
            g_MenuAlpha = savedAlphaInner;
        }
        
        // Render popups/modals
        RenderModMenuModals();
        
        // Draw the embedded media player card inside panel 1 (central box) if active tab is Dashboard (0)
        if (g_ActiveTab == 0) {
            MediaWidget* media = MediaWidget::GetInstance();
            if (media) {
                ImVec2 mediaPos(panel1Pos.x + (panel1Size.x - 380.0f) * 0.5f, panel1Pos.y + panel1Size.y - 125.0f - 20.0f);
                media->Draw(dl, mediaPos, g_MenuAlpha);
            }
        }
        
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
        g_MenuAlpha = savedAlpha;
        
        if (sr_glOut) {
            sr_ResetRenderState(true);
            gLogo::Display();
        }
        
        PaletteFrame();
    ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        
        rSysDep::SwapGL();
        rSysDep::ClearGL();
    }
    su_ClearKeys();
    while (su_GetSDLInput(purgeEvent)) {
        // Discard
    }
    SDL_HideCursor();
}

// The team you would rather be on.
//
// A client cannot ask for a particular team that does not exist yet. The two
// messages it can send are "put me on this team", which needs the team to
// already be there to point at, and "make me a team", which has no argument -
// the server takes the lowest free colour slot, and that is why creating one
// always lands you in the first team on the list.
//
// So this is the next best thing that is actually within a client's reach: name
// the team you want, and the moment it exists - because somebody made it, or a
// round started with it - you are moved onto it without having to watch for it.
//! Written as a dash when empty.
//!
//! An empty value in the config file reads back as a bare key, and the config
//! system answers a bare key by printing the current value - which is why
//! every start began with a line about a setting nobody had touched.
tString g_PreferredTeamName( "-" );
static tConfItem<tString> conf_preferredTeam( "MOD_PREFERRED_TEAM", g_PreferredTeamName );

//! The wanted team, or nothing if none is set.
static char const * PreferredTeam()
{
    char const * name = (char const *)g_PreferredTeamName;
    if ( !name || !*name || strcmp( name, "-" ) == 0 )
        return 0;
    return name;
}

bool g_PreferTeamAuto = false;
static tConfItem<bool> conf_preferTeamAuto( "MOD_PREFERRED_TEAM_AUTO", g_PreferTeamAuto );

//! Moves us across if the wanted team has appeared.

// Waiting for a place on a full server.
//
// A server with every seat taken lets you in as a spectator and leaves you
// there: the only way onto the grid is to notice the moment somebody quits and
// ask before anybody else does, which in practice means sitting on the team
// menu pressing a key. This asks on your behalf. It costs one small message
// every few seconds, the server ignores it while there is no room, and the
// first time there is, the answer arrives before a person could have reacted.
bool g_TakeFreeSlot = false;
static tConfItem<bool> conf_takeFreeSlot( "MOD_TAKE_FREE_SLOT", g_TakeFreeSlot );

void rc_TakeFreeSlot()
{
    if ( !g_TakeFreeSlot )
        return;

    if ( sn_GetNetState() != nCLIENT )
        return;

    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    if ( !seat || !seat->netPlayer )
        return;

    // Somebody who has chosen to watch is watching on purpose.
    if ( seat->spectate )
        return;

    ePlayerNetID * me = seat->netPlayer;

    // Already playing, or already promised a team: nothing to ask for.
    if ( me->CurrentTeam() || me->NextTeam() )
        return;

    static double last = 0;
    double const now = tSysTimeFloat();
    if ( now - last < 2.5 )
        return;
    last = now;

    // Prefer the emptiest team that will have us; failing that, ask for one of
    // our own. Both are ordinary wishes - the server decides, exactly as it
    // would if the request came from the menu.
    eTeam * best = 0;
    for ( int i = eTeam::teams.Len() - 1; i >= 0; --i )
    {
        eTeam * t = eTeam::teams( i );
        if ( !t || !t->IsHuman() || t->IsLocked() )
            continue;

        if ( !best || t->NumHumanPlayers() < best->NumHumanPlayers() )
            best = t;
    }

    if ( best )
        me->SetTeamWish( best );
    else
        me->CreateNewTeamWish();

    ePlayerNetID::Update();
}

void rc_FollowPreferredTeam()
{
    if ( !g_PreferTeamAuto || !PreferredTeam() )
        return;

    if ( sn_GetNetState() != nCLIENT )
        return;

    // not more than once every few seconds, whatever the server makes of it
    static double last = 0;
    double const now = tSysTimeFloat();
    if ( now - last < 3.0 )
        return;

    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    if ( !seat || !seat->netPlayer || seat->netPlayer->IsSpectating() )
        return;

    ePlayerNetID * me = seat->netPlayer;

    for ( int t = 0; t < eTeam::teams.Len(); ++t )
    {
        eTeam * team = eTeam::teams( t );
        if ( !team )
            continue;

        tString const & name = team->Name();
        if ( strcasecmp( (char const *)name, PreferredTeam() ) != 0 )
            continue;

        if ( me->NextTeam() == team )
            return;

        last = now;
        me->SetTeamWish( team );
        ePlayerNetID::Update();
        return;
    }
}

void ModMenu::RenderInGameTeams(float width) {
    bool foundPlayer = false;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (ePlayer::PlayerIsInGame(i)) {
            ePlayer* player = ePlayer::PlayerConfig(i);
            ePlayerNetID* pni = player->netPlayer;
            if (pni) {
                foundPlayer = true;
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "Player: %s", player->Name());
                
                ImGui::Text("Current Status: ");
                ImGui::SameLine();
                if (pni->IsSpectating()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Spectating");
                } else if (pni->NextTeam()) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", static_cast<const char*>(pni->NextTeam()->Name()));
                } else {
                    ImGui::Text("No Team");
                }
                
                ImGui::Spacing();
                
                // Join buttons
                if (!pni->IsSpectating()) {
                    char spectateId[64];
                    snprintf(spectateId, sizeof(spectateId), "Spectate##spec_%d", i);
                    if (ImGui::Button(spectateId, ImVec2(width - 20.0f, 28.0f))) {
                        player->spectate = true;
                        ePlayerNetID::Update();
                    }
                } else {
                    char joinGameId[64];
                    snprintf(joinGameId, sizeof(joinGameId), "Join Game##join_%d", i);
                    if (ImGui::Button(joinGameId, ImVec2(width - 20.0f, 28.0f))) {
                        player->spectate = false;
                        ePlayerNetID::Update();
                    }
                }
                
                ImGui::Spacing();
                
                // Active teams list
                for (int t = 0; t < eTeam::teams.Len(); ++t) {
                    eTeam* team = eTeam::teams(t);
                    if (team != pni->NextTeam()) {
                        char joinTeamId[128];
                        snprintf(joinTeamId, sizeof(joinTeamId), "Join Team %s##join_%d_%d", static_cast<const char*>(team->Name()), i, t);
                        if (ImGui::Button(joinTeamId, ImVec2(width - 20.0f, 28.0f))) {
                            player->spectate = false;
                            pni->SetTeamWish(team);
                            ePlayerNetID::Update();
                        }
                    }
                }
                
                // Preferred team
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), "Preferred team");

                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%s", (char const *)g_PreferredTeamName);
                    ImGui::SetNextItemWidth(width - 20.0f);
                    if (ImGui::InputText("##preferred_team", buf, sizeof(buf)))
                        g_PreferredTeamName = buf;
                }

                ImGui::Checkbox("Join it as soon as it exists##prefer_auto", &g_PreferTeamAuto);

                ImGui::Checkbox("Take the first free slot##take_free_slot", &g_TakeFreeSlot);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("On a full server you are let in as a spectator and left there.\n"
                                      "This keeps asking for a place on your behalf, and takes the one\n"
                                      "that opens the moment somebody leaves.");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("A client cannot ask for a team that does not exist yet - the server\n"
                                      "gives a new team the lowest free colour, which is why creating one\n"
                                      "always lands you in the first. This moves you across the moment the\n"
                                      "team you named turns up.");

                ImGui::Spacing();

                // Create new team
                if (pni->IsSpectating() ||
                    !(pni->NextTeam() && pni->NextTeam()->NumHumanPlayers() == 1 &&
                      pni->CurrentTeam() && pni->CurrentTeam()->NumHumanPlayers() == 1)) {
                    char createTeamId[64];
                    snprintf(createTeamId, sizeof(createTeamId), "Create New Team##create_%d", i);
                    if (ImGui::Button(createTeamId, ImVec2(width - 20.0f, 28.0f))) {
                        player->spectate = false;
                        pni->CreateNewTeamWish();
                        ePlayerNetID::Update();
                    }
                }
                
                ImGui::Separator();
                ImGui::Spacing();
            }
        }
    }
    
    if (!foundPlayer) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No local players found in game.");
    }
}

void ModMenu::RenderInGameSettings(float width) {
    ImIdScope idScope( "ingamesettings" );
    ePlayer* lp = ePlayer::PlayerConfig(0);

    DrawSettingsSearch(width + 30.0f);

    // --- PLAYER SETUP ---
    if (SettingsSectionOpen("PLAYER SETUP")) {
        if (lp) {
            char nameBuf[256];
            std::string nameUtf8 = Cp1251ToUtf8((const char*)lp->name);
            strncpy(nameBuf, nameUtf8.c_str(), sizeof(nameBuf));
            nameBuf[sizeof(nameBuf)-1] = '\0';
            if (ImGui::InputText("Screen Name##ingame", nameBuf, sizeof(nameBuf))) {
                lp->name = Utf8ToCp1251(nameBuf);
                // request network synchronization
                static nVersionFeature inGameRenames(5);
                if (inGameRenames.Supported()) {
                    ePlayerNetID::Update();
                    ePlayer::SendAuthNames();
                }
            }
            ImGui::Spacing();
            
            float col[3] = { (float)lp->rgb[0] / 15.0f, (float)lp->rgb[1] / 15.0f, (float)lp->rgb[2] / 15.0f };
            if (flt::ColorEdit3("Cycle Color", col)) {
                lp->rgb[0] = (int)(col[0] * 15.0f + 0.5f);
                lp->rgb[1] = (int)(col[1] * 15.0f + 0.5f);
                lp->rgb[2] = (int)(col[2] * 15.0f + 0.5f);
            }
            int rawR = lp->rgb[0];
            int rawG = lp->rgb[1];
            int rawB = lp->rgb[2];
            if (flt::SliderInt("Raw R (Glow)", &rawR, 0, 255)) lp->rgb[0] = rawR;
            if (flt::SliderInt("Raw G (Glow)", &rawG, 0, 255)) lp->rgb[1] = rawG;
            if (flt::SliderInt("Raw B (Glow)", &rawB, 0, 255)) lp->rgb[2] = rawB;
            ImGui::Spacing();
            DrawCycleColorPreview(lp);
            ImGui::Spacing();
            
            char gidBuf[512];
            std::string gidUtf8 = Cp1251ToUtf8((const char*)lp->globalID);
            strncpy(gidBuf, gidUtf8.c_str(), sizeof(gidBuf));
            gidBuf[sizeof(gidBuf)-1] = '\0';
            if (ImGui::InputText("Global ID##ingame", gidBuf, sizeof(gidBuf))) {
                lp->globalID = Utf8ToCp1251(gidBuf);
            }
            ImGui::Spacing();
            
            flt::Checkbox("Auto Login", &lp->autoLogin);
            flt::Checkbox("Stealth Mode", &lp->stealth);
            flt::Checkbox("Spectator Mode", &lp->spectate);
            flt::Checkbox("Name Team After Me", &lp->nameTeamAfterMe);
            
            char teamNameBuf[256];
            std::string teamNameUtf8 = Cp1251ToUtf8((const char*)lp->teamName);
            strncpy(teamNameBuf, teamNameUtf8.c_str(), sizeof(teamNameBuf));
            teamNameBuf[sizeof(teamNameBuf)-1] = '\0';
            if (ImGui::InputText("Custom Team Name##ingame", teamNameBuf, sizeof(teamNameBuf))) {
                lp->teamName = Utf8ToCp1251(teamNameBuf);
                static nVersionFeature inGameRenames(5);
                if (inGameRenames.Supported()) {
                    ePlayerNetID::Update();
                    ePlayer::SendAuthNames();
                }
            }
            
            flt::Label("Players Per Team:");
            flt::SliderInt("##InGamePlayersPerTeam", &lp->favoriteNumberOfPlayersPerTeam, 1, 16);
            
            ImGui::Spacing();
            if (ImGui::TreeNode("Instant Chat Macros")) {
                for (int m = 0; m < 20; m++) {
                    char macroBuf[256];
                    std::string macroUtf8 = Cp1251ToUtf8((const char*)lp->instantChatString[m]);
                    strncpy(macroBuf, macroUtf8.c_str(), sizeof(macroBuf));
                    macroBuf[sizeof(macroBuf)-1] = '\0';
                    char label[64];
                    snprintf(label, sizeof(label), "Macro %d##ig_macro_%d", m + 1, m);
                    if (ImGui::InputText(label, macroBuf, sizeof(macroBuf))) {
                        lp->instantChatString[m] = Utf8ToCp1251(macroBuf);
                    }
                }
                ImGui::TreePop();
            }
            
            ImGui::Spacing();
            if (ImGui::Button("SAVE PROFILE", ImVec2(width - 25.0f, 30.0f))) {
                st_SaveConfig();
            }
        }
    }
    
    // --- CAMERA SETTINGS ---
    if (SettingsSectionOpen("CAMERA CONFIG")) {
        if (lp) {
            flt::Label("Field of View (FOV):");
            if (flt::SliderInt("##InGameFOV", &lp->startFOV, 30, 160)) rc_CameraFOV(lp->startFOV);
            
            flt::Checkbox("Auto-Switch In-Cam", &lp->autoSwitchIncam);
            flt::Checkbox("Wobble In-Cam", &lp->wobbleIncam);
            flt::Checkbox("Center In-Cam on Turn", &lp->centerIncamOnTurn);
            
            flt::Checkbox("Allow Smart Camera", &lp->allowCam[CAMERA_SMART]);
            flt::Checkbox("Allow Follow Camera", &lp->allowCam[CAMERA_FOLLOW]);
            flt::Checkbox("Allow Free Camera", &lp->allowCam[CAMERA_FREE]);
            flt::Checkbox("Allow Custom Camera", &lp->allowCam[CAMERA_CUSTOM]);
            flt::Checkbox("Allow Server Custom Camera", &lp->allowCam[CAMERA_SERVER_CUSTOM]);
            flt::Checkbox("Allow In-Cam", &lp->allowCam[CAMERA_IN]);
        }
    }
    
    // --- GRAPHICS QUALITY ---
    if (SettingsSectionOpen("GRAPHICS QUALITY")) {
        flt::Checkbox("Alpha Blending", &sr_alphaBlend);
        flt::Checkbox("Smooth Shading", &sr_smoothShading);
        flt::Checkbox("Dither", &sr_dither);
        flt::Checkbox("High Rim", &sr_highRim);
        flt::Checkbox("Keep Window Active", &sr_keepWindowActive);
        
        {
            const char* floorDetailNames[] = { "Off", "Grid", "Texture", "Double Texture" };
            int floorDetailVal = sr_floorDetail;
            if (floorDetailVal < 0) floorDetailVal = 0;
            if (floorDetailVal > 3) floorDetailVal = 3;
            flt::Label("Floor Detail:");
            if (flt::Combo("##InGameFloorDetail", &floorDetailVal, floorDetailNames, 4)) {
                sr_floorDetail = floorDetailVal;
            }
        }
        
        {
            const char* floorMirrorNames[] = { "Off", "Objects", "Walls", "All" };
            int floorMirrorIdx = 0;
            if (sr_floorMirror == rMIRROR_OFF) floorMirrorIdx = 0;
            else if (sr_floorMirror == rMIRROR_OBJECTS) floorMirrorIdx = 1;
            else if (sr_floorMirror == rMIRROR_WALLS) floorMirrorIdx = 2;
            else if (sr_floorMirror == rMIRROR_ALL) floorMirrorIdx = 3;
            flt::Label("Floor Mirror:");
            if (flt::Combo("##InGameFloorMirror", &floorMirrorIdx, floorMirrorNames, 4)) {
                if (floorMirrorIdx == 0) sr_floorMirror = rMIRROR_OFF;
                else if (floorMirrorIdx == 1) sr_floorMirror = rMIRROR_OBJECTS;
                else if (floorMirrorIdx == 2) sr_floorMirror = rMIRROR_WALLS;
                else if (floorMirrorIdx == 3) sr_floorMirror = rMIRROR_ALL;
            }
        }

        TextureModeCombo("Floor Texture:",   "##InGameTexModeFloor",  rTextureGroups::TextureMode[0], true,  0.0f);
        TextureModeCombo("Wall Textures:",   "##InGameTexModeWall",   rTextureGroups::TextureMode[1], true,  0.0f);
        TextureModeCombo("Object Textures:", "##InGameTexModeObject", rTextureGroups::TextureMode[2], true,  0.0f);
        TextureModeCombo("Font:",            "##InGameTexModeFont",   rTextureGroups::TextureMode[3], false, 0.0f);

    }

    // --- SKY & ENVIRONMENT ---
    if (SettingsSectionOpen("SKY & ENVIRONMENT")) {
        flt::Checkbox("Upper Sky", &sr_upperSky);
        flt::Checkbox("Lower Sky", &sr_lowerSky);
        flt::Checkbox("Sky Wobble", &sr_skyWobble);
        flt::Checkbox("Infinity Plane", &sr_infinityPlane);
        flt::Checkbox("Predict Objects", &sr_predictObjects);
        flt::Checkbox("Truecolor Textures", &sr_texturesTruecolor);
        flt::Checkbox("Lag-O-Meter", &sr_laggometer);
        flt::Checkbox("Axes Indicator", &sg_axesIndicator);
        flt::Checkbox("Use Moviepack (Custom Walls/Sky/Floor)", &sg_moviepackUse);
    }
    
    // --- SOUND SETTINGS ---
    if (SettingsSectionOpen("SOUND SETTINGS")) {
        {
            const char* soundQualityNames[] = { "Off", "Low", "Medium", "High" };
            int soundQual = sound_quality;
            if (soundQual < 0 || soundQual > 3) soundQual = 2;
            flt::Label("Sound Quality:");
            if (flt::Combo("##InGameSoundQuality", &soundQual, soundQualityNames, 4)) {
                int oldQual = sound_quality;
                sound_quality = soundQual;
                if (oldQual != sound_quality) {
                    se_SoundExit();
                    se_SoundInit();
                }
            }
        }
        
        {
            const char* bufferShiftNames[] = { "Very Small", "Small", "Medium", "High", "Very High" };
            int bufShiftIdx = buffer_shift + 2;
            if (bufShiftIdx < 0 || bufShiftIdx > 4) bufShiftIdx = 2;
            flt::Label("Audio Buffer Size:");
            if (flt::Combo("##InGameAudioBuffer", &bufShiftIdx, bufferShiftNames, 5)) {
                int oldShift = buffer_shift;
                buffer_shift = bufShiftIdx - 2;
                if (oldShift != buffer_shift) {
                    se_SoundExit();
                    se_SoundInit();
                }
            }
        }
        
        flt::Label("Max Sound Sources:");
        flt::SliderInt("##InGameSoundSources", &sound_sources, 2, 20);
    }
    
    // --- KEYBINDINGS ---
    if (SettingsFiltering()) {
        DrawKeybindList(width + 30.0f, 0);
    } else if (ImGui::CollapsingHeader("KEYBINDINGS")) {
        DrawKeybindList(width + 30.0f, 0);
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // --- ADVANCED ENGINE SETTINGS ---
    if (SettingsSectionOpen("ADVANCED ENGINE SETTINGS")) {
        if (ImGui::Button("MATCH RULES & PHYSICS SETTINGS", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { GameSettingsCurrent(); };
        }
        ImGui::Spacing();
        DrawDisplaySettings(width + 30.0f);
        ImGui::Spacing();
        DrawSoundSettings(width + 30.0f);
        ImGui::Spacing();
    }
}

void ModMenu::RenderInGameVotingAndPolice(float width) {
    rcTheme const & th = rc_Theme();

    if (sn_GetNetState() == nSTANDALONE) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(th.textMute));
        ImGui::TextWrapped("Nobody to police in a game with only you in it. These are for servers.");
        ImGui::PopStyleColor();
        return;
    }

    rc_Tracked("PLAYERS", 1.6f, th.textMute);
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(th.textDim));
    ImGui::TextWrapped("Silencing takes effect for you alone and needs nobody's agreement. "
                       "A kick is a suggestion the server puts to everybody, unless you are "
                       "running it yourself.");
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, 12.0f));

    bool anybody = false;

    for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
        ePlayerNetID* p = se_PlayerNetIDs(i);
        if (!p || !p->IsHuman()) continue;

        // no policing yourself
        bool mine = false;
        for (int seat = 0; seat < MAX_PLAYERS; ++seat) {
            ePlayer* cfg = ePlayer::PlayerConfig(seat);
            if (cfg && cfg->netPlayer == p) { mine = true; break; }
        }
        if (mine) continue;

        anybody = true;
        ImGui::PushID(p);

        ImVec2 rowAt = ImGui::GetCursorScreenPos();
        float rowH = 46.0f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(rowAt, ImVec2(rowAt.x + width, rowAt.y + rowH),
                          rc_Fade(th.surface, 0.6f), th.radiusMedium);

        REAL pr, pg, pb;
        p->Color(pr, pg, pb);
        RenderColoredText(dl, ImVec2(rowAt.x + 14.0f, rowAt.y + 14.0f),
                          IM_COL32((int)(pr*255), (int)(pg*255), (int)(pb*255), 245),
                          (char const *)p->GetName());

        char const * login = rc_LoginOf(p->GetName());
        if (login && *login) {
            dl->AddText(ImVec2(rowAt.x + 14.0f, rowAt.y + 28.0f),
                        rc_Fade(th.second.solid, 0.8f), login);
        }

        ImGui::SetCursorScreenPos(ImVec2(rowAt.x + width - 250.0f, rowAt.y + 9.0f));
        bool silenced = p->IsSilenced();
        if (ImGui::Button(silenced ? "Unsilence" : "Silence", ImVec2(110.0f, 28.0f))) {
            p->SetSilenced(!silenced);
            se_PlayUi("click");
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4(rc_Fade(th.bad, 0.35f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4(rc_Fade(th.bad, 0.6f)));
        if (ImGui::Button("Kick", ImVec2(100.0f, 28.0f))) {
            se_RequestKick(p);
            rc_Toast("kick put to the server", "confirm");
        }
        ImGui::PopStyleColor(2);

        ImGui::SetCursorScreenPos(ImVec2(rowAt.x, rowAt.y + rowH + 8.0f));
        ImGui::PopID();
    }

    if (!anybody) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(th.textMute));
        ImGui::TextWrapped("Nobody else here yet.");
        ImGui::PopStyleColor();
    }

    ImGui::Dummy(ImVec2(0.0f, 18.0f));

    if (!eVoter::VotingPossible()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(th.textMute));
        ImGui::TextWrapped("This server is not taking votes.");
        ImGui::PopStyleColor();
    }
}

// What a server says when you walk in.
//
// The stock screen is the old menu renderer: one bitmap font, no layout, the
// text starting wherever it starts. It is the first thing anybody sees of a
// server and it looked like a different game from the rest of the client.
//
// This blocks the same way the old one did - the caller has already paused the
// game and put the players in spectator, and expects to continue when the
// reader is done - so it runs its own frame loop and returns on a key, a click
// or the server's timeout.
//! How tall the words came out last frame, so the card can fit them.
static float s_messageContentH = 0.0f;

// ---------------------------------------------------------------------------
// The screen for a game that ended without being asked to.
//
// All five of these used to arrive as one paragraph of prose with the server's
// own words buried in the middle of it, on the same card the message of the
// day uses. They are not the same thing: being thrown off a server is worth
// knowing at a glance, and what the server said about it is the one line
// anybody actually reads. So the reason gets a block of its own, the kind of
// ending gets a word and a colour, and the two things worth doing next are on
// the screen rather than left to be guessed at.
void ModMenu::ShowDisconnect( int kind, char const * title, char const * body,
                              char const * reason, char const * redirect, float timeout )
{
    if (!sr_glOut)
        return;

    if (!g_Initialized) Init();

    if (!title)    title = "";
    if (!body)     body = "";
    if (!reason)   reason = "";
    if (!redirect) redirect = "";

    // A server that says the word usually means it. Being told plainly beats
    // reading a paragraph to work out whether it is worth trying again.
    bool banned = false;
    {
        std::string lower( reason );
        for ( size_t i = 0; i < lower.size(); ++i )
            lower[i] = (char)tolower( (unsigned char)lower[i] );
        banned = lower.find( "ban" ) != std::string::npos ||
                 lower.find( "suspend" ) != std::string::npos;
    }

    char const * word = "DISCONNECTED";
    ImVec4 accent( 0.85f, 0.30f, 0.32f, 1.0f );

    switch ( kind )
    {
    case 0: word = banned ? "BANNED" : "KICKED";   accent = ImVec4( 0.90f, 0.26f, 0.28f, 1.0f ); break;
    case 1: word = banned ? "BANNED" : "REFUSED";  accent = ImVec4( 0.92f, 0.55f, 0.20f, 1.0f ); break;
    case 2: word = "NO ANSWER";                    accent = ImVec4( 0.62f, 0.62f, 0.70f, 1.0f ); break;
    case 3: word = "CONNECTION LOST";              accent = ImVec4( 0.90f, 0.40f, 0.30f, 1.0f ); break;
    case 4: word = "SYNC FAILED";                  accent = ImVec4( 0.85f, 0.70f, 0.25f, 1.0f ); break;
    default: break;
    }

    // Trying again makes sense when the line broke. It does not when the door
    // was shut in your face.
    bool const mayRetry = ( kind == 2 || kind == 3 || kind == 4 ) && sg_hasLastServer;

    ImGuiIO& io = ImGui::GetIO();
    su_ClearKeys();

    SDL_Event purge;
    while (su_GetSDLInput(purge)) {}
    for (int i = 0; i < 5; ++i) { io.MouseDown[i] = false; io.MouseClicked[i] = false; }

    rcTheme const & th = rc_Theme();

    double began = tSysTimeFloat();
    double lastFrame = began;
    bool   done = false;
    bool   retry = false;

    float const settle = 0.35f;

    while (!done && !uMenu::quickexit && !uMenu::exitToMain)
    {
        double now = tSysTimeFloat();
        float since = (float)( now - began );
        io.DeltaTime = (float)( now - lastFrame );
        if (io.DeltaTime <= 0.0f) io.DeltaTime = 1.0f / 60.0f;
        lastFrame = now;

        SDL_Event event;
        while (su_GetSDLInput(event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (since < settle)
                continue;

            if (event.type == SDL_EVENT_QUIT)
            {
                uMenu::quickexit = uMenu::QuickExit_Total;
                done = true;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                SDL_Keycode key = event.key.key;
                if (mayRetry && (key == SDLK_RETURN || key == SDLK_KP_ENTER))
                {
                    retry = true;
                    done = true;
                }
                else
                {
                    done = true;
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                done = true;
            }
        }

        if (timeout > 0.0f && since > timeout)
            done = true;

        io.MouseDrawCursor = true;
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        FeedMousePosition();
        ImGui::NewFrame();

        ImVec2 screen = io.DisplaySize;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(screen);
        ImGui::Begin("##disconnect", NULL,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        dl->AddRectFilled(ImVec2(0,0), screen, rc_Fade(th.page, 0.97f));
        rc_DrawBackdrop(dl, ImVec2(0,0), screen, 1.0f);

        float in = since / 0.35f; if (in > 1.0f) in = 1.0f;
        float ease = rc_Ease(ImGui::GetID("##disconnect"), in, 0.35f);

        float cardW = screen.x * 0.56f; if (cardW > 980.0f) cardW = 980.0f;

        static float cardH = 300.0f;
        float wantH = s_messageContentH > 0.0f ? s_messageContentH : 300.0f;
        cardH += ( wantH - cardH ) * 0.25f;

        ImVec2 at((screen.x - cardW) * 0.5f, (screen.y - cardH) * 0.5f + (1.0f - ease) * 26.0f);
        ImVec2 to(at.x + cardW, at.y + cardH);

        dl->AddRectFilled(at, to, rc_Fade(th.surface, 0.96f * ease), th.radiusLarge);
        dl->AddRect(at, to, rc_Fade(th.line, ease), th.radiusLarge, 0, 1.0f);
        dl->AddRectFilled(ImVec2(at.x, at.y), ImVec2(to.x, at.y + 3.0f),
                          ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, 0.95f * ease)),
                          th.radiusLarge);

        float const pad = 48.0f;
        float textX = at.x + pad;
        float room = cardW - pad * 2.0f;
        float y = at.y + 40.0f;

        // the word for what happened, and under it where it happened
        ImGui::SetCursorScreenPos(ImVec2(textX, y));
        rc_Tracked(word, 3.0f, ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, ease)));
        y += 34.0f;

        if (sg_hasLastServer)
        {
            char where[192];
            snprintf(where, sizeof(where), "%s:%d", (char const *)sg_lastServerIP, (int)sg_lastServerPort);
            dl->AddText(ImVec2(textX, y), rc_Fade(th.textMute, 0.9f * ease), where);
            y += ImGui::GetFontSize() + 16.0f;
        }

        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        RenderColoredText(dl, ImVec2(textX, y), rc_Fade(th.text, ease), title);
        y += ImGui::GetFontSize() + 20.0f;
        if (g_FontHeader) ImGui::PopFont();

        // What the server said, set apart. It is the only part of this screen
        // that is not boilerplate, and it used to be a sentence in a wall of
        // them.
        if (*reason)
        {
            float const blockPad = 16.0f;
            float const lineStep = ImGui::GetFontSize() + 6.0f;

            // wrapped once, so the plate can be drawn to fit before the words
            // go on top of it
            std::vector< std::string > lines;
            {
                std::string rest( reason );
                float const width = room - blockPad * 2.0f - 6.0f;

                while (!rest.empty() && (int)lines.size() < 6)
                {
                    size_t take = rest.size();
                    while (take > 0 && ImGui::CalcTextSize(Uncoloured(rest.substr(0, take)).c_str()).x > width)
                    {
                        size_t space = rest.rfind(' ', take - 1);
                        take = ( space == std::string::npos || space == 0 ) ? take - 1 : space;
                    }
                    if (take == 0) take = rest.size();

                    lines.push_back( rest.substr(0, take) );

                    rest = ( take < rest.size() ) ? rest.substr(take) : std::string();
                    while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
                }
            }

            float blockH = lines.size() * lineStep + blockPad * 2.0f - 6.0f;

            dl->AddRectFilled(ImVec2(textX, y), ImVec2(to.x - pad, y + blockH),
                              rc_Fade(th.page, 0.55f * ease), th.radiusSmall);
            dl->AddRectFilled(ImVec2(textX, y), ImVec2(textX + 3.0f, y + blockH),
                              ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, 0.75f * ease)));

            float ty = y + blockPad;
            for (size_t i = 0; i < lines.size(); ++i)
            {
                RenderColoredText(dl, ImVec2(textX + blockPad + 6.0f, ty), rc_Fade(th.text, ease),
                                  lines[i].c_str());
                ty += lineStep;
            }

            y += blockH + 22.0f;
        }

        // the plain explanation, quieter than the reason
        {
            std::string rest( body );
            while (!rest.empty())
            {
                size_t take = rest.size();
                while (take > 0 && ImGui::CalcTextSize(Uncoloured(rest.substr(0, take)).c_str()).x > room)
                {
                    size_t space = rest.rfind(' ', take - 1);
                    take = ( space == std::string::npos || space == 0 ) ? take - 1 : space;
                }
                if (take == 0) take = rest.size();

                RenderColoredText(dl, ImVec2(textX, y), rc_Fade(th.textDim, 0.9f * ease), rest.substr(0, take).c_str());
                y += ImGui::GetFontSize() + 7.0f;

                rest = ( take < rest.size() ) ? rest.substr(take) : std::string();
                while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
                if (y > at.y + 640.0f) break;
            }
        }

        if (*redirect)
        {
            y += 8.0f;
            RenderColoredText(dl, ImVec2(textX, y), rc_Fade(th.primary.bright, 0.95f * ease), redirect);
            y += ImGui::GetFontSize() + 8.0f;
        }

        y += 18.0f;

        // what to do about it
        {
            float keyY = y;
            char const * left  = mayRetry ? "ENTER" : "";
            char const * leftWhat = mayRetry ? "try the server again" : "";
            char const * right = "ANY KEY";
            char const * rightWhat = "back to the menu";

            if (mayRetry)
            {
                ImVec2 sz = ImGui::CalcTextSize(left);
                dl->AddRectFilled(ImVec2(textX - 6.0f, keyY - 3.0f), ImVec2(textX + sz.x + 10.0f, keyY + sz.y + 3.0f),
                                  rc_Fade(th.line, 0.7f * ease), th.radiusSmall);
                dl->AddText(ImVec2(textX + 2.0f, keyY), rc_Fade(th.text, ease), left);
                dl->AddText(ImVec2(textX + sz.x + 22.0f, keyY), rc_Fade(th.textDim, ease), leftWhat);
                keyY += ImGui::GetFontSize() + 12.0f;
            }

            ImVec2 sz2 = ImGui::CalcTextSize(right);
            dl->AddRectFilled(ImVec2(textX - 6.0f, keyY - 3.0f), ImVec2(textX + sz2.x + 10.0f, keyY + sz2.y + 3.0f),
                              rc_Fade(th.line, 0.5f * ease), th.radiusSmall);
            dl->AddText(ImVec2(textX + 2.0f, keyY), rc_Fade(th.textDim, ease), right);
            dl->AddText(ImVec2(textX + sz2.x + 22.0f, keyY), rc_Fade(th.textMute, ease), rightWhat);
            keyY += ImGui::GetFontSize() + 10.0f;

            y = keyY;
        }

        s_messageContentH = ( y - at.y ) + 34.0f;

        if (timeout > 0.0f)
        {
            float left = 1.0f - since / timeout;
            if (left < 0.0f) left = 0.0f;

            float barY = to.y - 22.0f;
            dl->AddRectFilled(ImVec2(at.x + pad, barY), ImVec2(to.x - pad, barY + 2.0f),
                              rc_Fade(th.line, ease));
            dl->AddRectFilled(ImVec2(at.x + pad, barY), ImVec2(at.x + pad + (cardW - pad * 2.0f) * left, barY + 2.0f),
                              ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, 0.7f * ease)));
        }

        ImGui::End();

        rc_DrawToasts();
    ImGui::Render();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glPopAttrib();

        rSysDep::SwapGL();
        rSysDep::ClearGL();

        st_DoToDo();
        tAdvanceFrame();
    }

    su_ClearKeys();

    if (retry)
        s_PendingReconnect = true;
}

void ModMenu::ShowServerMessage( char const * title, char const * body, float timeout )
{
    s_messageContentH = 0.0f;

    if (!sr_glOut)
        return;

    if (!g_Initialized) Init();

    if (!title) title = "";
    if (!body) body = "";

    ImGuiIO& io = ImGui::GetIO();
    su_ClearKeys();

    SDL_Event purge;
    while (su_GetSDLInput(purge)) {}
    for (int i = 0; i < 5; ++i) { io.MouseDown[i] = false; io.MouseClicked[i] = false; }

    rcTheme const & th = rc_Theme();

    double began = tSysTimeFloat();
    double lastFrame = began;
    bool   done = false;

    // A moment of grace before a stray keypress from the loading screen can
    // dismiss something nobody has read yet.
    float const settle = 0.35f;

    while (!done && !uMenu::quickexit && !uMenu::exitToMain)
    {
        CheckDisplaySizeRebuild();
        ShowSystemCursor();
        SDL_WM_GrabInput(SDL_GRAB_OFF);

        st_DoToDo();
        tAdvanceFrame();
        sn_SendPlanned();

        double now = tSysTimeFloat();
        float since = (float)(now - began);
        io.DeltaTime = (float)(now - lastFrame);
        if (io.DeltaTime <= 0.0f || io.DeltaTime > 0.25f) io.DeltaTime = 1.0f / 60.0f;
        lastFrame = now;

        SDL_Event event;
        while (su_GetSDLInput(event))
        {
            if (event.type == SDL_EVENT_QUIT) { uMenu::quickexit = uMenu::QuickExit_Total; break; }
            if (since > settle &&
                (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN))
                done = true;
            su_HandleEvent(event, true);
        }

        if (timeout > 0.0f && since > timeout)
            done = true;

        io.MouseDrawCursor = true;
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
        ImGui::NewFrame();

        ImVec2 screen = io.DisplaySize;

        // A window of our own, invisible and untouchable. Without one, the
        // cursor calls below land in ImGui's fallback window and it appears on
        // screen labelled "Debug".
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(screen);
        ImGui::Begin("##servermessage", NULL,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        dl->AddRectFilled(ImVec2(0,0), screen, rc_Fade(th.page, 0.97f));
        rc_DrawBackdrop(dl, ImVec2(0,0), screen, 1.0f);

        // arriving, then leaving, so it does not snap in or out
        float in = since / 0.35f; if (in > 1.0f) in = 1.0f;
        float ease = rc_Ease(ImGui::GetID("##motd"), in, 0.35f);

        float cardW = screen.x * 0.62f; if (cardW > 1180.0f) cardW = 1180.0f;

        // as tall as the words need and no taller
        static float cardH = 240.0f;
        float wantH = s_messageContentH > 0.0f ? s_messageContentH : 240.0f;
        cardH += ( wantH - cardH ) * 0.25f;
        ImVec2 at((screen.x - cardW) * 0.5f, (screen.y - cardH) * 0.5f + (1.0f - ease) * 26.0f);
        ImVec2 to(at.x + cardW, at.y + cardH);

        dl->AddRectFilled(at, to, rc_Fade(th.surface, 0.96f * ease), th.radiusLarge);
        dl->AddRect(at, to, rc_Fade(th.line, ease), th.radiusLarge, 0, 1.0f);
        dl->AddRectFilled(ImVec2(at.x, at.y), ImVec2(to.x, at.y + 3.0f),
                          rc_Fade(th.primary.solid, 0.9f * ease), th.radiusLarge);

        float textX = at.x + 54.0f;
        float y = at.y + 44.0f;

        ImGui::SetCursorScreenPos(ImVec2(textX, y));
        rc_Tracked("SERVER", 2.0f, rc_Fade(th.textMute, ease));
        y += 30.0f;

        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        RenderColoredText(dl, ImVec2(textX, y), rc_Fade(th.text, ease),
                          title && *title ? title : "Message of the Day");
        y += ImGui::GetFontSize() + 22.0f;
        if (g_FontHeader) ImGui::PopFont();

        dl->AddLine(ImVec2(textX, y), ImVec2(to.x - 54.0f, y), rc_Fade(th.line, ease), 1.0f);
        y += 26.0f;

        // The body arrives as one long line. Broken by hand on spaces, because
        // the coloured renderer draws a run and does not wrap it.
        {
            std::string rest = body ? body : "";
            float room = cardW - 108.0f;

            while (!rest.empty())
            {
                size_t take = rest.size();
                while (take > 0 && ImGui::CalcTextSize(Uncoloured(rest.substr(0, take)).c_str()).x > room)
                {
                    size_t space = rest.rfind(' ', take - 1);
                    take = ( space == std::string::npos || space == 0 ) ? take - 1 : space;
                }
                if (take == 0) take = rest.size();

                RenderColoredText(dl, ImVec2(textX, y), rc_Fade(th.textDim, ease), rest.substr(0, take).c_str());
                y += ImGui::GetFontSize() + 8.0f;

                rest = ( take < rest.size() ) ? rest.substr(take) : std::string();
                while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
                if (y > to.y - 70.0f) break;
            }
        }

        // what the next frame should make room for
        s_messageContentH = ( y - at.y ) + 62.0f;

        // how long is left, as a line that empties rather than a number
        if (timeout > 0.0f)
        {
            float left = 1.0f - since / timeout;
            if (left < 0.0f) left = 0.0f;

            float barY = to.y - 30.0f;
            dl->AddRectFilled(ImVec2(at.x + 54.0f, barY), ImVec2(to.x - 54.0f, barY + 2.0f),
                              rc_Fade(th.line, ease));
            dl->AddRectFilled(ImVec2(at.x + 54.0f, barY), ImVec2(at.x + 54.0f + (cardW - 108.0f) * left, barY + 2.0f),
                              rc_Fade(th.primary.solid, 0.8f * ease));
        }

        char const * hint = "press any key to continue";
        ImVec2 hintSz = ImGui::CalcTextSize(hint);
        dl->AddText(ImVec2((screen.x - hintSz.x) * 0.5f, to.y + 26.0f),
                    rc_Fade(th.textMute, 0.75f * ease), hint);

        ImGui::End();

        rc_DrawToasts();
    ImGui::Render();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glPopAttrib();

        rSysDep::SwapGL();
    }

    su_ClearKeys();
}


//! The login the server asks for, in the client's own clothes.
//!
//! Returns exactly what the menu it replaces returned: the two fields as the
//! caller left room for, the storage choice, and whether the reader walked
//! away. Nothing is kept here - the password goes straight back out and the
//! caller scrubs it, the same as before.
bool ModMenu::AskLogin( char const * heading, std::string & username,
                        std::string & password, int & storeMode )
{
    if (!sr_glOut)
        return false;

    if (!g_Initialized) Init();

    // The server can ask for this in the middle of a round, with the player
    // already spawned and driving - and then this runs inside a frame the game
    // has already begun. Opening another one on top of it leaves the interface
    // in a state where the caret blinks and nothing lands: no letter, no
    // click, only escape, which the layer underneath answers. So the frame in
    // progress is closed first and handed back when we are done.
    ImGuiContext * outer = ImGui::GetCurrentContext();
    bool const nested = outer && outer->WithinFrameScope;
    if ( nested )
        ImGui::EndFrame();

    struct Restore
    {
        bool on;
        ~Restore() { if ( on ) ImGui::NewFrame(); }
    } restore{ nested };

    ImGuiIO& io = ImGui::GetIO();
    su_ClearKeys();

    SDL_Event purge;
    while (su_GetSDLInput(purge)) {}
    for (int i = 0; i < 5; ++i) { io.MouseDown[i] = false; io.MouseClicked[i] = false; }

    rcTheme const & th = rc_Theme();

    char userBuf[ 128 ];
    char passBuf[ 128 ];
    snprintf( userBuf, sizeof( userBuf ), "%s", username.c_str() );
    passBuf[0] = 0;

    // the box always opens on the name, whatever was last touched in it
    s_loginField = 0;

    bool decided = false, accepted = false, focusOnce = true;
    double lastFrame = tSysTimeFloat();

    // Everything else keys off the menu flags, and during this loop they are
    // all false - so the input filter concludes no menu is up and turns
    // relative mouse mode back on, every frame, while this loop turns it off
    // again. That fight is what makes the pointer flicker and refuse to land
    // on the field.
    struct Claim
    {
        Claim()  { ModMenu::g_ModalOverlayOpen = true; }
        ~Claim() { ModMenu::g_ModalOverlayOpen = false; }
    } claim;

    // This loop draws the dialog and nothing else - the arena is not redrawn
    // while it is up, and with two buffers in play an uncovered screen would
    // flick between two stale frames. So the frame that was on screen when we
    // arrived is copied once into a texture and laid down underneath, which
    // is what makes a transparent dialog possible at all here.
    GLuint behind = 0;
    int behindW = (int)io.DisplaySize.x, behindH = (int)io.DisplaySize.y;
    if ( behindW > 0 && behindH > 0 )
    {
        glGenTextures( 1, &behind );
        glBindTexture( GL_TEXTURE_2D, behind );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F );
        glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, behindW, behindH, 0 );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    while (!decided && !uMenu::quickexit)
    {
        CheckDisplaySizeRebuild();
        // No pointer here at all. Everything in this box is reached from the
        // keyboard - tab between the fields, enter to sign in, escape to give
        // up - and a cursor floating over a game you cannot steer is only in
        // the way.
        SDL_HideCursor();
        ImGui::GetIO().MouseDrawCursor = false;
        SDL_WM_GrabInput(SDL_GRAB_OFF);



        st_DoToDo();
        tAdvanceFrame();
        sn_SendPlanned();

        double now = tSysTimeFloat();
        // The window system only sends characters to somebody who asked for
        // them. Normally the interface asks once a field is active; while none
        // is, nobody asks, and no key ever arrives as text - so the box could
        // not be typed into even in principle. Asked for here directly, for as
        // long as it is up.

        if ( sr_screen && !SDL_TextInputActive( sr_screen ) )
            SDL_StartTextInput( sr_screen );

        io.DeltaTime = (float)(now - lastFrame);
        if (io.DeltaTime <= 0.0f || io.DeltaTime > 0.25f) io.DeltaTime = 1.0f / 60.0f;
        lastFrame = now;

        // The same handling for a typed key wherever it arrives from. An event
        // reaches exactly one of the two queues: the game's filter takes most
        // of them into its own ring, and what it does not take stays in the
        // window system's.
        auto take = [&]( SDL_Event const & ev )
        {
            if ( ev.type == SDL_EVENT_TEXT_INPUT && ev.text.text )
            {
                if ( s_loginField > 1 )
                    return;

                char * to = s_loginField ? passBuf : userBuf;
                size_t const cap = s_loginField ? sizeof( passBuf ) : sizeof( userBuf );
                size_t at = strlen( to );

                for ( char const * c = ev.text.text; *c && at + 1 < cap; ++c )
                    to[ at++ ] = *c;

                to[ at ] = 0;
                return;
            }

            if ( ev.type != SDL_EVENT_KEY_DOWN )
                return;

            switch ( ev.key.key )
            {
            case SDLK_BACKSPACE:
                {
                    if ( s_loginField > 1 )
                        break;

                    char * to = s_loginField ? passBuf : userBuf;
                    size_t const at = strlen( to );
                    if ( at > 0 )
                        to[ at - 1 ] = 0;
                }
                break;

            case SDLK_TAB:
            case SDLK_DOWN:
                s_loginField = ( s_loginField + 1 ) % 3;
                break;

            case SDLK_UP:
                s_loginField = ( s_loginField + 2 ) % 3;
                break;

            case SDLK_LEFT:
                if ( s_loginField == 2 && storeMode > -1 )
                    --storeMode;
                break;

            case SDLK_RIGHT:
                if ( s_loginField == 2 && storeMode < 1 )
                    ++storeMode;
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                decided = true; accepted = true;
                break;

            case SDLK_ESCAPE:
                decided = true; accepted = false;
                break;

            default:
                break;
            }
        };

        {
            SDL_Event raw;
            while ( SDL_PollEvent( &raw ) )
            {
                ImGui_ImplSDL3_ProcessEvent( &raw );

                take( raw );

                if ( raw.type == SDL_EVENT_QUIT )
                    uMenu::quickexit = uMenu::QuickExit_Total;
            }
        }

        SDL_Event event;
        while (su_GetSDLInput(event))
        {
            if (event.type == SDL_EVENT_QUIT) { uMenu::quickexit = uMenu::QuickExit_Total; break; }

            take( event );

            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        // Did anybody else open a frame while we were away? If the game keeps
        // starting its own inside this loop, the interface is reset under us
        // every iteration and no fix on our side can hold.
        {
            ImGuiContext * ctx = ImGui::GetCurrentContext();
            if ( ctx && ctx->WithinFrameScope )
                ImGui::EndFrame();
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
        ImGui::NewFrame();

        ImVec2 screen = io.DisplaySize;
        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        if ( behind )
        {
            // upside down, because the screen was read the way GL stores it
            bg->AddImage( (ImTextureID)(intptr_t)behind, ImVec2(0,0), screen,
                          ImVec2(0,1), ImVec2(1,0) );
        }
        else
        {
            bg->AddRectFilled(ImVec2(0,0), screen, rc_Fade(th.page, 0.97f));
            rc_DrawBackdrop(bg, ImVec2(0,0), screen, 1.0f);
        }

        // Tall enough for what is in it. The height is fixed and the window
        // has no scrollbar, so anything that does not fit is not drawn at all
        // - which is how the storage row and both buttons came to be invisible
        // the moment they were added. Check this when adding a line.
        float w = 560.0f, h = 430.0f;
        ImGui::SetNextWindowPos(ImVec2((screen.x - w) * 0.5f, (screen.y - h) * 0.44f));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        // It has to be the front window and it has to hold the keyboard: a
        // field can only be focused inside a focused window, and told never to
        // come to the front this one sat behind whatever the game had open,
        // which is why asking for focus on the field alone changed nothing.
        if ( !io.WantTextInput )
            ImGui::SetNextWindowFocus();

        ImGui::Begin("##login", NULL,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoMove);

        // Asked for on this window, now that it exists. Requesting focus for
        // "the next window" is not the same thing when the window is rebuilt
        // every frame, and without a focused window no field inside it can
        // become active - which is what left the box deaf.
        if ( !ImGui::IsWindowFocused( ImGuiFocusedFlags_RootAndChildWindows ) )
            ImGui::SetWindowFocus();

        rc_Tracked("SIGN IN", 2.0f, th.textMute);
        ImGui::Spacing();

        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        ImGui::TextUnformatted(heading && *heading ? heading : "Login");
        if (g_FontHeader) ImGui::PopFont();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        // The two fields, drawn rather than asked for. What is typed is
        // collected above and shown here, so nothing depends on the interface
        // agreeing to give this box the keyboard. Click either one to aim at
        // it; tab moves between them.
        {
            ImDrawList * dr = ImGui::GetWindowDrawList();
            float const rowH = 34.0f;
            float const fullW = ImGui::GetContentRegionAvail().x;

            for ( int which = 0; which < 2; ++which )
            {
                ImGui::TextColored( ImGui::ColorConvertU32ToFloat4( th.textMute ),
                                    which ? "Password" : "Username" );

                ImVec2 const at = ImGui::GetCursorScreenPos();
                ImVec2 const to( at.x + fullW, at.y + rowH );

                bool const mine = ( s_loginField == which );

                ImGui::Dummy( ImVec2( fullW, rowH ) );

                dr->AddRectFilled( at, to, rc_Fade( th.surface, mine ? 1.0f : 0.75f ), 6.0f );
                dr->AddRect( at, to, mine ? th.second.line : th.line, 6.0f, 0, mine ? 1.6f : 1.0f );

                char shown[ 160 ];
                if ( which )
                {
                    size_t const n = strlen( passBuf );
                    size_t const dots = n < sizeof( shown ) - 1 ? n : sizeof( shown ) - 1;
                    for ( size_t i = 0; i < dots; ++i )
                        shown[i] = '*';
                    shown[ dots ] = 0;
                }
                else
                {
                    snprintf( shown, sizeof( shown ), "%s", userBuf );
                }

                ImVec2 const textAt( at.x + 10.0f, at.y + ( rowH - ImGui::GetFontSize() ) * 0.5f );
                dr->AddText( textAt, th.text, shown );

                // a caret that blinks only where you are actually typing
                if ( mine && fmodf( (float)tSysTimeFloat(), 1.0f ) < 0.5f )
                {
                    float const cx = textAt.x + ImGui::CalcTextSize( shown ).x + 1.0f;
                    dr->AddLine( ImVec2( cx, at.y + 6.0f ), ImVec2( cx, to.y - 6.0f ), th.text, 1.4f );
                }

                if ( !which )
                    ImGui::Dummy( ImVec2( 0.0f, 8.0f ) );
            }

            focusOnce = false;
        }

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        // What happens to the password afterwards. A drop down would have been
        // a setting nobody could reach: this box hides the pointer on purpose,
        // so the three choices are laid out side by side and walked with the
        // arrow keys, the same as the fields above are reached with tab.
        {
            ImGui::TextColored( ImGui::ColorConvertU32ToFloat4( th.textMute ), "Remember password" );

            if ( storeMode < -1 ) storeMode = -1;
            if ( storeMode >  1 ) storeMode =  1;

            char const * modes[ 3 ] = { "Never", "This session", "On disk" };

            ImDrawList * dr = ImGui::GetWindowDrawList();
            float const fullW = ImGui::GetContentRegionAvail().x;
            float const cellW = fullW / 3.0f;
            float const rowH = 32.0f;

            ImVec2 const at = ImGui::GetCursorScreenPos();
            ImGui::Dummy( ImVec2( fullW, rowH ) );

            bool const mine = ( s_loginField == 2 );

            for ( int i = 0; i < 3; ++i )
            {
                ImVec2 const cellAt( at.x + cellW * i, at.y );
                ImVec2 const cellTo( cellAt.x + cellW - 6.0f, at.y + rowH );

                bool const chosen = ( storeMode == i - 1 );

                dr->AddRectFilled( cellAt, cellTo,
                                   rc_Fade( chosen ? th.primary.solid : th.surface,
                                            chosen ? 0.9f : 0.7f ), 6.0f );
                dr->AddRect( cellAt, cellTo,
                             ( mine && chosen ) ? th.second.line : th.line, 6.0f, 0,
                             ( mine && chosen ) ? 1.8f : 1.0f );

                ImVec2 const sz = ImGui::CalcTextSize( modes[i] );
                dr->AddText( ImVec2( cellAt.x + ( cellW - 6.0f - sz.x ) * 0.5f,
                                     at.y + ( rowH - sz.y ) * 0.5f ),
                             chosen ? th.text : th.textDim, modes[i] );
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 6.0f));

        if (storeMode > 0)
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(th.warn),
                               "kept on disk, in the clear");
        else
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(th.textMute),
                               "tab moves  \xc2\xb7  arrows choose  \xc2\xb7  enter signs in");

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        if (ImGui::Button("Sign in", ImVec2(150.0f, 34.0f))) { decided = true; accepted = true; }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(150.0f, 34.0f))) { decided = true; accepted = false; }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { decided = true; accepted = false; }

        ImGui::End();


        rc_DrawToasts();

        // the same last word on the pointer every other screen gets,
        // instead of the plain arrow the library falls back to
        PaletteFrame();

    ImGui::Render();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glPopAttrib();

        rSysDep::SwapGL();
    }

    if ( behind )
        glDeleteTextures( 1, &behind );

    if (accepted)
    {
        username = userBuf;
        password = passBuf;
    }

    // not ours to keep
    memset(passBuf, 0, sizeof(passBuf));

    su_ClearKeys();
    return accepted;
}

void ModMenu::RunCustomInGameMenu() {
    if (!g_Initialized) {
        Init();
    }
    ImGuiIO& io = ImGui::GetIO();
    su_ClearKeys();
    
    // Purge pending SDL events
    SDL_Event purgeEvent;
    while (su_GetSDLInput(purgeEvent)) {
        // Discard
    }
    
    // Reset ImGui's mouse click state on enter
    for (int i = 0; i < 5; i++) {
        io.MouseDown[i] = false;
        io.MouseClicked[i] = false;
    }
    
    bool inGameMenuOpen = true;
    ModMenu::g_InGameMenuOpen = true;
    g_MenuOpen = false;
    g_ActiveTab = 0; // Default to Dashboard on entry
    
    static float targetScrollY = 0.0f;
    static float currentScrollY = 0.0f;
    static float diagFlashTimer = 0.0f;
    
    float startupCooldown = 0.3f; // Cooldown to absorb stray inputs
    
    while (inGameMenuOpen && !uMenu::quickexit && !uMenu::exitToMain) {
        CheckDisplaySizeRebuild();
        ShowSystemCursor();
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        io.MouseDrawCursor = false;
        
        st_DoToDo();
        tAdvanceFrame();
        
        if (sg_RequestedDisconnection) {
            break;
        }
        if (sg_IngameMenu && sg_IngameMenu->ShouldExit()) {
            break;
        }
        if (uMenu::exitToMain) {
            break;
        }
        
        // Aggressive network pump: flush all pending packets immediately.
        // The old uMenu::Enter() loop did this implicitly through its tight
        // idle callback. Without this, opening the pause menu wouldn't
        // "unfreeze" a stalled game — packets would remain queued.
        sn_Receive();
        nNetObject::SyncAll();
        sn_SendPlanned();
        
        gameloop_idle();
        
        // Second pump after gameloop_idle to push out any state changes
        // that the game simulation produced (acks, sync requests, etc.)
        sn_Receive();
        sn_SendPlanned();
        
        SDL_Event event;
        while (su_GetSDLInput(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                uMenu::quickexit = uMenu::QuickExit_Total;
                break;
            }
            if (ProcessEvent(&event)) {
                continue;
            }
            su_HandleEvent(event, true);
        }
        
        if (g_CloseInGameMenuRequested) {
            g_CloseInGameMenuRequested = false;
            inGameMenuOpen = false;
        }
        
        if (!inGameMenuOpen || uMenu::quickexit || uMenu::exitToMain) {
            break;
        }
        
        // --- HUD EDITOR MODE OVERLAY IN IN-GAME MENU ---
        if (isHudEditing) {
            // Update delta time
            // engine clock rather than gettimeofday: that one is POSIX only and
            // monotonic here, so a clock adjustment cannot skew the frame time
            static double lastFrameStamp = 0.0;
            const double frameStamp = tRealSysTimeFloat();
            if (lastFrameStamp > 0.0) {
                const double dt = frameStamp - lastFrameStamp;
                if (dt > 0.0) {
                    io.DeltaTime = (float)dt;
                    lastFrameStamp = frameStamp;
                }
            } else {
                lastFrameStamp = frameStamp;
            }

            ShowSystemCursor();
            io.MouseDrawCursor = true;

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
            ImGui::NewFrame();

            ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
            bgDl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.65f)));
            DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);

            HudManager::Update(io.DeltaTime);
            HudManager::Render();

            PaletteFrame();
    ImGui::Render();
            
            GLboolean blendEnabled = glIsEnabled(GL_BLEND);
            GLint blendSrcSrc, blendSrcDst;
            glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcSrc);
            glGetIntegerv(GL_BLEND_DST_ALPHA, &blendSrcDst);
            GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
            GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
            GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            
            if (!blendEnabled) glDisable(GL_BLEND);
            else glBlendFunc(blendSrcSrc, blendSrcDst);
            if (depthEnabled) glEnable(GL_DEPTH_TEST);
            if (lightingEnabled) glEnable(GL_LIGHTING);
            if (cullEnabled) glEnable(GL_CULL_FACE);

            rSysDep::SwapGL();
            rSysDep::ClearGL();
            continue;
        }
        
        // Setup ImGui frames
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
    FeedMousePosition();
        
        // engine clock rather than gettimeofday: that one is POSIX only and
        // monotonic here, so a clock adjustment cannot skew the frame time
        static double lastFrameStamp = 0.0;
        const double frameStamp = tRealSysTimeFloat();
        if (lastFrameStamp > 0.0) {
            const double dt = frameStamp - lastFrameStamp;
            if (dt > 0.0) {
                io.DeltaTime = (float)dt;
                lastFrameStamp = frameStamp;
            }
        } else {
            lastFrameStamp = frameStamp;
        }

        HudManager::Update(io.DeltaTime);
        
        if (diagFlashTimer > 0.0f) {
            diagFlashTimer -= io.DeltaTime;
        }
        
        if (startupCooldown > 0.0f) {
            startupCooldown -= io.DeltaTime;
            g_DashboardActionTriggered = false;
            for (int i = 0; i < 5; i++) {
                io.MouseDown[i] = false;
                io.MouseClicked[i] = false;
            }
        }
        
        ImGui::NewFrame();
        
        // --- DRAW IN-GAME DASHBOARD ---
        // 1. Semi-transparent backdrop overlay
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGuiWindowFlags bgFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        // Clear behind the menu, not curtained. The arena is still being drawn
        // underneath and covering it was never the intent - the card carries
        // its own background, which is the part that needs to be readable.
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.03f, g_MenuDim));
        ImGui::Begin("##InGameDashboardBG", nullptr, bgFlags);
        ImDrawList* bgDl = ImGui::GetWindowDrawList();
        DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        
        RCL_PollQueueSummaryTick(g_ActiveTab == 0 || g_ActiveTab == 7);

        // 2. Centered In-Game Dashboard Card Window
        bool hasPanel2 = (io.DisplaySize.x >= 1200.0f);
        ImVec2 cardSize(hasPanel2 ? 1180.0f : 750.0f, 700.0f);
        ImVec2 cardPos((io.DisplaySize.x - cardSize.x) * 0.5f, (io.DisplaySize.y - cardSize.y) * 0.5f);
        ImGui::SetNextWindowPos(cardPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(cardSize, ImGuiCond_Always);
        // this one sits on top of ##InGameDashboardBG, which is the piece that
        // stays put, so the card itself has to be free to rise above it
        ImGuiWindowFlags cardFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(g_MenuBgColor.x, g_MenuBgColor.y, g_MenuBgColor.z, g_MenuBgAlpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.14f, 0.16f, 0.5f));
        
        ImGui::Begin("##InGameDashboardCard", nullptr, cardFlags);
        
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        
        // Accent/RGB update
        float menuTime = (float)ImGui::GetTime();
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(fmodf(menuTime * g_RGBSpeed, 1.0f), 1.0f, 1.0f, r, g, b);
        if (g_RGBAccent) {
            g_AccentColor = ImVec4(r, g, b, 1.0f);
        } else {
            g_AccentColor = g_AccentColor1;
        }
        
        // 3. Header and title
        float paddingX = cardSize.x < 1180.0f ? 20.0f : 30.0f;
        float headerY = 70.0f;
        
        char verBuf[64];
        if (sn_programVersion == "ILONIUM" || sn_programVersion == "ilonium") {
            snprintf(verBuf, sizeof(verBuf), "ILONIUM | COMPETITIVE EDITION");
        } else {
            snprintf(verBuf, sizeof(verBuf), "v%s | COMPETITIVE EDITION", (const char*)sn_programVersion);
        }
        ImVec2 verSize = ImGui::CalcTextSize(verBuf);

        std::string headerTitle = "RETROCYCLES IN-GAME PAUSE MENU";
        if (sg_OnRemoteServer()) {
            if (sg_lastServerName.Len() > 0) {
                headerTitle = "SERVER: " + std::string(static_cast<const char*>(tColoredString::RemoveColors(sg_lastServerName)));
            } else if (sg_lastServerIP.Len() > 0) {
                char serverAddr[128];
                snprintf(serverAddr, sizeof(serverAddr), "%s:%u", (const char*)sg_lastServerIP, sg_lastServerPort);
                headerTitle = "SERVER: " + std::string(serverAddr);
            } else {
                headerTitle = "SERVER: Connected Server";
            }
        }

        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        float maxTitleW = cardSize.x - 2.0f * paddingX - verSize.x - 40.0f;
        std::string displayTitle = headerTitle;
        if (ImGui::CalcTextSize(displayTitle.c_str()).x > maxTitleW) {
            while (displayTitle.length() > 5 && ImGui::CalcTextSize((displayTitle + "...").c_str()).x > maxTitleW) {
                displayTitle.pop_back();
            }
            displayTitle += "...";
        }
        dl->AddText(ImVec2(winPos.x + paddingX, winPos.y + 25.0f), IM_COL32(255, 255, 255, 255), displayTitle.c_str());
        if (g_FontHeader) ImGui::PopFont();
        
        dl->AddText(ImVec2(winPos.x + cardSize.x - paddingX - verSize.x, winPos.y + 35.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.6f, 0.8f)), verBuf);
        
        // The same rule as the other layout uses: quiet in the middle, gone at
        // the edges. Written once here would be better still, and that is the
        // next thing this file needs.
        {
            rcTheme const & th = rc_Theme();
            float ruleY = winPos.y + headerY;
            ImVec2 from( winPos.x + paddingX, ruleY );
            ImVec2 to( winPos.x + cardSize.x - paddingX, ruleY + 1.0f );
            ImVec2 mid( ( from.x + to.x ) * 0.5f, to.y );

            dl->AddRectFilledMultiColor( from, mid,
                rc_Fade( th.primary.solid, 0.0f ), rc_Fade( th.primary.solid, 0.5f ),
                rc_Fade( th.primary.solid, 0.5f ), rc_Fade( th.primary.solid, 0.0f ) );

            dl->AddRectFilledMultiColor( ImVec2( mid.x, from.y ), to,
                rc_Fade( th.primary.solid, 0.5f ), rc_Fade( th.primary.solid, 0.0f ),
                rc_Fade( th.primary.solid, 0.0f ), rc_Fade( th.primary.solid, 0.5f ) );
        }
        
        // 4. Grid Columns layout (1180x700px Card Optimized)
        float colY = headerY + 30.0f;
        float colH = cardSize.y - colY - 35.0f; // Calculate dynamically
        float colW0 = 230.0f;
        float colW = hasPanel2 ? 425.0f : (cardSize.x - colW0 - 2.0f * paddingX - 20.0f);
        
        auto drawPanel = [&](ImVec2 pos, ImVec2 size, bool active, const char* title) {
            ::drawPanel(dl, pos, size, active, title);
        };
        
        // Setup column positions
        ImVec2 panel0Pos(winPos.x + paddingX, winPos.y + colY);
        ImVec2 panel0Size(colW0, colH);
        
        ImVec2 panel1Pos(winPos.x + paddingX + colW0 + 20.0f, winPos.y + colY);
        ImVec2 panel1Size(colW, colH);
        
        ImVec2 panel2Pos(winPos.x + paddingX + colW0 + 40.0f + colW, winPos.y + colY);
        ImVec2 panel2Size(colW, colH);
        ImVec2 panelSize = panel1Size; // For compatibility
        
        // --- COLUMN 0: NAVIGATION (Left) ---
        drawPanel(panel0Pos, panel0Size, g_DashboardActiveCol == 0, "NAVIGATION");
        
        std::vector<const char*> navLabels;
        navLabels.push_back("Dashboard");
        navLabels.push_back("Pickup");
        navLabels.push_back("Resume Match");
        navLabels.push_back("Teams & Voting");
        navLabels.push_back("Settings");
        navLabels.push_back("Mod Menu");
        if (sg_hasLastServer) {
            navLabels.push_back("Reconnect");
        }
        navLabels.push_back("Disconnect");
        
        ImGui::SetCursorScreenPos(ImVec2(panel0Pos.x + 5.0f, panel0Pos.y + 55.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        bool showNav = ImGui::BeginChild("##NavigationScroll", ImVec2(panel0Size.x - 10.0f, panel0Size.y - 70.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NavFlattened);
        if (showNav) {
            ImDrawList* navDl = ImGui::GetWindowDrawList();
            ImVec2 navStart = ImGui::GetCursorScreenPos();
            
            int numLabels = (int)navLabels.size();
            for (int i = 0; i < numLabels; i++) {
                float itemY = navStart.y + i * 48.0f;
                float itemH = 38.0f;
                ImVec2 itemMin(panel0Pos.x + 20.0f, itemY);
                ImVec2 itemMax(panel0Pos.x + colW0 - 20.0f, itemY + itemH);
                
                bool hovered = io.MousePos.x >= itemMin.x && io.MousePos.x <= itemMax.x &&
                               io.MousePos.y >= itemMin.y && io.MousePos.y <= itemMax.y &&
                               ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
                bool selected = (g_DashboardActiveCol == 0) && (g_DashboardLeftSelected == i);
                
                if (hovered && io.MouseClicked[0]) {
                    g_DashboardLeftSelected = i;
                    g_DashboardActiveCol = 0;
                    g_DashboardActionTriggered = true;
                }
                
                // Map the displayed items to g_ActiveTab
                bool isActiveTab = false;
                std::string labelStr = navLabels[i];
                if (labelStr == "Dashboard") isActiveTab = (g_ActiveTab == 0);
                else if (labelStr == "Pickup") isActiveTab = (g_ActiveTab == 7);
                else if (labelStr == "Teams & Voting") isActiveTab = (g_ActiveTab == 2);
                else if (labelStr == "Settings") isActiveTab = (g_ActiveTab == 3);
                else if (labelStr == "Mod Menu") isActiveTab = (g_ActiveTab == 4);
                
                ImU32 itemBg = (selected || hovered || isActiveTab) ? GetThemeColor(0.1f * i) : ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, 0.6f));
                ImU32 itemBorder = (selected || isActiveTab) ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.5f));
                
                navDl->AddRectFilled(itemMin, itemMax, itemBg, 8.0f);
                navDl->AddRect(itemMin, itemMax, itemBorder, 8.0f, 0, (selected || isActiveTab) ? 1.5f : 1.0f);
                
                if (selected) {
                    navDl->AddCircleFilled(ImVec2(itemMin.x + 15.0f, itemMin.y + itemH * 0.5f), 4.0f, GetThemeColor(0.5f));
                }
                
                navDl->AddText(ImVec2(itemMin.x + (selected ? 30.0f : 20.0f), itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                            IM_COL32(255, 255, 255, 255), navLabels[i]);
                
                if (g_DashboardActionTriggered && selected) {
                    g_DashboardActionTriggered = false;
                    if (labelStr == "Resume Match") {
                        inGameMenuOpen = false;
                    } else if (labelStr == "Mod Menu") {
                        if (g_ActiveTab == 4) {
                            g_ActiveTab = 0;
                            g_DashboardLeftSelected = 0;
                        } else {
                            g_ActiveTab = 4;
                        }
                    } else if (labelStr == "Reconnect") {
                        s_PendingReconnect = true;
                        ret_to_MainMenu();
                        inGameMenuOpen = false;
                    } else if (labelStr == "Disconnect") {
                        ret_to_MainMenu();
                        inGameMenuOpen = false;
                    } else {
                        if (labelStr == "Dashboard") g_ActiveTab = 0;
                        else if (labelStr == "Pickup") g_ActiveTab = 7;
                        else if (labelStr == "Teams & Voting") g_ActiveTab = 2;
                        else if (labelStr == "Settings") g_ActiveTab = 3;
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (g_ActiveTab == 0) {
            RenderOriginalDashboard(dl, io, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, colW, colH);
        } else if (g_ActiveTab == 7) {
            RenderEsportsDashboard(dl, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2, colW, colH);
        }
        else if (g_ActiveTab == 2) {
            // --- TAB 2: TEAMS & VOTING ---
            // COLUMN 1: TEAMS CONFIGURATION
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, hasPanel2 ? "TEAMS CONFIGURATION" : "TEAMS & VOTING");
            
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 60.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("##InGameTeamsScroll", ImVec2(colW - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            
            RenderInGameTeams(colW - 30.0f);
            
            if (!hasPanel2) {
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "VOTING & POLICE");
                ImGui::Spacing();
                RenderInGameVotingAndPolice(colW - 30.0f);
            }
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            if (hasPanel2) {
                // COLUMN 2: VOTING & POLICE
                drawPanel(panel2Pos, panel2Size, g_DashboardActiveCol == 2, "VOTING & POLICE");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 15.0f, panel2Pos.y + 60.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::BeginChild("##InGameVotingScroll", ImVec2(colW - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
                
                RenderInGameVotingAndPolice(colW - 30.0f);
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        }
        else if (g_ActiveTab == 3) {
            // --- TAB 3: SETTINGS ---
            // COLUMN 1: PLAYER SETUP & SETTINGS
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, hasPanel2 ? "PLAYER SETUP & SETTINGS" : "GAME SETTINGS");
            

            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 60.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("##InGameSettingsScroll", ImVec2(colW - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            
            RenderInGameSettings(colW - 30.0f);
            
            if (!hasPanel2) {
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.0f, 0.94f, 1.0f, 1.0f), "KEYBINDINGS");
                ImGui::Spacing();
                
                DrawKeybindList(colW, 0);
            }
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            if (hasPanel2) {
                // COLUMN 2: KEYBINDINGS
                drawPanel(panel2Pos, panel2Size, g_DashboardActiveCol == 2, "KEYBINDINGS");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 15.0f, panel2Pos.y + 60.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::BeginChild("KeybindingsScroll", ImVec2(colW - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
                
                DrawKeybindList(colW, 0);
                
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        } else if (g_ActiveTab == 4) {
            bool savedOpen = g_MenuOpen;
            float savedAlphaInner = g_MenuAlpha;
            g_MenuOpen = true;
            g_MenuAlpha = 1.0f;
            RenderInner();
            g_MenuOpen = savedOpen;
            g_MenuAlpha = savedAlphaInner;

        }
        
        // Render modals/popups
        RenderModMenuModals();
        
        // Draw the embedded media player card below the in-game pause menu card
        if (g_ActiveTab == 0) {
            MediaWidget* media = MediaWidget::GetInstance();
            if (media) {
                float bottomGap = io.DisplaySize.y - (cardPos.y + cardSize.y);
                float mediaY = cardPos.y + cardSize.y + (bottomGap - 125.0f) * 0.5f;
                if (mediaY + 125.0f > io.DisplaySize.y - 10.0f) {
                    mediaY = io.DisplaySize.y - 125.0f - 10.0f;
                }
                ImVec2 mediaPos(cardPos.x + (cardSize.x - 380.0f) * 0.5f, mediaY);
                media->Draw(ImGui::GetForegroundDrawList(), mediaPos, 1.0f);
            }
        }
        
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
        
        PaletteFrame();
    ImGui::Render();
        
        GLboolean blendEnabled = glIsEnabled(GL_BLEND);
        GLint blendSrcSrc, blendSrcDst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendSrcDst);
        GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        
        if (!blendEnabled) glDisable(GL_BLEND);
        else glBlendFunc(blendSrcSrc, blendSrcDst);
        if (depthEnabled) glEnable(GL_DEPTH_TEST);
        if (lightingEnabled) glEnable(GL_LIGHTING);
        if (cullEnabled) glEnable(GL_CULL_FACE);
        
        rSysDep::SwapGL();
        rSysDep::ClearGL();
    }
    
    su_ClearKeys();
    ModMenu::g_InGameMenuOpen = false;
    while (su_GetSDLInput(purgeEvent)) {
        // Discard
    }
    SDL_HideCursor();
    if (currentScreensetting.fullscreen || su_mouseGrab) {
        SDL_WM_GrabInput(SDL_GRAB_ON);
    } else {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }
    SDL_HideCursor();
}

// =========================================================================
// Pickup / Esports Dashboard and Queue Management Implementation
// =========================================================================

static char rcl_password[128] = "";
static std::string s_QueueStatusMsg = "";
static ImVec4 s_QueueStatusCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static float s_DashTargetScrollY = 0.0f;
static float s_DashCurrentScrollY = 0.0f;

static std::string LaneKeyToQueueId(const std::string& laneKey) {
    if (laneKey == "fort:open" || laneKey == "fort") return "fort";
    if (laneKey == "sumo:open") return "sumobar_open";
    if (laneKey == "sumo:beginner") return "sumobar_beginner";
    if (laneKey == "sumo:pro") return "sumobar_pro";
    if (laneKey == "tst:open") return "tst_open";
    if (laneKey == "tst:beginner") return "tst_beginner";
    if (laneKey == "tst:pro") return "tst_pro";
    if (laneKey == "2s:open") return "two_s_open";
    if (laneKey == "2s:beginner") return "two_s_beginner";
    if (laneKey == "2s:pro") return "two_s_pro";
    return "";
}

static std::string QueueIdToLaneKey(const std::string& queueId) {
    if (queueId == "fort") return "fort:open";
    if (queueId == "sumobar_open") return "sumo:open";
    if (queueId == "sumobar_beginner") return "sumo:beginner";
    if (queueId == "sumobar_pro") return "sumo:pro";
    if (queueId == "tst_open") return "tst:open";
    if (queueId == "tst_beginner") return "tst:beginner";
    if (queueId == "tst_pro") return "tst:pro";
    if (queueId == "two_s_open") return "2s:open";
    if (queueId == "two_s_beginner") return "2s:beginner";
    if (queueId == "two_s_pro") return "2s:pro";
    return "";
}

bool ModMenu::JoinQueue(const std::string& laneKey) {
    std::string qId = LaneKeyToQueueId(laneKey);
    if (qId.empty()) return false;
    for (const auto& jq : s_JoinedQueues) {
        if (jq == qId) return true;
    }
    s_JoinedQueues.push_back(qId);
    RCL_SendQueueAction(s_RclUsername, "join", laneKey);
    return true;
}

bool ModMenu::LeaveQueue(const std::string& laneKey) {
    std::string qId = LaneKeyToQueueId(laneKey);
    if (qId.empty()) return false;
    bool found = false;
    for (auto it = s_JoinedQueues.begin(); it != s_JoinedQueues.end(); ) {
        if (*it == qId) {
            it = s_JoinedQueues.erase(it);
            found = true;
        } else {
            ++it;
        }
    }
    if (found) {
        RCL_SendQueueAction(s_RclUsername, "leave", laneKey);
    }
    return found;
}

bool ModMenu::IsInQueue(const std::string& laneKey) {
    std::string qId = LaneKeyToQueueId(laneKey);
    if (qId.empty()) return false;
    for (const auto& jq : s_JoinedQueues) {
        if (jq == qId) return true;
    }
    return false;
}

bool ModMenu::CheckEligibility(const std::string& laneKey, std::string& outError) {
    if (!RCL_ElosLoaded()) {
        if (laneKey.find(":pro") != std::string::npos || laneKey.find(":beginner") != std::string::npos) {
            outError = "ELO is still loading. Please try again in a moment.";
            return false;
        }
        return true;
    }
    int playerElo = 0;
    std::string mode = "";
    int eloRequirement = 0;
    
    if (laneKey.find("tst:") == 0) { mode = "TST"; playerElo = RCL_GetPlayerEloTst(); }
    else if (laneKey.find("sumo:") == 0) { mode = "SUMOBAR"; playerElo = RCL_GetPlayerEloSumobar(); }
    else if (laneKey.find("2s:") == 0) { mode = "2S"; playerElo = RCL_GetPlayerElo2s(); }
    else return true; // fort has no requirements
    
    if (laneKey.find(":pro") != std::string::npos) eloRequirement = 1900;
    else if (laneKey.find(":beginner") != std::string::npos) eloRequirement = -1900;
    else return true; // open
    
    if (playerElo == -1) {
        outError = "ELO is still loading. Please try again in a moment.";
        return false;
    }
    if (eloRequirement == 1900 && playerElo < 1900) {
        outError = "Blocked: " + mode + " PRO requires at least 1900 ELO.";
        return false;
    }
    if (eloRequirement == -1900 && playerElo >= 1900) {
        outError = "Blocked: " + mode + " BEGINNER requires less than 1900 ELO.";
        return false;
    }
    return true;
}

static void SendClientChatMessage(const std::string& msg) {
    ModMenu::SendLocalChat(msg.c_str());
}

bool ModMenu::HandleChatCommand(const std::string& msg) {
    std::vector<std::string> tokens;
    std::stringstream ss(msg);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) return false;
    
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd != "!add" && cmd != "!who" && cmd != "!remove") {
        return false;
    }
    
    auto getCanonicalMode = [](const std::string& s) -> std::string {
        std::string lower = s;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "tst") return "tst";
        if (lower == "sumobar" || lower == "sumo") return "sumo";
        if (lower == "fort") return "fort";
        if (lower == "2s") return "2s";
        return "";
    };
    
    auto getCanonicalDivision = [](const std::string& s) -> std::string {
        std::string lower = s;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "pro") return "pro";
        if (lower == "beginner" || lower == "beg") return "beginner";
        if (lower == "open") return "open";
        return "";
    };
    
    if (cmd == "!add") {
        if (tokens.size() < 2) {
            SendClientChatMessage("[RCL] Usage: !add <mode1> [mode2] ... OR !add <mode> [pro|beginner]");
            return true;
        }
        
        std::string firstMode = getCanonicalMode(tokens[1]);
        if (firstMode.empty()) {
            SendClientChatMessage("[RCL] Unknown mode: " + tokens[1]);
            return true;
        }
        
        std::vector<std::string> lanesToJoin;
        
        if (tokens.size() == 3) {
            std::string div = getCanonicalDivision(tokens[2]);
            if (!div.empty()) {
                if (firstMode == "fort" && div != "open") {
                    SendClientChatMessage("[RCL] FORT only has an OPEN queue.");
                    return true;
                }
                lanesToJoin.push_back(firstMode + ":" + div);
            } else {
                std::string secondMode = getCanonicalMode(tokens[2]);
                if (secondMode.empty()) {
                    SendClientChatMessage("[RCL] Unknown mode/division: " + tokens[2]);
                    return true;
                }
                lanesToJoin.push_back(firstMode + ":open");
                lanesToJoin.push_back(secondMode + ":open");
            }
        } else if (tokens.size() > 3) {
            for (size_t i = 1; i < tokens.size(); i++) {
                std::string m = getCanonicalMode(tokens[i]);
                if (!m.empty()) {
                    lanesToJoin.push_back(m + ":open");
                } else {
                    SendClientChatMessage("[RCL] Unknown mode: " + tokens[i]);
                    return true;
                }
            }
        } else {
            lanesToJoin.push_back(firstMode + ":open");
        }
        
        std::vector<std::string> successfullyJoined;
        std::vector<std::string> alreadyIn;
        std::vector<std::string> blocked;
        
        for (const auto& lane : lanesToJoin) {
            std::string err;
            if (!CheckEligibility(lane, err)) {
                blocked.push_back(err);
            } else {
                std::string qId = LaneKeyToQueueId(lane);
                bool already = false;
                for (const auto& jq : s_JoinedQueues) {
                    if (jq == qId) {
                        already = true;
                        break;
                    }
                }
                
                int cur = 0, req = 0;
                if (!RCL_GetQueueCount(lane, cur, req)) {
                    cur = 0;
                    req = (lane.find("fort:") == 0) ? 12 : ((lane.find("2s:") == 0) ? 4 : 8);
                }
                
                if (already) {
                    char buf[64];
                    std::string modeName = (lane == "fort:open") ? "FORT" : (lane == "sumo:open" ? "SUMOBAR" : (lane == "sumo:pro" ? "SUMOBAR PRO" : (lane == "sumo:beginner" ? "SUMOBAR BEG" : (lane == "tst:open" ? "TST" : (lane == "tst:pro" ? "TST PRO" : (lane == "tst:beginner" ? "TST BEG" : (lane == "2s:open" ? "2S" : (lane == "2s:pro" ? "2S PRO" : "2S BEG"))))))));
                    snprintf(buf, sizeof(buf), "%s (%d/%d)", modeName.c_str(), cur, req);
                    alreadyIn.push_back(buf);
                } else {
                    JoinQueue(lane);
                    cur += 1;
                    char buf[64];
                    std::string modeName = (lane == "fort:open") ? "FORT" : (lane == "sumo:open" ? "SUMOBAR" : (lane == "sumo:pro" ? "SUMOBAR PRO" : (lane == "sumo:beginner" ? "SUMOBAR BEG" : (lane == "tst:open" ? "TST" : (lane == "tst:pro" ? "TST PRO" : (lane == "tst:beginner" ? "TST BEG" : (lane == "2s:open" ? "2S" : (lane == "2s:pro" ? "2S PRO" : "2S BEG"))))))));
                    snprintf(buf, sizeof(buf), "%s (%d/%d)", modeName.c_str(), cur, req);
                    successfullyJoined.push_back(buf);
                }
            }
        }
        
        if (!blocked.empty()) {
            for (const auto& err : blocked) {
                SendClientChatMessage("[RCL] " + err);
            }
        }
        if (!successfullyJoined.empty()) {
            std::string reply = "[RCL] Added to ";
            for (size_t i = 0; i < successfullyJoined.size(); i++) {
                reply += successfullyJoined[i];
                if (i + 1 < successfullyJoined.size()) reply += ", ";
            }
            SendClientChatMessage(reply);
        }
        if (!alreadyIn.empty() && successfullyJoined.empty()) {
            std::string reply = "[RCL] Already in ";
            for (size_t i = 0; i < alreadyIn.size(); i++) {
                reply += alreadyIn[i];
                if (i + 1 < alreadyIn.size()) reply += ", ";
            }
            SendClientChatMessage(reply);
        }
        return true;
    }
    
    if (cmd == "!remove") {
        if (tokens.size() < 2) {
            SendClientChatMessage("[RCL] Usage: !remove <mode> [division] OR !remove all");
            return true;
        }
        
        std::string target = tokens[1];
        std::transform(target.begin(), target.end(), target.begin(), ::tolower);
        
        std::vector<std::string> queuesToRemove;
        
        if (target == "all") {
            // Remove from all queues they are in
            for (const auto& jq : s_JoinedQueues) {
                std::string lane = QueueIdToLaneKey(jq);
                if (!lane.empty()) {
                    queuesToRemove.push_back(lane);
                }
            }
        } else {
            // Parse mode and optional division
            std::string mode = getCanonicalMode(target);
            if (mode.empty()) {
                SendClientChatMessage("[RCL] Unknown mode: " + tokens[1]);
                return true;
            }
            
            if (tokens.size() >= 3) {
                std::string div = getCanonicalDivision(tokens[2]);
                if (div.empty()) {
                    SendClientChatMessage("[RCL] Unknown division: " + tokens[2]);
                    return true;
                }
                queuesToRemove.push_back(mode + ":" + div);
            } else {
                // If division not specified, remove all divisions for this mode
                queuesToRemove.push_back(mode + ":open");
                queuesToRemove.push_back(mode + ":beginner");
                queuesToRemove.push_back(mode + ":pro");
            }
        }
        
        std::vector<std::string> successfullyRemoved;
        
        for (const auto& lane : queuesToRemove) {
            std::string qId = LaneKeyToQueueId(lane);
            bool joined = false;
            for (const auto& jq : s_JoinedQueues) {
                if (jq == qId) {
                    joined = true;
                    break;
                }
            }
            
            if (joined) {
                // Leave queue
                LeaveQueue(lane);
                
                int cur = 0, req = 0;
                if (!RCL_GetQueueCount(lane, cur, req)) {
                    cur = 0;
                    req = (lane.find("fort:") == 0) ? 12 : ((lane.find("2s:") == 0) ? 4 : 8);
                }
                
                // Subtract 1 since we just left (before API updates)
                if (cur > 0) cur -= 1;
                
                std::string modeName = (lane == "fort:open") ? "FORT" : (lane == "sumo:open" ? "SUMOBAR" : (lane == "sumo:pro" ? "SUMOBAR PRO" : (lane == "sumo:beginner" ? "SUMOBAR BEG" : (lane == "tst:open" ? "TST" : (lane == "tst:pro" ? "TST PRO" : (lane == "tst:beginner" ? "TST BEG" : (lane == "2s:open" ? "2S" : (lane == "2s:pro" ? "2S PRO" : "2S BEG"))))))));
                char buf[64];
                snprintf(buf, sizeof(buf), "%s (%d/%d)", modeName.c_str(), cur, req);
                successfullyRemoved.push_back(buf);
            }
        }
        
        if (!successfullyRemoved.empty()) {
            std::string reply = "[RCL] Removed from ";
            for (size_t i = 0; i < successfullyRemoved.size(); i++) {
                reply += successfullyRemoved[i];
                if (i + 1 < successfullyRemoved.size()) reply += ", ";
            }
            SendClientChatMessage(reply);
        } else {
            SendClientChatMessage("[RCL] You are not in any matching queue.");
        }
        
        return true;
    }
    
    if (cmd == "!who") {
        std::string div = "open";
        if (tokens.size() >= 2) {
            div = getCanonicalDivision(tokens[1]);
            if (div.empty()) {
                SendClientChatMessage("[RCL] Unknown division: " + tokens[1]);
                return true;
            }
        }
        
        auto getQueueCountStr = [](const std::string& lane, const char* name, int maxDefault) -> std::string {
            int cur = 0, req = 0;
            if (!RCL_GetQueueCount(lane, cur, req)) {
                cur = 0;
                req = maxDefault;
            }
            char buf[64];
            snprintf(buf, sizeof(buf), "%s (%d/%d)", name, cur, req);
            return buf;
        };
        
        if (div == "pro") {
            std::string reply = "[RCL] Pro Queues: " +
                                getQueueCountStr("tst:pro", "TST Pro", 8) + ", " +
                                getQueueCountStr("sumo:pro", "Sumobar Pro", 8) + ", " +
                                getQueueCountStr("2s:pro", "2s Pro", 4);
            SendClientChatMessage(reply);
        } else if (div == "beginner") {
            std::string reply = "[RCL] Beginner Queues: " +
                                getQueueCountStr("tst:beginner", "TST Beg", 8) + ", " +
                                getQueueCountStr("sumo:beginner", "Sumobar Beg", 8) + ", " +
                                getQueueCountStr("2s:beginner", "2s Beg", 4);
            SendClientChatMessage(reply);
        } else {
            std::string reply = "[RCL] Open Queues: " +
                                getQueueCountStr("tst:open", "TST", 8) + ", " +
                                getQueueCountStr("sumo:open", "Sumobar", 8) + ", " +
                                getQueueCountStr("fort:open", "Fort", 12) + ", " +
                                getQueueCountStr("2s:open", "2s", 4);
            SendClientChatMessage(reply);
        }
        return true;
    }
    
    return false;
}

static void RenderOriginalDashboard(ImDrawList* dl, ImGuiIO& io, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH)
{
    if (!hasPanel2) {
        ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 20.0f, panel1Pos.y + 55.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (ImGui::Button("System Info", ImVec2((panel1Size.x - 50.0f)/2.0f, 30.0f))) {
            s_DashSubTab = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Profiles", ImVec2((panel1Size.x - 50.0f)/2.0f, 30.0f))) {
            s_DashSubTab = 1;
        }
        ImGui::PopStyleVar();
    }
    
    float offsetDashY = hasPanel2 ? 0.0f : 40.0f;
    
    if (hasPanel2 || s_DashSubTab == 0) {
        bool isMultiplayer = sg_OnRemoteServer();
        const char* panel1Title = isMultiplayer ? "SERVER INFORMATION" : "SYSTEM INFORMATION";
        drawPanel(dl, panel1Pos, panel1Size, g_DashboardActiveCol == 1, panel1Title);
        
        if (isMultiplayer) {
            float listY = panel1Pos.y + 60.0f + offsetDashY;
            float listH = colH - 80.0f - offsetDashY;
            
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, listY));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
            
            bool showPlayers = ImGui::BeginChild("##DashboardServerPlayers", ImVec2(colW - 30.0f, listH), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            if (showPlayers) {
                for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
                    ePlayerNetID* pni = se_PlayerNetIDs(i);
                    if (pni) {
                        // Who this player logged in as, if there is any way to
                        // know it.
                        //
                        // On a server we are running ourselves the real name is
                        // right there. On somebody else's server it is not sent:
                        // the player sync carries the name, ping, score and team
                        // and says nothing about authentication. So the only
                        // login a client can speak for is its own seat's, and
                        // claiming "not logged in" for everybody else was simply
                        // wrong - most of them are.
                        std::string loginStr;
                        bool mine = false;

#ifdef KRAWALL_SERVER
                        if (pni->IsAuthenticated() && pni->GetRawAuthenticatedName().Len() > 1) {
                            loginStr = (const char*)pni->GetRawAuthenticatedName();
                        }
#endif
                        if (loginStr.empty()) {
                            char const * said = rc_LoginOf(pni->GetName());
                            if (said && *said)
                                loginStr = said;
                        }

                        for (int seatIdx = 0; seatIdx < MAX_PLAYERS; ++seatIdx) {
                            ePlayer* seat = ePlayer::PlayerConfig(seatIdx);
                            if (seat && seat->netPlayer == pni) {
                                mine = true;
                                // A fresh profile carries a bare "@forums" with
                                // nobody in front of it. That is a default, not
                                // a login.
                                if (loginStr.empty()) {
                                    std::string gid = Cp1251ToUtf8((const char*)seat->globalID);
                                    size_t at = gid.find('@');
                                    if (at != std::string::npos && at > 0)
                                        loginStr = gid;
                                }
                                break;
                            }
                        }
                        
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.4f));
                        
                        ImGui::BeginChildFrame(ImGui::GetID(pni), ImVec2(colW - 55.0f, 44.0f));
                        
                        ImDrawList* cellDl = ImGui::GetWindowDrawList();
                        ImVec2 framePos = ImGui::GetWindowPos();
                        
                        RenderColoredText(cellDl, ImVec2(framePos.x + 10.0f, framePos.y + 4.0f), IM_COL32(255, 255, 255, 255), pni->GetName());
                        // Nothing at all beats a wrong answer: a remote player
                        // whose login we cannot see gets no line rather than a
                        // line saying they have none.
                        if (!loginStr.empty() || mine) {
                            // The stored id is already user@authority; only a
                            // bare authority needs the sign putting back.
                            std::string loginDisplay = loginStr.empty()
                                ? std::string("not logged in")
                                : ( loginStr.find('@') != std::string::npos ? loginStr : "@" + loginStr );
                            ImU32 loginCol = loginStr.empty() ? ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.50f, 0.7f))
                                                              : ImGui::GetColorU32(ImVec4(0.0f, 0.83f, 1.0f, 0.85f));
                            cellDl->AddText(ImVec2(framePos.x + 10.0f, framePos.y + 24.0f), loginCol, loginDisplay.c_str());
                        }
                        
                        char pingBuf[32];
                        snprintf(pingBuf, sizeof(pingBuf), "%d ms", (int)(pni->ping * 1000.0f));
                        ImVec2 pSz = ImGui::CalcTextSize(pingBuf);
                        cellDl->AddText(ImVec2(framePos.x + colW - 75.0f - pSz.x, framePos.y + 14.0f), 
                                        pni->ping < 0.1f ? IM_COL32(100, 255, 100, 255) : (pni->ping < 0.25f ? IM_COL32(255, 200, 100, 255) : IM_COL32(255, 100, 100, 255)), 
                                        pingBuf);
                                        
                        ImGui::EndChildFrame();
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();
                        ImGui::Spacing();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        } else {
            const char* rawPlayerName = "Local Player";
            ePlayer* lp = ePlayer::PlayerConfig(0);
            if (lp) rawPlayerName = static_cast<const char*>(lp->Name());
            
            const char* connState = "Singleplayer Mode";
            
            ImVec2 avatarPos(panel1Pos.x + 25.0f, panel1Pos.y + 60.0f + offsetDashY);
            if (g_AvatarTexture == 0) LoadAvatarTexture();
            float avatarSize = 50.0f;
            if (g_AvatarTexture != 0) {
                dl->AddImageRounded((ImTextureID)(intptr_t)g_AvatarTexture, avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,255), 8.0f);
                dl->AddRect(avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.22f, 0.8f)), 8.0f, 0, 1.0f);
            } else {
                dl->AddRectFilled(avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 1.0f)), 8.0f);
                dl->AddText(ImVec2(avatarPos.x + avatarSize*0.5f - 5.0f, avatarPos.y + avatarSize*0.5f - 8.0f + offsetDashY), IM_COL32(255,255,255,255), "R");
            }
            
            RenderColoredText(dl, ImVec2(avatarPos.x + avatarSize + 15.0f, avatarPos.y + 6.0f), IM_COL32(255,255,255,255), rawPlayerName);
            dl->AddText(ImVec2(avatarPos.x + avatarSize + 15.0f, avatarPos.y + 26.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)), connState);
            
            float infoY = panel1Pos.y + 130.0f + offsetDashY;
            auto drawInfoRow = [&](const char* label, const char* val, ImU32 valCol = IM_COL32(255, 255, 255, 255)) {
                dl->AddText(ImVec2(panel1Pos.x + 25.0f, infoY), ImGui::GetColorU32(ImVec4(0.55f, 0.55f, 0.60f, 1.0f)), label);
                dl->AddText(ImVec2(panel1Pos.x + colW - 25.0f - ImGui::CalcTextSize(val).x, infoY), valCol, val);
                dl->AddLine(ImVec2(panel1Pos.x + 20.0f, infoY + 28.0f), ImVec2(panel1Pos.x + colW - 20.0f, infoY + 28.0f), rc_Theme().line);
                infoY += 40.0f;
            };
            
            char fpsBuf[32];
            snprintf(fpsBuf, sizeof(fpsBuf), "%.1f FPS", io.Framerate);
            
            drawInfoRow("System State:", connState);
            // A rate that is fine is not news and should not be coloured like
            // an alarm; it is only worth marking when it is not.
            drawInfoRow("Performance Rate:", fpsBuf,
                        io.Framerate > 55.0f ? rc_Theme().text : rc_Theme().bad);
            
            std::string glVendor = (const char*)glGetString(GL_VENDOR);
            std::string glRenderer = (const char*)glGetString(GL_RENDERER);
            if (glVendor.length() > 60) glVendor = glVendor.substr(0, 60) + "...";
            if (glRenderer.length() > 60) glRenderer = glRenderer.substr(0, 60) + "...";
            
            drawInfoRow("GPU Vendor:", glVendor.c_str());
            drawInfoRow("GPU Renderer:", glRenderer.c_str());
            
            const char* midLabels[2] = { "Refresh Settings", "Toggle FPS Overlay" };
            for (int i = 0; i < 2; i++) {
                float itemY = infoY + 15.0f + i * 55.0f;
                float itemH = 45.0f;
                ImVec2 itemMin(panel1Pos.x + 20.0f, itemY);
                ImVec2 itemMax(panel1Pos.x + colW - 20.0f, itemY + itemH);
                
                bool hovered = io.MousePos.x >= itemMin.x && io.MousePos.x <= itemMax.x &&
                               io.MousePos.y >= itemMin.y && io.MousePos.y <= itemMax.y;
                bool selected = (g_DashboardActiveCol == 1) && (g_DashboardMiddleSelected == i);
                
                if (hovered && io.MouseClicked[0]) {
                    g_DashboardMiddleSelected = i;
                    g_DashboardActiveCol = 1;
                    g_DashboardActionTriggered = true;
                }
                
                ImU32 itemBg = (selected || hovered) ? GetThemeColor(0.3f * i + 0.2f) : ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, 0.6f));
                ImU32 itemBorder = (selected) ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.5f));
                
                dl->AddRectFilled(itemMin, itemMax, itemBg, 8.0f);
                dl->AddRect(itemMin, itemMax, itemBorder, 8.0f, 0, selected ? 1.5f : 1.0f);
                
                if (selected) {
                    dl->AddCircleFilled(ImVec2(itemMin.x + 15.0f, itemMin.y + itemH * 0.5f), 4.0f, GetThemeColor(0.5f));
                }
                
                dl->AddText(ImVec2(itemMin.x + (selected ? 30.0f : 20.0f), itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                            IM_COL32(255, 255, 255, 255), midLabels[i]);
                
                if (i == 1) {
                    const char* statusStr = sr_FPSOut ? "ON" : "OFF";
                    ImVec2 sSize = ImGui::CalcTextSize(statusStr);
                    dl->AddText(ImVec2(itemMax.x - 20.0f - sSize.x, itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                                sr_FPSOut ? rc_Theme().primary.bright : rc_Theme().textMute, statusStr);
                }
                
                if (g_DashboardActionTriggered && selected) {
                    g_DashboardActionTriggered = false;
                    if (i == 0) {
                        LoadDashboardConfigs();
                    } else if (i == 1) {
                        sr_FPSOut = !sr_FPSOut;
                    }
                }
            }

            // The band between the buttons and the player at the foot of the
            // panel. The two buttons above are laid out from infoY without
            // advancing it, so anything measured from there lands on top of
            // them - which is exactly what happened twice.
            {
                float below = infoY + 15.0f + 2.0f * 55.0f + 30.0f;
                float floorY = panel1Pos.y + panel1Size.y - 170.0f;  // the player keeps the foot
                float room = floorY - below;

                // Nothing at all rather than something cramped: a block with
                // its last row clipped looks worse than an empty panel.
                if ( room > 230.0f )
                {
                    if ( room > 310.0f )
                        room = 310.0f;

                    DrawStatsBlock(dl, ImVec2(panel1Pos.x + 22.0f, below),
                                   ImVec2(colW - 44.0f, room));
                }
            }

        }
    }
    
    if (hasPanel2 || s_DashSubTab == 1) {
        ImVec2 targetPos = hasPanel2 ? panel2Pos : panel1Pos;
        ImVec2 targetSize = hasPanel2 ? panel2Size : panel1Size;
        
        drawPanel(dl, targetPos, targetSize, g_DashboardActiveCol == 2, "CONFIG PROFILES");
        
        int numConfigs = (int)s_DashboardConfigs.size();
        if (numConfigs > 0) {
            if (g_DashboardRightSelected < 0) g_DashboardRightSelected = numConfigs;
            if (g_DashboardRightSelected > numConfigs) g_DashboardRightSelected = 0;
        } else {
            g_DashboardRightSelected = 0;
        }
        
        float listStartY = targetPos.y + 65.0f + offsetDashY;
        float visibleHeight = colH - 145.0f - offsetDashY;
        float itemHeight = 55.0f;
        
        if (numConfigs > 0 && g_DashboardRightSelected < numConfigs) {
            float selectedY = g_DashboardRightSelected * itemHeight;
            if (selectedY < s_DashTargetScrollY) s_DashTargetScrollY = selectedY;
            else if (selectedY + itemHeight > s_DashTargetScrollY + visibleHeight) s_DashTargetScrollY = selectedY + itemHeight - visibleHeight;
        }
        s_DashCurrentScrollY += (s_DashTargetScrollY - s_DashCurrentScrollY) * 0.2f;
        
        float totalHeightOfItems = numConfigs * itemHeight;
        if (totalHeightOfItems > visibleHeight) {
            float scrollbarX = targetPos.x + targetSize.x - 12.0f;
            float scrollbarY = listStartY;
            float scrollbarH = visibleHeight;
            dl->AddRectFilled(ImVec2(scrollbarX, scrollbarY), ImVec2(scrollbarX + 4.0f, scrollbarY + scrollbarH), ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.5f)), 2.0f);
            
            float thumbH = std::max(20.0f, (visibleHeight / totalHeightOfItems) * scrollbarH);
            if (thumbH > scrollbarH) thumbH = scrollbarH;
            float thumbY = scrollbarY + (s_DashCurrentScrollY / totalHeightOfItems) * scrollbarH;
            dl->AddRectFilled(ImVec2(scrollbarX, thumbY), ImVec2(scrollbarX + 4.0f, thumbY + thumbH), GetThemeColor(0.5f), 2.0f);
        }
        
        dl->PushClipRect(ImVec2(targetPos.x, listStartY), ImVec2(targetPos.x + targetSize.x - 15.0f, listStartY + visibleHeight), true);
        
        if (numConfigs == 0) {
            dl->AddText(ImVec2(targetPos.x + 25.0f, listStartY + 20.0f), ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.45f, 0.8f)), "No custom profiles found.");
        } else {
            for (int i = 0; i < numConfigs; i++) {
                float itemY = listStartY + i * itemHeight - s_DashCurrentScrollY;
                ImVec2 itemMin(targetPos.x + 15.0f, itemY);
                ImVec2 itemMax(targetPos.x + targetSize.x - 20.0f, itemY + itemHeight - 8.0f);
                
                bool hovered = io.MousePos.x >= itemMin.x && io.MousePos.x <= itemMax.x &&
                               io.MousePos.y >= itemMin.y && io.MousePos.y <= itemMax.y &&
                               io.MousePos.y >= listStartY && io.MousePos.y <= listStartY + visibleHeight;
                bool selected = (g_DashboardActiveCol == 2) && (g_DashboardRightSelected == i);
                
                if (hovered && io.MouseClicked[0]) {
                    g_DashboardRightSelected = i;
                    g_DashboardActiveCol = 2;
                    g_DashboardActionTriggered = true;
                }
                
                ImU32 itemBg = (selected || hovered) ? GetThemeColor(0.4f) : ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, 0.6f));
                ImU32 itemBorder = (selected) ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.5f));
                
                dl->AddRectFilled(itemMin, itemMax, itemBg, 6.0f);
                dl->AddRect(itemMin, itemMax, itemBorder, 6.0f, 0, selected ? 1.5f : 1.0f);
                
                if (selected) {
                    dl->AddCircleFilled(ImVec2(itemMin.x + 12.0f, itemMin.y + (itemHeight - 8.0f) * 0.5f), 3.0f, GetThemeColor(0.5f));
                }
                
                std::string dispName = s_DashboardConfigs[i];
                if (dispName.length() > 4 && dispName.substr(dispName.length() - 4) == ".cfg") {
                    dispName = dispName.substr(0, dispName.length() - 4);
                }
                
                dl->AddText(ImVec2(itemMin.x + (selected ? 22.0f : 15.0f), itemMin.y + ((itemHeight - 8.0f) - ImGui::GetTextLineHeight()) * 0.5f),
                            IM_COL32(255, 255, 255, 255), dispName.c_str());
                
                if (g_DashboardActionTriggered && selected) {
                    g_DashboardActionTriggered = false;
                    strncpy(g_SelectedConfig, s_DashboardConfigs[i].c_str(), sizeof(g_SelectedConfig) - 1);
                    g_SelectedConfig[sizeof(g_SelectedConfig) - 1] = '\0';
                    g_OpenApplyConfigPopup = true;
                }
            }
        }
        
        dl->PopClipRect();
        
        float createY = targetPos.y + targetSize.y - 60.0f;
        float createH = 45.0f;
        ImVec2 createMin(targetPos.x + 20.0f, createY);
        ImVec2 createMax(targetPos.x + targetSize.x - 20.0f, createY + createH);
        
        bool createHovered = io.MousePos.x >= createMin.x && io.MousePos.x <= createMax.x &&
                             io.MousePos.y >= createMin.y && io.MousePos.y <= createMax.y;
        bool createSelected = (g_DashboardActiveCol == 2) && (g_DashboardRightSelected == numConfigs);
        
        if (createHovered && io.MouseClicked[0]) {
            g_DashboardRightSelected = numConfigs;
            g_DashboardActiveCol = 2;
            g_DashboardActionTriggered = true;
        }
        
        ImU32 createBg = (createSelected || createHovered) ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.08f, 0.25f, 0.12f, 0.4f));
        ImU32 createBorder = createSelected ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.12f, 0.40f, 0.16f, 0.5f));
        
        dl->AddRectFilled(createMin, createMax, createBg, 8.0f);
        dl->AddRect(createMin, createMax, createBorder, 8.0f, 0, createSelected ? 1.5f : 1.0f);
        
        if (createSelected) {
            dl->AddCircleFilled(ImVec2(createMin.x + 15.0f, createMin.y + createH * 0.5f), 4.0f, GetThemeColor(0.5f));
        }
        
        dl->AddText(ImVec2(createMin.x + (createSelected ? 30.0f : 20.0f), createMin.y + (createH - ImGui::GetTextLineHeight()) * 0.5f),
                    IM_COL32(255, 255, 255, 255), "+ Create New Config");
        
        if (g_DashboardActionTriggered && createSelected) {
            g_DashboardActionTriggered = false;
            g_NewConfigName[0] = '\0';
            g_OpenCreateConfigPopup = true;
        }
    }
}

static void RenderEsportsDashboard(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH)
{
    static bool s_RCLAutoLoginAttempted = false;

    if (!s_RCLAutoLoginAttempted) {
        s_RCLAutoLoginAttempted = true;
        std::string loadedUser;
        if (RCL_AttemptAutoLogin(loadedUser)) {
            strncpy(s_RclUsername, loadedUser.c_str(), sizeof(s_RclUsername) - 1);
            RCL_FetchPlayerElosAsync(loadedUser);
        }
    }

    float dashStartX = panel1Pos.x;
    float dashWidth = panel1Size.x;
    if (hasPanel2) {
        dashWidth = (panel2Pos.x + panel2Size.x) - panel1Pos.x;
    }
    float dashHeight = panel1Size.y;
    ImVec2 dashPos(dashStartX, panel1Pos.y);
    ImVec2 dashSize(dashWidth, dashHeight);

    float leftColW = dashSize.x * 0.25f;
    float rightColW = dashSize.x * 0.75f - 20.0f;

    ImVec2 profilePos = dashPos;
    ImVec2 profileSize = ImVec2(leftColW, dashSize.y);

    ImVec2 queuePos = ImVec2(dashPos.x + leftColW + 20.0f, dashPos.y);
    ImVec2 queueSize = ImVec2(rightColW, dashSize.y);

    drawPanel(dl, profilePos, profileSize, g_DashboardActiveCol == 1, "RCL PROFILE");
    drawPanel(dl, queuePos, queueSize, g_DashboardActiveCol == 2, "FACEIT QUEUES");

    RCLAuthState authState = RCL_GetAuthState();
    bool inProgress = (authState == RCLAuthState::Pending);

    ImGui::SetCursorScreenPos(ImVec2(profilePos.x + 20.0f, profilePos.y + 65.0f));
    ImGui::BeginGroup();
    ImGui::PushItemWidth(profileSize.x - 40.0f);

    if (authState != RCLAuthState::Success) {
        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "RCL AUTH");
        if (g_FontHeader) ImGui::PopFont();

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "USERNAME:");
        ImGui::InputText("##rcl_user", s_RclUsername, sizeof(s_RclUsername));

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "PASSWORD:");
        ImGui::InputText("##rcl_pass", rcl_password, sizeof(rcl_password), ImGuiInputTextFlags_Password);

        ImGui::Spacing();
        ImGui::Spacing();

        if (inProgress) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("LOGIN TO RCL", ImVec2(profileSize.x - 40.0f, 35.0f))) {
            RCL_Login(s_RclUsername, rcl_password);
        }
        if (inProgress) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        if (authState == RCLAuthState::Pending) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Logging in...");
        } else if (authState == RCLAuthState::Failed) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Login Failed:\n%s", RCL_GetAuthError().c_str());
        }
    } else {
        if (g_FontHeader) ImGui::PushFont(g_FontHeader);
        ImGui::TextColored(ImVec4(0.1f, 0.9f, 0.2f, 1.0f), "%s", s_RclUsername);
        if (g_FontHeader) ImGui::PopFont();
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "RCL Member");
        
        ImGui::Spacing();
        {
            ImVec2 curPos = ImGui::GetCursorScreenPos();
            dl->AddLine(ImVec2(curPos.x, curPos.y), ImVec2(curPos.x + profileSize.x - 40.0f, curPos.y), ImGui::GetColorU32(ImGuiCol_Separator));
            ImGui::Dummy(ImVec2(0.0f, 1.0f));
        }
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "PLAYER RATINGS (ELO):");
        ImGui::Spacing();
        
        auto drawEloRow = [leftColW](const char* modeName, int val) {
            ImGui::Text("%s:", modeName);
            ImGui::SameLine(leftColW - 60.0f);
            if (!RCL_ElosLoaded()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "[ Pending ]");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%d", val);
            }
        };

        drawEloRow("TST", RCL_GetPlayerEloTst());
        drawEloRow("SUMOBAR", RCL_GetPlayerEloSumobar());
        drawEloRow("2S", RCL_GetPlayerElo2s());

        ImGui::Spacing();
        {
            ImVec2 curPos = ImGui::GetCursorScreenPos();
            dl->AddLine(ImVec2(curPos.x, curPos.y), ImVec2(curPos.x + profileSize.x - 40.0f, curPos.y), ImGui::GetColorU32(ImGuiCol_Separator));
            ImGui::Dummy(ImVec2(0.0f, 1.0f));
        }
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("SIGN OUT", ImVec2(profileSize.x - 40.0f, 35.0f))) {
            RCL_Logout();
            s_RclUsername[0] = '\0';
            rcl_password[0] = '\0';
        }
        ImGui::PopStyleColor(3);
    }

    ImGui::Spacing();
    {
        ImVec2 cp = ImGui::GetCursorScreenPos();
        dl->AddLine(ImVec2(cp.x, cp.y), ImVec2(cp.x + profileSize.x - 40.0f, cp.y), ImGui::GetColorU32(ImGuiCol_Separator));
        ImGui::Dummy(ImVec2(0.0f, 1.0f));
    }
    ImGui::Spacing();

    {
        int oauthState = RCL_SupabaseOAuthState();
        bool linked = RCL_SupabaseIsLinked();

        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "QUEUE ACCESS:");
        ImGui::Spacing();

        if (linked && oauthState != 1) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4( rc_Theme().good ), "Linked - ready to join queues.");
            if (ImGui::SmallButton("Unlink##rclsite")) {
                RCL_SupabaseLogout();
            }
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.8f, 1.0f), "Link your league account to join queues:");
            ImGui::Spacing();

            bool busy = (oauthState == 1);
            if (busy) ImGui::BeginDisabled();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.39f, 0.83f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.44f, 0.49f, 0.94f, 1.0f));
            if (ImGui::Button("Continue with Discord", ImVec2(profileSize.x - 40.0f, 32.0f))) {
                RCL_SupabaseOAuthBegin("discord");
            }
            ImGui::PopStyleColor(2);

            ImGui::Spacing();
            if (ImGui::Button("Continue with Google", ImVec2(profileSize.x - 40.0f, 32.0f))) {
                RCL_SupabaseOAuthBegin("google");
            }

            if (busy) ImGui::EndDisabled();
        }

        std::string oauthMsg = RCL_SupabaseOAuthMessage();
        if (!oauthMsg.empty() && oauthState != 0) {
            ImGui::Spacing();
            ImVec4 col = (oauthState == 2) ? ImGui::ColorConvertU32ToFloat4( rc_Theme().good )
                       : (oauthState == 3) ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
                                           : ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
            ImGui::PushTextWrapPos(profilePos.x + profileSize.x - 20.0f);
            ImGui::TextColored(col, "%s", oauthMsg.c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::Spacing();
        if (ImGui::TreeNode("Paste session manually")) {
            static char s_sessBuf[8192] = "";
            static std::string s_importMsg;
            static int s_importOk = 0;
            ImGui::PushTextWrapPos(profilePos.x + profileSize.x - 20.0f);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.8f, 1.0f),
                "On the RCL website (logged in), click the RCL bookmark, then paste here:");
            ImGui::PopTextWrapPos();
            ImGui::InputTextMultiline("##rclsess", s_sessBuf, sizeof(s_sessBuf),
                ImVec2(profileSize.x - 40.0f, 55.0f));
            if (ImGui::Button("Import", ImVec2(profileSize.x - 40.0f, 30.0f))) {
                std::string err;
                if (RCL_SupabaseImportSession(s_sessBuf, err)) {
                    s_sessBuf[0] = '\0';
                    s_importMsg = "Linked - ready to join queues.";
                    s_importOk = 1;
                } else {
                    s_importMsg = err;
                    s_importOk = 2;
                }
            }
            if (!s_importMsg.empty()) {
                ImGui::PushTextWrapPos(profilePos.x + profileSize.x - 20.0f);
                ImGui::TextColored(s_importOk == 1 ? ImGui::ColorConvertU32ToFloat4( rc_Theme().good )
                                                   : ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                    "%s", s_importMsg.c_str());
                ImGui::PopTextWrapPos();
            }
            ImGui::TreePop();
        }
    }

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    ImGui::SetCursorScreenPos(ImVec2(queuePos.x + 20.0f, queuePos.y + 65.0f));
    if (authState != RCLAuthState::Success) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Please login to view and join competitive queues.");
    } else {
        ImGui::BeginChild("##QueuesScroll", ImVec2(queueSize.x - 40.0f, queueSize.y - 85.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        
        struct QueueInfo {
            std::string id;
            std::string mode;
            std::string lane;
            int maxPlayers;
            int eloRequirement;
            int currentPlayers;
        };

        static std::vector<QueueInfo> s_Queues = {
            { "fort", "FORT", "", 12, 0, 3 },
            { "sumobar_open", "SUMOBAR", "OPEN", 8, 0, 2 },
            { "sumobar_beginner", "SUMOBAR", "BEGINNER", 8, -1900, 1 },
            { "sumobar_pro", "SUMOBAR", "PRO", 8, 1900, 0 },
            { "tst_open", "TST", "OPEN", 8, 0, 4 },
            { "tst_beginner", "TST", "BEGINNER", 8, -1900, 2 },
            { "tst_pro", "TST", "PRO", 8, 1900, 0 },
            { "two_s_open", "2S", "OPEN", 4, 0, 0 },
            { "two_s_beginner", "2S", "BEGINNER", 4, -1900, 0 },
            { "two_s_pro", "2S", "PRO", 4, 1900, 0 }
        };

        auto isEligible = [](const QueueInfo& q) -> bool {
            if (!RCL_ElosLoaded()) {
                return (q.eloRequirement == 0);
            }
            int playerElo = 0;
            if (q.mode == "TST") playerElo = RCL_GetPlayerEloTst();
            else if (q.mode == "SUMOBAR") playerElo = RCL_GetPlayerEloSumobar();
            else if (q.mode == "2S") playerElo = RCL_GetPlayerElo2s();
            else return true;
            
            if (q.eloRequirement == 1900) {
                return playerElo >= 1900;
            } else if (q.eloRequirement == -1900) {
                return playerElo < 1900;
            }
            return true;
        };

        static std::string s_QueueStatusMsg = "";
        static ImVec4 s_QueueStatusCol = ImVec4(1, 1, 1, 1);

        float cardWidth = (queueSize.x - 80.0f) / 2.0f;
        if (cardWidth < 200.0f) cardWidth = queueSize.x - 60.0f;

        // Three groups rather than one list of ten.
        //
        // The lanes are not a menu of equals: which of the three a player can
        // even enter is decided for them by their rating, so a page that mixes
        // them is a page where most of what you see is not for you. Grouped,
        // the one that is yours is a block you can look straight at.
        struct Tier { char const * lane; char const * title; char const * note; ImVec4 colour; };
        static Tier const s_Tiers[3] = {
            { "BEGINNER", "BEGINNER", "for ratings under 1900",  ImVec4( 0.40f, 0.82f, 0.55f, 1.0f ) },
            { "OPEN",     "OPEN",     "anyone may enter",        ImVec4( 0.35f, 0.72f, 1.00f, 1.0f ) },
            { "PRO",      "PRO",      "1900 and above",          ImVec4( 0.95f, 0.62f, 0.28f, 1.0f ) }
        };

        int drawnInTier = 0;

        for (size_t tier = 0; tier < 3; tier++) {
        Tier const & thisTier = s_Tiers[tier];

        // the heading, with a rule that runs out to the edge
        {
            if (tier > 0) { ImGui::Dummy(ImVec2(0.0f, 14.0f)); }
            ImVec2 hp = ImGui::GetCursorScreenPos();
            ImDrawList * hdl = ImGui::GetWindowDrawList();

            ImU32 const col = ImGui::GetColorU32(thisTier.colour);
            hdl->AddRectFilled(ImVec2(hp.x, hp.y + 3.0f), ImVec2(hp.x + 3.0f, hp.y + 19.0f), col, 1.5f);
            hdl->AddText(ImVec2(hp.x + 12.0f, hp.y + 2.0f), col, thisTier.title);

            float const titleW = ImGui::CalcTextSize(thisTier.title).x;
            hdl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 2.0f,
                         ImVec2(hp.x + 12.0f + titleW + 12.0f, hp.y + 4.0f),
                         ImGui::GetColorU32(ImVec4(0.45f, 0.47f, 0.54f, 1.0f)), thisTier.note);

            hdl->AddLine(ImVec2(hp.x, hp.y + 26.0f), ImVec2(hp.x + queueSize.x - 80.0f, hp.y + 26.0f),
                         ImGui::GetColorU32(ImVec4(0.20f, 0.21f, 0.25f, 1.0f)), 1.0f);

            ImGui::Dummy(ImVec2(0.0f, 34.0f));
            drawnInTier = 0;
        }

        for (size_t i = 0; i < s_Queues.size(); i++) {
            const auto& q = s_Queues[i];

            // Fort has no lane of its own; it belongs with the open ones.
            std::string const lane = q.lane.empty() ? std::string( "OPEN" ) : q.lane;
            if (lane != thisTier.lane)
                continue;

            ImGui::PushID(q.id.c_str());
            
            int currentPlayers = q.currentPlayers;
            int maxPlayers = q.maxPlayers;
            bool loaded = false;
            
            std::string serverKey = "";
            if (q.id == "fort") serverKey = "fort:open";
            else if (q.id == "sumobar_open") serverKey = "sumo:open";
            else if (q.id == "sumobar_beginner") serverKey = "sumo:beginner";
            else if (q.id == "sumobar_pro") serverKey = "sumo:pro";
            else if (q.id == "tst_open") serverKey = "tst:open";
            else if (q.id == "tst_beginner") serverKey = "tst:beginner";
            else if (q.id == "tst_pro") serverKey = "tst:pro";
            else if (q.id == "two_s_open") serverKey = "2s:open";
            else if (q.id == "two_s_beginner") serverKey = "2s:beginner";
            else if (q.id == "two_s_pro") serverKey = "2s:pro";

            if (!serverKey.empty()) {
                int cur = 0, req = 0;
                if (RCL_GetQueueCount(serverKey, cur, req)) {
                    currentPlayers = cur;
                    maxPlayers = req;
                    loaded = true;
                }
            }

            ImGui::BeginGroup();
            
            ImVec2 curPos = ImGui::GetCursorScreenPos();
            ImDrawList* cardDl = ImGui::GetWindowDrawList();
            ImVec2 cardSize(cardWidth, 120.0f);
            
            bool inThisQueue = false;
            for (const auto& jq : s_JoinedQueues) {
                if (jq == q.id) {
                    inThisQueue = true;
                    break;
                }
            }
            bool eligible = isEligible(q);

            ImU32 cardBg = inThisQueue ? ImGui::GetColorU32(ImVec4(0.12f, 0.18f, 0.12f, 0.85f)) 
                                       : ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.85f));
            ImU32 cardBorder = inThisQueue ? GetThemeColor(0.2f) : ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.5f));
            
            cardDl->AddRectFilled(curPos, ImVec2(curPos.x + cardSize.x, curPos.y + cardSize.y), cardBg, 8.0f);
            cardDl->AddRect(curPos, ImVec2(curPos.x + cardSize.x, curPos.y + cardSize.y), cardBorder, 8.0f, 0, 1.0f);
            
            if (inThisQueue) {
                cardDl->AddRect(curPos, ImVec2(curPos.x + cardSize.x, curPos.y + cardSize.y), GetThemeColor(0.5f), 8.0f, 0, 1.5f);
            }
            
            ImGui::SetCursorScreenPos(ImVec2(curPos.x + 15.0f, curPos.y + 12.0f));
            
            std::string titleStr = q.mode;
            if (!q.lane.empty()) titleStr += " (" + q.lane + ")";
            
            if (g_FontHeader) ImGui::PushFont(g_FontHeader);
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.95f, 1.0f), "%s", titleStr.c_str());
            if (g_FontHeader) ImGui::PopFont();
            
            ImGui::SetCursorScreenPos(ImVec2(curPos.x + 15.0f, curPos.y + 38.0f));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Capacity: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.85f, 1.0f), "%d / %d", currentPlayers, maxPlayers);
            
            float progress = (float)currentPlayers / (float)maxPlayers;
            if (progress > 1.0f) progress = 1.0f;
            ImGui::SetCursorScreenPos(ImVec2(curPos.x + 15.0f, curPos.y + 58.0f));
            
            ImVec2 barMin(curPos.x + 15.0f, curPos.y + 58.0f);
            ImVec2 barMax(curPos.x + cardSize.x - 15.0f, curPos.y + 64.0f);
            cardDl->AddRectFilled(barMin, barMax, ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.07f, 0.8f)), 3.0f);
            if (progress > 0.0f) {
                ImVec2 fillMax(barMin.x + (barMax.x - barMin.x) * progress, barMax.y);
                cardDl->AddRectFilled(barMin, fillMax, GetThemeColor(0.4f), 3.0f);
            }
            
            ImGui::SetCursorScreenPos(ImVec2(curPos.x + 15.0f, curPos.y + 72.0f));
            
            if (!eligible) {
                ImGui::BeginDisabled();
                ImGui::Button("NOT ELIGIBLE", ImVec2(cardSize.x - 30.0f, 32.0f));
                ImGui::EndDisabled();
            } else if (inThisQueue) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("LEAVE QUEUE", ImVec2(cardSize.x - 30.0f, 32.0f))) {
                    for (auto it = s_JoinedQueues.begin(); it != s_JoinedQueues.end(); ) {
                        if (*it == q.id) {
                            it = s_JoinedQueues.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    s_QueueStatusMsg = "Left queue: " + titleStr;
                    s_QueueStatusCol = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                    RCL_SendQueueAction(s_RclUsername, "leave", serverKey);
                }
                ImGui::PopStyleColor(2);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
                if (ImGui::Button("JOIN QUEUE", ImVec2(cardSize.x - 30.0f, 32.0f))) {
                    s_JoinedQueues.push_back(q.id);
                    s_QueueStatusMsg = "Joined queue: " + titleStr;
                    s_QueueStatusCol = ImVec4(0.1f, 0.9f, 0.2f, 1.0f);
                    RCL_SendQueueAction(s_RclUsername, "join", serverKey);
                }
                ImGui::PopStyleColor(2);
            }

            ImGui::EndGroup();
            ImGui::PopID();

            // two to a row, counted within the group rather than across the
            // whole list, or a group with an odd number pushes the next one
            // out of line
            ++drawnInTier;
            if (drawnInTier % 2 != 0) {
                ImGui::SameLine(0.0f, 20.0f);
            } else {
                ImGui::Spacing();
                ImGui::Spacing();
            }
        }

        if (drawnInTier % 2 != 0) {
            ImGui::NewLine();
            ImGui::Spacing();
        }
        }
        
        if (!s_QueueStatusMsg.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(s_QueueStatusCol, "%s", s_QueueStatusMsg.c_str());
        }

        ImGui::EndChild();
    }
}

#ifndef WORKER_TICKET_URL
#define WORKER_TICKET_URL RC_URL_TICKET
#endif

static void RenderSupportTicketsTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH)
{
    static bool s_RCLAutoLoginAttempted = false;
    if (!s_RCLAutoLoginAttempted) {
        s_RCLAutoLoginAttempted = true;
        std::string loadedUser;
        if (RCL_AttemptAutoLogin(loadedUser)) {
            strncpy(s_RclUsername, loadedUser.c_str(), sizeof(s_RclUsername) - 1);
            RCL_FetchPlayerElosAsync(loadedUser);
        }
    }

    float dashStartX = panel1Pos.x;
    float dashWidth = panel1Size.x;
    if (hasPanel2) {
        dashWidth = (panel2Pos.x + panel2Size.x) - panel1Pos.x;
    }
    float dashHeight = panel1Size.y;
    ImVec2 dashPos(dashStartX, panel1Pos.y);
    ImVec2 dashSize(dashWidth, dashHeight);

    // Draw main panel wrapper
    drawPanel(dl, dashPos, dashSize, true, "SUPPORT & BUG REPORT SYSTEM");

    RCLAuthState authState = RCL_GetAuthState();
    bool inProgress = (authState == RCLAuthState::Pending);

    ImGui::SetCursorScreenPos(ImVec2(dashPos.x + 30.0f, dashPos.y + 65.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (ImGui::BeginChild("##TicketTabScroll", ImVec2(dashSize.x - 60.0f, dashSize.y - 85.0f), false, ImGuiWindowFlags_NavFlattened)) {
        {
            // =========================================================================
            // STEP 2: AUTHENTICATED TICKET SUBMISSION & COOLDOWN VIEW
            // =========================================================================
            static char s_TicketTitleBuf[256] = "";
            static char s_TicketDescBuf[2048] = "";
            static std::string s_StatusMessage = "";

            // Who the ticket will come from. There is no account and nothing to
            // sign into: the name is the one over the bike, and the key beside
            // it is what the answer will be attached to.
            std::string sender = "player";
            if (se_PlayerNetIDs.Len() > 0 && se_PlayerNetIDs(0))
                sender = (char const *)se_PlayerNetIDs(0)->GetName();
            else if (ePlayer::PlayerConfig(0))
                sender = (char const *)ePlayer::PlayerConfig(0)->Name();

            ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.65f, 1.0f), "SENDING AS:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.4f, 1.0f), "%s", sender.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.6f, 1.0f), "(%s)", cIRCChat::GetIdentity().c_str());

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // How long until another ticket may go, and whether tickets are
            // closed for this key at all. Both answers come from the server;
            // the client only draws them. The file this used to be read from
            // was on the sender's own disk, which made the hour a suggestion.
            static double s_lastAsked = 0.0;
            if (tSysTimeFloat() - s_lastAsked > 30.0) {
                s_lastAsked = tSysTimeFloat();
                std::thread([]() {
                    std::string ignored;
                    rc_IloniumRefreshMe(ignored);
                }).detach();
            }

            IloniumSelf const & self = rc_IloniumMe();
            bool isCooldown = self.ticketWait > 0;
            int minutesLeft = self.ticketWait / 60;
            int secondsLeft = self.ticketWait % 60;
            bool isTicketBanned = self.tickBanned;

            if (isTicketBanned) {
                // Banned State Banner
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.35f, 0.05f, 0.08f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.9f, 0.2f, 0.2f, 0.8f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);

                if (ImGui::BeginChild("##TicketBannedBox", ImVec2(dashSize.x - 80.0f, 220.0f), true)) {
                    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4( rc_Theme().bad ), "ACCOUNT / DEVICE BANNED FROM SUPPORT SYSTEM");
                    if (g_FontHeader) ImGui::PopFont();
                    ImGui::Separator(); ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.8f, 1.0f),
                        "Your RCL account or Hardware ID (HWID) has been blocked from submitting support tickets.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.65f, 0.7f, 0.8f, 1.0f),
                        "Violation reason: Terms of service abuse or ticket system spamming.");
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f),
                        "Appeal options: Contact community staff on Discord (@ilonamoer).");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

                ImGui::Spacing(); ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.45f, 0.75f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.95f, 1.0f));
                if (ImGui::Button("REFRESH UNBAN STATUS", ImVec2(230.0f, 40.0f))) {
                    tString bPath = rc_InternalPath("ticket_banned.cfg");
                    std::remove((const char*)bPath);
                }
                ImGui::PopStyleColor(2);

            } else if (isCooldown) {
                // Cooldown Warning Box
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.22f, 0.14f, 0.04f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.9f, 0.5f, 0.1f, 0.8f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

                if (ImGui::BeginChild("##CooldownCard", ImVec2(dashSize.x - 80.0f, 180.0f), true)) {
                    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "TICKET SUBMISSION COOLDOWN ACTIVE");
                    if (g_FontHeader) ImGui::PopFont();

                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.9f, 0.85f, 0.75f, 0.9f),
                        "To prevent spam and server overload, ticket submissions are limited to 1 ticket per hour.");
                    ImGui::Spacing();

                    char cdBuf[128];
                    snprintf(cdBuf, sizeof(cdBuf), "Time Remaining: %d min %d sec", minutesLeft, secondsLeft);
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "%s", cdBuf);

                    ImGui::Spacing();
                    float progress = 1.0f - (float)self.ticketWait / 3600.0f;
                    ImGui::ProgressBar(progress, ImVec2(-1, 14.0f));
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

            } else {
                // Active Ticket Form
                if (g_FontHeader) ImGui::PushFont(g_FontHeader);
                ImGui::TextColored(ImVec4(0.95f, 0.95f, 1.0f, 1.0f), "CREATE NEW SUPPORT TICKET");
                if (g_FontHeader) ImGui::PopFont();

                ImGui::Spacing();

                float inputW = dashSize.x - 80.0f;
                ImGui::PushItemWidth(inputW);

                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "TICKET TITLE / SUMMARY:");
                ImGui::InputTextWithHint("##ticket_title_input", "Brief title e.g. Glitch in Sumo Arena wall", s_TicketTitleBuf, sizeof(s_TicketTitleBuf));

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "DESCRIPTION & REPRODUCTION STEPS:");
                ImGui::InputTextMultiline("##ticket_desc_input", s_TicketDescBuf, sizeof(s_TicketDescBuf), ImVec2(inputW, 140.0f), 0, nullptr);

                ImGui::PopItemWidth();

                ImGui::Spacing();
                // Detailed System Specs Gathering
                struct SystemSpecs {
                    std::string osDistro;
                    std::string cpuModel;
                    std::string ramInfo;
                    std::string romInfo;
                    std::string gpuModel;
                };

                auto getSpecs = []() -> SystemSpecs {
                    SystemSpecs specs;
                    specs.gpuModel = "Unknown GPU";
#ifndef DEDICATED
                    if (sr_glOut) {
                        const char* r = (const char*)glGetString(GL_RENDERER);
                        if (r) specs.gpuModel = r;
                    }
#endif
#ifdef WIN32
                    specs.osDistro = "Windows";
                    HKEY hKey;
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        char productName[256] = {0};
                        DWORD bufSize = sizeof(productName);
                        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &bufSize) == ERROR_SUCCESS) {
                            specs.osDistro = productName;
                        }
                        RegCloseKey(hKey);
                    }

                    specs.cpuModel = "Unknown CPU";
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        char cpuName[256] = {0};
                        DWORD bufSize = sizeof(cpuName);
                        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpuName, &bufSize) == ERROR_SUCCESS) {
                            specs.cpuModel = cpuName;
                        }
                        RegCloseKey(hKey);
                    }
                    SYSTEM_INFO sysInfo;
                    GetSystemInfo(&sysInfo);
                    if (sysInfo.dwNumberOfProcessors > 0) {
                        specs.cpuModel += " (" + std::to_string(sysInfo.dwNumberOfProcessors) + " cores)";
                    }

                    specs.ramInfo = "Unknown RAM";
                    MEMORYSTATUSEX memStatus;
                    memStatus.dwLength = sizeof(memStatus);
                    if (GlobalMemoryStatusEx(&memStatus)) {
                        double totalGb = static_cast<double>(memStatus.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
                        char buf[64];
                        snprintf(buf, sizeof(buf), "%.1f GB", totalGb);
                        specs.ramInfo = buf;
                    }

                    specs.romInfo = "Unknown Disk";
                    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
                    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                        double totalGb = static_cast<double>(totalNumberOfBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
                        double freeGb  = static_cast<double>(totalNumberOfFreeBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
                        char buf[128];
                        snprintf(buf, sizeof(buf), "%.1f GB Free / %.1f GB Total", freeGb, totalGb);
                        specs.romInfo = buf;
                    }
#else
#ifdef WIN32
                    specs.osDistro = "Windows";
                    HKEY hKey;
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        char productName[256] = {0};
                        DWORD bufSize = sizeof(productName);
                        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &bufSize) == ERROR_SUCCESS) {
                            specs.osDistro = productName;
                        }
                        RegCloseKey(hKey);
                    }

                    specs.cpuModel = "Unknown CPU";
                    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                        char cpuName[256] = {0};
                        DWORD bufSize = sizeof(cpuName);
                        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpuName, &bufSize) == ERROR_SUCCESS) {
                            specs.cpuModel = cpuName;
                        }
                        RegCloseKey(hKey);
                    }
                    SYSTEM_INFO sysInfo;
                    GetSystemInfo(&sysInfo);
                    if (sysInfo.dwNumberOfProcessors > 0) {
                        specs.cpuModel += " (" + std::to_string(sysInfo.dwNumberOfProcessors) + " cores)";
                    }

                    specs.ramInfo = "Unknown RAM";
                    MEMORYSTATUSEX memStatus;
                    memStatus.dwLength = sizeof(memStatus);
                    if (GlobalMemoryStatusEx(&memStatus)) {
                        double totalGb = static_cast<double>(memStatus.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
                        char buf[64];
                        snprintf(buf, sizeof(buf), "%.1f GB", totalGb);
                        specs.ramInfo = buf;
                    }

                    specs.romInfo = "Unknown Disk";
                    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
                    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                        double totalGb = static_cast<double>(totalNumberOfBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
                        double freeGb  = static_cast<double>(totalNumberOfFreeBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
                        char buf[128];
                        snprintf(buf, sizeof(buf), "%.1f GB Free / %.1f GB Total", freeGb, totalGb);
                        specs.romInfo = buf;
                    }
#else
                    specs.osDistro = "Linux";
                    std::ifstream osFile("/run/host/etc/os-release");
                    if (!osFile.is_open()) {
                        osFile.open("/etc/os-release");
                    }
                    if (osFile.is_open()) {
                        std::string line;
                        while (std::getline(osFile, line)) {
                            if (line.find("PRETTY_NAME=") == 0) {
                                size_t first = line.find('"');
                                size_t last  = line.rfind('"');
                                if (first != std::string::npos && last != std::string::npos && last > first) {
                                    specs.osDistro = line.substr(first + 1, last - first - 1);
                                } else if (line.size() > 12) {
                                    specs.osDistro = line.substr(12);
                                }
                                break;
                            }
                        }
                        osFile.close();
                    }

                    specs.cpuModel = "Unknown CPU";
                    std::ifstream cpuFile("/proc/cpuinfo");
                    if (cpuFile.is_open()) {
                        std::string line;
                        while (std::getline(cpuFile, line)) {
                            if (line.find("model name") == 0) {
                                size_t colon = line.find(':');
                                if (colon != std::string::npos) {
                                    specs.cpuModel = line.substr(colon + 1);
                                    while (!specs.cpuModel.empty() && specs.cpuModel[0] == ' ') {
                                        specs.cpuModel.erase(0, 1);
                                    }
                                }
                                break;
                            }
                        }
                        cpuFile.close();
                    }
                    int cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
                    if (cores > 0) {
                        specs.cpuModel += " (" + std::to_string(cores) + " cores)";
                    }

                    specs.ramInfo = "Unknown RAM";
                    std::ifstream memFile("/proc/meminfo");
                    if (memFile.is_open()) {
                        std::string line;
                        while (std::getline(memFile, line)) {
                            if (line.find("MemTotal:") == 0) {
                                std::stringstream ss(line);
                                std::string key;
                                long totalKb = 0;
                                ss >> key >> totalKb;
                                if (totalKb > 0) {
                                    double totalGb = static_cast<double>(totalKb) / (1024.0 * 1024.0);
                                    char buf[64];
                                    snprintf(buf, sizeof(buf), "%.1f GB", totalGb);
                                    specs.ramInfo = buf;
                                }
                                break;
                            }
                        }
                        memFile.close();
                    }

                    specs.romInfo = "Unknown Disk";
                    struct statvfs stat;
                    const char* targetPath = getenv("HOME");
                    if (!targetPath || statvfs(targetPath, &stat) != 0) {
                        targetPath = "/";
                        statvfs(targetPath, &stat);
                    }
                    if (stat.f_blocks > 0) {
                        double totalGb = static_cast<double>(stat.f_blocks * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                        double freeGb  = static_cast<double>(stat.f_bavail * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                        char buf[128];
                        snprintf(buf, sizeof(buf), "%.1f GB Free / %.1f GB Total", freeGb, totalGb);
                        specs.romInfo = buf;
                    }
#endif
#endif

                    return specs;
                };

                SystemSpecs specs = getSpecs();

                ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.55f, 0.8f),
                    "Specs: OS: %s | CPU: %s | GPU: %s | RAM: %s | Disk: %s",
                    specs.osDistro.c_str(), specs.cpuModel.c_str(), specs.gpuModel.c_str(), specs.ramInfo.c_str(), specs.romInfo.c_str());

                ImGui::Spacing(); ImGui::Spacing();

                // SEND TICKET Button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.55f, 0.25f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.75f, 0.35f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.06f, 0.38f, 0.15f, 1.00f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                if (ImGui::Button("SUBMIT SUPPORT TICKET", ImVec2(inputW, 46.0f))) {
                    if (strlen(s_TicketTitleBuf) > 0 && strlen(s_TicketDescBuf) > 0) {
                        std::string loginName = sender;

                        std::string titleText = s_TicketTitleBuf;
                        std::string descText  = s_TicketDescBuf;

                        auto jsonEscape = [](const std::string& input) -> std::string {
                            std::string out;
                            for (char c : input) {
                                if (c == '"') out += "\\\"";
                                else if (c == '\\') out += "\\\\";
                                else if (c == '\n') out += "\\n";
                                else if (c == '\r') out += "\\r";
                                else if (c == '\t') out += "\\t";
                                else out += c;
                            }
                            return out;
                        };

                        // The ticket goes to our own machine, signed with the
                        // key this installation made for itself. The hour
                        // between tickets is counted there, in the database,
                        // where nobody can delete a file to be rid of it.
                        std::string const person = loginName.empty() ? std::string("player") : loginName;
                        std::string const title = titleText;
                        std::string const text = descText;

                        IloniumMachine box;
                        box.os = specs.osDistro;
                        box.cpu = specs.cpuModel;
                        box.gpu = specs.gpuModel;
                        box.ram = specs.ramInfo;
                        box.disk = specs.romInfo;

                        std::thread([person, title, text, box]() {
                            std::string error;
                            if (!rc_IloniumTicket(person, title, text, box, error))
                                cIRCChat::SetStatusMessage("[System] " + error);
                        }).detach();


                        s_TicketTitleBuf[0] = '\0';
                        s_TicketDescBuf[0] = '\0';
                        s_StatusMessage = "Ticket submitted successfully! 1-hour cooldown initiated.";
                    } else {
                        s_StatusMessage = "Error: Please fill out both Title and Description fields!";
                    }
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);

                if (!s_StatusMessage.empty()) {
                    ImGui::Spacing();
                    bool isErr = (s_StatusMessage.find("Error") != std::string::npos);
                    ImGui::TextColored(isErr ? ImGui::ColorConvertU32ToFloat4( rc_Theme().bad ) : ImGui::ColorConvertU32ToFloat4( rc_Theme().good ),
                        "%s", s_StatusMessage.c_str());
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void ModMenu::SendPublicChat(const std::string& text) {
    ePlayerNetID* me = nullptr;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        if (se_PlayerNetIDs(i) && se_PlayerNetIDs(i)->Owner() == sn_myNetID) {
            me = se_PlayerNetIDs(i);
            break;
        }
    }
    if (me) {
        me->Chat(tString(text.c_str()));
    }
}

void ModMenu::HandleIncomingChat(ePlayerNetID* sender, const std::string& msg) {
    (void)sender;
    (void)msg;
}

//! Which tool the drawer is showing. Out here so the finder can open one
//! directly rather than only opening the drawer and leaving somebody to click
//! again - a shortcut that gets you most of the way is barely a shortcut.
static int & rc_MiscTool()
{
    static int which = 0;
    return which;
}

static void rc_MiscToolPick( int what )
{
    rc_MiscTool() = what;
}

//! Misc: the tools that are not settings and not the game.
static void RenderMiscTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH)
{
    drawPanel(dl, panel1Pos, panel1Size, g_DashboardActiveCol == 1, "MISC");

    if (hasPanel2)
        drawPanel(dl, panel2Pos, panel2Size, g_DashboardActiveCol == 2, "TOOLS");

    int & which = rc_MiscTool();

    ImVec2 listPos = hasPanel2 ? panel2Pos : panel1Pos;
    float listW = hasPanel2 ? panel2Size.x : 220.0f;

    char const * names[ 3 ] = { "Command Finder", "Gradient Name", "Camera" };

    for (int i = 0; i < 3; ++i)
    {
        ImGui::SetCursorScreenPos(ImVec2(listPos.x + 20.0f, listPos.y + 60.0f + i * 42.0f));

        bool here = ( which == i );
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(
            here ? rc_Theme().text : rc_Theme().textDim ));

        if (ImGui::Selectable(names[i], here, 0, ImVec2(listW - 40.0f, 30.0f)))
            which = i;

        ImGui::PopStyleColor();
    }

    ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 22.0f, panel1Pos.y + 62.0f));

    ImGui::PushID("rcMiscBody");
    if (ImGui::BeginChild("##rcMiscBody", ImVec2(panel1Size.x - 44.0f, panel1Size.y - 84.0f), false))
    {
        if (which == 0)
            rc_DrawCommandFinder(panel1Size.x - 60.0f);
        else if (which == 1)
            rc_DrawGradientMaker(panel1Size.x - 60.0f);
        else
            rc_DrawCameraTool(panel1Size.x - 60.0f);
    }
    ImGui::EndChild();
    ImGui::PopID();
}

static void RenderSocialsTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH) {
    drawPanel(dl, panel1Pos, panel1Size, true, "DEVELOPER & COMMUNITY HUB");

    float startX = panel1Pos.x + 20.0f;
    float startY = panel1Pos.y + 55.0f;
    float availW = panel1Size.x - 40.0f;
    float availH = colH - 75.0f;

    if (availW <= 10.0f || availH <= 10.0f) return;

    ImGui::SetCursorScreenPos(ImVec2(startX, startY));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    bool childVisible = ImGui::BeginChild("SocialsMainScroll", ImVec2(availW, availH), false, ImGuiWindowFlags_NavFlattened);
    if (childVisible) {
        
        // ----------------------------------------------------
        // HERO CARD: DEVELOPER SPOTLIGHT
        // ----------------------------------------------------
        ImVec2 heroPos = ImGui::GetCursorScreenPos();
        float heroH = 105.0f;
        
        // Card Background with smooth dark glassmorphism
        dl->AddRectFilled(heroPos, ImVec2(heroPos.x + availW, heroPos.y + heroH), IM_COL32(18, 22, 34, 230), 10.0f);
        dl->AddRect(heroPos, ImVec2(heroPos.x + availW, heroPos.y + heroH), IM_COL32(0, 210, 211, 160), 10.0f, 0, 1.5f);
        
        // Subtle top glow bar
        dl->AddRectFilled(heroPos, ImVec2(heroPos.x + availW, heroPos.y + 4.0f), IM_COL32(0, 210, 211, 255), 10.0f, ImDrawFlags_RoundCornersTop);

        // Dev Badge Avatar Circle
        float circleX = heroPos.x + 42.0f;
        float circleY = heroPos.y + heroH * 0.5f;
        dl->AddCircleFilled(ImVec2(circleX, circleY), 24.0f, IM_COL32(24, 30, 48, 255));
        dl->AddCircle(ImVec2(circleX, circleY), 24.0f, IM_COL32(0, 210, 211, 255), 24, 2.0f);
        
        // Text inside avatar
        ImVec2 devTxtSz = ImGui::CalcTextSize("DEV");
        ImGui::SetCursorScreenPos(ImVec2(circleX - devTxtSz.x * 0.5f, circleY - devTxtSz.y * 0.5f));
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "DEV");

        // Developer Information
        ImGui::SetCursorScreenPos(ImVec2(heroPos.x + 82.0f, heroPos.y + 14.0f));
        ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "ilona");

        ImGui::SetCursorScreenPos(ImVec2(heroPos.x + 82.0f, heroPos.y + 36.0f));
        ImGui::TextColored(ImVec4(0.85f, 0.88f, 0.95f, 1.0f), "Lead Developer");

        // Badges Helper - Dynamic Text Size & Clean Positioning
        auto DrawDevBadge = [&](const char* badgeText, ImVec4 txtCol, ImU32 bgCol, ImU32 borderCol) {
            ImVec2 pad(10.0f, 3.0f);
            ImVec2 txtSz = ImGui::CalcTextSize(badgeText);
            ImVec2 bMin = ImGui::GetCursorScreenPos();
            ImVec2 bMax(bMin.x + txtSz.x + pad.x * 2.0f, bMin.y + txtSz.y + pad.y * 2.0f);

            dl->AddRectFilled(bMin, bMax, bgCol, 4.0f);
            dl->AddRect(bMin, bMax, borderCol, 4.0f, 0, 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(bMin.x + pad.x, bMin.y + pad.y));
            ImGui::TextColored(txtCol, "%s", badgeText);

            ImGui::SetCursorScreenPos(ImVec2(bMax.x, bMin.y));
        };

        // Badges Row - Perfectly Spaced without Overlap
        ImGui::SetCursorScreenPos(ImVec2(heroPos.x + 82.0f, heroPos.y + 64.0f));
        DrawDevBadge("OWNER", ImVec4(1.0f, 0.28f, 0.34f, 1.0f), IM_COL32(255, 71, 87, 40), IM_COL32(255, 71, 87, 180));
        ImGui::SameLine(0, 10.0f);
        DrawDevBadge("ENGINE DEV", ImVec4(0.0f, 0.85f, 1.0f, 1.0f), IM_COL32(0, 210, 211, 40), IM_COL32(0, 210, 211, 180));
        ImGui::SameLine(0, 10.0f);
        DrawDevBadge("ONLINE", ImVec4(0.18f, 0.84f, 0.45f, 1.0f), IM_COL32(46, 213, 115, 40), IM_COL32(46, 213, 115, 180));

        ImGui::SetCursorScreenPos(ImVec2(startX, heroPos.y + heroH + 18.0f));

        // Section Title
        rc_Tracked( "COMMUNITY", 1.6f, rc_Theme().textMute );
        ImGui::Spacing(); ImGui::Spacing();

        // ----------------------------------------------------
        // SOCIAL CARDS LIST (Stacking Cards Layout)
        // ----------------------------------------------------
        struct SocialCard {
            const char* title;
            const char* handle;
            const char* desc;
            const char* url;
            const char* tagLabel;
            ImVec4 mainColor;
            ImVec4 btnColor;
            ImVec4 btnHoverColor;
        };

        SocialCard cards[] = {
            {
                "Creator Discord",
                "@ilonamoer",
                "Direct contact with lead developer ilonamoer for feedback, custom features, and support.",
                "https://discord.com/users/1022869080711110717",
                "[DISCORD]",
                ImVec4(0.7f, 0.45f, 0.95f, 1.0f),
                ImVec4(0.55f, 0.27f, 0.68f, 0.85f),
                ImVec4(0.68f, 0.35f, 0.82f, 1.0f)
            },
            {
                "YouTube Channel",
                "@ilonamoer",
                "Watch high-level gameplay highlights, mod updates, and development streams.",
                "https://www.youtube.com/@ilonamoer",
                "[YOUTUBE]",
                ImVec4(1.00f, 0.30f, 0.35f, 1.0f),
                ImVec4(0.85f, 0.20f, 0.25f, 0.85f),
                ImVec4(1.00f, 0.32f, 0.38f, 1.0f)
            },
            {
                "RetroCycles Discord",
                "discord.gg/retrocycles",
                "Official community server for multiplayer matches, updates, tournaments, and chat.",
                "https://discord.gg/retrocycles",
                "[COMMUNITY]",
                ImVec4(0.35f, 0.55f, 0.98f, 1.0f),
                ImVec4(0.25f, 0.40f, 0.88f, 0.85f),
                ImVec4(0.35f, 0.50f, 0.98f, 1.0f)
            },
            {
                "Armagetron Wiki",
                "wiki.armagetronad.org",
                "The reference for the game itself - settings, mechanics, server configuration and the history behind all of it.",
                "https://wiki.armagetronad.org/index.php?title=Main_Page",
                "[WIKI]",
                ImVec4(0.62f, 0.68f, 0.78f, 1.0f),
                ImVec4(0.42f, 0.47f, 0.56f, 0.85f),
                ImVec4(0.55f, 0.61f, 0.72f, 1.0f)
            },
            {
                "RetroCycles League (RCL)",
                "retrocyclesleague.com",
                "Competitive portal for player rankings, match history, and tournament registrations.",
                "https://retrocyclesleague.com",
                "[LEAGUE]",
                ImVec4(1.00f, 0.83f, 0.16f, 1.0f),
                ImVec4(0.85f, 0.68f, 0.10f, 0.85f),
                ImVec4(1.00f, 0.78f, 0.20f, 1.0f)
            },
            {
                "TronStats",
                "tronstats.gg",
                "Leaderboards, match history and player statistics for the whole scene - every turn is on the record. (insane work Nanu)",
                "https://tronstats.gg/",
                "[STATS]",
                ImVec4(0.24f, 0.76f, 1.00f, 1.0f),
                ImVec4(0.18f, 0.55f, 0.80f, 0.85f),
                ImVec4(0.26f, 0.68f, 0.95f, 1.0f)
            },
            {
                "ArmaNelgTron",
                "armanelgtron.tk",
                "Rankings, tools and resources kept by NelgTron - one of the longest running collections the scene has.",
                "https://www.armanelgtron.tk/",
                "[TOOLS]",
                ImVec4(0.45f, 0.85f, 0.62f, 1.0f),
                ImVec4(0.30f, 0.62f, 0.44f, 0.85f),
                ImVec4(0.40f, 0.78f, 0.56f, 1.0f)
            }
        };

        static std::string toastMsg = "";
        static double toastTime = 0.0;

        float cardW = availW;
        float cardH = 82.0f;

        // Counted rather than written out: the last card vanished the moment
        // a new one was added, and nothing said so.
        for (int i = 0; i < IM_ARRAYSIZE(cards); i++) {
            ImGui::PushID(i);
            
            ImVec2 cPos = ImGui::GetCursorScreenPos();

            // Background & Border
            rcTheme const & th = rc_Theme();
            ImVec2 cFar( cPos.x + cardW, cPos.y + cardH );

            // Every card used to bring its own palette - a purple one, a blue
            // one, a red one - and five brands shouting at once is what makes
            // a page look like a link farm. The service keeps its colour, but
            // only as the strip down its edge; the card itself is the same
            // surface as everything else on the screen.
            bool over = ImGui::IsMouseHoveringRect( cPos, cFar );
            float warm = rc_Ease( ImGui::GetID( cards[i].title ), over ? 1.0f : 0.0f );

            dl->AddRectFilled( cPos, cFar, rc_Mix( th.surface, th.surfaceHi, warm ), th.radiusMedium );
            dl->AddRect( cPos, cFar, rc_Mix( th.line, th.lineStrong, warm ), th.radiusMedium, 0, 1.0f );

            ImU32 stripCol = IM_COL32((int)(cards[i].mainColor.x * 255), (int)(cards[i].mainColor.y * 255), (int)(cards[i].mainColor.z * 255), 255);
            dl->AddRectFilled( cPos, ImVec2( cPos.x + 3.0f, cFar.y ),
                               rc_Fade( stripCol, 0.55f + 0.45f * warm ), 3.0f );

            // The name reads first, the service second and quietly. Before,
            // the tag was the loudest thing on the row and it is the least
            // interesting part of it.
            ImGui::SetCursorScreenPos(ImVec2(cPos.x + 20.0f, cPos.y + 12.0f));
            ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.text ) );
            ImGui::TextUnformatted( cards[i].title );
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textMute ) );
            ImGui::Text( "%s", cards[i].handle );
            ImGui::PopStyleColor();

            // Description
            ImGui::SetCursorScreenPos(ImVec2(cPos.x + 20.0f, cPos.y + 36.0f));
            ImGui::PushTextWrapPos(cPos.x + cardW - 220.0f);
            ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textDim ) );
            ImGui::TextUnformatted( cards[i].desc );
            ImGui::PopStyleColor();
            ImGui::PopTextWrapPos();

            // Right Action Buttons
            ImGui::SetCursorScreenPos(ImVec2(cPos.x + cardW - 220.0f, cPos.y + 24.0f));

            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4( rc_Theme().primary.wash ));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4( rc_Fade( rc_Theme().primary.solid, 0.28f ) ));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertU32ToFloat4( rc_Fade( rc_Theme().primary.solid, 0.36f ) ));
            
            if (ImGui::Button("OPEN", ImVec2(115.0f, 32.0f))) {
                OpenURL(cards[i].url);
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 8.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4( rc_Theme().surfaceHi ));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4( rc_Theme().raised ));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertU32ToFloat4( rc_Theme().raisedHi ));

            if (ImGui::Button("COPY", ImVec2(85.0f, 32.0f))) {
                ImGui::SetClipboardText(cards[i].url);
                toastMsg = std::string("Copied link: ") + cards[i].url;
                toastTime = tSysTimeFloat();
            }
            ImGui::PopStyleColor(3);

            ImGui::SetCursorScreenPos(ImVec2(startX, cPos.y + cardH + 12.0f));
            ImGui::PopID();
        }

        // Toast feedback notification
        if (!toastMsg.empty() && (tSysTimeFloat() - toastTime < 3.0)) {
            ImVec2 tPos = ImGui::GetCursorScreenPos();
            dl->AddRectFilled(tPos, ImVec2(tPos.x + availW, tPos.y + 32.0f), IM_COL32(46, 213, 115, 40), 6.0f);
            dl->AddRect(tPos, ImVec2(tPos.x + availW, tPos.y + 32.0f), IM_COL32(46, 213, 115, 200), 6.0f);
            ImGui::SetCursorScreenPos(ImVec2(tPos.x + 12.0f, tPos.y + 6.0f));
            ImGui::TextColored(ImVec4(0.18f, 0.84f, 0.45f, 1.0f), "[OK] %s", toastMsg.c_str());
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

static void RenderIRCChatTab(ImDrawList* dl, ImVec2 panel1Pos, ImVec2 panel1Size, ImVec2 panel2Pos, ImVec2 panel2Size, bool hasPanel2, float colW, float colH) {
    cIRCChat::Init();
    cIRCChat::UpdatePolling(true);

    static double s_askedSelf = 0.0;
    if (tSysTimeFloat() - s_askedSelf > 20.0) {
        s_askedSelf = tSysTimeFloat();
        std::thread([]() { std::string ignored; rc_IloniumRefreshMe(ignored); }).detach();
    }

    drawPanel(dl, panel1Pos, panel1Size, true, "GLOBAL IRC CHAT");

    float startX = panel1Pos.x + 20.0f;
    float startY = panel1Pos.y + 55.0f;
    float availW = panel1Size.x - 40.0f;
    float availH = colH - 75.0f;

    if (availW <= 10.0f || availH <= 10.0f) return;

    {
        // --- CHAT MESSAGES & INPUT UI (Modern Glassmorphism & Cyberpunk Design) ---
        ImGui::SetCursorScreenPos(ImVec2(startX, startY));
        
        // Online Green Status Dot
        dl->AddCircleFilled(ImVec2(startX + 6.0f, startY + 10.0f), 4.5f, IM_COL32(46, 213, 115, 255));
        dl->AddCircle(ImVec2(startX + 6.0f, startY + 10.0f), 7.0f, IM_COL32(46, 213, 115, 120), 12, 1.5f);

        ImGui::SetCursorScreenPos(ImVec2(startX + 18.0f, startY));

        // The name over the bike, the tag the server gave, and the first
        // characters of this installation's key - enough for a person to say
        // which one they are without there being an account to sign into.
        std::string speaking = "player";
        if (se_PlayerNetIDs.Len() > 0 && se_PlayerNetIDs(0))
            speaking = (char const *)se_PlayerNetIDs(0)->GetName();
        else if (ePlayer::PlayerConfig(0))
            speaking = (char const *)ePlayer::PlayerConfig(0)->Name();

        ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%s", speaking.c_str());

        std::string const tag = cIRCChat::GetRoleTag();
        if (!tag.empty()) {
            ImGui::SameLine();
            ImVec4 tagCol = ImVec4(0.31f, 0.85f, 1.0f, 1.0f);
            unsigned int r = 0, g = 0, b = 0;
            if (sscanf(cIRCChat::GetRoleColor().c_str(), "#%02x%02x%02x", &r, &g, &b) == 3)
                tagCol = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
            ImGui::TextColored(tagCol, "[%s]", tag.c_str());
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.6f, 1.0f), "(%s)", cIRCChat::GetIdentity().c_str());

        // What the server decided about this key, said plainly. Before this the
        // only sign of a mute was that nothing happened when you pressed send.
        IloniumSelf const & self = rc_IloniumMe();

        if (self.chatBanned) {
            ImGui::SetCursorScreenPos(ImVec2(startX, startY + 40.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.05f, 0.07f, 0.85f));
            ImGui::BeginChild("IRCBanned", ImVec2(availW, availH - 40.0f), true);
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.37f, 0.42f, 1.0f), "YOU ARE BANNED FROM THIS CHAT");
            ImGui::Spacing();
            if (!self.chatBanReason.empty())
                ImGui::TextWrapped("Reason: %s", self.chatBanReason.c_str());
            else
                ImGui::TextWrapped("No reason was given.");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.55f, 0.58f, 0.66f, 1.0f),
                               "Tickets still work. Your key: %s", cIRCChat::GetIdentity().c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            return;
        }

        if (self.muted) {
            int const left = (int)(self.muteUntil - (long long)time(NULL));
            int const mins = left > 0 ? (left + 59) / 60 : 0;
            ImGui::SetCursorScreenPos(ImVec2(startX, startY + 22.0f));
            ImGui::TextColored(ImVec4(1.0f, 0.71f, 0.33f, 1.0f),
                               "Muted for another %d min%s%s", mins,
                               self.muteReason.empty() ? "" : " - ",
                               self.muteReason.c_str());
        }

        // Messages Box
        float msgBoxH = availH - 85.0f;
        if (msgBoxH < 30.0f) msgBoxH = 30.0f;

        ImGui::SetCursorScreenPos(ImVec2(startX, startY + 28.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.85f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        
        bool msgsVis = ImGui::BeginChild("IRCMessagesScroll", ImVec2(availW, msgBoxH), true, ImGuiWindowFlags_NavFlattened);
        if (msgsVis) {
            std::vector<IRCMessage> msgs = cIRCChat::GetMessages();

            if (msgs.empty()) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
                ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.6f, 1.0f), "[CHAT] No messages yet in Global Chat. Be the first to start a discussion!");
            } else {
                for (const auto& m : msgs) {
                    // Time formatting [HH:MM]
                    char timeBuf[32];
                    time_t ts = (time_t)m.timestamp;
                    struct tm* tmInfo = std::localtime(&ts);
                    if (tmInfo) {
                        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", tmInfo);
                    } else {
                        strncpy(timeBuf, "--:--", sizeof(timeBuf));
                    }

                    // Render time badge
                    ImGui::TextColored(ImVec4(0.45f, 0.5f, 0.62f, 1.0f), "[%s]", timeBuf);
                    ImGui::SameLine();

                    // Render role tag badge if present
                    if (!m.role_tag.empty()) {
                        ImVec4 tagColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                        if (!m.role_color.empty()) {
                            std::string hexStr = m.role_color;
                            if (hexStr[0] == '#') hexStr = hexStr.substr(1);
                            if (hexStr.size() == 6) {
                                unsigned int r = 0, g = 0, b = 0;
                                if (sscanf(hexStr.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                                    tagColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
                                }
                            }
                        } else {
                            std::string upperTag = m.role_tag;
                            std::transform(upperTag.begin(), upperTag.end(), upperTag.begin(), ::toupper);
                            if (upperTag.find("OWNER") != std::string::npos || upperTag.find("ADMIN") != std::string::npos) {
                                tagColor = ImVec4(1.0f, 0.28f, 0.34f, 1.0f);
                            } else if (upperTag.find("MOD") != std::string::npos) {
                                tagColor = ImVec4(0.18f, 0.8f, 1.0f, 1.0f);
                            } else if (upperTag.find("VIP") != std::string::npos) {
                                tagColor = ImVec4(0.8f, 0.4f, 1.0f, 1.0f);
                            }
                        }

                        ImGui::TextColored(tagColor, "[%s]", m.role_tag.c_str());
                        ImGui::SameLine();
                    }

                    // One name, the one the person plays under, with the colour
                    // codes armagetron writes into it honoured as colours.

                    const std::string& ign = m.in_game_name;
                    size_t cIdx = 0;
                    ImVec4 curNameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    std::string seg = "";
                    while (cIdx < ign.size()) {
                        if (ign[cIdx] == '0' && cIdx + 1 < ign.size() && (ign[cIdx + 1] == 'x' || ign[cIdx + 1] == 'X')) {
                            if (!seg.empty()) {
                                ImGui::TextColored(curNameColor, "%s", seg.c_str());
                                ImGui::SameLine(0, 0);
                                seg.clear();
                            }
                            if (cIdx + 7 < ign.size()) {
                                std::string hexPart = ign.substr(cIdx + 2, 6);
                                unsigned int r = 0, g = 0, b = 0;
                                if (sscanf(hexPart.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                                    curNameColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
                                    cIdx += 8;
                                    continue;
                                }
                            }
                        }
                        seg += ign[cIdx];
                        cIdx++;
                    }
                    if (!seg.empty()) {
                        ImGui::TextColored(curNameColor, "%s", seg.c_str());
                        ImGui::SameLine(0, 0);
                    }

                    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), ": ");
                    ImGui::SameLine(0, 0);
                    ImGui::TextWrapped("%s", m.message.c_str());
                }
            }

            // Auto-scroll to bottom on new messages
            static size_t lastMsgCount = 0;
            if (msgs.size() != lastMsgCount) {
                lastMsgCount = msgs.size();
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        // Bottom Input Area & Cooldown Timer Logic
        static char msgInput[256] = "";
        double cdRemaining = cIRCChat::GetCooldownRemaining();
        bool isCoolingDown = (cdRemaining > 0.0);
        int cdSec = (int)std::ceil(cdRemaining);

        // The field had no frame of its own against a dark panel, so there was
        // nothing on screen saying where to type. It gets a plate, a border and
        // a hint now, and sits far enough off the bottom edge to be whole.
        float const inputY = startY + availH - 56.0f;
        float const inputW = availW - 121.0f;

        dl->AddRectFilled(ImVec2(startX, inputY - 4.0f), ImVec2(startX + inputW, inputY + 32.0f),
                          IM_COL32(16, 18, 23, 235), 8.0f);
        dl->AddRect(ImVec2(startX, inputY - 4.0f), ImVec2(startX + inputW, inputY + 32.0f),
                    IM_COL32(52, 58, 70, 220), 8.0f);

        // The field sits in the middle of its plate rather than near the top of
        // it, whatever the font size happens to be.
        float const plateTop = inputY - 4.0f;
        float const plateH = 36.0f;
        ImGui::SetCursorScreenPos(ImVec2(startX + 12.0f, plateTop + (plateH - ImGui::GetFrameHeight()) * 0.5f));
        ImGui::SetNextItemWidth(inputW - 24.0f);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
        bool enterPressed = ImGui::InputText("##ircMsgInput", msgInput, sizeof(msgInput), ImGuiInputTextFlags_EnterReturnsTrue);
        bool const typing = ImGui::IsItemActive();
        ImGui::PopStyleColor(3);

        // The invitation goes away the moment the field is yours, not only once
        // you have managed to type a character into it.
        if (!typing && msgInput[0] == '\0')
        {
            ImVec2 const at = ImGui::GetItemRectMin();
            dl->AddText(ImVec2(at.x + 2.0f, at.y + ImGui::GetStyle().FramePadding.y),
                        IM_COL32(110, 116, 128, 200), "write something and press enter");
        }
        
        ImGui::SetCursorScreenPos(ImVec2(startX + inputW + 8.0f, inputY - 4.0f));

        bool sendBtnClicked = false;
        if (isCoolingDown) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.2f, 0.25f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.2f, 0.25f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.2f, 0.25f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.75f, 0.7f));
            
            std::string waitLabel = "WAIT (" + std::to_string(cdSec) + "s)";
            ImGui::Button(waitLabel.c_str(), ImVec2(105.0f, 36.0f));
            
            ImGui::PopStyleColor(4);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.9f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.55f, 0.8f, 1.0f));
            
            if (ImGui::Button("SEND", ImVec2(105.0f, 36.0f))) {
                sendBtnClicked = true;
            }
            
            ImGui::PopStyleColor(3);
        }

        // Try Send Message if triggered and NOT cooling down
        if ((enterPressed || sendBtnClicked) && strlen(msgInput) > 0) {
            if (isCoolingDown) {
                // Cooldown active: DO NOT CLEAR msgInput! Show system message.
                cIRCChat::SetStatusMessage("[System] Wait " + std::to_string(cdSec) + "s before sending again. Your message is saved.");
            } else {
                std::string err;
                if (cIRCChat::SendChatMessage(msgInput, err)) {
                    msgInput[0] = '\0'; // Clear input ONLY on success!
                }
            }
        }

        // Real-time Cooldown & Status Feedback line
        ImGui::SetCursorScreenPos(ImVec2(startX, startY + availH - 14.0f));
        if (isCoolingDown) {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "[Cooldown] %ds remaining - your message is saved in input box", cdSec);
        } else {
            std::string statusMsg = cIRCChat::GetStatusMessage();
            if (!statusMsg.empty()) {
                ImVec4 statCol = ImVec4(0.4f, 0.85f, 1.0f, 1.0f);
                if (statusMsg.find("Wait") != std::string::npos ||
                    statusMsg.find("Invalid") != std::string::npos ||
                    statusMsg.find("banned") != std::string::npos) {
                    statCol = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
                }
                ImGui::TextColored(statCol, "%s", statusMsg.c_str());
            }
        }
    }
}
#endif

