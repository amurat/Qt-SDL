#include "glesrhiwidget.h"
#include "glescontext.h"
#include "rendergles2.h"

#ifdef _DEBUG
#include "glesdebug.h"
#endif

#include <rhi/qrhi.h>
#include <rhi/qrhi_platform.h>
#include <QDebug>

#ifdef _WIN32
#include <d3d11.h>
#include <dxgi.h>

namespace {
template <typename T> void safeRelease(T*& p)
{
    if (p) {
        p->Release();
        p = nullptr;
    }
}

bool adapterLuid(ID3D11Device* device, LUID* luid)
{
    IDXGIDevice* dxgiDevice = nullptr;
    IDXGIAdapter* adapter = nullptr;
    DXGI_ADAPTER_DESC desc;
    bool ok = SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))
        && SUCCEEDED(dxgiDevice->GetAdapter(&adapter))
        && SUCCEEDED(adapter->GetDesc(&desc));
    if (ok) {
        *luid = desc.AdapterLuid;
    }
    safeRelease(adapter);
    safeRelease(dxgiDevice);
    return ok;
}
}
#endif

GLESRhiWidget::GLESRhiWidget(QWidget * parent) :
    QRhiWidget(parent),
#ifdef _WIN32
    angleTexture_(0), qtTexture_(0), sharedTexture_(0),
#endif
    context_(0), rendergl_(0), failed_(false)
{
#ifdef _WIN32
    setApi(QRhiWidget::Api::Direct3D11);
#else
    setApi(QRhiWidget::Api::Metal);
#endif
    // GL writes rows bottom-up into the texture
    setMirrorVertically(true);
}

GLESRhiWidget::~GLESRhiWidget()
{
    // drop the EGLImage before QRhiWidget releases the texture it wraps
    delete context_;
    context_ = 0;
#ifdef _WIN32
    releaseSharedTarget();
#endif
    delete rendergl_;
    rendergl_ = 0;
}

void GLESRhiWidget::releaseResources()
{
    if (context_) {
        context_->releaseRenderTarget();
    }
#ifdef _WIN32
    releaseSharedTarget();
#endif
}

void GLESRhiWidget::initialize(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    if (failed_) {
        return;
    }

    bool firstTime = false;
    if (!context_) {
        context_ = new GLESContext(0);
#ifdef _WIN32
        // ANGLE must use Qt's adapter to open the shared texture
        const QRhiD3D11NativeHandles *nh = static_cast<const QRhiD3D11NativeHandles *>(rhi()->nativeHandles());
        LUID luid;
        if (nh && nh->dev && adapterLuid(static_cast<ID3D11Device *>(nh->dev), &luid)) {
            context_->setD3D11Adapter(luid.LowPart, luid.HighPart);
        }
#endif
        if (!context_->createOffscreen()) {
            qWarning() << "GLESRhiWidget: unable to create EGL context";
            failed_ = true;
            return;
        }
        firstTime = true;
    }

    // called again whenever Qt reallocates colorTexture(), e.g. on resize
    if (!setRenderTarget()) {
        failed_ = true;
        return;
    }

    if (firstTime) {
#ifdef _DEBUG
        EnableGLESDebugHandler();
#endif
        rendergl_ = new RenderGLES2();
        rendergl_->setup(context_);
    }
}

void GLESRhiWidget::render(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    if (failed_ || !rendergl_) {
        return;
    }
    const QSize sz = colorTexture()->pixelSize();
    rendergl_->render(context_, sz.width(), sz.height());
    // complete ANGLE's GPU work before Qt reads the texture
    context_->finish();
#ifdef _WIN32
    QRhiResourceUpdateBatch *u = rhi()->nextResourceUpdateBatch();
    u->copyTexture(colorTexture(), sharedTexture_);
    cb->resourceUpdate(u);
#endif
}

#ifdef _WIN32
bool GLESRhiWidget::setRenderTarget()
{
    // drop the EGLImage before the texture it wraps
    context_->releaseRenderTarget();
    releaseSharedTarget();

    const QSize sz = colorTexture()->pixelSize();
    if (colorTexture()->format() != QRhiTexture::RGBA8) {
        qWarning() << "GLESRhiWidget: unsupported color buffer format" << colorTexture()->format();
        return false;
    }

    const QRhiD3D11NativeHandles *nh = static_cast<const QRhiD3D11NativeHandles *>(rhi()->nativeHandles());
    ID3D11Device *qtDevice = nh ? static_cast<ID3D11Device *>(nh->dev) : nullptr;
    ID3D11Device *angleDevice = static_cast<ID3D11Device *>(context_->d3d11Device());
    if (!qtDevice || !angleDevice) {
        qWarning() << "GLESRhiWidget: missing D3D11 device, Qt" << qtDevice << "ANGLE" << angleDevice;
        return false;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = UINT(sz.width());
    desc.Height = UINT(sz.height());
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
    HRESULT hr = angleDevice->CreateTexture2D(&desc, nullptr, &angleTexture_);
    if (FAILED(hr)) {
        qWarning() << "GLESRhiWidget: CreateTexture2D failed" << Qt::hex << hr;
        return false;
    }

    IDXGIResource *resource = nullptr;
    HANDLE shareHandle = nullptr;
    hr = angleTexture_->QueryInterface(IID_PPV_ARGS(&resource));
    if (SUCCEEDED(hr)) {
        hr = resource->GetSharedHandle(&shareHandle);
        resource->Release();
    }
    if (SUCCEEDED(hr)) {
        hr = qtDevice->OpenSharedResource(shareHandle, IID_PPV_ARGS(&qtTexture_));
    }
    if (FAILED(hr)) {
        qWarning() << "GLESRhiWidget: unable to share texture with Qt's device" << Qt::hex << hr;
        releaseSharedTarget();
        return false;
    }

    sharedTexture_ = rhi()->newTexture(QRhiTexture::RGBA8, sz);
    if (!sharedTexture_->createFrom({ quint64(qtTexture_), 0 })) {
        qWarning() << "GLESRhiWidget: QRhiTexture::createFrom failed";
        releaseSharedTarget();
        return false;
    }

    if (!context_->setD3D11RenderTarget(angleTexture_, sz.width(), sz.height())) {
        qWarning() << "GLESRhiWidget: unable to wrap D3D11 texture" << sz;
        releaseSharedTarget();
        return false;
    }
    return true;
}

void GLESRhiWidget::releaseSharedTarget()
{
    delete sharedTexture_;
    sharedTexture_ = 0;
    safeRelease(qtTexture_);
    safeRelease(angleTexture_);
}
#else
bool GLESRhiWidget::setRenderTarget()
{
    // ANGLE and Qt must share the MTLDevice for the texture to be valid in both
    const QRhiMetalNativeHandles *nh = static_cast<const QRhiMetalNativeHandles *>(rhi()->nativeHandles());
    void *angleDevice = context_->metalDevice();
    if (nh && angleDevice && nh->dev != angleDevice) {
        qWarning() << "GLESRhiWidget: ANGLE and Qt use different Metal devices" << angleDevice << nh->dev;
    }

    const QSize sz = colorTexture()->pixelSize();
    void *mtlTexture = reinterpret_cast<void *>(colorTexture()->nativeTexture().object);
    if (!context_->setMetalRenderTarget(mtlTexture, sz.width(), sz.height())) {
        qWarning() << "GLESRhiWidget: unable to wrap Metal texture" << sz;
        return false;
    }
    return true;
}
#endif
