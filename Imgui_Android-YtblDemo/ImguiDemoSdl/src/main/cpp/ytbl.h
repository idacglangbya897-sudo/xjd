/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
#pragma once

#include "ytbl_config.h"
#include "ytbl_spring.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace ytbl {

bool Init();
void Shutdown();
bool IsInitialized();

void ResetGlassJobs();

void SetDensity(float d);
float Dp(float v);

void SetLightTheme(bool light);
bool IsLightTheme();

void SetBackdropEnabled(bool enabled);

ImVec4 FrostedBackdrop();

typedef void (*GlassDrawFn)(void* userdata, const ImRect& rect, const GlassShape& shape);

struct GlassParams {
    GlassShape shape;
    SimpleGlassEffects effects;
    Highlight highlight;
    bool hasHighlight = true;
    Shadow shadow;
    bool hasShadow = false;
    InnerShadow innerShadow;
    bool hasInnerShadow = false;
    ImVec4 surfaceColor = ImVec4(0, 0, 0, 0);
    float tintMode = 0.0f;
    float alpha = 1.0f;
    bool useBackdrop = true;
    bool opaqueBackdrop = false;
    float backdropScaleX = 1.0f;
    float backdropScaleY = 1.0f;
    float backdropMix = 0.0f;
    ImVec4 backdropFallback = ImVec4(0, 0, 0, 0);
    float ihProgress = 0.0f;
    float ihX = 0.0f, ihY = 0.0f;
    GlassDrawFn onDrawBehind = nullptr;  void* behindUser = nullptr;
    GlassDrawFn onDrawSurface = nullptr; void* surfaceUser = nullptr;
    GlassDrawFn onDrawContentBackground = nullptr; void* contentBgUser = nullptr;
    GlassDrawFn onDrawFront = nullptr;   void* frontUser = nullptr;
    GlassDrawFn onDrawDynamicOverlay = nullptr; void* dynamicUser = nullptr;
};

void DrawGlass(const ImRect& rect, const GlassParams& params);

class InteractiveHighlight {
public:
    static constexpr float kPressDampingRatio = 0.5f;
    static constexpr float kPressStiffness = 300.0f;
    static constexpr float kPressVisibilityThreshold = 0.001f;
    static constexpr float kPositionDampingRatio = 0.5f;
    static constexpr float kPositionStiffness = 300.0f;
    static constexpr float kPositionVisibilityThreshold = 0.5f;
    static constexpr float kBaseAlpha = 0.08f;
    static constexpr float kSpotAlpha = 0.15f;
    static constexpr float kFallbackAlpha = 0.25f;
    static constexpr float kRadiusMultiplier = 1.5f;

    InteractiveHighlight() = default;

    void Press(float px, float py);
    void Move(float px, float py);
    void Release();
    void Cancel();

    void Step(float dt);

    float GetPressProgress() const { return pressProgress_.value; }
    float GetOffsetX() const { return posX_.value - startX_; }
    float GetOffsetY() const { return posY_.value - startY_; }
    float GetPointerX() const { return posX_.value; }
    float GetPointerY() const { return posY_.value; }
    bool IsHighlightVisible() const { return enabled_ && pressProgress_.value > 0.0f; }
    bool IsAnimating() const { return animating_; }

    void SetIntensityScale(float s) { intensityScale_ = s < 0 ? 0 : s; }
    float GetIntensityScale() const { return intensityScale_; }
    void SetBaseAlpha(float a) { baseAlpha_ = a < 0 ? 0 : a; }
    float GetBaseAlpha() const { return baseAlpha_; }
    void SetSpotAlpha(float a) { spotAlpha_ = a < 0 ? 0 : a; }
    float GetSpotAlpha() const { return spotAlpha_; }
    void SetEnabled(bool e) { enabled_ = e; if (!e) Cancel(); }
    bool IsEnabled() const { return enabled_; }

    float baseAlpha_ = kBaseAlpha;
    float spotAlpha_ = kSpotAlpha;
    float fallbackAlpha_ = kFallbackAlpha;
    float intensityScale_ = 1.0f;
    float radiusMultiplier_ = kRadiusMultiplier;
    bool enabled_ = true;

private:
    SpringFloat pressProgress_{0.0f, kPressDampingRatio, kPressStiffness, kPressVisibilityThreshold};
    SpringFloat posX_{0.0f, kPositionDampingRatio, kPositionStiffness, kPositionVisibilityThreshold};
    SpringFloat posY_{0.0f, kPositionDampingRatio, kPositionStiffness, kPositionVisibilityThreshold};
    float startX_ = 0.0f, startY_ = 0.0f;
    bool animating_ = false;
};

struct LiquidDialog {
    bool visible = false;
    bool closing = false;
    const char* title = "Dialog Title";
    const char* message = "This is a liquid glass dialog.";
    int result = 0;
    SpringFloat progress{0.0f, 0.82f, 260.0f, 0.001f};

    void Open(const char* dialogTitle, const char* dialogMessage) {
        title = dialogTitle;
        message = dialogMessage;
        result = 0;
        closing = false;
        visible = true;
        progress.SnapTo(0.0f);
        progress.AnimateTo(1.0f);
    }
    void Close(int dialogResult) {
        result = dialogResult;
        closing = true;
        progress.AnimateTo(0.0f);
    }
    void Step(float dt) {
        if (!visible) return;
        progress.Step(dt);
        if (closing && progress.value <= 0.001f && progress.target <= 0.001f) {
            progress.SnapTo(0.0f);
            visible = false;
            closing = false;
        }
    }
    int Draw(const ImVec2& areaSizePx);
    bool IsClosing() const { return closing; }
    int GetResult() const { return result; }
};

