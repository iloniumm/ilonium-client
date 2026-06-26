// =====================================================================
// MODERN MODULAR HUD & EDIT MODE SYSTEM
// RetroCycles Client Mod | "ilonium"
// =====================================================================
#pragma once

#ifndef DEDICATED
#include <vector>
#include <string>
#include "defs.h"
#include "thirdparty/imgui/imgui.h"
#include "tools/tString.h"

// UTF-8 / CP1251 encoding helper functions
bool IsValidUtf8(const std::string& str);
std::string Cp1251ToUtf8(const std::string& cp1251);
std::string ConvertNonUtf8ToUtf8(const std::string& str);
tString Utf8ToCp1251(const char* utf8);

// Font pointers declared in ModMenu
extern ImFont* g_FontDefault;
extern ImFont* g_FontHeader;


// HUD Global Settings (defined in HudManager.cpp)
extern bool sg_modClientNameEnabled;
extern bool sg_modFpsEnabled;
extern bool sg_modPingEnabled;
extern bool sg_modTimeEnabled;
extern bool sg_modKeybindsEnabled;
extern bool sg_modKDWidgetEnabled;
extern bool sg_modSpeedometerEnabled;
extern bool sg_modRubberMeterEnabled;
extern bool sg_modBrakeMeterEnabled;
extern bool sg_modScoreboardWidgetEnabled;
extern bool isHudEditing;
extern bool sg_demoRecorderOverlayEnabled;  // recording HUD pill on/off

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
extern REAL sg_modKDWidgetPosX;
extern REAL sg_modKDWidgetPosY;
extern REAL sg_modSpeedometerPosX;
extern REAL sg_modSpeedometerPosY;
extern REAL sg_modRubberMeterPosX;
extern REAL sg_modRubberMeterPosY;
extern REAL sg_modBrakeMeterPosX;
extern REAL sg_modBrakeMeterPosY;
extern REAL sg_modScoreboardWidgetPosX;
extern REAL sg_modScoreboardWidgetPosY;

class HudWidget {
public:
    HudWidget(const std::string& name, const ImVec2& defaultPos, const ImVec2& defaultSize,
              REAL* pScale = nullptr, REAL* pOpacity = nullptr, REAL* pBgOpacity = nullptr, bool* pUseCustomColors = nullptr,
              REAL* pTextColorR = nullptr, REAL* pTextColorG = nullptr, REAL* pTextColorB = nullptr, REAL* pTextColorA = nullptr,
              REAL* pBgColorR = nullptr, REAL* pBgColorG = nullptr, REAL* pBgColorB = nullptr, REAL* pBgColorA = nullptr,
              REAL* pBorderColorR = nullptr, REAL* pBorderColorG = nullptr, REAL* pBorderColorB = nullptr, REAL* pBorderColorA = nullptr,
              REAL* pAccentColorR = nullptr, REAL* pAccentColorG = nullptr, REAL* pAccentColorB = nullptr, REAL* pAccentColorA = nullptr,
              bool* pRgbMode = nullptr, REAL* pRgbSpeed = nullptr);
    virtual ~HudWidget() = default;

    virtual void Draw() = 0;
    virtual void Update(float dt);

    // Getters & Setters
    const std::string& GetName() const { return m_Name; }
    
    ImVec2 GetPosition() const;
    void SetPosition(const ImVec2& pos);
    
    const ImVec2& GetSize() const { return m_Size; }
    void SetSize(const ImVec2& size) { m_Size = size; }
    
    bool IsVisible() const;
    void SetVisible(bool visible);
    
    float GetAlpha() const { return m_Alpha; }

    bool IsDragging() const { return m_IsDragging; }
    void SetDragging(bool dragging) { m_IsDragging = dragging; }
    const ImVec2& GetDragMouseOffset() const { return m_DragMouseOffset; }
    void SetDragMouseOffset(const ImVec2& offset) { m_DragMouseOffset = offset; }

    float GetScale() const;
    float GetOpacitySetting() const;
    float GetBgOpacitySetting() const;
    bool UseCustomColors() const;
    bool GetRgbMode() const;
    float GetRgbSpeed() const;

    ImU32 GetTextCol(ImU32 defaultColor = IM_COL32(255, 255, 255, 255)) const;
    ImU32 GetBgCol() const;
    ImU32 GetBorderCol() const;
    ImU32 GetAccentCol(ImU32 defaultColor = IM_COL32(0, 190, 255, 255)) const;
    ImVec2 CalcTextSize(const char* text) const;

