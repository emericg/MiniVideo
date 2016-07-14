# Project informations
###############################################################################
include (GenerateExportHeader)

# General
project(minivideo)
cmake_minimum_required(VERSION 2.6)

# Check for valid operating system
if(UNIX)
    message(STATUS "* UNIX operating system")
elseif(WIN32)
	message(STATUS "* WINDOWS operating system")
else()
    message(WARNING "* This library is supported on UNIX and WINDOWS systems only !")
endif()

# Set policy for interpretation of if() arguments as variables or keywords when unquoted to NEW
# to avoid warnings.
cmake_policy(SET CMP0054 NEW)

# Pass various options to minivideo source code
###############################################################################

set(minivideo_VERSION_MAJOR 0)
set(minivideo_VERSION_MINOR 7)
set(minivideo_VERSION_PATCH 1)

set(ENABLE_COLORS 1) # "Toggle colored terminal output"
set(ENABLE_STDINT 1) # "Toggle C99 <stdint.h> usage"
set(ENABLE_STDBOOL 1) # "Toggle C99 <stdbool.h> usage"
set(ENABLE_STDALIGN 1) # "Toggle C11 <stdalign.h> usage"

set(ENABLE_JPEG 0) # "Toggle external libjpeg support"
set(ENABLE_PNG 0) # "Toggle external libpng support"
set(ENABLE_STBIMWRITE 1) # "Toggle internal stb_image_write library for bmp/png/tga support"


# Dependencies
###############################################################################

# libjpeg module
if(ENABLE_JPEG)
    find_package(JPEG) # FindJPEG.cmake

    if(JPEG_FOUND)
        message(STATUS "* libjpeg enabled")
    else()
        message(WARNING "> libjpeg not found! Disabling JPEG support.")
        set(ENABLE_JPEG 0)
    endif()
endif()

# libpng module
if(ENABLE_PNG)
    find_package(PNG) # FindPNG.cmake

    if(PNG_FOUND)
        message(STATUS "* libpng enabled")
    else()
        message(WARNING "> libpng not found! Disabling external PNG support. 'stb_image_write' library will still provide PNG export.")
        set(ENABLE_PNG 0)
    endif()
endif()

# Doxygen module
find_package(Doxygen) # FindDoxygen.cmake

if(DOXYGEN_FOUND)
    message(STATUS "* Doxygen found, you can use it to generate a nice API documentation")
endif(DOXYGEN_FOUND)

# C numerics library for Unix systems
if(UNIX)
    set(CMAKE_SHARED_LINKER_FLAGS "-lm")
elseif(WIN32)
    set(CMAKE_SHARED_LINKER_FLAGS "-lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc")
endif()
# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build options
###############################################################################

# Generic build flags
set(CMAKE_C_FLAGS "-std=c99 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")

# Detect build type (default is release)
# You can change this using "-DCMAKE_BUILD_TYPE=Debug/Release/RelWithDebInfo/MinSizeRel"
if(CMAKE_BUILD_TYPE STREQUAL "Debug")

    set(ENABLE_DEBUG 1)
    message(WARNING" * This is a DEBUG build. You can change this using -DCMAKE_BUILD_TYPE=Release.")

    # Custom cflags? Use "debug mode" and "-O0 -g" if you want better support for GDB and Valgrind.
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -Wextra -Wshadow")

else()

    set(ENABLE_DEBUG 0)
    message(STATUS "* This is a RELEASE build. You can change this using -DCMAKE_BUILD_TYPE=Debug.")

    # Custom cflags? Use "release mode" and "-O2" for maximum speed.
    set(CMAKE_C_FLAGS_RELEASE "-O2 -Wno-sign-compare")

endif()

# Add per-compiler custom rules
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "* Building with Clang compiler")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "* Building with GCC compiler")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable ")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
    message(STATUS "* Building with Intel C compiler")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    message(STATUS "* Building with Visual Studio C compiler")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "* Building with GCC compiler")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable ")
endif()


# Generate "minivideo_settings.h" file from a template
###############################################################################

configure_file(
    "${CMAKE_SOURCE_DIR}/src/minivideo_settings.h.in"
    "${CMAKE_SOURCE_DIR}/src/minivideo_settings.h"
)


# Source files
###############################################################################

set(minivideo_SRC
    src/minivideo.c
        src/minivideo_settings.h
        src/typedef.h
        src/utils.c
        src/avcodecs.c src/avutils.h
        src/fourcc.c
        src/minitraces.c
        src/minitraces_conf.h
    src/bitstream.c
        src/bitstream_utils.c
        src/bitstream_map.c
        src/bitstream_map_struct.h
    src/import.c
    src/export.c
        src/export_utils.c
        src/stb_image_write.h
    src/demuxer/mp4/mp4.c src/demuxer/mp4/mp4_struct.h
    src/demuxer/mp3/mp3.c src/demuxer/mp3/mp3_struct.h
    src/demuxer/mkv/mkv.c src/demuxer/mkv/mkv_struct.h src/demuxer/mkv/ebml.c
    src/demuxer/esparser/esparser.c
    src/demuxer/filter.c
    src/demuxer/mpeg/pes/pes.c src/demuxer/mpeg/pes/pes_struct.h
        src/demuxer/mpeg/descriptor_struct.h
        src/demuxer/mpeg/ps/ps.c src/demuxer/mpeg/ps/ps_struct.h
        src/demuxer/mpeg/ts/ts.c src/demuxer/mpeg/ts/ts_struct.h
    src/demuxer/riff/riff.c src/demuxer/riff/riff_struct.h
        src/demuxer/avi/avi.c src/demuxer/avi/avi_struct.h
        src/demuxer/wave/wave.c src/demuxer/wave/wave_struct.h
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
#include_directories(${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR})

# Build library
message("** Building shared library")
add_library(minivideo_shared SHARED ${minivideo_SRC})
if(WIN32)
	message("* Generating export headers for DLL")
	GENERATE_EXPORT_HEADER( minivideo_shared
				 BASE_NAME minivideo_shared
				 EXPORT_MACRO_NAME minivideo_EXPORT
				 EXPORT_FILE_NAME minivideo_Export.h
				 STATIC_DEFINE minivideo_BUILT_AS_STATIC
	)
endif()
#target_link_libraries(minivideo_shared ${JPEG_LIBRARY} ${PNG_LIBRARY})
target_link_libraries(minivideo_shared)
set_target_properties(minivideo_shared PROPERTIES OUTPUT_NAME minivideo)

if(CMAKE_BUILD_MODE STREQUAL "Static")
    message("** Building static library")
    add_library(minivideo_static STATIC ${minivideo_SRC})
    #target_link_libraries(minivideo_static ${JPEG_LIBRARY} ${PNG_LIBRARY})
    target_link_libraries(minivideo_static)
    set_target_properties(minivideo_static PROPERTIES OUTPUT_NAME minivideo)
endif(CMAKE_BUILD_MODE STREQUAL "Static")


# Install the shared library and its header into the system (optional step, requires root credentials)
# Relative to $<INSTALL_PREFIX>
###############################################################################

if(UNIX)
    install(TARGETS minivideo_shared DESTINATION lib/)
    install(FILES src/minivideo.h DESTINATION include/)
endif()
