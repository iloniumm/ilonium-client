

#include "gMenus.h"
#include "ePlayer.h"
#include "rScreen.h"
#include "nConfig.h"
#include "rConsole.h"
#include "tToDo.h"
#include "rGL.h"
#include "eTimer.h"
#include "eFloor.h"
#include "rRender.h"
#include "rModel.h"
#include "gGame.h"
#include "gCycle.h"
#include "gHud.h"
#include "tRecorder.h"
#include "rSysdep.h"

#include <sstream>
#include <set>
#include <array>
#include <memory>



bool sg_modWaypointsEnabled = true;
bool sg_modWaypointsEnemyWalls = false;
REAL sg_modWaypointsMinRubber = 3.0f;
REAL sg_modWaypointsCooldown = 5.0f;
REAL sg_modWaypointsLifetime = 30.0f;
bool sg_modRubberGaugeEnabled = true;
bool sg_modMinimapEnabled = true;
bool sg_modRubberBatteryEnabled = true;
bool sg_modClassicRubberBatteryEnabled = true;
REAL sg_modMinimapScale = 1.0f;
REAL sg_modMinimapZoom = 1.0f;
bool sg_modMinimapRotate = false;
bool sg_modProximityWarning = true;
bool sg_modZoneTimers = true;
bool sg_modCutoffAimbot = true;
bool sg_modPathLineEnabled = true;
bool sg_modTeammateDeathWarning = true;
bool sg_modFortressAlerts = true;
int  sg_modPathDepth = 4;
int  sg_modPathRange = 120;
bool sg_modScoreboardEnabled = true;
bool sg_modAnti360LockEnabled = true;
bool sg_modChatCalcEnabled = false;
bool sg_modTrashTalkEnabled = false;
bool sg_modKDEnabled = true;
bool sg_modKDResetFlag = false;
int  sg_kdKills = 0;
int  sg_kdDeaths = 0;
bool sg_modAutoGreet = false;
bool sg_modFriendlyChat = false;
bool sg_modColorPicker = true;
bool sg_modKillSoundsEnabled = true;
int  sg_modKillAnnouncerPack = 0;


bool sg_modParticleSystemEnabled = true;
int  sg_modDeathParticlesCount = 150;
bool sg_modScreenShakeEnabled = true;
REAL sg_modScreenShakeIntensity = 0.5f;
bool sg_modAmbientParticlesEnabled = true;
bool sg_modTrailGradientEnabled = true;
REAL sg_modTrailAlphaTop = 0.8f;
REAL sg_modTrailAlphaBottom = 0.0f;

tString sg_activeCameraConfig;
static tConfItemLine sg_activeCameraConfigConf("MOD_ACTIVE_CAMERA_CONFIG", sg_activeCameraConfig);

tString sg_activeTexturePack = tString("Default");
static tConfItemLine sg_activeTexturePackConf("MOD_ACTIVE_TEXTURE_PACK", sg_activeTexturePack);

static tConfItem<bool> sg_modWaypointsEnabledConf("MOD_WAYPOINTS_ENABLED", sg_modWaypointsEnabled);
static tConfItem<bool> sg_modWaypointsEnemyWallsConf("MOD_WAYPOINTS_ENEMY_WALLS", sg_modWaypointsEnemyWalls);
static tConfItem<REAL> sg_modWaypointsMinRubberConf("MOD_WAYPOINTS_MIN_RUBBER", sg_modWaypointsMinRubber);
static tConfItem<REAL> sg_modWaypointsCooldownConf("MOD_WAYPOINTS_COOLDOWN", sg_modWaypointsCooldown);
static tConfItem<REAL> sg_modWaypointsLifetimeConf("MOD_WAYPOINTS_LIFETIME", sg_modWaypointsLifetime);
static tConfItem<bool> sg_modRubberGaugeEnabledConf("MOD_RUBBER_GAUGE", sg_modRubberGaugeEnabled);
static tConfItem<bool> sg_modMinimapEnabledConf("MOD_MINIMAP", sg_modMinimapEnabled);
static tConfItem<bool> sg_modRubberBatteryEnabledConf("MOD_RUBBER_BATTERY", sg_modRubberBatteryEnabled);
static tConfItem<bool> sg_modClassicRubberBatteryEnabledConf("MOD_CLASSIC_RUBBER_BATTERY", sg_modClassicRubberBatteryEnabled);
static tConfItem<REAL> sg_modMinimapScaleConf("MOD_MINIMAP_SCALE", sg_modMinimapScale);
static tConfItem<REAL> sg_modMinimapZoomConf("MOD_MINIMAP_ZOOM", sg_modMinimapZoom);
static tConfItem<bool> sg_modMinimapRotateConf("MOD_MINIMAP_ROTATE", sg_modMinimapRotate);
static tConfItem<bool> sg_modProximityWarningConf("MOD_PROXIMITY_WARNING", sg_modProximityWarning);
static tConfItem<bool> sg_modZoneTimersConf("MOD_ZONE_TIMERS", sg_modZoneTimers);
static tConfItem<bool> sg_modCutoffAimbotConf("MOD_CUTOFF_AIMBOT", sg_modCutoffAimbot);
static tConfItem<bool> sg_modPathLineEnabledConf("MOD_PATH_LINE", sg_modPathLineEnabled);
static tConfItem<bool> sg_modTeammateDeathWarningConf("MOD_TEAMMATE_DEATH_WARNING", sg_modTeammateDeathWarning);
static tConfItem<bool> sg_modFortressAlertsConf("MOD_FORTRESS_ALERTS", sg_modFortressAlerts);
static tConfItem<int>  sg_modPathDepthConf("MOD_PATH_DEPTH", sg_modPathDepth);
static tConfItem<int>  sg_modPathRangeConf("MOD_PATH_RANGE", sg_modPathRange);
static tConfItem<bool> sg_modScoreboardEnabledConf("MOD_SCOREBOARD", sg_modScoreboardEnabled);
static tConfItem<bool> sg_modAnti360LockEnabledConf("ANTI_360_LOCK_ENABLED", sg_modAnti360LockEnabled);
static tConfItem<bool> sg_modChatCalcEnabledConf("MOD_CHAT_CALC", sg_modChatCalcEnabled);

static tConfItem<bool> sg_modKDEnabledConf("MOD_KD_COUNTER", sg_modKDEnabled);
static tConfItem<bool> sg_modAutoGreetConf("MOD_AUTO_GREET", sg_modAutoGreet);

static tConfItem<bool> sg_modColorPickerConf("MOD_COLOR_PICKER", sg_modColorPicker);
static tConfItem<bool> sg_modKillSoundsEnabledConf("MOD_KILL_SOUNDS_ENABLED", sg_modKillSoundsEnabled);
static tConfItem<int>  sg_modKillAnnouncerPackConf("MOD_KILL_ANNOUNCER_PACK", sg_modKillAnnouncerPack);
static tConfItem<bool> sg_modParticleSystemEnabledConf("MOD_PARTICLE_SYSTEM_ENABLED", sg_modParticleSystemEnabled);
static tConfItem<int>  sg_modDeathParticlesCountConf("MOD_DEATH_PARTICLES_COUNT", sg_modDeathParticlesCount);
static tConfItem<bool> sg_modScreenShakeEnabledConf("MOD_SCREEN_SHAKE_ENABLED", sg_modScreenShakeEnabled);
static tConfItem<REAL> sg_modScreenShakeIntensityConf("MOD_SCREEN_SHAKE_INTENSITY", sg_modScreenShakeIntensity);
static tConfItem<bool> sg_modAmbientParticlesEnabledConf("MOD_AMBIENT_PARTICLES_ENABLED", sg_modAmbientParticlesEnabled);
static tConfItem<bool> sg_modTrailGradientEnabledConf("MOD_TRAIL_GRADIENT_ENABLED", sg_modTrailGradientEnabled);
static tConfItem<REAL> sg_modTrailAlphaTopConf("MOD_TRAIL_ALPHA_TOP", sg_modTrailAlphaTop);
static tConfItem<REAL> sg_modTrailAlphaBottomConf("MOD_TRAIL_ALPHA_BOTTOM", sg_modTrailAlphaBottom);



