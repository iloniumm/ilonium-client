// =========================================================================
// DemoPlayer.cpp — AAREC Demo Playback System (Implementation)
// =========================================================================

#ifndef DEDICATED

#include "DemoPlayer.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/time.h>    // gettimeofday
#include <fstream>


// Engine / ImGui
#include "imgui.h"
#include "HudManager.h"     // sg_demoRecorderOverlayEnabled
#include "gGame.h"
#include "gCycle.h"
#include "eGrid.h"
#include "ePlayer.h"
#include "gWall.h"
#include "uInput.h"          // su_ClearKeys
#include "render/rSysdep.h"  // rSysDep::SwapGL / ClearGL
#include "render/rScreen.h"  // sr_screen
#include "rSDL.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_sdl3.h"

// ── Singleton ────────────────────────────────────────────────────────────
DemoPlayerManager& DemoPlayerManager::Instance() {
    static DemoPlayerManager inst;
    return inst;
}

DemoPlayerManager::DemoPlayerManager() {}

// ── Read helpers ─────────────────────────────────────────────────────────
template<typename T>
T DemoPlayerManager::Read() {
    if (read_cursor_ + sizeof(T) > file_data_.size())
        return T{};
    T v;
    std::memcpy(&v, file_data_.data() + read_cursor_, sizeof(T));
    read_cursor_ += sizeof(T);
    return v;
}
void DemoPlayerManager::Skip(size_t n) {
    read_cursor_ = std::min(read_cursor_ + n, file_data_.size());
}

// ── LoadFile ─────────────────────────────────────────────────────────────
bool DemoPlayerManager::LoadFile(const std::string& path) {
    Unload();

    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return false;
    }

    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    if (sz <= 0) { fclose(f); return false; }

    file_data_.resize((size_t)sz);
    if (fread(file_data_.data(), 1, (size_t)sz, f) != (size_t)sz) {
        fclose(f);
        file_data_.clear();
        return false;
    }
    fclose(f);

    file_path_    = path;
    read_cursor_  = 0;

    // Validate magic + version
    if (file_data_.size() < 8) { Unload(); return false; }
    if (std::memcmp(file_data_.data(), AAREC_MAGIC, 6) != 0) {
        Unload(); return false;
    }
    // Skip magic (6) + version (2)
    read_cursor_ = 8;

    // Skip FileHeader payload: matchId string (uint8 length-prefixed)
    // Format: type(u8) | gameTime(f64) | matchId(u8 len + chars)
    // First packet should be FileHeader type
    uint8_t ptype = Read<uint8_t>();
    if (ptype != (uint8_t)AarecPacketType::FileHeader) {
        // tolerate: reset and scan
    }
    // skip game time embedded in header
    Skip(8);
    // skip matchId: read the length byte then skip
    {
        uint8_t len = Read<uint8_t>();
        Skip(len);
    }

    data_start_ = read_cursor_;

    // IMPORTANT: compute start_time_offset_ FIRST so BuildRoundIndex stores relative times
    {
        size_t saved = read_cursor_;
        read_cursor_ = data_start_;
        double first_t = -1.0, last_t = 0.0;
        while (!AtEnd()) {
            uint8_t t = Read<uint8_t>();
            if (t == (uint8_t)AarecPacketType::FileEnd) {
                goto done_scan;
            }
            double gt  = Read<double>();
            if (first_t < 0.0) first_t = gt;
            last_t = gt;
            switch ((AarecPacketType)t) {
            case AarecPacketType::FrameBegin:  Skip(4+4); break;
            case AarecPacketType::CycleState:  Skip(26); break;
            case AarecPacketType::ChatEvent:   {
                Skip(1); // senderSlot
                uint16_t len = Read<uint16_t>();
                Skip(len);
                break;
            }
            case AarecPacketType::KillEvent:   Skip(2); break;
            case AarecPacketType::TurnEvent:   Skip(2); break;
            case AarecPacketType::RoundBegin:
            case AarecPacketType::RoundEnd:    break;
            default: goto done_scan;
            }
        }
        done_scan:
        start_time_offset_ = (first_t >= 0.0) ? first_t : 0.0;
        total_duration_ = last_t - start_time_offset_;
        if (total_duration_ < 0.0) total_duration_ = 0.0;
        read_cursor_ = saved;
    }

    // NOW build round index — start_time_offset_ is valid
    BuildRoundIndex();

    read_cursor_  = data_start_;
    current_time_ = 0.0;
    is_playing_   = false;
    is_paused_    = true;

    trails_.clear();
    trails_.resize(16);

    return true;
}

// ── BuildRoundIndex ───────────────────────────────────────────────────────
void DemoPlayerManager::BuildRoundIndex() {
    round_index_.clear();
    size_t saved = read_cursor_;
    read_cursor_ = data_start_;
    int rnum = 0;

    while (!AtEnd()) {
        size_t pktOffset = read_cursor_;
        uint8_t t = Read<uint8_t>();
        if (t == (uint8_t)AarecPacketType::FileEnd) {
            goto done;
        }
        double  gt = Read<double>();

        switch ((AarecPacketType)t) {
        case AarecPacketType::RoundBegin: {
            DemoRoundEntry e;
            e.byteOffset  = pktOffset;
            e.gameTime    = gt - start_time_offset_;  // store relative time
            e.roundNumber = ++rnum;
            round_index_.push_back(e);
            break;
        }
        case AarecPacketType::FrameBegin:  Skip(4+4); break;
        case AarecPacketType::CycleState:  Skip(26); break;
        case AarecPacketType::ChatEvent:   {
            Skip(1); // senderSlot
            uint16_t len = Read<uint16_t>();
            Skip(len);
            break;
        }
        case AarecPacketType::KillEvent:   Skip(2); break;
        case AarecPacketType::TurnEvent:   Skip(2); break;
        case AarecPacketType::RoundEnd:    break;
        default:                           goto done;
        }
    }
    done:
    read_cursor_ = saved;
}

