#-------------------------------------------------------------------------------
# MiniVideo "library" build system / qmake edition
# This file allows you to build shared & static library of MiniVideo
#-------------------------------------------------------------------------------

# build config
TARGET       = minivideo
TEMPLATE     = lib
CONFIG      += c++11 shared_and_static
CONFIG      -= qt

# build artifacts
OBJECTS_DIR  = build/artifacts
DESTDIR      = build/

# build settings ---------------------------------------------------------------

# build settings and version are usually set by the CMake build system.
# we force MINIVIDEO_SETTINGS_H to make sure we never use the CMake settings file.
DEFINES += MINIVIDEO_SETTINGS_H

VERSION      = 0.11.0
DEFINES     += minivideo_VERSION_MAJOR=0
DEFINES     += minivideo_VERSION_MINOR=10
DEFINES     += minivideo_VERSION_PATCH=0

CONFIG(debug, debug|release) { DEFINES += ENABLE_DEBUG=1 }
DEFINES += ENABLE_COLORS=1
DEFINES += ENABLE_STBIMWRITE=1
DEFINES += ENABLE_WEBP=0
DEFINES += ENABLE_JPEG=0
DEFINES += ENABLE_PNG=0

# build configuration ----------------------------------------------------------

unix {
    QMAKE_CXXFLAGS += -fPIC
    QMAKE_CXXFLAGS += -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
    QMAKE_CXXFLAGS_RELEASE += -O3
    QMAKE_CXXFLAGS_RELEASE += -fstack-protector-strong -D_FORTIFY_SOURCE=2

    QMAKE_CXXFLAGS += -Wall -Wextra -Wshadow
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable
    COMPILER_BASENAME = $$basename(QMAKE_CXX)
    contains(COMPILER_BASENAME, "g++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-but-set-variable } # GCC only
    contains(COMPILER_BASENAME, "clang++") { QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-private-field } # Clang only

    # Enables AddressSanitizer
    #QMAKE_CXXFLAGS += -fsanitize=address,undefined
    #QMAKE_LFLAGS += -fsanitize=address,undefined
}

linux {
    DEFINES += ENABLE_MEMFD=1 # Enables memfd
    QMAKE_LFLAGS += -lm -Wl,-z,now -Wl,-z,relro
}

macx {
    QMAKE_LFLAGS += -lm
}

win32 {
    VERSION =
    DEFINES -= UNICODE
    DEFINES += minivideo_EXPORT=__declspec(dllimport)
    DEFINES += _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES
    QMAKE_LFLAGS += -lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc
}

# minivideo files --------------------------------------------------------------

SOURCES = $$files(src/*.cpp, true)
HEADERS = $$files(src/*.h, true)

# minivideo installation -------------------------------------------------------

unix {
    isEmpty(PREFIX) { PREFIX = /usr/local }
    library.files   += $${OUT_PWD}/$${DESTDIR}/libminivideo.so
    library.files   += $${OUT_PWD}/$${DESTDIR}/libminivideo.so.*
    library.path     = $${PREFIX}/lib/
    headers.files   += $${OUT_PWD}/src/minivideo*.h
    headers.path     = $${PREFIX}/include/
    INSTALLS += library headers
}
