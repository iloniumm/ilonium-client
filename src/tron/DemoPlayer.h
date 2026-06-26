// =========================================================================
// DemoPlayer.h — AAREC Demo Playback System
// RetroCycles / Armagetron Advanced client mod
//
// Reads .aarec files recorded by DemoRecorder and replays them via a
// soft-simulation: the player reconstructs game state in memory without
// actually running the network stack.
//
// Architecture
// ─────────────
//   LoadFile()  → reads entire file into memory, builds RoundIndex
//   Play/Pause  → driven from ModMenu / HUD each frame
//   Tick(dt)    → advances playback cursor, applies scaled dt
//   Render HUD  → bottom transport bar + freecam overlay
//
// =========================================================================
#pragma once
#ifndef DEDICATED

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cmath>

#include "DemoRecorder.h"   // re-uses packet structs / AarecPacketType

// ── Playback state per cycle (reconstructed each frame) ──────────────────
struct DemoPlayerCycleState {
    int32_t playerSlot = -1;
    float   posX = 0.f, posY = 0.f;
    float   dirX = 1.f, dirY = 0.f;
    float   speed = 0.f;
    float   rubber = 0.f;
    uint8_t alive = 0;
    char    name[64] = {};          // filled in from first seen playerSlot
};

// ── One entry in the Round Index ─────────────────────────────────────────
struct DemoRoundEntry {
    size_t  byteOffset;     ///< byte offset in file_data_ where RoundBegin lives
    double  gameTime;       ///< game-time of the RoundBegin event
    int     roundNumber;    ///< 1-based
};

// ── Freecam state ─────────────────────────────────────────────────────────
struct DemoFreecamState {
    float x = 0.f, y = 0.f, z = 8.f;  // world position
    float yaw = 0.f, pitch = -0.3f;    // radians
    float speed = 15.f;                 // units/s
};

// ── Camera modes ─────────────────────────────────────────────────────────
enum class DemoCamMode {
    Spectator = 0,  // follow a cycle, use engine camera
    Freecam   = 1,  // free-flight, WASD+mouse
};

// ── Playback speed presets ────────────────────────────────────────────────
static constexpr float DEMO_SPEED_PRESETS[] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f };
static constexpr int   DEMO_SPEED_COUNT     = 5;
static constexpr int   DEMO_SPEED_NORMAL    = 2;  // index of 1.0×

// ── DemoPlayerManager ─────────────────────────────────────────────────────
class DemoPlayerManager {
public:
    // ── Singleton ─────────────────────────────────────────────────────
    static DemoPlayerManager& Instance();
    DemoPlayerManager(const DemoPlayerManager&) = delete;
    DemoPlayerManager& operator=(const DemoPlayerManager&) = delete;

    // ── File lifecycle ────────────────────────────────────────────────
    /// Load .aarec file into memory and build round index.
    /// Returns true on success.
    bool LoadFile(const std::string& path);

    /// Unload and reset all state.
    void Unload();

    bool IsLoaded()       const { return !file_data_.empty(); }
    bool IsPlaying()      const { return is_playing_ && !is_paused_; }
    bool IsPaused()       const { return is_paused_; }
    bool IsViewerActive() const { return is_viewer_active_; }

    const std::string& FilePath() const { return file_path_; }

    // ── Transport controls ────────────────────────────────────────────
    void Play();
    void Pause();
    void TogglePlayPause();

    /// Step exactly one packet forward (single-packet step while paused).
    void StepForward();

    /// Seek to absolute game-time (handles backward seek via round-start).
    void SeekToTime(double targetTime);

    /// Jump to round by index (0-based).
    void SeekToRound(int roundIndex);

    void PrevRound();
    void NextRound();

    // ── Time control ──────────────────────────────────────────────────
    int   SpeedIndex()    const { return speed_index_; }
    float PlaybackSpeed() const { return DEMO_SPEED_PRESETS[speed_index_]; }
    void  SetSpeedIndex(int idx);
    void  SpeedUp();
    void  SlowDown();

    // ── Per-frame update (call from render loop) ──────────────────────
    /// Advance playback by real_dt seconds (scaled internally).
    void Tick(float real_dt);
    void SyncGameObjects();
    void ClearGridObjects();