// ── Unload ────────────────────────────────────────────────────────────────
void DemoPlayerManager::Unload() {
    file_data_.clear();
    file_path_.clear();
    round_index_.clear();
    cycle_states_.clear();
    trails_.clear();
    read_cursor_       = 0;
    data_start_        = 0;
    current_time_      = 0.0;
    total_duration_    = 0.0;
    start_time_offset_ = 0.0;
    current_round_     = 0;
    is_playing_        = false;
    is_paused_         = false;
}

// ── Transport ─────────────────────────────────────────────────────────────
void DemoPlayerManager::Play()  { if (IsLoaded()) { is_playing_ = true; is_paused_ = false; } }
void DemoPlayerManager::Pause() { is_paused_ = true; }
void DemoPlayerManager::TogglePlayPause() {
    if (!IsLoaded()) return;
    if (!is_playing_) { Play(); return; }
    is_paused_ = !is_paused_;
}

void DemoPlayerManager::SetSpeedIndex(int idx) {
    speed_index_ = std::max(0, std::min(idx, DEMO_SPEED_COUNT - 1));
}
void DemoPlayerManager::SpeedUp()   { SetSpeedIndex(speed_index_ + 1); }
void DemoPlayerManager::SlowDown()  { SetSpeedIndex(speed_index_ - 1); }

float DemoPlayerManager::DtScale() const {
    if (!is_playing_ || is_paused_) return 0.f;
    return DEMO_SPEED_PRESETS[speed_index_];
}

// ── AdvanceOneCycle ───────────────────────────────────────────────────────
// Process exactly one packet, update live state. Returns false at EOF.
bool DemoPlayerManager::AdvanceOneCycle() {
    if (AtEnd()) return false;

    size_t saved = read_cursor_;
    uint8_t ptype = Read<uint8_t>();
    {
        std::ofstream logFile("/tmp/retrocycles_demo.log", std::ios::app);
        logFile << "    AdvanceOneCycle: read_cursor=" << saved << " ptype=" << (int)ptype << std::endl;
    }
    if (ptype == (uint8_t)AarecPacketType::FileEnd) {

        is_paused_ = true;
        return false;
    }
    double  gt    = Read<double>();

    switch ((AarecPacketType)ptype) {

    case AarecPacketType::FrameBegin: {
        float dt  = Read<float>();
        int   tck = Read<int32_t>();
        (void)tck;
        // Store relative time (subtract start offset so display shows 0:00 at start)
        current_time_ = gt - start_time_offset_;
        break;
    }

    case AarecPacketType::CycleState: {
        // Read fields individually to match serialization!
        uint8_t pSlot  = Read<uint8_t>();
        float   posX   = Read<float>();
        float   posY   = Read<float>();
        float   dirX   = Read<float>();
        float   dirY   = Read<float>();
        float   speed  = Read<float>();
        float   rubber = Read<float>();
        uint8_t alive  = Read<uint8_t>();

        // Find or create slot
        DemoPlayerCycleState* found = nullptr;
        for (auto& s : cycle_states_)
            if (s.playerSlot == (int32_t)pSlot) { found = &s; break; }
        if (!found) {
            cycle_states_.emplace_back();
            found = &cycle_states_.back();
            found->playerSlot = (int32_t)pSlot;
            snprintf(found->name, sizeof(found->name), "Player %d", (int32_t)pSlot);
        }
        found->posX   = posX;
        found->posY   = posY;
        found->dirX   = dirX;
        found->dirY   = dirY;
        found->speed  = speed;
        found->rubber = rubber;
        found->alive  = alive;

        if (pSlot < trails_.size()) {
            auto& tr = trails_[pSlot];
            if (!alive) {
                tr.clear();
            } else {
                if (tr.empty() || fabsf(tr.back().x - posX) > 0.3f || fabsf(tr.back().y - posY) > 0.3f) {
                    if (tr.size() >= 512) {
                        tr.erase(tr.begin());
                    }
                    tr.push_back({posX, posY});
                }
            }
        }
        break;
    }

    case AarecPacketType::ChatEvent: {
        Skip(1); // senderSlot
        uint16_t len = Read<uint16_t>();
        Skip(len);
        break;
    }
    case AarecPacketType::KillEvent:  Skip(2); break;
    case AarecPacketType::TurnEvent:  Skip(2); break;

    case AarecPacketType::RoundBegin:
        for (auto& tr : trails_) {
            tr.clear();
        }
        // Update current_round_ counter
        for (int i = 0; i < (int)round_index_.size(); ++i) {
            if (round_index_[i].byteOffset == saved) {
                current_round_ = i;
                break;
            }
        }
        break;

    case AarecPacketType::RoundEnd:   break;

    case AarecPacketType::FileEnd:
        is_paused_ = true;
        return false;

    default:
        // Unknown packet: cannot safely skip, stop
        is_paused_ = true;
        return false;
    }
    return true;
}

// ── SeekToTime ────────────────────────────────────────────────────────────
void DemoPlayerManager::SeekToTime(double targetTime) {
    if (!IsLoaded()) return;

    // targetTime is relative (0 = demo start)
    targetTime = std::max(0.0, std::min(targetTime, total_duration_));

    // Find the last round that started before targetTime
    int roundToUse = 0;
    for (int i = (int)round_index_.size() - 1; i >= 0; --i) {
        if (round_index_[i].gameTime <= targetTime) {  // gameTime is already relative
            roundToUse = i;
            break;
        }
    }

    double absTarget = targetTime + start_time_offset_;
    double absCurrentTime = current_time_ + start_time_offset_;

    bool needReSimulate = (absTarget < absCurrentTime) || (roundToUse != current_round_);

    if (needReSimulate) {
        ClearGridObjects();
        for (auto& tr : trails_) {
            tr.clear();
        }

        size_t startOff = round_index_.empty()
            ? data_start_
            : round_index_[roundToUse].byteOffset;

        cycle_states_.clear();
        current_time_ = round_index_.empty() ? 0.0 : round_index_[roundToUse].gameTime;
        current_round_ = roundToUse;
        FastForwardFrom(startOff, targetTime);
    } else {
        FastForwardFrom(read_cursor_, targetTime);
    }
    SyncGameObjects();
}

