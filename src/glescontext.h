#pragma once

#include "glad/glad_egl.h"

class GLESContext {
public:
    GLESContext(void* nativeWindowHandle);
    virtual ~GLESContext();

    // window surface on nativeWindowHandle
    bool create();
    // 1x1 pbuffer surface (the page canvas with Emscripten); rendering goes to the target set by set*RenderTarget()
    bool createOffscreen();
    // Render into an externally owned id<MTLTexture> (EGL_ANGLE_metal_texture_client_buffer):
    // the texture is wrapped as an EGLImage and attached to an FBO with its own depth/stencil.
    bool setMetalRenderTarget(void* mtlTexture, int width, int height);
    // Same for an ID3D11Texture2D created on d3d11Device() (EGL_ANGLE_image_d3d11_texture)
    bool setD3D11RenderTarget(void* d3d11Texture, int width, int height);
    // Render into an RGBA8 renderbuffer owned by this context, for glReadPixels
    bool setOffscreenRenderTarget(int width, int height);
    void releaseRenderTarget();
    // ANGLE's id<MTLDevice>, or null
    void* metalDevice();
    // ANGLE's ID3D11Device, or null
    void* d3d11Device();
    // D3D11 adapter for createOffscreen(), by LUID; the default adapter if not called
    void setD3D11Adapter(unsigned int luidLow, int luidHigh);
    // render with the WARP software rasterizer instead of an adapter
    void setD3D11Warp(bool warp);

    void swapBuffers();
    void makeCurrent();
    void finish();

private:
    bool initDisplayAndContext(EGLint surfaceType);
    bool attachRenderTarget(EGLenum target, void* buffer, int width, int height);
    // depth/stencil and FBO around colorTexture_ or colorRenderbuffer_
    bool createFramebuffer(int width, int height);
    void* queryDevice(EGLint attribute);

    EGLNativeWindowType nw_;
    EGLDisplay display_;
    EGLConfig config_;
    EGLSurface surface_;
    EGLContext context_;

    EGLImage colorImage_;
    unsigned int fbo_;
    unsigned int colorTexture_;
    unsigned int colorRenderbuffer_;
    unsigned int depthStencil_;

    bool useAdapterLuid_;
    unsigned int adapterLuidLow_;
    int adapterLuidHigh_;
    bool useWarp_;
};