static void sg_ModCombatMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Combat Settings");

    
    extern REAL sn_anti360Window;

    static int anti360WindowInt;
    anti360WindowInt = (int)(sn_anti360Window * 100.0f);

    uMenuItemToggle t3(&sub,
        "Cut-off Predictor",
        "Show CUT/NO indicator when you can box in an enemy",
        sg_modCutoffAimbot);

    uMenuItemToggle t2(&sub,
        "Proximity Warning",
        "Red edge glow when enemies are dangerously close behind",
        sg_modProximityWarning);

    uMenuItemToggle rG(&sub, "Rubber Gauge",
        "Display rubber percentage near cycle",
        sg_modRubberGaugeEnabled);

    uMenuItemInt a360w(&sub, "Anti-360 Window (x100)",
        "Time window in seconds (x100). 100=1.0s, 50=0.5s. Turns outside this window are allowed.",
        anti360WindowInt, 10, 300);

    uMenuItemToggle a360(&sub, "Anti-360 Lock",
        "Prevent 360-degree suicide by ignoring the 4th consecutive turn in the same direction",
        sg_modAnti360LockEnabled);

    sub.Enter();

    sn_anti360Window = (REAL)anti360WindowInt / 100.0f;
    if (sn_anti360Window < 0.1f) sn_anti360Window = 0.1f;
}


static void sg_ModFortressMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Fortress & Alerts");

    uMenuItemToggle t4(&sub,
        "Live Scoreboard",
        "Show team scores and player ranking on screen",
        sg_modScoreboardEnabled);

    uMenuItemToggle t3(&sub,
        "Zone Timers",
        "Show time until Sumo/Fortress zone collapse",
        sg_modZoneTimers);

    uMenuItemToggle t2(&sub,
        "Fortress Zone Alerts",
        "Show conquest/defense alerts for fortress zones",
        sg_modFortressAlerts);

    uMenuItemToggle t1(&sub,
        "Teammate Death Warning",
        "Flash warning when a teammate dies",
        sg_modTeammateDeathWarning);

    sub.Enter();
}


static void sg_ModMinimapMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Minimap Settings");

    static int minimapScaleInt, minimapZoomInt;
    minimapScaleInt = (int)(sg_modMinimapScale * 10.0f);
    minimapZoomInt = (int)(sg_modMinimapZoom * 10.0f);

    uMenuItemToggle rotate(&sub, "Rotate With Player",
        "Rotate minimap smoothly to match player direction", sg_modMinimapRotate);

    uMenuItemInt zoom(&sub, "Inner Zoom (x10)",
        "Zoom into map content (10=full, 20=2x, 50=5x)", minimapZoomInt, 10, 50);

    uMenuItemInt scale(&sub, "Size (x10)",
        "Minimap window size (1-30). Default 10", minimapScaleInt, 1, 30);

    uMenuItemToggle toggle(&sub, "Minimap Enabled",
        "Show/hide the minimap radar", sg_modMinimapEnabled);

    sub.Enter();

    sg_modMinimapScale = (REAL)minimapScaleInt / 10.0f;
    if (sg_modMinimapScale < 0.1f) sg_modMinimapScale = 0.1f;
    sg_modMinimapZoom = (REAL)minimapZoomInt / 10.0f;
    if (sg_modMinimapZoom < 1.0f) sg_modMinimapZoom = 1.0f;
}


static void sg_ModPathfindingMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Pathfinding Settings");

    static int pathDepthInt, pathRangeInt;
    pathDepthInt = sg_modPathDepth;
    pathRangeInt = sg_modPathRange;

    uMenuItemInt depth(&sub, "Search Depth (turns)",
        "How many turns ahead to explore (2-6)", pathDepthInt, 2, 6);

    uMenuItemInt range(&sub, "Max Range (units)",
        "Maximum distance the path can extend (50-200)", pathRangeInt, 50, 200);

    uMenuItemToggle toggle(&sub, "Pathfinding Line",
        "Show escape vector on the floor", sg_modPathLineEnabled);

    sub.Enter();

    sg_modPathDepth = pathDepthInt;
    sg_modPathRange = pathRangeInt;
}


static void sg_ModWaypointsMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Waypoint Settings");

    static int minRubberInt, cooldownInt, lifetimeInt;
    minRubberInt = (int)sg_modWaypointsMinRubber;
    cooldownInt = (int)sg_modWaypointsCooldown;
    lifetimeInt = (int)sg_modWaypointsLifetime;

    uMenuItemInt lifetime(&sub, "Lifetime (sec)",
        "How long waypoint markers stay visible (5-30)", lifetimeInt, 5, 30);

    uMenuItemInt cooldown(&sub, "Cooldown (sec)",
        "Seconds between markers (1-30)", cooldownInt, 1, 30);

    uMenuItemToggle enemy(&sub, "All Walls",
        "ON = all walls, OFF = rim only", sg_modWaypointsEnemyWalls);

    uMenuItemInt minrub(&sub, "Min Rubber",
        "Minimum rubber to create marker", minRubberInt, 0, 20);

    uMenuItemToggle toggle(&sub, "Waypoints Enabled",
        "Enable rubber waypoint markers", sg_modWaypointsEnabled);

    sub.Enter();

    sg_modWaypointsMinRubber = (REAL)minRubberInt;
    sg_modWaypointsCooldown = (REAL)cooldownInt;
    sg_modWaypointsLifetime = (REAL)lifetimeInt;
}


static void sg_ModPerfectTurnMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu sub("Perfect Turn & AutoEscape");

    
    extern REAL sg_perfectTurnCalibration;
    extern REAL sg_autoEscapeRubberMargin;
    extern bool sg_autoEscapePingComp;

    static int calibInt, marginInt;
    calibInt = (int)(sg_perfectTurnCalibration * 100.0f);
    marginInt = (int)(sg_autoEscapeRubberMargin * 100.0f);

    uMenuItemToggle pingComp(&sub,
        "AutoEscape: Ping Compensation",
        "Use network ping to time escape turns",
        sg_autoEscapePingComp);

    uMenuItemInt margin(&sub,
        "AutoEscape: Rubber Margin (x100)",
        "Safety margin before max rubber (0-100). 15 = 0.15 rubber units",
        marginInt, 0, 100);

    uMenuItemInt calib(&sub,
        "Perfect Turn: Calibration (x100)",
        "Manual trigger distance (0 = rubber-only mode). 50 = 0.50 units",
        calibInt, 0, 200);

    sub.Enter();

    sg_perfectTurnCalibration = (REAL)calibInt / 100.0f;
    sg_autoEscapeRubberMargin = (REAL)marginInt / 100.0f;
}



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
extern REAL sg_noclipOrbitSpeed;
extern REAL sg_noclipOrbitRadius;
extern REAL sg_noclipOrbitHeight;
extern REAL sg_noclipFollowDist;
extern REAL sg_noclipFollowHeight;
extern REAL sg_noclipSmoothFactor;

#include <SDL3/SDL.h>

class uMenuItemModKey : public uMenuItem {
    int& keycode;
    tString title;
    bool active;
public:
    uMenuItemModKey(uMenu* m, const char* t, const char* h, int& k)
        : uMenuItem(m, h), keycode(k), title(t), active(false) {}

    virtual ~uMenuItemModKey() {}

    virtual void Render(REAL x, REAL y, REAL alpha=1, bool selected=false) {
        DisplayText(x - 0.02f, y, title, selected, alpha, 1);
        if (active) {
            DisplayText(x + 0.02f, y, "Press any key... (ESC to cancel, DEL to clear)", selected, alpha, -1);
        } else {
            const char* name = "Unbound";
            if (keycode > 0) {
                name = SDL_GetKeyName((SDL_Keycode)keycode);
            } else if (keycode == 0) {
                name = "Unbound";
            } else {
                name = "Unknown";
            }
            DisplayText(x + 0.02f, y, name, selected, alpha, -1);
        }
    }

    virtual void Enter() { active = true; }

    virtual bool Event(SDL_Event& e) {
        if (!active) return false;
        
        if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.key == SDLK_ESCAPE) {
                active = false;
                return true;
            } else if (e.key.key == SDLK_DELETE || e.key.key == SDLK_BACKSPACE) {
                keycode = 0;
                active = false;
                return true;
            }
            keycode = e.key.key;
            active = false;
            return true;
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            active = false;
            return true;
        }
        return false;
    }
};

