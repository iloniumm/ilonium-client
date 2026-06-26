// =========================================================================
// DemoRecorder.cpp — AAREC Demo Recording System (Implementation)
// RetroCycles / Armagetron Advanced client mod
// =========================================================================

#ifndef DEDICATED

#include "DemoRecorder.h"

#include <cassert>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <chrono>

#include "tDirectories.h"
#include "tools/tSysTime.h"  // tSysTimeFloat()
#include "eTimer.h"

// ── DemoRingBuffer ────────────────────────────────────────────────────────

DemoRingBuffer::DemoRingBuffer() {
    // buf_ is a plain array on the stack-of-the-singleton, so it is
    // effectively pre-allocated (no heap alloc during gameplay).
    head_.store(0, std::memory_order_relaxed);
    tail_.store(0, std::memory_order_relaxed);
}

bool DemoRingBuffer::TryPush(const DemoFrame& frame) noexcept {
    const size_t head    = head_.load(std::memory_order_relaxed);
    const size_t nextHead = (head + 1) & (DEMO_RING_CAPACITY - 1);
    const size_t tail    = tail_.load(std::memory_order_acquire);

    if (nextHead == tail) {
        // Buffer full — drop frame, record stat
        droppedFrames_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    buf_[head] = frame;
    head_.store(nextHead, std::memory_order_release);
    return true;
}

bool DemoRingBuffer::TryPop(DemoFrame& out) noexcept {
    const size_t tail = tail_.load(std::memory_order_relaxed);
    const size_t head = head_.load(std::memory_order_acquire);

    if (tail == head) return false;  // Empty

    out = buf_[tail];
    const size_t nextTail = (tail + 1) & (DEMO_RING_CAPACITY - 1);
    tail_.store(nextTail, std::memory_order_release);
    return true;
}

size_t DemoRingBuffer::Size() const noexcept {
    const size_t h = head_.load(std::memory_order_relaxed);
    const size_t t = tail_.load(std::memory_order_relaxed);
    return (h - t + DEMO_RING_CAPACITY) & (DEMO_RING_CAPACITY - 1);
}

// ── DemoRecorder Singleton ─────────────────────────────────────────────────

DemoRecorder& DemoRecorder::Instance() {
    // Meyer's singleton — thread-safe in C++11
    static DemoRecorder instance;
    return instance;
}

DemoRecorder::DemoRecorder() = default;

DemoRecorder::~DemoRecorder() {
    if (IsRecording()) {
        StopRecording();
    }
}

// ── Lifecycle ─────────────────────────────────────────────────────────────

void DemoRecorder::StartRecording(const std::string& filename,
                                   const std::string& matchId)
{
    if (recording_.load(std::memory_order_acquire)) {
        // Already recording — stop first
        StopRecording();
    }

    // Determine output path
    std::string outPath = filename;
    if (outPath.empty()) {
        // Auto-generate filename from current time
        std::time_t now = std::time(nullptr);
        std::tm* lt = std::localtime(&now);
        char timebuf[64];
        std::strftime(timebuf, sizeof(timebuf), "demo_%Y%m%d_%H%M%S.aarec", lt);

        // Prefer the user's var directory for saves
        const char* varDir = nullptr;
        // tDirectories::Var().GetWritePath() returns an absolute path
        outPath = (const char*)tDirectories::Var().GetWritePath(timebuf);
    }

    // Open file (binary write)
    file_ = std::fopen(outPath.c_str(), "wb");
    if (!file_) {
        // Could not open file — bail silently (UI can query IsRecording())
        return;
    }

    // Reset state
    currentFilename_ = outPath;
    stats_.totalFrames.store(0,   std::memory_order_relaxed);
    stats_.droppedFrames.store(0, std::memory_order_relaxed);
    stats_.bytesWritten.store(0,  std::memory_order_relaxed);
    stats_.duration.store(0.0,    std::memory_order_relaxed);
    stats_.writing.store(true,    std::memory_order_relaxed);
    frameCounter_ = 0;
    startTime_    = se_GameTime();
    recordingTime_ = 0.0;

    stopRequested_.store(false, std::memory_order_release);
    recording_.store(true,      std::memory_order_release);

    // Write file header synchronously (so readers can trust it is complete)
    WriteFileHeader(matchId);

    // Spin up background writer thread
    writerThread_ = std::thread(&DemoRecorder::WriterThreadMain, this);
}

void DemoRecorder::StopRecording() {
    if (!recording_.load(std::memory_order_acquire)) return;

    // Signal the writer thread to drain and exit
    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        stopRequested_.store(true, std::memory_order_release);
    }
    cv_.notify_one();

    // Wait for writer to finish (it will flush + write EOF sentinel)
    if (writerThread_.joinable()) {
        writerThread_.join();
    }

    recording_.store(false, std::memory_order_release);
    stats_.writing.store(false, std::memory_order_relaxed);

    // Track recent files (newest first, cap at 20)
    recentFiles_.insert(recentFiles_.begin(), currentFilename_);
    if (recentFiles_.size() > 20) recentFiles_.resize(20);
}

