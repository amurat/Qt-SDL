#include "glescontext.h"
#include "glad/glad_gles32.h"
#include "glesloader.h"
#include <iostream>
#include "assert.h"

#if defined(__APPLE__) || defined(_WIN32)
#include "EGL/eglext_angle.h"
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
#endif

// Not in the bundled eglext_angle.h, values from upstream ANGLE's eglext_angle.h
#ifndef EGL_DEVICE_EXT
#define EGL_DEVICE_EXT 0x322C
#endif
#ifndef EGL_METAL_DEVICE_ANGLE
#define EGL_METAL_DEVICE_ANGLE 0x34A6
#endif
#ifndef EGL_METAL_TEXTURE_ANGLE
#define EGL_METAL_TEXTURE_ANGLE 0x34A7
#endif
#ifndef EGL_D3D11_DEVICE_ANGLE
#define EGL_D3D11_DEVICE_ANGLE 0x33A1
#endif
#ifndef EGL_D3D11_TEXTURE_ANGLE
#define EGL_D3D11_TEXTURE_ANGLE 0x3484
#endif
// EGL_ANGLE_platform_angle_device_id
#ifndef EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE
#define EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE 0x34D6
#endif
#ifndef EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE
#define EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE 0x34D7
#endif

typedef EGLBoolean (EGLAPIENTRYP PFNQUERYDISPLAYATTRIBEXTPROC) (EGLDisplay dpy, EGLint attribute, EGLAttrib *value);
typedef EGLBoolean (EGLAPIENTRYP PFNQUERYDEVICEATTRIBEXTPROC) (void *device, EGLint attribute, EGLAttrib *value);

GLESContext::GLESContext(void* nativeWindowHandle) :
    nw_((EGLNativeWindowType)nativeWindowHandle),
    display_(0),
    config_(0),
    surface_(0),
    context_(0),
    colorImage_(EGL_NO_IMAGE),
    fbo_(0),
    colorTexture_(0),
    colorRenderbuffer_(0),
    depthStencil_(0),
    useAdapterLuid_(false),
    adapterLuidLow_(0),
    adapterLuidHigh_(0),
    useWarp_(false)
{
}

GLESContext::~GLESContext()
{
    if (display_) {
        releaseRenderTarget();
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface_) {
            eglDestroySurface(display_, surface_);
            surface_ = 0;
        }
        if (context_) {
            eglDestroyContext(display_, context_);
            context_ = 0;
        }
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
    if (fbo_) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    }
}

void GLESContext::finish()
{
    makeCurrent();
    glFinish();
}

bool GLESContext::initDisplayAndContext(EGLint surfaceType)
{
    // first load
    if (!loadEGL(EGL_NO_DISPLAY)) {
        std::cout << "Unable to load EGL.\n";
       return false;
    }

    // Get Display
#if defined(__APPLE__) || defined(_WIN32)
#ifdef __APPLE__
    const EGLint defaultDisplayAttributes[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
        EGL_NONE,
    };
#else
    EGLint defaultDisplayAttributes[9] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    };
    int n = 2;
    if (useWarp_) {
        defaultDisplayAttributes[n++] = EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE;
        defaultDisplayAttributes[n++] = EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE;
    } else if (useAdapterLuid_) {
        // the adapter must match Qt's so the shared render target opens on both devices
        defaultDisplayAttributes[n++] = EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE;
        defaultDisplayAttributes[n++] = adapterLuidHigh_;
        defaultDisplayAttributes[n++] = EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE;
        defaultDisplayAttributes[n++] = (EGLint)adapterLuidLow_;
    }
    defaultDisplayAttributes[n] = EGL_NONE;
#endif

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
    loadEGL(display_);

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

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
        EGL_NONE, EGL_NONE };

    // pbuffers wrap Qt's RGBA8 texture, so they need alpha
    EGLint alphaSize = (surfaceType == EGL_PBUFFER_BIT) ? 8 : 0;
    EGLint configAttribs[] =
        {
          EGL_RED_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_BLUE_SIZE, 8,
          EGL_ALPHA_SIZE, alphaSize,
          EGL_DEPTH_SIZE, 24,
          EGL_STENCIL_SIZE, 8,
          EGL_SURFACE_TYPE, surfaceType,
          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
          EGL_NONE
        };
    // Choose config
    if ( !eglChooseConfig(display_, configAttribs, &config_, 1, &numConfigs) || numConfigs < 1 )
    {
        return false;
    }

    // Create a GL context
    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs );
    if ( context_ == EGL_NO_CONTEXT )
    {
        return false;
    }
    return true;
}

