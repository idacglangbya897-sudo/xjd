/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
#pragma once

#include <imgui.h>
#include <math.h>
#include <string.h>

namespace ytbl {

inline int Argb(float alpha, int rgb) {
    float a = alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
    int r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
    return ((int)(255.0f * a + 0.5f) << 24) | (r << 16) | (g << 8) | b;
}

inline ImU32 ToImU32(int argb) {
    const int a = (argb >> 24) & 0xFF;
    const int r = (argb >> 16) & 0xFF;
    const int g = (argb >> 8) & 0xFF;
    const int b = argb & 0xFF;
    return IM_COL32(r, g, b, a);
}

inline int ColorLerp(int start, int stop, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    int sa = (start >> 24) & 0xFF, sr = (start >> 16) & 0xFF, sg = (start >> 8) & 0xFF, sb = start & 0xFF;
    int ea = (stop >> 24) & 0xFF, er = (stop >> 16) & 0xFF, eg = (stop >> 8) & 0xFF, eb = stop & 0xFF;
    int a = (int)(sa + (ea - sa) * t + 0.5f);
    int r = (int)(sr + (er - sr) * t + 0.5f);
    int g = (int)(sg + (eg - sg) * t + 0.5f);
    int b = (int)(sb + (eb - sb) * t + 0.5f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

inline float Clamp01(float v) {
    if (!(v > 0.0f)) return 0.0f;
    return v > 1.0f ? 1.0f : v;
}
inline float Clampf(float v, float lo, float hi) {
    if (!(v >= lo)) return lo;
    return v > hi ? hi : v;
}
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }

inline float EaseOut(float x) {
    x = Clampf(x, 0.0f, 1.0f);
    if (x == 0.0f || x == 1.0f) return x;
    float t = x;
    for (int i = 0; i < 8; i++) {
        float omt = 1.0f - t;
        float curveX = 3.0f * 0.58f * omt * t * t + t * t * t;
        float derivative = 6.0f * 0.58f * omt * t - 3.0f * 0.58f * t * t + 3.0f * t * t;
        float error = curveX - x;
        if (fabsf(error) < 0.00001f || fabsf(derivative) < 0.00001f) break;
        t = Clampf(t - error / derivative, 0.0f, 1.0f);
    }
    float omt = 1.0f - t;
    return 3.0f * omt * t * t + t * t * t;
}

enum class GlassBlendMode { SRC_OVER, PLUS };

struct GlassShape {
    float topLeft, topRight, bottomRight, bottomLeft;

    GlassShape() : topLeft(3.4028235e38f), topRight(3.4028235e38f),
                   bottomRight(3.4028235e38f), bottomLeft(3.4028235e38f) {}
    explicit GlassShape(float radiusPx)
        : topLeft(radiusPx < 0 ? 0 : radiusPx), topRight(radiusPx < 0 ? 0 : radiusPx),
          bottomRight(radiusPx < 0 ? 0 : radiusPx), bottomLeft(radiusPx < 0 ? 0 : radiusPx) {}
    GlassShape(float tl, float tr, float br, float bl)
        : topLeft(tl < 0 ? 0 : tl), topRight(tr < 0 ? 0 : tr),
          bottomRight(br < 0 ? 0 : br), bottomLeft(bl < 0 ? 0 : bl) {}

    void GetCornerRadii(float width, float height, float out[4]) const {
        float m = width < height ? width : height;
        m = m * 0.5f; if (m < 0) m = 0;
        out[0] = topLeft < m ? topLeft : m;
        out[1] = topRight < m ? topRight : m;
        out[2] = bottomRight < m ? bottomRight : m;
        out[3] = bottomLeft < m ? bottomLeft : m;
    }
};

enum class HighlightStyleKind { Plain, Default, Ambient };

struct HighlightStyle {
    HighlightStyleKind kind = HighlightStyleKind::Default;
    int color = ((int)(255.0f * 0.5f + 0.5f) << 24) | 0x00FFFFFF;
    GlassBlendMode blendMode = GlassBlendMode::PLUS;
    float angle = 45.0f;
    float falloff = 1.0f;
    float intensity = 0.38f;

    static HighlightStyle Plain(int color = ((int)(255.0f * 0.38f + 0.5f) << 24) | 0x00FFFFFF,
                                GlassBlendMode mode = GlassBlendMode::PLUS) {
        HighlightStyle s; s.kind = HighlightStyleKind::Plain; s.color = color; s.blendMode = mode; return s;
    }
    static HighlightStyle Default(int color = ((int)(255.0f * 0.5f + 0.5f) << 24) | 0x00FFFFFF,
                                  GlassBlendMode mode = GlassBlendMode::PLUS,
                                  float angleDeg = 45.0f, float falloff = 1.0f) {
        HighlightStyle s; s.kind = HighlightStyleKind::Default;
        s.color = color; s.blendMode = mode; s.angle = angleDeg; s.falloff = falloff; return s;
    }
    static HighlightStyle Ambient(float intensity = 0.38f) {
        HighlightStyle s; s.kind = HighlightStyleKind::Ambient;
        s.color = ((int)(255.0f * Clamp01(intensity) + 0.5f) << 24) | 0x00FFFFFF;
        s.blendMode = GlassBlendMode::SRC_OVER; s.intensity = intensity; return s;
    }
};

struct Highlight {
    float widthDp = 0.5f;
    float blurRadiusDp = 0.25f;
    float alpha = 1.0f;
    HighlightStyle style;

    Highlight() = default;
    Highlight(float w, float blur, float a, const HighlightStyle& s)
        : widthDp(w), blurRadiusDp(blur), alpha(a), style(s) {}

    static Highlight Ambient() {
        return Highlight(0.5f, 0.25f, 1.0f, HighlightStyle::Ambient());
    }
    static Highlight Plain() {
        return Highlight(0.5f, 0.25f, 1.0f, HighlightStyle::Plain());
    }
};

struct Shadow {
    float radiusDp = 24.0f;
    float offsetXDp = 0.0f;
    float offsetYDp = 4.0f;
    int color = ((int)(255.0f * 0.1f + 0.5f) << 24);
    float alpha = 1.0f;
    GlassBlendMode blendMode = GlassBlendMode::SRC_OVER;

    Shadow() = default;
    Shadow(float radius, float ox, float oy, int col, float a = 1.0f,
           GlassBlendMode mode = GlassBlendMode::SRC_OVER)
        : radiusDp(radius), offsetXDp(ox), offsetYDp(oy), color(col), alpha(a), blendMode(mode) {}
    explicit Shadow(float radius, int col = ((int)(255.0f * 0.1f + 0.5f) << 24))
        : radiusDp(radius), offsetYDp(radius / 6.0f), color(col) {}
};

struct InnerShadow {
    float radiusDp = 24.0f;
    float offsetXDp = 0.0f;
    float offsetYDp = 24.0f;
    int color = ((int)(255.0f * 0.15f + 0.5f) << 24);
    float alpha = 1.0f;
    GlassBlendMode blendMode = GlassBlendMode::SRC_OVER;

    InnerShadow() = default;
    InnerShadow(float radius, float alpha_)
        : radiusDp(radius), offsetYDp(radius), alpha(alpha_) {}
    InnerShadow(float radius, float ox, float oy, int col, float a = 1.0f,
                GlassBlendMode mode = GlassBlendMode::SRC_OVER)
        : radiusDp(radius), offsetXDp(ox), offsetYDp(oy), color(col), alpha(a), blendMode(mode) {}

};

struct SimpleGlassEffects {
    bool vibrancyEnabled = false;
    bool colorControlsEnabled = false;
    float brightness = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    bool opacityEnabled = false;
    float opacity = 1.0f;
    float blurRadiusPx = 0.0f;
    float refractionHeightPx = 0.0f;
    float refractionAmountPx = 0.0f;
    bool depthEffect = false;
    bool chromaticAberration = false;
    float chromaticAberrationAmount = 1.0f;

    SimpleGlassEffects& SetVibrancyEnabled(bool e) { vibrancyEnabled = e; return *this; }
    SimpleGlassEffects& SetColorControls(float b, float c, float s) {
        brightness = b; contrast = c; saturation = s;
        colorControlsEnabled = !(b == 0.0f && c == 1.0f && s == 1.0f);
        return *this;
    }
    SimpleGlassEffects& SetOpacity(float a) { opacity = a; opacityEnabled = true; return *this; }
    SimpleGlassEffects& SetBlurRadiusPx(float px) { blurRadiusPx = px < 0 ? 0 : px; return *this; }
    SimpleGlassEffects& SetLens(float heightPx, float amountPx, bool depth = false, bool chroma = false) {
        refractionHeightPx = heightPx < 0 ? 0 : heightPx;
        refractionAmountPx = amountPx < 0 ? 0 : amountPx;
        depthEffect = depth;
        chromaticAberration = chroma;
        if (chroma && chromaticAberrationAmount <= 0.001f)
            chromaticAberrationAmount = 1.0f;
        return *this;
    }
    SimpleGlassEffects& SetChromaticAberration(float amount) {
        chromaticAberrationAmount = amount < 0.0f ? 0.0f : amount;
        chromaticAberration = chromaticAberrationAmount > 0.001f;
        return *this;
    }
};

}
