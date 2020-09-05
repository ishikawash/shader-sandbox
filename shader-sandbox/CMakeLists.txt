cmake_minimum_required(VERSION 3.18.0)

project(shader-sandbox)

set(CMAKE_CXX_STANDARD 14)
list(APPEND CMAKE_CXX_FLAGS -Wall)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
    Qt5
    REQUIRED COMPONENTS Widgets
)

add_executable(
    shader-sandbox
    main.cpp
    mainwindow.ui
    mainwindow.cpp
    glwidget.cpp
    shaderprogrambuilder.cpp
    assets.qrc
)
target_include_directories(
    shader-sandbox
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(
    shader-sandbox
    Qt5::Widgets
)