static void sg_NoclipControlsMenu() {
    uMenu sub("Noclip Controls");

    
    new uMenuItemModKey(&sub, "Toggle Noclip Key", "Enter/Exit spectator mode",          sg_noclipKeyToggle);
    new uMenuItemModKey(&sub, "Zone Focus Key",    "Cycle camera look toward active zones",  sg_noclipKeyZoneFocus);
    extern int sg_noclipKeyMapCenter;
    new uMenuItemModKey(&sub, "Map Center Key",    "Lock camera look toward map center",     sg_noclipKeyMapCenter);
    new uMenuItemModKey(&sub, "Smooth Follow Key", "Toggle smooth chase camera",         sg_noclipKeySmooth);
    new uMenuItemModKey(&sub, "Orbit Camera Key",  "Toggle auto-orbit around player",    sg_noclipKeyOrbit);
    new uMenuItemModKey(&sub, "Clean Screen Key",  "Toggle clean screen (hide ALL HUD)", sg_noclipKeyCinematic);
    
    
    extern bool sg_noclipHideConsole;
    new uMenuItemToggle(&sub, "Hide Console in Cinematic", "Hide the chat/console when Clean Screen is active", sg_noclipHideConsole);

    extern bool sg_noclipHideNames;
    new uMenuItemToggle(&sub, "Hide Names in Cinematic", "Hide player names and rubber gauge when Clean Screen is active", sg_noclipHideNames);

    
    new uMenuItemModKey(&sub, "Slow Fly Key",       "Hold to fly slowly", sg_noclipKeySlow);
    new uMenuItemModKey(&sub, "Prev Player Key",    "Teleport to previous player", sg_noclipKeyPrevPl);
    new uMenuItemModKey(&sub, "Next Player Key",    "Teleport to next player", sg_noclipKeyNextPl);
    new uMenuItemModKey(&sub, "Follow Player Key",  "Toggle player follow mode", sg_noclipKeyFollow);
    new uMenuItemModKey(&sub, "Lock Camera Key",    "Freeze all movement and look", sg_noclipKeyLock);
    new uMenuItemModKey(&sub, "Look-At Center Key", "Rotate camera toward map center", sg_noclipKeyLookAt);
    new uMenuItemModKey(&sub, "TP to Center Key",   "Teleport to map center", sg_noclipKeyCenter);
    new uMenuItemModKey(&sub, "Zoom Out Key",       "Increase FOV (zoom out)", sg_noclipKeyZoomOut);
    new uMenuItemModKey(&sub, "Zoom In Key",        "Decrease FOV (zoom in)", sg_noclipKeyZoomIn);
    new uMenuItemModKey(&sub, "Descend Key",        "Move camera down", sg_noclipKeyDown);
    new uMenuItemModKey(&sub, "Ascend Key",         "Move camera up", sg_noclipKeyUp);
    new uMenuItemModKey(&sub, "Strafe Right Key",   "Move camera right", sg_noclipKeyRight);
    new uMenuItemModKey(&sub, "Strafe Left Key",    "Move camera left", sg_noclipKeyLeft);
    new uMenuItemModKey(&sub, "Backward Key",       "Move camera backward", sg_noclipKeyBack);
    new uMenuItemModKey(&sub, "Forward Key",        "Move camera forward", sg_noclipKeyForward);

    sub.Enter();
}

static void sg_NoclipCinematicMenu() {
    uMenu sub("Cinematic Camera Settings");

    int orbitSpd  = (int)(sg_noclipOrbitSpeed * 100);
    int orbitRad  = (int)sg_noclipOrbitRadius;
    int orbitH    = (int)sg_noclipOrbitHeight;
    int followD   = (int)sg_noclipFollowDist;
    int followH   = (int)sg_noclipFollowHeight;
    int smooth    = (int)(sg_noclipSmoothFactor * 10);

    uMenuItemInt smoothItem(&sub, "Smooth Factor (x10)",
        "LERP smoothing strength (50 = 5.0)", smooth, 10, 200);

    uMenuItemInt fhItem(&sub, "Follow Height",
        "Camera height above followed player", followH, 5, 200);

    uMenuItemInt fdItem(&sub, "Follow Distance",
        "Camera distance behind followed player", followD, 5, 200);

    uMenuItemInt ohItem(&sub, "Orbit Height",
        "Height of orbit camera above target", orbitH, 5, 300);

    uMenuItemInt orItem(&sub, "Orbit Radius",
        "Distance from orbit center to camera", orbitRad, 10, 500);

    uMenuItemInt osItem(&sub, "Orbit Speed (x100)",
        "Angular speed of orbit (30 = 0.3 rad/s)", orbitSpd, 5, 200);

    uMenuItemToggle cinToggle(&sub, "Clean Screen (no HUD)",
        "Hide ALL HUD elements for pure cinematic view", sg_noclipCinematic);

    sub.Enter();

    sg_noclipOrbitSpeed = (REAL)orbitSpd / 100.0;
    sg_noclipOrbitRadius = (REAL)orbitRad;
    sg_noclipOrbitHeight = (REAL)orbitH;
    sg_noclipFollowDist = (REAL)followD;
    sg_noclipFollowHeight = (REAL)followH;
    sg_noclipSmoothFactor = (REAL)smooth / 10.0;
}

static void sg_NoclipSettingsMenu() {
    uMenu sub("Noclip Spectator Settings");

    
    int speedInt = (int)sg_noclipSpeed;
    int sensInt  = (int)(sg_noclipMouseSens * 10000);
    int slowInt  = (int)(sg_noclipSlowFactor * 100);

    uMenuItemFunction cin(&sub, "Cinematic Settings >>",
        "Orbit, smooth follow, clean screen", &sg_NoclipCinematicMenu);

    uMenuItemFunction ctrl(&sub, "Controls / Keybinds >>",
        "Rebind all noclip keys", &sg_NoclipControlsMenu);

    uMenuItemInt slowItem(&sub, "Slow Factor (x100)",
        "Speed multiplier when slow key is held (25 = 0.25x)", slowInt, 5, 100);

    uMenuItemInt sensItem(&sub, "Mouse Sensitivity (x10000)",
        "Mouse look sensitivity (30 = 0.003)", sensInt, 5, 200);

    uMenuItemInt speedItem(&sub, "Flight Speed",
        "Base movement speed in noclip mode", speedInt, 5, 500);

    sub.Enter();

    sg_noclipSpeed = (REAL)speedInt;
    sg_noclipMouseSens = (REAL)sensInt / 10000.0;
    sg_noclipSlowFactor = (REAL)slowInt / 100.0;
}


void sg_ModSettingsMenu() {
    volatile unsigned long long _salt = 0x496c6f6e61ULL;

    uMenu mod_menu("Ilona's Mod Menu");

    uMenuItemToggle battery(&mod_menu, "Rubber Battery Indicator",
        "Wide horizontal rubber bar above bottom panels", sg_modRubberBatteryEnabled);

    uMenuItemToggle classic_battery(&mod_menu, "Classic Rubber Battery",
        "Classic horizontal rubber bar above bottom panels", sg_modClassicRubberBatteryEnabled);

    uMenuItemFunction fw(&mod_menu, "Waypoint Settings >>",
        "Configure rubber waypoint markers", sg_ModWaypointsMenu);

    uMenuItemFunction fp(&mod_menu, "Pathfinding Settings >>",
        "Configure escape pathfinding visualization", sg_ModPathfindingMenu);

    uMenuItemFunction fm(&mod_menu, "Minimap Settings >>",
        "Configure minimap radar display", sg_ModMinimapMenu);

    uMenuItemFunction ff(&mod_menu, "Fortress & Alerts >>",
        "Scoreboard, zone timers, death warnings", sg_ModFortressMenu);

    uMenuItemFunction fc(&mod_menu, "Combat Settings >>",
        "Cut-off predictor, proximity warning, gauges", sg_ModCombatMenu);

    
    

    uMenuItemFunction fnoclip(&mod_menu, "Noclip Spectator >>",
        "Flight speed, mouse sensitivity, keybinds, slow mode", &sg_NoclipSettingsMenu);

    uMenuItemToggle calcToggle(&mod_menu, "Chat Calculator Bot",
        "Auto-reply to math expressions in chat (e.g. 2+2, sin(45))", sg_modChatCalcEnabled);

    
    

    uMenuItemToggle kdToggle(&mod_menu, "K/D Counter HUD",
        "Show kills/deaths/KD ratio on screen (top-left)", sg_modKDEnabled);

    uMenuItemToggle greetToggle(&mod_menu, "Auto-Greet on Join",
        "Automatically greet when joining a server", sg_modAutoGreet);

    
    

    uMenuItemToggle colorPickToggle(&mod_menu, "Color Picker (chat: color NAME)",
        "Type 'color NAME' in chat to see player RGB. 'color NAME apply' to copy.", sg_modColorPicker);

    mod_menu.Enter();
}


