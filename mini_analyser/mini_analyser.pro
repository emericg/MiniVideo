#-------------------------------------------------------------------------------
# mini_analyser
# Project created by QtCreator 2014-03-23T12:56:32
#-------------------------------------------------------------------------------

TARGET       = mini_analyser
TEMPLATE     = app
CONFIG      += c++11
QT          += core svg gui widgets xml printsupport

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
               src/tabcontainer.cpp

HEADERS     += src/main.h \
               src/mainwindow.h \
               src/utils.h \
               src/about.h \
               src/fourcchelper.h \
               src/tabexport.h \
               src/tabdev.h \
               src/tabcontainer.h

FORMS       += ui/mainwindow.ui \
               ui/fourcchelper.ui \
               ui/about.ui \
               ui/tabexport.ui \
               ui/tabdev.ui \
               ui/tabcontainer.ui

# mini_analyser resources
RESOURCES   += resources/resources.qrc

# mini_analyser OS icons (macOS and Windows)
ICON         = resources/app/icon.icns
RC_ICONS     = resources/app/icon.ico

# third party libraries
SOURCES     += src/thirdparty/qcustomplot/qcustomplot.cpp \
               src/thirdparty/qhexedit2/qhexedit.cpp \
               src/thirdparty/qhexedit2/chunks.cpp \
               src/thirdparty/qhexedit2/commands.cpp

HEADERS     += src/thirdparty/portable_endian.h \
               src/thirdparty/qcustomplot/qcustomplot.h \
               src/thirdparty/qhexedit2/qhexedit.h \
               src/thirdparty/qhexedit2/chunks.h \
               src/thirdparty/qhexedit2/commands.h


# minivideo library
INCLUDEPATH += ../minivideo/src
QMAKE_LIBDIR+= ../minivideo/build
LIBS        += -L../minivideo/build -lminivideo

#-------------------------------------------------------------------------------
# OS specifics

unix {
    linux {
        #
    }

    macx {
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
    # 'automatic' bundle packaging
    system(windeployqt build/)
}

macx {
    # 'automatic' bundle packaging (method 1)
    system(macdeployqt build/$${TARGET}.app)

    # 'manual' bundle packaging (method 2; debug only) (uncomment to enable)
#    QT_FW_DIR = $(QTDIR)/lib/
#    QT_PG_DIR = $(QTDIR)/plugins/
#    !isEmpty(QT_FW_DIR) {
#        FW_DIR = build/$${TARGET}.app/Contents/Frameworks
#        QMAKE_POST_LINK += (mkdir -p $${FW_DIR})
#        QMAKE_POST_LINK += && (cp ../minivideo/build/libminivideo.dylib $${FW_DIR})
#        QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtCore.framework/ ]; then cp -R $${QT_FW_DIR}/QtCore.framework $${FW_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtGui.framework/ ]; then cp -R $${QT_FW_DIR}/QtGui.framework $${FW_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtSvg.framework/ ]; then cp -R $${QT_FW_DIR}/QtSvg.framework $${FW_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtWidgets.framework/ ]; then cp -R $${QT_FW_DIR}/QtWidgets.framework $${FW_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtPrintSupport.framework/ ]; then cp -R $${QT_FW_DIR}/QtPrintSupport.framework $${FW_DIR}; fi)

#        PG_DIR = build/$${TARGET}.app/Contents/PlugIns
#        QMAKE_POST_LINK += && (if [ ! -d $${PG_DIR}/imageformats/ ]; then cp -R $${QT_PG_DIR}/imageformats $${PG_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${PG_DIR}/platforms/ ]; then cp -R $${QT_PG_DIR}/platforms $${PG_DIR}; fi)
#        QMAKE_POST_LINK += && (if [ ! -d $${PG_DIR}/printsupport/ ]; then cp -R $${QT_PG_DIR}/printsupport $${PG_DIR}; fi)

#        # Use bundled libraries (rewrite rpaths)
#        APP = build/$${TARGET}.app/Contents/MacOS/$${TARGET}
#        QMAKE_POST_LINK += && (install_name_tool -change $${PWD}/build/libminivideo.dylib @executable_path/../Frameworks/libminivideo.dylib $${APP})
#        QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore $${APP})
#        QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtSvg.framework/Versions/5/QtSvg @executable_path/../Frameworks/QtSvg.framework/Versions/5/QtSvg $${APP})
#        QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui $${APP})
#        QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets $${APP})
#    }
}
