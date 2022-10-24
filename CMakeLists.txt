cmake_minimum_required(VERSION 3.19)

project(helloworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

if (1)
    set(sdl_PREFIX "${CMAKE_BINARY_DIR}/sdl")
    set(sdl_INSTALL_DIR "${CMAKE_BINARY_DIR}/sdl")
    set(sdl_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/sdl/download")
    set(sdl_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${sdl_INSTALL_DIR} -DSDL_SHARED=OFF)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(sdl_CMAKE_ARGS "${sdl_CMAKE_ARGS}" -DFORCE_STATIC_VCRT=ON -DHAVE_LIBC=ON)
    endif()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(sdl_CMAKE_ARGS "${sdl_CMAKE_ARGS}" -D__MACOSX__=ON)
    endif()
            
    set(sdl_URL https://www.libsdl.org/release/SDL2-2.24.1.zip)
    set(SDL_SHARED OFF CACHE BOOL "sdl shared")
    set(SDL_SHARED_ENABLED_BY_DEFAULT OFF CACHE BOOL "sdl share enabled by default")
    include(ExternalProject)
    ExternalProject_Add(SDL
        PREFIX ${sdl_PREFIX}
        URL ${sdl_URL}
        INSTALL_DIR ${sdl_INSTALL_DIR}
        CMAKE_ARGS ${sdl_CMAKE_ARGS}
    )

    include_directories(
        ${CMAKE_BINARY_DIR}/sdl/include/SDL2
    )
    link_directories("${CMAKE_BINARY_DIR}/sdl/lib")

    set(OSX_FRAMEWORKS
        Foundation
        AppKit
        ApplicationServices
        CoreVideo
        CoreAudio
        AudioToolbox
        AudioUnit
        IOKit
        ForceFeedback
        Carbon
        Metal
        GameController
        CoreHaptics
    )


    foreach(FW ${OSX_FRAMEWORKS})
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework ${FW}")
    endforeach()
else()
    find_package(SDL2 REQUIRED)
endif()

add_executable(${CMAKE_PROJECT_NAME}
    "Qt SDL/mainwindow.cpp"
    "Qt SDL/Program.cpp"
    "Qt SDL/SDLChild.cpp"
    src/rendergles2.cpp
    #src/rendergl2.cpp
    src/eglsetup.cpp
    src/eglsetup.mm
    src/eglsetup.h
    vendor/galogen/gl.c
    vendor/glad/src/glad_egl.c
    vendor/glad/include/glad/glad_egl.h
)


# Dependencies
if(WIN32)
    set(EGL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/win64/angle/libEGL.lib")
    set(GLESV2_LIBRARY "${PROJECT_SOURCE_DIR}/lib/win64/angle/libGLESv2.lib")
    set(SDL2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/sdl/include/sdl2")
    set(SDL2_LIBRARY
        "${PROJECT_SOURCE_DIR}/lib/win64/sdl2/SDL2.lib"
        "${PROJECT_SOURCE_DIR}/lib/win64/sdl2/SDL2main.lib")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64")
    set(EGL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/angle/libEGL.dylib")
    set(GLESV2_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/angle/libGLESv2.dylib")
    set(GL4ES_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/gl4es/libGL.dylib")
    set(ABSL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/angle/libabsl.dylib")
    set(CHROME_ZLIB_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/angle/libchrome_zlib.dylib")
    set(CPP_CHROME_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos-arm64/angle/libc++_chrome.dylib")
   else()
    set(EGL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/angle/libEGL.dylib")
    set(GLESV2_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/angle/libGLESv2.dylib")
    set(GL4ES_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/gl4es/libGL.dylib")
  endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)
#target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} debug SDL2d optimized SDL2)
target_link_libraries(${PROJECT_NAME} iconv)

target_link_libraries(${PROJECT_NAME}
  ${EGL_LIBRARY}
  ${GLESV2_LIBRARY}
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    #${SDL2_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/vendor/glad/include
    vendor/angle/include
    vendor
    src
)

# Dynamic libraries
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/win64/angle/libEGL.dll"
        "${PROJECT_SOURCE_DIR}/lib/win64/angle/libGLESv2.dll"
        "${PROJECT_SOURCE_DIR}/lib/win64/sdl2/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EGL_LIBRARY}"
        "${GLESV2_LIBRARY}"
        "${ABSL_LIBRARY}"
        "${CHROME_ZLIB_LIBRARY}"
        "${CPP_CHROME_LIBRARY}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GL4ES_LIBRARY}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libGL4ES.dylib)

endif()

add_dependencies(${PROJECT_NAME} SDL)