bool GLESContext::create()
{
    if ( !initDisplayAndContext(EGL_WINDOW_BIT) )
    {
        return false;
    }

    // Create a surface
    surface_ = eglCreateWindowSurface(display_, config_, nw_, 0);
    if ( surface_ == EGL_NO_SURFACE )
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

bool GLESContext::createOffscreen()
{
    if ( !initDisplayAndContext(EGL_PBUFFER_BIT) )
    {
        return false;
    }

    // placeholder surface for eglMakeCurrent, rendering goes to the FBOs
    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    surface_ = eglCreatePbufferSurface(display_, config_, pbufferAttribs);
    if ( surface_ == EGL_NO_SURFACE )
    {
        return false;
    }

    if ( !eglMakeCurrent(display_, surface_, surface_, context_) )
    {
        return false;
    }

    // FBO setup needs GLES entry points before the renderer loads them
    if ( !loadGLES() )
    {
        std::cout << "Unable to load GLES.\n";
        return false;
    }
    return true;
}

void GLESContext::releaseRenderTarget()
{
    if (fbo_ || colorTexture_ || colorRenderbuffer_ || depthStencil_) {
        eglMakeCurrent(display_, surface_, surface_, context_);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo_);
        glDeleteTextures(1, &colorTexture_);
        glDeleteRenderbuffers(1, &colorRenderbuffer_);
        glDeleteRenderbuffers(1, &depthStencil_);
        fbo_ = 0;
        colorTexture_ = 0;
        colorRenderbuffer_ = 0;
        depthStencil_ = 0;
    }
    if (colorImage_ != EGL_NO_IMAGE) {
        eglDestroyImage(display_, colorImage_);
        colorImage_ = EGL_NO_IMAGE;
    }
}

bool GLESContext::setMetalRenderTarget(void* mtlTexture, int width, int height)
{
    return attachRenderTarget(EGL_METAL_TEXTURE_ANGLE, mtlTexture, width, height);
}

bool GLESContext::setD3D11RenderTarget(void* d3d11Texture, int width, int height)
{
    return attachRenderTarget(EGL_D3D11_TEXTURE_ANGLE, d3d11Texture, width, height);
}

bool GLESContext::attachRenderTarget(EGLenum target, void* buffer, int width, int height)
{
    releaseRenderTarget();

    colorImage_ = eglCreateImage(display_, EGL_NO_CONTEXT, target,
                                 (EGLClientBuffer)buffer, nullptr);
    if (colorImage_ == EGL_NO_IMAGE) {
        std::cout << "eglCreateImage failed: 0x" << std::hex << eglGetError() << std::dec << std::endl;
        return false;
    }

    eglMakeCurrent(display_, surface_, surface_, context_);

    glGenTextures(1, &colorTexture_);
    glBindTexture(GL_TEXTURE_2D, colorTexture_);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, colorImage_);
    glBindTexture(GL_TEXTURE_2D, 0);

    return createFramebuffer(width, height);
}

bool GLESContext::setOffscreenRenderTarget(int width, int height)
{
    releaseRenderTarget();
    eglMakeCurrent(display_, surface_, surface_, context_);

    glGenRenderbuffers(1, &colorRenderbuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return createFramebuffer(width, height);
}

bool GLESContext::createFramebuffer(int width, int height)
{
    glGenRenderbuffers(1, &depthStencil_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    if (colorTexture_) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);
    } else {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer_);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil_);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "render target FBO incomplete: 0x" << std::hex << status << std::dec << std::endl;
        releaseRenderTarget();
        return false;
    }
    return true;
}

void* GLESContext::metalDevice()
{
    return queryDevice(EGL_METAL_DEVICE_ANGLE);
}

void* GLESContext::d3d11Device()
{
    return queryDevice(EGL_D3D11_DEVICE_ANGLE);
}

void GLESContext::setD3D11Adapter(unsigned int luidLow, int luidHigh)
{
    useAdapterLuid_ = true;
    adapterLuidLow_ = luidLow;
    adapterLuidHigh_ = luidHigh;
}

void GLESContext::setD3D11Warp(bool warp)
{
    useWarp_ = warp;
}

void* GLESContext::queryDevice(EGLint attribute)
{
    PFNQUERYDISPLAYATTRIBEXTPROC queryDisplayAttrib =
        reinterpret_cast<PFNQUERYDISPLAYATTRIBEXTPROC>(eglGetProcAddress("eglQueryDisplayAttribEXT"));
    PFNQUERYDEVICEATTRIBEXTPROC queryDeviceAttrib =
        reinterpret_cast<PFNQUERYDEVICEATTRIBEXTPROC>(eglGetProcAddress("eglQueryDeviceAttribEXT"));
    if (!queryDisplayAttrib || !queryDeviceAttrib) {
        return nullptr;
    }

    EGLAttrib device = 0;
    if (!queryDisplayAttrib(display_, EGL_DEVICE_EXT, &device) || !device) {
        return nullptr;
    }
    EGLAttrib nativeDevice = 0;
    if (!queryDeviceAttrib(reinterpret_cast<void*>(device), attribute, &nativeDevice)) {
        return nullptr;
    }
    return reinterpret_cast<void*>(nativeDevice);
}
