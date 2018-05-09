#-------------------------------------------------------------------------------
# mini_analyser build system
# minivideo library must be built first in order for mini_analyser to work
#-------------------------------------------------------------------------------

TARGET       = mini_analyser
VERSION      = 44

CONFIG      += c++11
QT          += core svg gui widgets printsupport
DEFINES     += VERSION_STR=\\\"r$${VERSION}\\\"

# build artifacts
OBJECTS_DIR  = build/
MOC_DIR      = build/
RCC_DIR      = build/
UI_DIR       = build/
DESTDIR      = bin/

# mini_analyser files
SOURCES     += src/main.cpp \
               src/cli.cpp \
               src/mainwindow.cpp \
               src/mainwindow_datas.cpp \
               src/utils.cpp \
               src/about.cpp \
               src/fourcchelper.cpp \
               src/textexport.cpp \
               src/tabexport.cpp \
               src/tabdev.cpp \
               src/tabcontainer.cpp \
               src/hw_apis/videobackends.cpp \
               src/hw_apis/videobackends_ui.cpp

HEADERS     += src/main.h \
               src/cli.h \
               src/mainwindow.h \
               src/mediawrapper.h \
               src/utils.h \
               src/about.h \
               src/fourcchelper.h \
               src/textexport.h \
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
               ui/tabdev.ui \

# mini_analyser resources
RESOURCES   += resources/resources.qrc

# mini_analyser OS icons (macOS and Windows)
ICON         = resources/app/mini_analyser.icns
RC_ICONS     = resources/app/mini_analyser.ico
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

# OS specifics -----------------------------------------------------------------

unix {
    QMAKE_CXXFLAGS += -fPIE

    # Enables AddressSanitizer
    #QMAKE_CXXFLAGS += -fsanitize=address,undefined
    #QMAKE_LFLAGS += -fsanitize=address,undefined

    linux {
        # Add videobackends # Link with video decoding APIs
        exists("/usr/lib/libva.so") {
            DEFINES += VIDEOBACKEND_VAAPI
            SOURCES += src/hw_apis/videobackends_vaapi.cpp
            HEADERS += src/hw_apis/videobackends_vaapi.h
            LIBS    += -lva -lva-drm -lva-x11 -lX11
        }
        !exists("/usr/lib/libva.so") {
            message("You can install 'libva' on your system to enable VA-API decoding/encoding capabilities checker")
        }

        exists("/usr/lib/libvdpau.so") {
            DEFINES += VIDEOBACKEND_VDPAU
            SOURCES += src/hw_apis/videobackends_vdpau.cpp
            HEADERS += src/hw_apis/videobackends_vdpau.h src/hw_apis/vdpau/VDPDeviceImpl.h
            LIBS    += -lvdpau -lX11
        }
        !exists("/usr/lib/libvdpau.so") {
            message("You can install 'libvdpau' on your system to enable VADPAU decoding capabilities checker")
        }

        # Using RPATH
        QMAKE_RPATHDIR += $${PWD}/../minivideo/build

        # Using https://nixos.org/patchelf.html
        #QMAKE_POST_LINK = (patchelf --set-rpath $${PWD}/../minivideo/build/ $${PWD}/bin/mini_analyser)
    }

    macx {
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

# Deployment -------------------------------------------------------------------

win32 {
    # Application packaging
    #system(windeployqt $${OUT_PWD}/$${DESTDIR})

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
    # Bundle packaging
    #system(macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app)

    # Automatic bundle packaging
    deploy.commands = macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app
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
    target_app.extra    = cp $${OUT_PWD}/$${DESTDIR}/$${TARGET} $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    target_app.files   += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    target_app.path     = $${PREFIX}/bin/
    target_icon.files  += $${OUT_PWD}/assets/app/$$lower($${TARGET}).svg
    target_icon.path    = $${PREFIX}/share/pixmaps/
    target_appentry.files  += $$OUT_PWD/assets/app/$$lower($${TARGET}).desktop
    target_appentry.path    = $${PREFIX}/share/applications
    target_appdata.files   += $${OUT_PWD}/assets/app/$$lower($${TARGET}).appdata.xml
    target_appdata.path     = $${PREFIX}/share/appdata
    INSTALLS += target_app target_icon target_appentry target_appdata

    # Clean bin/ directory
    QMAKE_CLEAN += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
}
