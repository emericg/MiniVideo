#-------------------------------------------------------------------------------
# MiniVideo "library" build system / qmake edition
# This file allows you to build shared & static library of MiniVideo
#-------------------------------------------------------------------------------

TARGET       = minivideo
TEMPLATE     = lib
CONFIG      += shared_and_static build_all

VERSION      = 0.8.1
DEFINES     += minivideo_VERSION_MAJOR=0
DEFINES     += minivideo_VERSION_MINOR=8
DEFINES     += minivideo_VERSION_PATCH=1

# build artifacts
OBJECTS_DIR  = build/artifacts
DESTDIR      = build/

# build settings
DEFINES += ENABLE_COLORS=1        # Toggle colored terminal output
DEFINES += ENABLE_C99_STDINT=1    # Toggle C99 <stdint.h> usage
DEFINES += ENABLE_C99_STDBOOL=1   # Toggle C99 <stdbool.h> usage
DEFINES += ENABLE_C11_STDALIGN=0  # Toggle C11 <stdalign.h> usage

DEFINES += ENABLE_WEBP=0          # Toggle external libwebp support
DEFINES += ENABLE_JPEG=0          # Toggle external libjpeg support
DEFINES += ENABLE_PNG=0           # Toggle external libpng support
DEFINES += ENABLE_STBIMWRITE=1    # Toggle internal stb_image_write library for bmp/png/tga support

DEFINES += ENABLE_MEMFD=1         # Enable memfd support, for Linux kernels 3.17+

# build configuration
QMAKE_CFLAGS += -std=c99 -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
QMAKE_CFLAGS += -Wall -Wextra -Wshadow -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable

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
