cmake_minimum_required(VERSION 3.20)

#set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")

project(helloworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT EMSCRIPTEN)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# the browser build (webhelloworld) has no Qt
if (NOT EMSCRIPTEN)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
    # Qt 6.10+ ships the private module targets as separate packages
    if (Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
        find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    endif()
endif()

if (EMSCRIPTEN)
    # doctest's REQUIRE and ApprovalTests report failures with exceptions
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    # WebGL2 is GLES 3.0; glad looks GL entry points up by name
    set(WEB_GL_LINK_OPTIONS
        -sMIN_WEBGL_VERSION=2
        -sMAX_WEBGL_VERSION=2
        -sGL_ENABLE_GET_PROC_ADDRESS=1
        -sALLOW_MEMORY_GROWTH=1
    )
endif()

add_subdirectory(vendor/glesutil)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/angle/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glesutil/vendor/
    # fallbacks when the untracked vendor/glad and vendor/glm dirs are absent
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glesutil/vendor/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glesutil/vendor/glm
)

set(SRC_FILES 
    src/MainWindow.cpp
    src/MainWindow.h
    src/Program.cpp
    src/rendergl.h
    src/rendergles2.h
    src/rendergles2.cpp
    src/glesrhiwidget.cpp
    src/glesrhiwidget.h
    src/glescontext.cpp
    src/glescontext.h
    src/glesdebug.cpp
    src/hemisphere.cpp
    src/hemisphere.h
    src/icosahedron.cpp
    src/icosahedron.h
    src/glesdebug.h
    src/meshline.cpp
    src/meshline.h
    src/glesloader.h
    src/glesloader.cpp
    src/linegen.h
    src/linegen.cpp
)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SRC_FILES 
        src/datatransfer.cpp
        src/datatransfer.h
        ${SRC_FILES}
    )
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_executable(nshelloworld
        src/main.mm
        src/rendergl.h
        src/rendergles2.cpp
        src/glesutil.mm
        src/glescontext.cpp
        src/glescontext.h
        src/glesdebug.cpp
        src/glesdebug.h
        src/hemisphere.cpp
        src/hemisphere.h
        src/icosahedron.cpp
        src/icosahedron.h
        src/datatransfer.cpp
        src/datatransfer.h
        src/meshline.cpp
        src/meshline.h
        src/glesloader.h
        src/glesloader.cpp
        src/linegen.h
        src/linegen.cpp
    )

    target_link_libraries(nshelloworld glesutil "-framework Cocoa")

    target_include_directories(nshelloworld PUBLIC
    ${INCLUDE_DIRECTORIES}
)
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND NOT IOS AND NOT EMSCRIPTEN)

    add_executable(producer
        src/producer.cpp
        src/datatransfer.cpp
        src/datatransfer.h
    )

    add_executable(consumer
        src/consumer.cpp
        src/datatransfer.cpp
        src/datatransfer.h
    )

    add_executable(testsource
        src/testsource.cpp
        src/datatransfer.cpp
        src/datatransfer.h
    )

endif()

if (EMSCRIPTEN)
    add_executable(webhelloworld
        src/main_web.cpp
        src/rendergl.h
        src/rendergles2.h
        src/rendergles2.cpp
        src/glescontext.cpp
        src/glescontext.h
        src/glesdebug.cpp
        src/glesdebug.h
        src/hemisphere.cpp
        src/hemisphere.h
        src/icosahedron.cpp
        src/icosahedron.h
        src/datatransfer.cpp
        src/datatransfer.h
        src/meshline.cpp
        src/meshline.h
        src/glesloader.h
        src/glesloader.cpp
        src/glesloader_web.cpp
        src/linegen.h
        src/linegen.cpp
    )
    target_include_directories(webhelloworld PRIVATE ${INCLUDE_DIRECTORIES})
    target_link_libraries(webhelloworld PRIVATE glesutil)
    target_link_options(webhelloworld PRIVATE ${WEB_GL_LINK_OPTIONS}
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/src/web/shell.html")
    set_property(TARGET webhelloworld APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/web/shell.html")
else()  # Qt helloworld, up to the matching endif()

qt_add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glesutil)

# iOS app bundle with the ANGLE dynamic frameworks embedded
function(ios_angle_bundle target bundle_id)
    set(ANGLE_IOS_FRAMEWORKS
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/ios/angle/libEGL.xcframework"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/ios/angle/libGLESv2.xcframework"
    )
    target_link_libraries(${target} PRIVATE ${ANGLE_IOS_FRAMEWORKS})
    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER ${bundle_id}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${bundle_id}
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
        XCODE_EMBED_FRAMEWORKS "${ANGLE_IOS_FRAMEWORKS}"
        XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
        XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY ON
    )
