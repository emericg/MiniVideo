cmake_minimum_required(VERSION 3.20)

# MiniVideo "library" build system / CMake edition
################################################################################

project(minivideo)

set(minivideo_VERSION_MAJOR 0)
set(minivideo_VERSION_MINOR 15)
set(minivideo_VERSION_PATCH 0)
set(minivideo_VERSION_STRING ${minivideo_VERSION_MAJOR}.${minivideo_VERSION_MINOR}.${minivideo_VERSION_PATCH})

# Build settings
################################################################################

option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_SHARED_LIBS "Build shared library" ON)

set(ENABLE_COLORS 1)            # Toggle colored terminal output
set(ENABLE_MEMFD 1)             # Enable memfd support (for Linux kernels 3.17+)

set(ENABLE_STB_IMAGE 1)         # Toggle internal stb_image library for bmp/png/tga/... import
set(ENABLE_STB_IMAGE_WRITE 1)   # Toggle internal stb_image_write library for bmp/png/tga export
set(ENABLE_PNG 0)               # Toggle external libpng support
set(ENABLE_JPEG 0)              # Toggle external libjpeg support
set(ENABLE_WEBP 0)              # Toggle external libwebp support
set(ENABLE_AVIF 0)              # Toggle external libavif support

# Build dependencies
################################################################################

# custom memfd module
if(ENABLE_MEMFD)
    SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
    find_package(MemFD) # FindMemFD.cmake

    if(MEMFD_FOUND)
        message(STATUS "* memfd enabled")
    else()
        message(WARNING "> memfd not found! Disabling MEMFD support.")
        set(ENABLE_MEMFD 0)
    endif()
endif()

# custom libavif module
if(ENABLE_AVIF)
    SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
    find_package(AVIF) # FindAVIF.cmake

    if(AVIF_FOUND)
        message(STATUS "* libavif enabled")
    else()
        message(WARNING "> libavif not found! Disabling AVIF support.")
        set(ENABLE_AVIF 0)
    endif()
endif()

# custom libwebp module
if(ENABLE_WEBP)
    SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
    find_package(WebP) # FindWebP.cmake

    if(WEBP_FOUND)
        message(STATUS "* libwebp enabled")
    else()
        message(WARNING "> libwebp not found! Disabling WEBP support.")
        set(ENABLE_WEBP 0)
    endif()
endif()

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

# Doxygen OPTIONAL module
#find_package(Doxygen) # FindDoxygen.cmake

if(DOXYGEN_FOUND)
    message(STATUS "* Doxygen found, you can use it to generate a nice API documentation!")
endif()


# Build configuration
################################################################################

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect build type (default is "Release")
# Change it using "-DCMAKE_BUILD_TYPE= Debug / Release / RelWithDebInfo / MinSizeRel"
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_DEBUG 1)
    set(CMAKE_VERBOSE_MAKEFILE on)
    message(WARNING "> This is a DEBUG build. You can change this using -DCMAKE_BUILD_TYPE=Release")
else()
    set(ENABLE_DEBUG 0)
    message(STATUS "> This is a RELEASE build.")
endif()

# Add per-compiler custom rules
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")

    message(STATUS "Building with Clang compiler")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2")
    endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")

    message(STATUS "Building with Intel C++ compiler")

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")

    message(STATUS "Building with GCC compiler")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-missing-field-initializers")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2")
    endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

    message(STATUS "Building with Visual Studio C++ compiler")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

endif()


# Platform specific build configuration
################################################################################