bool DemoRecorder::IsRecording() const noexcept {
    return recording_.load(std::memory_order_acquire);
}

// ── Hot-path: Frame data injection ─────────────────────────────────────────

void DemoRecorder::LogFrameData(double gameTime, float dt, int tick,
                                 const std::vector<DemoCycleState>& cycles)
{
    recordingTime_ += dt;
    double useTime = recordingTime_;

    // Push tick header
    ring_.TryPush(DemoFrame::MakeTick(useTime, dt, tick));

    // Push per-cycle state entries (lock-free, no allocation)
    for (const auto& cs : cycles) {
        ring_.TryPush(DemoFrame::MakeCycle(useTime, cs));
    }

    // Update stats
    stats_.totalFrames.fetch_add(1 + (uint64_t)cycles.size(),
                                  std::memory_order_relaxed);
    stats_.duration.store(useTime, std::memory_order_relaxed);

    // Wake writer thread if it is sleeping
    cv_.notify_one();
}

void DemoRecorder::LogChat(double gameTime, int senderSlot, const char* message) {
    double useTime = recordingTime_;
    DemoChatEvent ce{};
    ce.senderSlot = senderSlot;
    std::strncpy(ce.message, message, sizeof(ce.message) - 1);
    ce.message[sizeof(ce.message) - 1] = '\0';
    ring_.TryPush(DemoFrame::MakeChat(useTime, ce));
    stats_.totalFrames.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void DemoRecorder::LogKill(double gameTime, int killedSlot, int killerSlot) {
    double useTime = recordingTime_;
    DemoKillEvent ke{ killedSlot, killerSlot };
    ring_.TryPush(DemoFrame::MakeKill(useTime, ke));
    stats_.totalFrames.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void DemoRecorder::LogTurn(double gameTime, int playerSlot, int direction) {
    double useTime = recordingTime_;
    DemoTurnEvent te{ playerSlot, (int8_t)direction };
    ring_.TryPush(DemoFrame::MakeTurn(useTime, te));
    stats_.totalFrames.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void DemoRecorder::LogRoundBegin(double gameTime) {
    double useTime = recordingTime_;
    ring_.TryPush(DemoFrame::MakeRoundBegin(useTime));
    stats_.totalFrames.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void DemoRecorder::LogRoundEnd(double gameTime) {
    double useTime = recordingTime_;
    ring_.TryPush(DemoFrame::MakeRoundEnd(useTime));
    stats_.totalFrames.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

// ── Writer thread ─────────────────────────────────────────────────────────

void DemoRecorder::WriterThreadMain() {
    DemoFrame frame;

    while (true) {
        // Drain the ring buffer as fast as possible
        while (ring_.TryPop(frame)) {
            SerializeFrame(frame);
        }

        // Check if we should stop
        const bool shouldStop = stopRequested_.load(std::memory_order_acquire);

        // If stop was requested AND ring is empty, we are done
        if (shouldStop && ring_.Empty()) {
            break;
        }

        if (!shouldStop) {
            // No more data right now — sleep until notified
            std::unique_lock<std::mutex> lk(cvMutex_);
            cv_.wait_for(lk, std::chrono::milliseconds(5), [this]() {
                return !ring_.Empty() ||
                        stopRequested_.load(std::memory_order_acquire);
            });
        }
    }

    // Final drain after stop signal
    while (ring_.TryPop(frame)) {
        SerializeFrame(frame);
    }

    // Update dropped-frame stat from ring buffer
    stats_.droppedFrames.store(ring_.DroppedFrames(), std::memory_order_relaxed);

    // Write EOF sentinel and close
    WriteFileEnd();

    if (file_) {
        std::fflush(file_);
        std::fclose(file_);
        file_ = nullptr;
    }
}

// ── Binary serialization ───────────────────────────────────────────────────
// Format is little-endian throughout.

// Low-level write helpers
void DemoRecorder::Write(const void* data, size_t bytes) {
    if (!file_) return;
    std::fwrite(data, 1, bytes, file_);
    stats_.bytesWritten.fetch_add(bytes, std::memory_order_relaxed);
}

void DemoRecorder::WriteU8(uint8_t v)   { Write(&v, 1); }

void DemoRecorder::WriteU16(uint16_t v) {
    uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF) };
    Write(b, 2);
}

void DemoRecorder::WriteU32(uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)( v        & 0xFF),
        (uint8_t)((v >>  8) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 24) & 0xFF)
    };
    Write(b, 4);
}

