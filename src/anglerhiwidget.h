#pragma once
#include <QRhiWidget>

class GLESContext;
class RenderGL;

// QRhiWidget (Metal) whose color texture ANGLE renders into directly: the
// widget's MTLTexture is wrapped as an EGLImage and attached to an ANGLE FBO.
class ANGLERhiWidget : public QRhiWidget {
public:
    ANGLERhiWidget(QWidget * parent = 0);
    virtual ~ANGLERhiWidget();

    void initialize(QRhiCommandBuffer *cb) override;
    void render(QRhiCommandBuffer *cb) override;
    void releaseResources() override;

private:
    GLESContext* context_;
    RenderGL* rendergl_;
    bool failed_;
};
