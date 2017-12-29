#-------------------------------------------------------------------------------
# MiniVideo "library" build system / qmake edition
# This file allows you to build shared & static library of MiniVideo
#-------------------------------------------------------------------------------

TARGET       = minivideo
TEMPLATE     = lib
CONFIG      += c++11
CONFIG      += shared_and_static
CONFIG      -= qt

# build artifacts
OBJECTS_DIR  = build/artifacts
DESTDIR      = build/

# build version are usually set by the CMake build system.
VERSION      = 0.10.0
DEFINES     += minivideo_VERSION_MAJOR=0
DEFINES     += minivideo_VERSION_MINOR=10
DEFINES     += minivideo_VERSION_PATCH=0

# build settings are usually set by the CMake build system.
CONFIG(debug, debug|release) { DEFINES += ENABLE_DEBUG=1 }
DEFINES += ENABLE_COLORS=1
DEFINES += ENABLE_STBIMWRITE=1
DEFINES += ENABLE_WEBP=0
DEFINES += ENABLE_JPEG=0
DEFINES += ENABLE_PNG=0

# build configuration
unix {
    QMAKE_CXXFLAGS += -fPIC
    QMAKE_CXXFLAGS += -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
    QMAKE_CXXFLAGS += -Wall -Wextra -Wshadow -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable
}

linux {
    # Enables AddressSanitizer
    #QMAKE_CXXFLAGS += -fsanitize=address,undefined
    #QMAKE_LFLAGS += -fsanitize=address,undefined

    # Enables memfd
    DEFINES += ENABLE_MEMFD=1

    QMAKE_LFLAGS += -lm -Wl,-z,now -Wl,-z,relro
}

macx {
    QMAKE_LFLAGS += -lm
}

win32 {
    QMAKE_LFLAGS += -lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc
}

# minivideo files
SOURCES += src/*.cpp \
           src/decoder/h264/*.cpp \
           src/demuxer/*.cpp \
           src/demuxer/asf/*.cpp \
           src/demuxer/avi/*.cpp \
           src/demuxer/esparser/*.cpp \
           src/demuxer/mkv/*.cpp \
           src/demuxer/mp3/*.cpp \
           src/demuxer/mp4/*.cpp \
           src/demuxer/mpeg/pes/*.cpp \
           src/demuxer/mpeg/ps/*.cpp \
           src/demuxer/mpeg/ts/*.cpp \
           src/demuxer/riff/*.cpp \
           src/demuxer/wave/*.cpp \
           src/depacketizer/*.cpp \
           src/depacketizer/h264/*.cpp \
           src/muxer/*.cpp \
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