void DemoRecorder::WriteU64(uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; ++i) b[i] = (uint8_t)((v >> (i * 8)) & 0xFF);
    Write(b, 8);
}

void DemoRecorder::WriteF32(float v) {
    uint32_t tmp;
    std::memcpy(&tmp, &v, 4);
    WriteU32(tmp);
}

void DemoRecorder::WriteF64(double v) {
    uint64_t tmp;
    std::memcpy(&tmp, &v, 8);
    WriteU64(tmp);
}

void DemoRecorder::WriteStr(const char* s) {
    uint16_t len = s ? (uint16_t)std::strlen(s) : 0;
    WriteU16(len);
    if (len) Write(s, len);
}

// ── Packet: File Header ───────────────────────────────────────────────────
// Layout:
//   [0]    magic[6]          AAREC\0
//   [6]    version (u16)     0x0003
//   [8]    timestamp (u64)   unix epoch (seconds)
//   [16]   matchId (str16)   variable-length string (u16 len + bytes)
//   [?]    reserved (u16)    0x0000
void DemoRecorder::WriteFileHeader(const std::string& matchId) {
    Write(AAREC_MAGIC, 6);
    WriteU16(AAREC_VERSION);
    WriteU8((uint8_t)AarecPacketType::FileHeader);
    WriteF64(0.0);
    uint8_t len = (uint8_t)std::min(matchId.size(), (size_t)255);
    WriteU8(len);
    if (len > 0) {
        Write(matchId.data(), len);
    }
}

// ── Packet: File End ───────────────────────────────────────────────────────
void DemoRecorder::WriteFileEnd() {
    WriteU8((uint8_t)AarecPacketType::FileEnd);
    WriteU64(stats_.totalFrames.load(std::memory_order_relaxed));
    WriteU64(stats_.droppedFrames.load(std::memory_order_relaxed));
    WriteF64(stats_.duration.load(std::memory_order_relaxed));
}

// ── Packet: Frame serialisation dispatcher ────────────────────────────────
void DemoRecorder::SerializeFrame(const DemoFrame& frame) {
    switch (frame.tag) {

    case DemoFrameTag::Tick: {
        // Packet: FrameBegin
        // [type u8][gameTime f64][dt f32][tick u32]
        WriteU8((uint8_t)AarecPacketType::FrameBegin);
        WriteF64(frame.gameTime);
        WriteF32(frame.tick.dt);
        WriteU32((uint32_t)frame.tick.tick);
        break;
    }

    case DemoFrameTag::Cycle: {
        // Packet: CycleState
        // [type u8][gameTime f64][slot u8][posX f32][posY f32]
        //          [dirX f32][dirY f32][speed f32][rubber f32][alive u8]
        const auto& cs = frame.cycle;
        WriteU8((uint8_t)AarecPacketType::CycleState);
        WriteF64(frame.gameTime);
        WriteU8((uint8_t)cs.playerSlot);
        WriteF32(cs.posX);
        WriteF32(cs.posY);
        WriteF32(cs.dirX);
        WriteF32(cs.dirY);
        WriteF32(cs.speed);
        WriteF32(cs.rubber);
        WriteU8(cs.alive);
        break;
    }

    case DemoFrameTag::Chat: {
        // Packet: ChatEvent
        // [type u8][gameTime f64][senderSlot u8][message str16]
        const auto& ce = frame.chat;
        WriteU8((uint8_t)AarecPacketType::ChatEvent);
        WriteF64(frame.gameTime);
        WriteU8((uint8_t)ce.senderSlot);
        WriteStr(ce.message);
        break;
    }

    case DemoFrameTag::Kill: {
        // Packet: KillEvent
        // [type u8][gameTime f64][killedSlot u8][killerSlot i8]
        const auto& ke = frame.kill;
        WriteU8((uint8_t)AarecPacketType::KillEvent);
        WriteF64(frame.gameTime);
        WriteU8((uint8_t)ke.killedSlot);
        WriteU8((uint8_t)(int8_t)ke.killerSlot);
        break;
    }

    case DemoFrameTag::Turn: {
        // Packet: TurnEvent
        // [type u8][gameTime f64][playerSlot u8][direction i8]
        const auto& te = frame.turn;
        WriteU8((uint8_t)AarecPacketType::TurnEvent);
        WriteF64(frame.gameTime);
        WriteU8((uint8_t)te.playerSlot);
        WriteU8((uint8_t)te.direction);
        break;
    }

    case DemoFrameTag::RoundBegin: {
        WriteU8((uint8_t)AarecPacketType::RoundBegin);
        WriteF64(frame.gameTime);
        break;
    }

    case DemoFrameTag::RoundEnd: {
        WriteU8((uint8_t)AarecPacketType::RoundEnd);
        WriteF64(frame.gameTime);
        break;
    }

    default:
        break;
    }
}

#endif // !DEDICATED