# Linux specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    message(STATUS "Building on Linux plateform")
    add_definitions(-D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lm -Wl,-z,now -Wl,-z,relro")

endif()

# macOS specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    message(STATUS "Building on macOS plateform")
    add_definitions(-D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lm")

endif()

# Windows specific stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    message(STATUS "Building on Windows plateform")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)

    # Turn on the ability to create folders to organize projects (.vcproj)
    # It creates "CMakePredefinedTargets" folder by default and adds CMake
    # defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    include(GenerateExportHeader)

    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lm -Wl,-no-undefined -Wl,--enable-runtime-pseudo-reloc")

endif()


# Source files
################################################################################

# Generate "minivideo_settings.h" file from a template
configure_file("${CMAKE_SOURCE_DIR}/src/minivideo_settings.h.in"
               "${CMAKE_SOURCE_DIR}/src/minivideo_settings.h")

# MiniVideo source code
set(minivideo_SRC
    src/minivideo.cpp
        src/minivideo_settings.h src/minivideo_typedef.h
        src/minivideo_mediafile.h src/minivideo_mediastream.h
        src/minivideo_codecs.cpp src/minivideo_codecs_private_struct.h
        src/minivideo_containers.cpp src/utils.cpp src/minivideo_avutils.h
        src/minivideo_fourcc.cpp src/minivideo_twocc.cpp src/minivideo_uuid.cpp
    src/minitraces.cpp src/minitraces_conf.h
    src/bitstream.cpp
        src/bitstream_utils.cpp src/bitstream_map.cpp
    src/import.cpp
    src/export.cpp
        src/export_utils.cpp
        src/thirdparty/stb_image_write.h
    src/demuxer/idr_filter.cpp
    src/demuxer/xml_mapper.cpp
    src/demuxer/mp4/mp4_struct.h src/demuxer/mp4/mp4.cpp src/demuxer/mp4/mp4_box.cpp
        src/demuxer/mp4/mp4_stbl.cpp src/demuxer/mp4/mp4_stsd.cpp src/demuxer/mp4/mp4_esds.cpp src/demuxer/mp4/mp4_meta.cpp src/demuxer/mp4/mp4_picture.cpp src/demuxer/mp4/mp4_gopro.cpp
        src/demuxer/mp4/mp4_virb.cpp src/demuxer/mp4/mp4_spatial.cpp src/demuxer/mp4/mp4_convert.cpp
    src/demuxer/mp3/mp3_struct.h src/demuxer/mp3/mp3.cpp
    src/demuxer/mkv/mkv_struct.h src/demuxer/mkv/mkv.cpp src/demuxer/mkv/ebml.cpp
        src/demuxer/mkv/mkv_tracks.cpp src/demuxer/mkv/mkv_cluster.cpp src/demuxer/mkv/mkv_codec.cpp src/demuxer/mkv/mkv_convert.cpp
    src/demuxer/esparser/esparser.cpp
    src/demuxer/mpeg/pes/pes_struct.h src/demuxer/mpeg/pes/pes.cpp
        src/demuxer/mpeg/descriptor_struct.h
        src/demuxer/mpeg/ps/ps_struct.h src/demuxer/mpeg/ps/ps.cpp
        src/demuxer/mpeg/ts/ts_struct.h src/demuxer/mpeg/ts/ts.cpp
    src/demuxer/riff/riff_struct.h src/demuxer/riff/riff.cpp src/demuxer/riff/riff_common.cpp
        src/demuxer/avi/avi_struct.h src/demuxer/avi/avi.cpp src/demuxer/avi/avi_convert.cpp
        src/demuxer/wave/wave_struct.h src/demuxer/wave/wave.cpp src/demuxer/wave/wave_convert.cpp
    src/demuxer/asf/asf_struct.h src/demuxer/asf/asf.cpp src/demuxer/asf/asf_object.cpp src/demuxer/asf/asf_convert.cpp
    src/muxer/muxer.cpp
        src/muxer/pes_packetizer.cpp
    src/depacketizer/depack_struct.h src/depacketizer/depack.cpp
        src/depacketizer/h264/depack_h264.cpp
        src/depacketizer/h265/depack_h265.cpp
        src/depacketizer/h266/depack_h266.cpp
    src/decoder/h264/h264.cpp
        src/decoder/h264/h264_nalu.cpp
        src/decoder/h264/h264_parameterset.cpp
        src/decoder/h264/h264_parameterset_struct.h
        src/decoder/h264/h264_decodingcontext.h
        src/decoder/h264/h264_expgolomb.cpp
        src/decoder/h264/h264_cavlc.cpp
        src/decoder/h264/h264_cavlc_tables.h
        src/decoder/h264/h264_cabac.cpp
        src/decoder/h264/h264_cabac_tables.h
        src/decoder/h264/h264_slice.cpp
        src/decoder/h264/h264_slice_struct.h
        src/decoder/h264/h264_macroblock.cpp
        src/decoder/h264/h264_macroblock_utils.cpp
        src/decoder/h264/h264_macroblock_struct.h
        src/decoder/h264/h264_intra_prediction.cpp
        src/decoder/h264/h264_inter_prediction.cpp
        src/decoder/h264/h264_spatial.cpp
        src/decoder/h264/h264_transform.cpp
    src/decoder/h265/h265_nalu.cpp
        src/decoder/h265/h265_expgolomb.cpp
        src/decoder/h265/h265_parameterset.cpp
        src/decoder/h265/h265_parameterset_struct.h
    src/decoder/h266/h266_nalu.cpp
        src/decoder/h266/h266_parameterset.cpp
        src/decoder/h266/h266_parameterset_struct.h
)

include_directories(src/)

# MiniVideo public headers
set(minivideo_PUBLIC_HEADERS
    src/minivideo.h src/minivideo_export.h
    src/minivideo_avutils.h
    src/minivideo_codecs.h src/minivideo_containers.h
    src/minivideo_fourcc.h src/minivideo_twocc.h src/minivideo_uuid.h
    src/minivideo_mediafile.h src/minivideo_mediastream.h src/minivideo_metadata_vendors.h
    src/minivideo_settings.h src/minivideo_typedef.h # internals?
    src/minivideo_codecs_private_struct.h
    #src/decoder/h264/h264_parameterset_struct.h
    #src/decoder/h265/h265_parameterset_struct.h
    #src/decoder/h266/h266_parameterset_struct.h
)

# Build
################################################################################

# Third party libraries
include_directories(${WEBP_INCLUDE_DIR} ${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR})
set(EXTRA_LIBS ${WEBP_LIBRARY} ${JPEG_LIBRARY} ${PNG_LIBRARY})

# Build library
if (BUILD_SHARED_LIBS)

    message("** Building shared library")
    add_library(minivideo_shared SHARED ${minivideo_SRC})
    target_link_libraries(minivideo_shared ${EXTRA_LIBS})
    set_target_properties(minivideo_shared PROPERTIES OUTPUT_NAME minivideo)
    set_target_properties(minivideo_shared PROPERTIES VERSION ${minivideo_VERSION_STRING} SOVERSION ${minivideo_VERSION_MAJOR})

    if(WIN32)
        message("* Generating symbols export header")
        GENERATE_EXPORT_HEADER(minivideo_shared
                               BASE_NAME minivideo_shared
                               EXPORT_MACRO_NAME minivideo_EXPORT
                               EXPORT_FILE_NAME "${CMAKE_SOURCE_DIR}/src/minivideo_export.h"
                               STATIC_DEFINE minivideo_BUILT_AS_STATIC)
    endif()
endif (BUILD_SHARED_LIBS)

if (BUILD_STATIC_LIBS)
    message("** Building static library")
    add_library(minivideo_static STATIC ${minivideo_SRC})
    target_link_libraries(minivideo_static ${EXTRA_LIBS})
    set_target_properties(minivideo_static PROPERTIES OUTPUT_NAME minivideo)
    set_target_properties(minivideo_static PROPERTIES VERSION ${minivideo_VERSION_STRING} SOVERSION ${minivideo_VERSION_MAJOR})
endif (BUILD_STATIC_LIBS)


# Deployment (optional step, requires root credentials, relative to CMAKE_INSTALL_PREFIX)
################################################################################

# pkg-config integration
set(PROJECT_VERSION "${minivideo_VERSION_MAJOR}.${minivideo_VERSION_MINOR}")
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name")
set(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
set(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
set(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH "Installation prefix for object code libraries" FORCE)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/minivideo CACHE PATH "Installation prefix for header files" FORCE)
configure_file(libminivideo.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libminivideo.pc)

# Install pkgconfig file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libminivideo.pc DESTINATION lib/pkgconfig/)

# Install the library into the system
if (BUILD_SHARED_LIBS)
    install(TARGETS minivideo_shared DESTINATION lib/)
endif (BUILD_SHARED_LIBS)
if (BUILD_STATIC_LIBS)
    install(TARGETS minivideo_static DESTINATION lib/)
endif (BUILD_STATIC_LIBS)

# Install headers
install(FILES ${minivideo_PUBLIC_HEADERS} DESTINATION include/minivideo/)
install(DIRECTORY src/decoder/ DESTINATION include/minivideo/decoder/ FILES_MATCHING PATTERN "*_parameterset_struct.h")