    // Config variable pointers
    REAL* m_pScale;
    REAL* m_pOpacity;
    REAL* m_pBgOpacity;
    bool* m_pUseCustomColors;
    REAL* m_pTextColorR; REAL* m_pTextColorG; REAL* m_pTextColorB; REAL* m_pTextColorA;
    REAL* m_pBgColorR; REAL* m_pBgColorG; REAL* m_pBgColorB; REAL* m_pBgColorA;
    REAL* m_pBorderColorR; REAL* m_pBorderColorG; REAL* m_pBorderColorB; REAL* m_pBorderColorA;
    REAL* m_pAccentColorR; REAL* m_pAccentColorG; REAL* m_pAccentColorB; REAL* m_pAccentColorA;
    bool* m_pRgbMode;
    REAL* m_pRgbSpeed;

protected:
    std::string m_Name;
    ImVec2 m_Size;
    float m_Alpha; // Lerped alpha (0.0f - 1.0f)
    bool m_IsVisible;

    // Animation targets for Lerp
    float m_TargetAlpha;
    ImVec2 m_SlideOffset;
    ImVec2 m_TargetSlideOffset;

    // Drag-and-drop state
    bool m_IsDragging;
    ImVec2 m_DragMouseOffset;

    // Helper for generating colors with widget alpha applied
    ImU32 GetColorWithAlpha(ImU32 baseColor, float alphaMultiplier = 1.0f) const;
    ImU32 GetColorWithAlpha(float r, float g, float b, float a) const;
};

class ClientNameWidget : public HudWidget {
public:
    ClientNameWidget();
    void Draw() override;
};

class FPSWidget : public HudWidget {
public:
    FPSWidget();
    void Draw() override;
};

class PingWidget : public HudWidget {
public:
    PingWidget();
    void Draw() override;
};

class TimeWidget : public HudWidget {
public:
    TimeWidget();
    void Draw() override;
};

class KeybindsWidget : public HudWidget {
public:
    KeybindsWidget();
    void Draw() override;
};

class KDWidget : public HudWidget {
public:
    KDWidget();
    void Draw() override;
};

class SpeedometerWidget : public HudWidget {
public:
    SpeedometerWidget();
    void Draw() override;
};

class RubberMeterWidget : public HudWidget {
public:
    RubberMeterWidget();
    void Draw() override;
};

class BrakeMeterWidget : public HudWidget {
public:
    BrakeMeterWidget();
    void Draw() override;
};

extern bool sg_modAliveWidgetEnabled;
extern REAL sg_modAliveWidgetPosX;
extern REAL sg_modAliveWidgetPosY;

// [MOD] New Widget Settings
extern bool sg_modLiveScoreboardEnabled;
extern bool sg_modZoneTimerEnabled;
extern bool sg_modScoreboardShowTeams;
extern bool sg_modScoreboardShowPlayers;
extern int sg_modScoreboardMaxPlayers;
extern bool sg_modScoreboardShowPing;

extern REAL sg_modLiveScoreboardPosX;
extern REAL sg_modLiveScoreboardPosY;
extern REAL sg_modZoneTimerPosX;
extern REAL sg_modZoneTimerPosY;

extern bool sg_modNetHealthEnabled;
extern REAL sg_modNetHealthPosX;
extern REAL sg_modNetHealthPosY;

extern bool sg_modRubberBatteryEnabled;
extern REAL sg_modRubberBatteryPosX;
extern REAL sg_modRubberBatteryPosY;

extern bool sg_modClassicRubberBatteryEnabled;
extern REAL sg_modClassicRubberBatteryPosX;
extern REAL sg_modClassicRubberBatteryPosY;

class ScoreboardWidget : public HudWidget {
public:
    ScoreboardWidget();
    void Draw() override;
};

class LiveScoreboardWidget : public HudWidget {
public:
    LiveScoreboardWidget();
    void Draw() override;
};

class ZoneTimerWidget : public HudWidget {
public:
    ZoneTimerWidget();
    void Draw() override;
};

class AliveWidget : public HudWidget {
public:
    AliveWidget();
    void Draw() override;
};

class NetworkHealthWidget : public HudWidget {
public:
    NetworkHealthWidget();
    void Draw() override;
};

class RubberBatteryWidget : public HudWidget {
public:
    RubberBatteryWidget();
    void Draw() override;
};

class ClassicRubberBatteryWidget : public HudWidget {
public:
    ClassicRubberBatteryWidget();
    void Draw() override;
};

class HudManager {
public:
    static void Init();
    static void Shutdown();
    static void Update(float dt);
    static void Render();

    static std::vector<HudWidget*>& GetWidgets() { return s_Widgets; }
    static HudWidget* FindWidget(const std::string& name);

private:
    static std::vector<HudWidget*> s_Widgets;
    static bool s_Initialized;
};

#endif
