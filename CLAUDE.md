# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A testbed for rendering into Qt widgets (and plain Cocoa views) with OpenGL ES via **ANGLE/EGL**. The repo name and `README.md` date from an earlier Qt 5 + SDL2 version. SDL no longer appears in the source, though CMake still defines SDL paths for Windows. The README's instructions (Qt 5.9, SDL 2.0.4, Visual Studio VSIX) are out of date.

## Build

The primary platform is macOS (arm64); the build generates an Xcode project.

```sh
git submodule update --init          # vendor/glesutil (glad EGL/GLES loaders)
THIRDPARTY=/path/to/thirdparty ./build.sh   # cmake -G Xcode -S . -B build; expects $THIRDPARTY/Qt-6.8.4 (+ libpng/libjpeg paths)
cmake --build build --config Debug --target helloworld
```

- `build.sh` only configures. It reads `$THIRDPARTY` for the Qt 6 install (`CMAKE_PREFIX_PATH`).
- Include paths come from `vendor/glad/include`, `vendor/angle/include`, `vendor/glm` and `vendor/glesutil/vendor/`. These vendor dirs are not tracked in git, so they must exist locally.
- Prebuilt ANGLE libraries (`libEGL`, `libGLESv2`, `libabsl`, `libchrome_zlib`, `libc++_chrome`) live in `lib/<platform>/angle/`. A post-build step copies them next to the `helloworld` and `nshelloworld` executables. The executables load them at runtime, so they must sit in the same directory as the binary.
- The project has no tests and no lint setup.

### iOS (`helloworld` only)

```sh
./build-ios.sh sim        # -> build-ios-sim (arm64 simulator, Qt 6.9.1)
xcodebuild -project build-ios-sim/helloworld.xcodeproj -target helloworld -configuration Debug -sdk iphonesimulator ARCHS=arm64 build
xcrun simctl boot "iPhone 16 Pro"
xcrun simctl install booted build-ios-sim/Debug-iphonesimulator/helloworld.app
xcrun simctl launch --console-pty booted com.amurat.helloworld

IOS_DEVELOPMENT_TEAM=<team id> ./build-ios.sh device   # -> build-ios (arm64 device, Qt 6.9.0)
xcodebuild -project build-ios/helloworld.xcodeproj -scheme helloworld -configuration Debug \
  -destination 'id=<xcodebuild device id>' -allowProvisioningUpdates build   # ids: xcodebuild ... -showdestinations
xcrun devicectl device install app --device <devicectl id> build-ios/Debug-iphoneos/helloworld.app
xcrun devicectl device process launch --device <devicectl id> --console com.amurat.helloworld
```

- For device builds, use `-scheme` and `-destination` rather than `-target`, so Xcode can create the development provisioning profile for that device. The Apple ID must be signed in under Xcode › Settings › Accounts. The first time a device is used, add `-allowProvisioningDeviceRegistration` to register it with the team. `xcodebuild` and `devicectl` use different ids for the same device.
- The first launch on a device is refused until the developer certificate is trusted on the device (Settings › General › VPN & Device Management).

- `build-ios.sh` runs the iOS Qt's `qt-cmake`. It reads `QT_IOS_SIM` and `QT_IOS_DEVICE`, which default to the Qt installs under `~/Development/3rdparty.ios-sim` and `~/Development/qt6-gles-ios/3rdparty.ios`.
- Qt adds a default `LaunchScreen.storyboard`. Compiling it needs Xcode's iOS platform component (Xcode › Settings › Components).
- ANGLE for iOS is in `lib/ios/angle/*.xcframework` (iPhone and simulator slices only). It is a newer ANGLE (2.1.22473) than the macOS dylibs (2.1.19841). The frameworks are embedded in the app bundle. On iOS, `glesloader.cpp` loads EGL and GLES from the frameworks with `dlopen("@rpath/lib*.framework/...")` and passes them to glad, instead of glad's `libEGL.dylib` loader.
- `nshelloworld`, `producer`, `consumer` and `testsource` are not built for iOS. `helloworld` uses `qt_add_executable`, so every `target_link_libraries` call on it must use a keyword (`PRIVATE`).

