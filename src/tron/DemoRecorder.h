// =========================================================================
// DemoRecorder.h — AAREC Demo Recording System
// RetroCycles / Armagetron Advanced client mod
//
// Records match state into the .aarec binary format.
// Disk I/O runs on a dedicated background thread so the game render loop
// is never stalled waiting for disk writes.
//
// Architecture overview
// ---------------------
//   GameThread                        WriterThread
//   ──────────                        ────────────
//   LogFrameData()                    ┐
//     → build DemoFrame               │  dequeues
//     → push_back into RingBuffer     │  from RingBuffer
//       (lock-free slot claim via     │  compresses + writes
//        atomic counter)             ─┘  to file
//
// The RingBuffer is pre-allocated once (reserve(DEMO_RING_CAPACITY)) so
// there are ZERO heap allocations during active gameplay after Start().
//
// =========================================================================

#pragma once

#ifndef DEDICATED

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>

#include "engine/eCoord.h"     // eCoord, REAL

// ── Forward declarations ──────────────────────────────────────────────────
class ePlayer;

// ── Constants ─────────────────────────────────────────────────────────────

/// Pre-allocated ring-buffer slot count.
/// Must be a power of 2 for the bitmask wrap trick.
static constexpr size_t DEMO_RING_CAPACITY = 4096;

/// Magic bytes at the start of every .aarec file (ASCII "AAREC\0")
static constexpr uint8_t AAREC_MAGIC[6] = { 'A','A','R','E','C','\0' };

/// Current format version
static constexpr uint16_t AAREC_VERSION = 0x0003;

// ── .aarec packet types ───────────────────────────────────────────────────
enum class AarecPacketType : uint8_t {
    FileHeader    = 0x00, ///< File preamble (written once)
    FrameBegin    = 0x01, ///< Marks the start of a new game-tick
    CycleState    = 0x02, ///< Per-cycle position / direction / rubber
    ChatEvent     = 0x03, ///< Chat message
    KillEvent     = 0x04, ///< Player death / kill
    TurnEvent     = 0x05, ///< Cycle direction change
    RoundBegin    = 0x10, ///< Round start
    RoundEnd      = 0x11, ///< Round over
    FileEnd       = 0xFF, ///< End-of-recording sentinel
};

// ── Per-bike state snapshot ───────────────────────────────────────────────
struct DemoCycleState {
    int32_t  playerSlot;   ///< ePlayer slot index (0-15)
    float    posX, posY;   ///< Position in world units
    float    dirX, dirY;   ///< Normalised direction vector
    float    speed;        ///< Current speed (units/s)
    float    rubber;       ///< Current rubber amount
    uint8_t  alive;        ///< 1 = alive, 0 = dead
};

// ── Generic game events ───────────────────────────────────────────────────
struct DemoChatEvent {
    int32_t senderSlot;
    char    message[192];
};

struct DemoKillEvent {
    int32_t killedSlot;
    int32_t killerSlot;  ///< -1 = self/environment
};

struct DemoTurnEvent {
    int32_t playerSlot;
    int8_t  direction; ///< +1 = right, -1 = left
};

// ── A single frame-packet as stored in the ring buffer ───────────────────
// We use a union-like structure so the vector element size is fixed and
// the ring buffer needs only one pre-allocated vector<DemoFrame>.

enum class DemoFrameTag : uint8_t {
    Tick  = 0,
    Cycle,
    Chat,
    Kill,
    Turn,
    RoundBegin,
    RoundEnd,
};

struct DemoFrame {
    DemoFrameTag tag;
    double       gameTime;  ///< se_GameTime() value when event was generated

    union {
        // For DemoFrameTag::Tick — game-tick meta
        struct {
            float dt;   ///< delta-time (seconds)
            int   tick; ///< frame counter
        } tick;

        // For DemoFrameTag::Cycle
        DemoCycleState cycle;

        // For DemoFrameTag::Chat
        DemoChatEvent chat;

        // For DemoFrameTag::Kill
        DemoKillEvent kill;

        // For DemoFrameTag::Turn
        DemoTurnEvent turn;
    };

