#-------------------------------------------------------------------------------
# MiniVideo library build system / qmake edition
# This file allows you to build shared & static library of MiniVideo
#-------------------------------------------------------------------------------

TARGET       = minivideo
VERSION      = 0.15.0

TEMPLATE     = lib
CONFIG      += c++14 shared_and_static

# minivideo files --------------------------------------------------------------

INCLUDEPATH += $${PWD}/src/

SOURCES = $$files(src/*.cpp, true)
HEADERS = $$files(src/*.h, true)

INCLUDEPATH += $${PWD}/src/

# build artifacts --------------------------------------------------------------

OBJECTS_DIR  = build/$${QT_ARCH}/

DESTDIR      = bin/$${QT_ARCH}/

# build settings ---------------------------------------------------------------

VER_MAJ = 0
VER_MIN = 15
VER_PAT = 0

# build settings and version are usually set by the CMake build system.
# we force MINIVIDEO_SETTINGS_H to make sure we don't use the CMake settings file.
DEFINES += MINIVIDEO_SETTINGS_H

DEFINES     += minivideo_VERSION_MAJOR=0
DEFINES     += minivideo_VERSION_MINOR=15
DEFINES     += minivideo_VERSION_PATCH=0

CONFIG(debug, debug|release) { DEFINES += ENABLE_DEBUG=1 }
DEFINES += ENABLE_COLORS=1
DEFINES += ENABLE_STB_IMAGE=0
DEFINES += ENABLE_STB_IMAGE_WRITE=1
DEFINES += ENABLE_AVIF=0
DEFINES += ENABLE_WEBP=0
DEFINES += ENABLE_JPEG=0
DEFINES += ENABLE_PNG=0

# build configuration ----------------------------------------------------------

unix {
    QMAKE_CXXFLAGS += -fPIC
    QMAKE_CXXFLAGS += -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
    QMAKE_CXXFLAGS_RELEASE += -O2
    QMAKE_CXXFLAGS_RELEASE += -fstack-protector-strong -D_FORTIFY_SOURCE=2

    QMAKE_CXXFLAGS += -Wall -Wextra -Wshadow
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable
    COMPILER_BASENAME = $$basename(QMAKE_CXX)
    contains(COMPILER_BASENAME, "g++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-but-set-variable } # GCC only
    contains(COMPILER_BASENAME, "clang++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-private-field } # Clang only

    # Enables AddressSanitizer
    #QMAKE_CXXFLAGS += -fsanitize=address,undefined,pointer-compare,pointer-subtract -fno-omit-frame-pointer
    #QMAKE_LFLAGS += -fsanitize=address,undefined,pointer-compare,pointer-subtract
}

linux {
    DEFINES += ENABLE_MEMFD=1 # Enables memfd

    # Linker flags
    QMAKE_LFLAGS += -lm -Wl,-z,now -Wl,-z,relro
}

macx {
    # Target architecture(s)
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    # Target OS
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

    # Linker flags
    QMAKE_LFLAGS += -lm
}

win32 {
    DEFINES -= UNICODE
    #DEFINES += _MINIVIDEO_BUILD
    #DEFINES += minivideo_EXPORT=__declspec(dllimport)
    DEFINES += _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES

    # Linker flags
    QMAKE_LFLAGS += -lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc
}

# minivideo installation -------------------------------------------------------

unix {
    isEmpty(PREFIX) { PREFIX = /usr/local }

    library.files   += $${DESTDIR}/libminivideo.so
    library.path     = $${PREFIX}/lib/
    library_links.extra+= ln -sf libminivideo.so $${PREFIX}/lib/libminivideo.so.$${VER_MAJ};
    library_links.extra+= ln -sf libminivideo.so $${PREFIX}/lib/libminivideo.so.$${VER_MAJ}.$${VER_MIN};
    library_links.extra+= ln -sf libminivideo.so $${PREFIX}/lib/libminivideo.so.$${VER_MAJ}.$${VER_MIN}.$${VER_PAT};
    library_links.path  = $${PREFIX}/lib/

    headers.files   += $${PWD}/src/minivideo*.h
    headers.path     = $${PREFIX}/include/minivideo/
    headers_h264.files += $${PWD}/src/decoder/h264/h264_parameterset_struct.h
    headers_h264.path   = $${PREFIX}/include/minivideo/decoder/h264/
    headers_h265.files += $${PWD}/src/decoder/h265/h265_parameterset_struct.h
    headers_h265.path   = $${PREFIX}/include/minivideo/decoder/h265/
    headers_h266.files += $${PWD}/src/decoder/h266/h266_parameterset_struct.h
    headers_h266.path   = $${PREFIX}/include/minivideo/decoder/h266/

    INSTALLS += library library_links headers headers_h264 headers_h265 headers_h266
}
