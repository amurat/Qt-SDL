#pragma once
#include <QRhiWidget>

class GLESContext;
class RenderGL;
class QRhiTexture;
struct ID3D11Texture2D;

// QRhiWidget whose color texture ANGLE renders into.
// macOS/iOS (Metal): the widget's MTLTexture is wrapped as an EGLImage and
// attached to an ANGLE FBO.
// Windows (Direct3D 11): ANGLE has its own D3D11 device on Qt's adapter and
// renders into a shared texture, which Qt copies into the widget's texture.
class GLESRhiWidget : public QRhiWidget {
public:
    GLESRhiWidget(QWidget * parent = 0);
    virtual ~GLESRhiWidget();

    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;

private:
    bool setRenderTarget();
#ifdef _WIN32
    void releaseSharedTarget();

    ID3D11Texture2D* angleTexture_;  // on ANGLE's device
    ID3D11Texture2D* qtTexture_;     // the same texture opened on Qt's device
    QRhiTexture* sharedTexture_;     // qtTexture_ for QRhi
#endif
    GLESContext* context_;
    RenderGL* rendergl_;
    bool failed_;
};