    // ── Convenience constructors ────────────────────────────────────────
    static DemoFrame MakeTick(double t, float dt, int tick) {
        DemoFrame f;
        f.tag          = DemoFrameTag::Tick;
        f.gameTime     = t;
        f.tick.dt      = dt;
        f.tick.tick    = tick;
        return f;
    }

    static DemoFrame MakeCycle(double t, const DemoCycleState& cs) {
        DemoFrame f;
        f.tag      = DemoFrameTag::Cycle;
        f.gameTime = t;
        f.cycle    = cs;
        return f;
    }

    static DemoFrame MakeChat(double t, const DemoChatEvent& ce) {
        DemoFrame f;
        f.tag      = DemoFrameTag::Chat;
        f.gameTime = t;
        f.chat     = ce;
        return f;
    }

    static DemoFrame MakeKill(double t, const DemoKillEvent& ke) {
        DemoFrame f;
        f.tag      = DemoFrameTag::Kill;
        f.gameTime = t;
        f.kill     = ke;
        return f;
    }

    static DemoFrame MakeTurn(double t, const DemoTurnEvent& te) {
        DemoFrame f;
        f.tag      = DemoFrameTag::Turn;
        f.gameTime = t;
        f.turn     = te;
        return f;
    }

    static DemoFrame MakeRoundBegin(double t) {
        DemoFrame f;
        f.tag      = DemoFrameTag::RoundBegin;
        f.gameTime = t;
        return f;
    }

    static DemoFrame MakeRoundEnd(double t) {
        DemoFrame f;
        f.tag      = DemoFrameTag::RoundEnd;
        f.gameTime = t;
        return f;
    }
};

// ── Lock-free single-producer / single-consumer ring buffer ──────────────
// The game thread is the single producer; the disk-writer thread is the
// single consumer.  Two separate atomics (head_ / tail_) make this
// wait-free for both sides — no mutex needed in the hot path.
class DemoRingBuffer {
public:
    DemoRingBuffer();

    /// Try to push one frame.  Returns false if buffer is full (frame dropped).
    bool TryPush(const DemoFrame& frame) noexcept;

    /// Try to pop one frame.  Returns false if buffer is empty.
    bool TryPop(DemoFrame& out) noexcept;

    /// Number of used slots (approximate, without lock).
    size_t Size() const noexcept;

    /// Whether the ring has any data at all.
    bool Empty() const noexcept { return Size() == 0; }

    /// Total frames dropped due to overflow since construction.
    uint64_t DroppedFrames() const noexcept { return droppedFrames_.load(std::memory_order_relaxed); }

private:
    // Fixed-size storage – pre-allocated to avoid in-game heap allocs
    DemoFrame buf_[DEMO_RING_CAPACITY];

    // Written exclusively by producer; read by both
    std::atomic<size_t> head_{ 0 };
    // Written exclusively by consumer; read by both
    std::atomic<size_t> tail_{ 0 };

    std::atomic<uint64_t> droppedFrames_{ 0 };
};

// ── Recording statistics (for the UI overlay) ────────────────────────────
struct DemoRecorderStats {
    std::atomic<uint64_t> totalFrames{ 0 };
    std::atomic<uint64_t> droppedFrames{ 0 };
    std::atomic<uint64_t> bytesWritten{ 0 };
    std::atomic<bool>     writing{ false };
    std::atomic<double>   duration{ 0.0 };   ///< seconds recorded so far
};

// ── DemoRecorder ─────────────────────────────────────────────────────────
class DemoRecorder {
public:
    // Singleton access
    static DemoRecorder& Instance();

