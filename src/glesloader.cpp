#include "glesloader.h"

#include "glad/glad_egl.h"
#include "glad/glad_gles32.h"
#include <iostream>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <dlfcn.h>

namespace {
void* eglHandle()
{
    static void* handle = dlopen("@rpath/libEGL.framework/libEGL", RTLD_LAZY | RTLD_LOCAL);
    return handle;
}

void* glesHandle()
{
    static void* handle = dlopen("@rpath/libGLESv2.framework/libGLESv2", RTLD_LAZY | RTLD_LOCAL);
    return handle;
}

typedef GLADapiproc (*GetProcAddressFunc)(const char* name);

GetProcAddressFunc eglGetProcAddressFunc()
{
    void* handle = eglHandle();
    return handle ? reinterpret_cast<GetProcAddressFunc>(dlsym(handle, "eglGetProcAddress")) : nullptr;
}

GLADapiproc lookup(void* handle, const char* name)
{
    GLADapiproc proc = handle ? reinterpret_cast<GLADapiproc>(dlsym(handle, name)) : nullptr;
    if (!proc) {
        GetProcAddressFunc getProcAddress = eglGetProcAddressFunc();
        if (getProcAddress) {
            proc = getProcAddress(name);
        }
    }
    return proc;
}

GLADapiproc loadEGLProc(const char* name)
{
    return lookup(eglHandle(), name);
}

GLADapiproc loadGLESProc(const char* name)
{
    return lookup(glesHandle(), name);
}
}
#endif

bool loadEGL(void* display)
{
#if TARGET_OS_IPHONE
    if (!eglHandle()) {
        std::cout << "Unable to open libEGL.framework: " << dlerror() << std::endl;
        return false;
    }
    return gladLoadEGL((EGLDisplay)display, loadEGLProc) != 0;
#else
    return gladLoaderLoadEGL((EGLDisplay)display) != 0;
#endif
}

bool loadGLES()
{
#if TARGET_OS_IPHONE
    if (!glesHandle()) {
        std::cout << "Unable to open libGLESv2.framework: " << dlerror() << std::endl;
        return false;
    }
    return gladLoadGLES2(loadGLESProc) != 0;
#else
    return gladLoaderLoadGLES2() != 0;
#endif
}

void initializeGLES()
{
    if (!loadGLES()) {
        std::cout << "Unable to load GLES." << std::endl;
        return;
    }
}
