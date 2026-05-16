cmake_minimum_required(VERSION 3.20)

project(mini_analyser VERSION 50 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(APP_NAME ${CMAKE_PROJECT_NAME})
set(APP_VERSION ${CMAKE_PROJECT_VERSION})

################################################################################

message(STATUS ">>> BUILDING ${PROJECT_NAME} >>>")
message(STATUS "[CMAKE] CMAKE_VERSION               = ${CMAKE_VERSION}")
message(STATUS "[CMAKE] CMAKE_PREFIX_PATH           = ${CMAKE_PREFIX_PATH}")
message(STATUS "[BUILD] QT_HOST_PATH                = ${QT_HOST_PATH}")
message(STATUS "[BUILD] TOOLCHAIN_FILE              = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[SYSTEM] (HOST) SYSTEM NAME         = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (HOST) SYSTEM VERSION      = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (HOST) SYSTEM PROCESSOR    = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM NAME       = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM VERSION    = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM PROCESSOR  = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "[PROJECT] CMAKE_PROJECT_NAME        = ${PROJECT_NAME}")
message(STATUS "[PROJECT] CURRENT_SOURCE_DIR        = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "[PROJECT] PROJECT_NAME              = ${CMAKE_PROJECT_NAME}")
message(STATUS "[PROJECT] SOURCE_DIR                = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] BUILD_TYPE                = ${CMAKE_BUILD_TYPE}")

################################################################################

set(OpenGL_GL_PREFERENCE GLVND) # CMake policy CMP0072

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg SvgWidgets PrintSupport)

qt_standard_project_setup(REQUIRES 6.5)

add_compile_definitions(VERSION_STR="r${PROJECT_VERSION}")

set(SOURCES
    src/main.cpp
    src/cli.cpp
    src/mainwindow.cpp
    src/mainwindow_datas.cpp
    src/minivideo_utils_qt.cpp
    src/minivideo_textexport_qt.cpp
    src/about.cpp
    src/fourcchelper.cpp
    src/tabexport.cpp
    src/tabdev.cpp
    src/tabcontainer.cpp
    src/hw_apis/videobackends.cpp
    src/hw_apis/videobackends_ui.cpp
)

set(HEADERS
    src/main.h
    src/cli.h
    src/mainwindow.h
    src/mediawrapper.h
    src/minivideo_utils_qt.h
    src/minivideo_textexport_qt.h
    src/about.h
    src/fourcchelper.h
    src/tabexport.h
    src/tabdev.h
    src/tabcontainer.h
    src/hw_apis/videobackends.h
    src/hw_apis/videobackends_ui.h
    thirdparty/portable_endian.h
)

set(FORMS
    ui/mainwindow.ui
    ui/fourcchelper.ui
    ui/videobackends.ui
    ui/about.ui
    ui/tabcontainer.ui
    ui/tabexport.ui
    ui/tabdev.ui
)

set(RESOURCES
    resources/resources.qrc
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ui)

################################################################################

# Release flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG QT_NO_DEBUG QT_NO_DEBUG_OUTPUT)
endif()

# --------------------------------------------------
# Optional Linux video backends
# --------------------------------------------------
if(UNIX AND NOT APPLE)

    add_compile_options(-fPIE -Wall -Wextra)

    # VAAPI
    if(EXISTS "/usr/lib/libva.so")
        add_compile_definitions(VIDEOBACKEND_VAAPI)
        list(APPEND SOURCES src/hw_apis/videobackends_vaapi.cpp)
        list(APPEND HEADERS src/hw_apis/videobackends_vaapi.h)
        list(APPEND EXTRA_LIBS va va-drm va-x11 X11)
    endif()

    # VDPAU
    if(EXISTS "/usr/lib/libvdpau.so")
        add_compile_definitions(VIDEOBACKEND_VDPAU)
        list(APPEND SOURCES src/hw_apis/videobackends_vdpau.cpp)
        list(APPEND HEADERS
            src/hw_apis/videobackends_vdpau.h
            src/hw_apis/vdpau/VDPDeviceImpl.h
        )
        list(APPEND EXTRA_LIBS vdpau X11)
    endif()

endif()

# --------------------------------------------------
# macOS specifics
# --------------------------------------------------
if(APPLE)

    #add_compile_definitions(
    #    VIDEOBACKEND_VDA
    #    VIDEOBACKEND_VTB
    #)

    #list(APPEND SOURCES
    #    src/hw_apis/videobackends_vda.cpp
    #    src/hw_apis/videobackends_vtb.cpp
    #)

    #list(APPEND HEADERS
    #    src/hw_apis/videobackends_vda.h
    #    src/hw_apis/videobackends_vtb.h
    #)

    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

endif()

# --------------------------------------------------
# Target
# --------------------------------------------------
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# --------------------------------------------------
# minivideo dependency
# --------------------------------------------------

if(NOT TARGET MiniVideo::MiniVideo)
    find_package(MiniVideo REQUIRED)
endif()

# --------------------------------------------------
# Third-party includes (from .pri files)
# --------------------------------------------------

add_subdirectory(thirdparty/pugixml)
#target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)

add_subdirectory(thirdparty/QHexView)
#target_link_libraries(${PROJECT_NAME} PRIVATE QHexView)

add_subdirectory(thirdparty/qhexedit2)
#target_link_libraries(${PROJECT_NAME} PRIVATE qhexedit2)

add_subdirectory(thirdparty/qcustomplot)
#target_link_libraries(${PROJECT_NAME} PRIVATE qcustomplot)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        Qt6::SvgWidgets
        Qt6::PrintSupport
        MiniVideo::MiniVideo
        pugixml
        qhexedit
        QHexView
        qcustomplot
        ${EXTRA_LIBS}
)

################################################################################

# --------------------------------------------------
# Output directory
# --------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# --------------------------------------------------
# Windows specifics
# --------------------------------------------------
if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP /Zc:__cplusplus /permissive-
    )
endif()

# --------------------------------------------------
# Installation
# --------------------------------------------------

include(GNUInstallDirs)

install(TARGETS mini_analyser
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# --------------------------------------------------
# Installation (Linux artifacts)
# --------------------------------------------------

## Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES ${CMAKE_SOURCE_DIR}/resources/app/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/app/${CMAKE_PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/app/${CMAKE_PROJECT_NAME}.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/appdata)
endif()

## macOS
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

    set(app_icon_macos "${CMAKE_SOURCE_DIR}/assets/macos/${CMAKE_PROJECT_NAME}.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/assets/macos/Info.plist"
    )
endif()
