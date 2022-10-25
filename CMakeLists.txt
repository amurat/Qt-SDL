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
find_package(OPENGL REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
    src/mainwindow.cpp
    src/mainwindow.h
    src/Program.cpp
    src/rendergles2.cpp
    src/rendergl2.cpp
    src/eglsetup.cpp
    src/eglsetup.mm
    src/eglsetup.h
    vendor/galogen/gl.cpp
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
target_link_libraries(${PROJECT_NAME} iconv)

target_link_libraries(${PROJECT_NAME}
  ${EGL_LIBRARY}
  ${GLESV2_LIBRARY}
  #${OPENGL_LIBRARIES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
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

