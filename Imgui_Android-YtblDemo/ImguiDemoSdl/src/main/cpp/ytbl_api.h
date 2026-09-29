/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
#pragma once

#include "ytbl.h"

namespace ytbl_api {

inline float density    = 2.0f;
inline bool  lightTheme = false;

inline float       buttonWidthDp   = 120.0f;
inline float       buttonHeightDp  = 48.0f;
inline const char* buttonText      = "液态玻璃按钮";
inline float       buttonTextSp    = 16.0f;
inline int         buttonTextColor = 0xFFFFFFFF;
inline bool        buttonTintOn    = true;
inline int         buttonTint      = 0x5C0091FF;
inline bool        buttonSurfaceOn = false;
inline int         buttonSurface   = 0x4DFFFFFF;

inline float toggleWidthDp  = 76.0f;
inline float toggleHeightDp = 40.0f;
inline bool  toggleOn       = false;
inline int   toggleAccent   = 0xFF30D158;
inline int   toggleTrack    = 0x5C787880;

inline float sliderWidthDp  = 300.0f;
inline float sliderHeightDp = 44.0f;
inline float sliderMin      = 0.0f;
inline float sliderMax      = 100.0f;
inline float sliderValue    = 35.0f;
inline int   sliderAccent   = 0xFF0091FF;

inline float tabsWidthDp  = 320.0f;
inline float tabsHeightDp = 56.0f;
inline int   tabCount     = 4;
inline int   tabIndex     = 0;
inline const char* tabLabel0 = "首页";
inline const char* tabLabel1 = "发现";
inline const char* tabLabel2 = "消息";
inline const char* tabLabel3 = "我的";
inline const char* tabLabel4 = "Tab5";
inline const char* tabLabel5 = "Tab6";
inline const char* tabLabel6 = "Tab7";
inline const char* tabLabel7 = "Tab8";
inline int   tabsAccent    = 0xFF0091FF;
inline int   tabsContainer = 0x66121212;

inline float panelCornerDp           = 24.0f;
inline float panelHeightDp           = 96.0f;
inline float panelBlurDp             = 10.0f;
inline float panelRefractionHeightDp = 24.0f;
inline float panelRefractionAmountDp = 24.0f;
inline bool  panelDepthEffect        = true;
inline bool  panelChromatic          = true;
inline float panelDispersion         = 0.9f;
inline bool  panelVibrancy           = true;
inline int   panelSurface            = 0x26FFFFFF;
inline int   panelShadowAlpha        = 0x1A000000;

inline const char* dialogTitle   = "液态玻璃弹窗";
inline const char* dialogMessage =
    "这是一个液态玻璃模态弹窗:\n遮罩 + 玻璃卡片 + 缩放弹簧入场/退场。\n"
    "点遮罩或\"取消\"关闭, 点\"确定\"返回 2。";
inline int   dialogResult        = 0;

inline ImVec4 Argb4(int c) {
    return ImVec4(((c >> 16) & 0xFF) / 255.0f,
                  ((c >> 8) & 0xFF) / 255.0f,
                  (c & 0xFF) / 255.0f,
                  ((c >> 24) & 0xFF) / 255.0f);
}

inline bool Init() {
    ytbl::SetDensity(density);
    ytbl::SetLightTheme(lightTheme);
    ytbl::SetBackdropEnabled(true);
    return ytbl::Init();
}

inline void Tick() {
    ytbl::ResetGlassJobs();
}

inline bool DrawButton() {
    static ytbl::LiquidButton btn;
    btn.label        = buttonText;
    btn.textSizeSp   = buttonTextSp;
    btn.contentColor = buttonTextColor;
    btn.hasTint      = buttonTintOn;
    btn.tint         = Argb4(buttonTint);
    btn.hasSurface   = buttonSurfaceOn;
    btn.surfaceColor = Argb4(buttonSurface);
    return btn.Draw(ImVec2(ytbl::Dp(buttonWidthDp), ytbl::Dp(buttonHeightDp)));
}

inline bool DrawToggle() {
    static ytbl::LiquidToggle tgl;
    tgl.accentColor = toggleAccent;
    tgl.trackColor  = toggleTrack;
    if (tgl.selected != toggleOn) tgl.SetSelected(toggleOn, false);
    const bool flipped = tgl.Draw(ImVec2(ytbl::Dp(toggleWidthDp), ytbl::Dp(toggleHeightDp)));
    toggleOn = tgl.selected;
    return flipped;
}

inline bool DrawSlider() {
    static ytbl::LiquidSlider sld;
    sld.accentColor = sliderAccent;
    if (sld.rangeStart != sliderMin || sld.rangeEnd != sliderMax)
        sld.SetValueRange(sliderMin, sliderMax);
    if (!sld.dragging && sld.value != sliderValue) {
        sld.value = sliderValue;
        sld.animation.AnimateToValue(sliderValue);
    }
    const bool changed = sld.Draw(ImVec2(ytbl::Dp(sliderWidthDp), ytbl::Dp(sliderHeightDp)));
    sliderValue = sld.value;
    return changed;
}

inline int DrawTabs() {
    static ytbl::LiquidBottomTabs tabs;
    const char* lbl[8] = {tabLabel0, tabLabel1, tabLabel2, tabLabel3,
                          tabLabel4, tabLabel5, tabLabel6, tabLabel7};
    for (int i = 0; i < 8; ++i) tabs.labels[i] = lbl[i];
    tabs.count          = tabCount < 1 ? 1 : (tabCount > 8 ? 8 : tabCount);
    tabs.accentColor    = tabsAccent;
    tabs.containerColor = tabsContainer;
    const int want = tabIndex < 0 ? 0
                   : (tabIndex > tabs.count - 1 ? tabs.count - 1 : tabIndex);
    if (tabs.selected != want) tabs.SetSelectedIndex(want, false);
    const bool changed = tabs.Draw(ImVec2(ytbl::Dp(tabsWidthDp), ytbl::Dp(tabsHeightDp)));
    if (changed) tabIndex = tabs.selected;
    return tabs.selected;
}

inline void DrawPanel(const ImVec2& minPx, const ImVec2& maxPx) {
    ytbl::GlassParams p;
    p.shape = ytbl::GlassShape(ytbl::Dp(panelCornerDp));
    p.effects.SetVibrancyEnabled(panelVibrancy);
    p.effects.SetBlurRadiusPx(ytbl::Dp(panelBlurDp));
    p.effects.SetLens(ytbl::Dp(panelRefractionHeightDp),
                      ytbl::Dp(panelRefractionAmountDp),
                      panelDepthEffect, panelChromatic);
    p.effects.SetChromaticAberration(panelChromatic ? panelDispersion : 0.0f);
    p.surfaceColor = Argb4(panelSurface);
    p.tintMode     = 2.0f;
    if (panelShadowAlpha != 0) {
        p.hasShadow = true;
        p.shadow = ytbl::Shadow(24.0f, 0.0f, 4.0f, panelShadowAlpha, 1.0f);
    }
    ytbl::DrawGlass(ImRect(minPx, maxPx), p);
}

inline ytbl::LiquidDialog& Dialog() {
    static ytbl::LiquidDialog dlg;
    return dlg;
}

inline void OpenDialog() { Dialog().Open(dialogTitle, dialogMessage); }

inline void CloseDialog(int result) { Dialog().Close(result); }

inline void DrawDialog() {
    ytbl::LiquidDialog& dlg = Dialog();
    dlg.title   = dialogTitle;
    dlg.message = dialogMessage;
    dlg.Draw(ImGui::GetWindowSize());
    dialogResult = dlg.result;
}

inline int GetDialogResult() { return Dialog().GetResult(); }

inline void DrawAll() {
    ImGui::TextUnformatted("Ytbl Liquid Glass / 液态玻璃组件");
    ImGui::Spacing();
    {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float  w   = ImGui::GetContentRegionAvail().x;
        const float  h   = ytbl::Dp(panelHeightDp);
        ImDrawList*  dl  = ImGui::GetWindowDrawList();
        const ImU32  stripes[4] = {
            IM_COL32(0xFF, 0x6B, 0x6B, 255), IM_COL32(0x4C, 0xD9, 0x8B, 255),
            IM_COL32(0x5A, 0x9C, 0xFF, 255), IM_COL32(0xFF, 0xD1, 0x66, 255)};
        const float sw = w > 0.0f ? w / 4.0f : 1.0f;
        for (int i = 0; i < 4; ++i) {
            dl->AddRectFilled(ImVec2(pos.x + sw * i, pos.y),
                              ImVec2(pos.x + sw * (i + 1), pos.y + h), stripes[i]);
        }
        dl->AddText(ImVec2(pos.x + ytbl::Dp(12.0f), pos.y + ytbl::Dp(10.0f)),
                    ytbl::Dp(14.0f), IM_COL32(255, 255, 255, 255),
                    "玻璃面板折射身后的条纹内容");
        DrawPanel(pos, ImVec2(pos.x + w, pos.y + h));
        ImGui::Dummy(ImVec2(w, h));
    }
    ImGui::Spacing();
    DrawButton();
    ImGui::Spacing();
    DrawToggle();
    ImGui::Spacing();
    DrawSlider();
    ImGui::Spacing();
    DrawTabs();
    ImGui::Spacing();
    if (ImGui::Button("打开弹窗(模态)")) OpenDialog();
    DrawDialog();
}

}
