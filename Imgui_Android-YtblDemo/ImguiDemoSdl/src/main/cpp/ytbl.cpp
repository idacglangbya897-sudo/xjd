/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
 
#include "ytbl.h"
#include "ytbl_shaders.h"
#include "ytbl_backdrop.h"

#include <imgui_internal.h>
#include <GLES3/gl3.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace ytbl {

static GLuint s_GlassProgram = 0;
static GLuint s_QuadVao = 0;
static GLuint s_QuadVbo = 0;
static bool s_Initialized = false;
static float s_Density = 2.0f;

struct GlassUniforms {
    GLint uContent = -1, uSize = -1, uCaptureOriginOffset = -1, uCaptureSize = -1;
    GLint uCaptureTextureSize = -1;
    GLint uCornerRadii = -1, uRefractionHeight = -1, uRefractionAmount = -1, uDepthEffect = -1;
    GLint uDispersion = -1, uBlurRadius = -1, uOpacity = -1, uColorCtrl = -1, uVibrancy = -1;
    GLint uSurfaceColor = -1, uTintMode = -1;
    GLint uHighlightMode = -1, uHighlightColor = -1, uHighlightColorAlpha = -1;
    GLint uHighlightAngle = -1, uHighlightFalloff = -1, uHighlightAlpha = -1;
    GLint uHighlightWidth = -1, uHighlightBlur = -1, uUseTexture = -1, uOpaque = -1;
    GLint uFallback = -1;
    GLint uBackdropScaleX = -1, uBackdropScaleY = -1, uBackdropMix = -1;
    GLint uInnerShadowColor = -1, uInnerShadowOffset = -1, uInnerShadowRadius = -1, uInnerShadowAlpha = -1;
    GLint uIHMode = -1, uIHPosition = -1, uIHRadius = -1, uIHAlpha = -1;
};
static GlassUniforms U;

static GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint LinkProgram(const char* vs, const char* fs) {
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    if (v == 0 || f == 0) { glDeleteShader(v); glDeleteShader(f); return 0; }
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glBindAttribLocation(p, 0, "aPos");
    glBindAttribLocation(p, 1, "aCoord");
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok = GL_FALSE;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) { glDeleteProgram(p); return 0; }
    return p;
}

bool Init() {
    if (s_Initialized) return true;
    s_GlassProgram = LinkProgram(kGlassVertexShader, kGlassFragmentShader);
    if (s_GlassProgram == 0) return false;

    glUseProgram(s_GlassProgram);
    U.uContent = glGetUniformLocation(s_GlassProgram, "uContent");
    U.uSize = glGetUniformLocation(s_GlassProgram, "uSize");
    U.uCaptureOriginOffset = glGetUniformLocation(s_GlassProgram, "uCaptureOriginOffset");
    U.uCaptureSize = glGetUniformLocation(s_GlassProgram, "uCaptureSize");
    U.uCaptureTextureSize = glGetUniformLocation(s_GlassProgram, "uCaptureTextureSize");
    U.uCornerRadii = glGetUniformLocation(s_GlassProgram, "uCornerRadii");
    U.uRefractionHeight = glGetUniformLocation(s_GlassProgram, "uRefractionHeight");
    U.uRefractionAmount = glGetUniformLocation(s_GlassProgram, "uRefractionAmount");
    U.uDepthEffect = glGetUniformLocation(s_GlassProgram, "uDepthEffect");
    U.uDispersion = glGetUniformLocation(s_GlassProgram, "uDispersion");
    U.uBlurRadius = glGetUniformLocation(s_GlassProgram, "uBlurRadius");
    U.uOpacity = glGetUniformLocation(s_GlassProgram, "uOpacity");
    U.uColorCtrl = glGetUniformLocation(s_GlassProgram, "uColorCtrl");
    U.uVibrancy = glGetUniformLocation(s_GlassProgram, "uVibrancy");
    U.uSurfaceColor = glGetUniformLocation(s_GlassProgram, "uSurfaceColor");
    U.uTintMode = glGetUniformLocation(s_GlassProgram, "uTintMode");
    U.uHighlightMode = glGetUniformLocation(s_GlassProgram, "uHighlightMode");
    U.uHighlightColor = glGetUniformLocation(s_GlassProgram, "uHighlightColor");
    U.uHighlightColorAlpha = glGetUniformLocation(s_GlassProgram, "uHighlightColorAlpha");
    U.uHighlightAngle = glGetUniformLocation(s_GlassProgram, "uHighlightAngle");
    U.uHighlightFalloff = glGetUniformLocation(s_GlassProgram, "uHighlightFalloff");
    U.uHighlightAlpha = glGetUniformLocation(s_GlassProgram, "uHighlightAlpha");
    U.uHighlightWidth = glGetUniformLocation(s_GlassProgram, "uHighlightWidth");
    U.uHighlightBlur = glGetUniformLocation(s_GlassProgram, "uHighlightBlur");
    U.uUseTexture = glGetUniformLocation(s_GlassProgram, "uUseTexture");
    U.uOpaque = glGetUniformLocation(s_GlassProgram, "uOpaque");
    U.uFallback = glGetUniformLocation(s_GlassProgram, "uFallback");
    U.uBackdropScaleX = glGetUniformLocation(s_GlassProgram, "uBackdropScaleX");
    U.uBackdropScaleY = glGetUniformLocation(s_GlassProgram, "uBackdropScaleY");
    U.uBackdropMix = glGetUniformLocation(s_GlassProgram, "uBackdropMix");
    U.uInnerShadowColor = glGetUniformLocation(s_GlassProgram, "uInnerShadowColor");
    U.uInnerShadowOffset = glGetUniformLocation(s_GlassProgram, "uInnerShadowOffset");
    U.uInnerShadowRadius = glGetUniformLocation(s_GlassProgram, "uInnerShadowRadius");
    U.uInnerShadowAlpha = glGetUniformLocation(s_GlassProgram, "uInnerShadowAlpha");
    U.uIHMode = glGetUniformLocation(s_GlassProgram, "uIHMode");
    U.uIHPosition = glGetUniformLocation(s_GlassProgram, "uIHPosition");
    U.uIHRadius = glGetUniformLocation(s_GlassProgram, "uIHRadius");
    U.uIHAlpha = glGetUniformLocation(s_GlassProgram, "uIHAlpha");
    glUseProgram(0);

    glGenVertexArrays(1, &s_QuadVao);
    glGenBuffers(1, &s_QuadVbo);

    s_Initialized = true;
    return true;
}

void Shutdown() {
    ReleaseCapture();
    if (s_QuadVbo) { glDeleteBuffers(1, &s_QuadVbo); s_QuadVbo = 0; }
    if (s_QuadVao) { glDeleteVertexArrays(1, &s_QuadVao); s_QuadVao = 0; }
    if (s_GlassProgram) { glDeleteProgram(s_GlassProgram); s_GlassProgram = 0; }
    s_Initialized = false;
}

bool IsInitialized() { return s_Initialized; }
void SetDensity(float d) { s_Density = d; }
float Dp(float v) { return v * s_Density; }

static bool s_LightTheme = false;
static bool s_BackdropEnabled = true;
void SetLightTheme(bool light) { s_LightTheme = light; }
bool IsLightTheme() { return s_LightTheme; }
void SetBackdropEnabled(bool enabled) { s_BackdropEnabled = enabled; }

ImVec4 FrostedBackdrop() {
    return IsLightTheme() ? ImVec4(0.98f, 0.98f, 0.99f, 1.0f)
                          : ImVec4(0.12f, 0.13f, 0.15f, 1.0f);
}

