// Emscripten's EGL entry points by name, for glad. Emscripten's eglGetProcAddress
// only knows GL functions, and glad's own loader dlopens libEGL, which wasm can't.
// Kept apart from glesloader.cpp because glad_egl.h hides the real prototypes.
#include <EGL/egl.h>
#include <cstring>

#define WEB_EGL_PROC(name) { #name, reinterpret_cast<void*>(name) }

void* webEGLProc(const char* name)
{
    static const struct {
        const char* name;
        void* proc;
    } procs[] = {
        WEB_EGL_PROC(eglBindAPI),
        WEB_EGL_PROC(eglChooseConfig),
        WEB_EGL_PROC(eglCreateContext),
        WEB_EGL_PROC(eglCreateWindowSurface),
        WEB_EGL_PROC(eglDestroyContext),
        WEB_EGL_PROC(eglDestroySurface),
        WEB_EGL_PROC(eglGetConfigAttrib),
        WEB_EGL_PROC(eglGetConfigs),
        WEB_EGL_PROC(eglGetCurrentContext),
        WEB_EGL_PROC(eglGetCurrentDisplay),
        WEB_EGL_PROC(eglGetCurrentSurface),
        WEB_EGL_PROC(eglGetDisplay),
        WEB_EGL_PROC(eglGetError),
        WEB_EGL_PROC(eglGetProcAddress),
        WEB_EGL_PROC(eglInitialize),
        WEB_EGL_PROC(eglMakeCurrent),
        WEB_EGL_PROC(eglQueryAPI),
        WEB_EGL_PROC(eglQueryContext),
        WEB_EGL_PROC(eglQueryString),
        WEB_EGL_PROC(eglQuerySurface),
        WEB_EGL_PROC(eglReleaseThread),
        WEB_EGL_PROC(eglSwapBuffers),
        WEB_EGL_PROC(eglSwapInterval),
        WEB_EGL_PROC(eglTerminate),
        WEB_EGL_PROC(eglWaitClient),
        WEB_EGL_PROC(eglWaitGL),
        WEB_EGL_PROC(eglWaitNative),
    };
    for (const auto& proc : procs) {
        if (std::strcmp(proc.name, name) == 0) {
            return proc.proc;
        }
    }
    return nullptr;
}