static tConfItem<int>   tm0("TEXTURE_MODE_0",rTextureGroups::TextureMode[0]);
static tConfItem<int>   tm1("TEXTURE_MODE_1",rTextureGroups::TextureMode[1]);
static tConfItem<int>   tm2("TEXTURE_MODE_2",rTextureGroups::TextureMode[2]);
static tConfItem<int>   tm3("TEXTURE_MODE_3",rTextureGroups::TextureMode[3]);

uMenu sg_screenMenu("$display_settings_menu");

static uMenuItemFunction defaul
(&sg_screenMenu,"$graphics_load_defaults_text",
 "$graphics_load_defaults_help",
 &sr_LoadDefaultConfig);
uMenu screen_menu_detail("$detail_settings_menu");
uMenu screen_menu_tweaks("$performance_tweaks_menu");
uMenu screen_menu_prefs("$preferences_menu");
uMenu hud_prefs("$hud_menu");

static void sg_ScreenModeMenu();

static uMenuItemSubmenu smt(&sg_screenMenu,&screen_menu_tweaks,
                            "$performance_tweaks_menu_help");
static uMenuItemSubmenu smd(&sg_screenMenu,&screen_menu_detail,
                            "$detail_settings_menu_help");
static uMenuItemSubmenu smp(&sg_screenMenu,&screen_menu_prefs,
                            "$preferences_menu_help");
static uMenuItemFunction smm(&sg_screenMenu,"$screen_mode_menu",
                             "$screen_mode_menu_help", sg_ScreenModeMenu );

static uMenuItemSubmenu huds(&screen_menu_prefs,&hud_prefs,
                             "$hud_menu_help");


static tConfItemLine c_ext("GL_EXTENSIONS",gl_extensions);
static tConfItemLine c_ver("GL_VERSION",gl_version);
static tConfItemLine c_rEnd("GL_RENDERER",gl_renderer);
static tConfItemLine c_vEnd("GL_VENDOR",gl_vendor);


static std::deque<tString> sg_consoleHistory; 
static int sg_consoleHistoryMaxSize=10; 
static tSettingItem< int > sg_consoleHistoryMaxSizeConf("HISTORY_SIZE_CONSOLE",sg_consoleHistoryMaxSize);

class ArmageTron_feature_menuitem: public uMenuItemSelection<int>{
    void NewChoice(uSelectItem<bool> *){}
    void NewChoice(char *,bool ){}
public:
    ArmageTron_feature_menuitem(uMenu *m,char const * tit,char const * help,int &targ)
            :uMenuItemSelection<int>(m,tit,help,targ){
        uMenuItemSelection<int>::NewChoice(
            "$feature_disabled_text",
            "$feature_disabled_help",
            rFEAT_OFF);
        uMenuItemSelection<int>::NewChoice(
            "$feature_default_text",
            "$feature_default_help",
            rFEAT_DEFAULT);
        uMenuItemSelection<int>::NewChoice(
            "$feature_enabled_text",
            "$feature_enabled_help",
            rFEAT_ON);
    }

    ~ArmageTron_feature_menuitem(){}
};


class ArmageTron_texmode_menuitem: public uMenuItemSelection<int>{
    void NewChoice(uSelectItem<bool> *){}
    void NewChoice(char *,bool ){}
public:
    ArmageTron_texmode_menuitem(uMenu *m,char const * tit,int &targ,
                                bool font=false)
            :uMenuItemSelection<int>
    (m,tit,"$texture_menuitem_help",targ){

        if(!font)
            uMenuItemSelection<int>::NewChoice
            ("$texture_off_text","$texture_off_help",-1);
#ifndef DEDICATED
        uMenuItemSelection<int>::NewChoice
        ("$texture_nearest_text","$texture_nearest_help",GL_NEAREST);

        uMenuItemSelection<int>::NewChoice
        ("$texture_bilinear_text","$texture_bilinear_help",GL_LINEAR);

        if(!font)
        {
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_nearest_text",
             "$texture_mipmap_nearest_help",
             GL_NEAREST_MIPMAP_NEAREST);
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_bilinear_text",
             "$texture_mipmap_bilinear_help",
             GL_LINEAR_MIPMAP_NEAREST);
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_trilinear_text",
             "$texture_mipmap_trilinear_help",
             GL_LINEAR_MIPMAP_LINEAR);
        }
    #endif
    }

    ~ArmageTron_texmode_menuitem(){}
};

static tConfItem<bool>    ab("ALPHA_BLEND",sr_alphaBlend);
static tConfItem<bool>    ss("SMOOTH_SHADING",sr_smoothShading);
static tConfItem<bool>    to("TEXT_OUT",sr_textOut);
static tConfItem<bool>    fps("SHOW_FPS",sr_FPSOut);
static tConfItem<bool>    pbt("SHOW_RECORDING_TIME",sr_RecordingTimeOut);

static tConfItem<int> fm("FLOOR_MIRROR",sr_floorMirror);
static tConfItem<int> fd("FLOOR_DETAIL",sr_floorDetail);
static tConfItem<bool> hr("HIGH_RIM",sr_highRim);
static tConfItem<bool> dt("DITHER",sr_dither);
static tConfItem<bool> us("UPPER_SKY",sr_upperSky);
static tConfItem<bool> ls("LOWER_SKY",sr_lowerSky);
static tConfItem<bool> wos("SKY_WOBBLE",sr_skyWobble);
static tConfItem<bool> ip("INFINITY_PLANE",sr_infinityPlane);

extern bool sg_axesIndicator;

static tConfItem<bool> lm("LAG_O_METER",sr_laggometer);
static tConfItem<bool> ai("AXES_INDICATOR",sg_axesIndicator);
static tConfItem<bool> po("PREDICT_OBJECTS",sr_predictObjects);
static tConfItem<bool> t32("TEXTURES_HI",sr_texturesTruecolor);

static tConfItem<bool> kwa("KEEP_WINDOW_ACTIVE",sr_keepWindowActive);
#ifdef USE_HEADLIGHT
static tConfItem<bool> chl("HEADLIGHT",headlights);
#endif

#ifndef SDL_OPENGL
#ifndef DIRTY
#define DIRTY
#endif
#endif

bool operator < ( rScreenSize const & a, rScreenSize const & b )
{
    return a.Compare(b) < 0;
}

class gResMenEntry
{
    uMenuItemSelection<rScreenSize> res_men; 
    std::set< rScreenSize > sizes;           

    
    void NewChoice( rScreenSize const & size )
    {
        if ( sizes.find( size ) == sizes.end() )
        {
            sizes.insert(size);
        }
    }

    
    void NewChoice( rResolution res )
    {
        rScreenSize size( res );
        NewChoice( size );
    }

public:
    gResMenEntry( uMenu & screen_menu_mode, rScreenSize& res, const tOutput& text, const tOutput& help, bool addFixed )
            :res_men
            (&screen_menu_mode,
             text,
             help,
             res)
    {
#ifndef DEDICATED
        
        SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
        int numModes = 0;
        SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes( displayID, &numModes );

        int i;
        if( !modes || numModes == 0 )
        {
            
            for ( i = ArmageTron_Custom; i>=0; --i )
            {
                NewChoice( rResolution(i) );
            }
        }
        else
        {
            
            NewChoice( ArmageTron_Custom );

            
            if ( sr_DesktopScreensizeSupported() && !addFixed )
                NewChoice( ArmageTron_Desktop );

            rScreenSize maxSize(0,0);

            for( i = 0; i < numModes; ++i )
            {
                rScreenSize size( modes[i]->w, modes[i]->h );
                NewChoice( size );
                if ( maxSize.width  < size.width  ) maxSize.width  = size.width;
                if ( maxSize.height < size.height ) maxSize.height = size.height;
            }

            SDL_free( (void*)modes );

            if ( addFixed )
            {
                for ( i = ArmageTron_Custom; i>=ArmageTron_Min; --i )
                {
                    rScreenSize size( static_cast< rResolution >(i) );
                    if ( maxSize.height >= size.height && maxSize.width >= size.width )
                        NewChoice( size );
                }
            }
        }

        
        for( std::set< rScreenSize >::iterator iter = sizes.begin(); iter != sizes.end(); ++iter )
            {
                rScreenSize const & size = *iter;

                std::stringstream s;
                if ( size.width + size.height > 0 )
                    s << size.width << " x " << size.height;
                else
                    s << tOutput("$screen_size_desktop");

                res_men.NewChoice( s.str().c_str(), "", size );
            }

#endif
    }
};

