/*!
 * COPYRIGHT (C) 2010-2016 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
 *
 * MiniVideo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniVideo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiniVideo.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      minivideo.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// Library public header
#include "minivideo.h"

// Library privates headers
#include "minitraces.h"
#include "typedef.h"
#include "import.h"

// Demuxers
#include "demuxer/avi/avi.h"
#include "demuxer/mkv/mkv.h"
#include "demuxer/mp4/mp4.h"
#include "demuxer/mp3/mp3.h"
#include "demuxer/wave/wave.h"
#include "demuxer/mpeg/ps/ps.h"

#include "demuxer/bruteforce/bruteforce.h"
#include "demuxer/filter.h"

// Decoder
#include "decoder/h264/h264.h"

// Muxer
#include "muxer/muxer.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void minivideo_print_infos(void)
{
    printf(BLD_GREEN "minivideo_print_infos()\n" CLR_RESET);
    printf("* Library version '%d.%d-%d'\n", minivideo_VERSION_MAJOR,
                                             minivideo_VERSION_MINOR,
                                             minivideo_VERSION_PATCH);

#if defined(__ICC) || defined(__INTEL_COMPILER)
    printf("* Library built with ICC '%d / %s'\n", __INTEL_COMPILER, __INTEL_COMPILER_BUILD_DATE);
#elif defined(__clang__)
    printf("* Library built with CLANG '%d.%d'\n", __clang_major__, __clang_minor__);
#elif defined(__GNUC__) || defined(__GNUG__)
    printf("* Library built with GCC '%d.%d.%d'\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

    printf("* Library built on '%s, %s'\n", __DATE__ , __TIME__);

#if ENABLE_DEBUG
    printf("* DEBUG traces are " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* DEBUG traces are " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_COLORS
    printf("* COLORS are " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* COLORS are OFF\n");
#endif

#if ENABLE_STDINT
    printf("* C99 integer types support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* C99 integer types support is " BLD_RED "OFF\n" CLR_RESET);
    printf("> Integer types support will be emulated\n");
#endif

#if ENABLE_STDBOOL
    printf("* C99 boolean support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* C99 boolean support is " BLD_RED "OFF\n" CLR_RESET);
    printf("> Boolean support will be emulated\n");
#endif

#if ENABLE_STDALIGN
    printf("* C11 memory alignement support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* C11 memory alignement support is " BLD_RED "OFF\n" CLR_RESET);
    printf("> Memory alignement support will be emulated\n");
#endif

#if ENABLE_JPEG
    printf("* EXTERNAL JPEG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL JPEG support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_PNG
    printf("* EXTERNAL PNG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL PNG support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_STBIMWRITE
    printf("* STB_IMAGE_WRITE support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* STB_IMAGE_WRITE support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_DEBUG
    printf("* MiniVideo library tracing test:\n");
    TRACE_ERROR(MAIN, "TEST error\n");
    TRACE_WARNING(MAIN, "TEST warning\n");
    TRACE_INFO(MAIN, "TEST info\n");
    TRACE_1(MAIN, "TEST lvl 1\n");
    TRACE_2(MAIN, "TEST lvl 2\n");
    TRACE_3(MAIN, "TEST lvl 3\n\n");
#endif
}

/* ************************************************************************** */

void minivideo_get_infos(int *minivideo_major, int *minivideo_minor, int *minivideo_patch,
                         const char **minivideo_builddate, const char **minivideo_buildtime)
{
    if (minivideo_major && minivideo_minor && minivideo_patch)
    {
        *minivideo_major = minivideo_VERSION_MAJOR;
        *minivideo_minor = minivideo_VERSION_MINOR;
        *minivideo_patch = minivideo_VERSION_PATCH;
    }

    if (minivideo_builddate && minivideo_buildtime)
    {
        *minivideo_builddate = __DATE__;
        *minivideo_buildtime = __TIME__;
    }
}

/* ************************************************************************** */

int minivideo_endianness(void)
{
    int endianness = -1;
    int i = 1;
    char *p = (char *)&i;

#if ENABLE_DEBUG
    printf(BLD_GREEN "minivideo_endianness()\n" CLR_RESET);

    if (p[0] == 1)
    {
        printf("* ENDIANNESS is set to LITTLE_ENDIAN\n");
        endianness = LITTLE_ENDIAN; // 1234
    }
    else
    {
        printf("* ENDIANNESS is set to BIG_ENDIAN\n");
        endianness = BIG_ENDIAN; // 4321
    }
#else
    if (p[0] == 1)
    {
        endianness = LITTLE_ENDIAN; // 1234
    }
    else
    {
        endianness = BIG_ENDIAN; // 4321
    }
#endif // ENABLE_DEBUG

    return endianness;
}