struct GlassJob {
    float rect[4];
    float fbSize[2];
    float captureOrigin[2];
    float captureSize[2];
    float captureTextureSize[2];
    float cornerRadii[4];
    float refractionHeight, refractionAmount, depthEffect, dispersion;
    float blurRadius, opacity;
    float colorCtrl[3];
    float vibrancy;
    float surfaceColor[4], tintMode;
    float highlightMode, highlightColor[4], highlightColorAlpha;
    float highlightAngle, highlightFalloff, highlightAlpha;
    float highlightWidth;
    float highlightBlur;
    float innerShadowColor[4], innerShadowOffset[2], innerShadowRadius, innerShadowAlpha;
    float ihMode, ihPosition[2], ihRadius, ihAlpha;
    float useTexture;
    float opaque;
    float backdropScaleX, backdropScaleY, backdropMix;
    float fallback[4];
};
static GlassJob s_Jobs[64];
static int s_JobCount = 0;
void ResetGlassJobs() { s_JobCount = 0; }

static void DrawGlassQuad(const GlassJob& j) {
    float l = j.rect[0], t = j.rect[1], r = j.rect[2], b = j.rect[3];
    float fbW = j.fbSize[0], fbH = j.fbSize[1];
    float cl = 2.0f * l / fbW - 1.0f, cr = 2.0f * r / fbW - 1.0f;
    float ct = 1.0f - 2.0f * t / fbH, cb = 1.0f - 2.0f * b / fbH;
    float w = r - l, h = b - t;
    float triangleVerts[6][4] = {
        { cl, ct, 0.0f, 0.0f },
        { cr, ct,   w, 0.0f },
        { cr, cb,   w,   h },
        { cl, ct, 0.0f, 0.0f },
        { cr, cb,   w,   h },
        { cl, cb, 0.0f,   h },
    };

    glBindVertexArray(s_QuadVao);
    glBindBuffer(GL_ARRAY_BUFFER, s_QuadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerts), triangleVerts, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void GlassDrawCallback(const ImDrawList*, const ImDrawCmd* cmd) {
    int index = (int)(intptr_t)cmd->UserCallbackData;
    if (index < 0 || index >= s_JobCount) return;
    GlassJob& j = s_Jobs[index];

    int screenW = (int)j.fbSize[0], screenH = (int)j.fbSize[1];

    if (j.useTexture > 0.5f) {
        glDisable(GL_SCISSOR_TEST);

        int cx = (int)floorf(j.captureOrigin[0]);
        int cy = (int)floorf(j.captureOrigin[1]);
        int cw = (int)ceilf(j.captureSize[0]);
        int ch = (int)ceilf(j.captureSize[1]);
        if (cx < 0) { cw += cx; cx = 0; }
        if (cy < 0) { ch += cy; cy = 0; }
        if (cx > screenW) cw = 0;
        if (cy > screenH) ch = 0;
        if (cx + cw > screenW) cw = screenW - cx;
        if (cy + ch > screenH) ch = screenH - cy;

        if (cw > 0 && ch > 0 && screenW > 0 && screenH > 0) {
            GLuint captureTex = EnsureCaptureTexture(cw, ch);
            if (captureTex == 0 || cw > s_CaptureW || ch > s_CaptureH) {
                j.useTexture = 0.0f;
            } else {
                j.captureOrigin[0] = (float)cx;
                j.captureOrigin[1] = (float)cy;
                j.captureSize[0] = (float)cw;
                j.captureSize[1] = (float)ch;
                j.captureTextureSize[0] = (float)s_CaptureW;
                j.captureTextureSize[1] = (float)s_CaptureH;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, captureTex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                while (glGetError() != GL_NO_ERROR) {}
                bool ok = CaptureRegion(cx, cy, cw, ch, screenW, screenH);
                GLenum err = glGetError();
                if (!ok || err != GL_NO_ERROR) {
                    j.useTexture = 0.0f;
                } else {
                    j.captureSize[0] = (float)cw;
                    j.captureSize[1] = (float)ch;
                    j.captureTextureSize[0] = (float)s_CaptureW;
                    j.captureTextureSize[1] = (float)s_CaptureH;
                }
            }
        } else {
            j.useTexture = 0.0f;
        }
    }

    glUseProgram(s_GlassProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_CaptureTex);
    glUniform1i(U.uContent, 0);
    glUniform1f(U.uUseTexture, j.useTexture);
    glUniform1f(U.uOpaque, j.opaque);
    glUniform4f(U.uFallback, j.fallback[0], j.fallback[1], j.fallback[2], j.fallback[3]);
    glUniform1f(U.uBackdropScaleX, j.backdropScaleX);
    glUniform1f(U.uBackdropScaleY, j.backdropScaleY);
    glUniform1f(U.uBackdropMix, j.backdropMix);
    glUniform2f(U.uSize, j.rect[2] - j.rect[0], j.rect[3] - j.rect[1]);
    glUniform2f(U.uCaptureOriginOffset,
                j.rect[0] - j.captureOrigin[0], j.rect[1] - j.captureOrigin[1]);
    glUniform2f(U.uCaptureSize, j.captureSize[0], j.captureSize[1]);
    glUniform2f(U.uCaptureTextureSize, j.captureTextureSize[0], j.captureTextureSize[1]);
    glUniform4f(U.uCornerRadii, j.cornerRadii[0], j.cornerRadii[1], j.cornerRadii[2], j.cornerRadii[3]);
    glUniform1f(U.uRefractionHeight, j.refractionHeight);
    glUniform1f(U.uRefractionAmount, j.refractionAmount);
    glUniform1f(U.uDepthEffect, j.depthEffect);
    glUniform1f(U.uDispersion, j.dispersion);
    glUniform1f(U.uBlurRadius, j.blurRadius);
    glUniform1f(U.uOpacity, j.opacity);
    glUniform3f(U.uColorCtrl, j.colorCtrl[0], j.colorCtrl[1], j.colorCtrl[2]);
    glUniform1f(U.uVibrancy, j.vibrancy);
    glUniform4f(U.uSurfaceColor, j.surfaceColor[0], j.surfaceColor[1], j.surfaceColor[2], j.surfaceColor[3]);
    glUniform1f(U.uTintMode, j.tintMode);
    glUniform1f(U.uHighlightMode, j.highlightMode);
    glUniform4f(U.uHighlightColor, j.highlightColor[0], j.highlightColor[1], j.highlightColor[2], j.highlightColor[3]);
    glUniform1f(U.uHighlightColorAlpha, j.highlightColorAlpha);
    glUniform1f(U.uHighlightAngle, j.highlightAngle);
    glUniform1f(U.uHighlightFalloff, j.highlightFalloff);
    glUniform1f(U.uHighlightAlpha, j.highlightAlpha);
    glUniform1f(U.uHighlightWidth, j.highlightWidth);
    glUniform1f(U.uHighlightBlur, j.highlightBlur);
    glUniform4f(U.uInnerShadowColor, j.innerShadowColor[0], j.innerShadowColor[1], j.innerShadowColor[2], j.innerShadowColor[3]);
    glUniform2f(U.uInnerShadowOffset, j.innerShadowOffset[0], j.innerShadowOffset[1]);
    glUniform1f(U.uInnerShadowRadius, j.innerShadowRadius);
    glUniform1f(U.uInnerShadowAlpha, j.innerShadowAlpha);
    glUniform1f(U.uIHMode, j.ihMode);
    glUniform2f(U.uIHPosition, j.ihPosition[0], j.ihPosition[1]);
    glUniform1f(U.uIHRadius, j.ihRadius);
    glUniform1f(U.uIHAlpha, j.ihAlpha);

    const ImVec2 displayScale = ImGui::GetIO().DisplayFramebufferScale;
    const float scaleX = displayScale.x > 0.0f ? displayScale.x : 1.0f;
    const float scaleY = displayScale.y > 0.0f ? displayScale.y : 1.0f;
    ImVec4 clip = cmd->ClipRect;
    int scx = (int)floorf(clip.x * scaleX);
    int scy = (int)floorf(clip.y * scaleY);
    int scw = (int)ceilf((clip.z - clip.x) * scaleX);
    int sch = (int)ceilf((clip.w - clip.y) * scaleY);
    if (scx < 0) { scw += scx; scx = 0; }
    if (scy < 0) { sch += scy; scy = 0; }
    if (scx + scw > screenW) scw = screenW - scx;
    if (scy + sch > screenH) sch = screenH - scy;
    if (scw < 0) scw = 0;
    if (sch < 0) sch = 0;
    glScissor(scx, screenH - scy - sch, scw, sch);
    glEnable(GL_SCISSOR_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    DrawGlassQuad(j);

}

void DrawGlass(const ImRect& rect, const GlassParams& params) {
    if (!s_Initialized || s_JobCount >= 64) return;
    ImGuiIO& io = ImGui::GetIO();
    const float framebufferScaleX = io.DisplayFramebufferScale.x > 0.0f
        ? io.DisplayFramebufferScale.x : 1.0f;
    const float framebufferScaleY = io.DisplayFramebufferScale.y > 0.0f
        ? io.DisplayFramebufferScale.y : 1.0f;
    const float fbW = io.DisplaySize.x * framebufferScaleX;
    const float fbH = io.DisplaySize.y * framebufferScaleY;
    if (fbW <= 0.0f || fbH <= 0.0f) return;

    float l = rect.Min.x * framebufferScaleX;
    float t = rect.Min.y * framebufferScaleY;
    float r = rect.Max.x * framebufferScaleX;
    float b = rect.Max.y * framebufferScaleY;
    float w = r - l, h = b - t;
    if (w <= 0 || h <= 0) return;

    const SimpleGlassEffects& fx = params.effects;
    const float uniformScale = 0.5f * (framebufferScaleX + framebufferScaleY);
    float refH = fx.refractionHeightPx * uniformScale;
    float refA = fx.refractionAmountPx * uniformScale;
    float blur = fx.blurRadiusPx * uniformScale;

    float pad = 2.0f;
    if (blur > 0) pad += blur;

    GlassJob& j = s_Jobs[s_JobCount];
    memset(&j, 0, sizeof(GlassJob));
    j.rect[0] = l; j.rect[1] = t; j.rect[2] = r; j.rect[3] = b;
    j.fbSize[0] = fbW; j.fbSize[1] = fbH;
    j.captureOrigin[0] = l - pad; j.captureOrigin[1] = t - pad;
    j.captureSize[0] = w + pad * 2; j.captureSize[1] = h + pad * 2;
    j.captureTextureSize[0] = j.captureSize[0];
    j.captureTextureSize[1] = j.captureSize[1];
    float displayCornerRadii[4];
    params.shape.GetCornerRadii(rect.GetWidth(), rect.GetHeight(), displayCornerRadii);
    for (int i = 0; i < 4; ++i) {
        j.cornerRadii[i] = displayCornerRadii[i] * uniformScale;
    }
    j.refractionHeight = refH;
    j.refractionAmount = refA;
    j.depthEffect = fx.depthEffect ? 1.0f : 0.0f;
    j.dispersion = fx.chromaticAberration
        ? Clampf(fx.chromaticAberrationAmount, 0.0f, 4.0f)
        : 0.0f;
    j.blurRadius = blur;
    j.opacity = fx.opacityEnabled ? fx.opacity : 1.0f;
    j.opacity *= params.alpha;
    j.colorCtrl[0] = fx.brightness;
    j.colorCtrl[1] = fx.contrast;
    j.colorCtrl[2] = fx.saturation;
    j.vibrancy = fx.vibrancyEnabled ? 1.0f : 0.0f;
    j.surfaceColor[0] = params.surfaceColor.x;
    j.surfaceColor[1] = params.surfaceColor.y;
    j.surfaceColor[2] = params.surfaceColor.z;
    j.surfaceColor[3] = params.surfaceColor.w;
    j.tintMode = params.tintMode;
    j.useTexture = (params.useBackdrop && s_BackdropEnabled) ? 1.0f : 0.0f;
    j.opaque = params.opaqueBackdrop ? 1.0f : 0.0f;
    j.backdropScaleX = params.backdropScaleX;
    j.backdropScaleY = params.backdropScaleY;
    j.backdropMix = params.backdropMix;
    const ImVec4 fallback = params.backdropFallback;
    j.fallback[0] = fallback.x;
    j.fallback[1] = fallback.y;
    j.fallback[2] = fallback.z;
    j.fallback[3] = fallback.w;

    if (params.hasHighlight && params.highlight.widthDp > 0.0f) {
        const Highlight& hl = params.highlight;
        const HighlightStyle& st = hl.style;
        j.highlightMode = st.kind == HighlightStyleKind::Ambient ? 2.0f
                        : (st.kind == HighlightStyleKind::Plain ? 3.0f : 1.0f);
        j.highlightColor[0] = ((st.color >> 16) & 0xFF) / 255.0f;
        j.highlightColor[1] = ((st.color >> 8) & 0xFF) / 255.0f;
        j.highlightColor[2] = (st.color & 0xFF) / 255.0f;
        j.highlightColor[3] = 1.0f;
        j.highlightColorAlpha = (float)(((st.color >> 24) & 0xFF) / 255.0f);
        j.highlightAngle = (st.kind == HighlightStyleKind::Default ? st.angle : 45.0f)
                           * (float)(3.14159265358979323846 / 180.0);
        j.highlightFalloff = st.kind == HighlightStyleKind::Default ? st.falloff : 1.0f;
        j.highlightAlpha = hl.alpha * params.alpha;
        if (st.kind == HighlightStyleKind::Ambient) j.highlightAlpha *= st.intensity;
        float widthPx = hl.widthDp * s_Density * uniformScale;
        float maxW = (w < h ? w : h) * 0.5f;
        if (widthPx > maxW) widthPx = maxW;
        j.highlightWidth = ceilf(widthPx);
        j.highlightBlur = hl.blurRadiusDp * s_Density * uniformScale;
    } else {
        j.highlightMode = 0.0f;
        j.highlightWidth = 0.0f;
        j.highlightBlur = 0.0f;
    }

    if (params.hasInnerShadow) {
        const InnerShadow& is = params.innerShadow;
        j.innerShadowColor[0] = ((is.color >> 16) & 0xFF) / 255.0f;
        j.innerShadowColor[1] = ((is.color >> 8) & 0xFF) / 255.0f;
        j.innerShadowColor[2] = (is.color & 0xFF) / 255.0f;
        j.innerShadowColor[3] = 1.0f;
        j.innerShadowOffset[0] = is.offsetXDp * s_Density * framebufferScaleX;
        j.innerShadowOffset[1] = is.offsetYDp * s_Density * framebufferScaleY;
        j.innerShadowRadius = is.radiusDp * s_Density * uniformScale;
        j.innerShadowAlpha = is.alpha * params.alpha
            * (((is.color >> 24) & 0xFF) / 255.0f);
    } else {
        j.innerShadowAlpha = 0.0f;
    }

    if (params.ihProgress > 0.001f) {
        j.ihMode = 1.0f;
        j.ihPosition[0] = params.ihX * framebufferScaleX;
        j.ihPosition[1] = params.ihY * framebufferScaleY;
        j.ihRadius = (w < h ? w : h) * InteractiveHighlight::kRadiusMultiplier;
        j.ihAlpha = params.ihProgress;
    } else {
        j.ihMode = 0.0f;
    }

    if (params.hasShadow && params.shadow.alpha > 0.0f) {
        const Shadow& s = params.shadow;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float radius = s.radiusDp * s_Density;
        float ox = s.offsetXDp * s_Density;
        float oy = s.offsetYDp * s_Density;
        const float drawL = rect.Min.x;
        const float drawT = rect.Min.y;
        const float drawR = rect.Max.x;
        const float drawB = rect.Max.y;
        const float drawW = drawR - drawL;
        const float drawH = drawB - drawT;
        float drawCornerRadii[4];
        params.shape.GetCornerRadii(drawW, drawH, drawCornerRadii);
        int layers = 5;
        for (int layer = layers; layer >= 1; layer--) {
            float expand = radius * (float)layer / (float)layers;
            float weight = (float)(layers - layer + 1) / (float)(layers + 1);
            weight = weight * weight;
            int a = (int)(255.0f * (((s.color >> 24) & 0xFF) / 255.0f) * s.alpha * weight);
            if (a <= 0) continue;
            ImU32 col = ToImU32((a << 24) | (s.color & 0x00FFFFFF));
            float maxR = (drawW < drawH ? drawW : drawH) * 0.5f;
            float rr = drawCornerRadii[0] + expand;
            if (rr > maxR + expand) rr = maxR + expand;
            dl->AddRectFilled(ImVec2(drawL - expand + ox, drawT - expand + oy),
                              ImVec2(drawR + expand + ox, drawB + expand + oy),
                              col, rr, 0);
        }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (params.onDrawBehind) {
        params.onDrawBehind(params.behindUser, rect, params.shape);
    }

    int jobIndex = s_JobCount;
    s_JobCount++;
    dl->AddCallback(&GlassDrawCallback, (void*)(intptr_t)jobIndex);
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    if (params.onDrawSurface) {
        params.onDrawSurface(params.surfaceUser, rect, params.shape);
    }
    if (params.onDrawContentBackground) {
        params.onDrawContentBackground(params.contentBgUser, rect, params.shape);
    }
    if (params.onDrawDynamicOverlay) {
        params.onDrawDynamicOverlay(params.dynamicUser, rect, params.shape);
    }
    if (params.onDrawFront) {
        params.onDrawFront(params.frontUser, rect, params.shape);
    }
}

void InteractiveHighlight::Press(float px, float py) {
    if (!enabled_) return;
    startX_ = px;
    startY_ = py;
    pressProgress_.AnimateTo(1.0f);
    posX_.SnapTo(startX_);
    posY_.SnapTo(startY_);
    animating_ = true;
}

void InteractiveHighlight::Move(float px, float py) {
    if (!enabled_) return;
    posX_.SnapTo(px);
    posY_.SnapTo(py);
    if (pressProgress_.value != pressProgress_.target) animating_ = true;
}

void InteractiveHighlight::Release() {
    if (!enabled_) return;
    pressProgress_.AnimateTo(0.0f);
    posX_.AnimateTo(startX_);
    posY_.AnimateTo(startY_);
    animating_ = true;
}

void InteractiveHighlight::Cancel() {
    pressProgress_.SnapTo(0.0f);
    posX_.SnapTo(startX_);
    posY_.SnapTo(startY_);
    animating_ = false;
}

void InteractiveHighlight::Step(float dt) {
    bool active = false;
    active |= pressProgress_.Step(dt);
    active |= posX_.Step(dt);
    active |= posY_.Step(dt);
    animating_ = active;
}

int LiquidDialog::Draw(const ImVec2& areaSizePx) {
    if (!visible) return 0;

    const float dt = ImGui::GetIO().DeltaTime > 0.0f
        ? ImGui::GetIO().DeltaTime : 1.0f / 60.0f;
    Step(dt);
    if (!visible) return result;

    const float p = Clamp01(progress.value);
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const float fullW = areaSizePx.x > 1.0f ? areaSizePx.x : windowSize.x;
    const float fullH = areaSizePx.y > 1.0f ? areaSizePx.y : windowSize.y;
    const ImVec2 fullMin(windowPos.x, windowPos.y);
    const ImVec2 fullMax(windowPos.x + fullW, windowPos.y + fullH);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const bool modalClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

    const float baseW = fminf(fullW - Dp(80.0f), Dp(420.0f));
    const float dialogW = fmaxf(Dp(260.0f), baseW);
    const float dialogH = Dp(252.0f);
    const float scale = Lerpf(0.94f, 1.0f, p);
    const ImVec2 center(fullMin.x + fullW * 0.5f, fullMin.y + fullH * 0.5f);
    const ImVec2 half(dialogW * scale * 0.5f, dialogH * scale * 0.5f);
    const ImRect dialogRect(ImVec2(center.x - half.x, center.y - half.y),
                            ImVec2(center.x + half.x, center.y + half.y));

    const int dimRgb = IsLightTheme() ? 0x29293A : 0x121212;
    const float dimAlpha = (IsLightTheme() ? 0.23f : 0.56f) * p;
    dl->AddRectFilled(fullMin, fullMax,
                      ToImU32(Argb(dimAlpha, dimRgb)));

    if (modalClicked) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        if (!dialogRect.Contains(mouse)) Close(1);
    }

    GlassParams params;
    params.shape = GlassShape(Dp(48.0f));
    params.effects.SetVibrancyEnabled(true);
    params.effects.SetColorControls(IsLightTheme() ? 0.2f : 0.0f,
                                    1.0f, 1.5f);
    params.effects.SetBlurRadiusPx(Dp(IsLightTheme() ? 16.0f : 8.0f));
    params.effects.SetLens(Dp(24.0f), Dp(48.0f), true, false);
    params.hasHighlight = true;
    params.highlight = Highlight::Plain();
    const int surfaceRgb = IsLightTheme() ? 0xFAFAFA : 0x121212;
    params.surfaceColor = ImVec4(((surfaceRgb >> 16) & 0xFF) / 255.0f,
                                 ((surfaceRgb >> 8) & 0xFF) / 255.0f,
                                 (surfaceRgb & 0xFF) / 255.0f,
                                 (IsLightTheme() ? 0.60f : 0.40f) * p);
    params.tintMode = 2.0f;
    params.alpha = p;
    DrawGlass(dialogRect, params);

    const float titleSize = Dp(24.0f);
    const float bodySize = Dp(15.0f);
    const float buttonSize = Dp(16.0f);
    ImFont* font = ImGui::GetFont();
    const int textRgb = IsLightTheme() ? 0x000000 : 0xFFFFFF;
    const ImU32 fg = ToImU32(Argb(p, textRgb));
    const ImU32 body = ToImU32(Argb(0.68f * p, textRgb));

    const float left = dialogRect.Min.x;
    const float top = dialogRect.Min.y;
    const float contentW = dialogRect.GetWidth();
    const float padX = Dp(28.0f) * scale;
    const float titleY = top + Dp(24.0f) * scale;
    const float bodyY = top + Dp(68.0f) * scale;
    dl->AddText(font, titleSize * scale, ImVec2(left + padX, titleY), fg,
                title ? title : "Dialog Title");
    const char* bodyText = message ? message : "This is a liquid glass dialog.";
    dl->AddText(font, bodySize * scale,
                ImVec2(left + Dp(24.0f) * scale, bodyY), body,
                bodyText, nullptr, contentW - Dp(48.0f) * scale);

    const float buttonGap = Dp(16.0f) * scale;
    const float buttonH = Dp(48.0f) * scale;
    const float buttonPad = Dp(24.0f) * scale;
    const float buttonW = (contentW - buttonPad * 2.0f - buttonGap) * 0.5f;
    const float buttonY = dialogRect.Max.y - buttonPad - buttonH;
    const ImRect cancelRect(ImVec2(left + buttonPad, buttonY),
                            ImVec2(left + buttonPad + buttonW, buttonY + buttonH));
    const ImRect okayRect(ImVec2(cancelRect.Max.x + buttonGap, buttonY),
                          ImVec2(cancelRect.Max.x + buttonGap + buttonW, buttonY + buttonH));

    ImGui::PushID(this);
    ImGui::SetCursorScreenPos(cancelRect.Min);
    ImGui::InvisibleButton("##dialog_cancel", cancelRect.GetSize(),
                           ImGuiButtonFlags_MouseButtonLeft);
    const bool cancelClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    ImGui::SetCursorScreenPos(okayRect.Min);
    ImGui::InvisibleButton("##dialog_okay", okayRect.GetSize(),
                           ImGuiButtonFlags_MouseButtonLeft);
    const bool okayClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    ImGui::PopID();
    if (cancelClicked) Close(1);
    if (okayClicked) Close(2);

    dl->AddRectFilled(cancelRect.Min, cancelRect.Max,
                      ToImU32(Argb(0.12f * p, surfaceRgb)), buttonH * 0.5f);
    dl->AddRectFilled(okayRect.Min, okayRect.Max,
                      ToImU32(Argb(p, IsLightTheme() ? 0x0088FF : 0x0091FF)), buttonH * 0.5f);
    const char* cancelText = "Cancel";
    const char* okayText = "Okay";
    ImVec2 cancelTextSize = font->CalcTextSizeA(buttonSize * scale, 1e6f, 0.0f, cancelText);
    ImVec2 okayTextSize = font->CalcTextSizeA(buttonSize * scale, 1e6f, 0.0f, okayText);
    dl->AddText(font, buttonSize * scale,
                ImVec2(cancelRect.GetCenter().x - cancelTextSize.x * 0.5f,
                       cancelRect.GetCenter().y - cancelTextSize.y * 0.5f), fg, cancelText);
    dl->AddText(font, buttonSize * scale,
                ImVec2(okayRect.GetCenter().x - okayTextSize.x * 0.5f,
                       okayRect.GetCenter().y - okayTextSize.y * 0.5f),
                IM_COL32(255, 255, 255, (int)(255.0f * p)), okayText);

    ImGui::SetCursorScreenPos(ImVec2(fullMin.x, fullMax.y));
    return 0;
}

bool LiquidButton::Draw(const ImVec2& sizePx) {
    bool clicked = false;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImRect rect(p, ImVec2(p.x + sizePx.x, p.y + sizePx.y));
    ImGui::PushID(this);
    ImGui::InvisibleButton("##ytbl_btn", sizePx, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::PopID();
    bool held = ImGui::IsItemActive();
    if (ImGui::IsItemClicked()) { clicked = true; clickCount++; }

    if (interactive && held) {
        if (!wasPressed) {
            ImVec2 mp = ImGui::GetIO().MousePos;
            highlight.Press(mp.x - p.x, mp.y - p.y);
        } else {
            ImVec2 mp = ImGui::GetIO().MousePos;
            highlight.Move(mp.x - p.x, mp.y - p.y);
        }
    } else if (wasPressed && !held) {
        highlight.Release();
    }
    wasPressed = held;

    contentColor = IsLightTheme() ? 0xFF000000 : 0xFFFFFFFF;

    GlassParams params;
    params.shape = GlassShape();
    params.effects.SetVibrancyEnabled(true);
    params.effects.SetBlurRadiusPx(Dp(2.0f));
    params.effects.SetLens(Dp(12.0f), Dp(24.0f), false, true);
    params.effects.SetChromaticAberration(0.28f);
    params.hasHighlight = true;
    params.highlight = Highlight();
    params.alpha = 1.0f;
    params.backdropFallback = FrostedBackdrop();

    if (hasTint) {
        params.surfaceColor = tint;
        params.tintMode = 1.0f;
    } else if (hasSurface) {
        params.surfaceColor = surfaceColor;
        params.tintMode = 2.0f;
    }

    float pw = sizePx.x, ph = sizePx.y;
    float prog = highlight.GetPressProgress();
    float scale = Lerpf(1.0f, 1.0f + Dp(4.0f) / (ph > 1 ? ph : 1), prog);
    float maxOffset = pw < ph ? pw : ph;
    float ox = highlight.GetOffsetX(), oy = highlight.GetOffsetY();
    float tx = maxOffset * tanhf(0.05f * ox / (maxOffset > 0 ? maxOffset : 1));
    float ty = maxOffset * tanhf(0.05f * oy / (maxOffset > 0 ? maxOffset : 1));
    float maxDragScale = Dp(4.0f) / (ph > 1 ? ph : 1);
    float angle = atan2f(oy, ox);
    float sx = scale + maxDragScale * fabsf(cosf(angle) * ox / (pw > ph ? pw : ph)) * fminf(pw / ph, 1.0f);
    float sy = scale + maxDragScale * fabsf(sinf(angle) * oy / (pw > ph ? pw : ph)) * fminf(ph / pw, 1.0f);

    ImVec2 center((rect.Min.x + rect.Max.x) * 0.5f, (rect.Min.y + rect.Max.y) * 0.5f);
    float halfW = pw * 0.5f * sx, halfH = ph * 0.5f * sy;
    ImRect drawRect(ImVec2(center.x - halfW + tx, center.y - halfH + ty),
                    ImVec2(center.x + halfW + tx, center.y + halfH + ty));

    params.ihProgress = interactive && highlight.IsHighlightVisible()
                        ? Clamp01(highlight.GetPressProgress()) * highlight.intensityScale_ : 0.0f;
    params.ihX = Clampf(highlight.GetPointerX(), 0, pw);
    params.ihY = Clampf(highlight.GetPointerY(), 0, ph);
    DrawGlass(drawRect, params);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* font = ImGui::GetFont();
    float fontSize = textSizeSp * s_Density;
    if (fontSize < 8.0f) fontSize = 8.0f;
    ImVec2 ts = font->CalcTextSizeA(fontSize, 1e6f, 0.0f, label ? label : "");
    float cx0 = (drawRect.Min.x + drawRect.Max.x) * 0.5f - ts.x * 0.5f;
    float cy = (drawRect.Min.y + drawRect.Max.y) * 0.5f;
    ImU32 textCol = IM_COL32((contentColor >> 16) & 0xFF, (contentColor >> 8) & 0xFF,
                             contentColor & 0xFF, 255);
    if (label && label[0]) {
        dl->AddText(font, fontSize,
                    ImVec2(cx0, cy - ts.y * 0.5f), textCol, label);
    }

    return clicked;
}

bool LiquidToggle::Draw(const ImVec2& sizePx) {
    bool toggled = false;
    ImVec2 p = ImGui::GetCursorScreenPos();
    float trackW = Dp(64.0f), trackH = Dp(28.0f);
    float thumbW = Dp(40.0f), thumbH = Dp(24.0f);
    float trackX = p.x, trackY = p.y + (sizePx.y - trackH) * 0.5f;
    float thumbY = p.y + (sizePx.y - thumbH) * 0.5f;

    ImGui::PushID(this);
    ImGui::InvisibleButton("##ytbl_toggle", sizePx, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::PopID();
    bool held = ImGui::IsItemActive();

    if (ImGui::IsItemActivated()) {
        float mx = ImGui::GetIO().MousePos.x, my = ImGui::GetIO().MousePos.y;
        float valueNow = animation.GetValue();
        float txNow = Lerpf(Dp(2.0f), Dp(22.0f), valueNow);
        float tl = trackX + txNow, tr = tl + thumbW;
        float tt = thumbY, tb = thumbY + thumbH;
        if (mx >= tl && mx <= tr && my >= tt && my <= tb) {
            lastRawX = mx;
            touchActive = true;
            didDrag = false;
            heldSince = (float)ImGui::GetTime();
            animation.Press();
        }
    }
    if (held && touchActive) {
        float now = ImGui::GetIO().MousePos.x;
        float dx = now - lastRawX;
        lastRawX = now;
        if (fabsf(dx) > 2.0f) {
            didDrag = true;
            heldSince = (float)ImGui::GetTime();
        }
        if ((double)heldSince > 0.0 && ImGui::GetTime() - (double)heldSince > 2.0) {
            ImGui::GetIO().MouseDown[0] = false;
            ImGui::ClearActiveID();
            bool newSelected = didDrag ? animation.GetTargetValue() >= 0.5f : !selected;
            if (newSelected != selected) { selected = newSelected; toggled = true; }
            animation.UpdateValue(newSelected ? 1.0f : 0.0f);
            didDrag = false;
            animation.Release();
            touchActive = false;
        }
        if (touchActive) {
            float delta = dx / Dp(20.0f);
            animation.UpdateValue(Clampf(animation.GetTargetValue() + delta, 0.0f, 1.0f));
        }
    }
    if (ImGui::IsItemDeactivated() && touchActive) {
        touchActive = false;
        bool newSelected;
        if (didDrag) newSelected = animation.GetTargetValue() >= 0.5f;
        else newSelected = !selected;
        if (newSelected != selected) { selected = newSelected; toggled = true; }
        animation.UpdateValue(newSelected ? 1.0f : 0.0f);
        didDrag = false;
        animation.Release();
    }

    float value = animation.GetValue();
    float pressP = animation.GetPressProgress();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const int effectiveAccent =
        IsLightTheme() && accentColor == 0xFF30D158 ? 0xFF34C759 : accentColor;
    int track = IsLightTheme() ? Argb(1.0f, 0x787878)
                               : Argb(1.0f, trackColor & 0x00FFFFFF);
    ImU32 trackCol = ToImU32(ColorLerp(track, effectiveAccent, Clamp01(value)));
    dl->AddRectFilled(ImVec2(trackX, trackY), ImVec2(trackX + trackW, trackY + trackH),
                      trackCol, trackH * 0.5f, 0);

    float padding = Dp(2.0f);
    float tx = Lerpf(padding, padding + Dp(20.0f), value);
    float velocity = animation.GetVelocity() / 50.0f;
    float sx = animation.GetScaleX() / (1.0f - Clampf(velocity * 0.75f, -0.2f, 0.2f));
    float sy = animation.GetScaleY() * (1.0f - Clampf(velocity * 0.25f, -0.2f, 0.2f));
    float tw = thumbW * sx, th = thumbH * sy;
    ImVec2 thumbPos(trackX + tx - (tw - thumbW) * 0.5f,
                    thumbY + (thumbH - th) * 0.5f);

    GlassParams params;
    params.shape = GlassShape();
    params.effects.SetVibrancyEnabled(true);
    params.effects.SetBlurRadiusPx(Dp(8.0f) * (1.0f - pressP));
    params.effects.SetLens(Dp(5.0f) * pressP,
                           Dp(10.0f) * pressP, false, true);
    params.effects.SetChromaticAberration(1.0f);
    params.backdropScaleX = Lerpf(2.0f / 3.0f, 0.75f, pressP);
    params.backdropScaleY = Lerpf(0.0f, 0.75f, pressP);
    params.backdropMix = 1.0f;
    const float sa = Clampf(1.0f - pressP, 0.0f, 1.0f);
    params.surfaceColor = ImVec4(1, 1, 1, sa);
    params.tintMode = 2.0f;
    const ImVec4& pageBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    params.backdropFallback = ImVec4(pageBg.x, pageBg.y, pageBg.z, 1.0f);
    params.opaqueBackdrop = false;
    float hp = Clampf(pressP * pressHighlightIntensity, 0.0f, 1.0f);
    params.hasHighlight = hp > 0.001f;
    Highlight ambient = Highlight::Ambient();
    params.highlight = Highlight(ambient.widthDp / 1.5f,
                                 ambient.blurRadiusDp / 1.5f,
                                 hp, ambient.style);
    params.hasShadow = true;
    params.shadow = Shadow(4.0f, ((int)(255.0f * 0.05f + 0.5f) << 24));
    params.hasInnerShadow = pressP > 0.001f;
    params.innerShadow = InnerShadow(4.0f * pressP, pressP);

    DrawGlass(ImRect(thumbPos, ImVec2(thumbPos.x + tw, thumbPos.y + th)), params);
    return toggled;
}

bool LiquidSlider::Draw(const ImVec2& sizePx) {
    bool changed = false;
    ImVec2 p = ImGui::GetCursorScreenPos();
    float trackH = Dp(6.0f);
    float thumbW = Dp(40.0f), thumbH = Dp(24.0f);
    float trackY = p.y + (sizePx.y - trackH) * 0.5f;
    float thumbY = p.y + (sizePx.y - thumbH) * 0.5f;

    ImGui::PushID(this);
    ImGui::InvisibleButton("##ytbl_slider", sizePx, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::PopID();
    bool held = ImGui::IsItemActive();

    if (ImGui::IsItemActivated()) {
        float mx = ImGui::GetIO().MousePos.x, my = ImGui::GetIO().MousePos.y;
        float prog0 = Clamp01(animation.GetProgress());
        float raw0 = -thumbW * 0.5f + sizePx.x * prog0;
        float tx0 = Clampf(raw0, -thumbW * 0.25f, sizePx.x - thumbW * 0.75f);
        float tl = p.x + tx0, tr = p.x + tx0 + thumbW;
        float tt = thumbY, tb = thumbY + thumbH;
        if (mx < tl || mx > tr || my < tt || my > tb) {
            trackTapPending = true;
            animation.Press();
        } else {
            dragging = true;
            didDrag = false;
            lastRawX = mx;
            animation.Press();
        }
        heldSince = (float)ImGui::GetTime();
    }
    if (held && (dragging || trackTapPending)) {
        if ((double)heldSince > 0.0 && ImGui::GetTime() - (double)heldSince > 2.0) {
            ImGui::GetIO().MouseDown[0] = false;
            ImGui::ClearActiveID();
            if (dragging) {
                dragging = false;
                animation.Release();
            }
            if (trackTapPending) {
                trackTapPending = false;
                float mx = ImGui::GetIO().MousePos.x;
                float prog = Clampf((mx - p.x) / (sizePx.x > 1 ? sizePx.x : 1), 0.0f, 1.0f);
                value = Clampf(rangeStart + (rangeEnd - rangeStart) * prog,
                               rangeStart, rangeEnd);
                animation.AnimateToValue(value);
                changed = true;
            }
        }
    }
    if (held && dragging) {
        float mx = ImGui::GetIO().MousePos.x;
        float dx = mx - lastRawX;
        lastRawX = mx;
        if (dx != 0.0f) {
            didDrag = true;
            changed = true;
        }
        if (fabsf(dx) > 1.5f) {
            heldSince = (float)ImGui::GetTime();
        }
        float target = animation.GetTargetValue()
                       + (rangeEnd - rangeStart) * (dx / (sizePx.x > 1 ? sizePx.x : 1));
        value = Clampf(target, rangeStart, rangeEnd);
        animation.UpdateValue(value);
    }
    if (ImGui::IsItemDeactivated()) {
        if (dragging) {
            dragging = false;
            animation.Release();
        }
        if (trackTapPending) {
            trackTapPending = false;
            float mx = ImGui::GetIO().MousePos.x;
            float prog = Clampf((mx - p.x) / (sizePx.x > 1 ? sizePx.x : 1), 0.0f, 1.0f);
            value = Clampf(rangeStart + (rangeEnd - rangeStart) * prog, rangeStart, rangeEnd);
            animation.AnimateToValue(value);
            changed = true;
        }
    }

    float progress = Clamp01(animation.GetProgress());
    float pressP = animation.GetPressProgress();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const int effectiveAccent =
        IsLightTheme() && accentColor == 0xFF0091FF ? 0xFF0088FF : accentColor;
    int track = IsLightTheme() ? Argb(0.20f, 0x787878) : trackColor;

    float raw = -thumbW * 0.5f + sizePx.x * progress;
    float tx = Clampf(raw, -thumbW * 0.25f, sizePx.x - thumbW * 0.75f);
    float velocity = animation.GetVelocity() / 10.0f;
    float sx = animation.GetScaleX() / (1.0f - Clampf(velocity * 0.75f, -0.2f, 0.2f));
    float sy = animation.GetScaleY() * (1.0f - Clampf(velocity * 0.25f, -0.2f, 0.2f));
    float tw = thumbW * sx, th = thumbH * sy;
    ImVec2 thumbPos(p.x + tx - (tw - thumbW) * 0.5f,
                    thumbY + (thumbH - th) * 0.5f);

    dl->AddRectFilled(ImVec2(p.x, trackY), ImVec2(p.x + sizePx.x, trackY + trackH),
                      ToImU32(track), trackH * 0.5f, 0);
    float fillW = sizePx.x * progress;
    if (fillW > 0.5f) {
        float fillRadius = (fillW < trackH ? fillW : trackH) * 0.5f;
        dl->AddRectFilled(ImVec2(p.x, trackY), ImVec2(p.x + fillW, trackY + trackH),
                          ToImU32(effectiveAccent), fillRadius, 0);
    }

    GlassParams params;
    params.shape = GlassShape();
    params.effects.SetVibrancyEnabled(true);
    params.effects.SetBlurRadiusPx(Dp(8.0f) * (1.0f - pressP));
    params.effects.SetLens(Dp(10.0f) * pressP,
                           Dp(14.0f) * pressP, false, true);
    params.effects.SetChromaticAberration(1.0f);
    float sa = Clampf(1.0f - pressP, 0.0f, 1.0f);
    params.surfaceColor = ImVec4(1, 1, 1, sa);
    params.tintMode = 2.0f;
    params.backdropFallback = FrostedBackdrop();
    params.opaqueBackdrop = false;
    float hp = Clampf(pressP * pressHighlightIntensity, 0.0f, 1.0f);
    params.hasHighlight = hp > 0.001f;
    Highlight ambient = Highlight::Ambient();
    params.highlight = Highlight(ambient.widthDp / 1.5f, ambient.blurRadiusDp / 1.5f,
                                 hp, ambient.style);
    params.hasShadow = true;
    params.shadow = Shadow(4.0f, ((int)(255.0f * 0.05f + 0.5f) << 24));
    params.hasInnerShadow = pressP > 0.001f;
    params.innerShadow = InnerShadow(4.0f * pressP, pressP);

    params.backdropScaleX = Lerpf(2.0f / 3.0f, 1.0f, pressP);
    params.backdropScaleY = Lerpf(0.0f, 1.0f, pressP);
    params.backdropMix = 1.0f;
    DrawGlass(ImRect(thumbPos, ImVec2(thumbPos.x + tw, thumbPos.y + th)), params);
    return changed;
}

bool LiquidBottomTabs::Draw(const ImVec2& sizePx) {
    changed = false;
    ImVec2 p = ImGui::GetCursorScreenPos();
    int n = count > 0 ? count : 1;
    if (n > 8) n = 8;

    float panelH = Dp(64.0f);
    float innerH = Dp(56.0f);
    float inset = Dp(4.0f);
    float panelY = p.y + (sizePx.y - panelH) * 0.5f;
    float tabWidth = (sizePx.x - Dp(8.0f)) / (float)n;
    if (tabWidth < 1.0f) tabWidth = 1.0f;

    ImGui::PushID(this);
    ImGui::InvisibleButton("##ytbl_tabs", sizePx, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::PopID();
    bool held = ImGui::IsItemActive();
    if (!held && !touchActive) gestureMoved = false;
    if (ImGui::IsItemActivated()) {
        lastRawX = ImGui::GetIO().MousePos.x;
        gestureDownX = ImGui::GetIO().MousePos.x;
        gestureDownY = ImGui::GetIO().MousePos.y;
        gestureMoved = false;
        releaseHandled = false;
        touchActive = true;
        heldSince = (float)ImGui::GetTime();
        interactive.Press(ImGui::GetIO().MousePos.x - p.x, ImGui::GetIO().MousePos.y - p.y);
        animation.Press();
        panelOffsetSpring.SnapTo(rawDragOffsetPx);
    }
    if (held && touchActive) {
        if ((double)heldSince > 0.0 && ImGui::GetTime() - (double)heldSince > 2.0) {
            ImGui::GetIO().MouseDown[0] = false;
            ImGui::ClearActiveID();
            touchActive = false;
            releaseHandled = true;
            interactive.Release();
            int healTarget = (int)lroundf(animation.GetTargetValue());
            if (!gestureMoved) {
                const float localX = ImGui::GetIO().MousePos.x - (p.x + inset);
                healTarget = (int)floorf(localX / tabWidth);
            }
            if (healTarget < 0) healTarget = 0;
            if (healTarget > n - 1) healTarget = n - 1;
            if (healTarget != selected) { selected = healTarget; changed = true; }
            animation.AnimateToValue((float)healTarget);
            panelOffsetSpring.AnimateTo(0.0f);
            panelOffsetActive = true;
            didTapSelect = true;
        }
        float mx = ImGui::GetIO().MousePos.x, my = ImGui::GetIO().MousePos.y;
        float dx = mx - lastRawX;
        lastRawX = mx;
        float ddx = mx - gestureDownX, ddy = my - gestureDownY;
        if (ddx * ddx + ddy * ddy > Dp(8.0f) * Dp(8.0f)) gestureMoved = true;
        if (fabsf(dx) > 1.5f || fabsf(ddy) > 1.5f) heldSince = (float)ImGui::GetTime();
        float target = animation.GetTargetValue() + dx / tabWidth;
        animation.UpdateValue(Clampf(target, 0.0f, (float)(n - 1)));
        rawDragOffsetPx += dx;
        panelOffsetSpring.SnapTo(rawDragOffsetPx);
        interactive.Move(mx - p.x, my - p.y);
    }
    if (ImGui::IsItemDeactivated() && touchActive && !releaseHandled) {
        const ImVec2 releasePos = ImGui::GetIO().MousePos;
        const bool wasTap = !gestureMoved;
        touchActive = false;
        releaseHandled = true;
        interactive.Release();

        int target = (int)lroundf(animation.GetTargetValue());
        if (wasTap) {
            const float localX = releasePos.x - (p.x + inset);
            target = (int)floorf(localX / tabWidth);
        }
        if (target < 0) target = 0;
        if (target > n - 1) target = n - 1;
        if (target != selected) { selected = target; changed = true; }
        animation.AnimateToValue((float)target);
        panelOffsetSpring.AnimateTo(0.0f);
        panelOffsetActive = true;
        didTapSelect = true;
    }
    if (!held && !ImGui::IsMouseDown(0)) {
        didTapSelect = false;
        releaseHandled = false;
    }

    if (panelOffsetActive) {
        bool active = panelOffsetSpring.Step(ImGui::GetIO().DeltaTime > 0.0f
                                             ? ImGui::GetIO().DeltaTime : 1.0f / 60.0f);
        rawDragOffsetPx = panelOffsetSpring.value;
        if (!active) panelOffsetActive = false;
    }

    float pressP = animation.GetPressProgress();
    float value = animation.GetValue();
    float panelOffset = 0.0f;
    if (sizePx.x > 0) {
        float fraction = Clampf(rawDragOffsetPx / sizePx.x, -1.0f, 1.0f);
        float sign = fraction < 0.0f ? -1.0f : (fraction > 0.0f ? 1.0f : 0.0f);
        panelOffset = Dp(4.0f) * sign * EaseOut(fabsf(fraction));
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    float ihP = 0.0f;
    if (interactive.IsHighlightVisible()) {
        ihP = Clamp01(interactive.GetPressProgress()) * interactive.intensityScale_;
    }

    float panelScale = Lerpf(1.0f, 1.0f + Dp(16.0f) / (sizePx.x > 1 ? sizePx.x : 1), pressP);
    float pw2 = sizePx.x * panelScale, ph2 = panelH * panelScale;
    ImVec2 panelMin(p.x + (sizePx.x - pw2) * 0.5f + panelOffset,
                    panelY + (panelH - ph2) * 0.5f);
    ImRect panelRect(panelMin, ImVec2(panelMin.x + pw2, panelMin.y + ph2));

    {
        GlassParams params;
        params.shape = GlassShape();
        params.effects.SetVibrancyEnabled(true);
        params.effects.SetBlurRadiusPx(Dp(8.0f));
        params.effects.SetLens(Dp(24.0f), Dp(24.0f));
        const int container = IsLightTheme() && containerColor == 0x66121212
            ? 0x66FAFAFA : containerColor;
        float cr2 = ((container >> 16) & 0xFF) / 255.0f;
        float cg2 = ((container >> 8) & 0xFF) / 255.0f;
        float cb2 = (container & 0xFF) / 255.0f;
        float ca2 = ((container >> 24) & 0xFF) / 255.0f;
        params.surfaceColor = ImVec4(cr2, cg2, cb2, ca2);
        params.tintMode = 2.0f;
        params.backdropFallback = FrostedBackdrop();
        params.opaqueBackdrop = false;
        params.hasHighlight = true;
        params.highlight = Highlight();
        params.hasShadow = true;
        params.shadow = Shadow();
        params.hasInnerShadow = false;
        if (ihP > 0.001f) {
            float xg = (value + 0.5f) * tabWidth + panelOffset;
            float indCx = pw2 * 0.5f + (xg - sizePx.x * 0.5f) * panelScale;
            params.ihProgress = ihP;
            params.ihX = Clampf(indCx, 0.0f, pw2);
            params.ihY = Clampf(ph2 * 0.5f, 0.0f, ph2);
        }
        DrawGlass(panelRect, params);
    }

    const int accent =
        IsLightTheme() && accentColor == 0xFF0091FF ? 0xFF0088FF : accentColor;

    float indW = tabWidth;
    float indH = innerH;
    float indX = p.x + inset + value * tabWidth + panelOffset;
    float indY = panelY + inset;
    ImRect indRect(ImVec2(indX, indY), ImVec2(indX + indW, indY + indH));

    float velocity = animation.GetVelocity() / 10.0f;
    float isx = animation.GetScaleX() / (1.0f - Clampf(velocity * 0.75f, -0.2f, 0.2f));
    float isy = animation.GetScaleY() * (1.0f - Clampf(velocity * 0.25f, -0.2f, 0.2f));
    float iw = indW * isx, ih = indH * isy;
    ImVec2 indMin(indRect.Min.x - (iw - indW) * 0.5f, indRect.Min.y - (ih - indH) * 0.5f);
    ImRect indVisual(indMin, ImVec2(indMin.x + iw, indMin.y + ih));

    const float tabTextSize = Dp(12.0f);
    ImFont* tabFont = ImGui::GetFont();
    const ImU32 labelCol = IsLightTheme()
        ? ToImU32(Argb(1.0f, 0x000000))
        : ToImU32(Argb(1.0f, 0xFFFFFF));

    for (int i = 0; i < n; i++) {
        float cx = p.x + inset + tabWidth * ((float)i + 0.5f) + panelOffset;
        float cy = (panelY + inset) + innerH * 0.5f;
        ImVec2 ts = tabFont->CalcTextSizeA(tabTextSize, 1e6f, 0.0f,
                                           labels[i] ? labels[i] : "");
        ImU32 col = (i == selected) ? ToImU32(accent) : labelCol;
        dl->AddText(tabFont, tabTextSize,
                    ImVec2(cx - ts.x * 0.5f, cy - ts.y * 0.5f), col,
                    labels[i] ? labels[i] : "");
    }

    {
        GlassParams params;
        params.shape = GlassShape();
        params.effects.SetVibrancyEnabled(false);
        params.effects.SetBlurRadiusPx(0.0f);
        params.effects.SetLens(Dp(10.0f) * pressP,
                               Dp(14.0f) * pressP, false, true);
        params.effects.SetChromaticAberration(0.35f + 0.75f * pressP);
        params.backdropScaleX = 1.0f;
        params.backdropScaleY = 1.0f;
        params.backdropMix = 0.0f;
        float ea = Clampf(0.1f * (1.0f - pressP), 0.0f, 1.0f);
        float ba = Clampf(0.03f * pressP, 0.0f, 1.0f);
        float outA = ba + ea * (1.0f - ba);
        const float edgeColor = IsLightTheme() ? 0.0f : 1.0f;
        const float edgeOut = outA > 0.0001f
            ? (edgeColor * ea * (1.0f - ba)) / outA : 0.0f;
        params.surfaceColor = ImVec4(edgeOut, edgeOut, edgeOut, outA);
        params.tintMode = 2.0f;
        params.backdropFallback = FrostedBackdrop();
        params.opaqueBackdrop = false;
        float hp = Clampf(pressP * pressHighlightIntensity, 0.0f, 1.0f);
        params.hasHighlight = hp > 0.001f;
        params.highlight = Highlight(0.5f, 0.25f, hp, HighlightStyle::Default());
        params.hasShadow = pressP > 0.001f;
        params.shadow = Shadow(24.0f, 0.0f, 4.0f,
                               ((int)(255.0f * 0.1f + 0.5f) << 24), pressP);
        params.hasInnerShadow = pressP > 0.001f;
        params.innerShadow = InnerShadow(8.0f * pressP, pressP);
        DrawGlass(indVisual, params);
    }

    if (interactive.IsAnimating() || interactive.IsHighlightVisible()) {
        interactive.Step(ImGui::GetIO().DeltaTime > 0.0f ? ImGui::GetIO().DeltaTime : 1.0f / 60.0f);
    }

    return changed;
}

}
