#ifndef GLESLOADER_H
#define GLESLOADER_H

// Load EGL / GLES entry points from ANGLE. On macOS glad finds libEGL.dylib /
// libGLESv2.dylib next to the executable; on iOS the libraries are frameworks
// embedded in the app bundle.
// display: EGL_NO_DISPLAY for client entry points, then the initialized display.
bool loadEGL(void* display);
bool loadGLES();

void initializeGLES();


#endif