void DemoPlayerManager::FastForwardFrom(size_t byteOffset, double targetRelativeTime) {
    read_cursor_ = byteOffset;
    while (!AtEnd() && current_time_ < targetRelativeTime) {
        if (!AdvanceOneCycle()) break;
    }
}

// ── SeekToRound ──────────────────────────────────────────────────────────
void DemoPlayerManager::SeekToRound(int idx) {
    if (!IsLoaded() || round_index_.empty()) return;
    ClearGridObjects();
    for (auto& tr : trails_) {
        tr.clear();
    }
    idx = std::max(0, std::min(idx, (int)round_index_.size() - 1));
    double relT = round_index_[idx].gameTime;  // already relative
    cycle_states_.clear();
    read_cursor_  = round_index_[idx].byteOffset;
    current_time_ = relT;
    current_round_ = idx;
    SyncGameObjects();
}

void DemoPlayerManager::PrevRound() { SeekToRound(current_round_ - 1); }
void DemoPlayerManager::NextRound() { SeekToRound(current_round_ + 1); }

// ── StepForward ──────────────────────────────────────────────────────────
void DemoPlayerManager::StepForward() {
    if (!IsLoaded()) return;
    // Advance until the next FrameBegin packet (one full tick)
    double startT = current_time_;
    while (!AtEnd()) {
        if (!AdvanceOneCycle()) break;
        if (current_time_ > startT) break;
    }
    SyncGameObjects();
}

void DemoPlayerManager::Tick(float real_dt) {
    if (!IsLoaded()) return;

    {
        std::ofstream logFile("/tmp/retrocycles_demo.log", std::ios::app);
        logFile << "Tick: real_dt=" << real_dt
                << " playing=" << is_playing_
                << " paused=" << is_paused_
                << " current_time=" << current_time_
                << " read_cursor=" << read_cursor_
                << " total_duration=" << total_duration_
                << std::endl;
    }

    if (is_playing_ && !is_paused_) {
        float scaled_dt = real_dt * PlaybackSpeed();
        double targetTime = current_time_ + (double)scaled_dt;

        if (targetTime >= total_duration_) {
            targetTime = total_duration_;
            is_paused_ = true;
        }

        // Fast-forward packet stream to targetTime (relative)
        while (!AtEnd() && current_time_ < targetTime) {
            if (!AdvanceOneCycle()) break;
        }
    }
    SyncGameObjects();
}

// ── Camera / Spectator ────────────────────────────────────────────────────
void DemoPlayerManager::ToggleCamMode() {
    cam_mode_ = (cam_mode_ == DemoCamMode::Spectator)
              ? DemoCamMode::Freecam
              : DemoCamMode::Spectator;
}

void DemoPlayerManager::SetSpectatorTarget(int slot) {
    spectator_target_ = slot;
}

void DemoPlayerManager::NextSpectatorTarget() {
    if (cycle_states_.empty()) return;
    spectator_target_ = (spectator_target_ + 1) % (int)cycle_states_.size();
}

void DemoPlayerManager::PrevSpectatorTarget() {
    if (cycle_states_.empty()) return;
    spectator_target_ = (spectator_target_ - 1 + (int)cycle_states_.size())
                      % (int)cycle_states_.size();
}

// ── Freecam Input ─────────────────────────────────────────────────────────
void DemoPlayerManager::FreecamInput(float dt, float dx_mouse, float dy_mouse,
                                     bool fwd, bool back,
                                     bool left, bool right,
                                     bool up, bool down) {
    // Mouse look
    freecam_.yaw   += dx_mouse * 0.003f;
    freecam_.pitch -= dy_mouse * 0.003f;
    freecam_.pitch  = std::max(-1.4f, std::min(1.4f, freecam_.pitch));

    // Local direction vectors (yaw only for WASD)
    float cy = cosf(freecam_.yaw), sy = sinf(freecam_.yaw);
    // Forward in XY plane
    float fx =  cy, fy = sy;
    // Right
    float rx = -sy, ry = cy;

    float spd = freecam_.speed * dt;
    if (fwd)   { freecam_.x += fx * spd; freecam_.y += fy * spd; }
    if (back)  { freecam_.x -= fx * spd; freecam_.y -= fy * spd; }
    if (right) { freecam_.x += rx * spd; freecam_.y += ry * spd; }
    if (left)  { freecam_.x -= rx * spd; freecam_.y -= ry * spd; }
    if (up)    { freecam_.z += spd; }
    if (down)  { freecam_.z -= spd; freecam_.z = std::max(0.5f, freecam_.z); }
}

void DemoPlayerManager::GetFreecamTransform(float& ox, float& oy, float& oz,
                                            float& dx, float& dy) const {
    ox = freecam_.x;
    oy = freecam_.y;
    oz = freecam_.z;
    dx = cosf(freecam_.yaw);
    dy = sinf(freecam_.yaw);
}

