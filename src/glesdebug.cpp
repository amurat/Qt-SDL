#include "glesdebug.h"
#include "glad/glad_gles32.h"
#include <iostream>
#include <cassert>

static void on_gl_error(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void *userParam)
{
    std::cout << "glDebugMessageCallbackKHR: " << message << std::endl;
    assert(false);
}

void EnableGLESDebugHandler()
{
    int gles_version = gladLoaderLoadGLES2();
    if (!gles_version) {
        std::cout << "EnableGLESDebugHandler ERROR: Unable to load GLES." << std::endl;
        return;
    }
   
    glDebugMessageCallbackKHR(on_gl_error, 0);
}
