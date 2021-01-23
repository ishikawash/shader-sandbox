cmake_minimum_required(VERSION 3.18.0)

project(shader-sandbox)

set(CMAKE_CXX_STANDARD 14)
list(APPEND CMAKE_CXX_FLAGS -Wall)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
    Qt6
    REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets
)

set(
    SHADER_SANDBOX_SOURCES
    main.cpp
    mainwindow.ui
    mainwindow.cpp
    glwidget.cpp
    shaderprogrambuilder.cpp
    assets.qrc
)

if(APPLE)
    add_executable(
        shader-sandbox
        MACOSX_BUNDLE
        ${SHADER_SANDBOX_SOURCES}
    )
else()
    add_executable(
        shader-sandbox
        ${SHADER_SANDBOX_SOURCES}
    )
endif()
target_include_directories(
    shader-sandbox
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(
    shader-sandbox
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::OpenGL
    PRIVATE Qt6::OpenGLWidgets
)