// ── RenderHUD ─────────────────────────────────────────────────────────────
void DemoPlayerManager::RenderHUD() {
    if (!IsLoaded()) return;

    ImGuiIO& io = ImGui::GetIO();
    const float HUD_H      = 90.f;
    const float HUD_PAD    = 12.f;
    const float BAR_W      = io.DisplaySize.x;
    const float BAR_Y      = io.DisplaySize.y - HUD_H;

    // ── Background pill ──
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddRectFilled(
        ImVec2(0.f, BAR_Y - 4.f),
        ImVec2(BAR_W, io.DisplaySize.y),
        IM_COL32(10, 10, 14, 220));
    dl->AddLine(
        ImVec2(0.f, BAR_Y - 4.f),
        ImVec2(BAR_W, BAR_Y - 4.f),
        IM_COL32(60, 80, 120, 200), 1.5f);

    // ── Invisible ImGui window over the HUD area ──
    ImGui::SetNextWindowPos(ImVec2(0.f, BAR_Y - 4.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(BAR_W, HUD_H + 8.f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.f);
    ImGuiWindowFlags hflags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##DemoPlayerHUD", nullptr, hflags);

    const float total   = (float)total_duration_;
    const float current = (float)current_time_;

    // ── Row 1: timeline slider ──
    {
        float sliderW = BAR_W - HUD_PAD * 2.f - 220.f;
        ImGui::SetCursorPos(ImVec2(HUD_PAD, 10.f));

        ImGui::PushStyleColor(ImGuiCol_SliderGrab,      ImVec4(0.40f, 0.70f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive,ImVec4(0.55f, 0.85f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg,         ImVec4(0.10f, 0.12f, 0.18f, 0.9f));

        float t = (total > 0.f) ? (current / total) : 0.f;
        ImGui::PushItemWidth(sliderW);

        if (hud_seeking_) {
            // commit seek when mouse released
            if (!io.MouseDown[0]) {
                SeekToTime((double)(hud_seek_val_ * total));
                hud_seeking_ = false;
            }
            ImGui::SliderFloat("##timeline", &hud_seek_val_, 0.f, 1.f, "");
        } else {
            hud_seek_val_ = t;
            if (ImGui::SliderFloat("##timeline", &hud_seek_val_, 0.f, 1.f, "")) {
                hud_seeking_ = true;
            }
        }
        ImGui::PopItemWidth();
        ImGui::PopStyleColor(3);

        // Time labels
        int cm = (int)(current / 60); int cs_ = (int)current % 60;
        int tm = (int)(total  / 60);  int ts_ = (int)total   % 60;
        char timeBuf[48];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d / %02d:%02d", cm, cs_, tm, ts_);
        ImGui::SetCursorPos(ImVec2(HUD_PAD + sliderW + 8.f, 12.f));
        ImGui::TextColored(ImVec4(0.75f, 0.85f, 1.f, 1.f), "%s", timeBuf);

        // Round indicator
        if (!round_index_.empty()) {
            char rndBuf[32];
            snprintf(rndBuf, sizeof(rndBuf), "R%d/%d",
                     current_round_ + 1, (int)round_index_.size());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 1.f, 0.7f, 1.f), "  %s", rndBuf);
        }
    }

    // ── Row 2: Transport buttons ──
    {
        float btnY = 36.f;
        ImGui::SetCursorPos(ImVec2(HUD_PAD, btnY));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f,0.20f,0.30f,0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f,0.38f,0.60f,1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f,0.15f,0.25f,1.00f));

        // |<  Prev Round
        if (ImGui::Button("|<##prevRound", ImVec2(34.f,28.f)))
            PrevRound();
        ImGui::SameLine();

        // <| Step back — not easily supported w/o re-seek; grey it out
        ImGui::BeginDisabled();
        ImGui::Button("<|##stepBack", ImVec2(34.f,28.f));
        ImGui::EndDisabled();
        ImGui::SameLine();

        // Play / Pause
        const char* ppLabel = (is_playing_ && !is_paused_) ? "||##pause" : "> ##play";
        if (ImGui::Button(ppLabel, ImVec2(44.f,28.f)))
            TogglePlayPause();
        ImGui::SameLine();

        // Step forward
        if (ImGui::Button("|>##stepFwd", ImVec2(34.f,28.f)))
            StepForward();
        ImGui::SameLine();

        // >|  Next Round
        if (ImGui::Button(">|##nextRound", ImVec2(34.f,28.f)))
            NextRound();
        ImGui::SameLine();

        ImGui::Dummy(ImVec2(8.f, 0.f)); ImGui::SameLine();

        // Speed buttons
        for (int i = 0; i < DEMO_SPEED_COUNT; ++i) {
            char spLabel[16];
            snprintf(spLabel, sizeof(spLabel), "%s##sp%d",
                     i == 0 ? "0.25x" : i == 1 ? "0.5x" :
                     i == 2 ? "1x"    : i == 3 ? "2x" : "4x", i);
            bool active = (i == speed_index_);
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f,0.55f,1.0f,0.85f));
            if (ImGui::Button(spLabel, ImVec2(40.f,28.f))) SetSpeedIndex(i);
            if (active) ImGui::PopStyleColor();
            ImGui::SameLine();
        }

        ImGui::Dummy(ImVec2(12.f,0.f)); ImGui::SameLine();

        // Camera mode toggle
        {
            bool isFreecam = (cam_mode_ == DemoCamMode::Freecam);
            if (isFreecam) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f,0.4f,0.1f,0.85f));
            if (ImGui::Button(isFreecam ? "FREECAM##cam" : "SPECTATOR##cam", ImVec2(90.f,28.f)))
                ToggleCamMode();
            if (isFreecam) ImGui::PopStyleColor();
        }
        ImGui::SameLine();

        ImGui::Dummy(ImVec2(8.f,0.f)); ImGui::SameLine();

        // EXIT DEMO button — only visible while viewer loop is running
        if (is_viewer_active_) {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f,0.10f,0.10f,0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f,0.15f,0.15f,1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.00f,0.20f,0.20f,1.00f));
            if (ImGui::Button("✕ EXIT DEMO##exitDemo", ImVec2(110.f,28.f)))
                hud_exit_requested_ = true;
            ImGui::PopStyleColor(3);
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
    }

    // ── Row 3: Player list (only when in Spectator mode) ──
    if (cam_mode_ == DemoCamMode::Spectator && !cycle_states_.empty()) {
        float listY = 68.f;
        ImGui::SetCursorPos(ImVec2(HUD_PAD, listY));
        ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 0.85f), "Watch:");
        ImGui::SameLine();

        float slotW = std::min(90.f, (BAR_W - 200.f) / (float)cycle_states_.size());
        for (int i = 0; i < (int)cycle_states_.size(); ++i) {
            const auto& st = cycle_states_[i];
            bool sel = (i == spectator_target_);
            if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f,0.55f,1.f,0.9f));
            char btnId[80];
            snprintf(btnId, sizeof(btnId), "%s##watch%d", st.name, i);
            if (ImGui::Button(btnId, ImVec2(slotW, 18.f)))
                SetSpectatorTarget(i);
            if (sel) ImGui::PopStyleColor();
            if (i + 1 < (int)cycle_states_.size()) ImGui::SameLine();
        }

        // Alive / dead indicator for current target
        if (spectator_target_ < (int)cycle_states_.size()) {
            const auto& tgt = cycle_states_[spectator_target_];
            ImGui::SameLine(0.f, 14.f);
            ImGui::TextColored(
                tgt.alive ? ImVec4(0.3f,1.f,0.4f,1.f) : ImVec4(1.f,0.3f,0.3f,1.f),
                tgt.alive ? "● ALIVE" : "✕ DEAD");

            // Current target pos / speed debug info
            ImGui::SameLine(0.f, 20.f);
            ImGui::TextColored(ImVec4(0.55f,0.55f,0.65f,0.85f),
                "pos(%.1f, %.1f)  spd %.1f  rub %.2f",
                tgt.posX, tgt.posY, tgt.speed, tgt.rubber);
        }
    }

    // Freecam controls hint
    if (cam_mode_ == DemoCamMode::Freecam) {
        float listY = 68.f;
        ImGui::SetCursorPos(ImVec2(HUD_PAD, listY));
        ImGui::TextColored(ImVec4(1.f,0.7f,0.3f,0.85f),
            "FREECAM: WASD move | Mouse look | Q/E up-down | F=toggle cam | Esc/Tab=exit demo");
    }

    ImGui::End();

    // ── Round markers on timeline ──
    {
        float yMarker = BAR_Y - 4.f;
        float sliderX0 = HUD_PAD;
        float sliderX1 = BAR_W - HUD_PAD - 220.f;
        for (auto& re : round_index_) {
            float frac = (total > 0.f) ? (float)(re.gameTime / total_duration_) : 0.f;
            float mx = sliderX0 + frac * (sliderX1 - sliderX0);
            dl->AddLine(ImVec2(mx, yMarker), ImVec2(mx, yMarker - 10.f),
                        IM_COL32(80, 255, 140, 180), 1.5f);
        }
    }
}