static void sg_ScreenModeMenu()
{
    uMenu screen_menu_mode("$screen_mode_menu");

    uMenuItemFunction appl
    (&screen_menu_mode,
     "$screen_apply_changes_text",
     "$screen_apply_changes_help",
     &sr_ReinitDisplay);

    uMenuItemToggle kwa_t(
        &screen_menu_mode,
        "$screen_keep_window_active_text",
        "$screen_keep_window_active_help",
        sr_keepWindowActive);

    uMenuItemToggle ie_t
    (&screen_menu_mode,
     "$screen_check_errors_text",
     "$screen_check_errors_help",
     currentScreensetting.checkErrors);

    
    std::unique_ptr<uMenuItem> zfm_t;
    {
        
        std::array<int, 17> pickableLimits{0, 20, 30, 40, 60, 80, 100, 120, 180, 240, 300, 360, 480, 600, 720, 900, 1200};
        bool pickable = false;
        for (auto const limit : pickableLimits)
        {
            if (limit == sr_maxFPS)
                pickable = true;
        }

        if (pickable)
        {
            
            auto zmf_t_p = new uMenuItemSelection<int>(&screen_menu_mode,
                                                       "$screen_max_fps_text",
                                                       "$max_fps_help",
                                                       sr_maxFPS);
            zfm_t.reset(zmf_t_p);

            zmf_t_p->NewChoice("$screen_max_fps_off_text", "$screen_max_fps_off_help", 0);
            for (auto const limit : pickableLimits)
            {
                if (!limit)
                    continue;
                std::stringstream s;
                s << limit;
                zmf_t_p->NewChoice(s.str().c_str(), "", limit);
            }
        }
        else
        {
            
            zfm_t.reset(new uMenuItemInt(&screen_menu_mode, "$screen_max_fps_text", "$max_fps_help", sr_maxFPS, 0, 9990, 10));
        }
    }

#ifdef SDL_OPENGL
#ifdef DIRTY
    uMenuItemToggle sdl_t
    (&screen_menu_mode,
     "$screen_use_sdl_text",
     "$screen_use_sdl_help",
     currentScreensetting.useSDL);
#endif 

#if SDL_VERSION_ATLEAST(1, 2, 10)
    uMenuItemSelection<rVSync> zvs_t
    (&screen_menu_mode,
     "$screen_vsync_text",
     "$screen_vsync_help",
     currentScreensetting.vSync);

    uSelectEntry<rVSync> zvs_on(zvs_t,"$screen_vsync_on_text","$screen_vsync_on_help",ArmageTron_VSync_On);
    uSelectEntry<rVSync> zvs_d(zvs_t,"$screen_vsync_default_text","$screen_vsync_default_help",ArmageTron_VSync_Default);
    uSelectEntry<rVSync> zvs_off(zvs_t,"$screen_vsync_off_text","$screen_vsync_off_help",ArmageTron_VSync_Off);
#ifdef HAVE_GLEW
    uSelectEntry<rVSync> zvs_blur(zvs_t,"$screen_vsync_motionblur_text","$screen_vsync_motionblur_help",ArmageTron_VSync_MotionBlur);
#endif 
#endif 
#endif 

    uMenuItemToggle gm(
        &screen_menu_mode,
        "$screen_grab_mouse_text",
        "$screen_grab_mouse_help",
        su_mouseGrab);

    uMenuItemSelection<rColorDepth> zd_t
    (&screen_menu_mode,
     "$screen_zdepth_text",
     "$screen_zdepth_help",
     currentScreensetting.zDepth);

    uSelectEntry<rColorDepth> zd_16(zd_t,"$screen_zdepth_16_text","$screen_zdepth_16_help",ArmageTron_ColorDepth_16);
    uSelectEntry<rColorDepth> zd_d(zd_t,"$screen_zdepth_desk_text","$screen_zdepth_desk_help",ArmageTron_ColorDepth_Desktop);
    uSelectEntry<rColorDepth> zd_32(zd_t,"$screen_zdepth_32_text","$screen_zdepth_32_help",ArmageTron_ColorDepth_32);

    uMenuItemSelection<rColorDepth> cd_t
    (&screen_menu_mode,
     "$screen_colordepth_text",
     "$screen_colordepth_help",
     currentScreensetting.colorDepth);

    uSelectEntry<rColorDepth> cd_16(cd_t,"$screen_colordepth_16_text","$screen_colordepth_16_help",ArmageTron_ColorDepth_16);
    uSelectEntry<rColorDepth> cd_d(cd_t,"$screen_colordepth_desk_text","$screen_colordepth_desk_help",ArmageTron_ColorDepth_Desktop);
    uSelectEntry<rColorDepth> cd_32(cd_t,"$screen_colordepth_32_text","$screen_colordepth_32_help",ArmageTron_ColorDepth_32);

    uMenuItemToggle fs_t
    (&screen_menu_mode,
     "$screen_fullscreen_text",
     "$screen_fullscreen_help",
     currentScreensetting.fullscreen);


    gResMenEntry res( screen_menu_mode, currentScreensetting.res, "$screen_resolution_text", "$screen_resolution_help", false );
    gResMenEntry winsize( screen_menu_mode, currentScreensetting.windowSize, "$window_size_text", "$window_size_help", true );

    

    screen_menu_mode.Enter();
}


static uMenuItemSelection<int> mfm
(&screen_menu_detail,
 "$detail_floor_mirror_text",
 "$detail_floor_mirror_help",
 sr_floorMirror);
static uSelectEntry<int> mfma(mfm,"$detail_floor_mirror_off_text","$detail_floor_mirror_off_help",rMIRROR_OFF);


static uSelectEntry<int> mfmb(mfm,"$detail_floor_mirror_obj_text",
                              "$detail_floor_mirror_obj_help",
                              rMIRROR_OBJECTS);

static uSelectEntry<int> mfmc(mfm,"$detail_floor_mirror_ow_text",
                              "$detail_floor_mirror_ow_help",
                              rMIRROR_WALLS);

static uSelectEntry<int> mfme(mfm,"$detail_floor_mirror_ev_text","$detail_floor_mirror_ev_help",rMIRROR_ALL);

static uMenuItemToggle fs_dither
(&screen_menu_detail,"$detail_dither_text",
 "$detail_dither_help",
 sr_dither);

#ifndef DEDICATED

extern bool sg_simpleTrail;
static uMenuItemToggle sgm_simpleTrail
(&screen_menu_detail,
 "$detail_simple_trail_text",
 "$detail_simple_trail_help",
 sg_simpleTrail);
#endif

static uMenuItemSelection<int> mfd
(&screen_menu_detail,
 "$detail_floor_text",
 "$detail_floor_help",
 sr_floorDetail);

static uSelectEntry<int> mfda(mfd,"$detail_floor_no_text",
                              "$detail_floor_no_help",
                              rFLOOR_OFF);
static uSelectEntry<int> mfdb(mfd,"$detail_floor_grid_text",
                              "$detail_floor_grid_help",
                              rFLOOR_GRID);
static uSelectEntry<int> mfdc(mfd,"$detail_floor_tex_text",
                              "$detail_floor_tex_help",
                              rFLOOR_TEXTURE);
static uSelectEntry<int> mfdd(mfd,"$detail_floor_2tex_text",
                              "$detail_floor_2tex_help",
                              rFLOOR_TWOTEXTURE);

static uMenuItemToggle  abm
(&screen_menu_detail,"$detail_alpha_text",
 "$detail_alpha_help",
 sr_alphaBlend);

static uMenuItemToggle  ssm
(&screen_menu_detail,"$detail_smooth_text",
 "$detail_smooth_help",
 sr_smoothShading);

extern bool crash_sparks;		
extern bool white_sparks;		
static tConfItem<bool> cs2("SPARKS",crash_sparks);
static tConfItem<bool> wsp("WHITE_SPARKS",white_sparks);

