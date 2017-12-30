#-------------------------------------------------------------------------------
# mini_thumbnailer build system
# minivideo library must be built first in order for mini_thumbnailer to work
#-------------------------------------------------------------------------------

TARGET       = mini_thumbnailer
TEMPLATE     = app
CONFIG      += c++11
DESTDIR      = bin/

# build artifacts
OBJECTS_DIR  = build/
MOC_DIR      = build/
RCC_DIR      = build/
UI_DIR       = build/

# mini_thumbnailer files
SOURCES     += src/main.cpp
HEADERS     += src/main.h

# minivideo library
INCLUDEPATH += ../minivideo/src
QMAKE_LIBDIR+= ../minivideo/build
LIBS        += -L../minivideo/build -lminivideo # dynamic linking
#LIBS        += ../minivideo/build/libminivideo.a # static linking

unix {
    QMAKE_CFLAGS += -fPIE

    QMAKE_RPATHDIR += $${PWD}/../minivideo/build
}
