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
find_package(SDL2 REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
    "Qt SDL/mainwindow.cpp"
    "Qt SDL/MdiWindow.cpp"
    "Qt SDL/Program.cpp"
    "Qt SDL/SDLChild.cpp"
)

target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
