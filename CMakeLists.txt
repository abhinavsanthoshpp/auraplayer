cmake_minimum_required(VERSION 3.16)
project(OrionPlayer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGL
    OpenGLWidgets
)

# Search for libmpv shared library (support unversioned and versioned sonames)
find_library(MPV_LIBRARY NAMES mpv PATHS /usr/lib64 /usr/lib /usr/local/lib)
if(NOT MPV_LIBRARY)
    find_file(MPV_LIBRARY NAMES libmpv.so.2 libmpv.so.1 libmpv.so PATHS /usr/lib64 /usr/lib /usr/local/lib)
endif()

if(NOT MPV_LIBRARY)
    message(FATAL_ERROR "Could not find libmpv! Please verify mpv is installed.")
endif()

message(STATUS "Found libmpv: ${MPV_LIBRARY}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SOURCES
    src/main.cpp
    src/OrionEngine.cpp
    src/OrionVideoWidget.cpp
    src/OrionVlcToolbar.cpp
    src/OrionEffectsDialog.cpp
    src/OrionPlaylistView.cpp
    src/OrionPreferencesDialog.cpp
    src/OrionMediaInfoDialog.cpp
    src/OrionStreamDialog.cpp
    src/MainWindow.cpp
)

set(HEADERS
    include/OrionEngine.h
    include/OrionVideoWidget.h
    include/OrionVlcToolbar.h
    include/OrionEffectsDialog.h
    include/OrionPlaylistView.h
    include/OrionPreferencesDialog.h
    include/OrionMediaInfoDialog.h
    include/OrionStreamDialog.h
    include/MainWindow.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(orionplayer
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(orionplayer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${MPV_LIBRARY}
)

# Compiler optimization flags for maximum performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(orionplayer PRIVATE
        -O3
        -march=native
        -pipe
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()

install(TARGETS orionplayer RUNTIME DESTINATION bin)
