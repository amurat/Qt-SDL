#include "MainWindow.h"
#include "eglsetup.h"
#include <SDL_syswm.h>

MainWindow::MainWindow() : MainWindowWidget(new QWidget) {
	setWindowTitle("QMainWindow SDL Rendering Example");
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

	//RendererRef = 0;
	WindowRef = 0;
	position = 0;
	dir = 1;
}

MainWindow::~MainWindow() {
	//SDL_DestroyRenderer(RendererRef);	// Basic SDL garbage collection
	SDL_DestroyWindow(WindowRef);

	delete Time;

	//RendererRef = 0;
	WindowRef = 0;
	Time = 0;
}

extern void SetupGLES2Renderer();
extern void RenderGLES2Renderer();

extern void SetupGL2Renderer();
extern void RenderGL2Renderer();

void MainWindow::SDLInit() {
	/*
		In order to do rendering, I need to save the window and renderer contexts
		of this window.
		I use SDL_CreateWindowFrom and pass it the winId() of the widget I wish to
		render to. In this case, I want to render to the main central widget.
	*/
	//setenv("GALOGEN_GL4ES_LIBRARY", "libGL4ES.dylib", 1);
    
	// Create window
	const bool bInitGLES = true;
	const bool bRenderGLES = true;

    void* nativeWindow = (void*)centralWidget()->winId();
#if 0
    //SetWindow(SDL_CreateWindowFrom((void *)nativeWindow));
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version); /* initialize info structure with SDL version info */
    if (SDL_GetWindowWMInfo(WindowRef, &info)) {
        if (SDL_SYSWM_COCOA == info.subsystem) {
         //   std::cout << "Cocoa\n";
        }
    }
    if (!SetupEGLFromNSWindow(info.info.cocoa.window)) {
        assert(false && "SetEGL failed");
    }
#else
    SetupEGLFromNSView(nativeWindow);
#endif
    SetupGLES2Renderer();
}

void MainWindow::Render() {
	RenderGLES2Renderer();
    EndEGLFrame();
}

void MainWindow::SetWindow(SDL_Window * ref) {
	WindowRef = ref;
}

SDL_Window * MainWindow::GetWindow() {
	return WindowRef;
}
/*
SDL_Renderer * MainWindow::GetRenderer() {
	return RendererRef;
}
*/
