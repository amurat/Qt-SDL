// standalone browser application: Emscripten EGL on the page's <canvas>, WebGL2 as GLES 3.0
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include "rendergles2.h"
#include "glescontext.h"

namespace {

struct App {
    GLESContext* context;
    RenderGL* rendergl;
};

void frame(void* arg)
{
    App* app = static_cast<App*>(arg);

    // backing store follows the CSS size at the device pixel ratio
    double cssWidth = 0.0;
    double cssHeight = 0.0;
    emscripten_get_element_css_size("#canvas", &cssWidth, &cssHeight);
    const double ratio = emscripten_get_device_pixel_ratio();
    const int w = static_cast<int>(cssWidth * ratio + 0.5);
    const int h = static_cast<int>(cssHeight * ratio + 0.5);
    if (w <= 0 || h <= 0) {
        return;
    }
    int canvasWidth = 0;
    int canvasHeight = 0;
    emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    if (canvasWidth != w || canvasHeight != h) {
        emscripten_set_canvas_element_size("#canvas", w, h);
    }

    // the browser presents the canvas when the frame callback returns
    app->rendergl->render(app->context, w, h);
}

}  // namespace

int main()
{
    // Emscripten's eglCreateWindowSurface always binds Module.canvas
    GLESContext* context = new GLESContext(nullptr);
    if (!context->create()) {
        std::cout << "Unable to create the EGL context." << std::endl;
        return 1;
    }

    RenderGL* rendergl = new RenderGLES2();
    rendergl->setup(context);

    static App app = { context, rendergl };
    // requestAnimationFrame
    emscripten_set_main_loop_arg(frame, &app, 0, false);
    return 0;
}
