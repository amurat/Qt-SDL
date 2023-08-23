cmake_minimum_required(VERSION 3.19)

project(helloworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(QTDIR /Users/AMurat/Development/3rdparty-src/qt-everywhere-src-5.15.8)

add_subdirectory(vendor/glesutil)

set(INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/vendor/glesutil/vendor/glad/include
    ${PROJECT_SOURCE_DIR}/vendor/glesutil/vendor
    vendor/glm
    vendor/angle/include
    vendor
    src
)

set(SRC_FILES 
    src/MainWindow.cpp
    src/MainWindow.h
    src/Program.cpp
    src/rendergl.h
    src/rendergles2.h
    src/rendergles2.cpp
    src/rendergl2.h
    src/rendergl2.cpp
    src/gleswidget.cpp
    src/gleswidget.h
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
    set(SRC_FILES 
        src/gleswidget.mm
        ${SRC_FILES}
    )
    
    add_executable(nshelloworld
        src/main.mm
        src/rendergl.h
        src/rendergles2.cpp
        src/rendergl2.cpp
        src/gleswidget.mm
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

    target_link_libraries(nshelloworld glesutil galogen "-framework Cocoa")

    target_include_directories(nshelloworld PUBLIC
    ${INCLUDE_DIRECTORIES}
)
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")

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

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} glesutil galogen)

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
    set(ABSL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/angle/libabsl.dylib")
    set(CHROME_ZLIB_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/angle/libchrome_zlib.dylib")
    set(CPP_CHROME_LIBRARY "${PROJECT_SOURCE_DIR}/lib/macos/angle/libc++_chrome.dylib")
  endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EGL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/angle/libEGL.so")
    set(GLESV2_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/angle/libGLESv2.so")
    set(GL4ES_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/gl4es/libGL.so")
    set(ABSL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/angle/libabsl.so")
    set(CHROME_ZLIB_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/angle/libchrome_zlib.so")
    set(CPP_CHROME_LIBRARY "${PROJECT_SOURCE_DIR}/lib/linux/angle/libc++.so")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(${PROJECT_NAME} iconv)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)



target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    ${INCLUDE_DIRECTORIES}
)




# Dynamic libraries
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/win64/angle/libEGL.dll"
        "${PROJECT_SOURCE_DIR}/lib/win64/angle/libGLESv2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EGL_LIBRARY}"
        "${GLESV2_LIBRARY}"
        "${ABSL_LIBRARY}"
        "${CHROME_ZLIB_LIBRARY}"
        "${CPP_CHROME_LIBRARY}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GL4ES_LIBRARY}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libGL4ES.so)
endif()