## Targets

| Target | Entry | Notes |
|---|---|---|
| `helloworld` | `src/Program.cpp` | Qt6 `QMainWindow` with an `GLESRhiWidget` (`QRhiWidget`, Metal) as central widget; a 60 fps `QTimer` calls `update()`. |
| `nshelloworld` | `src/main.mm` | macOS-only, pure Cocoa (`NSView` + `NSTimer`), no Qt. Renders to an EGL window surface on the view's `CALayer`. |
| `producer`, `consumer`, `testsource` | `src/*.cpp` | Non-Windows helpers for the POSIX shared-memory transfer in `datatransfer.cpp` (object name `sharedmemtest`). `testsource` writes the float data that `Hemisphere` reads. |

When you add a source file used by rendering, add it to **both** `SRC_FILES` and the `nshelloworld` list in `CMakeLists.txt`.

## Rendering architecture

1. **EGL context.** `GLESContext` (`glescontext.cpp`) loads EGL with glad (`gladLoaderLoadEGL`). On macOS it gets the display through `eglGetPlatformDisplayEXT` with the **Metal ANGLE backend**. It creates a single GLES 3 context with the debug bit set. There are two surface modes:
   - **`create()`** (`nshelloworld`): an EGL window surface. `glesutil.mm` turns the `NSView` into its `CALayer`, which serves as the `EGLNativeWindowType`. The caller calls `swapBuffers()` after each frame.
   - **`createOffscreen()` + `setMetalRenderTarget()`** (`helloworld`): `GLESRhiWidget::initialize()` passes the widget's `colorTexture()` (an `id<MTLTexture>`) to ANGLE. ANGLE wraps it as an EGLImage (`EGL_METAL_TEXTURE_ANGLE`), which is bound to a GL texture and attached to an FBO with a depth24/stencil8 renderbuffer. `makeCurrent()` binds that FBO. The EGL surface is only a 1×1 placeholder pbuffer. `initialize()` runs again whenever Qt reallocates the texture (on resize), and the target is rebuilt each time. `render()` ends with `finish()` (`glFinish`) before Qt composites the texture. No QRhi drawing is recorded. The widget sets `setMirrorVertically(true)` because GL writes rows bottom-up.
   - ANGLE rejects `EGL_METAL_TEXTURE_ANGLE` as a pbuffer buftype (`eglCreatePbufferFromClientBuffer` returns `EGL_BAD_PARAMETER`); it works only through `eglCreateImage`. The bundled `eglext_angle.h` doesn't define that constant, so `glescontext.cpp` defines it locally. The texture must come from ANGLE's own `MTLDevice`; `initialize()` warns if Qt's device differs.
2. **Renderer.** `RenderGLES2` implements the `RenderGL` interface (`rendergl.h`): `setup(ctx)` and `render(ctx, w, h)`, where callers pass pixel sizes.
3. **Scene selection.** The scene is picked at compile time with a `#define` at the top of `rendergles2.cpp`: `RENDER_LINES` (the current default), `RENDER_HEMISPHERE`, `RENDER_ICOSAHEDRON` or `RENDER_TRIANGLE`. Scene objects are file-level statics.
   - `MeshLine` + `linegen`: screen-space thick lines built as triangle strips.
   - `Hemisphere`: geometry driven by data it reads from shared memory (`datatransfer`).
   - `Icosahedron`/`IcoSphere`: a subdivided sphere.
   These classes call GLES through `glad/glad_gles32.h` and use glm for math.
4. **Debugging.** `EnableGLESDebugHandler()` (`glesdebug.cpp`) installs `glDebugMessageCallbackKHR`, which **asserts on any GL debug message**. `helloworld` enables it only under `_DEBUG`; `nshelloworld` always enables it.

The Qt render timer stops when the main window closes (`running_` flag in `MainWindow`).