endfunction()

if (IOS)
    ios_angle_bundle(${CMAKE_PROJECT_NAME} com.amurat.helloworld)
endif()

# Dependencies
if(MINGW)
    get_filename_component(TOOLCHAIN_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    find_file(ANGLE_EGL_DLL libEGL.dll HINTS "${TOOLCHAIN_BIN}" REQUIRED)
    find_file(ANGLE_GLESV2_DLL libGLESv2.dll HINTS "${TOOLCHAIN_BIN}" REQUIRED)
elseif(WIN32)
    set(EGL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/libEGL.lib")
    set(GLESV2_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/libGLESv2.lib")
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64")
    set(EGL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos-arm64/angle/libEGL.dylib")
    set(GLESV2_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos-arm64/angle/libGLESv2.dylib")
    set(ABSL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos-arm64/angle/libabsl.dylib")
    set(CHROME_ZLIB_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos-arm64/angle/libchrome_zlib.dylib")
    set(CPP_CHROME_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos-arm64/angle/libc++_chrome.dylib")
   else()
    set(EGL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/angle/libEGL.dylib")
    set(GLESV2_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/angle/libGLESv2.dylib")
    set(ABSL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/angle/libabsl.dylib")
    set(CHROME_ZLIB_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/angle/libchrome_zlib.dylib")
    set(CPP_CHROME_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/angle/libc++_chrome.dylib")
  endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EGL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/angle/libEGL.so")
    set(GLESV2_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/angle/libGLESv2.so")
    set(ABSL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/angle/libabsl.so")
    set(CHROME_ZLIB_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/angle/libchrome_zlib.so")
    set(CPP_CHROME_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/angle/libc++.so")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(${PROJECT_NAME} PRIVATE iconv)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::GuiPrivate)



target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    ${INCLUDE_DIRECTORIES}
)

endif()



function(post_build_commands APP_NAME)
    # Dynamic libraries
    if(MINGW)
        # lib/win64 ANGLE needs Chromium's MSVC libc++.dll, which clashes with the
        # toolchain's libc++.dll, so use the toolchain's ANGLE (angleproject package)
        add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ANGLE_EGL_DLL}"
            "${ANGLE_GLESV2_DLL}"
            $<TARGET_FILE_DIR:${APP_NAME}>)
    elseif(WIN32)
        add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/libEGL.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/libGLESv2.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/libc++.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/zlib.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/angle/absl.dll"
            $<TARGET_FILE_DIR:${APP_NAME}>)
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EGL_LIBRARY}"
            "${GLESV2_LIBRARY}"
            "${ABSL_LIBRARY}"
            "${CHROME_ZLIB_LIBRARY}"
            "${CPP_CHROME_LIBRARY}"
            $<TARGET_FILE_DIR:${APP_NAME}>)
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EGL_LIBRARY}"
            "${GLESV2_LIBRARY}"
            "${ABSL_LIBRARY}"
            "${CHROME_ZLIB_LIBRARY}"
            "${CPP_CHROME_LIBRARY}"
            $<TARGET_FILE_DIR:${APP_NAME}>)
    endif()
endfunction()

if (TARGET helloworld)
    post_build_commands(helloworld)
endif()
if (TARGET nshelloworld)
    post_build_commands(nshelloworld)
endif()

# Reference-image tests: render each scene offscreen and compare with tests/approved/<backend>
# macOS, the iOS simulator (run there with simctl spawn), Windows MinGW (D3D11 WARP) and the browser (headless Chrome)
if ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT IOS) OR (IOS AND CMAKE_OSX_SYSROOT MATCHES "iphonesimulator") OR MINGW OR EMSCRIPTEN)
    option(BUILD_RENDER_TESTS "Build the rendertests target (fetches doctest, ApprovalTests.cpp and stb)" ON)
endif()

if (BUILD_RENDER_TESTS)
    include(FetchContent)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.5.3
        GIT_SHALLOW TRUE)
    set(APPROVAL_TESTS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(APPROVAL_TESTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(ApprovalTests
        GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
        GIT_TAG v.10.13.0
        GIT_SHALLOW TRUE)
    # header-only PNG I/O for the reference images
    FetchContent_Declare(stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG 2c980bb59875b0d32144a71867fbdebb2f77cd20)
    FetchContent_MakeAvailable(doctest ApprovalTests stb)

    add_executable(rendertests
        tests/main.cpp
        tests/rendertests.cpp
        tests/imageapproval.cpp
        tests/imageapproval.h
        tests/stb_impl.cpp
        src/rendergl.h
        src/rendergles2.h
        src/rendergles2.cpp
        src/glescontext.cpp
        src/glescontext.h
        src/hemisphere.cpp
        src/hemisphere.h
        src/icosahedron.cpp
        src/icosahedron.h
        src/meshline.cpp
        src/meshline.h
        src/glesloader.h
        src/glesloader.cpp
        src/linegen.h
        src/linegen.cpp
    )
    if (NOT WIN32)
        target_sources(rendertests PRIVATE src/datatransfer.cpp src/datatransfer.h)
    endif()
    target_include_directories(rendertests PRIVATE ${INCLUDE_DIRECTORIES} ${stb_SOURCE_DIR})
    target_link_libraries(rendertests PRIVATE glesutil doctest::doctest ApprovalTests::ApprovalTests)
    enable_testing()
    if (EMSCRIPTEN)
        target_sources(rendertests PRIVATE src/glesloader_web.cpp)
        # tests/web/rendertests_pre.js mirrors tests/approved/webgl into MEMFS and sends the
        # results back to run_rendertests.py, which drives headless Chrome
        target_link_options(rendertests PRIVATE ${WEB_GL_LINK_OPTIONS}
            -sEXIT_RUNTIME=1
            -sFORCE_FILESYSTEM=1
            "--pre-js=${CMAKE_CURRENT_SOURCE_DIR}/tests/web/rendertests_pre.js"
            "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/tests/web/rendertests_shell.html")
        set_property(TARGET rendertests APPEND PROPERTY LINK_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/web/rendertests_pre.js"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/web/rendertests_shell.html")
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_test(NAME rendertests COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/web/run_rendertests.py"
            --build-dir $<TARGET_FILE_DIR:rendertests>
            --source-dir "${CMAKE_CURRENT_SOURCE_DIR}"
            --approved-dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/approved/webgl")
    elseif (IOS)
        target_compile_options(ApprovalTests PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/tests/ios_no_system.h")
        ios_angle_bundle(rendertests com.amurat.rendertests)
        # a UDID picks one simulator when several are booted
        set(RENDERTESTS_SIMULATOR "booted" CACHE STRING "simctl device that runs the iOS rendertests")
        # $<TARGET_FILE> carries $(EFFECTIVE_PLATFORM_NAME) under Xcode, which ctest can't expand
        add_test(NAME rendertests COMMAND xcrun simctl spawn ${RENDERTESTS_SIMULATOR}
            "${CMAKE_BINARY_DIR}/$<CONFIG>-iphonesimulator/rendertests.app/rendertests")
    else()
        if (WIN32)
            # ApprovalTests finds the approved/ dir by splitting __FILE__ on "\", but Ninja
            # passes C:/... paths; clang then spells __FILE__ with the target's separator
            target_compile_options(rendertests PRIVATE -ffile-reproducible)
        endif()
        post_build_commands(rendertests)
        # glad dlopens libEGL.dylib by name, which resolves against the working directory
        add_test(NAME rendertests COMMAND rendertests WORKING_DIRECTORY $<TARGET_FILE_DIR:rendertests>)
    endif()
endif()