// =====================================================================
// MEDIA WIDGET — OS Media / Spotify HUD Overlay
// RetroCycles Client Mod | "ilonium"
// Integrates with HudManager drag-and-drop edit mode (isHudEditing).
// Uses a background thread to poll playerctl safely without lag.
// =====================================================================
#ifndef DEDICATED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "thirdparty/imgui/imgui_internal.h"
#include "HudManager.h"
#include "MediaWidget.h"
#include "tools/tConfiguration.h"
#include "tools/tDirectories.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// -----------------------------------------------------------------------
// Persistent configuration variables
// -----------------------------------------------------------------------
bool   sg_modMediaWidgetEnabled  = false;
REAL   sg_modMediaWidgetPosX     = 20.0f;
REAL   sg_modMediaWidgetPosY     = 400.0f;

static tConfItem<bool> sg_modMediaWidgetEnabledConf ("MOD_MEDIA_WIDGET_ENABLED", sg_modMediaWidgetEnabled);
static tConfItem<REAL> sg_modMediaWidgetPosXConf    ("MOD_MEDIA_WIDGET_POS_X",   sg_modMediaWidgetPosX);
static tConfItem<REAL> sg_modMediaWidgetPosYConf    ("MOD_MEDIA_WIDGET_POS_Y",   sg_modMediaWidgetPosY);

// Per-widget style vars — defined using the same macro as other widgets in HudManager.cpp
REAL sg_hudMedia_Scale           = 1.0f;
REAL sg_hudMedia_Opacity         = 1.0f;
REAL sg_hudMedia_BgOpacity       = 1.0f;
bool sg_hudMedia_UseCustomColors = false;
REAL sg_hudMedia_TextColorR      = 1.0f;
REAL sg_hudMedia_TextColorG      = 1.0f;
REAL sg_hudMedia_TextColorB      = 1.0f;
REAL sg_hudMedia_TextColorA      = 1.0f;
REAL sg_hudMedia_BgColorR        = 10.0f/255.0f;
REAL sg_hudMedia_BgColorG        = 10.0f/255.0f;
REAL sg_hudMedia_BgColorB        = 12.0f/255.0f;
REAL sg_hudMedia_BgColorA        = 195.0f/255.0f;
REAL sg_hudMedia_BorderColorR    = 80.0f/255.0f;
REAL sg_hudMedia_BorderColorG    = 80.0f/255.0f;
REAL sg_hudMedia_BorderColorB    = 95.0f/255.0f;
REAL sg_hudMedia_BorderColorA    = 60.0f/255.0f;
REAL sg_hudMedia_AccentColorR    = 30.0f/255.0f;
REAL sg_hudMedia_AccentColorG    = 215.0f/255.0f;
REAL sg_hudMedia_AccentColorB    = 96.0f/255.0f;
REAL sg_hudMedia_AccentColorA    = 1.0f;
bool sg_hudMedia_RgbMode         = false;
REAL sg_hudMedia_RgbSpeed        = 1.0f;

