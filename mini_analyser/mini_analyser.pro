#-------------------------------------------------------------------------------
# mini_analyser
# Project created by QtCreator 2014-03-23T12:56:32
#-------------------------------------------------------------------------------

TARGET       = mini_analyser
TEMPLATE     = app
CONFIG      += c++11
QT          += core svg gui widgets printsupport

VERSION      = 41
DEFINES     += VERSION_STR=\\\"r$${VERSION}\\\"

# build artifacts
OBJECTS_DIR  = build/
MOC_DIR      = build/
RCC_DIR      = build/
UI_DIR       = build/
DESTDIR      = build/

# mini_analyser files
SOURCES     += src/main.cpp \
               src/mainwindow.cpp \
               src/mainwindow_datas.cpp \
               src/utils.cpp \
               src/about.cpp \
               src/fourcchelper.cpp \
               src/tabexport.cpp \
               src/tabdev.cpp \
               src/tabcontainer.cpp \
               src/videobackends.cpp \
               src/videobackends_ui.cpp

HEADERS     += src/main.h \
               src/mainwindow.h \
               src/utils.h \
               src/about.h \
               src/fourcchelper.h \
               src/tabexport.h \
               src/tabdev.h \
               src/tabcontainer.h \
               src/videobackends.h \
               src/videobackends_ui.h

FORMS       += ui/mainwindow.ui \
               ui/fourcchelper.ui \
               ui/videobackends.ui \
               ui/about.ui \
               ui/tabexport.ui \
               ui/tabdev.ui \
               ui/tabcontainer.ui

# mini_analyser resources
RESOURCES   += resources/resources.qrc

# mini_analyser OS icons (macOS and Windows)
ICON         = resources/app/icon.icns
RC_ICONS     = resources/app/icon.ico
QMAKE_INFO_PLIST = resources/app/Info.plist

# third party libraries
SOURCES     += src/thirdparty/qcustomplot/qcustomplot.cpp \
               src/thirdparty/qhexedit2/qhexedit.cpp \
               src/thirdparty/qhexedit2/chunks.cpp \
               src/thirdparty/qhexedit2/commands.cpp \
               src/thirdparty/pugixml/pugixml.cpp

HEADERS     += src/thirdparty/portable_endian.h \
               src/thirdparty/qcustomplot/qcustomplot.h \
               src/thirdparty/qhexedit2/qhexedit.h \
               src/thirdparty/qhexedit2/chunks.h \
               src/thirdparty/qhexedit2/commands.h \
               src/thirdparty/pugixml/pugixml.hpp \
               src/thirdparty/pugixml/pugiconfig.hpp

# minivideo library
INCLUDEPATH += ../minivideo/src
QMAKE_LIBDIR+= ../minivideo/build
LIBS        += -L../minivideo/build -lminivideo # dynamic linking
#LIBS        += ../minivideo/build/libminivideo.a # static linking

#-------------------------------------------------------------------------------
# OS specifics

unix {
    linux {
        # Add videobackends
        SOURCES += src/videobackends_vdpau.cpp \
                   src/videobackends_vaapi.cpp
        HEADERS += src/videobackends_vdpau.h \
                   src/vdpau/VDPDeviceImpl.h \
                   src/videobackends_vaapi.h

        # Link with video decoding APIs
        LIBS += -lvdpau -lX11
        LIBS += -lva -lva-drm -lva-x11 -lX11

        # Using RPATH
        QMAKE_RPATHDIR += $${PWD}/../minivideo/build

        # Using https://nixos.org/patchelf.html
        #QMAKE_POST_LINK = (patchelf --set-rpath $${PWD}/../minivideo/build/ $${PWD}/build/mini_analyser)
    }

    macx {
        # Link with video decoding APIs
        #LIBS += -Wl,-framework,Foundation -Wl,-framework,VideoToolbox -Wl,-framework,CoreMedia -Wl,-framework,CoreVideo

        # Force compiler to use available macOS SDK version (with automatic detection)
        XCODE_SDK_VERSION = $$system("xcodebuild -sdk macosx -version | grep SDKVersion | cut -d' ' -f2-")
        QMAKE_MAC_SDK = "macosx$${XCODE_SDK_VERSION}"
        #QMAKE_MACOSX_DEPLOYMENT_TARGET = $${XCODE_SDK_VERSION}

        # Force RPATH to look into the 'Frameworks' dir (doesn't really seems to work...)
        #QMAKE_RPATHDIR += @executable_path/../Frameworks
    }
}

win32 {
    #
}

#-------------------------------------------------------------------------------
# Deployment

win32 {
    # 'automatic' application packaging
    system(windeployqt build/)
}

macx {
    # 'automatic' bundle packaging
    system(macdeployqt build/$${TARGET}.app)
}

linux {
    # 'manual' application packaging (uncomment to enable)

#    isEmpty(PREFIX) {
#        PREFIX = /usr
#    }
#    BINDIR = $${PREFIX}/bin
#    DATADIR =$${PREFIX}/share

#    INSTALLS += target desktop icon

#    target.path = $${BINDIR}

#    desktop.path = $${DATADIR}/applications
#    desktop.files += resources/app/$${TARGET}.desktop

#    icon.path = $${DATADIR}/icons/hicolor/scalable/apps
#    icon.files += resources/app/$${TARGET}.svg
}
