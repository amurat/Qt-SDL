#include "MainWindow.h"
#include "eglsetup.h"

MainWindow::MainWindow() : MainWindowWidget(new QWidget) {
	setWindowTitle("QMainWindow EGL Rendering Example");
	setCentralWidget(MainWindowWidget);	// Basic setup, ensuring that the window has a widget
	setBaseSize(640, 480);				// inside of it that we can render to
	resize(640, 480);

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

	position = 0;
	dir = 1;
}

MainWindow::~MainWindow() {

	delete Time;

	Time = 0;
}

extern void SetupGLES2Renderer();
extern void RenderGLES2Renderer();

extern void SetupGL2Renderer();
extern void RenderGL2Renderer();

void MainWindow::EGLInit() {
	//setenv("GALOGEN_GL4ES_LIBRARY", "libGL4ES.dylib", 1);
   
	// Create window
	const bool bInitGLES = true;
	const bool bRenderGLES = true;

    void* nativeWindow = (void*)centralWidget()->winId();
    SetupEGLFromNSView(nativeWindow);
    SetupGLES2Renderer();
}

void MainWindow::EGLTerminate(){
    TerminateEGL();
}

void MainWindow::Render() {
	RenderGLES2Renderer();
    EndEGLFrame();
}

