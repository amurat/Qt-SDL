#include "glesloader.h"

#include "glad/glad_gles32.h"
#include <iostream>

void initializeGLES()
{
    int gles_version = gladLoaderLoadGLES2();
    if (!gles_version) {
        std::cout << "Unable to load GLES." << std::endl;
        return;
    }
}