extern bool sg_crashExplosion;   
static tConfItem<bool> crexp("EXPLOSION",sg_crashExplosion);

#ifndef DEDICATED



static uMenuItemToggle  t32b
(&screen_menu_detail,"$detail_text_truecolor_text",
 "$detail_text_truecolor_help"
 ,sr_texturesTruecolor);


static ArmageTron_texmode_menuitem tmm0(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[0],
                                        rTextureGroups::TextureMode[0]);

static ArmageTron_texmode_menuitem tmm1(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[1],
                                        rTextureGroups::TextureMode[1]);

static ArmageTron_texmode_menuitem tmm2(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[2],
                                        rTextureGroups::TextureMode[2]);

static ArmageTron_texmode_menuitem tmm3(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[3],
                                        rTextureGroups::TextureMode[3],true);


uMenuItemToggle bpt2
(&screen_menu_prefs,"$misc_recording_time_text",
 "$misc_recording_time_help",sr_RecordingTimeOut);

static uMenuItemToggle s2
(&screen_menu_prefs,"$pref_highrim_text",
 "$pref_highrim_help",sr_highRim);

static uMenuItemToggle us2
(&screen_menu_prefs,"$pref_uppersky_text",
 "$pref_uppersky_help",
 sr_upperSky);

static uMenuItemToggle ls2
(&screen_menu_prefs,"$pref_lowersky_text",
 "$pref_lowersky_help",
 sr_lowerSky);

uMenuItemToggle fps2
(&screen_menu_prefs,"$misc_fps_text",
 "$misc_fps_help",sr_FPSOut);

#ifdef USE_HEADLIGHT
static uMenuItemToggle uchl(&screen_menu_prefs,"$pref_headlight_text","$pref_headlight_help",
                            headlights);
#endif


static uMenuItemToggle ws2
(&screen_menu_prefs,"$pref_skymove_text",
 "$pref_skymove_help",
 sr_skyWobble);

static uMenuItemToggle crexp2
(&screen_menu_prefs,"$pref_explosion_text",
 "$pref_explosion_help",
 sg_crashExplosion);

static uMenuItemToggle cs
(&screen_menu_prefs,"$pref_sparks_text",
 "$pref_sparks_help",
 crash_sparks);

static uMenuItemSelection<rDisplayListUsage> dl
(&screen_menu_tweaks,"$tweaks_displaylists_text",
 "$tweaks_displaylists_help", sr_useDisplayLists);
static uSelectEntry<rDisplayListUsage> dl_off(dl,"$tweaks_displaylists_off_text","$tweaks_displaylists_off_help",rDisplayList_Off);
static uSelectEntry<rDisplayListUsage> dl_cac(dl,"$tweaks_displaylists_cac_text","$tweaks_displaylists_cac_help",rDisplayList_CAC);
static uSelectEntry<rDisplayListUsage> dl_cae(dl,"$tweaks_displaylists_cae_text","$tweaks_displaylists_cae_help",rDisplayList_CAE);

static uMenuItemToggle infp
(&screen_menu_tweaks,"$tweaks_infinity_text",
 "$tweaks_infinity_help"
 ,sr_infinityPlane);

uMenuItemSelection<rSysDep::rSwapMode> swapMode
(&screen_menu_tweaks,
 "$swapmode_text",
 "$swapmode_help",
 rSysDep::swapMode_);

static uSelectEntry<rSysDep::rSwapMode> swapMode_fastest(swapMode,"$swapmode_fastest_text","$swapmode_fastest_help",rSysDep::rSwap_Fastest);
static uSelectEntry<rSysDep::rSwapMode> swapMode_glFlush(swapMode,"$swapmode_glflush_text","$swapmode_glflush_help",rSysDep::rSwap_glFlush);
static uSelectEntry<rSysDep::rSwapMode> swapMode_glFinish(swapMode,"$swapmode_glfinish_text","$swapmode_glfinish_help",rSysDep::rSwap_glFinish);



tCONFIG_ENUM( rSysDep::rSwapMode );

static tConfItem< rSysDep::rSwapMode > swapModeCI("SWAP_MODE", rSysDep::swapMode_ );

static tConfItem<REAL> sgs("SPEED_GAUGE_SIZE",subby_SpeedGaugeSize);
static tConfItem<REAL> sgx("SPEED_GAUGE_LOCX",subby_SpeedGaugeLocX);
static tConfItem<REAL> sgy("SPEED_GAUGE_LOCY",subby_SpeedGaugeLocY);

static tConfItem<REAL> bgs("BRAKE_GAUGE_SIZE",subby_BrakeGaugeSize);
static tConfItem<REAL> bgx("BRAKE_GAUGE_LOCX",subby_BrakeGaugeLocX);
static tConfItem<REAL> bgy("BRAKE_GAUGE_LOCY",subby_BrakeGaugeLocY);

static tConfItem<REAL> rgs("RUBBER_GAUGE_SIZE",subby_RubberGaugeSize);
static tConfItem<REAL> rgx("RUBBER_GAUGE_LOCX",subby_RubberGaugeLocX);
static tConfItem<REAL> rgy("RUBBER_GAUGE_LOCY",subby_RubberGaugeLocY);

static tConfItem<bool> showh("SHOW_HUD",subby_ShowHUD);
static tConfItem<bool> showf("SHOW_FASTEST",subby_ShowSpeedFastest);
static tConfItem<bool> shows("SHOW_SCORE",subby_ShowScore);
static tConfItem<bool> showae("SHOW_ALIVE",subby_ShowAlivePeople);
static tConfItem<bool> showp("SHOW_PING",subby_ShowPing);
static tConfItem<bool> showsm("SHOW_SPEED",subby_ShowSpeedMeter);
static tConfItem<bool> showbm("SHOW_BRAKE",subby_ShowBrakeMeter);
static tConfItem<bool> showrm("SHOW_RUBBER",subby_ShowRubberMeter);
static tConfItem<bool> showtim("SHOW_TIME",showTime);
static tConfItem<bool> show24("SHOW_TIME_24",show24hour);

static tConfItem<REAL> scorex("SCORE_LOCX",subby_ScoreLocX);
static tConfItem<REAL> scorey("SCORE_LOCY",subby_ScoreLocY);
static tConfItem<REAL> scores("SCORE_SIZE",subby_ScoreSize);

static tConfItem<REAL> fastx("FASTEST_LOCX",subby_FastestLocX);
static tConfItem<REAL> fasty("FASTEST_LOCY",subby_FastestLocY);
static tConfItem<REAL> fasts("FASTEST_SIZE",subby_FastestSize);

static tConfItem<REAL> aex("ALIVE_LOCX",subby_AlivePeopleLocX);
static tConfItem<REAL> aey("ALIVE_LOCY",subby_AlivePeopleLocY);
static tConfItem<REAL> aes("ALIVE_SIZE",subby_AlivePeopleSize);

static tConfItem<REAL> px("PING_LOCX",subby_PingLocX);
static tConfItem<REAL> py("PING_LOCY",subby_PingLocY);
static tConfItem<REAL> ps("PING_SIZE",subby_PingSize);

uMenuItemToggle hud3
(&hud_prefs,"$pref_showfastest_text",
 "$pref_showfastest_help",subby_ShowSpeedFastest);

uMenuItemToggle mud4
(&hud_prefs,"$pref_showscore_text",
 "$pref_showscore_help",subby_ShowScore);

uMenuItemToggle hud5
(&hud_prefs,"$pref_showenemies_text",
 "$pref_showenemies_help",subby_ShowAlivePeople);

uMenuItemToggle hud6
(&hud_prefs,"$pref_showping_text",
 "$pref_showping_help",subby_ShowPing);

uMenuItemToggle hud7
(&hud_prefs,"$pref_showspeed_text",
 "$pref_showspeed_help",subby_ShowSpeedMeter);

uMenuItemToggle hud8
(&hud_prefs,"$pref_showbrake_text",
 "$pref_showbrake_help",subby_ShowBrakeMeter);

uMenuItemToggle hud9
(&hud_prefs,"$pref_showrubber_text",
 "$pref_showrubber_help",subby_ShowRubberMeter);

uMenuItemToggle hud10
(&hud_prefs,"$pref_showtime_text",
 "$pref_showtime_help",showTime);

