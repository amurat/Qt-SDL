#include "MainWindow.h"
#include "gleswidget.h"
#include "rendergles2.h"
#include "rendergl2.h"

#ifdef _DEBUG
#include "glesdebug.h"
#endif

#include <QWindow>

MainWindow::MainWindow() : mainWindowWidget_(0), rendergl(0) {
    mainWindowWidget_ = new GLESWidget();
	setWindowTitle("QMainWindow EGL Rendering Example");
	setCentralWidget(mainWindowWidget_);	// Basic setup, ensuring that the window has a widget
	setBaseSize(640, 480);				// inside of it that we can render to
	resize(640, 480);
    
    mainWindowWidget_->initialize();
#ifdef _DEBUG
    EnableGLESDebugHandler();
#endif
	/*
		I used a timer for animation rendering.
		I tried using update() and repaint() as the slot, but this had
		no effect. I later tried calling update/repaint from within the
		Render() slot, and this caused a flicker. The only thing that I
		can assume is that after the paintEvent override, qt paints the
		grey backgrounds.
	*/
	Time = new QTimer(this);
	connect(Time, SIGNAL(timeout()), this, SLOT(Render()));
	Time->start(1000 / 60);
    bGL2Render = false;
    Init();
}

MainWindow::~MainWindow() {

	delete Time;
	Time = 0;
}

void MainWindow::Init() {
    if (bGL2Render) {
#ifdef __APPLE__
        setenv("GALOGEN_GL4ES_LIBRARY", "libGL4ES.dylib", 1);
#elif defined(__linux__)
        setenv("GALOGEN_GL4ES_LIBRARY", "./libGL4ES.so", 1);
#endif
    }
	// Create window
	const bool bRenderGLES = true;
    // Init GL
    if (!bGL2Render) {
        rendergl = new RenderGLES2();
    } else {
        rendergl = new RenderGL2();
    }
    rendergl->setup();
}

void MainWindow::Render()
{
    float devicePixelRatio = 1.0;
    QWindow * winHandle = windowHandle();
    if (winHandle)
    {
        devicePixelRatio = winHandle->devicePixelRatio();
    }
    rendergl->render(devicePixelRatio*width(), devicePixelRatio*height());
    mainWindowWidget_->swapBuffers();
}

