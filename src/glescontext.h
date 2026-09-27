#pragma once

#include "glad/glad_egl.h"

class GLESContext {
public:
    GLESContext(void* nativeWindowHandle);
    virtual ~GLESContext();

    // window surface on nativeWindowHandle
    bool create();
    // 1x1 pbuffer surface; rendering goes to the target set by setMetalRenderTarget()
    bool createOffscreen();
    // Render into an externally owned id<MTLTexture> (EGL_ANGLE_metal_texture_client_buffer):
    // the texture is wrapped as an EGLImage and attached to an FBO with its own depth/stencil.
    bool setMetalRenderTarget(void* mtlTexture, int width, int height);
    void releaseRenderTarget();
    // ANGLE's id<MTLDevice>, or null
    void* metalDevice();

    void swapBuffers();
    void makeCurrent();
    void finish();

private:
    bool initDisplayAndContext(EGLint surfaceType);

    EGLNativeWindowType nw_;
    EGLDisplay display_;
    EGLConfig config_;
    EGLSurface surface_;
    EGLContext context_;

    EGLImage colorImage_;
    unsigned int fbo_;
    unsigned int colorTexture_;
    unsigned int depthStencil_;
};