uMenuItemToggle hud11
(&hud_prefs,"$pref_show24hour_text",
 "$pref_show24hour_help",show24hour);

uMenuItemToggle hud2
(&hud_prefs,"$pref_showhud_text",
 "$pref_showhud_help",subby_ShowHUD);

static tConfItem<bool> WRAP("WRAP_MENU",uMenu::wrap);

class gMemuItemConsole: public uMenuItemStringWithHistory{
public:
    gMemuItemConsole(uMenu *M,tString &c):
    uMenuItemStringWithHistory(M,"Con:","", c, 1024, sg_consoleHistory, sg_consoleHistoryMaxSize) {}

    virtual ~gMemuItemConsole(){}

    

    virtual bool Event(SDL_Event &e){

        if (e.type==SDL_EVENT_KEY_DOWN &&
                (e.key.key==SDLK_KP_ENTER || e.key.key==SDLK_RETURN)){

            con << tColoredString::ColorString(.5,.5,1) << " > " << *content << '\n';

            
            tCurrentAccessLevel level( tAccessLevel_Owner, true );

            
            std::stringstream s(&((*content)[0]));
            tConfItemBase::LoadAll( s, false );

            MyMenu()->Exit();
            return true;
        }
        else if (e.type==SDL_EVENT_KEY_DOWN &&
                 uActionGlobal::IsBreakingGlobalBind(e.key.scancode))
            return su_HandleEvent(e, true);
        else
            return uMenuItemStringWithHistory::Event(e);
    }
};

void do_con(){
    su_ClearKeys();
        
    se_ChatState( ePlayerNetID::ChatFlags_Console, true );
    sr_con.SetHeight(20,false);
    se_SetShowScoresAuto(false);
    tString c;

    uMenu con_menu("",false);
    gMemuItemConsole s(&con_menu,c);
    con_menu.SetCenter(-.75);
    con_menu.SetBot(-2);
    con_menu.SetTop(-.7);
#ifndef DEDICATED
    SDL_StartTextInput(sr_screen);
#endif
    con_menu.Enter();
#ifndef DEDICATED
    SDL_StopTextInput(sr_screen);
#endif

    se_ChatState( ePlayerNetID::ChatFlags_Console, false );

    se_SetShowScoresAuto(true);
    sr_con.SetHeight(7,false);
}
#endif

void sg_ConsoleInput(){
#ifndef DEDICATED
    st_ToDoOnce(&do_con);
#endif
}















class ArmageTron_viewport_menuitem:public uMenuItemInt{
public:
    ArmageTron_viewport_menuitem(uMenu *m):
            uMenuItemInt(m,"$viewport_menu_title",
                         "$viewport_menu_help",
                         rViewportConfiguration::next_conf_num,
                 0,rViewportConfiguration::s_viewportNumConfigurations-1){
        m->RequestSpaceBelow(.9);
    }

    virtual REAL SpaceRight(){return 1;}

    virtual void RenderBackground(){
        uMenuItem::RenderBackground();

        if (rViewportConfiguration::next_conf_num<0) rViewportConfiguration::next_conf_num=0;
        if (rViewportConfiguration::next_conf_num>=rViewportConfiguration::s_viewportNumConfigurations)
            rViewportConfiguration::next_conf_num=rViewportConfiguration::s_viewportNumConfigurations-1;

        tString  titles[MAX_VIEWPORTS];

        for(int i=MAX_VIEWPORTS-1;i>=0;i--)
            titles[i] << i+1;
#ifndef DEDICATED
        rViewportConfiguration::DemonstrateViewport(titles);
#endif
    }

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){
        if (rViewportConfiguration::next_conf_num<0) rViewportConfiguration::next_conf_num=0;
        if (rViewportConfiguration::next_conf_num>=rViewportConfiguration::s_viewportNumConfigurations)
            rViewportConfiguration::next_conf_num=rViewportConfiguration::s_viewportNumConfigurations-1;

        tOutput disp;

        disp << "$viewport_conf_text";
        disp.AddSpace();
        disp << rViewportConfiguration::s_viewportConfigurationNames[rViewportConfiguration::next_conf_num];
        DisplayText(x-.02,y,disp,selected,alpha);
    }

};



class ArmageTronPlayer_to_viewport_menuitem:public uMenuItemInt{
    int    vp;
public:
    ArmageTronPlayer_to_viewport_menuitem(uMenu *m,int v):
            uMenuItemInt(m,"",
                         "$viewport_assign_help",
                         s_newViewportBelongsToPlayer[v],
                 0,MAX_PLAYERS-1),vp(v){
        m->RequestSpaceBelow(.9);
    }

    virtual REAL SpaceRight(){return 1;}

    virtual void LeftRight(int x){
        rViewport::SetDirectionOfCorrection(vp,x);
        target=(target + MAX_PLAYERS + x) % MAX_PLAYERS;
    }

    virtual void RenderBackground(){
        rViewport::CorrectViewport(vp, MAX_PLAYERS);

        uMenuItem::RenderBackground();

        tString titles[MAX_VIEWPORTS];
        for(int i=MAX_VIEWPORTS-1;i>=0;i--){
            titles[i] << s_newViewportBelongsToPlayer[i]+1;
            titles[i] << " (";
            titles[i] << ePlayer::PlayerConfig(s_newViewportBelongsToPlayer[i])->Name();
            titles[i] << ")";
        }
#ifndef DEDICATED
        rViewportConfiguration::DemonstrateViewport(titles);
#endif
    }

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){

        tOutput disp;

        disp.SetTemplateParameter(1, vp +1);
        disp.SetTemplateParameter(2, s_newViewportBelongsToPlayer[vp]+1);
        disp.SetTemplateParameter(3, ePlayer::PlayerConfig(s_newViewportBelongsToPlayer[vp])->Name());
        disp << "$viewport_belongs_text";

        DisplayText(x-.02,y,disp,selected,alpha);
    }

};

#include "rSysdep.h"
extern void Render(int);


class ArmageTron_color_menuitem:public uMenuItemInt{
protected:
    int *rgb;
    unsigned short me;
public:
    ArmageTron_color_menuitem(uMenu *m,const char *tit,
                              const char *help, int *RGB,int Me)
            :uMenuItemInt(m,tit,help,RGB[Me],0,15),
    rgb(RGB),me(Me) {
        m->RequestSpaceBelow(.2);
    }

    ~ArmageTron_color_menuitem(){}

    virtual REAL SpaceRight(){return .2;}


    virtual void RenderBackground(){
        
        
#ifndef DEDICATED
        uMenuItem::RenderBackground();
        if (!sr_glOut)
            return;
        REAL r = rgb[0]/15.0;
        REAL g = rgb[1]/15.0;
        REAL b = rgb[2]/15.0;
        se_MakeColorValid(r, g, b, 1.0f);
        RenderEnd();
        glColor3f(r, g, b);
        glRectf(.8,-.8,.98,-.98);
#endif
    }

};



