#-------------------------------------------------
# mini_analyser
# Project created by QtCreator 2014-03-23T12:56:32
#-------------------------------------------------

TARGET       = mini_analyser
TEMPLATE     = app
QT          += core gui widgets svg

OBJECTS_DIR  = build/
MOC_DIR      = build/
RCC_DIR      = build/
UI_DIR       = build/
DESTDIR      = build/

# mini_analyser files
SOURCES     += src/main.cpp \
               src/mainwindow.cpp \
               src/utils.cpp \
               src/fourcchelper.cpp \
               src/hexeditor.cpp \
               src/thirdparty/qhexedit2/qhexedit.cpp \
               src/thirdparty/qhexedit2/chunks.cpp \
               src/thirdparty/qhexedit2/commands.cpp

HEADERS     += src/main.h \
               src/mainwindow.h \
               src/utils.h \
               src/fourcchelper.h \
               src/hexeditor.h \
               src/thirdparty/qhexedit2/qhexedit.h \
               src/thirdparty/qhexedit2/chunks.h \
               src/thirdparty/qhexedit2/commands.h

FORMS       += ui/mainwindow.ui \
               ui/fourcchelper.ui \
               ui/hexeditor.ui

RESOURCES   += resources/resources.qrc

# OS icons (Mac and Windows)
ICON         = resources/app/icon.icns
RC_ICONS     = resources/app/icon.ico

# minivideo library
INCLUDEPATH += ../minivideo/src
QMAKE_LIBDIR+= ../minivideo/build
LIBS        += -L../minivideo/build -lminivideo

# Mac OS X target
unix:macx {
    # Force RPATH to look into the 'Frameworks' dir? Doesn't really seems to work...
    #QMAKE_RPATHDIR += @executable_path/../Frameworks

    # Force Qt to use a particular SDK version
    #QMAKE_MAC_SDK = macosx10.11
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    # Copy libraries into the package
    QT_DIR = /usr/local/lib/
    FW_DIR = build/$${TARGET}.app/Contents/Frameworks
    QMAKE_POST_LINK += (mkdir -p $${FW_DIR})
    QMAKE_POST_LINK += && (cp ../minivideo/build/libminivideo.dylib $${FW_DIR})
    QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtCore.framework/ ]; then cp -R $${QT_DIR}/QtCore.framework $${FW_DIR}; fi)
    QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtGui.framework/ ]; then cp -R $${QT_DIR}/QtGui.framework $${FW_DIR}; fi)
    QMAKE_POST_LINK += && (if [ ! -d $${FW_DIR}/QtWidgets.framework/ ]; then cp -R $${QT_DIR}/QtWidgets.framework $${FW_DIR}; fi)

    # Use bundled libraries (rewrite rpaths)
    APP = build/$${TARGET}.app/Contents/MacOS/$${TARGET}
    QMAKE_POST_LINK += && (install_name_tool -change $${PWD}/build/libminivideo.dylib @executable_path/../Frameworks/libminivideo.dylib $${APP})
    QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore $${APP})
    QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui $${APP})
    QMAKE_POST_LINK += && (install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets $${APP})
}