/* ************************************************************************** */

int minivideo_open(const char *input_filepath, MediaFile_t **input_media)
{
    return import_fileOpen(input_filepath, input_media);
}

/* ************************************************************************** */

int minivideo_parse(MediaFile_t *input_media,
                    const bool extract_audio, const bool extract_video, const bool extract_subtitles)
{
    int retcode = FAILURE;

    if (input_media == NULL)
    {
        TRACE_ERROR(MAIN, "Unable to parse NULL MediaFile_t struct!\n");
    }
    else if (input_media->file_size == 0)
    {
        TRACE_ERROR(MAIN, "Unable to parse emtpy file!\n");
    }
    else
    {
        // Start container parsing
        switch (input_media->container)
        {
            case CONTAINER_UNKNOWN:
                TRACE_ERROR(MAIN, "Unwknown container file format: unable to parse this file!\n");
                break;
            case CONTAINER_AVI:
                retcode = avi_fileParse(input_media);
                break;
            case CONTAINER_MP4:
                retcode = mp4_fileParse(input_media);
                break;
            case CONTAINER_WAVE:
                retcode = wave_fileParse(input_media);
                break;
            case CONTAINER_MPEG_PS:
                retcode = ps_fileParse(input_media);
                break;
            case CONTAINER_ES:
                retcode = bruteforce_fileParse(input_media, CODEC_H264);
                break;
            case CONTAINER_ES_MP3:
                retcode = mp3_fileParse(input_media);
                break;
            default:
                TRACE_ERROR(MAIN, "Unable to parse given container format '%s': no parser available!\n",
                            getContainerString(input_media->container, 0));
                break;
        }

        // Compute some additional metadatas
        computeCodecs(input_media);
        computeAspectRatios(input_media);
        computeSamplesDatas(input_media);
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_decode(MediaFile_t *input_media,
                     const char *output_directory,
                     const int picture_format,
                     const int picture_quality,
                     const int picture_number,
                     const int picture_extractionmode)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_decode()\n" CLR_RESET);

    if (input_media != NULL)
    {
        // IDR frame filtering
        int picture_number_filtered = idr_filtering(&input_media->tracks_video[0],
                                                    picture_number, picture_extractionmode);

        if (picture_number_filtered == 0)
        {
            TRACE_ERROR(MAIN, "No picture to decode after filtering!\n");
        }
        else
        {
            // Print status
            //import_fileStatus(input_media);

            // Start video decoding
            switch (input_media->tracks_video[0]->stream_codec)
            {
                case CODEC_UNKNOWN:
                    TRACE_ERROR(MAIN, "Unknown video format: unable to decode this file!\n");
                    break;
                case CODEC_H264:
                    retcode = h264_decode(input_media, output_directory, picture_format, picture_quality, picture_number_filtered, picture_extractionmode);
                    break;
                default:
                    TRACE_ERROR(MAIN, "Unable to decode given file format '%s': no decoder available!\n",
                                getCodecString(stream_VIDEO, input_media->tracks_video[0]->stream_codec, true));
                    break;
            }
        }
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to start decoding because of an empty MediaFile_t structure! Parsing failed?\n");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_extract(MediaFile_t *input_media,
                      const char *output_directory,
                      const bool extract_audio,
                      const bool extract_video,
                      const bool extract_subtitles,
                      const int output_format)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_extract()\n" CLR_RESET);

    if (input_media != NULL && output_directory != NULL)
    {
        // Print status
        //import_fileStatus(input_media);

        // Export audio and video PES stream
        if (extract_audio)
            retcode = muxer_export_samples(input_media, input_media->tracks_audio[0], output_format);

        if (extract_video)
            retcode = muxer_export_samples(input_media, input_media->tracks_video[0], output_format);

        if (extract_subtitles)
            retcode = muxer_export_samples(input_media, input_media->tracks_subt[0], output_format);
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to extract from a NULL MediaFile_t struct!\n");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_close(MediaFile_t **input_media)
{
    return import_fileClose(input_media);
}

/* ************************************************************************** */