void sg_PlayerMenu(int Player){
    tOutput name;
    name.SetTemplateParameter(1, Player+1);

    name << "$player_menu_text";


    uMenu playerMenu(name);

    uMenu camera_menu("$player_camera_text");
    uMenu chat_menu("$player_chat_text");
    
    chat_menu.SetCenter(-.5);

    uMenuItemString *ic[MAX_INSTANT_CHAT];

    ePlayer *p = ePlayer::PlayerConfig(Player);
    if (!p)
        return;

    int i;
    for(i=MAX_INSTANT_CHAT-1;i>=0;i--){
        tOutput name;
        name.SetTemplateParameter(1, i+1);
        name << "$player_chat_chat";
        ic[i]=new uMenuItemString
              (&chat_menu,name,
               "$player_chat_chat_help",
               p->instantChatString[i], se_SpamMaxLen);
    }

    uMenuItemToggle al
    (&playerMenu,"$player_autologin_text",
     "$player_autologin_help",
     p->autoLogin);

    uMenuItemString gid(&playerMenu,
                      "$player_global_id_text",
                      "$player_global_id_help",
                      p->globalID, 400);
    gid.SetColorMode( rTextField::COLOR_IGNORE );

    uMenuItemToggle st
    (&playerMenu,"$player_stealth_text",
     "$player_stealth_help",
     p->stealth);

    uMenuItemToggle sp
    (&playerMenu,"$player_spectator_text",
     "$player_spectator_help",
     p->spectate);

    uMenuItemToggle pnt
    (&playerMenu,"$player_name_team_text",
     "$player_name_team_help",
     p->nameTeamAfterMe);

    uMenuItemInt npt
    (&playerMenu,"$player_num_per_team_text",
     "$player_num_per_team_help",
     p->favoriteNumberOfPlayersPerTeam, 1, 16, 1);

    ArmageTron_color_menuitem B(&playerMenu,"$player_blue_text",
                                "$player_blue_help",
                                p->rgb,2);

    ArmageTron_color_menuitem G(&playerMenu,"$player_green_text",
                                "$player_green_help",
                                p->rgb,1);

    ArmageTron_color_menuitem R(&playerMenu,"$player_red_text",
                                "$player_red_help",
                                p->rgb,0);



    uMenuItemSubmenu chm(&playerMenu,&chat_menu,
                         "$player_chat_chat_help");

    uMenuItemSubmenu cm(&playerMenu,&camera_menu,
                        "$player_camera_help");

    uMenuItemFunctionInt icc(&playerMenu,"$player_camera_input_text",
                             "$player_camera_input_help",
                             &su_InputConfigCamera,Player);

    uMenuItemFunctionInt inc(&playerMenu,"$player_input_text",
                             "$player_input_help",
                             &su_InputConfig,Player);


    camera_menu.SetCenter(.3);

    uMenuItemToggle cam_glance
    (&camera_menu,
     "$camera_smart_glance_custom_text",
     "$camera_smart_glance_custom_help",
     p->smartCustomGlance);

    uMenuItemToggle cis(&camera_menu,
                        "$player_camera_autoin_text",
                        "$player_camera_autoin_help",
                        p->autoSwitchIncam);

    uMenuItemToggle cim(&camera_menu,
                        "$player_camera_wobble_text",
                        "$player_camera_wobble_help",
                        p->wobbleIncam);

    uMenuItemToggle cic(&camera_menu,
                        "$player_camera_center_int_text",
                        "$player_camera_center_int_help",
                        p->centerIncamOnTurn);

    uMenuItemToggle al_s
    (&camera_menu,
     "$player_camera_smartcam_text",
     "$player_camera_smartcam_help",
     p->allowCam[CAMERA_SMART]);

    uMenuItemToggle al_f
    (&camera_menu,
     "$player_camera_fixed_text",
     "$player_camera_fixed_help",
     p->allowCam[CAMERA_FOLLOW]);


    uMenuItemToggle al_fr
    (&camera_menu,
     "$player_camera_free_text",
     "$player_camera_free_help",
     p->allowCam[CAMERA_FREE]);

    uMenuItemToggle al_c
    (&camera_menu,
     "$player_camera_custom_text",
     "$player_camera_custom_help",
     p->allowCam[CAMERA_CUSTOM]);

    uMenuItemToggle al_sc
    (&camera_menu,
     "$player_camera_server_custom_text",
     "$player_camera_server_custom_help",
     p->allowCam[CAMERA_SERVER_CUSTOM]);

    uMenuItemToggle al_i
    (&camera_menu,
     "$player_camera_incam_text",
     "$player_camera_incam_help",
     p->allowCam[CAMERA_IN]);


    uMenuItemInt cam_fov
    (&camera_menu,
     "$player_camera_fov_text",
     "$player_camera_fov_help",
     p->startFOV,30,160,5);

    uMenuItemSelection<eCamMode> cam_s
    (&camera_menu,
     "$player_camera_initial_text",
     "$player_camera_initial_help",
     p->startCamera);

    cam_s.NewChoice("$player_camera_initial_scust_text","$player_camera_initial_scust_help",CAMERA_SERVER_CUSTOM);
    cam_s.NewChoice("$player_camera_initial_cust_text","$player_camera_initial_cust_help",CAMERA_CUSTOM);
    cam_s.NewChoice("$player_camera_initial_int_text","$player_camera_initial_int_help",CAMERA_IN);
    cam_s.NewChoice("$player_camera_initial_smrt_text","$player_camera_initial_smrt_help",CAMERA_SMART);
    cam_s.NewChoice("$player_camera_initial_ext_text","$player_camera_initial_ext_help",CAMERA_FOLLOW);
    cam_s.NewChoice("$player_camera_initial_free_text","$player_camera_initial_free_help",CAMERA_FREE);

    uMenuItemString n(&playerMenu,
                      "$player_name_text",
                      "$player_name_help",
                      p->name, 16);

    playerMenu.Enter();

    for(i=MAX_INSTANT_CHAT-1; i>=0; i--)
        delete ic[i];


    
    static nVersionFeature inGameRenames( 5 );
    if ( inGameRenames.Supported() )
    {
        ePlayerNetID::Update();
        ePlayer::SendAuthNames();
    }

    
}


VOIDFUNC viewport_menu_x;










void sg_PlayerMenu(){
    uMenu Player_men("$player_mainmenu_text");


    uMenuItemFunction vp_selec(&Player_men,
                               "$viewport_assign_text",
                               "$viewport_assign_help",
                               &viewport_menu_x);

    ArmageTron_viewport_menuitem vp(&Player_men);
    uMenuItemFunctionInt  *names[MAX_PLAYERS];

    int i;

    for(i=MAX_PLAYERS-1;i>=0;i--){
        tOutput title;
        title.SetTemplateParameter(1, i+1);
        title << "$player_menu_text";

        tOutput help;
        help.SetTemplateParameter(1, i+1);
        help << "$player_menu_help";

        names[i]=new uMenuItemFunctionInt(&Player_men,
                                          title,
                                          help,
                                          sg_PlayerMenu,i);
    }


    Player_men.Enter();

    
    for(i=MAX_VIEWPORTS-1;i>=0;i--){
        delete names[i];
    }
}




void viewport_menu_x(void){
    uMenu sg_PlayerMenu("$viewport_assign_text");

    ArmageTronPlayer_to_viewport_menuitem *select[MAX_VIEWPORTS];

    int i;
    for(i=rViewportConfiguration::s_viewportConfigurations[rViewportConfiguration::next_conf_num]->num_viewports-1;i>=0;i--){
        select[i]=new ArmageTronPlayer_to_viewport_menuitem(&sg_PlayerMenu,i);
    }

    

    sg_PlayerMenu.Enter();

    for(i=rViewportConfiguration::s_viewportConfigurations[rViewportConfiguration::next_conf_num]->num_viewports-1;i>=0;i--){
        delete select[i];
    }
}


static uActionGlobal con_input( "CONSOLE_INPUT" );


static uActionGlobal screenshot( "SCREENSHOT" );

static uActionGlobal togglefullscreen( "TOGGLE_FULLSCREEN" );

static bool screenshot_func(REAL x){
    if (x>0){
#ifndef DEDICATED
        sr_screenshotIsPlanned=true;
#endif
    }

    return true;
}

static bool con_func(REAL x){
    if (x>0){
        sg_ConsoleInput();
    }

    return true;
}

static bool toggle_fullscreen_func( REAL x )
{
#ifndef DEDICATED
#ifdef DEBUG
    
    if ( tRecorder::IsPlayingBack() )
        return true;
#endif

    
    if ( x > 0 && ( SDL_GetWindowFlags(sr_screen) & SDL_WINDOW_OCCLUDED ) )
    {
        currentScreensetting.fullscreen = !currentScreensetting.fullscreen;
        sr_ReinitDisplay();
    }
#endif

    return true;
}

static uActionGlobalFunc gaf_ss(&screenshot,&screenshot_func, true );
static uActionGlobalFunc gaf_md(&con_input,&con_func);
static uActionGlobalFunc gaf_tf(&togglefullscreen,&toggle_fullscreen_func, true );




#undef glBegin
#undef glEnd
#undef glMatrixMode

#include "../render/rGL.h"
#include <math.h>
#include "rTexture.h"
#include "ModMenu.h"

bool cVisualMenu::isOpen = false;
float cVisualMenu::menuAlpha = 0.0f;

void cVisualMenu::Init() {
    ModMenu::Init();
}

void cVisualMenu::Toggle() {
    ModMenu::Toggle();
    isOpen = ModMenu::IsOpen();
}

void cVisualMenu::Render() {
    ModMenu::Render();
}

bool cVisualMenu::HandleEvent(const SDL_Event& event) {
    return ModMenu::ProcessEvent(&event);
}

