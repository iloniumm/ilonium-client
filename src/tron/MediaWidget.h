// =====================================================================
// MediaWidget — Header
// RetroCycles Client Mod | "ilonium"
// =====================================================================
#pragma once
#ifndef DEDICATED
#include "HudManager.h"
#include <thread>
#include <mutex>
#include <atomic>

// Style vars (defined in MediaWidget.cpp)
extern REAL sg_hudMedia_Scale;
extern REAL sg_hudMedia_Opacity;
extern REAL sg_hudMedia_BgOpacity;
extern bool sg_hudMedia_UseCustomColors;
extern REAL sg_hudMedia_TextColorR, sg_hudMedia_TextColorG, sg_hudMedia_TextColorB, sg_hudMedia_TextColorA;
extern REAL sg_hudMedia_BgColorR,   sg_hudMedia_BgColorG,   sg_hudMedia_BgColorB,   sg_hudMedia_BgColorA;
extern REAL sg_hudMedia_BorderColorR, sg_hudMedia_BorderColorG, sg_hudMedia_BorderColorB, sg_hudMedia_BorderColorA;
extern REAL sg_hudMedia_AccentColorR, sg_hudMedia_AccentColorG, sg_hudMedia_AccentColorB, sg_hudMedia_AccentColorA;
extern bool sg_hudMedia_RgbMode;
extern REAL sg_hudMedia_RgbSpeed;

void SendMediaCommand(int cmd);

// -----------------------------------------------------------------------
class MediaWidget : public HudWidget {
public:
    MediaWidget();
    virtual ~MediaWidget();

    void Draw()           override;
    void Draw(ImDrawList* dl, ImVec2 pos, float alphaMult = 1.0f);
    void Update(float dt) override;

    bool   IsVisible()              const override;
    void   SetVisible(bool visible)       override;
    ImVec2 GetPosition()            const override;
    void   SetPosition(const ImVec2& pos) override;

    static MediaWidget* GetInstance() { return s_Instance; }

private:
    static MediaWidget* s_Instance;

    void StartPollingThread();
    void StopPollingThread();
    void PollingThreadFunc();

    // Background polling variables
    std::thread m_PollThread;
    std::mutex m_StateMutex;
    std::atomic<bool> m_ThreadShouldExit;
    std::atomic<bool> m_ThreadRunning;

    // Polled state populated by thread
    struct PolledState {
        bool  isPlaying;
        float position; // in seconds
        float length;   // in seconds
        char  title[256];
        char  artist[256];
        char  album[256];
    };
    PolledState m_SharedState;

    // Local interpolated/smoothed values
    bool  m_IsPlaying;
    float m_CurrentPosition; // in seconds, interpolated
    float m_TotalLength;     // in seconds
    float m_TimeSinceLastPosUpdate;
    float m_LastPolledPos;
    char  m_Title[256];
    char  m_Artist[256];
    char  m_Album[256];

    // Marquee scrolling
    float m_MarqueeOffset;

    // Advanced EQ visualizer: 10 bars
    static const int NUM_EQ_BARS = 10;
    float m_EqHeights[NUM_EQ_BARS];
    float m_EqPeaks[NUM_EQ_BARS];
    float m_EqPeakHold[NUM_EQ_BARS];

    // Vinyl Record spin animation angle
    float m_VinylAngle;
    float m_NeedleAngle;

    // Button hover timers
    float m_BtnHoverT[3]; // Prev, PlayPause, Next
};

#endif // !DEDICATED
