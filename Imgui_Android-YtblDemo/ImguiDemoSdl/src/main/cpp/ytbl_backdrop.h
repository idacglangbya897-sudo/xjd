/**
 * 来自·默檐 -  2026 默檐
 * 2026/9/8 14:10
 *
 * 免责声明:本项目仅供学习、交流与合法用途,开发者本意不在于造成任何不良影响。
 * 使用者应遵守适用法律法规,并自行承担因不当使用所产生的责任。
 */
#pragma once

#include <GLES3/gl3.h>
#include <imgui.h>

namespace ytbl {

inline GLuint s_CaptureTex = 0;
inline int s_CaptureW = 0;
inline int s_CaptureH = 0;

inline GLuint EnsureCaptureTexture(int w, int h) {
    if (w <= 0 || h <= 0) return 0;
    if (s_CaptureTex == 0) {
        glGenTextures(1, &s_CaptureTex);
        glBindTexture(GL_TEXTURE_2D, s_CaptureTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, s_CaptureTex);
    if (w > s_CaptureW || h > s_CaptureH) {
        const int newW = w > s_CaptureW ? w : s_CaptureW;
        const int newH = h > s_CaptureH ? h : s_CaptureH;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newW, newH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        s_CaptureW = newW;
        s_CaptureH = newH;
    }
    return s_CaptureTex;
}

inline bool CaptureRegion(int x, int y, int w, int h, int screenW, int screenH) {
    if (w <= 0 || h <= 0) return false;
    if (!EnsureCaptureTexture(w, h)) return false;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    int glY = screenH - (y + h);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, glY, w, h);
    return glGetError() == GL_NO_ERROR;
}

inline void ReleaseCapture() {
    if (s_CaptureTex != 0) {
        glDeleteTextures(1, &s_CaptureTex);
        s_CaptureTex = 0;
    }
    s_CaptureW = 0;
    s_CaptureH = 0;
}

}