// Config persistence
static tConfItem<REAL> sg_hudMedia_ScaleConf          ("MOD_HUD_MEDIA_SCALE",          sg_hudMedia_Scale);
static tConfItem<REAL> sg_hudMedia_OpacityConf        ("MOD_HUD_MEDIA_OPACITY",        sg_hudMedia_Opacity);
static tConfItem<REAL> sg_hudMedia_BgOpacityConf      ("MOD_HUD_MEDIA_BGOPACITY",      sg_hudMedia_BgOpacity);
static tConfItem<bool> sg_hudMedia_UseCustomColorsConf("MOD_HUD_MEDIA_CUSTOMCOLORS",   sg_hudMedia_UseCustomColors);
static tConfItem<REAL> sg_hudMedia_TextColorRConf     ("MOD_HUD_MEDIA_TEXTCOLOR_R",    sg_hudMedia_TextColorR);
static tConfItem<REAL> sg_hudMedia_TextColorGConf     ("MOD_HUD_MEDIA_TEXTCOLOR_G",    sg_hudMedia_TextColorG);
static tConfItem<REAL> sg_hudMedia_TextColorBConf     ("MOD_HUD_MEDIA_TEXTCOLOR_B",    sg_hudMedia_TextColorB);
static tConfItem<REAL> sg_hudMedia_TextColorAConf     ("MOD_HUD_MEDIA_TEXTCOLOR_A",    sg_hudMedia_TextColorA);
static tConfItem<REAL> sg_hudMedia_BgColorRConf       ("MOD_HUD_MEDIA_BGCOLOR_R",      sg_hudMedia_BgColorR);
static tConfItem<REAL> sg_hudMedia_BgColorGConf       ("MOD_HUD_MEDIA_BGCOLOR_G",      sg_hudMedia_BgColorG);
static tConfItem<REAL> sg_hudMedia_BgColorBConf       ("MOD_HUD_MEDIA_BGCOLOR_B",      sg_hudMedia_BgColorB);
static tConfItem<REAL> sg_hudMedia_BgColorAConf       ("MOD_HUD_MEDIA_BGCOLOR_A",      sg_hudMedia_BgColorA);
static tConfItem<REAL> sg_hudMedia_BorderColorRConf   ("MOD_HUD_MEDIA_BORDERCOLOR_R",  sg_hudMedia_BorderColorR);
static tConfItem<REAL> sg_hudMedia_BorderColorGConf   ("MOD_HUD_MEDIA_BORDERCOLOR_G",  sg_hudMedia_BorderColorG);
static tConfItem<REAL> sg_hudMedia_BorderColorBConf   ("MOD_HUD_MEDIA_BORDERCOLOR_B",  sg_hudMedia_BorderColorB);
static tConfItem<REAL> sg_hudMedia_BorderColorAConf   ("MOD_HUD_MEDIA_BORDERCOLOR_A",  sg_hudMedia_BorderColorA);
static tConfItem<REAL> sg_hudMedia_AccentColorRConf   ("MOD_HUD_MEDIA_ACCENTCOLOR_R",  sg_hudMedia_AccentColorR);
static tConfItem<REAL> sg_hudMedia_AccentColorGConf   ("MOD_HUD_MEDIA_ACCENTCOLOR_G",  sg_hudMedia_AccentColorG);
static tConfItem<REAL> sg_hudMedia_AccentColorBConf   ("MOD_HUD_MEDIA_ACCENTCOLOR_B",  sg_hudMedia_AccentColorB);
static tConfItem<REAL> sg_hudMedia_AccentColorAConf   ("MOD_HUD_MEDIA_ACCENTCOLOR_A",  sg_hudMedia_AccentColorA);
static tConfItem<bool> sg_hudMedia_RgbModeConf        ("MOD_HUD_MEDIA_RGBMODE",        sg_hudMedia_RgbMode);
static tConfItem<REAL> sg_hudMedia_RgbSpeedConf       ("MOD_HUD_MEDIA_RGBSPEED",       sg_hudMedia_RgbSpeed);

// Helper function to safely execute processes
static ImVec4 GetRgbColor(float speed, float offset = 0.0f) {
    float hue = fmodf((float)ImGui::GetTime() * 0.2f * speed + offset, 1.0f);
    ImVec4 col;
    ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.95f, col.x, col.y, col.z);
    col.w = 1.0f;
    return col;
}

void SendMediaCommand(int cmd) {
    tString path = tDirectories::Var().GetWritePath("retrocycles_media_cmd.txt");
    FILE* f = fopen((const char*)path, "w");
    if (!f) return;
    switch (cmd) {
        case -1: fprintf(f, "prev\n"); break;
        case  0: fprintf(f, "play-pause\n"); break;
        case  1: fprintf(f, "next\n"); break;
    }
    fclose(f);
}

// Static instance initialization
MediaWidget* MediaWidget::s_Instance = nullptr;

// -----------------------------------------------------------------------
// MediaWidget Implementation
// -----------------------------------------------------------------------

MediaWidget::MediaWidget()
    : HudWidget("Media",
                ImVec2(sg_modMediaWidgetPosX, sg_modMediaWidgetPosY),
                ImVec2(380.0f, 125.0f),
                &sg_hudMedia_Scale, &sg_hudMedia_Opacity, &sg_hudMedia_BgOpacity, &sg_hudMedia_UseCustomColors,
                &sg_hudMedia_TextColorR, &sg_hudMedia_TextColorG, &sg_hudMedia_TextColorB, &sg_hudMedia_TextColorA,
                &sg_hudMedia_BgColorR,   &sg_hudMedia_BgColorG,   &sg_hudMedia_BgColorB,   &sg_hudMedia_BgColorA,
                &sg_hudMedia_BorderColorR, &sg_hudMedia_BorderColorG, &sg_hudMedia_BorderColorB, &sg_hudMedia_BorderColorA,
                &sg_hudMedia_AccentColorR, &sg_hudMedia_AccentColorG, &sg_hudMedia_AccentColorB, &sg_hudMedia_AccentColorA,
                &sg_hudMedia_RgbMode, &sg_hudMedia_RgbSpeed)
    , m_ThreadShouldExit(false)
    , m_ThreadRunning(false)
    , m_IsPlaying(false)
    , m_CurrentPosition(0.0f)
    , m_TotalLength(0.0f)
    , m_TimeSinceLastPosUpdate(0.0f)
    , m_LastPolledPos(-1.0f)
    , m_MarqueeOffset(0.0f)
    , m_VinylAngle(0.0f)
    , m_NeedleAngle(-0.15f)
{
    s_Instance = this;
    m_SharedState.isPlaying = false;
    m_SharedState.position = 0.0f;
    m_SharedState.length = 0.0f;
    memset(m_SharedState.title, 0, sizeof(m_SharedState.title));
    memset(m_SharedState.artist, 0, sizeof(m_SharedState.artist));
    memset(m_SharedState.album, 0, sizeof(m_SharedState.album));
    strncpy(m_SharedState.title, "No media playing", sizeof(m_SharedState.title) - 1);

    memset(m_Title,  0, sizeof(m_Title));
    memset(m_Artist, 0, sizeof(m_Artist));
    memset(m_Album,  0, sizeof(m_Album));
    strncpy(m_Title,  "No media playing", sizeof(m_Title) - 1);

    for (int i = 0; i < NUM_EQ_BARS; i++) {
        m_EqHeights[i] = 0.0f;
        m_EqPeaks[i] = 0.0f;
        m_EqPeakHold[i] = 0.0f;
    }
    m_BtnHoverT[0] = m_BtnHoverT[1] = m_BtnHoverT[2] = 0.0f;

    StartPollingThread();
}

