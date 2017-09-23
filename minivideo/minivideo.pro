#-------------------------------------------------------------------------------
# MiniVideo "library" build system / qmake edition
# This file allows you to build shared & static library of MiniVideo
#-------------------------------------------------------------------------------

TARGET       = minivideo
TEMPLATE     = lib
CONFIG      += shared_and_static build_all
CONFIG      -= qt

VERSION      = 0.9.1
DEFINES     += minivideo_VERSION_MAJOR=0
DEFINES     += minivideo_VERSION_MINOR=9
DEFINES     += minivideo_VERSION_PATCH=1

# build artifacts
OBJECTS_DIR  = build/artifacts
DESTDIR      = build/

# build settings are usually set by the CMake build system.
# default settings have been set in "minivideo_settings.h", but if you wish to
# change them you'll need to do it manually by editing this file

# build configuration
unix {
    QMAKE_CFLAGS += -std=c99 -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
    QMAKE_CFLAGS += -Wall -Wextra -Wshadow -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable
    QMAKE_CFLAGS += -fPIC
}

linux {
    QMAKE_LFLAGS += -lm -Wl,-z,now -Wl,-z,relro
}

macx {
    QMAKE_LFLAGS += -lm
}

win32 {
    QMAKE_LFLAGS += -lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc
}

# minivideo files
SOURCES += src/*.c \
           src/decoder/h264/*.c \
           src/demuxer/*.c \
           src/demuxer/asf/*.c \
           src/demuxer/avi/*.c \
           src/demuxer/esparser/*.c \
           src/demuxer/mkv/*.c \
           src/demuxer/mp3/*.c \
           src/demuxer/mp4/*.c \
           src/demuxer/mpeg/pes/*.c \
           src/demuxer/mpeg/ps/*.c \
           src/demuxer/mpeg/ts/*.c \
           src/demuxer/riff/*.c \
           src/demuxer/wave/*.c \
           src/depacketizer/*.c \
           src/depacketizer/h264/*.c \
           src/muxer/*.c \
           src/thirdparty/*.c

HEADERS += src/*.h \
           src/decoder/h264/*.h \
           src/demuxer/*.h \
           src/demuxer/asf/*.h \
           src/demuxer/avi/*.h \
           src/demuxer/esparser/*.h \
           src/demuxer/mkv/*.h \
           src/demuxer/mp3/*.h \
           src/demuxer/mp4/*.h \
           src/demuxer/mpeg/*.h \
           src/demuxer/mpeg/pes/*.h \
           src/demuxer/mpeg/ps/*.h \
           src/demuxer/mpeg/ts/*.h \
           src/demuxer/riff/*.h \
           src/demuxer/wave/*.h \
           src/depacketizer/*.h \
           src/depacketizer/h264/*.h \
           src/muxer/*.h \
           src/thirdparty/*.h
