cmake_minimum_required(VERSION 3.20)

project(MiniVideo VERSION 0.15 LANGUAGES CXX C)

set(minivideo_VERSION_MAJOR 0)
set(minivideo_VERSION_MINOR 15)
set(minivideo_VERSION_PATCH 0)
set(minivideo_VERSION_STRING ${minivideo_VERSION_MAJOR}.${minivideo_VERSION_MINOR}.${minivideo_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################

message(STATUS ">>> BUILDING ${PROJECT_NAME} v${PROJECT_VERSION} >>>")
message(STATUS "[CMAKE] CMAKE_VERSION               = ${CMAKE_VERSION}")
message(STATUS "[CMAKE] CMAKE_PREFIX_PATH           = ${CMAKE_PREFIX_PATH}")
message(STATUS "[BUILD] QT_HOST_PATH                = ${QT_HOST_PATH}")
message(STATUS "[BUILD] TOOLCHAIN_FILE              = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[BUILD] BUILD_TYPE                  = ${CMAKE_BUILD_TYPE}")
message(STATUS "[SYSTEM] (HOST) SYSTEM NAME         = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (HOST) SYSTEM VERSION      = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (HOST) SYSTEM PROCESSOR    = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM NAME       = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM VERSION    = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM PROCESSOR  = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_NAME    = ${CMAKE_PROJECT_NAME}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_VERSION = ${CMAKE_PROJECT_VERSION}")
message(STATUS "[TOP PROJECT] CMAKE_SOURCE_DIR      = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] PROJECT_NAME              = ${PROJECT_NAME}")
message(STATUS "[PROJECT] PROJECT_VERSION           = ${PROJECT_VERSION}")
message(STATUS "[PROJECT] PROJECT_SOURCE_DIR        = ${PROJECT_SOURCE_DIR}")
message(STATUS "[PROJECT] CMAKE_CURRENT_SOURCE_DIR  = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "--------------------------------------------------------------")

# Build options ################################################################

option(MINIVIDEO_BUILD_SHARED   "Build shared library"                          ON)
option(MINIVIDEO_BUILD_STATIC   "Build static library"                          ON)
option(MINIVIDEO_INSTALL        "Generate install targets"                      ON)

option(ENABLE_COLORS            "Toggle colored terminal output"                ON)
option(ENABLE_MEMFD             "Enable memfd support (for Linux kernels)"      ON)

option(ENABLE_STB_IMAGE_WRITE   "Toggle internal stb_image_write library for bmp/png/tga export" ON)
option(ENABLE_AVIF              "Toggle external libavif support"               OFF)
option(ENABLE_WEBP              "Toggle external libwebp support"               OFF)
option(ENABLE_JPEG              "Toggle external libjpeg support"               OFF)
option(ENABLE_JPEGXL            "Toggle external libjxl support"                OFF)
option(ENABLE_PNG               "Toggle external libpng support"                OFF)

# Project files ################################################################

set(minivideo_SOURCES
    src/minivideo.cpp
        src/minivideo_typedef.h
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
    src/demuxer/mp4/mp4_struct.h src/demuxer/mp4/mp4.cpp src/demuxer/mp4/mp4_box.cpp src/demuxer/mp4/mp4_convert.cpp
        src/demuxer/mp4/mp4_stbl.cpp src/demuxer/mp4/mp4_stsd.cpp src/demuxer/mp4/mp4_esds.cpp src/demuxer/mp4/mp4_meta.cpp
        src/demuxer/mp4/mp4_picture.cpp src/demuxer/mp4/mp4_gopro.cpp src/demuxer/mp4/mp4_virb.cpp
        src/demuxer/mp4/mp4_spatial.cpp
    src/demuxer/mp3/mp3_struct.h src/demuxer/mp3/mp3.cpp
    src/demuxer/mkv/mkv_struct.h src/demuxer/mkv/mkv.cpp src/demuxer/mkv/ebml.cpp src/demuxer/mkv/mkv_convert.cpp
        src/demuxer/mkv/mkv_tracks.cpp src/demuxer/mkv/mkv_cluster.cpp src/demuxer/mkv/mkv_codec.cpp
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
    src/depacketizer/depack_struct.h src/depacketizer/depack.h src/depacketizer/depack.cpp
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

set(minivideo_PUBLIC_HEADERS
    src/minivideo.h
    src/minivideo_typedef.h # internals?
    src/minivideo_avutils.h src/minivideo_codecs_private_struct.h
    src/minivideo_codecs.h src/minivideo_containers.h
    src/minivideo_fourcc.h src/minivideo_twocc.h src/minivideo_uuid.h
    src/minivideo_mediafile.h src/minivideo_mediastream.h src/minivideo_metadata_vendors.h
    src/bitstream.h src/bitstream_map.h
    src/depacketizer/depack.h src/depacketizer/depack_struct.h
    src/decoder/h264/h264_parameterset_struct.h
    src/decoder/h265/h265_parameterset_struct.h
    src/decoder/h266/h266_parameterset_struct.h
)

# Internal settings file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/minivideo_settings.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_settings.h"
               @ONLY
)

# Include our custom cmake module files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Public header handling #######################################################

# Selected headers are put in a common directory, exported as <minivideo/minivideo.h>

set(minivideo_PUBLIC_HEADERS_export "")

foreach(HEADER_SRC ${minivideo_PUBLIC_HEADERS})
    file(RELATIVE_PATH HEADER_DST "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_SRC}")
    set(STAGED "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/${HEADER_DST}")
    get_filename_component(STAGED_DIR "${STAGED}" DIRECTORY)

    add_custom_command(OUTPUT  "${STAGED}"
                       COMMAND  ${CMAKE_COMMAND} -E make_directory "${STAGED_DIR}"
                       COMMAND  ${CMAKE_COMMAND} -E copy_if_different
                               "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_SRC}"
                               "${STAGED}"
                       DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_SRC}"
                       COMMENT "Staging public header: ${HEADER_DST}"
    )

    list(APPEND minivideo_PUBLIC_HEADERS_export "${STAGED}")
endforeach()

add_custom_command(OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_settings.h"
                           "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_export.h"
                   COMMAND  ${CMAKE_COMMAND} -E copy_if_different
                           "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_settings.h"
                           "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_settings.h"
                   COMMAND  ${CMAKE_COMMAND} -E copy_if_different
                           "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_export.h"
                           "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_export.h"
                   DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_settings.h"
                           "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_export.h"
                   COMMENT "Staging generated headers"
)

add_custom_target(minivideo_stage_headers
                  DEPENDS ${minivideo_PUBLIC_HEADERS_export}
                         "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_settings.h"
                         "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo/minivideo_export.h"
)

# TARGET CONFIGURATION MACRO ###################################################

macro(minivideo_configure_target TARGET_NAME)
    # Private: src/ for internal includes
    target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    # Public: staged build-dir structure → consumers get <minivideo/foo.h>
    target_include_directories(${TARGET_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Ensure headers are staged before this target compiles
    add_dependencies(${TARGET_NAME} minivideo_stage_headers)

    # Build-tree vs install-tree include paths
    target_include_directories(${TARGET_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src> # for generated settings.h
        $<INSTALL_INTERFACE:include/minivideo>
    )

    # Math lib on Linux/Unix
    if(UNIX)
        target_link_libraries(${TARGET_NAME} PRIVATE m)
    endif()

    # Math defines on Windows
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_definitions(-D_USE_MATH_DEFINES)
    endif()
endmacro()

# SHARED TARGET ################################################################

if(MINIVIDEO_BUILD_SHARED)
    add_library(minivideo_shared SHARED ${minivideo_SOURCES})
    add_library(MiniVideo::shared ALIAS minivideo_shared)
    add_library(MiniVideo::MiniVideo ALIAS minivideo_shared)

    # libminivideo.so / libminivideo.dll
    set_target_properties(minivideo_shared PROPERTIES
                          OUTPUT_NAME minivideo
                          VERSION ${PROJECT_VERSION}
                          SOVERSION ${PROJECT_VERSION_MAJOR}
                          EXPORT_NAME shared
                          POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(minivideo_shared PRIVATE MINIVIDEO_SHARED_BUILD)
    minivideo_configure_target(minivideo_shared)

    # Generate export header (handles dllexport/dllimport on Windows)
    include(GenerateExportHeader)
    generate_export_header(minivideo_shared
                           BASE_NAME minivideo
                           EXPORT_MACRO_NAME minivideo_EXPORT
                           EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/src/minivideo_export.h")
endif()

# STATIC TARGET ################################################################

if(MINIVIDEO_BUILD_STATIC)
    add_library(minivideo_static STATIC ${minivideo_SOURCES})
    add_library(MiniVideo::static ALIAS minivideo_static)
    #add_library(MiniVideo::MiniVideo ALIAS minivideo_static) # FIXME can't be the same as the shared target

    # libminivideo.a
    set_target_properties(minivideo_static PROPERTIES
                          OUTPUT_NAME minivideo
                          EXPORT_NAME static
    )

    target_compile_definitions(minivideo_static PRIVATE MINIVIDEO_STATIC_BUILD)
    minivideo_configure_target(minivideo_static)
endif()

# Deployment steps #############################################################

# Register targets into the export set
# This must happen unconditionally so export() below always finds the set,
# whether we are the top-level project or a subproject/submodule.

include(GNUInstallDirs)

set(INSTALL_TARGETS "")

if(MINIVIDEO_BUILD_SHARED)
    list(APPEND INSTALL_TARGETS minivideo_shared)
endif()
if(MINIVIDEO_BUILD_STATIC)
    list(APPEND INSTALL_TARGETS minivideo_static)
endif()

install(TARGETS ${INSTALL_TARGETS}
        EXPORT   MiniVideoTargets # populates the export set, always
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/minivideo"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Export build-tree targets (used by super project, no install needed)
export(EXPORT MiniVideoTargets
       FILE      "${CMAKE_CURRENT_BINARY_DIR}/MiniVideoTargets.cmake"
       NAMESPACE MiniVideo::
)

# Deployment steps (for installation) ##########################################

if(MINIVIDEO_INSTALL)
    include(CMakePackageConfigHelpers)

    # CMake package files
    install(EXPORT MiniVideoTargets
            FILE MiniVideoTargets.cmake
            NAMESPACE MiniVideo::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MiniVideo"
    )

    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/MiniVideoConfigVersion.cmake"
                                     VERSION          ${PROJECT_VERSION}
                                     COMPATIBILITY    SameMajorVersion
    )

    configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MiniVideoConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/MiniVideoConfig.cmake"
                                  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MiniVideo"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/MiniVideoConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/MiniVideoConfigVersion.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MiniVideo"
    )

    # Also install the Find*.cmake modules so consumers can reuse them
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MiniVideo/modules"
    )

    # pkg-config
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libminivideo.pc.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/libminivideo.pc"
                   @ONLY
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libminivideo.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    )
endif()