struct LiquidButton {
    float widthDp = 120.0f;
    float heightDp = 48.0f;
    const char* label = "Button";
    bool interactive = true;
    bool hasTint = false;
    ImVec4 tint = ImVec4(1, 1, 1, 1);
    bool hasSurface = false;
    ImVec4 surfaceColor = ImVec4(1, 1, 1, 0.3f);
    int contentColor = 0xFFFFFFFF;
    int clickCount = 0;
    bool wasPressed = false;
    float textSizeSp = 16.0f;

    InteractiveHighlight highlight;

    bool Draw(const ImVec2& sizePx);
    void Step(float dt) {
        if (highlight.IsAnimating() || highlight.IsHighlightVisible()) highlight.Step(dt);
    }
};

struct LiquidToggle {
    bool selected = false;
    int accentColor = 0xFF30D158;
    int trackColor = 0x5C787880;
    float pressHighlightIntensity = 1.0f;
    bool wasToggled = false;
    bool touchActive = false;
    bool didDrag = false;
    float lastRawX = 0.0f;
    float heldSince = -1.0f;

    DampedDragAnimation animation{0.0f, 0.0f, 1.0f, 0.001f, 1.0f, 1.5f};

    bool Draw(const ImVec2& sizePx);
    void Step(float dt) { animation.Step(dt); }
    void SetSelected(bool value, bool animated = true) {
        selected = value;
        if (animated) animation.AnimateToValue(value ? 1.0f : 0.0f);
        else { animation.SnapValue(value ? 1.0f : 0.0f); }
    }
};

struct LiquidSlider {
    float rangeStart = 0.0f;
    float rangeEnd = 1.0f;
    float value = 0.5f;
    union {
        int accentColor = 0xFF0091FF;
        int effectiveAccent;
    };
    int trackColor = 0x5C787880;
    float pressHighlightIntensity = 1.0f;
    bool dragging = false;
    bool didDrag = false;
    float lastRawX = 0.0f;
    bool trackTapPending = false;
    float heldSince = -1.0f;

    DampedDragAnimation animation{0.5f, 0.0f, 1.0f, 0.001f, 1.0f, 1.5f};

    bool Draw(const ImVec2& sizePx);
    void Step(float dt) { animation.Step(dt); }
    float GetAnimatedValue() const { return animation.GetValue(); }
    float GetDisplayValue() const {
        return rangeStart + (rangeEnd - rangeStart) * Clamp01(animation.GetProgress());
    }
    void SetValueRange(float start, float end) {
        const float oldSpan = rangeEnd - rangeStart;
        const float oldProgress = oldSpan != 0.0f
            ? Clamp01((animation.GetValue() - rangeStart) / oldSpan) : 0.0f;
        const bool changedRange = start != rangeStart || end != rangeEnd;
        rangeStart = start;
        rangeEnd = end;
        animation.SetRange(start, end);
        value = Clampf(value, start, end);
        if (changedRange) {
            const float mapped = start + (end - start) * oldProgress;
            value = mapped;
            animation.SnapValue(mapped);
        }
    }
};

struct LiquidBottomTabs {
    int selected = 0;
    int count = 4;
    const char* labels[8] = {"首页", "发现", "消息", "我的", "Tab5", "Tab6", "Tab7", "Tab8"};
    int accentColor = 0xFF0091FF;
    int containerColor = 0x66121212;
    float pressHighlightIntensity = 1.0f;
    bool changed = false;

    bool AddTab(const char* text) {
        if (count >= 8) return false;
        labels[count++] = text;
        if (selected > count - 1) selected = count - 1;
        return true;
    }
    bool RemoveTabAt(int index) {
        if (index < 0 || index >= count) return false;
        for (int i = index; i < count - 1; i++) labels[i] = labels[i + 1];
        count--;
        if (selected > count - 1) selected = count - 1;
        return true;
    }
    void ClearTabs() { count = 0; selected = 0; }

    bool touchActive = false;
    bool gestureMoved = false;
    float lastRawX = 0.0f;
    float gestureDownX = 0.0f, gestureDownY = 0.0f;
    float rawDragOffsetPx = 0.0f;
    float heldSince = -1.0f;
    bool panelOffsetActive = false;
    bool didTapSelect = false;
    bool releaseHandled = false;

    DampedDragAnimation animation{0.0f, 0.0f, 3.0f, 0.001f, 1.0f, 78.0f / 56.0f};
    SpringFloat panelOffsetSpring{0.0f, 1.0f, 300.0f, 0.5f};
    InteractiveHighlight interactive;

    bool Draw(const ImVec2& sizePx);
    void Step(float dt) { animation.Step(dt); }
    float GetIndicatorValue() const { return animation.GetValue(); }
    void SetSelectedIndex(int index, bool animated = true) {
        if (index < 0) index = 0;
        if (index > count - 1) index = count - 1;
        selected = index;
        if (animated) animation.AnimateToValue((float)index);
        else animation.SnapValue((float)index);
    }
};

}