void DemoPlayerManager::SyncGameObjects() {
    extern tCONTROLLED_PTR(gGame) sg_currentGame;
    if (!sg_currentGame) return;
    eGrid* grid = sg_currentGame->Grid();
    if (!grid) return;

    bool synced_slots[16];
    std::memset(synced_slots, 0, sizeof(synced_slots));

    const auto& gameObjects = grid->GameObjects();

    gCycle* active_cycles[16];
    std::memset(active_cycles, 0, sizeof(active_cycles));
    for (int i = 0; i < gameObjects.Len(); ++i) {
        gCycle* c = dynamic_cast<gCycle*>(gameObjects(i));
        if (c && c->Player() && c->Player()->pID >= 0 && c->Player()->pID < 16) {
            active_cycles[c->Player()->pID] = c;
        }
    }

    for (const auto& cs : cycle_states_) {
        if (cs.playerSlot < 0 || cs.playerSlot >= 16) continue;

        ePlayerNetID* pni = nullptr;
        for (int i = 0; i < se_PlayerNetIDs.Len(); ++i) {
            if (se_PlayerNetIDs(i)->pID == cs.playerSlot) {
                pni = se_PlayerNetIDs(i);
                break;
            }
        }
        if (!pni) {
            pni = new ePlayerNetID(cs.playerSlot);
            pni->SetName(cs.name);
        }

        gCycle* cycle = active_cycles[cs.playerSlot];

        if (cycle && !cycle->Alive() && cs.alive) {
            delete cycle;
            cycle = nullptr;
            active_cycles[cs.playerSlot] = nullptr;
        }

        if (!cycle) {
            if (cs.alive) {
                eCoord pos(cs.posX, cs.posY);
                eCoord dir(cs.dirX, cs.dirY);
                cycle = new gCycle(grid, pos, dir, pni);
                pni->ControlObject(cycle);
                cycle->lastTime = current_time_;
                cycle->SetDistance(0.0f);
                cycle->currentWall = new gNetPlayerWall(cycle, pos, dir, current_time_, 0.0f);
                active_cycles[cs.playerSlot] = cycle;
            }
        }

        if (cycle) {
            synced_slots[cs.playerSlot] = true;
            if (cs.alive) {
                eCoord cp(cs.posX, cs.posY);
                eCoord old_cp = cycle->Position();

                // Accumulate distance
                double step_dist = (cp - old_cp).Norm();
                cycle->SetDistance(cycle->GetDistance() + step_dist);

                eCoord recorded_dir(cs.dirX, cs.dirY);

                // Check if direction changed
                bool turnOccurred = (recorded_dir.x != cycle->dirDrive.x || recorded_dir.y != cycle->dirDrive.y);

                // Keep cycle time in sync
                cycle->lastTime = current_time_;

                if (!cycle->currentWall) {
                    cycle->currentWall = new gNetPlayerWall(cycle, cp, recorded_dir, current_time_, cycle->GetDistance());
                } else if (turnOccurred) {
                    // Turn occurred: close current segment and start a new one
                    cycle->currentWall->Update(current_time_, cp);
                    cycle->currentWall->CopyIntoGrid(grid);
                    cycle->currentWall = nullptr;

                    // Apply the turn direction and winding
                    cycle->windingNumberWrapped_ = cycle->windingNumber_ = grid->DirectionWinding(recorded_dir);
                    cycle->dirDrive = recorded_dir;
                    cycle->dir = recorded_dir;

                    cycle->currentWall = new gNetPlayerWall(cycle, cp, recorded_dir, current_time_, cycle->GetDistance());
                } else {
                    // Update current wall end point
                    cycle->currentWall->Update(current_time_, cp);
                }

                // Sync engine properties
                cycle->SetPosition(cp);
                cycle->predictPosition_ = cp;
                cycle->lastGoodPosition_ = cp;
                cycle->SetSpeedState(cs.speed);
                cycle->SetRubber(cs.rubber);
                cycle->SetAliveState(1);
            } else {
                if (cycle->Alive()) {
                    // Close the current wall segment before dying!
                    if (cycle->currentWall) {
                        cycle->currentWall->Update(current_time_, cycle->Position());
                        cycle->currentWall->CopyIntoGrid(grid);
                        cycle->currentWall = nullptr;
                    }
                    cycle->Die(current_time_);
                }
                cycle->SetAliveState(0);
                eCoord cp(-10000.f, -10000.f);
                cycle->SetPosition(cp);
                cycle->predictPosition_ = cp;
                cycle->lastGoodPosition_ = cp;
            }
        }
    }

    // Clean up cycles that aren't in the demo player's active player list at all
    for (int i = 0; i < gameObjects.Len(); ++i) {
        gCycle* c = dynamic_cast<gCycle*>(gameObjects(i));
        if (c && c->Player()) {
            int slot = c->Player()->pID;
            if (slot < 0 || slot >= 16 || !synced_slots[slot]) {
                if (c->Alive()) {
                    if (c->currentWall) {
                        c->currentWall->Update(current_time_, c->Position());
                        c->currentWall->CopyIntoGrid(grid);
                        c->currentWall = nullptr;
                    }
                    c->Die(current_time_);
                }
                c->SetAliveState(0);
                eCoord cp(-10000.f, -10000.f);
                c->SetPosition(cp);
                c->predictPosition_ = cp;
                c->lastGoodPosition_ = cp;
            }
        }
    }
}

