#include "anglerhiwidget.h"
#include "glescontext.h"
#include "rendergles2.h"
#include "rendergl2.h"

#ifdef _DEBUG
#include "glesdebug.h"
#endif

#include <rhi/qrhi.h>
#include <rhi/qrhi_platform.h>
#include <QDebug>
#include <cstdlib>

ANGLERhiWidget::ANGLERhiWidget(QWidget * parent) :
    QRhiWidget(parent), context_(0), rendergl_(0), failed_(false)
{
    setApi(QRhiWidget::Api::Metal);
    // GL writes rows bottom-up into the Metal texture
    setMirrorVertically(true);
}

ANGLERhiWidget::~ANGLERhiWidget()
{
    // drop the EGLImage before QRhiWidget releases the texture it wraps
    delete context_;
    context_ = 0;
    delete rendergl_;
    rendergl_ = 0;
}

void ANGLERhiWidget::releaseResources()
{
    if (context_) {
        context_->releaseRenderTarget();
    }
}

void ANGLERhiWidget::initialize(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    if (failed_) {
        return;
    }

    bool firstTime = false;
    if (!context_) {
        context_ = new GLESContext(0);
        if (!context_->createOffscreen()) {
            qWarning() << "ANGLERhiWidget: unable to create EGL context";
            failed_ = true;
            return;
        }
        firstTime = true;
    }

    // ANGLE and Qt must share the MTLDevice for the texture to be valid in both
    const QRhiMetalNativeHandles *nh = static_cast<const QRhiMetalNativeHandles *>(rhi()->nativeHandles());
    void *angleDevice = context_->metalDevice();
    if (nh && angleDevice && nh->dev != angleDevice) {
        qWarning() << "ANGLERhiWidget: ANGLE and Qt use different Metal devices" << angleDevice << nh->dev;
    }

    // called again whenever Qt reallocates colorTexture(), e.g. on resize
    const QSize sz = colorTexture()->pixelSize();
    void *mtlTexture = reinterpret_cast<void *>(colorTexture()->nativeTexture().object);
    if (!context_->setMetalRenderTarget(mtlTexture, sz.width(), sz.height())) {
        qWarning() << "ANGLERhiWidget: unable to wrap Metal texture" << sz;
        failed_ = true;
        return;
    }

    if (firstTime) {
#ifdef _DEBUG
        EnableGLESDebugHandler();
#endif
        if (getenv("GLCORE")) {
            rendergl_ = new RenderGL2();
        } else {
            rendergl_ = new RenderGLES2();
        }
        rendergl_->setup(context_);
    }
}

void ANGLERhiWidget::render(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    if (failed_ || !rendergl_) {
        return;
    }
    const QSize sz = colorTexture()->pixelSize();
    rendergl_->render(context_, sz.width(), sz.height());
    // complete ANGLE's Metal work before Qt samples the texture
    context_->finish();
}