MediaWidget::~MediaWidget() {
    StopPollingThread();
    if (s_Instance == this) {
        s_Instance = nullptr;
    }
}

void MediaWidget::StartPollingThread() {
    m_ThreadShouldExit = false;
    m_ThreadRunning = false;
    m_PollThread = std::thread(&MediaWidget::PollingThreadFunc, this);
}

void MediaWidget::StopPollingThread() {
    m_ThreadShouldExit = true;
    if (m_PollThread.joinable()) {
        m_PollThread.join();
    }
}

void MediaWidget::PollingThreadFunc() {
    m_ThreadRunning = true;

    while (!m_ThreadShouldExit) {
        // Read state from IPC file
        tString path = tDirectories::Var().GetReadPath("retrocycles_media_state.txt");
        FILE* f = fopen((const char*)path, "r");
        
        bool isPlaying = false;
        float position = 0.0f;
        float length = 0.0f;
        char title[256] = "No media playing";
        char artist[256] = "";
        char album[256] = "";

        if (f) {
            char buf[1024];
            if (fgets(buf, sizeof(buf), f)) {
                // Remove trailing newline
                size_t len_str = strlen(buf);
                while (len_str > 0 && (buf[len_str - 1] == '\n' || buf[len_str - 1] == '\r')) {
                    buf[len_str - 1] = '\0';
                    len_str--;
                }

                std::vector<std::string> parts;
                std::stringstream ss(buf);
                std::string item;
                while (std::getline(ss, item, '|')) {
                    parts.push_back(item);
                }

                if (parts.size() >= 3) {
                    isPlaying = (parts[0] == "Playing");
                    
                    // Position is in microseconds (Spotify/MPRIS)
                    if (!parts[1].empty()) {
                        position = std::max(0.0f, (float)atof(parts[1].c_str()) / 1000000.0f);
                    }
                    if (!parts[2].empty()) {
                        length = std::max(0.0f, (float)atof(parts[2].c_str()) / 1000000.0f);
                    }

                    if (parts.size() >= 4 && !parts[3].empty()) {
                        strncpy(title, parts[3].c_str(), sizeof(title) - 1);
                        title[sizeof(title) - 1] = '\0';
                    }
                    if (parts.size() >= 5 && !parts[4].empty()) {
                        strncpy(artist, parts[4].c_str(), sizeof(artist) - 1);
                        artist[sizeof(artist) - 1] = '\0';
                    }
                    if (parts.size() >= 6 && !parts[5].empty()) {
                        strncpy(album, parts[5].c_str(), sizeof(album) - 1);
                        album[sizeof(album) - 1] = '\0';
                    }
                }
            }
            fclose(f);
        }

        // Lock and update shared state
        {
            std::lock_guard<std::mutex> lock(m_StateMutex);
            m_SharedState.isPlaying = isPlaying;
            m_SharedState.position = position;
            m_SharedState.length = length;
            strncpy(m_SharedState.title, title, sizeof(m_SharedState.title) - 1);
            strncpy(m_SharedState.artist, artist, sizeof(m_SharedState.artist) - 1);
            strncpy(m_SharedState.album, album, sizeof(m_SharedState.album) - 1);
        }

        // Check exit flag in small sleep increments to remain responsive
        int sleepMs = isPlaying ? 100 : 500;
        for (int i = 0; i < sleepMs / 50; i++) {
            if (m_ThreadShouldExit) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    m_ThreadRunning = false;
}

bool MediaWidget::IsVisible() const        { return sg_modMediaWidgetEnabled; }
void MediaWidget::SetVisible(bool visible) { sg_modMediaWidgetEnabled = visible; }

ImVec2 MediaWidget::GetPosition() const {
    ImVec2 pos(sg_modMediaWidgetPosX, sg_modMediaWidgetPosY);
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x > 50.0f && displaySize.y > 50.0f) {
        pos.x = std::max(0.0f, std::min(pos.x, displaySize.x - m_Size.x));
        pos.y = std::max(0.0f, std::min(pos.y, displaySize.y - m_Size.y));
    }
    return pos;
}

void MediaWidget::SetPosition(const ImVec2& rawPos) {
    ImVec2 pos = rawPos;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x > 50.0f && displaySize.y > 50.0f) {
        pos.x = std::max(0.0f, std::min(pos.x, displaySize.x - m_Size.x));
        pos.y = std::max(0.0f, std::min(pos.y, displaySize.y - m_Size.y));
    }
    sg_modMediaWidgetPosX = pos.x;
    sg_modMediaWidgetPosY = pos.y;
}

