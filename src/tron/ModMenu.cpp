#include "ModMenu.h"
#include "HudManager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl2.h"
#include "rSDL.h"
#include <sys/time.h>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "tron/DemoRecorder.h"
#include "tron/DemoPlayer.h"
#include "tron/MediaWidget.h"

#include "render/rSysdep.h"
#include "tron/gMenus.h"

extern REAL sg_modNameSizeScale;

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
#include "ui/uInput.h"
#include "ui/uInputQueue.h"
#include "ui/uMenu.h"
#include "tools/tToDo.h"
#include "tools/tSysTime.h"
#include "tron/gLanguageMenu.h"
#include "network/nServerInfo.h"
#include "render/rScreen.h"
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

static void se_TestLegacyMenu(std::istream& s) {
    ModMenu::g_PendingLegacyMenuAction = []() { sg_screenMenu.Enter(); };
}
static tConfItemFunc testLegacyMenuConf("TEST_LEGACY_MENU", &se_TestLegacyMenu);

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

extern REAL sn_anti360Window;
extern REAL sg_perfectTurnCalibration;
extern REAL sg_autoEscapeRubberMargin;
extern bool sg_autoEscapePingComp;

// Globals
bool g_NoclipMode = false;
bool g_CleanScreen = false;
bool g_CameraLock = false;
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
ImVec4 g_AccentColor1 = ImVec4(0.478f, 0.345f, 1.0f, 1.0f); // Default purple
ImVec4 g_AccentColor2 = ImVec4(0.0f, 0.75f, 1.0f, 1.0f);    // Default cyan
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
int g_NoclipKeybind = 0;      // 0 = NONE
int g_CleanScreenKeybind = 0;
int g_CameraLockKeybind = 0;
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

// Auto Packet Refresh configuration
bool g_AutoPacketRefresh = true;
int g_PacketRefreshKeybind = 0;

// K/D Reset keybind
int g_ResetKDKeybind = 0;
extern bool sg_modKDResetFlag;

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
static tConfItem<int> conf_cameraLockKeybind("MOD_CAMERA_LOCK_KEYBIND", g_CameraLockKeybind);
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
static tConfItem<bool> conf_modMenuKeybindEnabled("MOD_MENU_KEYBIND_ENABLED", g_ModMenuKeybindEnabled);
static tConfItem<int> conf_modMenuKeybind("MOD_MENU_KEYBIND", g_ModMenuKeybind);


// Mod modes & variables
static tConfItem<bool> conf_noclipMode("MOD_NOCLIP_MODE", g_NoclipMode);
static tConfItem<bool> conf_cleanScreen("MOD_CLEAN_SCREEN", g_CleanScreen);
static tConfItem<bool> conf_cameraLock("MOD_CAMERA_LOCK", g_CameraLock);
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
static bool prev_CameraLock = false;
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
    g_CameraLockKeybind = TranslateLegacyKeycodeToSDL3Keycode(g_CameraLockKeybind);
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
#include <unistd.h>
#include <map>

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
    if (g_LastDisplaySize.x != io.DisplaySize.x || g_LastDisplaySize.y != io.DisplaySize.y) {
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
static GLuint g_AvatarTexture = 0;

// Font pointers and main menu active flag
ImFont* g_FontDefault = nullptr;
ImFont* g_FontHeader = nullptr;
bool ModMenu::g_MainMenuActive = true;
bool ModMenu::g_CustomMainMenuTempDisabled = false;
bool ModMenu::g_InGameMenuOpen = false;
std::function<void()> ModMenu::g_PendingLegacyMenuAction = nullptr;

static nServerInfoBase* s_PendingConnectServer = nullptr;
static nServerInfoRedirect* s_DirectRedirectServer = nullptr;
static bool s_PendingStartLocalGame = false;
static bool s_PendingStartDemoPlayback = false;
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
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    
    style.WindowRounding = 14.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    
    // Colors
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.15f, 0.8f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.20f, 0.8f);
    
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.94f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.80f, 1.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.94f, 1.0f, 1.0f);
    
    style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.16f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.24f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.80f, 1.0f, 0.4f);
    
    style.Colors[ImGuiCol_Header] = ImVec4(0.12f, 0.12f, 0.16f, 0.6f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.24f, 0.8f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.80f, 1.0f, 0.4f);
    
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.94f, 1.0f, 0.8f);
    
    // Strict Figma Padding
    style.WindowPadding = ImVec2(25.0f, 25.0f);
    style.ItemSpacing = ImVec2(15.0f, 15.0f);
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
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), desc, nullptr, renderSize.x - 76.0f);
    
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
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 30.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), desc, nullptr, renderSize.x - 36.0f);
    
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
    } else if (strcmp(format, "AMBIENT_PARTICLES_MODE_FORMAT") == 0) {
        int mode = (int)*v;
        if (mode == 0) strcpy(valBuf, "Uniform (Everywhere)");
        else strcpy(valBuf, "Zone Only (Inside Sumo)");
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
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), desc, nullptr, renderSize.x - 76.0f);
    
    // Render Color Picker Button relative to renderPos
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
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() - 1.0f, ImVec2(renderPos.x + 18.0f, renderPos.y + 32.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, itemAlpha)), desc, nullptr, renderSize.x - 56.0f);
    
    // Render an action circle on the right
    ImU32 themeCol = GetThemeColor(pos.x / 1180.0f);
    ImVec2 actionPos = ImVec2(renderPos.x + renderSize.x - 30.0f, renderPos.y + renderSize.y * 0.5f);
    dl->AddCircle(actionPos, 8.0f, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.35f, itemAlpha)), 12, 1.0f);
    dl->AddCircleFilled(actionPos, 4.0f, ((themeCol & 0x00FFFFFF) | (((unsigned int)(255.0f * (0.3f + 0.7f * hoverT) * itemAlpha)) << 24)), 12);
    
    ImGui::PopID();
    return clicked;
}