void DemoPlayerManager::ClearGridObjects() {
    extern tCONTROLLED_PTR(gGame) sg_currentGame;
    if (!sg_currentGame) return;
    eGrid* grid = sg_currentGame->Grid();
    if (!grid) return;

    // Clear all net walls first to ensure no dangling references!
    gNetPlayerWall::Clear();

    const auto& gameObjects = grid->GameObjects();

    for (int i = gameObjects.Len() - 1; i >= 0; --i) {
        if (i >= gameObjects.Len()) continue;
        eGameObject* go = gameObjects(i);
        if (dynamic_cast<gCycle*>(go) || dynamic_cast<gPlayerWall*>(go)) {
            delete go;
        }
    }
}

void DemoPlayerManager::RunDemoViewerLoop() {
    if (!IsLoaded()) return;

    is_viewer_active_ = true;
    ImGuiIO& io = ImGui::GetIO();

    SDL_ShowCursor();
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    io.MouseDrawCursor = true;

    // Purge stale events + ImGui mouse state
    SDL_Event pEvent;
    while (SDL_PollEvent(&pEvent)) {}
    for (int i = 0; i < 5; i++) { io.MouseDown[i] = false; io.MouseClicked[i] = false; }

    Play();

    // Trail history: cycle slot → list of positions
    struct TrailPt { float x, y; };
    static const int MAX_TRAIL = 512;
    std::vector<std::vector<TrailPt>> trails(16);

    // Compute world bounds from first pass
    float wMinX = 1e9f, wMinY = 1e9f, wMaxX = -1e9f, wMaxY = -1e9f;
    {
        size_t saved = read_cursor_;
        read_cursor_ = data_start_;
        bool gotBounds = false;
        while (!AtEnd()) {
            uint8_t t = Read<uint8_t>();
            if (t == (uint8_t)AarecPacketType::FileEnd) {
                goto bounds_done;
            }
            double gt = Read<double>(); (void)gt;
            if ((AarecPacketType)t == AarecPacketType::CycleState) {
                uint8_t pSlot  = Read<uint8_t>();
                float   posX   = Read<float>();
                float   posY   = Read<float>();
                float   dirX   = Read<float>();
                float   dirY   = Read<float>();
                float   speed  = Read<float>();
                float   rubber = Read<float>();
                uint8_t alive  = Read<uint8_t>();

                wMinX = std::min(wMinX, posX); wMaxX = std::max(wMaxX, posX);
                wMinY = std::min(wMinY, posY); wMaxY = std::max(wMaxY, posY);
                gotBounds = true;
            } else {
                switch ((AarecPacketType)t) {
                case AarecPacketType::FrameBegin: Skip(4+4); break;
                case AarecPacketType::ChatEvent:  {
                    Skip(1); // senderSlot
                    uint16_t len = Read<uint16_t>();
                    Skip(len);
                    break;
                }
                case AarecPacketType::KillEvent:  Skip(2); break;
                case AarecPacketType::TurnEvent:  Skip(2); break;
                case AarecPacketType::FileEnd: goto bounds_done;
                default: break;
                }
            }
        }
        bounds_done:
        if (!gotBounds) { wMinX=-200; wMinY=-200; wMaxX=200; wMaxY=200; }
        // Expand bounds slightly
        float px = (wMaxX-wMinX)*0.08f, py = (wMaxY-wMinY)*0.08f;
        wMinX -= px+5; wMinY -= py+5; wMaxX += px+5; wMaxY += py+5;
        read_cursor_ = saved;
    }

    // Player colors (cycling)
    static const ImU32 PLAYER_COLORS[] = {
        IM_COL32(80,200,255,255), IM_COL32(255,80,100,255),
        IM_COL32(80,255,140,255), IM_COL32(255,200,50,255),
        IM_COL32(200,80,255,255), IM_COL32(255,140,40,255),
        IM_COL32(40,220,220,255), IM_COL32(255,80,200,255),
    };

    struct timeval last_tv = {0,0};
    bool running = true;
    bool exit_requested = false; // set by EXIT button in RenderHUD

    // Expose exit flag to RenderHUD via a static pointer hack
    hud_exit_requested_ = false;

    while (running) {
        SDL_ShowCursor();
        struct timeval tv; gettimeofday(&tv, nullptr);
        if (last_tv.tv_sec != 0) {
            double dt = (double)(tv.tv_sec-last_tv.tv_sec)+(double)(tv.tv_usec-last_tv.tv_usec)/1e6;
            if (dt > 0.0 && dt < 1.0) io.DeltaTime = (float)dt;
        }
        last_tv = tv;

        Tick(io.DeltaTime);

        // Update trails from current cycle_states_
        for (int pi = 0; pi < (int)cycle_states_.size(); ++pi) {
            const auto& cs = cycle_states_[pi];
            int slot = cs.playerSlot;
            if (slot < 0 || slot >= 16) continue;
            auto& tr = trails[slot];
            if (tr.empty() || fabsf(tr.back().x - cs.posX) > 0.5f || fabsf(tr.back().y - cs.posY) > 0.5f) {
                if ((int)tr.size() >= MAX_TRAIL) tr.erase(tr.begin());
                tr.push_back({cs.posX, cs.posY});
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { running = false; break; }
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_KEY_DOWN) {
                int k = event.key.key;
                if (k == SDLK_ESCAPE || k == SDLK_TAB) { running = false; break; }
                if (k == SDLK_SPACE)    { TogglePlayPause(); }
                if (k == SDLK_LEFT)     { SeekToTime(current_time_ - 5.0); }
                if (k == SDLK_RIGHT)    { SeekToTime(current_time_ + 5.0); }
                if (k == SDLK_UP)       { SpeedUp(); }
                if (k == SDLK_DOWN)     { SlowDown(); }
                if (k == SDLK_PAGEUP)   { PrevRound(); }
                if (k == SDLK_PAGEDOWN) { NextRound(); }
                if (k == SDLK_N)        { NextSpectatorTarget(); }
                if (k == SDLK_P)        { PrevSpectatorTarget(); }
            }
        }

        if (hud_exit_requested_) { running = false; }
        if (!running) break;

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImDrawList* bgDl = ImGui::GetBackgroundDrawList();

        // ── Dark background ──
        bgDl->AddRectFilled(ImVec2(0,0), io.DisplaySize, IM_COL32(5,6,10,255));

        // ── MINIMAP ──
        const float MAP_PAD   = 12.f;
        const float HUD_H     = 98.f;  // transport bar height
        const float MAP_SZ_W  = io.DisplaySize.x * 0.60f;
        const float MAP_SZ_H  = io.DisplaySize.y - HUD_H - MAP_PAD * 2.f - 42.f;
        const float mapLeft   = (io.DisplaySize.x - MAP_SZ_W) * 0.5f;
        const float mapTop    = 42.f;
        ImVec2 mapMin(mapLeft, mapTop);
        ImVec2 mapMax(mapLeft + MAP_SZ_W, mapTop + MAP_SZ_H);

        float scaleX = MAP_SZ_W / (wMaxX - wMinX);
        float scaleY = MAP_SZ_H / (wMaxY - wMinY);
        float scale  = std::min(scaleX, scaleY) * 0.92f;
        float offX   = mapLeft + MAP_SZ_W * 0.5f - (wMinX + wMaxX) * 0.5f * scale;
        float offY   = mapTop  + MAP_SZ_H * 0.5f - (wMinY + wMaxY) * 0.5f * scale;

        auto worldToScreen = [&](float wx, float wy) -> ImVec2 {
            return ImVec2(offX + wx * scale, offY + wy * scale);
        };

        // Arena border
        bgDl->AddRectFilled(mapMin, mapMax, IM_COL32(10,12,18,230), 8.f);
        bgDl->AddRect(mapMin, mapMax, IM_COL32(40,55,85,200), 8.f, 0, 1.5f);

        // Grid lines on minimap
        {
            ImU32 gc = IM_COL32(20,28,42,180);
            for (float wx = wMinX; wx < wMaxX; wx += (wMaxX-wMinX)/8.f) {
                ImVec2 a = worldToScreen(wx, wMinY), b = worldToScreen(wx, wMaxY);
                if (a.x >= mapMin.x && a.x <= mapMax.x)
                    bgDl->AddLine(ImVec2(a.x, mapMin.y), ImVec2(b.x, mapMax.y), gc);
            }
            for (float wy = wMinY; wy < wMaxY; wy += (wMaxY-wMinY)/8.f) {
                ImVec2 a = worldToScreen(wMinX, wy), b = worldToScreen(wMaxX, wy);
                if (a.y >= mapMin.y && a.y <= mapMax.y)
                    bgDl->AddLine(ImVec2(mapMin.x, a.y), ImVec2(mapMax.x, b.y), gc);
            }
        }

        // Draw trails
        for (int pi = 0; pi < (int)cycle_states_.size(); ++pi) {
            int slot = cycle_states_[pi].playerSlot;
            if (slot < 0 || slot >= 16) continue;
            const auto& tr = trails[slot];
            ImU32 trailCol = (PLAYER_COLORS[pi % 8] & 0x00FFFFFF) | (uint32_t)(80 << 24);
            for (int ti = 1; ti < (int)tr.size(); ++ti) {
                ImVec2 a = worldToScreen(tr[ti-1].x, tr[ti-1].y);
                ImVec2 b = worldToScreen(tr[ti].x,   tr[ti].y);
                // Clamp to map bounds
                a.x = std::max(mapMin.x, std::min(mapMax.x, a.x));
                a.y = std::max(mapMin.y, std::min(mapMax.y, a.y));
                b.x = std::max(mapMin.x, std::min(mapMax.x, b.x));
                b.y = std::max(mapMin.y, std::min(mapMax.y, b.y));
                bgDl->AddLine(a, b, trailCol, 1.5f);
            }
        }

        // Draw cycles
        for (int pi = 0; pi < (int)cycle_states_.size(); ++pi) {
            const auto& cs = cycle_states_[pi];
            ImVec2 pos = worldToScreen(cs.posX, cs.posY);
            pos.x = std::max(mapMin.x, std::min(mapMax.x, pos.x));
            pos.y = std::max(mapMin.y, std::min(mapMax.y, pos.y));

            ImU32 col = PLAYER_COLORS[pi % 8];
            bool sel = (pi == spectator_target_);
            float r = cs.alive ? (sel ? 7.f : 5.f) : 3.f;

            if (cs.alive) {
                bgDl->AddCircleFilled(pos, r + 2.f, col & 0x44FFFFFF);
                bgDl->AddCircleFilled(pos, r, col);
                // Direction arrow
                float len = r + 6.f;
                ImVec2 tip(pos.x + cs.dirX * len, pos.y + cs.dirY * len);
                bgDl->AddLine(pos, tip, col, 1.5f);
            } else {
                bgDl->AddCircle(pos, r, col & 0x66FFFFFF, 8, 1.5f);
            }

            // Name label
            if (cs.alive || sel) {
                ImVec2 labelPos(pos.x + 8.f, pos.y - 8.f);
                bgDl->AddText(labelPos, col, cs.name);
            }
        }

        // Title
        {
            const char* title = "RETROCYCLES DEMO VIEWER";
            ImVec2 tsz = ImGui::CalcTextSize(title);
            bgDl->AddText(ImVec2((io.DisplaySize.x - tsz.x) * 0.5f, 14.f),
                          IM_COL32(60,90,150,200), title);
        }

        // Right panel: player list
        {
            float panX = mapMax.x + 14.f;
            float panW = io.DisplaySize.x - panX - 8.f;
            float panY = mapTop;
            if (panW > 60.f) {
                ImGui::SetNextWindowPos(ImVec2(panX, panY), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(panW, MAP_SZ_H), ImGuiCond_Always);
                ImGui::SetNextWindowBgAlpha(0.78f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
                ImGui::Begin("##DPPanel", nullptr,
                    ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
                    ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar);
                ImGui::TextColored(ImVec4(0.4f,0.75f,1.f,1.f), "PLAYERS");
                ImGui::Separator();
                for (int pi = 0; pi < (int)cycle_states_.size(); ++pi) {
                    const auto& cs = cycle_states_[pi];
                    bool sel = (pi == spectator_target_);
                    ImU32 pcol = PLAYER_COLORS[pi % 8];
                    ImVec4 cv((pcol&0xFF)/255.f,((pcol>>8)&0xFF)/255.f,((pcol>>16)&0xFF)/255.f,1.f);
                    if (sel) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f,0.35f,0.70f,0.9f));
                    ImGui::PushStyleColor(ImGuiCol_Text, cv);
                    char lbl[96]; snprintf(lbl,sizeof(lbl),"%s %s##pl%d", cs.alive?"●":"✕", cs.name, pi);
                    if (ImGui::Selectable(lbl, sel)) SetSpectatorTarget(pi);
                    ImGui::PopStyleColor();
                    if (sel) ImGui::PopStyleColor();
                    if (sel && cs.alive) {
                        ImGui::TextColored(ImVec4(0.5f,0.6f,0.75f,1.f),
                            "spd %.1f rub %.2f", cs.speed, cs.rubber);
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar();
            }
        }

        // Key hints line above HUD
        {
            const char* h = "[Space] Pause  [<>] ±5s  [v^] Speed  [PgUp/Dn] Round  [N/P] Player  [Esc] Exit";
            ImVec2 hsz = ImGui::CalcTextSize(h);
            bgDl->AddText(ImVec2((io.DisplaySize.x-hsz.x)*0.5f, io.DisplaySize.y-HUD_H-16.f),
                          IM_COL32(80,100,140,180), h);
        }

        // Transport HUD (draws RenderHUD which now has EXIT button)
        RenderHUD();

        ImGui::Render();

        GLboolean blendWas = glIsEnabled(GL_BLEND);
        GLboolean depthWas = glIsEnabled(GL_DEPTH_TEST);
        GLboolean lightWas = glIsEnabled(GL_LIGHTING);
        GLboolean cullWas  = glIsEnabled(GL_CULL_FACE);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); glDisable(GL_CULL_FACE);
        glClearColor(0.02f, 0.024f, 0.04f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        if (!blendWas) glDisable(GL_BLEND);
        if (depthWas)  glEnable(GL_DEPTH_TEST);
        if (lightWas)  glEnable(GL_LIGHTING);
        if (cullWas)   glEnable(GL_CULL_FACE);
        rSysDep::SwapGL();
        rSysDep::ClearGL();
    }

    Pause();
    is_viewer_active_ = false;
    hud_exit_requested_ = false;
    io.MouseDrawCursor = false;

    su_ClearKeys();
    SDL_HideCursor();
    while (SDL_PollEvent(&pEvent)) {}
}

void DemoPlayerManager::StartViewer3D() {
    is_viewer_active_ = true;
    hud_exit_requested_ = false;
    Play();
}

void DemoPlayerManager::StopViewer3D() {
    is_viewer_active_ = false;
    Pause();
    ClearGridObjects();
    for (auto& tr : trails_) {
        tr.clear();
    }
}

#endif // !DEDICATED