void MediaWidget::Update(float dt) {
    HudWidget::Update(dt);

    // 1. Thread sync
    bool threadIsPlaying = false;
    float threadPos = 0.0f;
    float threadLen = 0.0f;
    {
        std::lock_guard<std::mutex> lock(m_StateMutex);
        threadIsPlaying = m_SharedState.isPlaying;
        threadPos = m_SharedState.position;
        threadLen = m_SharedState.length;

        m_IsPlaying = threadIsPlaying;
        m_TotalLength = threadLen;

        if (strcmp(m_Title, m_SharedState.title) != 0) {
            strncpy(m_Title, m_SharedState.title, sizeof(m_Title) - 1);
            m_MarqueeOffset = 0.0f; // Reset marquee
        }
        strncpy(m_Artist, m_SharedState.artist, sizeof(m_Artist) - 1);
        strncpy(m_Album, m_SharedState.album, sizeof(m_Album) - 1);
    }

    // 2. Track time interpolation
    if (threadPos != m_LastPolledPos) {
        m_LastPolledPos = threadPos;
        m_CurrentPosition = threadPos;
        m_TimeSinceLastPosUpdate = 0.0f;
    } else {
        if (m_IsPlaying) {
            m_TimeSinceLastPosUpdate += dt;
            m_CurrentPosition = threadPos + m_TimeSinceLastPosUpdate;
        }
    }
    if (m_CurrentPosition > m_TotalLength) {
        m_CurrentPosition = m_TotalLength;
    }

    // 3. Marquee scroll advance
    float titleW = ImGui::CalcTextSize(m_Title).x;
    float availW = 165.0f; // Clip rect width
    if (titleW > availW) {
        if (m_IsPlaying) {
            m_MarqueeOffset += 28.0f * dt; // Pixels per second
            if (m_MarqueeOffset > titleW + 30.0f) {
                m_MarqueeOffset = 0.0f;
            }
        } else {
            m_MarqueeOffset = ImLerp(m_MarqueeOffset, 0.0f, dt * 4.0f);
        }
    } else {
        m_MarqueeOffset = 0.0f;
    }

    // 4. Equalizer Bars logic
    const float eqFreqs[NUM_EQ_BARS] = { 3.1f, 4.5f, 2.2f, 5.8f, 3.9f, 6.2f, 2.7f, 4.8f, 5.1f, 3.4f };
    const float eqPhases[NUM_EQ_BARS] = { 0.0f, 1.1f, 2.4f, 0.5f, 1.8f, 3.0f, 0.9f, 2.1f, 1.5f, 0.2f };
    float time = (float)ImGui::GetTime();

    for (int i = 0; i < NUM_EQ_BARS; i++) {
        float target = 0.0f;
        if (m_IsPlaying) {
            target = 0.2f + 0.6f * (0.5f * sinf(time * eqFreqs[i] + eqPhases[i]) + 
                                    0.5f * cosf(time * (eqFreqs[i] * 0.7f) + eqPhases[i] * 1.5f));
            target += 0.1f * ((rand() % 100) / 100.0f);
            target = std::max(0.05f, std::min(1.0f, target));
        } else {
            target = 0.0f;
        }

        m_EqHeights[i] = ImLerp(m_EqHeights[i], target, dt * 10.0f);

        if (m_EqHeights[i] > m_EqPeaks[i]) {
            m_EqPeaks[i] = m_EqHeights[i];
            m_EqPeakHold[i] = 0.6f;
        } else {
            m_EqPeakHold[i] -= dt;
            if (m_EqPeakHold[i] <= 0.0f) {
                m_EqPeaks[i] = ImLerp(m_EqPeaks[i], m_EqHeights[i], dt * 3.0f);
            }
        }
    }

    // 5. Vinyl record rotation + needle arm LERP
    if (m_IsPlaying) {
        m_VinylAngle += dt * 1.8f;
        if (m_VinylAngle > 2.0f * M_PI) {
            m_VinylAngle -= 2.0f * M_PI;
        }
        m_NeedleAngle = ImLerp(m_NeedleAngle, 1.20f, dt * 4.0f);
    } else {
        m_VinylAngle = ImLerp(m_VinylAngle, m_VinylAngle, dt * 2.0f);
        m_NeedleAngle = ImLerp(m_NeedleAngle, -0.15f, dt * 4.0f);
    }
}

