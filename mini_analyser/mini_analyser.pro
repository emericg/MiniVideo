#-------------------------------------------------------------------------------
# mini_analyser build system
# minivideo library must be built first for mini_analyser to work
#-------------------------------------------------------------------------------

TARGET   = mini_analyser
VERSION  = 49
DEFINES += VERSION_STR=\\\"r$${VERSION}\\\"

CONFIG  += c++14
QT      += core svg gui widgets svgwidgets printsupport

!versionAtLeast(QT_VERSION, 6.0) : error("You need at least Qt version 6.0 for $${TARGET}")

# Project files ----------------------------------------------------------------

SOURCES     += src/main.cpp \
               src/cli.cpp \
               src/mainwindow.cpp \
               src/mainwindow_datas.cpp \
               src/minivideo_utils_qt.cpp \
               src/minivideo_textexport_qt.cpp \
               src/about.cpp \
               src/fourcchelper.cpp \
               src/tabexport.cpp \
               src/tabdev.cpp \
               src/tabcontainer.cpp \
               src/hw_apis/videobackends.cpp \
               src/hw_apis/videobackends_ui.cpp

HEADERS     += src/main.h \
               src/cli.h \
               src/mainwindow.h \
               src/mediawrapper.h \
               src/minivideo_utils_qt.h \
               src/minivideo_textexport_qt.h \
               src/about.h \
               src/fourcchelper.h \
               src/tabexport.h \
               src/tabdev.h \
               src/tabcontainer.h \
               src/hw_apis/videobackends.h \
               src/hw_apis/videobackends_ui.h

FORMS       += ui/mainwindow.ui \
               ui/fourcchelper.ui \
               ui/videobackends.ui \
               ui/about.ui \
               ui/tabcontainer.ui \
               ui/tabexport.ui \
               ui/tabdev.ui

# mini_analyser resources
RESOURCES   += resources/resources.qrc

# third party libraries
HEADERS += src/thirdparty/portable_endian.h
include(src/thirdparty/pugixml/pugixml.pri)
include(src/thirdparty/QHexView/QHexView.pri)
include(src/thirdparty/qhexedit2/qhexedit2.pri)
include(src/thirdparty/qcustomplot/qcustomplot.pri)

# Dependencies -----------------------------------------------------------------

# minivideo library
INCLUDEPATH     += $${PWD}/../minivideo/src/
QMAKE_LIBDIR    += $${PWD}/../minivideo/bin/$${QT_ARCH}/
QMAKE_RPATHDIR  += $${PWD}/../minivideo/bin/$${QT_ARCH}/
LIBS            += -lminivideo                                                  # dynamic linking
#LIBS           += $${PWD}/../minivideo/bin/$${QT_ARCH}/libminivideo.a          # static linking

# Build artifacts --------------------------------------------------------------

OBJECTS_DIR  = build/$${QT_ARCH}
MOC_DIR      = build/$${QT_ARCH}
RCC_DIR      = build/$${QT_ARCH}
UI_DIR       = build/$${QT_ARCH}

DESTDIR      = bin/

# Build settings ---------------------------------------------------------------

win32 { DEFINES += _USE_MATH_DEFINES }

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(release, debug|release) : DEFINES += NDEBUG QT_NO_DEBUG QT_NO_DEBUG_OUTPUT

unix {
    # Enables AddressSanitizer
    #QMAKE_CXXFLAGS += -fsanitize=address,undefined,pointer-compare,pointer-subtract -fno-omit-frame-pointer
    #QMAKE_LFLAGS += -fsanitize=address,undefined,pointer-compare,pointer-subtract
}

# OS specifics -----------------------------------------------------------------

