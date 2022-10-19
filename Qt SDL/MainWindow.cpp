#include "MainWindow.h"

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

    SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
    
	if (bInitGLES) {
		SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    } else {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	}
		
	// Explicitly set channel depths, otherwise we might get some < 8
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


#if 0
	SetWindow(SDL_CreateWindowFrom((void *)centralWidget()->winId()));
#else
    auto windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    SetWindow(SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, 512, 512, windowFlags));
    
#endif
	//SetRenderer(SDL_CreateRenderer(GetWindow(), -1, SDL_RENDERER_ACCELERATED));

/*	
  auto windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
  auto window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, 512, 512, windowFlags);
*/
  // Init GL
  auto window = WindowRef;
  auto glContext = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, glContext);
  SetupGLES2Renderer();
  /*
  if (bRenderGLES) {
    RunGLES2Renderer(window);
  } else {
    RunGL2Renderer(window);
  }

    // Clean up
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  */
}

void MainWindow::Render() {
	auto window = WindowRef;
	RenderGLES2Renderer();
    SDL_GL_SwapWindow(window);
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
