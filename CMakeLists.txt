cmake_minimum_required(VERSION 3.15)
project(OCViewer)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)  # Ensures the .ui file is processed

add_executable(${PROJECT_NAME} main.cpp mainwindow.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${OpenCV_LIBS} ${SDL2_LIBRARIES})