unix {
    QMAKE_CXXFLAGS += -fPIE

    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable
    COMPILER_BASENAME = $$basename(QMAKE_CXX)
    contains(COMPILER_BASENAME, "g++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-but-set-variable } # GCC only
    contains(COMPILER_BASENAME, "clang++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-private-field } # Clang only

    linux {
        # Add videobackends # Link with video decoding APIs
        exists("/usr/lib/libva.so") {
            DEFINES += VIDEOBACKEND_VAAPI
            SOURCES += src/hw_apis/videobackends_vaapi.cpp
            HEADERS += src/hw_apis/videobackends_vaapi.h
            LIBS    += -lva -lva-drm -lva-x11 -lX11
        } else {
            message("You can install 'libva' on your system to enable VA-API decoding/encoding capabilities checker")
        }

        exists("/usr/lib/libvdpau.so") {
            DEFINES += VIDEOBACKEND_VDPAU
            SOURCES += src/hw_apis/videobackends_vdpau.cpp
            HEADERS += src/hw_apis/videobackends_vdpau.h src/hw_apis/vdpau/VDPDeviceImpl.h
            LIBS    += -lvdpau -lX11
        } else {
            message("You can install 'libvdpau' on your system to enable VADPAU decoding capabilities checker")
        }

        # Using RPATH
        QMAKE_RPATHDIR += $${PWD}/../minivideo/bin/$${QT_ARCH}/

        # Using https://nixos.org/patchelf.html
        #QMAKE_POST_LINK = (patchelf --set-rpath $${PWD}/../minivideo/bin/$${QT_ARCH}/ $${PWD}/bin/mini_analyser)
    }

    macx {
        # macOS resources
        ICON = resources/app/mini_analyser.icns
        QMAKE_INFO_PLIST = resources/app/Info.plist

        # Add videobackends
        DEFINES += VIDEOBACKEND_VDA
        DEFINES += VIDEOBACKEND_VTB

        SOURCES += src/hw_apis/videobackends_vda.cpp \
                   src/hw_apis/videobackends_vtb.cpp
        HEADERS += src/hw_apis/videobackends_vda.h \
                   src/hw_apis/videobackends_vtb.h

        # Link with video decoding APIs
        LIBS += -Wl,-framework,Foundation -Wl,-framework,CoreFoundation -Wl,-framework,CoreMedia -Wl,-framework,CoreVideo
        LIBS += -Wl,-framework,VideoToolbox -Wl,-framework,VideoDecodeAcceleration

        # Target architecture(s)
        QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

        # Target OS
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

        # Force compiler to use available macOS SDK version (with automatic detection)
        #XCODE_SDK_VERSION = $$system("xcodebuild -sdk macosx -version | grep SDKVersion | cut -d' ' -f2-")
        #QMAKE_MAC_SDK = "macosx$${XCODE_SDK_VERSION}"
        #QMAKE_MACOSX_DEPLOYMENT_TARGET = $${XCODE_SDK_VERSION}

        # Force RPATH to look into the 'Frameworks' dir (doesn't really seems to work...)
        #QMAKE_RPATHDIR += @executable_path/../Frameworks
        #QMAKE_RPATHDIR += $${PWD}/../minivideo/bin/$${QT_ARCH}/

        # Rewrite minivideo rpath
        #QMAKE_POST_LINK = (install_name_tool -add_rpath @executable_path/../Frameworks/. "bin/mini_analyser.app/Contents/MacOS/mini_analyser")
        #QMAKE_POST_LINK = (install_name_tool -change libminivideo.0.dylib @executable_path/../Frameworks/libminivideo.dylib "bin/mini_analyser.app/Contents/MacOS/mini_analyser")
    }
}

win32 {
    # OS icon
    RC_ICONS = resources/app/mini_analyser.ico

    # MSVC compiler flags
    QMAKE_CXXFLAGS += /MP /Zc:__cplusplus /std:c++17 /permissive-
}

# Deployment -------------------------------------------------------------------

win32 {
    # Automatic application packaging
    deploy.commands = $$quote(windeployqt $${OUT_PWD}/$${DESTDIR}/)
    install.depends = deploy
    QMAKE_EXTRA_TARGETS += install deploy

    # Installation
    # TODO?

    # Clean bin/ directory
    # TODO
}

macx {
    # Automatic bundle packaging
    deploy.commands = macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
    deploy.commands += && ($${QMAKE_COPY_FILE} $${OUT_PWD}/../minivideo/bin/$${QT_ARCH}/libminivideo.dylib $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app/Contents/Frameworks/libminivideo.dylib)
    deploy.commands += && (install_name_tool -change @rpath/libminivideo.0.dylib @executable_path/../Frameworks/libminivideo.dylib $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app/Contents/MacOS/mini_analyser)
    install.depends = deploy
    QMAKE_EXTRA_TARGETS += install deploy

    # Installation
    target.files += $${OUT_PWD}/${DESTDIR}/${TARGET}.app
    target.path = $$(HOME)/Applications
    INSTALLS += target

    # Clean bin/ directory
    QMAKE_DISTCLEAN += -r $${OUT_PWD}/${DESTDIR}/${TARGET}.app
}

linux {
    # Installation
    isEmpty(PREFIX) { PREFIX = /usr/local }
    target_app.files   += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    target_app.path     = $${PREFIX}/bin/
    target_icon.files  += $${OUT_PWD}/resources/app/$$lower($${TARGET}).svg
    target_icon.path    = $${PREFIX}/share/pixmaps/
    target_appentry.files  += $${OUT_PWD}/resources/app/$$lower($${TARGET}).desktop
    target_appentry.path    = $${PREFIX}/share/applications
    target_appdata.files   += $${OUT_PWD}/resources/app/$$lower($${TARGET}).appdata.xml
    target_appdata.path     = $${PREFIX}/share/appdata
    INSTALLS += target_app target_icon target_appentry target_appdata
}
