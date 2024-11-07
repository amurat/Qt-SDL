#include "glescontext.h"
#include <iostream>
#include "assert.h"

#ifdef __APPLE__
#include "EGL/eglext_angle.h"
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
#endif

GLESContext::GLESContext(void* nativeWindowHandle) :
    display_(0),
    surface_(0),
    context_(0),
    nw_((EGLNativeWindowType)nativeWindowHandle)
{
}

GLESContext::~GLESContext()
{
    if (surface_) {
        eglDestroySurface(display_, surface_);
        surface_ = 0;
        eglTerminate(display_);
        display_ = 0;
    }
}

void GLESContext::swapBuffers()
{
    eglSwapBuffers(display_, surface_);
}

void GLESContext::makeCurrent()
{
    eglMakeCurrent(display_, surface_, surface_, context_);
}

void GLESContext::makeSecondaryCurrent()
{
    eglMakeCurrent(display_, surface_, surface_, secondary_);
}

bool GLESContext::create()
{
    // first load
    int egl_version = gladLoaderLoadEGL(NULL);
    if (!egl_version) {
        std::cout << "Unable to load EGL.\n";
       return false;
    }

    // Get Display
#ifdef __APPLE__
    const EGLint defaultDisplayAttributes[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
        EGL_NONE,
    };

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
    assert(eglGetPlatformDisplayEXT != nullptr);

    display_ = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                           reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY),
                                           defaultDisplayAttributes);
#else
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
    if ( display_ == EGL_NO_DISPLAY )
    {
        return false;
    }

    // Initialize EGL
    EGLint majorVersion;
    EGLint minorVersion;
    if ( !eglInitialize(display_, &majorVersion, &minorVersion) )
    {
        return false;
    }
    
    // reload egl version
    egl_version = gladLoaderLoadEGL(display_);

    if ( !eglBindAPI(EGL_OPENGL_ES_API) )
    {
        return false;
    }

    // Get configs
    EGLint numConfigs;
    if ( !eglGetConfigs(display_, 0, 0, &numConfigs) )
    {
        return false;
    }
    
    EGLConfig config;
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
        EGL_NONE, EGL_NONE };

    EGLint configAttribs[] =
        {
          EGL_RED_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_BLUE_SIZE, 8,
          EGL_ALPHA_SIZE, 0,
          EGL_DEPTH_SIZE, 24,
          EGL_STENCIL_SIZE, 8,
          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
          EGL_NONE
        };
    // Choose config
    if ( !eglChooseConfig(display_, configAttribs, &config, 1, &numConfigs) )
    {
        return false;
    }
    
    // Create a surface
    surface_ = eglCreateWindowSurface(display_, config, nw_, 0);
    if ( surface_ == EGL_NO_SURFACE )
    {
        return false;
    }
    
    // Create a GL context
    context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttribs );
    if ( context_ == EGL_NO_CONTEXT )
    {
        return false;
    }

    secondary_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttribs );
    if ( secondary_ == EGL_NO_CONTEXT )
    {
        return false;
    }

    // Make primary context current
    if ( !eglMakeCurrent(display_, surface_, surface_, context_) )
    {
        return false;
    }
    return true;
}
