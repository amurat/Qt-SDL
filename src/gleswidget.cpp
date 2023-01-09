#include "gleswidget.h"
#include "glescontext.h"
#include <iostream>
#include <QThread>

extern void* GetNativeWindowHandleFromNSView(void *view);
extern void* GetNativeWindowHandleFromNSWindow(void *window);

GLESWidget::GLESWidget(QWidget * parent,
            const GLESWidget * shareWidget,
            Qt::WindowFlags f) : QWidget(parent), context_(0), autoSwap_(false)
{
}

GLESWidget::~GLESWidget()
{
    if (context_) {
        delete context_;
        context_ = 0;
    }
}

bool
GLESWidget::autoBufferSwap() const
{
    return false;
}

void 
GLESWidget::setAutoBufferSwap(bool on)
{
    autoSwap_ = on;
}

void GLESWidget::makeCurrent()
{
    if (context_) {
        context_->makeCurrent();
    }
}

void GLESWidget::doneCurrent()
{
    // TODO
}

GLESWidget* GLESWidget::glWidget()
{
    return this;
}

void GLESWidget::swapBuffers()
{
    context_->swapBuffers();
}



void GLESWidget::initialize()
{
    if (context_) {
        return;
    }
    setenv("GALOGEN_GL4ES_LIBRARY", "libGL4ES.dylib", 1);

    void* windowView = (void*)winId();
    void* windowHandle = GetNativeWindowHandleFromNSView(windowView);
    
    context_ = new GLESContext(windowHandle);
    context_->create();
}
