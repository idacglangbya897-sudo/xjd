#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include "ytbl_api.h"

static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;
static bool       g_Initialized = false;
static int        g_Width = 0, g_Height = 0;

static bool InitEGL(ANativeWindow* window) {
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) return false;
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) return false;
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_NONE};
    EGLConfig config; EGLint numConfigs;
    if (!eglChooseConfig(g_EglDisplay, attribs, &config, 1, &numConfigs) || numConfigs == 0) return false;
    EGLint format;
    eglGetConfigAttrib(g_EglDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    if (g_EglSurface == EGL_NO_SURFACE) return false;
    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_EglContext == EGL_NO_CONTEXT) return false;
    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) return false;
    eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_WIDTH, &g_Width);
    eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_HEIGHT, &g_Height);
    return true;
}

static void TermEGL() {
    if (g_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_EglContext != EGL_NO_CONTEXT) eglDestroyContext(g_EglDisplay, g_EglContext);
        if (g_EglSurface != EGL_NO_SURFACE) eglDestroySurface(g_EglDisplay, g_EglSurface);
        eglTerminate(g_EglDisplay);
    }
    g_EglDisplay = EGL_NO_DISPLAY; g_EglContext = EGL_NO_CONTEXT; g_EglSurface = EGL_NO_SURFACE; g_Initialized = false;
}

static void RenderFrame() {
    if (!g_Initialized) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    ytbl_api::Tick();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)g_Width, (float)g_Height));
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
    ytbl_api::DrawAll();
    ImGui::End();
    ImGui::Render();
    glViewport(0, 0, g_Width, g_Height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

static void HandleAppCmd(android_app* app, int32_t cmd) {
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        if (app->window != nullptr && InitEGL(app->window)) {
            IMGUI_CHECKVERSION(); ImGui::CreateContext();
            ImGui::GetIO().DisplaySize = ImVec2((float)g_Width, (float)g_Height);
            ImGui_ImplAndroid_Init(app->window);
            ImGui_ImplOpenGL3_Init("#version 300 es");
            ytbl_api::Init();
            g_Initialized = true;
        }
        break;
    case APP_CMD_TERM_WINDOW:
        if (g_Initialized) {
            ytbl::Shutdown();
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplAndroid_Shutdown();
            ImGui::DestroyContext();
        }
        TermEGL();
        break;
    case APP_CMD_WINDOW_RESIZED:
        if (g_EglDisplay != EGL_NO_DISPLAY) {
            eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_WIDTH, &g_Width);
            eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_HEIGHT, &g_Height);
            if (g_Initialized) ImGui::GetIO().DisplaySize = ImVec2((float)g_Width, (float)g_Height);
        }
        break;
    default: break;
    }
}

static int32_t HandleInput(android_app* app, AInputEvent* event) {
    if (g_Initialized && ImGui_ImplAndroid_HandleInputEvent(event)) return 1;
    return 0;
}

void android_main(android_app* app) {
    app_dummy();
    app->onAppCmd = HandleAppCmd;
    app->onInputEvent = HandleInput;
    while (true) {
        int events; android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) source->process(app, source);
            if (app->destroyRequested != 0) { TermEGL(); return; }
        }
        if (g_Initialized) RenderFrame();
    }
}