# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A testbed for rendering into Qt widgets (and plain Cocoa views) with OpenGL ES via **ANGLE/EGL**. The repo name dates from an earlier Qt 5 + SDL2 version. SDL is no longer used.

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
- There is no lint setup. The only tests are the `rendertests` reference-image tests (see below).
- CI: `.github/workflows/macos.yml` builds all targets on `macos-15` (arm64) with Qt 6.11.2 from `install-qt-action`, configuring with `cmake` directly instead of `build.sh`. It then runs `rendertests` with `ctest`; the runner's paravirtual Metal device renders within tolerance of the references. On failure, the `*.received.png` and `*.diff.png` files are uploaded as the `rendertests-mismatches` artifact. The workflow runs only on pushes to `master`/`gles` and on PRs, so start it on another branch with `gh workflow run macos.yml --ref <branch>`.

### Render tests (macOS)

```sh
cmake --build build --config Debug --target rendertests
ctest --test-dir build -C Debug --output-on-failure
(cd build/Debug && RENDERTESTS_APPROVE=1 ./rendertests)   # accept the current output as the reference
```

- `rendertests` (doctest + ApprovalTests.cpp, fetched by `FetchContent`; `-DBUILD_RENDER_TESTS=OFF` skips it) renders each scene offscreen at a fixed size and frame. It compares the result with `tests/approved/metal/rendertests.<test case>.approved.png`.
- The comparison is `ToleranceImageComparator` (`tests/imageapproval.cpp`, PNG through `QImage`). It fails when more than 0.1% of the pixels differ by more than 2/255 in a channel. A mismatch leaves `*.received.png` and `*.diff.png` (differing pixels in red) next to the reference. Both are gitignored.
- An approve run reports every changed image as failed while it copies it over the reference. Rerun to confirm.
- Each image is its own `TEST_CASE`, because doctest stops a test case at its first failed approval. ApprovalTests creates only the last directory level, so `tests/approved/` must exist.
- glad `dlopen`s `libEGL.dylib` by name, so `rendertests` must run from its own directory. `ctest` sets that working directory.
- `Hemisphere` isn't tested, because its input comes from shared memory.

### Windows (MSYS2 CLANG64)

```sh
git submodule update --init --recursive   # also pulls glesutil's glm
./build-win.sh            # cmake -G Ninja with C:/msys64/clang64 clang/clang++ and Qt 6 -> build-win
PATH=/c/msys64/clang64/bin:$PATH cmake --build build-win
```

- Needs the CLANG64 packages `mingw-w64-clang-x86_64-{clang,lld,cmake,ninja,qt6-base,angleproject}`. `MSYS2` overrides the `/c/msys64` root.
- Run with `/c/msys64/clang64/bin` on `PATH` (Qt and libc++ DLLs).
- MinGW builds copy the toolchain's ANGLE (`clang64/bin/libEGL.dll`, `libGLESv2.dll`) next to the exe, not `lib/win64/angle`. The `lib/win64` ANGLE imports Chromium's MSVC-ABI `libc++.dll`, which has the same name as the toolchain's `libc++.dll` that Qt needs, so the process fails to start (exit 127).
- When `vendor/glad` and `vendor/glm` are absent, the copies under `vendor/glesutil/vendor/` are used.
- Qt 6.10+ needs `find_package(Qt6 COMPONENTS GuiPrivate)` for `Qt6::GuiPrivate`; `CMakeLists.txt` does this by version.
- CI: `.github/workflows/windows.yml` runs `build-win.sh` and builds on `windows-latest`, using the runner's `C:\msys64` (`setup-msys2` with `release: false`).

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
- CI: `.github/workflows/ios.yml` builds an unsigned device app (`CODE_SIGNING_ALLOWED=NO`) with Qt 6.11.2 for iOS from `install-qt-action` (`autodesktop: true`), running `qt-cmake` directly. The official Qt iOS binaries have no arm64 simulator slice, so CI doesn't build for the simulator.
- `nshelloworld`, `producer`, `consumer` and `testsource` are not built for iOS. `helloworld` uses `qt_add_executable`, so every `target_link_libraries` call on it must use a keyword (`PRIVATE`).

## Targets

| Target | Entry | Notes |
|---|---|---|
| `helloworld` | `src/Program.cpp` | Qt6 `QMainWindow` with an `GLESRhiWidget` (`QRhiWidget`, Metal on Apple, Direct3D 11 on Windows) as central widget; a 60 fps `QTimer` calls `update()`. |
| `nshelloworld` | `src/main.mm` | macOS-only, pure Cocoa (`NSView` + `NSTimer`), no Qt. Renders to an EGL window surface on the view's `CALayer`. |
| `rendertests` | `tests/*.cpp` | macOS-only reference-image tests (see Render tests). |
| `producer`, `consumer`, `testsource` | `src/*.cpp` | Non-Windows helpers for the POSIX shared-memory transfer in `datatransfer.cpp` (object name `sharedmemtest`). `testsource` writes the float data that `Hemisphere` reads. |