void MediaWidget::Draw() {
    if (m_Alpha <= 0.0f) return;
    Draw(ImGui::GetBackgroundDrawList(), GetPosition() + m_SlideOffset, m_Alpha);
}

void MediaWidget::Draw(ImDrawList* dl, ImVec2 pos, float alphaMult) {
    if (alphaMult <= 0.0f) return;

    // Temporarily redirect opacity pointers and set m_Alpha to draw with dashboard's alpha
    REAL* savedOpacityPtr = m_pOpacity;
    REAL* savedBgOpacityPtr = m_pBgOpacity;
    float savedAlpha = m_Alpha;

    REAL tempOpacity = alphaMult * (savedOpacityPtr ? *savedOpacityPtr : 1.0f);
    REAL tempBgOpacity = alphaMult * (savedBgOpacityPtr ? *savedBgOpacityPtr : 1.0f);

    const_cast<MediaWidget*>(this)->m_pOpacity = &tempOpacity;
    const_cast<MediaWidget*>(this)->m_pBgOpacity = &tempBgOpacity;
    const_cast<MediaWidget*>(this)->m_Alpha = 1.0f;

    bool isEditing = isHudEditing && (dl == ImGui::GetBackgroundDrawList());

    float scale = GetScale();
    m_Size = ImVec2(380.0f * scale, 125.0f * scale);

    float       W   = m_Size.x;
    float       H   = m_Size.y;
    ImVec2      pMax(pos.x + W, pos.y + H);

    // 1. Background Glassmorphism
    ImU32 bgCol     = GetBgCol();
    ImU32 borderCol = isEditing ? GetColorWithAlpha(1.0f, 0.4f, 0.0f, 0.9f) : GetBorderCol();

    if (isEditing) {
        dl->AddRectFilled(ImVec2(pos.x - 4.0f * scale, pos.y - 4.0f * scale), ImVec2(pMax.x + 4.0f * scale, pMax.y + 4.0f * scale),
                          IM_COL32(255, 100, 0, (int)(30 * alphaMult)), 14.0f * scale);
    }

    dl->AddRectFilled(pos, pMax, bgCol, 16.0f * scale);
    dl->AddRect(pos, pMax, borderCol, 16.0f * scale, 0, isEditing ? 2.0f * scale : 1.0f * scale);

    // 2. Vinyl Record Drawing
    ImU32 centerCol;
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
        centerCol = IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(a * tempOpacity * 255.0f));
    } else {
        centerCol = IM_COL32(30, 215, 96, (int)(255 * tempOpacity));
    }

    ImVec2 vinylCenter(pos.x + 42.0f * scale, pos.y + 45.0f * scale);
    float vinylRadius = 26.0f * scale;
    dl->AddCircleFilled(vinylCenter, vinylRadius, IM_COL32(18, 18, 20, (int)(255 * tempOpacity)));
    dl->AddCircle(vinylCenter, vinylRadius, IM_COL32(40, 40, 42, (int)(255 * tempOpacity)), 0, 1.0f * scale);

    float grooveRadii[] = { 8.0f * scale, 13.0f * scale, 18.0f * scale, 23.0f * scale };
    for (float r : grooveRadii) {
        dl->AddCircle(vinylCenter, r, IM_COL32(50, 50, 52, (int)(90 * tempOpacity)), 0, 0.8f * scale);
    }

    if (m_IsPlaying) {
        for (int i = 0; i < 4; i++) {
            float angle = m_VinylAngle + (i * M_PI / 2.0f);
            ImVec2 start(vinylCenter.x + cosf(angle) * 10.0f * scale, vinylCenter.y + sinf(angle) * 10.0f * scale);
            ImVec2 end(vinylCenter.x + cosf(angle) * 22.0f * scale, vinylCenter.y + sinf(angle) * 22.0f * scale);
            dl->AddLine(start, end, IM_COL32(65, 65, 68, (int)(110 * tempOpacity)), 1.0f * scale);
        }
    }

    dl->AddCircleFilled(vinylCenter, 7.0f * scale, centerCol);
    dl->AddCircleFilled(vinylCenter, 1.5f * scale, IM_COL32(255, 255, 255, (int)(255 * tempOpacity)));

    // 3. bent Tonearm Needle
    ImVec2 needlePivot(pos.x + 65.0f * scale, pos.y + 15.0f * scale);
    dl->AddCircleFilled(needlePivot, 4.0f * scale, IM_COL32(80, 80, 85, (int)(255 * tempOpacity)));
    dl->AddCircle(needlePivot, 4.0f * scale, IM_COL32(120, 120, 125, (int)(255 * tempOpacity)), 0, 1.0f * scale);

    float armAngle1 = m_NeedleAngle + 0.35f;
    ImVec2 midPoint(needlePivot.x + cosf(armAngle1) * 15.0f * scale, needlePivot.y + sinf(armAngle1) * 15.0f * scale);
    dl->AddLine(needlePivot, midPoint, IM_COL32(180, 180, 185, (int)(255 * tempOpacity)), 1.5f * scale);

    float armAngle2 = m_NeedleAngle + 0.95f;
    ImVec2 endPoint(midPoint.x + cosf(armAngle2) * 18.0f * scale, midPoint.y + sinf(armAngle2) * 18.0f * scale);
    dl->AddLine(midPoint, endPoint, IM_COL32(180, 180, 185, (int)(255 * tempOpacity)), 1.5f * scale);

    ImVec2 headEnd(endPoint.x + cosf(armAngle2 + 0.2f) * 3.0f * scale, endPoint.y + sinf(armAngle2 + 0.2f) * 3.0f * scale);
    dl->AddLine(endPoint, headEnd, IM_COL32(40, 40, 40, (int)(255 * tempOpacity)), 3.0f * scale);

    // 4. Track Info: Title, Artist, Album
    {
        float textX = pos.x + 85.0f * scale;
        float textW = 165.0f * scale;
        float titleY = pos.y + 12.0f * scale;

        ImVec2 clipMin(textX, titleY - 2.0f * scale);
        ImVec2 clipMax(textX + textW, titleY + ImGui::GetTextLineHeight() * scale + 2.0f * scale);
        dl->PushClipRect(clipMin, clipMax, true);

        float titleW = CalcTextSize(m_Title).x;
        if (titleW > textW) {
            float drawX = textX - m_MarqueeOffset * scale;
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawX, titleY), GetTextCol(), m_Title);
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(drawX + titleW + 30.0f * scale, titleY), GetTextCol(), m_Title);
        } else {
            dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textX, titleY), GetTextCol(), m_Title);
        }
        dl->PopClipRect();

        // Artist & Album Lines
        float artistY = titleY + ImGui::GetTextLineHeight() * scale + 3.0f * scale;
        ImU32 artistCol = GetColorWithAlpha(0.75f, 0.75f, 0.78f, 0.85f);
        dl->PushClipRect(ImVec2(textX, artistY - 1.0f * scale), ImVec2(textX + textW, artistY + ImGui::GetTextLineHeight() * scale + 2.0f * scale), true);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textX, artistY), artistCol, m_Artist[0] ? m_Artist : "Unknown artist");
        dl->PopClipRect();

        float albumY = artistY + ImGui::GetTextLineHeight() * scale + 2.0f * scale;
        ImU32 albumCol = GetColorWithAlpha(0.55f, 0.55f, 0.58f, 0.70f);
        dl->PushClipRect(ImVec2(textX, albumY - 1.0f * scale), ImVec2(textX + textW, albumY + ImGui::GetTextLineHeight() * scale + 2.0f * scale), true);
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(textX, albumY), albumCol, m_Album[0] ? m_Album : "No Album");
        dl->PopClipRect();
    }

    // 5. 10-Bar Reactive Equalizer (Top-Right)
    {
        const float BAR_W = 6.0f * scale;
        const float BAR_GAP = 3.0f * scale;
        const float BAR_MAXH = 32.0f * scale;
        float eqStartX = pos.x + 265.0f * scale;
        float eqBaseY = pos.y + 48.0f * scale;

        for (int i = 0; i < NUM_EQ_BARS; i++) {
            float barH = m_EqHeights[i] * BAR_MAXH;
            float peakH = m_EqPeaks[i] * BAR_MAXH;
            float bx = eqStartX + i * (BAR_W + BAR_GAP);

            // Colorful Spotify green gradient
            float accentBlend = m_EqHeights[i];
            ImU32 barCol = GetColorWithAlpha(
                ImLerp(30.0f / 255.0f, 0.0f / 255.0f, accentBlend),
                ImLerp(215.0f / 255.0f, 255.0f / 255.0f, accentBlend),
                ImLerp(96.0f / 255.0f, 150.0f / 255.0f, accentBlend),
                0.90f
            );

            // Bar background slot
            dl->AddRectFilled(ImVec2(bx, eqBaseY - BAR_MAXH), ImVec2(bx + BAR_W, eqBaseY), 
                              IM_COL32(40, 40, 45, 60), 1.5f * scale);

            // Real filled bar
            if (barH > 0.5f) {
                dl->AddRectFilled(ImVec2(bx, eqBaseY - barH), ImVec2(bx + BAR_W, eqBaseY), barCol, 1.5f * scale);
            }

            // Floating peak dot
            ImU32 peakCol = GetColorWithAlpha(0.8f, 1.0f, 0.8f, 0.95f);
            dl->AddRectFilled(ImVec2(bx, eqBaseY - peakH - 1.5f * scale), ImVec2(bx + BAR_W, eqBaseY - peakH), peakCol, 0.5f * scale);
        }
    }

    // 6. Interactive Progress Bar & Time Tracker
    float pbY = pos.y + 70.0f * scale;
    float pbX0 = pos.x + 15.0f * scale;
    float pbX1 = pos.x + W - 15.0f * scale;
    float pbH = 3.0f * scale;

    ImGuiIO& io = ImGui::GetIO();
    bool pbHovered = (io.MousePos.x >= pbX0 && io.MousePos.x <= pbX1 &&
                      io.MousePos.y >= pbY - 5.0f * scale && io.MousePos.y <= pbY + 7.0f * scale);

    float progressNorm = (m_TotalLength > 0.0f) ? (m_CurrentPosition / m_TotalLength) : 0.0f;
    progressNorm = std::max(0.0f, std::min(1.0f, progressNorm));

    // Seek interaction
    if (pbHovered && !isEditing && io.MouseClicked[0] && m_TotalLength > 0.0f) {
        float tParam = (io.MousePos.x - pbX0) / (pbX1 - pbX0);
        float seekSec = tParam * m_TotalLength;
        tString pathCmd = tDirectories::Var().GetWritePath("retrocycles_media_cmd.txt");
        FILE* fCmd = fopen((const char*)pathCmd, "w");
        if (fCmd) {
            fprintf(fCmd, "seek%f\n", seekSec);
            fclose(fCmd);
        }
        m_CurrentPosition = seekSec; // immediate visual response
        progressNorm = seekSec / m_TotalLength;
    }

    // Track Background
    dl->AddRectFilled(ImVec2(pbX0, pbY), ImVec2(pbX1, pbY + pbH),
                      GetColorWithAlpha(0.2f, 0.2f, 0.25f, 0.70f), 2.0f * scale);

    // Track Fill
    float fillX = pbX0 + (pbX1 - pbX0) * progressNorm;
    if (fillX > pbX0) {
        dl->AddRectFilled(ImVec2(pbX0, pbY), ImVec2(fillX, pbY + pbH),
                          centerCol, 2.0f * scale);
    }

    // Handle/Cursor dot
    float cursorRadius = pbHovered ? 6.0f * scale : 4.0f * scale;
    ImU32 cursorCol;
    if (pbHovered) {
        cursorCol = IM_COL32(255, 255, 255, (int)(255 * tempOpacity));
    } else {
        if (UseCustomColors() || GetRgbMode()) {
            cursorCol = centerCol;
        } else {
            cursorCol = IM_COL32(255, 255, 255, (int)(220 * tempOpacity));
        }
    }
    dl->AddCircleFilled(ImVec2(fillX, pbY + pbH * 0.5f), cursorRadius, cursorCol);

    // Time Indicators
    int curMin = (int)m_CurrentPosition / 60;
    int curSec = (int)m_CurrentPosition % 60;
    int totMin = (int)m_TotalLength / 60;
    int totSec = (int)m_TotalLength % 60;
    char timeStrCur[32], timeStrTot[32];
    snprintf(timeStrCur, sizeof(timeStrCur), "%02d:%02d", curMin, curSec);
    snprintf(timeStrTot, sizeof(timeStrTot), "%02d:%02d", totMin, totSec);

    ImU32 timeCol = GetColorWithAlpha(0.6f, 0.6f, 0.65f, 0.75f);
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pbX0, pbY + 6.0f * scale), timeCol, timeStrCur);

    float totTimeW = CalcTextSize(timeStrTot).x;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(pbX1 - totTimeW, pbY + 6.0f * scale), timeCol, timeStrTot);

    // 7. Vector Controls Buttons
    {
        float btnW = 24.0f * scale;
        float btnH = 24.0f * scale;
        float btnY = pos.y + H - 24.0f * scale;
        float centerX = pos.x + W * 0.5f;
        float btnSpacing = btnW + 16.0f * scale;

        const ImVec4 green(30.0f/255.0f, 215.0f/255.0f, 96.0f/255.0f, 1.0f);
        const ImVec4 white(0.92f, 0.92f, 0.95f, 0.85f);
        const float  hoverSpeed = 10.0f;

        int         cmds[3] = { -1, 0, 1 };
        float       btnXs[3] = {
            centerX - btnSpacing - btnW * 0.5f,
            centerX - btnW * 0.5f,
            centerX + btnSpacing - btnW * 0.5f
        };

        for (int b = 0; b < 3; b++) {
            ImVec2 bMin(btnXs[b], btnY);
            ImVec2 bMax(btnXs[b] + btnW, btnY + btnH);

            bool hovered = (io.MousePos.x >= bMin.x && io.MousePos.x <= bMax.x &&
                            io.MousePos.y >= bMin.y && io.MousePos.y <= bMax.y);

            // Smooth hover tween
            float target = hovered ? 1.0f : 0.0f;
            m_BtnHoverT[b] = ImLerp(m_BtnHoverT[b], target, io.DeltaTime * hoverSpeed);

            // Interpolate button color: white/grey → green
            ImVec4 col(
                ImLerp(white.x, green.x, m_BtnHoverT[b]),
                ImLerp(white.y, green.y, m_BtnHoverT[b]),
                ImLerp(white.z, green.z, m_BtnHoverT[b]),
                1.0f
            );
            ImU32 btnCol = GetColorWithAlpha(col.x, col.y, col.z, col.w);

            // Click background circle glow on hover
            if (m_BtnHoverT[b] > 0.01f) {
                ImU32 hoverBg = IM_COL32(
                    (int)(green.x * 255 * m_BtnHoverT[b] * 0.15f),
                    (int)(green.y * 255 * m_BtnHoverT[b] * 0.15f),
                    (int)(green.z * 255 * m_BtnHoverT[b] * 0.15f),
                    (int)(60 * m_BtnHoverT[b] * tempOpacity));
                dl->AddCircleFilled(ImVec2(bMin.x + btnW*0.5f, bMin.y + btnH*0.5f), 13.0f * m_BtnHoverT[b] * scale, hoverBg);
            }

            ImVec2 center(bMin.x + btnW * 0.5f, bMin.y + btnH * 0.5f);

            // Draw vector icons
            if (b == 0) {
                // Prev button: |<
                float size = 6.0f * scale;
                // Bar
                dl->AddRectFilled(ImVec2(center.x - 7.0f * scale, center.y - size), ImVec2(center.x - 5.0f * scale, center.y + size), btnCol, 1.0f * scale);
                // Triangle 1
                dl->AddTriangleFilled(
                    ImVec2(center.x - 5.0f * scale, center.y),
                    ImVec2(center.x + 1.0f * scale, center.y - size),
                    ImVec2(center.x + 1.0f * scale, center.y + size),
                    btnCol
                );
                // Triangle 2
                dl->AddTriangleFilled(
                    ImVec2(center.x + 1.0f * scale, center.y),
                    ImVec2(center.x + 7.0f * scale, center.y - size),
                    ImVec2(center.x + 7.0f * scale, center.y + size),
                    btnCol
                );
            } else if (b == 1) {
                // Play / Pause
                float size = 7.0f * scale;
                if (m_IsPlaying) {
                    // Pause icon: ||
                    dl->AddRectFilled(ImVec2(center.x - 4.5f * scale, center.y - size), ImVec2(center.x - 1.5f * scale, center.y + size), btnCol, 1.0f * scale);
                    dl->AddRectFilled(ImVec2(center.x + 1.5f * scale, center.y - size), ImVec2(center.x + 4.5f * scale, center.y + size), btnCol, 1.0f * scale);
                } else {
                    // Play icon: >
                    dl->AddTriangleFilled(
                        ImVec2(center.x - 3.0f * scale, center.y - size),
                        ImVec2(center.x - 3.0f * scale, center.y + size),
                        ImVec2(center.x + 6.0f * scale, center.y),
                        btnCol
                    );
                }
            } else if (b == 2) {
                // Next button: >|
                float size = 6.0f * scale;
                // Triangle 1
                dl->AddTriangleFilled(
                    ImVec2(center.x - 7.0f * scale, center.y - size),
                    ImVec2(center.x - 7.0f * scale, center.y + size),
                    ImVec2(center.x - 1.0f * scale, center.y),
                    btnCol
                );
                // Triangle 2
                dl->AddTriangleFilled(
                    ImVec2(center.x - 1.0f * scale, center.y - size),
                    ImVec2(center.x - 1.0f * scale, center.y + size),
                    ImVec2(center.x + 5.0f * scale, center.y),
                    btnCol
                );
                // Bar
                dl->AddRectFilled(ImVec2(center.x + 5.0f * scale, center.y - size), ImVec2(center.x + 7.0f * scale, center.y + size), btnCol, 1.0f * scale);
            }

            // Click handling
            if (hovered && !isEditing && io.MouseClicked[0]) {
                SendMediaCommand(cmds[b]);
            }
        }
    }

    // 8. Edit mode label
    if (isEditing) {
        ImVec2 labelSz = CalcTextSize("MEDIA");
        float  lx = pos.x + (W - labelSz.x) * 0.5f;
        float  ly = pos.y + 2.0f * scale;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(lx, ly), GetColorWithAlpha(1.0f, 0.5f, 0.1f, 0.8f), "MEDIA");
    }

    // Restore original state
    const_cast<MediaWidget*>(this)->m_pOpacity = savedOpacityPtr;
    const_cast<MediaWidget*>(this)->m_pBgOpacity = savedBgOpacityPtr;
    const_cast<MediaWidget*>(this)->m_Alpha = savedAlpha;
}

#endif // !DEDICATED