    // ── Query current state ───────────────────────────────────────────
    double CurrentTime()   const { return current_time_; }
    double TotalDuration() const { return total_duration_; }
    int    CurrentRound()  const { return current_round_; }
    int    TotalRounds()   const { return (int)round_index_.size(); }

    const std::vector<DemoPlayerCycleState>& CycleStates() const { return cycle_states_; }

    /// Round index entries (for round-marker rendering / count checks).
    const std::vector<DemoRoundEntry>& GetRoundIndex() const { return round_index_; }

    // ── Camera / spectator ────────────────────────────────────────────
    DemoCamMode CamMode() const { return cam_mode_; }
    void        SetCamMode(DemoCamMode m) { cam_mode_ = m; }
    void        ToggleCamMode();

    int  SpectatorTarget()    const { return spectator_target_; }
    void SetSpectatorTarget(int slot);
    void NextSpectatorTarget();
    void PrevSpectatorTarget();

    DemoFreecamState& Freecam() { return freecam_; }

    /// Must be called from the render/input loop to update freecam.
    /// Returns true if freecam consumed the event (caller should not pass to game).
    void FreecamInput(float dt, float dx_mouse, float dy_mouse,
                      bool fwd, bool back, bool left, bool right,
                      bool up, bool down);

    /// Fill pos/dir/z for eCamera::Render() override when in Freecam mode.
    void GetFreecamTransform(float& ox, float& oy, float& oz,
                             float& dx, float& dy) const;

    // ── HUD rendering ─────────────────────────────────────────────────
    /// Renders the bottom transport bar HUD using ImGui.
    void RenderHUD();

    /// Self-contained SDL/GL loop that renders the demo player UI.
    /// Blocks until the user exits. Call from the main menu loop.
    void RunDemoViewerLoop();

    bool  ExitRequested() const { return hud_exit_requested_; }
    void  StartViewer3D();
    void  StopViewer3D();

    struct TrailPt { float x, y; };
    const std::vector<std::vector<TrailPt>>& Trails() const { return trails_; }

    // ── Global playback speed scalar (for gCycle::Timestep hook) ─────
    /// Returns the current effective dt multiplier.
    /// Use: effective_dt = real_dt * DemoPlayerManager::Instance().DtScale();
    float DtScale() const;

private:
    DemoPlayerManager();
    ~DemoPlayerManager() = default;

    // ── Internal parser ───────────────────────────────────────────────
    void BuildRoundIndex();

    /// Read next packet from read_cursor_, fill out the live state.
    /// Returns false when end-of-data reached.
    bool AdvanceOneCycle();

    /// Fast-forward without rendering from the given byte offset
    /// until target_time is reached (for backward seek).
    void FastForwardFrom(size_t byteOffset, double targetRelativeTime);

    // Raw file helpers (little-endian)
    template<typename T> T Read();
    void Skip(size_t n);
    bool AtEnd() const { return read_cursor_ >= file_data_.size(); }

    // ── State ─────────────────────────────────────────────────────────
    std::string              file_path_;
    std::vector<uint8_t>     file_data_;     ///< entire file in RAM
    size_t                   read_cursor_ = 0;
    size_t                   data_start_  = 0; ///< offset past FileHeader

    double  current_time_   = 0.0;  ///< relative time from demo start
    double  total_duration_ = 0.0;  ///< total relative duration
    double  start_time_offset_ = 0.0; ///< absolute gametime of first packet
    int     current_round_  = 0;   ///< 0-based

    bool    is_playing_ = false;
    bool    is_paused_  = false;

    int     speed_index_ = DEMO_SPEED_NORMAL;

    // Round index
    std::vector<DemoRoundEntry>        round_index_;

    // Live reconstructed state
    std::vector<DemoPlayerCycleState>  cycle_states_;

    // Camera
    DemoCamMode      cam_mode_        = DemoCamMode::Spectator;
    int              spectator_target_= 0;   ///< index into cycle_states_
    DemoFreecamState freecam_;

    // HUD state
    bool  hud_seeking_        = false;
    float hud_seek_val_        = 0.f;
    bool  is_viewer_active_    = false;
    bool  hud_exit_requested_  = false; ///< set by EXIT button in RenderHUD, checked in RunDemoViewerLoop

    std::vector<std::vector<TrailPt>> trails_;
};

// Convenience alias
namespace DemoPlay {
    inline DemoPlayerManager& Get() { return DemoPlayerManager::Instance(); }
}

#endif // !DEDICATED