When you add a source file used by rendering, add it to `SRC_FILES`, the `nshelloworld` list and the `rendertests` list in `CMakeLists.txt`.

## Rendering architecture

1. **EGL context.** `GLESContext` (`glescontext.cpp`) loads EGL with glad (`gladLoaderLoadEGL`). On macOS it gets the display through `eglGetPlatformDisplayEXT` with the **Metal ANGLE backend**. It creates a single GLES 3 context with the debug bit set. There are three surface modes:
   - **`create()`** (`nshelloworld`): an EGL window surface. `glesutil.mm` turns the `NSView` into its `CALayer`, which serves as the `EGLNativeWindowType`. The caller calls `swapBuffers()` after each frame.
   - **`createOffscreen()` + `setOffscreenRenderTarget()`** (`rendertests`): an FBO with a GL-owned RGBA8 color renderbuffer, read back with `glReadPixels`.
   - **`createOffscreen()` + `setMetalRenderTarget()`** (`helloworld` on macOS/iOS): `GLESRhiWidget::initialize()` passes the widget's `colorTexture()` (an `id<MTLTexture>`) to ANGLE. ANGLE wraps it as an EGLImage (`EGL_METAL_TEXTURE_ANGLE`), which is bound to a GL texture and attached to an FBO with a depth24/stencil8 renderbuffer. `makeCurrent()` binds that FBO. The EGL surface is only a 1×1 placeholder pbuffer. `initialize()` runs again whenever Qt reallocates the texture (on resize), and the target is rebuilt each time. `render()` ends with `finish()` (`glFinish`) before Qt composites the texture. No QRhi drawing is recorded. The widget sets `setMirrorVertically(true)` because GL writes rows bottom-up.
   - ANGLE rejects `EGL_METAL_TEXTURE_ANGLE` as a pbuffer buftype (`eglCreatePbufferFromClientBuffer` returns `EGL_BAD_PARAMETER`); it works only through `eglCreateImage`. The bundled `eglext_angle.h` doesn't define that constant, so `glescontext.cpp` defines it locally. The texture must come from ANGLE's own `MTLDevice`; `initialize()` warns if Qt's device differs.
2. **Renderer.** `RenderGLES2` implements the `RenderGL` interface (`rendergl.h`): `setup(ctx)` and `render(ctx, w, h)`, where callers pass pixel sizes.
3. **Scene selection.** `RenderGLES2(Scene)` picks the scene at runtime. `RenderGLES2()` uses the default set by a `#define` at the top of `rendergles2.cpp`: `RENDER_LINES` (the current default), `RENDER_HEMISPHERE`, `RENDER_ICOSAHEDRON` or `RENDER_TRIANGLE`. The scene objects and the animation state are members. `render()` draws frame `frame_` and then advances it. `setFrame()` sets that frame, so a given frame always renders the same way (`Hemisphere` still keeps its rotation in statics).
   - `MeshLine` + `linegen`: screen-space thick lines built as triangle strips.
   - `Hemisphere`: geometry driven by data it reads from shared memory (`datatransfer`).
   - `Icosahedron`/`IcoSphere`: a subdivided sphere.
   These classes call GLES through `glad/glad_gles32.h` and use glm for math.
4. **Debugging.** `EnableGLESDebugHandler()` (`glesdebug.cpp`) installs `glDebugMessageCallbackKHR`, which **asserts on any GL debug message**. `helloworld` enables it only under `_DEBUG`; `nshelloworld` always enables it.

The Qt render timer stops when the main window closes (`running_` flag in `MainWindow`).

**Windows (`helloworld`, Direct3D 11).** `GLESContext` gets a D3D11 ANGLE display (`EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE`) on Qt's adapter: the widget passes the adapter LUID of Qt's device through `setD3D11Adapter()` (`EGL_ANGLE_platform_angle_device_id`). ANGLE keeps its own `ID3D11Device`. Sharing Qt's device (`EGL_ANGLE_device_creation`) would also share its immediate context, and Qt's state changes would invalidate ANGLE's cached D3D state.
- `GLESRhiWidget::setRenderTarget()` creates an RGBA8 `D3D11_RESOURCE_MISC_SHARED` texture on ANGLE's device (`d3d11Device()`). It opens the texture on Qt's device with `OpenSharedResource` and wraps it as a `QRhiTexture` with `createFrom()`. `setD3D11RenderTarget()` wraps the ANGLE side as an EGLImage (`EGL_D3D11_TEXTURE_ANGLE`) and attaches it to the FBO, as on Metal.
- `render()` draws, calls `finish()`, then records a QRhi `copyTexture` from the shared texture into `colorTexture()`.
