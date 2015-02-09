# Project informations
###############################################################################

# General
project(minivideo)
cmake_minimum_required(VERSION 2.6)

# Check for valid operating system
IF(UNIX)
    MESSAGE(STATUS "* UNIX operating system found")
ELSE(UNIX)
    MESSAGE(ERROR "* This is an UNIX shared library, non UNIX system are not (yet?) supported!")
ENDIF(UNIX)


# Pass various options to minivideo source code
###############################################################################

SET(minivideo_VERSION_MAJOR 5)
SET(minivideo_VERSION_MINOR 0)
SET(minivideo_VERSION_PATCH 0)

SET(ENABLE_COLORS 1) # "Toggle colored terminal output"
SET(ENABLE_STDINT 1) # "Toggle C99 <stdint.h> usage"
SET(ENABLE_STDBOOL 1) # "Toggle C99 <stdbool.h> usage"

SET(ENABLE_JPEG 1) # "Toggle external libjpeg support"
SET(ENABLE_PNG 1) # "Toggle external libpng support"
SET(ENABLE_STBIMWRITE 1) # "Toggle internal stb_image_write library for bmp/png/tga support"

# libjpeg module
IF(ENABLE_JPEG)
    FIND_PACKAGE(JPEG) # FindJPEG.cmake

    IF(JPEG_FOUND)
        MESSAGE(STATUS "* libjpeg found")
    ELSE(JPEG_FOUND)
        MESSAGE(WARNING "> libjpeg not found! Disabling JPEG support.")
        SET(ENABLE_JPEG 0)
    ENDIF(JPEG_FOUND)
ENDIF(ENABLE_JPEG)

# libpng module
IF(ENABLE_PNG)
    FIND_PACKAGE(PNG) # FindPNG.cmake

    IF(PNG_FOUND)
        MESSAGE(STATUS "* libpng found")
    ELSE(PNG_FOUND)
        MESSAGE(WARNING "> libpng not found! Disabling external PNG support. 'stb_image_write' library will still provide PNG export.")
        SET(ENABLE_PNG 0)
    ENDIF(PNG_FOUND)
ENDIF(ENABLE_PNG)

# Doxygen module
FIND_PACKAGE(Doxygen) # FindDoxygen.cmake

IF(DOXYGEN_FOUND)
    MESSAGE(STATUS "* Doxygen found")
ELSE(DOXYGEN_FOUND)
    MESSAGE(WARNING " > Doxygen not found")
ENDIF(DOXYGEN_FOUND)

# Detect build type. You can change it using "-DCMAKE_BUILD_TYPE="
IF(CMAKE_BUILD_TYPE STREQUAL "Release")

    MESSAGE(STATUS "* This is a RELEASE build. You can change it using -DCMAKE_BUILD_TYPE=Debug.")
    SET(ENABLE_DEBUG 0)
    SET(CMAKE_BUILD_TYPE release)
    # Custom cflags? Use "release mode" and "-O2" for maximum speed.
    SET(CMAKE_C_FLAGS "-O2 -lm -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Wno-unused")

ELSE(CMAKE_BUILD_TYPE STREQUAL "Release")

    MESSAGE("!! * This is a DEBUG build. You can change it using -DCMAKE_BUILD_TYPE=Release.")
    SET(ENABLE_DEBUG 1)
    SET(CMAKE_BUILD_TYPE debug)
    # Custom cflags? Use "debug mode" and "-O0 -g" if you want better support for GDB and Valgrind.
    SET(CMAKE_C_FLAGS "-O0 -g -lm -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -Wextra -Wshadow -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable")

ENDIF(CMAKE_BUILD_TYPE STREQUAL "Release")

# Save options into a "cmake_defines.h" file, using "cmake_defines.h.in" template
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/src/cmake_defines.h.in"
    "${CMAKE_SOURCE_DIR}/src/cmake_defines.h"
)


# Source files
###############################################################################

SET(minivideo_SRC
    src/minivideo.c
        src/cmake_defines.h
        src/typedef.h
        src/utils.c
        src/avcodecs.c
        src/minitraces.c
    src/bitstream.c
        src/bitstream_utils.c
        src/bitstream_map.c
    src/import.c
    src/export.c
        src/export_utils.c
        src/stb_image_write.h
    src/demuxer/filter.c
        src/demuxer/bruteforce/bruteforce.c
        src/demuxer/mpeg/pes/pes.c src/demuxer/mpeg/pes/pes_struct.h
        src/demuxer/mpeg/ps/ps.c src/demuxer/mpeg/ps/ps_struct.h
        src/demuxer/mpeg/ts/ts_struct.h
        src/demuxer/avi/avi.c src/demuxer/avi/avi_struct.h
        src/demuxer/mp4/mp4.c src/demuxer/mp4/mp4_struct.h
        src/demuxer/mkv/mkv.c src/demuxer/mkv/mkv_struct.h src/demuxer/mkv/ebml.c
    src/muxer/muxer.c
        src/muxer/pes_packetizer.c
    src/decoder/h264/h264.c
        src/decoder/h264/h264_decodingcontext.h
        src/decoder/h264/h264_expgolomb.c
        src/decoder/h264/h264_cavlc.c
        src/decoder/h264/h264_cavlc_tables.h
        src/decoder/h264/h264_cabac.c
        src/decoder/h264/h264_cabac_tables.h
        src/decoder/h264/h264_nalu.c
        src/decoder/h264/h264_parameterset.c
        src/decoder/h264/h264_parameterset_struct.h
        src/decoder/h264/h264_slice.c
        src/decoder/h264/h264_slice_struct.h
        src/decoder/h264/h264_macroblock.c
        src/decoder/h264/h264_macroblock_utils.c
        src/decoder/h264/h264_macroblock_struct.h
        src/decoder/h264/h264_intra_prediction.c
        src/decoder/h264/h264_inter_prediction.c
        src/decoder/h264/h264_spatial.c
        src/decoder/h264/h264_transform.c
)


# Build
###############################################################################

# Third party libraries
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR})

# Build library
ADD_LIBRARY(minivideo_shared SHARED ${minivideo_SRC})
TARGET_LINK_LIBRARIES(minivideo_shared ${JPEG_LIBRARY} ${PNG_LIBRARY})
SET_TARGET_PROPERTIES(minivideo_shared PROPERTIES OUTPUT_NAME minivideo)

IF(CMAKE_BUILD_MODE STREQUAL "Static")
    MESSAGE("** Static library will be built")
    ADD_LIBRARY(minivideo_static STATIC ${minivideo_SRC})
    TARGET_LINK_LIBRARIES(minivideo_static ${JPEG_LIBRARY} ${PNG_LIBRARY})
    SET_TARGET_PROPERTIES(minivideo_static PROPERTIES OUTPUT_NAME minivideo)
ENDIF(CMAKE_BUILD_MODE STREQUAL "Static")


# Install the shared library and its header into the system (optional step, requires root credentials)
###############################################################################

INSTALL(TARGETS minivideo_shared DESTINATION lib)
INSTALL(FILES src/minivideo.h DESTINATION include)