    // Non-copyable
    DemoRecorder(const DemoRecorder&) = delete;
    DemoRecorder& operator=(const DemoRecorder&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────
    /// Begin recording to the given file path.
    /// The .aarec file header is written synchronously; subsequent data
    /// is queued into the ring buffer and written by the background thread.
    /// @param filename  Absolute or relative path for the output file.
    /// @param matchId   Optional match identifier string (embedded in header).
    void StartRecording(const std::string& filename,
                        const std::string& matchId = "");

    /// Flush pending data and close the file.  Blocks until the writer
    /// thread has drained the ring buffer and written the EOF sentinel.
    void StopRecording();

    /// Returns true if a recording is currently active.
    bool IsRecording() const noexcept;

    // ── Hot-path data injection (called from game / render thread) ────
    /// Log a complete game tick.  Call once per Timestep() with all
    /// live cycle states.
    /// @param gameTime   Current game clock (seconds).
    /// @param dt         Frame delta time (seconds).
    /// @param tick       Monotonic frame counter.
    /// @param cycles     Snapshot of all cycle states this tick.
    void LogFrameData(double gameTime, float dt, int tick,
                      const std::vector<DemoCycleState>& cycles);

    /// Log a single chat message.
    void LogChat(double gameTime, int senderSlot, const char* message);

    /// Log a kill/death event.
    void LogKill(double gameTime, int killedSlot, int killerSlot);

    /// Log a turn.
    void LogTurn(double gameTime, int playerSlot, int direction);

    /// Notify that a new round has started.
    void LogRoundBegin(double gameTime);

    /// Notify that a round has ended.
    void LogRoundEnd(double gameTime);

    // ── Statistics ────────────────────────────────────────────────────
    const DemoRecorderStats& Stats() const noexcept { return stats_; }

    /// Human-readable path of the current (or last) recording.
    const std::string& CurrentFilename() const noexcept { return currentFilename_; }

    /// List of recently completed recordings (newest first, max 20).
    const std::vector<std::string>& RecentFiles() const noexcept { return recentFiles_; }

private:
    DemoRecorder();
    ~DemoRecorder();

    // ── Writer thread ─────────────────────────────────────────────────
    void WriterThreadMain();

    /// Write a single DemoFrame to the open file.
    void SerializeFrame(const DemoFrame& frame);

    /// Write the .aarec file header.
    void WriteFileHeader(const std::string& matchId);

    /// Write the end-of-file sentinel packet.
    void WriteFileEnd();

    // Helper: write raw bytes directly to file (no buffering needed –
    // fwrite already buffers internally via the C runtime).
    void Write(const void* data, size_t bytes);

    // Write little-endian integers
    void WriteU8(uint8_t v);
    void WriteU16(uint16_t v);
    void WriteU32(uint32_t v);
    void WriteU64(uint64_t v);
    void WriteF32(float v);
    void WriteF64(double v);
    void WriteStr(const char* s);

    // ── State ─────────────────────────────────────────────────────────
    std::atomic<bool>   recording_{ false };
    std::atomic<bool>   stopRequested_{ false };

    DemoRingBuffer      ring_;
    DemoRecorderStats   stats_;

    FILE*               file_{ nullptr };
    std::string         currentFilename_;
    std::vector<std::string> recentFiles_;

    std::thread         writerThread_;
    std::mutex          cvMutex_;
    std::condition_variable cv_;

    double startTime_{ 0.0 };
    double recordingTime_{ 0.0 };
    int    frameCounter_{ 0 };
};

// ── Convenience free functions (thin wrappers around Instance()) ──────────
namespace DemoRec {
    inline DemoRecorder& Get()          { return DemoRecorder::Instance(); }
    inline bool IsRecording()           { return Get().IsRecording(); }

    inline void LogFrameData(double gameTime, float dt, int tick,
                              const std::vector<DemoCycleState>& cycles) {
        if (IsRecording()) Get().LogFrameData(gameTime, dt, tick, cycles);
    }
    inline void LogChat(double t, int slot, const char* msg) {
        if (IsRecording()) Get().LogChat(t, slot, msg);
    }
    inline void LogKill(double t, int killed, int killer) {
        if (IsRecording()) Get().LogKill(t, killed, killer);
    }
    inline void LogTurn(double t, int slot, int dir) {
        if (IsRecording()) Get().LogTurn(t, slot, dir);
    }
    inline void LogRoundBegin(double t) {
        if (IsRecording()) Get().LogRoundBegin(t);
    }
    inline void LogRoundEnd(double t) {
        if (IsRecording()) Get().LogRoundEnd(t);
    }
}

#endif // !DEDICATED