static void SaveModSettings(std::ostream &s) {
    tConfItemBase::tConfItemMap const & confmap = tConfItemBase::GetConfItemMap();
    for (tConfItemBase::tConfItemMap::const_iterator iter = confmap.begin(); iter != confmap.end(); ++iter) {
        tConfItemBase* ci = iter->second;
        if (ci && ci->GetTitle().StartsWith("MOD_") && ci->Save()) {
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
    rSurface surface("textures/avatar.jpg");
    SDL_Surface* surf = surface.GetSurface();
    if (!surf) return;
    
    glGenTextures(1, &g_AvatarTexture);
    glBindTexture(GL_TEXTURE_2D, g_AvatarTexture);
    
    GLenum format = surface.GetFormat();
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, surf->w, surf->h, 0, format, GL_UNSIGNED_BYTE, surf->pixels);
}

bool ItemMatchesSearch(const char* title, const char* desc, const char* query) {
    if (!query || query[0] == '\0') return true;
    std::string t = title;
    std::string d = desc;
    std::string q = query;
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    std::transform(d.begin(), d.end(), d.begin(), ::tolower);
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    return t.find(q) != std::string::npos || d.find(q) != std::string::npos;
}

// Function to draw text parsing game color codes (0xRRGGBB and 0xRESETT) in ImGui
static inline bool IsHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
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

void ModMenu::Init() {
    if (g_Initialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load System Font DejaVuSans supporting Cyrillic/Unicode natively
    tString fontPath = tDirectories::Data().GetReadPath("textures/DejaVuSans.ttf");
    bool fontExists = false;
    if (FILE* f = fopen((const char*)fontPath, "rb")) {
        fclose(f);
        fontExists = true;
    }
    if (!fontExists) {
        fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
        if (FILE* f = fopen((const char*)fontPath, "rb")) {
            fclose(f);
            fontExists = true;
        }
    }
    if (!fontExists) {
        const char* windir = getenv("WINDIR");
        if (!windir) windir = getenv("SystemRoot");
        if (windir) {
            std::string winFont = std::string(windir) + "\\Fonts\\segoeui.ttf";
            if (FILE* f = fopen(winFont.c_str(), "rb")) {
                fclose(f);
                fontPath = winFont.c_str();
                fontExists = true;
            }
        }
    }
    if (!fontExists) {
        const char* windir = getenv("WINDIR");
        if (!windir) windir = getenv("SystemRoot");
        if (windir) {
            std::string winFont = std::string(windir) + "\\Fonts\\arial.ttf";
            if (FILE* f = fopen(winFont.c_str(), "rb")) {
                fclose(f);
                fontPath = winFont.c_str();
                fontExists = true;
            }
        }
    }
    if (!fontExists) {
        fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
        if (FILE* f = fopen((const char*)fontPath, "rb")) {
            fclose(f);
            fontExists = true;
        }
    }
    if (!fontExists) {
        fontPath = "C:\\Windows\\Fonts\\arial.ttf";
        if (FILE* f = fopen((const char*)fontPath, "rb")) {
            fclose(f);
            fontExists = true;
        }
    }
    if (!fontExists) {
        fontPath = "/System/Library/Fonts/Supplemental/Arial.ttf";
        if (FILE* f = fopen((const char*)fontPath, "rb")) {
            fclose(f);
            fontExists = true;
        }
    }

    if (fontExists) {
        g_FontDefault = io.Fonts->AddFontFromFileTTF((const char*)fontPath, 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
        g_FontHeader = io.Fonts->AddFontFromFileTTF((const char*)fontPath, 22.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    } else {
        g_FontDefault = io.Fonts->AddFontDefault();
        g_FontHeader = io.Fonts->AddFontDefault();
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
    if (sg_activeTexturePack != "Default") {
        ApplyTexturePackNoSave((const char*)sg_activeTexturePack);
    }
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

    g_CameraLock = lp_cam ? lp_cam->smartCustomGlance : false;
    prev_CameraLock = g_CameraLock;

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
    if (getenv("RETRO_TEST_START_GAME")) {
        g_ModMenuTab = 0;
    }
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

void ModMenu::SetOpen(bool open) {
    g_MenuOpen = open;
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
        lp_cam->smartCustomGlance = g_CameraLock;
        lp_cam->startFOV = g_FOV;
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
    sr_floorDetail = (int)g_FloorDetail;

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
        prev_CameraLock = g_CameraLock;
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
        return true;
    }

    // Capture gameplay action keybinding if waiting
    if (s_BindingAction && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.scancode;
        if (key == SDL_SCANCODE_ESCAPE) {
            // ESC clears all existing keymap binds for this action
            for (int sym_i = 0; sym_i < SDLK_NEWLAST; ++sym_i) {
                if (keymap[sym_i] && keymap[sym_i]->act == s_BindingAction) {
                    keymap[sym_i] = nullptr;
                }
            }
        } else {
            if (key >= 0 && key < SDLK_NEWLAST) {
                keymap[key] = uBindPlayer::NewBind(s_BindingAction, 1); // Player 1
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
                if (g_ActiveTab == 0 || g_DashboardActiveCol == 0) {
                    if (g_DashboardActiveCol < 2) {
                        g_DashboardActiveCol++;
                        return true;
                    }
                }
            }
            if (key == SDLK_LEFT) {
                if (g_ActiveTab == 0 || g_DashboardActiveCol == 0) {
                    if (g_DashboardActiveCol > 0) {
                        g_DashboardActiveCol--;
                        return true;
                    }
                }
            }

            // Decide if ImGui's native keyboard navigation should handle this key
            // We want ImGui to handle keys if we are on a settings page (g_ActiveTab != 0) AND focused on settings columns (g_DashboardActiveCol != 0)
            bool letImGuiHandle = (g_ActiveTab != 0 && g_DashboardActiveCol != 0);

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
                    if (g_DashboardActiveCol == 0) {
                        int numNavItems = 8;
                        if (ModMenu::g_InGameMenuOpen) {
                            numNavItems = sg_hasLastServer ? 9 : 8;
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
                    if (g_DashboardActiveCol == 0) {
                        int numNavItems = 8;
                        if (ModMenu::g_InGameMenuOpen) {
                            numNavItems = sg_hasLastServer ? 9 : 8;
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

    if (!g_MenuOpen && !ModMenu::g_InGameMenuOpen && !isChatting && event->type == SDL_EVENT_KEY_DOWN) {
        int key = event->key.key;
        if (key != 0) {
            bool matched = false;
            auto checkAndToggle = [&](int bind, bool& val) {
                if (bind != 0 && key == bind) {
                    val = !val;
                    matched = true;
                }
            };
            checkAndToggle(g_NoclipKeybind, g_NoclipMode);
            checkAndToggle(g_CleanScreenKeybind, g_CleanScreen);
            checkAndToggle(g_CameraLockKeybind, g_CameraLock);
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
            g_CameraLock = false;
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
            g_CameraLockKeybind = 0;
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
            sr_floorDetail = (int)g_FloorDetail;

            if (lp_cam) {
                lp_cam->smartCustomGlance = g_CameraLock;
                lp_cam->startFOV = g_FOV;
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
            prev_CameraLock = g_CameraLock;
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
                lp_cam->smartCustomGlance = g_CameraLock;
                lp_cam->startFOV = g_FOV;
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

void ModMenu::Render() {
    if (!g_Initialized) { Init(); if (!g_Initialized) return; }

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
    if (g_AutoPacketRefresh && sn_GetNetState() == nCLIENT) {
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

    ImGuiIO& io = ImGui::GetIO();
    
    if (DemoPlayerManager::Instance().IsViewerActive() && DemoPlayerManager::Instance().IsLoaded()) {
        if (DemoPlayerManager::Instance().CamMode() == DemoCamMode::Freecam) {
            Uint8 mouseState = SDL_GetMouseState((int*)NULL, (int*)NULL);
            bool rmbDown = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
            if (rmbDown) {
                SDL_HideCursor();
                io.MouseDrawCursor = false;
            } else {
                SDL_HideCursor();
                io.MouseDrawCursor = true;
            }
        } else {
            SDL_HideCursor();
            io.MouseDrawCursor = true;
        }
    } else if (isHudEditing) {
        SDL_ShowCursor();
        io.MouseDrawCursor = true;
    } else if (g_InGameMenuOpen) {
        SDL_ShowCursor();
        io.MouseDrawCursor = false;
    } else if (!g_MenuOpen) {
        SDL_HideCursor();
        io.MouseDrawCursor = false;
    } else {
        SDL_HideCursor();
        io.MouseDrawCursor = true;
    }

    float target = g_MenuOpen ? 1.0f : 0.0f;
    g_MenuAlpha += (target - g_MenuAlpha) * 0.15f;

    bool needsHud = sg_modClientNameEnabled || sg_modFpsEnabled || 
                    sg_modPingEnabled || sg_modTimeEnabled || 
                    sg_modKeybindsEnabled || 
                    sg_modKDWidgetEnabled || sg_modSpeedometerEnabled || 
                    sg_modRubberMeterEnabled || sg_modBrakeMeterEnabled || 
                    sg_modScoreboardWidgetEnabled || sg_modAliveWidgetEnabled || 
                    sg_modLiveScoreboardEnabled || sg_modZoneTimerEnabled || 
                    sg_modRubberBatteryEnabled || sg_modClassicRubberBatteryEnabled ||
                    sg_modNetHealthEnabled || sg_modMediaWidgetEnabled || isHudEditing;
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
        if (lp_cam->smartCustomGlance != prev_CameraLock) { g_CameraLock = lp_cam->smartCustomGlance; prev_CameraLock = lp_cam->smartCustomGlance; }
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

    static struct timeval last_tv = {0, 0};
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (last_tv.tv_sec != 0) {
        double dt = (double)(tv.tv_sec - last_tv.tv_sec) + (double)(tv.tv_usec - last_tv.tv_usec) / 1000000.0;
        if (dt > 0.0) {
            io.DeltaTime = (float)dt;
            last_tv = tv;
        }
    } else {
        last_tv = tv;
    }

    ImGui::NewFrame();

    HudManager::Update(io.DeltaTime);
    HudManager::Render();

    if (g_MenuAlpha > 0.01f) {
        if (isHudEditing) {
            // Draw background particles while editing HUD via standalone menu
            ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
            DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);
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

void ModMenu::RenderInner() {
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

    const char* tabs[] = { "Visuals", "HUD", "Client", "Combat", "Util", "Theme", "Configs", "Player", "Profiles" };
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
        } else if (i == 3) { // Combat: Crosshair
            dl->AddCircle(iconCenter, 5.0f, iconColor, 12, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x - 7.0f, iconCenter.y), ImVec2(iconCenter.x + 7.0f, iconCenter.y), iconColor, 1.0f);
            dl->AddLine(ImVec2(iconCenter.x, iconCenter.y - 7.0f), ImVec2(iconCenter.x, iconCenter.y + 7.0f), iconColor, 1.0f);
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

    // Search bar (English, fully functional)
    static char searchBuf[64] = "";
    if (showSearch) {
        ImVec2 searchPos = ImVec2(winPos.x + winSize.x - 300.0f, winPos.y + 16.0f);
        ImGui::SetCursorScreenPos(searchPos);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f)); // Vertically center with CLOSE button
        ImGui::PushItemWidth(160.0f);
        ImGui::InputTextWithHint("##search", "Search...", searchBuf, sizeof(searchBuf));
        ImGui::PopItemWidth();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    } else {
        // Clear search so we don't hide items when the search bar itself is hidden
        searchBuf[0] = '\0';
    }

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
    if (isInline && g_DashboardActiveCol == 2 && !ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        ImGui::SetWindowFocus();
    }

    // Smooth scroll interpolation logic
    static float currentScrollY = 0.0f;
    static float targetScrollY = -1.0f;
    static float prevNativeScrollY = -1.0f;
    static int prevActiveTab = -1;

    if (g_ModMenuTab != prevActiveTab) {
        targetScrollY = (getenv("RETRO_TEST_START_GAME") && !se_mainGameTimer) ? 600.0f : 0.0f;
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
    if (getenv("RETRO_TEST_START_GAME") && !se_mainGameTimer) {
        targetScrollY = 1900.0f;
        currentScrollY = 1900.0f;
    } else {
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

    if (searchBuf[0] != '\0') {
        // Global search view with reflowing
        renderItem("Noclip Mode", "Changes camera physics for cinematic flight", &g_NoclipMode, &g_NoclipKeybind);
        renderItem("Clean Screen", "Hide ALL HUD elements for pure cinematic view", &g_CleanScreen, &g_CleanScreenKeybind);
        renderItem("Camera Lock", "Locks camera orientation during glance", &g_CameraLock, &g_CameraLockKeybind);
        renderItem("Custom Hitboxes", "Displays wireframe boundaries of objects", &g_CustomHitbox, &g_CustomHitboxKeybind);
        renderItem("Custom Fog", "Toggle customizable atmospheric world fog", &g_CustomFog, &g_CustomFogKeybind);
        renderItem("Zone Center Indicator", "Show a soft beacon and crosshair at the zone center", &sg_drawZoneCenter);
        renderItem("Corpse Fade Timer", "Show a timer above dead players until their corpse disappears", &sg_corpseTimerEnabled);
        renderItem("Corpse Timer Override", "Override server corpse delay with custom duration", &sg_corpseTimerOverride);
        renderRealSlider("Corpse Fade Duration", "Custom duration in seconds for corpse fade", &sg_corpseTimerDuration, 1.0f, 30.0f, "%.1f");
        renderRealSlider("Player Name Size", "Scale factor for other players' names displayed above their cycles", &sg_modNameSizeScale, 0.2f, 5.0f, "%.2f");
        renderIntSlider("Corpse Trail Style", "0: Legacy, 1: Blinking Warning, 2: Gradual Fade & Shrink", &sg_corpseTrailStyle, 0.0f, 2.0f, "CORPSE_TRAIL_STYLE_FORMAT");
        renderItem("Ambient Dust", "Enable glowing ambient dust particles rising from floor", &sg_modAmbientParticlesEnabled);
        renderIntSlider("Ambient Particles Spawn Mode", "Spawn mode for ambient floor particles", &sg_modAmbientParticlesMode, 0.0f, 1.0f, "AMBIENT_PARTICLES_MODE_FORMAT");
        renderIntSlider("Ambient Particles Min Limit", "Minimum active ambient floor particles count", &sg_modAmbientParticlesMin, 0.0f, 500.0f, "%.0f");
        renderIntSlider("Ambient Particles Max Limit", "Maximum active ambient floor particles count", &sg_modAmbientParticlesMax, 0.0f, 1000.0f, "%.0f");
        renderItem("Rubber Battery", "Wide horizontal rubber bar above bottom panels", &sg_modRubberBatteryEnabled);
        renderItem("Classic Rubber Battery", "Classic horizontal rubber bar above bottom panels", &sg_modClassicRubberBatteryEnabled);
        renderItem("Network Health Widget", "Display connection quality, latency, and packet loss", &sg_modNetHealthEnabled);
        renderItem("Client Name Widget", "Separate widget showing the client logo name", &sg_modClientNameEnabled);
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
        renderItem("Media Player Widget", "OS media / Spotify overlay with EQ bars and controls", &sg_modMediaWidgetEnabled);
        {
            bool dummyPlayPause = false;
            bool dummyNext = false;
            bool dummyPrev = false;
            renderItem("Media: Play/Pause", "Bind key to play or pause media", &dummyPlayPause, &g_MediaPlayPauseKeybind);
            renderItem("Media: Next Track", "Bind key to skip to next track", &dummyNext, &g_MediaNextKeybind);
            renderItem("Media: Prev Track", "Bind key to go back to previous track", &dummyPrev, &g_MediaPrevKeybind);
        }
        
        renderItem("Show HUD", "Toggle global visibility of HUD overlay", &g_ShowHUD, &g_ShowHUDKeybind);
        renderItem("Rubber Gauge", "Toggle display of the rubber gauge", &g_RubberGauge, &g_RubberGaugeKeybind);
        renderItem("Speed Meter", "Toggle display of the speedometer", &g_SpeedMeter, &g_SpeedMeterKeybind);
        renderItem("Brake Meter", "Toggle display of the brake gauge", &g_BrakeMeter, &g_BrakeMeterKeybind);
        renderItem("Scoreboard", "Toggle display of scoreboard", &g_ShowScores, &g_ShowScoresKeybind);
        renderItem("Show Ping", "Toggle display of latency information", &g_ShowPing, &g_ShowPingKeybind);
        renderItem("Alive Counter", "Toggle player alive counter", &g_AliveCounter, &g_AliveCounterKeybind);
        renderItem("Show Fastest", "Toggle display of peak speed attained", &g_ShowFastest, &g_ShowFastestKeybind);
        renderItem("Show FPS (Legacy)", "Toggle display of legacy FPS counter", &sr_FPSOut);
        
        renderItem("Show Time", "Toggle clock rendering in the HUD", &g_ShowTime, &g_ShowTimeKeybind);
        renderItem("24h Format", "Use 24-hour style for the clock", &g_24hFormat, &g_24hFormatKeybind);

        renderSlider("Field of View (FOV)", "Adjust the client camera Field of View", &g_FOV, 30.0f, 150.0f, "%.0f");
        renderSlider("Speed Gauge Size", "Scale factor of the speed meter gauge", &g_SpeedGaugeSize, 0.1f, 3.0f, "%.2f");
        renderSlider("Speed Gauge X Offset", "Horizontal position of the speed meter", &g_SpeedGaugeX, -2.0f, 2.0f, "%.2f");
        renderSlider("Speed Gauge Y Offset", "Vertical position of the speed meter", &g_SpeedGaugeY, -2.0f, 2.0f, "%.2f");
        
        renderSlider("Rubber Gauge Size", "Scale factor of the rubber gauge", &g_RubberGaugeSize, 0.1f, 3.0f, "%.2f");
        renderSlider("Rubber Gauge X Offset", "Horizontal position of the rubber gauge", &g_RubberGaugeX, -2.0f, 2.0f, "%.2f");
        renderSlider("Rubber Gauge Y Offset", "Vertical position of the rubber gauge", &g_RubberGaugeY, -2.0f, 2.0f, "%.2f");
        
        renderItem("Cycle Sparks", "Toggle sparks generation on collisions", &g_Sparks, &g_SparksKeybind);
        renderItem("White Sparks", "Use clean white particles for sparks", &g_WhiteSparks, &g_WhiteSparksKeybind);
        renderItem("Explosions", "Toggle particle explosions upon death", &g_Explosions, &g_ExplosionsKeybind);
        renderItem("Single Line Explosion", "Render legacy crash explosion as a single vertical vector line", &sg_explosionSingleLineUp);
        renderItem("Alpha Blending", "Enables smooth transparency blending", &g_AlphaBlend, &g_AlphaBlendKeybind);
        renderItem("Smooth Shading", "Enables smooth vertex normals shading", &g_SmoothShading, &g_SmoothShadingKeybind);
        renderSlider("Floor Detail", "Sets floor grid/texture complexity (0-3)", &g_FloorDetail, 0.0f, 3.0f, "%.0f");

        renderItem("Rainbow Top Bar", "Enables cycling RGB gradient on top bar", &g_RGBTopBar, &g_RGBTopBarKeybind);
        renderItem("Rainbow Accent", "Enables cycling RGB on all elements", &g_RGBAccent, &g_RGBAccentKeybind);
        renderSlider("Rainbow Speed", "Controls velocity of hue rotation", &g_RGBSpeed, 0.0f, 2.0f, "%.2f");
        renderItem("Particle Backdrop", "Toggle floating background particles", &g_ShowParticles, &g_ShowParticlesKeybind);
        renderSlider("Particle Type", "Choose background particle design (0-2)", &g_ParticleType, 0.0f, 2.0f, "%.0f");
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

        ImVec4 fogCol(g_FogR, g_FogG, g_FogB, 1.0f);
        if (ColorItemAbsolute(GET_CELL_POS(cellIdx % 3, cellIdx / 3), ImVec2(columnW, itemH), "Fog Color", "Customizes the color of atmospheric fog", &fogCol, alphaMultiplier)) {
            g_FogR = fogCol.x;
            g_FogG = fogCol.y;
            g_FogB = fogCol.z;
        }
        cellIdx++;
        renderSlider("Fog Density", "Sets the thickness of the world fog", &g_FogDensity, 0.0f, 0.1f, "%.4f");

        // NEW MOD SETTINGS SEARCH:
        renderItem("Pathfinding Line", "Show escape vector on the floor", &sg_modPathLineEnabled);
        renderIntSlider("Search Depth", "How many turns ahead to explore (2-6)", &sg_modPathDepth, 2.0f, 6.0f, "%.0f");
        renderIntSlider("Max Range", "Maximum distance the path can extend (50-200)", &sg_modPathRange, 50.0f, 200.0f, "%.0f");
        
        renderItem("Waypoints Enabled", "Enable rubber waypoint markers on wall turns", &sg_modWaypointsEnabled);
        renderItem("All Walls Waypoints", "ON = all walls, OFF = rim only", &sg_modWaypointsEnemyWalls);
        renderRealSlider("Min Rubber Waypoint", "Minimum rubber consumed to create marker", &sg_modWaypointsMinRubber, 0.0f, 20.0f, "%.0f");
        renderRealSlider("Waypoint Cooldown", "Cooldown in seconds between markers", &sg_modWaypointsCooldown, 1.0f, 30.0f, "%.0f");
        renderRealSlider("Waypoint Lifetime", "How long waypoint markers stay visible", &sg_modWaypointsLifetime, 5.0f, 30.0f, "%.0f");

        renderItem("Minimap Enabled", "Show/hide the static minimap radar overlay", &sg_modMinimapEnabled);
        renderItem("Minimap Rotate", "Rotate minimap smoothly to match player direction", &sg_modMinimapRotate);
        renderRealSlider("Minimap Rot Speed", "Speed of minimap rotation. 1=Slow/smooth, 30=Instant", &sg_modMinimapRotateSpeed, 1.0f, 30.0f, "%.0f");
        renderRealSlider("Minimap Zoom", "Zoom into minimap content", &sg_modMinimapZoom, 1.0f, 5.0f, "%.1fx");
        renderRealSlider("Minimap Size", "Minimap window scale size", &sg_modMinimapScale, 0.1f, 3.0f, "%.1fx");
        
        renderItem("Modern Live Scoreboard", "Esports-style rank list of teams and players", &sg_modLiveScoreboardEnabled);
        renderItem("Modern Zone Timer", "Sleek capsule showing remaining zone time with alarms", &sg_modZoneTimerEnabled);
        renderItem("Fortress Zone Alerts", "Show conquest/defense alerts for fortress zones", &sg_modFortressAlerts);
        renderItem("Teammate Death", "Flash red warning when a teammate dies", &sg_modTeammateDeathWarning);
        renderItem("K/D Counter HUD", "Show kills/deaths/KD ratio on screen (top-left)", &sg_modKDEnabled);
        {
            bool dummyResetKD = false;
            renderItem("Reset K/D Stats", "Hotkey or toggle to reset your kills/deaths count", &dummyResetKD, &g_ResetKDKeybind);
            if (dummyResetKD) {
                sg_modKDResetFlag = true;
            }
        }
        renderItem("Rubber Battery", "Wide horizontal rubber bar above bottom panels", &sg_modRubberBatteryEnabled);
        renderItem("Classic Rubber Battery", "Classic horizontal rubber bar above bottom panels", &sg_modClassicRubberBatteryEnabled);

        renderItem("Cut-off Predictor", "Show CUT/NO indicator when you can box in an enemy", &sg_modCutoffAimbot);
        renderItem("Proximity Warning", "Red edge glow when enemies are close behind", &sg_modProximityWarning);
        renderItem("3D Rubber Gauge", "Display rubber percentage near cycle in game world", &sg_modRubberGaugeEnabled);
        renderItem("Anti-360 Lock", "Prevent 360-degree suicide by ignoring fast 4th turns", &sg_modAnti360LockEnabled);
        renderRealSlider("Anti-360 Window", "Time window in seconds to check for consecutive turns", &sn_anti360Window, 0.1f, 3.0f, "%.2fs");
#if !PUBLIC_BUILD
        renderRealSlider("Perfect Turn Calib", "Manual trigger distance (0 = rubber-only mode)", &sg_perfectTurnCalibration, 0.0f, 2.0f, "%.2f");
        renderItem("AutoEscape Ping Comp", "Use network ping to time escape turns", &sg_autoEscapePingComp);
        renderRealSlider("AutoEscape Margin", "Safety margin before max rubber", &sg_autoEscapeRubberMargin, 0.0f, 1.0f, "%.2f");
#endif

        renderItem("Chat Calculator", "Auto-reply to math expressions in chat (e.g. 2+2)", &sg_modChatCalcEnabled);
#if !PUBLIC_BUILD
        renderItem("AI Trash Talker", "Auto trash talk based on game events and keywords", &sg_modTrashTalkEnabled);
#endif
        renderItem("Auto-Greet", "Automatically greet when joining a server", &sg_modAutoGreet);
        renderItem("Friendly Responder", "Polite responses only (gg, wp, gf, hi, bb)", &sg_modFriendlyChat);
        renderItem("Color Picker", "Type 'color NAME' in chat to see or apply player RGB", &sg_modColorPicker);
        renderItem("Mod Menu Toggle Keybind", "Keybind to open the Mod Menu", &g_ModMenuKeybindEnabled, &g_ModMenuKeybind);
        renderItem("Auto Packet Refresh", "Trigger sync commands when packet delay exceeds 1.2s", &g_AutoPacketRefresh, &g_PacketRefreshKeybind);

        renderRealSlider("Flight Speed", "Base movement speed in noclip mode", &sg_noclipSpeed, 5.0f, 500.0f, "%.0f");
        renderRealSlider("Slow Factor", "Speed multiplier when slow key is held", &sg_noclipSlowFactor, 0.05f, 1.0f, "%.2f");
        renderRealSlider("Mouse Sensitivity", "Mouse look sensitivity in noclip mode", &sg_noclipMouseSens, 0.0005f, 0.02f, "%.4f");
        renderRealSlider("Orbit Speed", "Angular speed of orbit camera", &sg_noclipOrbitSpeed, 0.05f, 2.0f, "%.2f");
        renderRealSlider("Orbit Radius", "Distance from orbit center to camera", &sg_noclipOrbitRadius, 10.0f, 500.0f, "%.0f");
        renderRealSlider("Orbit Height", "Height of orbit camera above target", &sg_noclipOrbitHeight, 5.0f, 300.0f, "%.0f");
        renderRealSlider("Follow Distance", "Camera distance behind followed player", &sg_noclipFollowDist, 5.0f, 200.0f, "%.0f");
        renderRealSlider("Follow Height", "Camera height above followed player", &sg_noclipFollowHeight, 5.0f, 200.0f, "%.0f");
        renderRealSlider("Smooth Factor", "LERP smoothing strength for follow camera", &sg_noclipSmoothFactor, 1.0f, 20.0f, "%.1f");

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
    } else {
        // Tab-specific screens
        if (g_ModMenuTab == 0) { // Visuals
            renderItem("Noclip Mode", "Changes camera physics for cinematic flight", &g_NoclipMode, &g_NoclipKeybind);
            renderItem("Clean Screen", "Hide ALL HUD elements for pure cinematic view", &g_CleanScreen, &g_CleanScreenKeybind);
            renderItem("Camera Lock", "Locks camera orientation during glance", &g_CameraLock, &g_CameraLockKeybind);
            renderItem("Custom Hitboxes", "Displays wireframe boundaries of objects", &g_CustomHitbox, &g_CustomHitboxKeybind);
            renderItem("Custom Fog", "Toggle customizable atmospheric world fog", &g_CustomFog, &g_CustomFogKeybind);
            ImVec4 fogCol(g_FogR, g_FogG, g_FogB, 1.0f);
            if (ColorItemAbsolute(GET_CELL_POS(cellIdx % 3, cellIdx / 3), ImVec2(columnW, itemH), "Fog Color", "Customizes the color of atmospheric fog", &fogCol, alphaMultiplier)) {
                g_FogR = fogCol.x;
                g_FogG = fogCol.y;
                g_FogB = fogCol.z;
            }
            cellIdx++;
            renderSlider("Fog Density", "Sets the thickness of the world fog", &g_FogDensity, 0.0f, 0.1f, "%.4f");
            renderItem("Zone Center Indicator", "Show a soft beacon and crosshair at the zone center", &sg_drawZoneCenter);
            renderItem("Corpse Fade Timer", "Show a timer above dead players until their corpse disappears", &sg_corpseTimerEnabled);
            renderItem("Corpse Timer Override", "Override server corpse delay with custom duration", &sg_corpseTimerOverride);
            renderRealSlider("Corpse Fade Duration", "Custom duration in seconds for corpse fade", &sg_corpseTimerDuration, 1.0f, 30.0f, "%.1f");
            renderRealSlider("Player Name Size", "Scale factor for other players' names displayed above their cycles", &sg_modNameSizeScale, 0.2f, 5.0f, "%.2f");
            renderIntSlider("Corpse Trail Style", "0: Legacy, 1: Blinking Warning, 2: Gradual Fade & Shrink", &sg_corpseTrailStyle, 0.0f, 2.0f, "CORPSE_TRAIL_STYLE_FORMAT");
            renderItem("Alpha Blending", "Enables smooth transparency blending", &g_AlphaBlend, &g_AlphaBlendKeybind);
            renderItem("Smooth Shading", "Enables smooth vertex normals shading", &g_SmoothShading, &g_SmoothShadingKeybind);
            renderSlider("Floor Detail", "Sets floor grid/texture complexity (0-3)", &g_FloorDetail, 0.0f, 3.0f, "%.0f");
            
            // Migrated Visuals: Pathfinding & Waypoints
            renderItem("Pathfinding Line", "Show escape vector on the floor", &sg_modPathLineEnabled);
            renderIntSlider("Search Depth", "How many turns ahead to explore (2-6)", &sg_modPathDepth, 2.0f, 6.0f, "%.0f");
            renderIntSlider("Max Range", "Maximum distance the path can extend (50-200)", &sg_modPathRange, 50.0f, 200.0f, "%.0f");
            
            renderItem("Waypoints Enabled", "Enable rubber waypoint markers on wall turns", &sg_modWaypointsEnabled);
            renderItem("All Walls Waypoints", "ON = all walls, OFF = rim only", &sg_modWaypointsEnemyWalls);
            renderRealSlider("Min Rubber Waypoint", "Minimum rubber consumed to create marker", &sg_modWaypointsMinRubber, 0.0f, 20.0f, "%.0f");
            renderRealSlider("Waypoint Cooldown", "Cooldown in seconds between markers", &sg_modWaypointsCooldown, 1.0f, 30.0f, "%.0f");
            renderRealSlider("Waypoint Lifetime", "How long waypoint markers stay visible", &sg_modWaypointsLifetime, 5.0f, 30.0f, "%.0f");

            // [MOD] Visual Overhaul Settings
            renderItem("Particle Engine", "Enable custom cycle/ambient particles", &sg_modParticleSystemEnabled);
            renderIntSlider("Death Burst Count", "Number of particles spawned on cycle death", &sg_modDeathParticlesCount, 0.0f, 500.0f, "%.0f");
            renderItem("Camera Shake", "Enable camera shake when a cycle crashes", &sg_modScreenShakeEnabled);
            renderRealSlider("Shake Intensity", "Intensity offset of screen shaking", &sg_modScreenShakeIntensity, 0.0f, 2.0f, "%.2f");
            renderItem("Ambient Dust", "Enable glowing ambient dust particles rising from floor", &sg_modAmbientParticlesEnabled);
            renderIntSlider("Ambient Particles Spawn Mode", "Spawn mode for ambient floor particles", &sg_modAmbientParticlesMode, 0.0f, 1.0f, "AMBIENT_PARTICLES_MODE_FORMAT");
            renderIntSlider("Ambient Particles Min Limit", "Minimum active ambient floor particles count", &sg_modAmbientParticlesMin, 0.0f, 500.0f, "%.0f");
            renderIntSlider("Ambient Particles Max Limit", "Maximum active ambient floor particles count", &sg_modAmbientParticlesMax, 0.0f, 1000.0f, "%.0f");
            renderItem("Trail Gradient", "Fades trail opacity from top (opaque) to bottom (transparent)", &sg_modTrailGradientEnabled);
            renderRealSlider("Trail Alpha Top", "Opacity level near the cycle/trail top", &sg_modTrailAlphaTop, 0.0f, 1.0f, "%.2f");
            renderRealSlider("Trail Alpha Bottom", "Opacity level near the grid floor", &sg_modTrailAlphaBottom, 0.0f, 1.0f, "%.2f");
            renderRealSlider("Trail Height", "Height scale of all cycle trails (1.0 = normal, 0.1 = pixel line)", &sg_modWallHeightMultiplier, 0.1f, 1.0f, "%.2f");
            renderItem("Instanced Trails", "Enable next-gen instanced rendering for trail walls (Safe Fallback if disabled)", &cfg_EnableInstancing);
            bool prevMSAA = cfg_MSAA;
            renderItem("Anti-Aliasing (MSAA)", "Enables 8x hardware multi-sample anti-aliasing", &cfg_MSAA);
            if (cfg_MSAA != prevMSAA) {
                st_SaveConfig();
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
        } else if (g_ModMenuTab == 1) { // HUD
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
            if (sg_modLiveScoreboardEnabled) {
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
            renderItem("K/D Counter HUD", "Show kills/deaths/KD ratio on screen (top-left)", &sg_modKDEnabled);
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
            renderItem("Edit HUD Layout", "Enter interactive dragging/snapping design mode", &isHudEditing);
        } else if (g_ModMenuTab == 2) { // Client HUD Position adjustments
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

            renderItem("Noclip Mode", "Changes camera physics for cinematic flight", &g_NoclipMode, &g_NoclipKeybind);
            renderItem("Clean Screen", "Hide ALL HUD elements for pure cinematic view", &g_CleanScreen, &g_CleanScreenKeybind);
            renderRealSlider("Flight Speed", "Base movement speed in noclip mode", &sg_noclipSpeed, 5.0f, 500.0f, "%.0f");
            renderRealSlider("Slow Factor", "Speed multiplier when slow key is held", &sg_noclipSlowFactor, 0.05f, 1.0f, "%.2f");
            renderRealSlider("Mouse Sensitivity", "Mouse look sensitivity in noclip mode", &sg_noclipMouseSens, 0.0005f, 0.02f, "%.4f");
            renderRealSlider("Orbit Speed", "Angular speed of orbit camera", &sg_noclipOrbitSpeed, 0.05f, 2.0f, "%.2f");
            renderRealSlider("Orbit Radius", "Distance from orbit center to camera", &sg_noclipOrbitRadius, 10.0f, 500.0f, "%.0f");
            renderRealSlider("Orbit Height", "Height of orbit camera above target", &sg_noclipOrbitHeight, 5.0f, 300.0f, "%.0f");
            renderRealSlider("Follow Distance", "Camera distance behind followed player", &sg_noclipFollowDist, 5.0f, 200.0f, "%.0f");
            renderRealSlider("Follow Height", "Camera height above followed player", &sg_noclipFollowHeight, 5.0f, 200.0f, "%.0f");
            renderRealSlider("Smooth Factor", "LERP smoothing strength for follow camera", &sg_noclipSmoothFactor, 1.0f, 20.0f, "%.1f");

            if (renderButton("Reset HUD Layout", "Restore default positions and sizes for all HUD gauges")) {
                g_OpenResetHUDPopup = true;
            }
        } else if (g_ModMenuTab == 3) { // Combat
            renderItem("Cut-off Predictor", "Show CUT/NO indicator when you can box in an enemy", &sg_modCutoffAimbot);
            renderItem("Proximity Warning", "Red edge glow when enemies are close behind", &sg_modProximityWarning);
            renderItem("3D Rubber Gauge", "Display rubber percentage near cycle in game world", &sg_modRubberGaugeEnabled);
            renderItem("Anti-360 Lock", "Prevent 360-degree suicide by ignoring fast 4th turns", &sg_modAnti360LockEnabled);
            renderRealSlider("Anti-360 Window", "Time window in seconds to check for consecutive turns", &sn_anti360Window, 0.1f, 3.0f, "%.2fs");
#if !PUBLIC_BUILD
            renderRealSlider("Perfect Turn Calib", "Manual trigger distance (0 = rubber-only mode)", &sg_perfectTurnCalibration, 0.0f, 2.0f, "%.2f");
            renderItem("AutoEscape Ping Comp", "Use network ping to time escape turns", &sg_autoEscapePingComp);
            renderRealSlider("AutoEscape Margin", "Safety margin before max rubber", &sg_autoEscapeRubberMargin, 0.0f, 1.0f, "%.2f");
#endif
        } else if (g_ModMenuTab == 4) { // Util
            // Kill sounds
            renderItem("Kill Sounds", "Voice announcement pack on kill streaks", &sg_modKillSoundsEnabled);
            renderIntSlider("Announcer Pack", "Choose announcer pack style", &sg_modKillAnnouncerPack, 0.0f, 2.0f, "ANNOUNCER_PACK_FORMAT");

            renderItem("Show Time", "Toggle clock rendering in the HUD", &g_ShowTime, &g_ShowTimeKeybind);
            renderItem("24h Format", "Use 24-hour style for the clock", &g_24hFormat, &g_24hFormatKeybind);
            renderItem("Cycle Sparks", "Toggle sparks generation on collisions", &g_Sparks, &g_SparksKeybind);
            renderItem("White Sparks", "Use clean white particles for sparks", &g_WhiteSparks, &g_WhiteSparksKeybind);
            renderItem("Explosions", "Toggle particle explosions upon death", &g_Explosions, &g_ExplosionsKeybind);
            renderItem("Single Line Explosion", "Render legacy crash explosion as a single vertical vector line", &sg_explosionSingleLineUp);
            renderItem("Auto Packet Refresh", "Trigger sync commands when packet delay exceeds 1.2s", &g_AutoPacketRefresh, &g_PacketRefreshKeybind);
            
            // Migrated Util: Chat Bots & Automation
            renderItem("Chat Calculator", "Auto-reply to math expressions in chat (e.g. 2+2)", &sg_modChatCalcEnabled);
#if !PUBLIC_BUILD
            renderItem("AI Trash Talker", "Auto trash talk based on game events and keywords", &sg_modTrashTalkEnabled);
#endif
            renderItem("Auto-Greet", "Automatically greet when joining a server", &sg_modAutoGreet);
            renderItem("Friendly Responder", "Polite responses only (gg, wp, gf, hi, bb)", &sg_modFriendlyChat);
            renderItem("Color Picker", "Type 'color NAME' in chat to see or apply player RGB", &sg_modColorPicker);
            renderItem("Mod Menu Toggle Keybind", "Keybind to open the Mod Menu", &g_ModMenuKeybindEnabled, &g_ModMenuKeybind);
        } else if (g_ModMenuTab == 5) { // Theme
            renderItem("Rainbow Top Bar", "Enables cycling RGB gradient on top bar", &g_RGBTopBar, &g_RGBTopBarKeybind);
            renderItem("Rainbow Accent", "Enables cycling RGB on all elements", &g_RGBAccent, &g_RGBAccentKeybind);
            renderSlider("Rainbow Speed", "Controls velocity of hue rotation", &g_RGBSpeed, 0.0f, 2.0f, "%.2f");

            renderItem("Particle Backdrop", "Toggle floating background particles", &g_ShowParticles, &g_ShowParticlesKeybind);
            renderSlider("Particle Type", "Choose particle shape (0=Dust, 1=Rain, 2=Stars)", &g_ParticleType, 0.0f, 2.0f, "%.0f");
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
        } else if (g_ModMenuTab == 6) { // Configs
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
        } else if (g_ModMenuTab == 7) { // Player Setup
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
                if (ImGui::InputText("Screen Name", nameBuf, sizeof(nameBuf))) {
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
                if (ImGui::InputText("Global ID", gidBuf, sizeof(gidBuf))) {
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
                if (ImGui::InputText("Custom Team Name", teamNameBuf, sizeof(teamNameBuf))) {
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
                ImGui::SliderInt("##FOV_Slider", &lp->startFOV, 30, 160);
                
                ImGui::Checkbox("Smart Glance Custom", &lp->smartCustomGlance);
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
        } else if (g_ModMenuTab == 8) { // Profiles & Managers
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
                
                float rightAlignX = ImGui::GetContentRegionMax().x - 110.0f;
                ImGui::SameLine(rightAlignX);
                if (ImGui::Button("Load", ImVec2(50, 20))) {
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
                ImGui::SameLine(rightAlignX + 55.0f);
                if (ImGui::Button("Delete", ImVec2(50, 20))) {
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
                
                float rightAlignX = ImGui::GetContentRegionMax().x - 110.0f;
                float nameWidth = rightAlignX - ImGui::GetCursorPosX() - 5.0f;
                if (nameWidth < 50.0f) nameWidth = 50.0f;
                if (ImGui::Selectable(g_SavedColors[i].name.c_str(), isSelected, 0, ImVec2(nameWidth, 0))) {
                    selectedColorIdx = i;
                }
                
                ImGui::SameLine(rightAlignX);
                if (ImGui::Button("Load", ImVec2(50, 20))) {
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
                ImGui::SameLine(rightAlignX + 55.0f);
                if (ImGui::Button("Delete", ImVec2(50, 20))) {
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
        lp_cam->smartCustomGlance = g_CameraLock;
        lp_cam->startFOV = g_FOV;
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
    sr_floorDetail = (int)g_FloorDetail;
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
        prev_CameraLock = g_CameraLock;
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
    } else if (sn_GetNetState() == nCLIENT) {
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
        if (s_lower == "user.cfg" || s_lower == "settings.cfg" || s_lower == "master.cfg" || s_lower == "aiplayers.cfg" || s_lower == "autoexec.cfg") {
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
    SDL_ShowCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    
    // Purge input queue
    SDL_Event pEvent;
    while (su_GetSDLInput(pEvent)) {}
}

void StartLocalGameHelper() {
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
    SDL_ShowCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    
    // Purge input queue
    SDL_Event pEvent;
    while (su_GetSDLInput(pEvent)) {}
}

void StartDemoPlaybackHelper() {
    auto& dp = DemoPlayerManager::Instance();
    if (!dp.IsLoaded()) return;

    // Temporarily disable the dashboard so it doesn't render on top
    ModMenu::g_CustomMainMenuTempDisabled = true;
    ModMenu::g_MainMenuActive = false;
    g_MenuOpen = false;
    g_MenuAlpha = 0.0f;

    dp.StartViewer3D();

    // Run the game's actual local singleplayer game match loop
    sg_SinglePlayerGame();

    dp.StopViewer3D();

    // Restore dashboard state
    ModMenu::g_MainMenuActive = true;
    ModMenu::g_CustomMainMenuTempDisabled = false;
    g_MenuOpen = true;
    g_MenuAlpha = 1.0f;

    SDL_ShowCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = false;

    // Purge stale SDL events accumulated during viewer
    SDL_Event pEvent;
    while (su_GetSDLInput(pEvent)) {}
}



// Forward declaration — drawPanel is defined below with RenderDemoRecorderTabContent
static void drawPanel(ImDrawList* dl, ImVec2 pos, ImVec2 size, bool active, const char* title);

// ── Demo Player Full Tab ─────────────────────────────────────────────────
// Standalone tab (g_ActiveTab == 6) with file picker, transport controls
// and live playback state — occupies all 3 panels, panel0 is NAVIGATION.
static void RenderDemoPlayerFullTab(ImDrawList* dl, ImGuiIO& io,
                                    ImVec2 panel0Pos, ImVec2 panel0Size,
                                    ImVec2 panel1Pos, ImVec2 panel1Size,
                                    ImVec2 panel2Pos, ImVec2 panel2Size,
                                    bool hasPanel2) {
    auto& dp = DemoPlayerManager::Instance();

    // ── Left main panel: File picker + Transport ──────────────────────────
    drawPanel(dl, panel1Pos, panel1Size, true, "DEMO PLAYER");

    // Pulsing blue dot status
    {
        float t   = (float)ImGui::GetTime();
        bool  act = dp.IsPlaying();
        float a   = act ? (0.4f + 0.6f * ((sinf(t * 3.5f) + 1.f) * .5f)) : 0.18f;
        ImU32 col = act ? IM_COL32(70, 160, 255, (uint8_t)(a * 255))
                        : IM_COL32(60, 60, 80,   100);
        ImVec2 dc(panel1Pos.x + panel1Size.x - 22.f, panel1Pos.y + 30.f);
        if (act) dl->AddCircleFilled(dc, 9.f,  IM_COL32(70,160,255,(uint8_t)(a*50)));
        dl->AddCircleFilled(dc, act ? 5.5f : 4.f, col);
    }

    ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 14.f, panel1Pos.y + 54.f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (ImGui::BeginChild("##DPMain",
                          ImVec2(panel1Size.x - 28.f, panel1Size.y - 68.f),
                          false, ImGuiWindowFlags_NavFlattened)) {

        float bW = ImGui::GetContentRegionAvail().x;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.f);

        // ── File path ────────────────────────────────────────────────────
        static char s_Path[512] = "";
        static bool s_PathInited = false;
        if (!s_PathInited) {
            std::string d = (const char*)tDirectories::Var().GetWritePath("demos/");
            snprintf(s_Path, sizeof(s_Path), "%s", d.c_str());
            s_PathInited = true;
        }

        ImGui::TextColored(ImVec4(0.55f,0.65f,0.80f,1.f), "Demo file (.aarec):");
        ImGui::PushItemWidth(bW - 100.f);
        ImGui::InputText("##dpPath", s_Path, sizeof(s_Path));
        ImGui::PopItemWidth();
        ImGui::SameLine();

        bool loaded = dp.IsLoaded();
        ImGui::PushStyleColor(ImGuiCol_Button,
            loaded ? ImVec4(0.18f,0.50f,0.85f,0.9f) : ImVec4(0.10f,0.44f,0.10f,0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            loaded ? ImVec4(0.28f,0.65f,1.f,1.f) : ImVec4(0.15f,0.65f,0.15f,1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.06f,0.28f,0.06f,1.f));
        if (ImGui::Button(loaded ? "Reload##dpload" : "Open##dpload", ImVec2(92.f, 24.f))) {
            if (strlen(s_Path) > 0 && dp.LoadFile(std::string(s_Path))) {
                dp.Play();
                s_PendingStartDemoPlayback = true;
            }
        }
        ImGui::PopStyleColor(3);

        // Recent recordings quick-pick
        const auto& recent = DemoRecorder::Instance().RecentFiles();
        if (!recent.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.50f,0.55f,0.65f,0.85f), "Recent recordings:");
            ImGui::Spacing();
            size_t mx = std::min(recent.size(), (size_t)5);
            for (int ri = 0; ri < (int)mx; ++ri) {
                std::string base = recent[ri];
                size_t sl = base.find_last_of("/\\");
                if (sl != std::string::npos) base = base.substr(sl+1);
                if (base.size() > 6 && base.substr(base.size()-6) == ".aarec")
                    base = base.substr(0, base.size()-6);
                char lbl[96];
                snprintf(lbl, sizeof(lbl), "%s##rp%d", base.c_str(), ri);
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f,0.12f,0.18f,0.85f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f,0.28f,0.45f,0.95f));
                if (ImGui::Button(lbl, ImVec2(bW, 24.f))) {
                    snprintf(s_Path, sizeof(s_Path), "%s", recent[ri].c_str());
                    if (dp.LoadFile(recent[ri])) {
                        dp.Play();
                        s_PendingStartDemoPlayback = true;
                    }
                }
                ImGui::PopStyleColor(2);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (!loaded) {
            ImGui::TextColored(ImVec4(0.40f,0.40f,0.48f,0.85f),
                "Load a .aarec file to start playback.");
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.48f, 0.22f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.32f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.35f, 0.15f, 1.00f));
            if (ImGui::Button("START 3D PLAYBACK", ImVec2(bW, 36.f))) {
                s_PendingStartDemoPlayback = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // ── Transport controls ────────────────────────────────────────
            ImGui::TextColored(ImVec4(0.55f,0.65f,0.80f,1.f), "Transport:");
            ImGui::Spacing();

            // Timeline slider
            float tot = (float)dp.TotalDuration();
            float cur = (float)dp.CurrentTime();
            static float s_SeekVal = 0.f;
            static bool  s_Seeking = false;

            ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0.35f,0.65f,1.f,1.f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.50f,0.80f,1.f,1.f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0.08f,0.10f,0.16f,0.9f));
            float frac = (tot > 0.f) ? (cur / tot) : 0.f;
            if (!s_Seeking) s_SeekVal = frac;
            ImGui::PushItemWidth(bW);
            if (ImGui::SliderFloat("##dpSlider", &s_SeekVal, 0.f, 1.f, ""))
                s_Seeking = true;
            if (s_Seeking && !io.MouseDown[0]) {
                dp.SeekToTime((double)(s_SeekVal * tot));
                s_Seeking = false;
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleColor(3);

            int cm = (int)(cur/60), cs_ = (int)cur%60;
            int tm = (int)(tot/60), ts_ = (int)tot%60;
            char tbuf[40];
            snprintf(tbuf, sizeof(tbuf), "%02d:%02d / %02d:%02d", cm, cs_, tm, ts_);
            ImGui::TextColored(ImVec4(0.70f,0.85f,1.f,1.f), "%s", tbuf);
            if (!dp.GetRoundIndex().empty()) {
                ImGui::SameLine();
                char rbuf[24];
                snprintf(rbuf, sizeof(rbuf), "  Round %d / %d",
                         dp.CurrentRound()+1, dp.TotalRounds());
                ImGui::TextColored(ImVec4(0.45f,1.f,0.65f,1.f), "%s", rbuf);
            }

            ImGui::Spacing();

            // Playback buttons row
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.13f,0.18f,0.28f,0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f,0.38f,0.62f,1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.08f,0.12f,0.22f,1.f));

            if (ImGui::Button("|<##dpPrev", ImVec2(36.f,28.f))) dp.PrevRound();
            ImGui::SameLine();
            const char* ppLbl = dp.IsPlaying() ? "||##dpPause" : " >##dpPlay";
            if (ImGui::Button(ppLbl, ImVec2(44.f,28.f))) dp.TogglePlayPause();
            ImGui::SameLine();
            if (ImGui::Button("|>##dpStep", ImVec2(36.f,28.f))) dp.StepForward();
            ImGui::SameLine();
            if (ImGui::Button(">|##dpNext", ImVec2(36.f,28.f))) dp.NextRound();

            ImGui::SameLine(0.f,14.f);

            // Speed buttons
            for (int i = 0; i < DEMO_SPEED_COUNT; ++i) {
                static const char* spLabels[] = {"0.25x","0.5x","1x","2x","4x"};
                char sid[24]; snprintf(sid, sizeof(sid), "%s##dps%d", spLabels[i], i);
                bool act = (i == dp.SpeedIndex());
                if (act) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f,0.55f,1.f,0.9f));
                if (ImGui::Button(sid, ImVec2(42.f,28.f))) dp.SetSpeedIndex(i);
                if (act) ImGui::PopStyleColor();
                ImGui::SameLine();
            }

            ImGui::PopStyleColor(3);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Camera mode
            ImGui::TextColored(ImVec4(0.55f,0.65f,0.80f,1.f), "Camera mode:");
            ImGui::Spacing();
            {
                bool fc = (dp.CamMode() == DemoCamMode::Freecam);
                ImGui::PushStyleColor(ImGuiCol_Button,
                    !fc ? ImVec4(0.15f,0.50f,0.20f,0.85f) : ImVec4(0.12f,0.14f,0.20f,0.75f));
                if (ImGui::Button("Spectator##dpCam0", ImVec2(bW*0.48f,28.f)))
                    dp.SetCamMode(DemoCamMode::Spectator);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button,
                    fc ? ImVec4(0.65f,0.35f,0.05f,0.9f) : ImVec4(0.12f,0.14f,0.20f,0.75f));
                if (ImGui::Button("Freecam##dpCam1", ImVec2(bW*0.48f,28.f)))
                    dp.SetCamMode(DemoCamMode::Freecam);
                ImGui::PopStyleColor();
            }
            if (dp.CamMode() == DemoCamMode::Freecam) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.f,0.7f,0.3f,0.85f),
                    "WASD move | Mouse look | Q/E up/down");
            }
        }

        ImGui::PopStyleVar(); // FrameRounding
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // ── Right panel: Player list + Stats ─────────────────────────────────
    ImVec2 rPos  = hasPanel2 ? panel2Pos : ImVec2(panel1Pos.x + panel1Size.x + 14.f, panel1Pos.y);
    ImVec2 rSize = hasPanel2 ? panel2Size : ImVec2(panel2Size.x, panel1Size.y);
    drawPanel(dl, rPos, rSize, false, "PLAYBACK INFO");

    {
        float sX = rPos.x + 18.f;
        float sY = rPos.y + 60.f;
        float W  = rPos.x + rSize.x - 18.f - sX;

        auto infoRow = [&](const char* label, const char* val, ImU32 valCol) {
            dl->AddText(ImVec2(sX, sY), IM_COL32(100,110,130,220), label);
            ImVec2 vs = ImGui::CalcTextSize(val);
            dl->AddText(ImVec2(rPos.x + rSize.x - 18.f - vs.x, sY), valCol, val);
            sY += ImGui::GetTextLineHeight() + 4.f;
            dl->AddLine(ImVec2(sX,sY), ImVec2(sX+W,sY), IM_COL32(28,28,36,160));
            sY += 8.f;
        };

        char tmp[64];
        if (dp.IsLoaded()) {
            int cm2=(int)(dp.CurrentTime()/60), cs2=(int)dp.CurrentTime()%60;
            snprintf(tmp,sizeof(tmp),"%02d:%02d", cm2, cs2);
            infoRow("Position", tmp, IM_COL32(255,255,255,240));

            int tm2=(int)(dp.TotalDuration()/60), ts2=(int)dp.TotalDuration()%60;
            snprintf(tmp,sizeof(tmp),"%02d:%02d", tm2, ts2);
            infoRow("Duration", tmp, IM_COL32(200,220,255,230));

            snprintf(tmp,sizeof(tmp),"%d", dp.TotalRounds());
            infoRow("Rounds", tmp, IM_COL32(140,220,170,230));

            static const char* spNames[] = {"0.25x","0.5x","1x","2x","4x"};
            infoRow("Speed", spNames[dp.SpeedIndex()], IM_COL32(255,210,100,230));

            infoRow("Camera",
                    dp.CamMode()==DemoCamMode::Freecam ? "Freecam" : "Spectator",
                    IM_COL32(100,180,255,230));

            sY += 10.f;
            dl->AddText(ImVec2(sX, sY), IM_COL32(100,110,130,220), "Players:");
            sY += ImGui::GetTextLineHeight() + 8.f;

            // Player cards
            const auto& states = dp.CycleStates();
            ImGui::SetCursorScreenPos(ImVec2(sX, sY));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
            float listH = rPos.y + rSize.y - sY - 14.f;
            if (ImGui::BeginChild("##dpPlayerList",
                                  ImVec2(rSize.x - 36.f, listH),
                                  false, ImGuiWindowFlags_NavFlattened)) {
                ImDrawList* rDl = ImGui::GetWindowDrawList();
                for (int pi = 0; pi < (int)states.size(); ++pi) {
                    const auto& st = states[pi];
                    bool sel = (pi == dp.SpectatorTarget());

                    ImVec2 cMin = ImGui::GetCursorScreenPos();
                    float  cW   = rSize.x - 36.f;
                    ImVec2 cMax(cMin.x + cW, cMin.y + 48.f);

                    bool hov = io.MousePos.x >= cMin.x && io.MousePos.x <= cMax.x &&
                               io.MousePos.y >= cMin.y && io.MousePos.y <= cMax.y;
                    if (hov && io.MouseClicked[0])
                        dp.SetSpectatorTarget(pi);

                    ImU32 bg = sel ? IM_COL32(25,50,90,220) :
                               (hov ? IM_COL32(20,25,38,200) : IM_COL32(12,14,20,170));
                    rDl->AddRectFilled(cMin, cMax, bg, 6.f);
                    rDl->AddRectFilled(ImVec2(cMin.x, cMin.y),
                                       ImVec2(cMin.x+3.f, cMax.y),
                                       st.alive ? IM_COL32(60,210,90,220) : IM_COL32(200,50,50,200), 2.f);
                    rDl->AddRect(cMin, cMax,
                        sel ? IM_COL32(80,140,255,220) : IM_COL32(30,35,50,140), 6.f);

                    // Alive dot
                    ImVec2 dotC(cMin.x + 16.f, cMin.y + 24.f);
                    rDl->AddCircleFilled(dotC, 5.f,
                        st.alive ? IM_COL32(60,230,90,255) : IM_COL32(200,50,50,255));

                    // Name
                    rDl->AddText(ImVec2(cMin.x + 30.f, cMin.y + 8.f),
                                 IM_COL32(220,230,250,245), st.name);

                    // Stats line
                    char stLine[80];
                    snprintf(stLine,sizeof(stLine),"spd %.1f  rub %.2f  pos(%.0f,%.0f)",
                             st.speed, st.rubber, st.posX, st.posY);
                    rDl->AddText(ImVec2(cMin.x + 30.f, cMin.y + 28.f),
                                 IM_COL32(100,115,140,200), stLine);

                    ImGui::Dummy(ImVec2(cW, 54.f));
                }
                if (states.empty()) {
                    ImGui::SetCursorPosY(20.f);
                    ImGui::TextColored(ImVec4(0.38f,0.38f,0.43f,0.8f), "No player data yet.");
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        } else {
            dl->AddText(ImVec2(sX, sY), IM_COL32(80,85,100,200),
                "Load a demo file");
            sY += ImGui::GetTextLineHeight() + 4.f;
            dl->AddText(ImVec2(sX, sY), IM_COL32(80,85,100,200),
                "to see player info.");
        }
    }
}

// ── Demo Player: file browser + open controls ────────────────────────────
static void RenderDemoPlayerBrowserPanel(ImDrawList* dl, ImGuiIO& io,
                                         ImVec2 panel1Pos, ImVec2 panel1Size) {
    float panH  = 180.f;
    float panY  = panel1Pos.y + panel1Size.y + 14.f;
    ImVec2 pPos(panel1Pos.x, panY);
    ImVec2 pSz (panel1Size.x, panH);

    dl->AddRectFilled(pPos, ImVec2(pPos.x + pSz.x, pPos.y + pSz.y),
                      IM_COL32(10,10,14,200), 10.f);
    dl->AddRect(pPos, ImVec2(pPos.x + pSz.x, pPos.y + pSz.y),
                IM_COL32(50,80,140,180), 10.f, 0, 1.f);

    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImVec2(pPos.x + 16.f, pPos.y + 14.f),
                IM_COL32(130,190,255,240), "DEMO PLAYER");
    if (g_FontHeader) ImGui::PopFont();
    dl->AddRectFilledMultiColor(
        ImVec2(pPos.x + 16.f, pPos.y + 38.f),
        ImVec2(pPos.x + pSz.x - 16.f, pPos.y + 40.f),
        GetThemeColor(0.f), GetThemeColor(0.8f), GetThemeColor(0.8f), GetThemeColor(0.f));

    auto& dp = DemoPlayerManager::Instance();

    ImGui::SetCursorScreenPos(ImVec2(pPos.x + 14.f, pPos.y + 48.f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (ImGui::BeginChild("##DPBrowser", ImVec2(pSz.x - 28.f, panH - 58.f),
                          false, ImGuiWindowFlags_NavFlattened)) {

        float bW = ImGui::GetContentRegionAvail().x;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);

        static char s_DPFilePath[512] = "";
        static bool s_DPInited = false;
        if (!s_DPInited) {
            std::string varDir = (const char*)tDirectories::Var().GetWritePath("demos/");
            snprintf(s_DPFilePath, sizeof(s_DPFilePath), "%s", varDir.c_str());
            s_DPInited = true;
        }

        ImGui::TextColored(ImVec4(0.55f,0.60f,0.70f,1.f), "File path (.aarec):");
        ImGui::PushItemWidth(bW - 90.f);
        ImGui::InputText("##dpFilePath", s_DPFilePath, sizeof(s_DPFilePath));
        ImGui::PopItemWidth();
        ImGui::SameLine();

        bool loaded = dp.IsLoaded();
        ImGui::PushStyleColor(ImGuiCol_Button,
            loaded ? ImVec4(0.20f,0.55f,0.85f,0.9f) : ImVec4(0.12f,0.45f,0.12f,0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            loaded ? ImVec4(0.30f,0.70f,1.00f,1.0f) : ImVec4(0.18f,0.65f,0.18f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f,0.30f,0.08f,1.f));
        if (ImGui::Button(loaded ? "Reload##dpOpen" : "Open##dpOpen", ImVec2(82.f, 22.f))) {
            if (strlen(s_DPFilePath) > 0) {
                if (dp.LoadFile(std::string(s_DPFilePath))) dp.Play();
            }
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();

        const auto& recent = DemoRecorder::Instance().RecentFiles();
        if (loaded) {
            int m_ = (int)(dp.TotalDuration()/60);
            int s_ = (int)(dp.TotalDuration()) % 60;
            ImGui::TextColored(ImVec4(0.4f,1.f,0.5f,1.f),
                "Loaded: %d rounds  |  %02d:%02d", dp.TotalRounds(), m_, s_);
        } else {
            ImGui::TextColored(ImVec4(0.45f,0.45f,0.52f,0.85f), "No demo loaded.");
        }

        if (!recent.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.55f,0.55f,0.65f,0.85f), "Recent (click to load):");
            ImGui::Spacing();
            size_t maxShow = std::min(recent.size(), (size_t)3);
            for (int ri = 0; ri < (int)maxShow; ++ri) {
                std::string base = recent[ri];
                size_t sl = base.find_last_of("/\\");
                if (sl != std::string::npos) base = base.substr(sl+1);
                if (base.size() > 6 && base.substr(base.size()-6) == ".aarec")
                    base = base.substr(0, base.size()-6);
                char btnLbl[96];
                snprintf(btnLbl, sizeof(btnLbl), "%s##rpick%d", base.c_str(), ri);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f,0.14f,0.20f,0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f,0.28f,0.45f,0.9f));
                if (ImGui::Button(btnLbl, ImVec2(bW, 22.f))) {
                    snprintf(s_DPFilePath, sizeof(s_DPFilePath), "%s", recent[ri].c_str());
                    dp.LoadFile(recent[ri]);
                    dp.Play();
                }
                ImGui::PopStyleColor(2);
            }
        }

        ImGui::PopStyleVar(); // FrameRounding
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

static void drawPanel(ImDrawList* dl, ImVec2 pos, ImVec2 size, bool active, const char* title) {
    if (pos.x < -500.0f) return; // Hidden panel
    ImU32 bgCol = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, 0.82f));
    ImU32 borderCol = active ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.16f, 0.7f));
    
    // Draw drop shadow glow if active
    if (active) {
        RenderTextureGlow(pos, size, (GetThemeColor(0.5f) & 0x00FFFFFF) | 0x22000000, 16.0f);
    } else {
        RenderTextureGlow(pos, size, IM_COL32(0, 0, 0, 45), 10.0f);
    }
    
    // Draw main panel background
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgCol, 12.0f);
    // Draw border
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderCol, 12.0f, 0, active ? 2.0f : 1.0f);
    
    // Panel Header text
    if (g_FontHeader) ImGui::PushFont(g_FontHeader);
    dl->AddText(ImVec2(pos.x + 20.0f, pos.y + 18.0f), GetThemeColor(0.2f), title);
    if (g_FontHeader) ImGui::PopFont();
    
    // Small panel accent line
    dl->AddRectFilledMultiColor(ImVec2(pos.x + 20.0f, pos.y + 42.0f), ImVec2(pos.x + size.x - 20.0f, pos.y + 44.0f),
                               GetThemeColor(0.0f), GetThemeColor(0.8f), GetThemeColor(0.8f), GetThemeColor(0.0f));
}

void RenderDemoRecorderTabContent(ImDrawList* dl, ImGuiIO& io,
                                  ImVec2 panel0Pos, ImVec2 panel0Size,
                                  ImVec2 panel1Pos, ImVec2 panel1Size,
                                  ImVec2 panel2Pos, ImVec2 panel2Size,
                                  bool hasPanel2) {
    DemoRecorder& rec  = DemoRecorder::Instance();
    const DemoRecorderStats& rstats = rec.Stats();
    bool isRec = rec.IsRecording();

    // Persistent config: auto-record & overlay toggle
    // (sg_demoRecorderOverlayEnabled declared in HudManager.h)
    static bool s_AutoRecord = false;
    static bool s_AutoInit   = false;
    if (!s_AutoInit) {
        static tConfItem<bool> conf_autoRecord("MOD_DEMO_AUTO_RECORD", s_AutoRecord);
        s_AutoInit = true;
    }

    // Filename state
    static char s_DemoFilename[512] = "";
    static bool s_FilenameInit = false;
    if (!s_FilenameInit) {
        std::time_t now2 = std::time(nullptr);
        std::tm*    lt2  = std::localtime(&now2);
        char tbuf[80];
        std::strftime(tbuf, sizeof(tbuf), "demo_%Y%m%d_%H%M%S.aarec", lt2);
        snprintf(s_DemoFilename, sizeof(s_DemoFilename), "%s", tbuf);
        s_FilenameInit = true;
    }

    // Dynamically calculate positions depending on whether we have 2 or 3 columns.
    // panel0Pos is ALWAYS the Sidebar Navigation, so we MUST NEVER draw over it.
    ImVec2 ctrlPos, ctrlSize;
    ImVec2 statsPos, statsSize;
    ImVec2 recentPos, recentSize;

    if (hasPanel2) {
        // 3 columns layout:
        // Left Column (panel0): NAVIGATION Sidebar
        // Middle Column (panel1): Controls (full height)
        // Right Column (panel2): Stats (top) and Recent Demos (bottom)
        ctrlPos = panel1Pos;
        ctrlSize = panel1Size;

        statsPos = panel2Pos;
        statsSize = ImVec2(panel2Size.x, 250.f);

        recentPos = ImVec2(panel2Pos.x, panel2Pos.y + 270.f);
        recentSize = ImVec2(panel2Size.x, panel2Size.y - 270.f);
    } else {
        // 2 columns layout:
        // Left Column (panel0): NAVIGATION Sidebar
        // Right Column (panel1): Split horizontally into Controls (left half) and Stats + Recent (right half stacked)
        float halfW = panel1Size.x * 0.5f - 10.f;

        ctrlPos = panel1Pos;
        ctrlSize = ImVec2(halfW, panel1Size.y);

        statsPos = ImVec2(panel1Pos.x + halfW + 20.f, panel1Pos.y);
        statsSize = ImVec2(halfW, 250.f);

        recentPos = ImVec2(panel1Pos.x + halfW + 20.f, panel1Pos.y + 270.f);
        recentSize = ImVec2(halfW, panel1Size.y - 270.f);
    }

    // ---- Controls Sub-Panel ----
    drawPanel(dl, ctrlPos, ctrlSize, true, "REC CONTROL");

    // Animated status dot in top-right of panel header
    {
        float t    = (float)ImGui::GetTime();
        float dotA = isRec ? (0.4f + 0.6f * ((sinf(t * 4.6f) + 1.f) * .5f)) : 0.22f;
        ImU32 glowCol = isRec
            ? IM_COL32(255, 45, 45, (uint8_t)(dotA * 55))
            : IM_COL32(0, 0, 0, 0);
        ImU32 dotCol  = isRec
            ? IM_COL32(255, 45, 45, (uint8_t)(dotA * 255))
            : IM_COL32(80, 80, 90,  (uint8_t)(dotA * 255));
        ImVec2 dotCenter(ctrlPos.x + ctrlSize.x - 22.f, ctrlPos.y + 30.f);
        dl->AddCircleFilled(dotCenter, isRec ? 9.f : 6.f, glowCol);
        dl->AddCircleFilled(dotCenter, isRec ? 5.5f : 4.f, dotCol);
    }

    ImGui::SetCursorScreenPos(ImVec2(ctrlPos.x + 14.f, ctrlPos.y + 54.f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (ImGui::BeginChild("##DRCtrl",
                          ImVec2(ctrlSize.x - 28.f, ctrlSize.y - 68.f),
                          false, ImGuiWindowFlags_NavFlattened)) {

        float btnW = ImGui::GetContentRegionAvail().x;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);

        // BIG REC / STOP button
        if (isRec) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.68f, 0.10f, 0.10f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.16f, 0.16f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.04f, 0.04f, 1.00f));
            if (ImGui::Button("##stop_rec", ImVec2(btnW, 56.f))) {
                rec.StopRecording();
                s_FilenameInit = false; // refresh timestamp on next frame
            }
            // Custom draw on the invisible button
            {
                ImVec2 bMin = ImGui::GetItemRectMin();
                ImVec2 bMax = ImGui::GetItemRectMax();
                float  midY = (bMin.y + bMax.y) * 0.5f;
                float  sqH  = 9.f;
                float  sqX  = bMin.x + 20.f + sqH;
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImVec2(sqX - sqH, midY - sqH),
                    ImVec2(sqX + sqH, midY + sqH),
                    IM_COL32(255,255,255,230), 3.f);
                ImGui::GetWindowDrawList()->AddText(
                    g_FontHeader ? g_FontHeader : ImGui::GetFont(),
                    ImGui::GetFontSize() * 1.05f,
                    ImVec2(sqX + sqH + 10.f, midY - ImGui::GetFontSize() * 0.5f),
                    IM_COL32(255,255,255,235), "STOP RECORDING");
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.58f, 0.20f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.78f, 0.28f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.04f, 0.38f, 0.08f, 1.00f));
            if (ImGui::Button("##start_rec", ImVec2(btnW, 56.f))) {
                std::string outPath = (const char*)tDirectories::Var().GetWritePath(s_DemoFilename);
                rec.StartRecording(outPath);
            }
            {
                ImVec2 bMin = ImGui::GetItemRectMin();
                ImVec2 bMax = ImGui::GetItemRectMax();
                float  midY = (bMin.y + bMax.y) * 0.5f;
                float  dotR = 8.5f;
                float  dotX = bMin.x + 20.f + dotR;
                ImGui::GetWindowDrawList()->AddCircleFilled(
                    ImVec2(dotX, midY), dotR * 1.8f, IM_COL32(60,220,80,40));
                ImGui::GetWindowDrawList()->AddCircleFilled(
                    ImVec2(dotX, midY), dotR, IM_COL32(80,255,100,235));
                ImGui::GetWindowDrawList()->AddText(
                    g_FontHeader ? g_FontHeader : ImGui::GetFont(),
                    ImGui::GetFontSize() * 1.05f,
                    ImVec2(dotX + dotR + 10.f, midY - ImGui::GetFontSize() * 0.5f),
                    IM_COL32(255,255,255,235), "START RECORDING");
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::PopStyleVar(); // FrameRounding

        ImGui::Spacing(); ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Output filename
        ImGui::TextColored(ImVec4(0.55f,0.55f,0.60f,1.f), "Output file:");
        ImGui::PushItemWidth(btnW);
        if (isRec) ImGui::BeginDisabled();
        ImGui::InputText("##demofilename", s_DemoFilename, sizeof(s_DemoFilename));
        if (isRec) ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::TextColored(ImVec4(0.38f,0.38f,0.43f,0.8f), "(saved to var dir)");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Auto-record checkbox
        ImGui::TextColored(ImVec4(0.55f,0.55f,0.60f,1.f), "Automation:");
        ImGui::Spacing();
        if (ImGui::Checkbox("Auto-record all matches", &s_AutoRecord)) { /* conf auto-saved */ }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f,0.35f,0.40f,0.8f), "(starts on round begin)");
        ImGui::Spacing();
        if (ImGui::Checkbox("Show HUD overlay (REC pill)", &sg_demoRecorderOverlayEnabled)) {}
        ImGui::Spacing();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // ---- Live Statistics Sub-Panel ----
    drawPanel(dl, statsPos, statsSize, false, "LIVE STATISTICS");
    {
        double   dur   = rstats.duration.load(std::memory_order_relaxed);
        int      dur_m = (int)(dur / 60.);
        int      dur_s = (int)(dur) % 60;
        uint64_t tot   = rstats.totalFrames.load(std::memory_order_relaxed);
        uint64_t drp   = rstats.droppedFrames.load(std::memory_order_relaxed);
        uint64_t byt   = rstats.bytesWritten.load(std::memory_order_relaxed);
        double   mb    = (double)byt / (1024. * 1024.);

        float sY = statsPos.y + 62.f;
        float sX = statsPos.x + 22.f;
        float W  = statsSize.x - 44.f;

        auto statCard = [&](const char* label, const char* val, ImU32 valCol) {
            dl->AddText(ImVec2(sX, sY), IM_COL32(110,110,125,220), label);
            ImVec2 vSize = ImGui::CalcTextSize(val);
            dl->AddText(ImVec2(statsPos.x + statsSize.x - 22.f - vSize.x, sY),
                        valCol, val);
            sY += ImGui::GetTextLineHeight() + 4.f;
            dl->AddLine(ImVec2(sX, sY), ImVec2(sX+W, sY), IM_COL32(30,30,36,180));
            sY += 10.f;
        };

        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%02d:%02d", dur_m, dur_s);
        statCard("Duration",  tmp, IM_COL32(255,255,255,245));

        snprintf(tmp, sizeof(tmp), "%llu", (unsigned long long)tot);
        statCard("Packets",   tmp, IM_COL32(160,210,255,235));

        if (mb >= 1.f)
            snprintf(tmp, sizeof(tmp), "%.2f MB", mb);
        else
            snprintf(tmp, sizeof(tmp), "%llu KB", (unsigned long long)(byt/1024));
        statCard("Written",   tmp, IM_COL32(120,255,160,235));

        ImU32 drpCol = drp > 0 ? IM_COL32(255,80,80,245) : IM_COL32(80,80,95,200);
        snprintf(tmp, sizeof(tmp), "%llu", (unsigned long long)drp);
        statCard("Dropped",   tmp, drpCol);

        // Ring buffer health bar
        sY += 6.f;
        dl->AddText(ImVec2(sX, sY), IM_COL32(110,110,125,220), "Ring buffer:");
        sY += ImGui::GetTextLineHeight() + 4.f;

        float healthFill = (drp == 0) ? 1.0f : std::max(0.f, 1.f - (float)drp / 512.f);
        ImVec2 barMin(sX, sY);
        ImVec2 barMax(sX + W, sY + 8.f);
        dl->AddRectFilled(barMin, barMax, IM_COL32(20,20,24,200), 4.f);
        if (healthFill > 0.f) {
            ImU32 fillCol = drp > 0 ? IM_COL32(255,60,60,220) : IM_COL32(60,210,100,220);
            dl->AddRectFilled(barMin,
                ImVec2(barMin.x + W * healthFill, barMax.y), fillCol, 4.f);
        }
        dl->AddRect(barMin, barMax, IM_COL32(50,50,60,200), 4.f);
        sY += 16.f;

        const char* healthStr = drp == 0 ? "OK - no drops" : "OVERFLOW";
        ImU32 healthCol = drp == 0 ? IM_COL32(80,220,110,220) : IM_COL32(255,80,80,220);
        dl->AddText(ImVec2(sX, sY), healthCol, healthStr);
        sY += ImGui::GetTextLineHeight() + 14.f;

        if (!rec.CurrentFilename().empty()) {
            dl->AddLine(ImVec2(sX, sY), ImVec2(sX+W, sY), IM_COL32(30,30,36,180));
            sY += 10.f;
            dl->AddText(ImVec2(sX, sY), IM_COL32(110,110,125,220), "Current file:");
            sY += ImGui::GetTextLineHeight() + 4.f;
            std::string cf = rec.CurrentFilename();
            size_t slash = cf.find_last_of("/\\");
            std::string base = (slash != std::string::npos) ? cf.substr(slash+1) : cf;
            dl->AddText(ImVec2(sX + 6.f, sY), IM_COL32(130,200,255,220), base.c_str());
        }
    }

    // ---- Recent Recordings Sub-Panel ----
    drawPanel(dl, recentPos, recentSize, false, "RECENT DEMOS");
    const auto& recent = rec.RecentFiles();
    float listY = recentPos.y + 60.f;
    float listH = recentSize.y - 70.f;

    ImGui::SetCursorScreenPos(ImVec2(recentPos.x + 10.f, listY));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    if (ImGui::BeginChild("##DRRecent",
                          ImVec2(recentSize.x - 20.f, listH),
                          false, ImGuiWindowFlags_NavFlattened)) {

        if (recent.empty()) {
            ImGui::SetCursorPosY(30.f);
            ImGui::TextColored(ImVec4(0.38f,0.38f,0.43f,0.8f), "No recordings yet.");
            ImGui::TextColored(ImVec4(0.30f,0.30f,0.35f,0.7f), "Press START RECORDING");
            ImGui::TextColored(ImVec4(0.30f,0.30f,0.35f,0.7f), "to capture a match.");
        } else {
            ImDrawList* rDl = ImGui::GetWindowDrawList();
            for (int ri = 0; ri < (int)recent.size(); ri++) {
                const std::string& rpath = recent[ri];
                std::string base = rpath;
                size_t slash = rpath.find_last_of("/\\");
                if (slash != std::string::npos) base = rpath.substr(slash + 1);
                // Strip .aarec extension
                if (base.size() > 6 && base.substr(base.size()-6) == ".aarec")
                    base = base.substr(0, base.size()-6);

                ImVec2 cMin = ImGui::GetCursorScreenPos();
                float  cW   = recentSize.x - 20.f;
                float  cH   = 60.f;
                ImVec2 cMax(cMin.x + cW, cMin.y + cH - 5.f);

                bool rHov = io.MousePos.x >= cMin.x && io.MousePos.x <= cMax.x &&
                            io.MousePos.y >= cMin.y && io.MousePos.y <= cMax.y;

                rDl->AddRectFilled(cMin, cMax,
                    rHov ? IM_COL32(22,22,30,230) : IM_COL32(14,14,18,180), 6.f);
                rDl->AddRectFilled(
                    ImVec2(cMin.x, cMin.y), ImVec2(cMin.x + 3.f, cMax.y),
                    GetThemeColor(0.05f * ri), 2.f);
                rDl->AddRect(cMin, cMax,
                    rHov ? IM_COL32(60,60,80,200) : IM_COL32(30,30,40,120), 6.f);

                char idxBuf[16];
                snprintf(idxBuf, sizeof(idxBuf), "#%d", ri + 1);
                rDl->AddText(ImVec2(cMin.x + 10.f, cMin.y + 8.f),
                             IM_COL32(80,80,100,200), idxBuf);
                rDl->AddText(ImVec2(cMin.x + 32.f, cMin.y + 8.f),
                             IM_COL32(210,220,235,245), base.c_str());

                std::string shortPath = rpath;
                if (shortPath.size() > 38)
                    shortPath = "..." + shortPath.substr(shortPath.size() - 35);
                rDl->AddText(ImVec2(cMin.x + 10.f, cMin.y + 30.f),
                             IM_COL32(70,70,90,180), shortPath.c_str());

                ImGui::Dummy(ImVec2(cW, cH));
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
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
    
    if (getenv("RETRO_TEST_START_GAME")) {
        s_PendingStartLocalGame = true;
    }
    if (getenv("RETRO_TEST_PLAY_DEMO")) {
        unsetenv("RETRO_TEST_PLAY_DEMO");
        const char* path_env = getenv("RETRO_TEST_PLAY_DEMO_PATH");
        std::string dp_path = path_env ? path_env : "1.aarec";
        if (DemoPlayerManager::Instance().LoadFile(dp_path)) {
            DemoPlayerManager::Instance().Play();
            s_PendingStartDemoPlayback = true;
        }
    }

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
        if (s_PendingStartDemoPlayback) {
            s_PendingStartDemoPlayback = false;
            StartDemoPlaybackHelper();
            continue;
        }
        if (uMenu::exitToMain) {
            uMenu::exitToMain = false;
        }
        SDL_ShowCursor();
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
            static struct timeval last_tv = {0, 0};
            struct timeval tv;
            gettimeofday(&tv, NULL);
            if (last_tv.tv_sec != 0) {
                double dt = (double)(tv.tv_sec - last_tv.tv_sec) + (double)(tv.tv_usec - last_tv.tv_usec) / 1000000.0;
                if (dt > 0.0) {
                    io.DeltaTime = (float)dt;
                    last_tv = tv;
                }
            } else {
                last_tv = tv;
            }

            SDL_ShowCursor();
            io.MouseDrawCursor = true;

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
            bgDl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.65f)));
            DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);

            HudManager::Update(io.DeltaTime);
            HudManager::Render();

            if (sr_glOut) {
                sr_ResetRenderState(true);
                gLogo::Display();
            }
            
            ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            
            rSysDep::SwapGL();
            rSysDep::ClearGL();
            continue;
        }
        
        // Setup ImGui frames
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        static struct timeval last_tv = {0, 0};
        struct timeval tv;
        gettimeofday(&tv, NULL);
        if (last_tv.tv_sec != 0) {
            double dt = (double)(tv.tv_sec - last_tv.tv_sec) + (double)(tv.tv_usec - last_tv.tv_usec) / 1000000.0;
            if (dt > 0.0) {
                io.DeltaTime = (float)dt;
                last_tv = tv;
            }
        } else {
            last_tv = tv;
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
        
        // Fullscreen Dashboard Window
        ImVec2 cardSize = io.DisplaySize;
        ImVec2 cardPos(0.0f, 0.0f);
        ImGui::SetNextWindowPos(cardPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(cardSize, ImGuiCond_Always);
        ImGuiWindowFlags cardFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
                                     
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
        
        // Beautiful gradient separator line below header
        dl->AddRectFilledMultiColor(ImVec2(winPos.x + paddingX, winPos.y + headerY), ImVec2(winPos.x + cardSize.x - paddingX, winPos.y + headerY + 3.0f),
                                   GetThemeColor(0.0f), GetThemeColor(1.0f), GetThemeColor(1.0f), GetThemeColor(0.0f));
        
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
        auto drawPanel = [&](ImVec2 pos, ImVec2 size, bool active, const char* title) {
            if (pos.x < -500.0f) return; // Hidden panel
            ImU32 bgCol = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, 0.82f));
            ImU32 borderCol = active ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.16f, 0.7f));
            
            // Draw drop shadow glow if active
            if (active) {
                RenderTextureGlow(pos, size, (GetThemeColor(0.5f) & 0x00FFFFFF) | 0x22000000, 16.0f);
            } else {
                RenderTextureGlow(pos, size, IM_COL32(0, 0, 0, 45), 10.0f);
            }
            
            // Draw main panel background
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgCol, 12.0f);
            // Draw border
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderCol, 12.0f, 0, active ? 2.0f : 1.0f);
            
            // Panel Header text
            if (g_FontHeader) ImGui::PushFont(g_FontHeader);
            dl->AddText(ImVec2(pos.x + 20.0f, pos.y + 18.0f), GetThemeColor(0.2f), title);
            if (g_FontHeader) ImGui::PopFont();
            
            // Small panel accent line
            dl->AddRectFilledMultiColor(ImVec2(pos.x + 20.0f, pos.y + 42.0f), ImVec2(pos.x + size.x - 20.0f, pos.y + 44.0f),
                                       GetThemeColor(0.0f), GetThemeColor(0.8f), GetThemeColor(0.8f), GetThemeColor(0.0f));
        };
        
        ImVec2 panelSize = panel1Size; // For compatibility

        
        // --- COLUMN 0: NAVIGATION (Left) ---
        drawPanel(panel0Pos, panel0Size, g_DashboardActiveCol == 0, "NAVIGATION");
        
        const char* navLabels[8] = { "Dashboard", "Local Play", "Server Browser", "Settings", "Mod Menu", "Demo Recorder", "Demo Player", "Exit Game" };
        
        ImGui::SetCursorScreenPos(ImVec2(panel0Pos.x + 5.0f, panel0Pos.y + 55.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        bool showNav = ImGui::BeginChild("##NavigationScroll", ImVec2(panel0Size.x - 10.0f, panel0Size.y - 70.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NavFlattened);
        if (showNav) {
            ImDrawList* navDl = ImGui::GetWindowDrawList();
            ImVec2 navStart = ImGui::GetCursorScreenPos();
            
            for (int i = 0; i < 8; i++) {
                float itemY = navStart.y + i * 55.0f;
                float itemH = 45.0f;
                ImVec2 itemMin(panel0Pos.x + 20.0f, itemY);
                ImVec2 itemMax(panel0Pos.x + panel0Size.x - 20.0f, itemY + itemH);

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
                bool isActiveTab = (g_ActiveTab == i);
                
                ImU32 itemBg = (selected || hovered || isActiveTab) ? GetThemeColor(0.1f * i) : ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.12f, 0.6f));
                ImU32 itemBorder = (selected || isActiveTab) ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 0.5f));
                
                navDl->AddRectFilled(itemMin, itemMax, itemBg, 8.0f);
                navDl->AddRect(itemMin, itemMax, itemBorder, 8.0f, 0, (selected || isActiveTab) ? 1.5f : 1.0f);
                
                // Draw neon indicator dot on active selected item
                if (selected) {
                    navDl->AddCircleFilled(ImVec2(itemMin.x + 15.0f, itemMin.y + itemH * 0.5f), 4.0f, GetThemeColor(0.5f));
                }
                
                // Draw text
                navDl->AddText(ImVec2(itemMin.x + (selected ? 30.0f : 20.0f), itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                            IM_COL32(255, 255, 255, 255), navLabels[i]);
                
                if (g_DashboardActionTriggered && selected) {
                    g_DashboardActionTriggered = false;
                    if (i == 4) { // Mod Menu (toggle)
                        if (g_ActiveTab == 4) {
                            g_ActiveTab = 0;
                            g_DashboardLeftSelected = 0;
                        } else {
                            g_ActiveTab = 4;
                        }
                    } else if (i == 5) { // Demo Recorder
                        g_ActiveTab = 5;
                    } else if (i == 6) { // Demo Player
                        g_ActiveTab = 6;
                    } else if (i == 7) { // Exit Game
                        uMenu::quickexit = uMenu::QuickExit_Total;
                    } else {
                        g_ActiveTab = i;
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (g_ActiveTab == 0) {
            // --- TAB 0: DASHBOARD ---
            static int s_DashSubTab = 0; // 0: Info, 1: Profiles
            
            if (!hasPanel2) {
                // Render sub-tabs at the top of Panel 1
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
                // --- COLUMN 1: SYSTEM INFORMATION (Middle) ---
                drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, "SYSTEM INFORMATION");
                
                // Profile Info
                const char* rawPlayerName = "Local Player";
                ePlayer* lp = ePlayer::PlayerConfig(0);
                if (lp) {
                    rawPlayerName = static_cast<const char*>(lp->Name());
                }
                
                const char* connState = "Singleplayer";
                if (sn_GetNetState() == nSERVER) {
                    connState = "Hosting Server";
                } else if (sn_GetNetState() == nCLIENT) {
                    connState = "Multiplayer Client";
                } else if (uMenu::MenuActive()) {
                    connState = "Main Menu";
                }
                
                // Draw avatar at the top of Middle Column
                ImVec2 avatarPos(panel1Pos.x + 25.0f, panel1Pos.y + 60.0f + offsetDashY);
                if (g_AvatarTexture == 0) {
                    LoadAvatarTexture();
                }
                float avatarSize = 50.0f;
                if (g_AvatarTexture != 0) {
                    dl->AddImageRounded((ImTextureID)(intptr_t)g_AvatarTexture, avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImVec2(0,0), ImVec2(1,1), IM_COL32(255,255,255,255), 8.0f);
                    dl->AddRect(avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.22f, 0.8f)), 8.0f, 0, 1.0f);
                } else {
                    dl->AddRectFilled(avatarPos, ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 1.0f)), 8.0f);
                    dl->AddText(ImVec2(avatarPos.x + avatarSize*0.5f - 5.0f, avatarPos.y + avatarSize*0.5f - 8.0f + offsetDashY), IM_COL32(255,255,255,255), "R");
                }
                
                // Name and state text next to avatar
                RenderColoredText(dl, ImVec2(avatarPos.x + avatarSize + 15.0f, avatarPos.y + 6.0f), IM_COL32(255,255,255,255), rawPlayerName);
                dl->AddText(ImVec2(avatarPos.x + avatarSize + 15.0f, avatarPos.y + 26.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)), connState);
                
                float infoY = panel1Pos.y + 130.0f + offsetDashY;
                auto drawInfoRow = [&](const char* label, const char* val, ImU32 valCol = IM_COL32(255, 255, 255, 255)) {
                    dl->AddText(ImVec2(panel1Pos.x + 25.0f, infoY), ImGui::GetColorU32(ImVec4(0.55f, 0.55f, 0.60f, 1.0f)), label);
                    dl->AddText(ImVec2(panel1Pos.x + panel1Size.x - 25.0f - ImGui::CalcTextSize(val).x, infoY), valCol, val);
                    
                    // Subtle horizontal row divider
                    dl->AddLine(ImVec2(panel1Pos.x + 20.0f, infoY + 28.0f), ImVec2(panel1Pos.x + panel1Size.x - 20.0f, infoY + 28.0f),
                                ImGui::GetColorU32(ImVec4(0.11f, 0.11f, 0.13f, 0.4f)));
                    infoY += 40.0f;
                };
                
                char fpsBuf[32];
                snprintf(fpsBuf, sizeof(fpsBuf), "%.1f FPS", io.Framerate);
                
                drawInfoRow("System State:", connState);
                drawInfoRow("Performance Rate:", fpsBuf, io.Framerate > 55.0f ? IM_COL32(100, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
                
                std::string glVendor = (const char*)glGetString(GL_VENDOR);
                std::string glRenderer = (const char*)glGetString(GL_RENDERER);
                if (glVendor.length() > 60) glVendor = glVendor.substr(0, 60) + "...";
                if (glRenderer.length() > 60) glRenderer = glRenderer.substr(0, 60) + "...";
                
                drawInfoRow("GPU Vendor:", glVendor.c_str());
                drawInfoRow("GPU Renderer:", glRenderer.c_str());
                
                // interactive buttons in middle column
                const char* midLabels[2] = { "Refresh Diagnostics", "Toggle FPS Overlay" };
                for (int i = 0; i < 2; i++) {
                    float itemY = infoY + 20.0f + i * 65.0f;
                    float itemH = 50.0f;
                    ImVec2 itemMin(panel1Pos.x + 20.0f, itemY);
                    ImVec2 itemMax(panel1Pos.x + panel1Size.x - 20.0f, itemY + itemH);
                    
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
                    
                    // If we are currently flashing diagnostics green, override bg color
                    if (i == 0 && diagFlashTimer > 0.0f) {
                        itemBg = ImGui::GetColorU32(ImVec4(0.1f, 0.6f, 0.2f, 0.8f * (diagFlashTimer / 0.5f)));
                    }
                    
                    // If FPS is on, highlight the toggle button slightly
                    if (i == 1 && sr_FPSOut) {
                        itemBg = ImGui::GetColorU32(ImVec4(g_AccentColor.x * 0.2f, g_AccentColor.y * 0.2f, g_AccentColor.z * 0.2f, 0.6f));
                    }
                    
                    dl->AddRectFilled(itemMin, itemMax, itemBg, 8.0f);
                    dl->AddRect(itemMin, itemMax, itemBorder, 8.0f, 0, selected ? 1.5f : 1.0f);
                    
                    if (selected) {
                        dl->AddCircleFilled(ImVec2(itemMin.x + 15.0f, itemMin.y + itemH * 0.5f), 4.0f, GetThemeColor(0.5f));
                    }
                    
                    dl->AddText(ImVec2(itemMin.x + (selected ? 30.0f : 20.0f), itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                                IM_COL32(255, 255, 255, 255), midLabels[i]);
                    
                    // Draw status for FPS Overlay
                    if (i == 1) {
                        const char* statusStr = sr_FPSOut ? "ON" : "OFF";
                        ImVec2 sSize = ImGui::CalcTextSize(statusStr);
                        dl->AddText(ImVec2(itemMax.x - 20.0f - sSize.x, itemMin.y + (itemH - ImGui::GetTextLineHeight()) * 0.5f),
                                    sr_FPSOut ? IM_COL32(100, 255, 100, 255) : IM_COL32(255, 100, 100, 255), statusStr);
                    }
                    
                    if (g_DashboardActionTriggered && selected) {
                        g_DashboardActionTriggered = false;
                        if (i == 0) { // Refresh
                            LoadDashboardConfigs();
                            diagFlashTimer = 0.5f;
                        } else if (i == 1) { // Toggle FPS
                            sr_FPSOut = !sr_FPSOut;
                        }
                    }
                }
            }
            
            if (hasPanel2 || s_DashSubTab == 1) {
                ImVec2 targetPos = hasPanel2 ? panel2Pos : panel1Pos;
                ImVec2 targetSize = hasPanel2 ? panel2Size : panel1Size;
                
                // --- COLUMN 2: CONFIG PROFILES (Right) ---
                drawPanel(targetPos, targetSize, g_DashboardActiveCol == 2, "CONFIG PROFILES");
                
                // Clamp selection to actual configs count
                int numConfigs = (int)s_DashboardConfigs.size();
                if (numConfigs > 0) {
                    if (g_DashboardRightSelected < 0) g_DashboardRightSelected = numConfigs;
                    if (g_DashboardRightSelected > numConfigs) g_DashboardRightSelected = 0;
                } else {
                    g_DashboardRightSelected = 0;
                }
                
                // Scroll view boundaries
                float listStartY = targetPos.y + 65.0f + offsetDashY;
                float visibleHeight = targetSize.y - 65.0f - 45.0f - 60.0f - offsetDashY;
                float itemHeight = 55.0f;
                
                // Scroll target calculation
                if (numConfigs > 0 && g_DashboardRightSelected < numConfigs) {
                    float selectedY = g_DashboardRightSelected * itemHeight;
                    if (selectedY < targetScrollY) {
                        targetScrollY = selectedY;
                    } else if (selectedY + itemHeight > targetScrollY + visibleHeight) {
                        targetScrollY = selectedY + itemHeight - visibleHeight;
                    }
                }
                currentScrollY += (targetScrollY - currentScrollY) * 0.2f;
                
                float totalHeightOfItems = numConfigs * itemHeight;
                if (totalHeightOfItems > visibleHeight) {
                    float scrollbarX = targetPos.x + targetSize.x - 12.0f;
                    float scrollbarY = listStartY;
                    float scrollbarH = visibleHeight;
                    dl->AddRectFilled(ImVec2(scrollbarX, scrollbarY), ImVec2(scrollbarX + 4.0f, scrollbarY + scrollbarH), ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.5f)), 2.0f);
                    
                    float thumbH = std::max(20.0f, (visibleHeight / totalHeightOfItems) * scrollbarH);
                    if (thumbH > scrollbarH) thumbH = scrollbarH;
                    float thumbY = scrollbarY + (currentScrollY / totalHeightOfItems) * scrollbarH;
                    dl->AddRectFilled(ImVec2(scrollbarX, thumbY), ImVec2(scrollbarX + 4.0f, thumbY + thumbH), GetThemeColor(0.5f), 2.0f);
                }
                
                // Push clipping rect to prevent items from rendering outside scroll view
                dl->PushClipRect(ImVec2(targetPos.x, listStartY), ImVec2(targetPos.x + targetSize.x - 15.0f, listStartY + visibleHeight), true);
                
                if (numConfigs == 0) {
                    dl->AddText(ImVec2(targetPos.x + 25.0f, listStartY + 20.0f), ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.45f, 0.8f)), "No custom profiles found.");
                    dl->AddText(ImVec2(targetPos.x + 25.0f, listStartY + 40.0f), ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.45f, 0.8f)), "Create one using '+ Create New'.");
                } else {
                    for (int i = 0; i < numConfigs; i++) {
                        float itemY = listStartY + i * itemHeight - currentScrollY;
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
                
                // "+ Create New Profile" button at bottom of Right Column
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
                
                ImU32 createBg = (createSelected || createHovered) ? ImGui::GetColorU32(ImVec4(0.12f, 0.45f, 0.20f, 0.8f)) : ImGui::GetColorU32(ImVec4(0.08f, 0.25f, 0.12f, 0.6f));
                ImU32 createBorder = createSelected ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.12f, 0.40f, 0.16f, 0.5f));
                
                dl->AddRectFilled(createMin, createMax, createBg, 8.0f);
                dl->AddRect(createMin, createMax, createBorder, 8.0f, 0, createSelected ? 1.5f : 1.0f);
                
                const char* createLabel = "+ Create New Profile";
                ImVec2 labelSize = ImGui::CalcTextSize(createLabel);
                dl->AddText(ImVec2(createMin.x + (createMax.x - createMin.x - labelSize.x) * 0.5f, createMin.y + (createH - labelSize.y) * 0.5f),
                            IM_COL32(255, 255, 255, 255), createLabel);
                if (g_DashboardActionTriggered && createSelected) {
                    g_DashboardActionTriggered = false;
                    g_NewConfigName[0] = '\0';
                    g_OpenCreateConfigPopup = true;
                }
            }
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
            
            // 1. Game Mode
            const char* modeNames[] = { "Free For All", "Duel", "Human vs AI" };
            int currentMode = (int)singlePlayer.gameType;
            ImGui::Text("Game Mode:");
            ImGui::SetNextItemWidth(width1 - 50.0f);
            if (ImGui::Combo("##SPMode", &currentMode, modeNames, 3)) {
                singlePlayer.gameType = (gGameType)currentMode;
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
                ImGui::InputInt("##SPScoreLimit", &singlePlayer.limitScore);
                
                ImGui::Spacing(); ImGui::Spacing();
                
                // 5. Round Limit
                ImGui::Text("Match Round Limit:");
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputInt("##SPRoundLimit", &singlePlayer.limitRounds);
                
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
            ImGui::SameLine(0.0f, 15.0f);
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
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 20.0f, panel1Pos.y + 80.0f));
            ImGui::PushItemWidth(width1 - 110.0f);
            ImGui::InputTextWithHint("##Search", "Search servers...", s_ServerSearchBuf, sizeof(s_ServerSearchBuf));
            ImGui::PopItemWidth();
            if (s_ServerSearchBuf[0] != '\0') {
                ImGui::SameLine(0.0f, 8.0f);
                if (ImGui::Button("Clear", ImVec2(50.0f, 0.0f))) {
                    s_ServerSearchBuf[0] = '\0';
                }
            }
            
            // Server List Scrollable Child Area
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 115.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("ServerListScroll", ImVec2(width1 - 30.0f, colH - 135.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImDrawList* child_dl = ImGui::GetWindowDrawList();
            child_dl->PushClipRect(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 115.0f), ImVec2(panel1Pos.x + width1 - 15.0f, panel1Pos.y + colH - 20.0f), true);
            
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
                    float dcStartY = panel2Pos.y + colH - 180.0f;
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
                
                float dcStartY = panel2Pos.y + colH - 180.0f;
                dl->AddLine(ImVec2(panel2Pos.x + 20.0f, dcStartY), ImVec2(panel2Pos.x + width2 - 20.0f, dcStartY), ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.25f, 0.6f)));
                dl->AddText(ImVec2(panel2Pos.x + 25.0f, dcStartY + 5.0f), GetThemeColor(0.3f), "DIRECT CONNECT");
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 25.0f));
                ImGui::Text("IP Address:");
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 45.0f));
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputText("##DirectIP", s_DirectIP, sizeof(s_DirectIP));
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 80.0f));
                ImGui::Text("Port:");
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 25.0f, dcStartY + 100.0f));
                ImGui::SetNextItemWidth(width2 - 50.0f);
                ImGui::InputInt("##DirectPort", &s_DirectPort);
                
                ImGui::SetCursorScreenPos(ImVec2(panel2Pos.x + 20.0f, dcStartY + 135.0f));
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
            
            struct KeybindItem {
                const char* label;
                const char* actionName;
                uAction* act;
            };
            static KeybindItem binds[] = {
                { "Turn Left", "CYCLE_TURN_LEFT", nullptr },
                { "Turn Right", "CYCLE_TURN_RIGHT", nullptr },
                { "Brake", "CYCLE_BRAKE", nullptr },
                { "Glance Left", "GLANCE_LEFT", nullptr },
                { "Glance Right", "GLANCE_RIGHT", nullptr },
                { "Glance Back", "GLANCE_BACK", nullptr },
                { "Switch View", "SWITCH_VIEW", nullptr },
                { "Chat / Say", "CHAT", nullptr },
                { "Team Chat", "CHAT_TEAM", nullptr },
                { "Toggle Console", "CONSOLE_TOGGLE", nullptr },
                { "Take Screenshot", "SCREENSHOT", nullptr },
                { "Pause Game", "PAUSE", nullptr },
                { "Perfect Turn Left", "CYCLE_PERFECT_LEFT", nullptr },
                { "Perfect Turn Right", "CYCLE_PERFECT_RIGHT", nullptr },
                { "AutoEscape Left", "CYCLE_AUTOESCAPE_LEFT", nullptr },
                { "AutoEscape Right", "CYCLE_AUTOESCAPE_RIGHT", nullptr }
            };
            
            for (auto& b : binds) {
                if (!b.act) {
                    b.act = uAction::Find(b.actionName);
                }
            }
            
            auto getBindsString = [](uAction* act) -> std::string {
                if (!act) return "NONE";
                std::string res = "";
                for (int sym = 0; sym < SDLK_NEWLAST; ++sym) {
                    if (keymap[sym] && keymap[sym]->act == act) {
                        if (!res.empty()) res += " / ";
                        res += GetScancodeName(sym);
                    }
                }
                return res.empty() ? "NONE" : res;
            };
            
            drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, hasPanel2 ? "PROFILE & VIDEO" : "PROFILE, VIDEO & KEYS");
            
            ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, panel1Pos.y + 60.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("GeneralSettingsScroll", ImVec2(width1 - 30.0f, colH - 80.0f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NavFlattened);
            
            // --- SECTION 1: PLAYER PROFILE ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "PLAYER PROFILE");
            ImGui::Separator();
            ImGui::Spacing();
            
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
            
            ImGui::Text("Nickname:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::InputText("##Nickname", s_NickBuf, sizeof(s_NickBuf))) {
                if (lp) {
                    lp->name = Utf8ToCp1251(s_NickBuf);
                }
            }
            
            ImGui::Text("Global ID:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::InputText("##GlobalID", s_GlobalIDBuf, sizeof(s_GlobalIDBuf))) {
                if (lp) {
                    lp->globalID = Utf8ToCp1251(s_GlobalIDBuf);
                }
            }
            
            if (lp) {
                int colR = lp->rgb[0];
                int colG = lp->rgb[1];
                int colB = lp->rgb[2];
                
                ImGui::Text("Color Red:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::SliderInt("##ColorRed", &colR, 0, 255)) {
                    lp->rgb[0] = colR;
                }
                
                ImGui::Text("Color Green:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::SliderInt("##ColorGreen", &colG, 0, 255)) {
                    lp->rgb[1] = colG;
                }
                
                ImGui::Text("Color Blue:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::SliderInt("##ColorBlue", &colB, 0, 255)) {
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
            
            ImGui::Checkbox("Fullscreen", &currentScreensetting.fullscreen);
            
            ImGui::Checkbox("Grab Mouse", &su_mouseGrab);
            
            const char* vsyncModes[] = { "On", "Default", "Off", "Motion Blur" };
            int vsyncIdx = (int)currentScreensetting.vSync;
            ImGui::Text("VSync Mode:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::Combo("##VSyncMode", &vsyncIdx, vsyncModes, IM_ARRAYSIZE(vsyncModes))) {
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
            ImGui::Text("Resolution:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::Combo("##Resolution", &resIdx, resNames, IM_ARRAYSIZE(resNames))) {
                currentScreensetting.res.res = (rResolution)resIdx;
                currentScreensetting.res.UpdateSize();
            }
            
            ImGui::Text("Max FPS (0 for Off):");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            ImGui::InputInt("##MaxFPS", &sr_maxFPS);
            
            const char* depthNames[] = { "16-bit", "Desktop", "32-bit" };
            int zDepthIdx = (int)currentScreensetting.zDepth;
            ImGui::Text("Z-Depth:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::Combo("##ZDepth", &zDepthIdx, depthNames, IM_ARRAYSIZE(depthNames))) {
                currentScreensetting.zDepth = (rColorDepth)zDepthIdx;
            }
            
            int colDepthIdx = (int)currentScreensetting.colorDepth;
            ImGui::Text("Color Depth:");
            ImGui::SetNextItemWidth(width1 - 55.0f);
            if (ImGui::Combo("##ColorDepth", &colDepthIdx, depthNames, IM_ARRAYSIZE(depthNames))) {
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
                ImGui::Text("Field of View (FOV):");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                ImGui::SliderInt("##FOV", &lp->startFOV, 30, 160);
                
                ImGui::Checkbox("Smart Glance Custom", &lp->smartCustomGlance);
                ImGui::Checkbox("Auto-Switch In-Cam", &lp->autoSwitchIncam);
                ImGui::Checkbox("Wobble In-Cam", &lp->wobbleIncam);
                ImGui::Checkbox("Center In-Cam on Turn", &lp->centerIncamOnTurn);
                
                ImGui::Checkbox("Allow Smart Camera", &lp->allowCam[CAMERA_SMART]);
                ImGui::Checkbox("Allow Follow Camera", &lp->allowCam[CAMERA_FOLLOW]);
                ImGui::Checkbox("Allow Free Camera", &lp->allowCam[CAMERA_FREE]);
                ImGui::Checkbox("Allow Custom Camera", &lp->allowCam[CAMERA_CUSTOM]);
                ImGui::Checkbox("Allow Server Custom Camera", &lp->allowCam[CAMERA_SERVER_CUSTOM]);
                ImGui::Checkbox("Allow In-Cam", &lp->allowCam[CAMERA_IN]);
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 4: GRAPHICS QUALITY ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "GRAPHICS QUALITY");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Checkbox("Alpha Blending", &sr_alphaBlend);
            ImGui::Checkbox("Smooth Shading", &sr_smoothShading);
            ImGui::Checkbox("Dither", &sr_dither);
            ImGui::Checkbox("High Rim", &sr_highRim);
            ImGui::Checkbox("Keep Window Active", &sr_keepWindowActive);
            
            {
                const char* floorDetailNames[] = { "Off", "Grid", "Texture", "Double Texture" };
                int floorDetailVal = sr_floorDetail;
                if (floorDetailVal < 0) floorDetailVal = 0;
                if (floorDetailVal > 3) floorDetailVal = 3;
                ImGui::Text("Floor Detail:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::Combo("##FloorDetailCombo", &floorDetailVal, floorDetailNames, 4)) {
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
                ImGui::Text("Floor Mirror:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::Combo("##FloorMirrorCombo", &floorMirrorIdx, floorMirrorNames, 4)) {
                    if (floorMirrorIdx == 0) sr_floorMirror = rMIRROR_OFF;
                    else if (floorMirrorIdx == 1) sr_floorMirror = rMIRROR_OBJECTS;
                    else if (floorMirrorIdx == 2) sr_floorMirror = rMIRROR_WALLS;
                    else if (floorMirrorIdx == 3) sr_floorMirror = rMIRROR_ALL;
                }
            }
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // --- SECTION 5: SKY & ENVIRONMENT ---
            ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "SKY & ENVIRONMENT");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Checkbox("Upper Sky", &sr_upperSky);
            ImGui::Checkbox("Lower Sky", &sr_lowerSky);
            ImGui::Checkbox("Sky Wobble", &sr_skyWobble);
            ImGui::Checkbox("Infinity Plane", &sr_infinityPlane);
            ImGui::Checkbox("Predict Objects", &sr_predictObjects);
            ImGui::Checkbox("Truecolor Textures", &sr_texturesTruecolor);
            ImGui::Checkbox("Lag-O-Meter", &sr_laggometer);
            ImGui::Checkbox("Axes Indicator", &sg_axesIndicator);
            ImGui::Checkbox("Use Moviepack (Custom Walls/Sky/Floor)", &sg_moviepackUse);
            
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
                ImGui::Text("Sound Quality:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::Combo("##SoundQualityCombo", &soundQual, soundQualityNames, 4)) {
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
                ImGui::Text("Audio Buffer Size:");
                ImGui::SetNextItemWidth(width1 - 55.0f);
                if (ImGui::Combo("##AudioBufferCombo", &bufShiftIdx, bufferShiftNames, 5)) {
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
            if (ImGui::Button("DISPLAY & VIDEO SETTINGS (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { sg_screenMenu.Enter(); };
            }
            ImGui::Spacing();
            if (ImGui::Button("AUDIO & SOUND SETTINGS (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { se_SoundMenu(); };
            }
            ImGui::Spacing();
            if (ImGui::Button("PLAYER PROFILE & VIEWPORTS (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { sg_PlayerMenu(); };
            }
            ImGui::Spacing();
            if (ImGui::Button("GLOBAL CYCLE INPUT CONFIG (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { su_InputConfigGlobal(); };
            }
            ImGui::Spacing();
            if (ImGui::Button("PLAYER 1 CONTROLS CONFIG (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { su_InputConfig(0); };
            }
            ImGui::Spacing();
            if (ImGui::Button("PLAYER 1 CAMERA CONFIG (LEGACY)", ImVec2(width1 - 55.0f, 30.0f))) {
                g_PendingLegacyMenuAction = []() { su_InputConfigCamera(0); };
            }
            
            if (!hasPanel2) {
                // If single column layout, stack the Keybindings directly inside the scroll pane
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "KEYBINDINGS");
                ImGui::Spacing();
                
                for (auto& b : binds) {
                    ImGui::Text("%s", b.label);
                    
                    std::string btnLabel = getBindsString(b.act);
                    if (s_BindingAction == b.act) {
                        btnLabel = "PRESS KEY TO BIND (ESC TO CLEAR ALL)";
                    }
                    
                    ImGui::SetNextItemWidth(width1 - 55.0f);
                    if (ImGui::Button((btnLabel + "##btn_" + b.actionName).c_str(), ImVec2(width1 - 55.0f, 28.0f))) {
                        s_BindingAction = b.act;
                    }
                    ImGui::Spacing();
                }
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
                
                for (auto& b : binds) {
                    ImGui::Text("%s", b.label);
                    
                    std::string btnLabel = getBindsString(b.act);
                    if (s_BindingAction == b.act) {
                        btnLabel = "PRESS KEY TO BIND (ESC TO CLEAR ALL)";
                    }
                    
                    ImGui::SetNextItemWidth(width2 - 55.0f);
                    if (ImGui::Button((btnLabel + "##btn_" + b.actionName).c_str(), ImVec2(width2 - 55.0f, 28.0f))) {
                        s_BindingAction = b.act;
                    }
                    ImGui::Spacing();
                }
                
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
        } else if (g_ActiveTab == 5) {
            RenderDemoRecorderTabContent(dl, io, panel0Pos, panel0Size, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2);
        } else if (g_ActiveTab == 6) {
            RenderDemoPlayerFullTab(dl, io, panel0Pos, panel0Size, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2);
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
    ePlayer* lp = ePlayer::PlayerConfig(0);
    
    // --- PLAYER SETUP ---
    if (ImGui::CollapsingHeader("PLAYER SETUP")) {
        if (lp) {
            char nameBuf[256];
            std::string nameUtf8 = Cp1251ToUtf8((const char*)lp->name);
            strncpy(nameBuf, nameUtf8.c_str(), sizeof(nameBuf));
            nameBuf[sizeof(nameBuf)-1] = '\0';
            if (ImGui::InputText("Screen Name", nameBuf, sizeof(nameBuf))) {
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
            if (ImGui::InputText("Global ID", gidBuf, sizeof(gidBuf))) {
                lp->globalID = Utf8ToCp1251(gidBuf);
            }
            ImGui::Spacing();
            
            ImGui::Checkbox("Auto Login", &lp->autoLogin);
            ImGui::Checkbox("Stealth Mode", &lp->stealth);
            ImGui::Checkbox("Spectator Mode", &lp->spectate);
            ImGui::Checkbox("Name Team After Me", &lp->nameTeamAfterMe);
            
            char teamNameBuf[256];
            std::string teamNameUtf8 = Cp1251ToUtf8((const char*)lp->teamName);
            strncpy(teamNameBuf, teamNameUtf8.c_str(), sizeof(teamNameBuf));
            teamNameBuf[sizeof(teamNameBuf)-1] = '\0';
            if (ImGui::InputText("Custom Team Name", teamNameBuf, sizeof(teamNameBuf))) {
                lp->teamName = Utf8ToCp1251(teamNameBuf);
                static nVersionFeature inGameRenames(5);
                if (inGameRenames.Supported()) {
                    ePlayerNetID::Update();
                    ePlayer::SendAuthNames();
                }
            }
            
            ImGui::Text("Players Per Team:");
            ImGui::SliderInt("##InGamePlayersPerTeam", &lp->favoriteNumberOfPlayersPerTeam, 1, 16);
            
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
    if (ImGui::CollapsingHeader("CAMERA CONFIG")) {
        if (lp) {
            ImGui::Text("Field of View (FOV):");
            ImGui::SliderInt("##InGameFOV", &lp->startFOV, 30, 160);
            
            ImGui::Checkbox("Smart Glance Custom", &lp->smartCustomGlance);
            ImGui::Checkbox("Auto-Switch In-Cam", &lp->autoSwitchIncam);
            ImGui::Checkbox("Wobble In-Cam", &lp->wobbleIncam);
            ImGui::Checkbox("Center In-Cam on Turn", &lp->centerIncamOnTurn);
            
            ImGui::Checkbox("Allow Smart Camera", &lp->allowCam[CAMERA_SMART]);
            ImGui::Checkbox("Allow Follow Camera", &lp->allowCam[CAMERA_FOLLOW]);
            ImGui::Checkbox("Allow Free Camera", &lp->allowCam[CAMERA_FREE]);
            ImGui::Checkbox("Allow Custom Camera", &lp->allowCam[CAMERA_CUSTOM]);
            ImGui::Checkbox("Allow Server Custom Camera", &lp->allowCam[CAMERA_SERVER_CUSTOM]);
            ImGui::Checkbox("Allow In-Cam", &lp->allowCam[CAMERA_IN]);
        }
    }
    
    // --- GRAPHICS QUALITY ---
    if (ImGui::CollapsingHeader("GRAPHICS QUALITY")) {
        ImGui::Checkbox("Alpha Blending", &sr_alphaBlend);
        ImGui::Checkbox("Smooth Shading", &sr_smoothShading);
        ImGui::Checkbox("Dither", &sr_dither);
        ImGui::Checkbox("High Rim", &sr_highRim);
        ImGui::Checkbox("Keep Window Active", &sr_keepWindowActive);
        
        {
            const char* floorDetailNames[] = { "Off", "Grid", "Texture", "Double Texture" };
            int floorDetailVal = sr_floorDetail;
            if (floorDetailVal < 0) floorDetailVal = 0;
            if (floorDetailVal > 3) floorDetailVal = 3;
            ImGui::Text("Floor Detail:");
            if (ImGui::Combo("##InGameFloorDetail", &floorDetailVal, floorDetailNames, 4)) {
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
            ImGui::Text("Floor Mirror:");
            if (ImGui::Combo("##InGameFloorMirror", &floorMirrorIdx, floorMirrorNames, 4)) {
                if (floorMirrorIdx == 0) sr_floorMirror = rMIRROR_OFF;
                else if (floorMirrorIdx == 1) sr_floorMirror = rMIRROR_OBJECTS;
                else if (floorMirrorIdx == 2) sr_floorMirror = rMIRROR_WALLS;
                else if (floorMirrorIdx == 3) sr_floorMirror = rMIRROR_ALL;
            }
        }
    }
    
    // --- SKY & ENVIRONMENT ---
    if (ImGui::CollapsingHeader("SKY & ENVIRONMENT")) {
        ImGui::Checkbox("Upper Sky", &sr_upperSky);
        ImGui::Checkbox("Lower Sky", &sr_lowerSky);
        ImGui::Checkbox("Sky Wobble", &sr_skyWobble);
        ImGui::Checkbox("Infinity Plane", &sr_infinityPlane);
        ImGui::Checkbox("Predict Objects", &sr_predictObjects);
        ImGui::Checkbox("Truecolor Textures", &sr_texturesTruecolor);
        ImGui::Checkbox("Lag-O-Meter", &sr_laggometer);
        ImGui::Checkbox("Axes Indicator", &sg_axesIndicator);
        ImGui::Checkbox("Use Moviepack (Custom Walls/Sky/Floor)", &sg_moviepackUse);
    }
    
    // --- SOUND SETTINGS ---
    if (ImGui::CollapsingHeader("SOUND SETTINGS")) {
        {
            const char* soundQualityNames[] = { "Off", "Low", "Medium", "High" };
            int soundQual = sound_quality;
            if (soundQual < 0 || soundQual > 3) soundQual = 2;
            ImGui::Text("Sound Quality:");
            if (ImGui::Combo("##InGameSoundQuality", &soundQual, soundQualityNames, 4)) {
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
            ImGui::Text("Audio Buffer Size:");
            if (ImGui::Combo("##InGameAudioBuffer", &bufShiftIdx, bufferShiftNames, 5)) {
                int oldShift = buffer_shift;
                buffer_shift = bufShiftIdx - 2;
                if (oldShift != buffer_shift) {
                    se_SoundExit();
                    se_SoundInit();
                }
            }
        }
        
        ImGui::Text("Max Sound Sources:");
        ImGui::SliderInt("##InGameSoundSources", &sound_sources, 2, 20);
    }
    
    // --- KEYBINDINGS ---
    if (ImGui::CollapsingHeader("KEYBINDINGS")) {
        struct KeybindItem {
            const char* label;
            const char* actionName;
            uAction* act;
        };
        static KeybindItem binds[] = {
            { "Turn Left", "CYCLE_TURN_LEFT", nullptr },
            { "Turn Right", "CYCLE_TURN_RIGHT", nullptr },
            { "Brake", "CYCLE_BRAKE", nullptr },
            { "Glance Left", "GLANCE_LEFT", nullptr },
            { "Glance Right", "GLANCE_RIGHT", nullptr },
            { "Glance Back", "GLANCE_BACK", nullptr },
            { "Switch View", "SWITCH_VIEW", nullptr },
            { "Chat / Say", "CHAT", nullptr },
            { "Team Chat", "CHAT_TEAM", nullptr },
            { "Toggle Console", "CONSOLE_TOGGLE", nullptr },
            { "Take Screenshot", "SCREENSHOT", nullptr },
            { "Pause Game", "PAUSE", nullptr },
            { "Perfect Turn Left", "CYCLE_PERFECT_LEFT", nullptr },
            { "Perfect Turn Right", "CYCLE_PERFECT_RIGHT", nullptr },
            { "AutoEscape Left", "CYCLE_AUTOESCAPE_LEFT", nullptr },
            { "AutoEscape Right", "CYCLE_AUTOESCAPE_RIGHT", nullptr }
        };
        
        for (auto& b : binds) {
            if (!b.act) {
                b.act = uAction::Find(b.actionName);
            }
        }
        
        auto getBindsString = [](uAction* act) -> std::string {
            if (!act) return "NONE";
            std::string res = "";
            for (int sym = 0; sym < SDLK_NEWLAST; ++sym) {
                if (keymap[sym] && keymap[sym]->act == act) {
                    if (!res.empty()) res += " / ";
                    res += GetScancodeName(sym);
                }
            }
            return res.empty() ? "NONE" : res;
        };
        
        for (auto& b : binds) {
            ImGui::Text("%s", b.label);
            
            std::string btnLabel = getBindsString(b.act);
            if (s_BindingAction == b.act) {
                btnLabel = "PRESS KEY TO BIND (ESC TO CLEAR ALL)";
            }
            
            if (ImGui::Button((btnLabel + "##ingame_btn_" + b.actionName).c_str(), ImVec2(width - 25.0f, 28.0f))) {
                s_BindingAction = b.act;
            }
        }
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // --- ADVANCED ENGINE SETTINGS ---
    if (ImGui::CollapsingHeader("ADVANCED ENGINE SETTINGS")) {
        if (ImGui::Button("MATCH RULES & PHYSICS SETTINGS", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { GameSettingsCurrent(); };
        }
        ImGui::Spacing();
        if (ImGui::Button("DISPLAY & VIDEO SETTINGS (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { sg_screenMenu.Enter(); };
        }
        ImGui::Spacing();
        if (ImGui::Button("AUDIO & SOUND SETTINGS (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { se_SoundMenu(); };
        }
        ImGui::Spacing();
        if (ImGui::Button("PLAYER PROFILE & VIEWPORTS (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { sg_PlayerMenu(); };
        }
        ImGui::Spacing();
        if (ImGui::Button("GLOBAL CYCLE INPUT CONFIG (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { su_InputConfigGlobal(); };
        }
        ImGui::Spacing();
        if (ImGui::Button("PLAYER 1 CONTROLS CONFIG (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { su_InputConfig(0); };
        }
        ImGui::Spacing();
        if (ImGui::Button("PLAYER 1 CAMERA CONFIG (LEGACY)", ImVec2(width - 25.0f, 30.0f))) {
            g_PendingLegacyMenuAction = []() { su_InputConfigCamera(0); };
        }
    }
}

void ModMenu::RenderInGameVotingAndPolice(float width) {
    if (sn_GetNetState() == nSTANDALONE) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Voting/Police features only available in network games.");
        return;
    }
    
    float btnH = 36.0f;
    
    if (ImGui::Button("OPEN POLICE MENU", ImVec2(width, btnH))) {
        ePlayerNetID::PoliceMenu();
    }
    
    ImGui::Dummy(ImVec2(0.0f, 16.0f));
    
    if (eVoter::VotingPossible()) {
        if (ImGui::Button("OPEN VOTING MENU", ImVec2(width, btnH))) {
            eVoter::VotingMenu();
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Voting is not currently possible.");
    }
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
    if (getenv("RETRO_TEST_START_GAME")) {
        g_ActiveTab = 4;
        g_DashboardLeftSelected = 4;
        g_DashboardActiveCol = 2;
        g_ModMenuTab = 1;
    }
    
    static float targetScrollY = 0.0f;
    static float currentScrollY = 0.0f;
    static float diagFlashTimer = 0.0f;
    
    float startupCooldown = 0.3f; // Cooldown to absorb stray inputs
    
    while (inGameMenuOpen && !uMenu::quickexit && !uMenu::exitToMain) {
        CheckDisplaySizeRebuild();
        SDL_ShowCursor();
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
            static struct timeval last_tv = {0, 0};
            struct timeval tv;
            gettimeofday(&tv, NULL);
            if (last_tv.tv_sec != 0) {
                double dt = (double)(tv.tv_sec - last_tv.tv_sec) + (double)(tv.tv_usec - last_tv.tv_usec) / 1000000.0;
                if (dt > 0.0) {
                    io.DeltaTime = (float)dt;
                    last_tv = tv;
                }
            } else {
                last_tv = tv;
            }

            SDL_ShowCursor();
            io.MouseDrawCursor = true;

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
            bgDl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.65f)));
            DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);

            HudManager::Update(io.DeltaTime);
            HudManager::Render();

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
        
        static struct timeval last_tv = {0, 0};
        struct timeval tv;
        gettimeofday(&tv, NULL);
        if (last_tv.tv_sec != 0) {
            double dt = (double)(tv.tv_sec - last_tv.tv_sec) + (double)(tv.tv_usec - last_tv.tv_usec) / 1000000.0;
            if (dt > 0.0) {
                io.DeltaTime = (float)dt;
                last_tv = tv;
            }
        } else {
            last_tv = tv;
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
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.03f, 0.65f)); // Soft dark backdrop
        ImGui::Begin("##InGameDashboardBG", nullptr, bgFlags);
        ImDrawList* bgDl = ImGui::GetWindowDrawList();
        DrawBackgroundParticles(bgDl, ImVec2(0, 0), io.DisplaySize, 1.0f);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        
        // 2. Centered In-Game Dashboard Card Window
        bool hasPanel2 = (io.DisplaySize.x >= 1200.0f);
        ImVec2 cardSize(hasPanel2 ? 1180.0f : 750.0f, 700.0f);
        ImVec2 cardPos((io.DisplaySize.x - cardSize.x) * 0.5f, (io.DisplaySize.y - cardSize.y) * 0.5f);
        ImGui::SetNextWindowPos(cardPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(cardSize, ImGuiCond_Always);
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
        if (sn_GetNetState() == nCLIENT) {
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
        
        dl->AddRectFilledMultiColor(ImVec2(winPos.x + paddingX, winPos.y + headerY), ImVec2(winPos.x + cardSize.x - paddingX, winPos.y + headerY + 3.0f),
                                   GetThemeColor(0.0f), GetThemeColor(1.0f), GetThemeColor(1.0f), GetThemeColor(0.0f));
        
        // 4. Grid Columns layout (1180x700px Card Optimized)
        float colY = headerY + 30.0f;
        float colH = cardSize.y - colY - 35.0f; // Calculate dynamically
        float colW0 = 230.0f;
        float colW = hasPanel2 ? 425.0f : (cardSize.x - colW0 - 2.0f * paddingX - 20.0f);
        
        auto drawPanel = [&](ImVec2 pos, ImVec2 size, bool active, const char* title) {
            ImU32 bgCol = ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.08f, 0.82f));
            ImU32 borderCol = active ? GetThemeColor(0.5f) : ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.16f, 0.7f));
            
            if (active) {
                RenderTextureGlow(pos, size, (GetThemeColor(0.5f) & 0x00FFFFFF) | 0x22000000, 16.0f);
            } else {
                RenderTextureGlow(pos, size, IM_COL32(0, 0, 0, 45), 10.0f);
            }
            
            dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgCol, 12.0f);
            dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), borderCol, 12.0f, 0, active ? 2.0f : 1.0f);
            
            if (g_FontHeader) ImGui::PushFont(g_FontHeader);
            dl->AddText(ImVec2(pos.x + 20.0f, pos.y + 18.0f), GetThemeColor(0.2f), title);
            if (g_FontHeader) ImGui::PopFont();
            
            dl->AddRectFilledMultiColor(ImVec2(pos.x + 20.0f, pos.y + 42.0f), ImVec2(pos.x + size.x - 20.0f, pos.y + 44.0f),
                                       GetThemeColor(0.0f), GetThemeColor(0.8f), GetThemeColor(0.8f), GetThemeColor(0.0f));
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
        navLabels.push_back("Resume Match");
        navLabels.push_back("Teams & Voting");
        navLabels.push_back("Settings");
        navLabels.push_back("Mod Menu");
        navLabels.push_back("Demo Recorder");
        navLabels.push_back("Demo Player");
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
                float itemY = navStart.y + i * 55.0f;
                float itemH = 45.0f;
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
                else if (labelStr == "Teams & Voting") isActiveTab = (g_ActiveTab == 2);
                else if (labelStr == "Settings") isActiveTab = (g_ActiveTab == 3);
                else if (labelStr == "Mod Menu") isActiveTab = (g_ActiveTab == 4);
                else if (labelStr == "Demo Recorder") isActiveTab = (g_ActiveTab == 5);
                else if (labelStr == "Demo Player") isActiveTab = (g_ActiveTab == 6);
                
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
                        else if (labelStr == "Teams & Voting") g_ActiveTab = 2;
                        else if (labelStr == "Settings") g_ActiveTab = 3;
                        else if (labelStr == "Demo Recorder") g_ActiveTab = 5;
                        else if (labelStr == "Demo Player") g_ActiveTab = 6;
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (g_ActiveTab == 0) {
            // --- TAB 0: DASHBOARD ---
            static int s_DashSubTab = 0; // 0: Info, 1: Profiles
            
            if (!hasPanel2) {
                // Render sub-tabs at the top of Panel 1
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
                // COLUMN 1: SYSTEM INFORMATION / SERVER INFORMATION
                bool isMultiplayer = (sn_GetNetState() == nCLIENT);
                const char* panel1Title = isMultiplayer ? "SERVER INFORMATION" : "SYSTEM INFORMATION";
                drawPanel(panel1Pos, panel1Size, g_DashboardActiveCol == 1, panel1Title);
                
                if (isMultiplayer) {
                    float listY = panel1Pos.y + 60.0f + offsetDashY;
                    float listH = colH - 80.0f - offsetDashY; // Fills the column area nicely
                    
                    ImGui::SetCursorScreenPos(ImVec2(panel1Pos.x + 15.0f, listY));
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
                    
                    bool showPlayers = ImGui::BeginChild("##DashboardServerPlayers", ImVec2(colW - 30.0f, listH), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    if (showPlayers) {
                        for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
                            ePlayerNetID* pni = se_PlayerNetIDs(i);
                            if (pni) {
                                std::string loginStr = "not logged in";
#ifdef KRAWALL_SERVER
                                if (pni->IsAuthenticated() && pni->GetRawAuthenticatedName().Len() > 0) {
                                    loginStr = (const char*)pni->GetRawAuthenticatedName();
                                }
#else
                                if (pni->IsLoggedIn()) {
                                    loginStr = "admin";
                                }
#endif
                                
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.4f));
                                
                                ImGui::BeginChildFrame(ImGui::GetID(pni), ImVec2(colW - 55.0f, 44.0f));
                                
                                ImDrawList* cellDl = ImGui::GetWindowDrawList();
                                ImVec2 framePos = ImGui::GetWindowPos();
                                
                                // Player Name
                                RenderColoredText(cellDl, ImVec2(framePos.x + 10.0f, framePos.y + 4.0f), IM_COL32(255, 255, 255, 255), pni->GetName());
                                // Login
                                std::string loginDisplay = "@" + loginStr;
                                cellDl->AddText(ImVec2(framePos.x + 10.0f, framePos.y + 24.0f), ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.6f, 0.8f)), loginDisplay.c_str());
                                
                                // Ping on the right
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
                        dl->AddLine(ImVec2(panel1Pos.x + 20.0f, infoY + 28.0f), ImVec2(panel1Pos.x + colW - 20.0f, infoY + 28.0f), ImGui::GetColorU32(ImVec4(0.11f, 0.11f, 0.13f, 0.4f)));
                        infoY += 40.0f;
                    };
                    
                    char fpsBuf[32];
                    snprintf(fpsBuf, sizeof(fpsBuf), "%.1f FPS", io.Framerate);
                    
                    drawInfoRow("System State:", connState);
                    drawInfoRow("Performance Rate:", fpsBuf, io.Framerate > 55.0f ? IM_COL32(100, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
                    
                    std::string glVendor = (const char*)glGetString(GL_VENDOR);
                    std::string glRenderer = (const char*)glGetString(GL_RENDERER);
                    if (glVendor.length() > 60) glVendor = glVendor.substr(0, 60) + "...";
                    if (glRenderer.length() > 60) glRenderer = glRenderer.substr(0, 60) + "...";
                    
                    drawInfoRow("GPU Vendor:", glVendor.c_str());
                    drawInfoRow("GPU Renderer:", glRenderer.c_str());
                    
                    // interactive buttons in middle column
                    const char* midLabels[2] = { "Refresh Settings", "Toggle FPS Overlay" };
                    for (int i = 0; i < 2; i++) {
                        float itemY = panel1Pos.y + colH - 140.0f + i * 65.0f;
                        float itemH = 50.0f;
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
                                        sr_FPSOut ? IM_COL32(100, 255, 100, 255) : IM_COL32(255, 100, 100, 255), statusStr);
                        }
                        
                        if (g_DashboardActionTriggered && selected) {
                            g_DashboardActionTriggered = false;
                            if (i == 0) {
                                LoadDashboardConfigs();
                                diagFlashTimer = 0.5f;
                            } else if (i == 1) {
                                sr_FPSOut = !sr_FPSOut;
                            }
                        }
                    }
                }
            }
            
            if (hasPanel2 || s_DashSubTab == 1) {
                ImVec2 targetPos = hasPanel2 ? panel2Pos : panel1Pos;
                ImVec2 targetSize = hasPanel2 ? panel2Size : panel1Size;
                
                // COLUMN 2: CONFIG PROFILES (Right)
                drawPanel(targetPos, targetSize, g_DashboardActiveCol == 2, "CONFIG PROFILES");
                
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
                    if (selectedY < targetScrollY) targetScrollY = selectedY;
                    else if (selectedY + itemHeight > targetScrollY + visibleHeight) targetScrollY = selectedY + itemHeight - visibleHeight;
                }
                currentScrollY += (targetScrollY - currentScrollY) * 0.2f;
                
                float totalHeightOfItems = numConfigs * itemHeight;
                if (totalHeightOfItems > visibleHeight) {
                    float scrollbarX = targetPos.x + targetSize.x - 12.0f;
                    float scrollbarY = listStartY;
                    float scrollbarH = visibleHeight;
                    dl->AddRectFilled(ImVec2(scrollbarX, scrollbarY), ImVec2(scrollbarX + 4.0f, scrollbarY + scrollbarH), ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.10f, 0.5f)), 2.0f);
                    
                    float thumbH = std::max(20.0f, (visibleHeight / totalHeightOfItems) * scrollbarH);
                    if (thumbH > scrollbarH) thumbH = scrollbarH;
                    float thumbY = scrollbarY + (currentScrollY / totalHeightOfItems) * scrollbarH;
                    dl->AddRectFilled(ImVec2(scrollbarX, thumbY), ImVec2(scrollbarX + 4.0f, thumbY + thumbH), GetThemeColor(0.5f), 2.0f);
                }
                
                dl->PushClipRect(ImVec2(targetPos.x, listStartY), ImVec2(targetPos.x + targetSize.x - 15.0f, listStartY + visibleHeight), true);
                
                if (numConfigs == 0) {
                    dl->AddText(ImVec2(targetPos.x + 25.0f, listStartY + 20.0f), ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.45f, 0.8f)), "No custom profiles found.");
                } else {
                    for (int i = 0; i < numConfigs; i++) {
                        float itemY = listStartY + i * itemHeight - currentScrollY;
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
            
            struct KeybindItem {
                const char* label;
                const char* actionName;
                uAction* act;
            };
            static KeybindItem binds[] = {
                { "Turn Left", "CYCLE_TURN_LEFT", nullptr },
                { "Turn Right", "CYCLE_TURN_RIGHT", nullptr },
                { "Brake", "CYCLE_BRAKE", nullptr },
                { "Glance Left", "GLANCE_LEFT", nullptr },
                { "Glance Right", "GLANCE_RIGHT", nullptr },
                { "Glance Back", "GLANCE_BACK", nullptr },
                { "Switch View", "SWITCH_VIEW", nullptr },
                { "Chat / Say", "CHAT", nullptr },
                { "Team Chat", "CHAT_TEAM", nullptr },
                { "Toggle Console", "CONSOLE_TOGGLE", nullptr },
                { "Take Screenshot", "SCREENSHOT", nullptr },
                { "Pause Game", "PAUSE", nullptr },
#if !PUBLIC_BUILD
                { "Perfect Turn Left", "CYCLE_PERFECT_LEFT", nullptr },
                { "Perfect Turn Right", "CYCLE_PERFECT_RIGHT", nullptr },
                { "AutoEscape Left", "CYCLE_AUTOESCAPE_LEFT", nullptr },
                { "AutoEscape Right", "CYCLE_AUTOESCAPE_RIGHT", nullptr }
#endif
            };
            
            for (auto& b : binds) {
                if (!b.act) b.act = uAction::Find(b.actionName);
            }
            
            auto getBindsString = [](uAction* act) -> std::string {
                if (!act) return "NONE";
                std::string res = "";
                for (int sym = 0; sym < SDLK_NEWLAST; ++sym) {
                    if (keymap[sym] && keymap[sym]->act == act) {
                        if (!res.empty()) res += " / ";
                        res += GetScancodeName(sym);
                    }
                }
                return res.empty() ? "NONE" : res;
            };

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
                
                for (auto& b : binds) {
                    ImGui::Text("%s", b.label);
                    
                    std::string btnLabel = getBindsString(b.act);
                    if (s_BindingAction == b.act) {
                        btnLabel = "PRESS KEY TO BIND (ESC TO CLEAR ALL)";
                    }
                    
                    ImGui::SetNextItemWidth(colW - 55.0f);
                    if (ImGui::Button((btnLabel + "##ingame_btn_" + b.actionName).c_str(), ImVec2(colW - 55.0f, 30.0f))) {
                        s_BindingAction = b.act;
                    }
                    ImGui::Spacing();
                }
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
                
                for (auto& b : binds) {
                    ImGui::Text("%s", b.label);
                    
                    std::string btnLabel = getBindsString(b.act);
                    if (s_BindingAction == b.act) {
                        btnLabel = "PRESS KEY TO BIND (ESC TO CLEAR ALL)";
                    }
                    
                    ImGui::SetNextItemWidth(colW - 55.0f);
                    if (ImGui::Button((btnLabel + "##ingame_btn_" + b.actionName).c_str(), ImVec2(colW - 55.0f, 30.0f))) {
                        s_BindingAction = b.act;
                    }
                    ImGui::Spacing();
                }
                
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

        } else if (g_ActiveTab == 5) {

            RenderDemoRecorderTabContent(dl, io, panel0Pos, panel0Size, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2);

        } else if (g_ActiveTab == 6) {

            RenderDemoPlayerFullTab(dl, io, panel0Pos, panel0Size, panel1Pos, panel1Size, panel2Pos, panel2Size, hasPanel2);

        }
        
        // Render modals/popups
        RenderModMenuModals();
        
        // Draw the embedded media player card below the in-game pause menu card
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
        
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
        
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
