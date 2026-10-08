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

# Search for libmpv shared library across Linux and Windows
if(WIN32)
    find_library(MPV_LIBRARY NAMES mpv-2 mpv libmpv-2 libmpv PATHS ${CMAKE_CURRENT_SOURCE_DIR}/windows/lib ${CMAKE_PREFIX_PATH})
    if(NOT MPV_LIBRARY)
        set(MPV_LIBRARY mpv)
    endif()
else()
    find_library(MPV_LIBRARY NAMES mpv PATHS /usr/lib64 /usr/lib /usr/local/lib)
    if(NOT MPV_LIBRARY)
        find_file(MPV_LIBRARY NAMES libmpv.so.2 libmpv.so.1 libmpv.so PATHS /usr/lib64 /usr/lib /usr/local/lib)
    endif()
    if(NOT MPV_LIBRARY)
        message(FATAL_ERROR "Could not find libmpv! Please verify mpv is installed.")
    endif()
endif()

message(STATUS "Using libmpv: ${MPV_LIBRARY}")

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
    src/OrionDonateDialog.cpp
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
    include/OrionDonateDialog.h
    include/MainWindow.h
)

set(RESOURCES
    resources/resources.qrc
)

# Windows resource script with icon and version metadata
if(WIN32)
    list(APPEND SOURCES resources/orionplayer.rc)
endif()

if(WIN32)
    add_executable(orionplayer WIN32
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
else()
    add_executable(orionplayer
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

target_link_libraries(orionplayer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${MPV_LIBRARY}
)

if(WIN32)
    target_link_libraries(orionplayer PRIVATE
        dwmapi
        winmm
        imm32
        version
        shell32
        user32
        gdi32
    )
endif()

# Compiler optimization flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(orionplayer PRIVATE
        -O3
        -pipe
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
    if(NOT WIN32)
        target_compile_options(orionplayer PRIVATE -march=native)
    endif()
elseif(MSVC)
    target_compile_options(orionplayer PRIVATE
        /O2
        /W3
        /permissive-
    )
endif()

install(TARGETS orionplayer RUNTIME DESTINATION bin)
