#-------------------------------------------------
# mini_analyser
# Project created by QtCreator 2014-03-23T12:56:32
#-------------------------------------------------

TARGET       = mini_analyser
TEMPLATE     = app
QT          += core gui widgets

OBJECTS_DIR  = build/
MOC_DIR      = build/
RCC_DIR      = build/
UI_DIR       = build/
DESTDIR      = build/

# mini_analyser files
SOURCES     += src/main.cpp\
               src/mainwindow.cpp
HEADERS     += src/main.h\
               src/mainwindow.h
RESOURCES   += resources/resources.qrc
FORMS       += ui/mainwindow.ui

# minivideo library
INCLUDEPATH += ../minivideo/src
QMAKE_LIBDIR+= ../minivideo/build
LIBS        += -L../minivideo/build -lminivideo
