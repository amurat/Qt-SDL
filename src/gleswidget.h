#pragma once
#include <QWidget>
//#include <QGLFormat>

class GLESContext;

class GLESWidget : public QWidget {
public:
    GLESWidget(QWidget * parent = 0,
             const GLESWidget * shareWidget = 0,
             Qt::WindowFlags f = Qt::WindowFlags());

/*
    GLESWidget(QGLFormat &fmt,
             QWidget * parent = 0,
             const GLESWidget * shareWidget = 0,
             Qt::WindowFlags f = Qt::WindowFlags());

    GLESWidget(QGLContext * context,
             QWidget * parent = 0,
             const GLESWidget * shareWidget = 0,
             Qt::WindowFlags f = Qt::WindowFlags());
 */
    virtual ~GLESWidget();
    
    void initialize();

    bool autoBufferSwap() const;
    void setAutoBufferSwap(bool);
    void makeCurrent();
    void doneCurrent();
    //QGLFormat format() const;
    //QGLContext* context() const;
    //void setFormat(QGLFormat format);
    GLESWidget* glWidget();
    
    void swapBuffers();
    
private:
    GLESContext* context_;
    bool autoSwap_;
};
