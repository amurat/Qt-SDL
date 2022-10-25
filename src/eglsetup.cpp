#include <glad/glad_egl.h>

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

void EndEGLFrame()
{
    eglSwapBuffers(display, surface);
}

void* GetNativeWindowHandleFromNSWindow(void *window);
void* GetNativeWindowHandleFromNSView(void *view);

static bool SetupEGLNative(void* nw)
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    
    EGLConfig config;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    EGLint configAttribs[] =
        {
          EGL_RED_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_BLUE_SIZE, 8,
          EGL_ALPHA_SIZE, 8,
          EGL_DEPTH_SIZE, 16,
          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
          EGL_NONE
        };
    
    // Get Display
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if ( display == EGL_NO_DISPLAY )
    {
        return false;
    }

    // Initialize EGL
    if ( !eglInitialize(display, &majorVersion, &minorVersion) )
    {
        return false;
    }

    if ( !eglBindAPI(EGL_OPENGL_ES_API) )
    {
        return false;
    }

    // Get configs
    if ( !eglGetConfigs(display, 0, 0, &numConfigs) )
    {
        return false;
    }
    

    // Choose config
    if ( !eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) )
    {
        return false;
    }
    
    // Create a surface
    
    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)nw, 0);
    if ( surface == EGL_NO_SURFACE )
    {
        return false;
    }
    
    // Create a GL context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
    if ( context == EGL_NO_CONTEXT )
    {
        return false;
    }
    
    // Make the context current
    if ( !eglMakeCurrent(display, surface, surface, context) )
    {
        return false;
    }
    
    //*eglDisplay = display;
    //*eglSurface = surface;
    //*eglContext = context;
    return EGL_TRUE;
}

bool SetupEGLFromNSWindow(void* window)
{
    void* nw = GetNativeWindowHandleFromNSWindow(window);
    return SetupEGLNative(nw);
}

bool SetupEGLFromNSView(void* view)
{
    void* nw = GetNativeWindowHandleFromNSView(view);
    return SetupEGLNative(nw);
}

void TerminateEGL()
{
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
}
