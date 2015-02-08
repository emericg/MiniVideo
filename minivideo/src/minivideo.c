/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
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
 * \date      2014
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Library privates headers
#include "minitraces.h"
#include "typedef.h"
#include "import.h"

// Library public header
#include "minivideo.h"

// demuxers
#include "demuxer/bruteforce/bruteforce.h"
#include "demuxer/mpeg/ps/ps.h"
#include "demuxer/avi/avi.h"
#include "demuxer/mp4/mp4.h"
#include "demuxer/mkv/mkv.h"
#include "demuxer/filter.h"

// muxer
#include "muxer/muxer.h"

// decoder
#include "decoder/h264/h264.h"

/* ************************************************************************** */

void minivideo_infos(void)
{
    printf(BLD_GREEN "\nminivideo_infos()\n" CLR_RESET);
    printf("* Library version '%d.%d-%d'\n", minivideo_VERSION_MAJOR,
                                           minivideo_VERSION_MINOR,
                                           minivideo_VERSION_PATCH);
    printf("* Library built on '%s, %s'\n", __DATE__ , __TIME__);

#if ENABLE_DEBUG
    printf("* DEBUG traces are " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* DEBUG traces are " BLD_RED "OFF\n" CLR_RESET);
#endif /* ENABLE_DEBUG */

#if ENABLE_COLORS
    printf("* COLORS are " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* COLORS are OFF\n" CLR_RESET);
#endif /* ENABLE_COLORS */

#if ENABLE_STDINT
    printf("* C99 integer support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* C99 integer support is " BLD_RED "OFF\n" CLR_RESET);
    printf("* Integer support is emulated\n");
#endif /* ENABLE_STDINT */

#if ENABLE_STDBOOL
    printf("* C99 boolean support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* C99 boolean support is " BLD_RED "OFF\n" CLR_RESET);
    printf("* boolean support is emulated\n");
#endif /* ENABLE_STDBOOL */

#if ENABLE_JPEG
    printf("* EXTERNAL JPEG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL JPEG support is " BLD_RED "OFF\n" CLR_RESET);
#endif /* ENABLE_JPEG */

#if ENABLE_PNG
    printf("* EXTERNAL PNG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL PNG support is " BLD_RED "OFF\n" CLR_RESET);
#endif /* ENABLE_PNG */

#if ENABLE_STBIMWRITE
    printf("* STB_IMAGE_WRITE support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* STB_IMAGE_WRITE support is " BLD_RED "OFF\n" CLR_RESET);
#endif /* ENABLE_STBIMWRITE */

#if ENABLE_DEBUG
    printf("\n* MiniVideo library tracing test:\n");
    TRACE_ERROR(MAIN, "TEST error\n");
    TRACE_WARNING(MAIN, "TEST warning\n");
    TRACE_INFO(MAIN, "TEST info\n");
    TRACE_1(MAIN, "TEST lvl 1\n");
    TRACE_2(MAIN, "TEST lvl 2\n");
    TRACE_3(MAIN, "TEST lvl 3\n\n");
#endif /* ENABLE_DEBUG */
}

/* ************************************************************************** */

int minivideo_endianness(void)
{
    int retcode = -1;
    int i = 1;
    char *p = (char *)&i;

#if ENABLE_DEBUG
    printf(BLD_GREEN "\nminivideo_endianness()\n" CLR_RESET);

    if (p[0] == 1)
    {
        printf("* ENDIANNESS is set to LITTLE_ENDIAN\n");
        retcode = LITTLE_ENDIAN; // 1234
    }
    else
    {
        printf("* ENDIANNESS is set to BIG_ENDIAN\n");
        retcode = BIG_ENDIAN; // 4321
    }
#else
    if (p[0] == 1)
    {
        retcode = LITTLE_ENDIAN; // 1234
    }
    else
    {
        retcode = BIG_ENDIAN; // 4321
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

int minivideo_open(const char *input_filepath, VideoFile_t **input_video)
{
    return import_fileOpen(input_filepath, input_video);
}

/* ************************************************************************** */

int minivideo_parse(VideoFile_t *input_video,
                    const bool extract_audio, const bool extract_video, const bool extract_subtitles)
{
    int retcode = FAILURE;

    if (input_video == NULL)
    {
        TRACE_ERROR(MAIN, "Unable to parse NULL VideoFile_t struct!\n");
    }
    else
    {
        // Start container parsing
        switch (input_video->container)
        {
            case CONTAINER_UNKNOWN:
                TRACE_ERROR(MAIN, "Unwknown container file format: unable to parse this file!\n");
                break;
            case CONTAINER_AVI:
                retcode = avi_fileParse(input_video);
                break;
            case CONTAINER_MP4:
                retcode = mp4_fileParse(input_video);
                break;
            case CONTAINER_MKV:
                retcode = mkv_fileParse(input_video);
                break;
            case CONTAINER_ES:
                retcode = bruteforce_fileParse(input_video, CODEC_H264);
                break;
            default:
                TRACE_ERROR(MAIN, "Unable to parse given container format '%s': no parser available!\n",
                            getContainerString(input_video->container, 0));
                break;
        }
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_decode(VideoFile_t *input_video,
                     const char *output_directory,
                     const int picture_format,
                     const int picture_quality,
                     const int picture_number,
                     const int picture_extractionmode)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_decode()\n" CLR_RESET);

    if (input_video != NULL)
    {
        // IDR frame filtering
        int picture_number_filtered = idr_filtering(&input_video->tracks_video[0],
                                                    picture_number, picture_extractionmode);

        if (picture_number_filtered == 0)
        {
            TRACE_ERROR(MAIN, "No picture to decode after filtering!\n");
        }
        else
        {
            // Print status
            //import_fileStatus(input_video);

            // Start video decoding
            switch (input_video->tracks_video[0]->stream_codec)
            {
                case CODEC_UNKNOWN:
                    TRACE_ERROR(MAIN, "Unknown video format: unable to decode this file!\n");
                    break;
                case CODEC_H264:
                    retcode = h264_decode(input_video, output_directory, picture_format, picture_quality, picture_number_filtered, picture_extractionmode);
                    break;
                default:
                    TRACE_ERROR(MAIN, "Unable to decode given file format '%s': no decoder available!\n",
                                getCodecString(stream_VIDEO, input_video->tracks_video[0]->stream_codec));
                    break;
            }
        }
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to start decoding because of an empty VideoFile_t structure! Parsing failed?\n");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_extract(VideoFile_t *input_video,
                      const char *output_directory,
                      const bool extract_audio,
                      const bool extract_video,
                      const bool extract_subtitles,
                      const int output_format)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_extract()\n" CLR_RESET);

    if (input_video != NULL && output_directory != NULL)
    {
        // Print status
        //import_fileStatus(input_video);

        // Export audio and video PES stream
        if (extract_audio)
            retcode = muxer_export_samples(input_video, input_video->tracks_audio[0], output_format);

        if (extract_video)
            retcode = muxer_export_samples(input_video, input_video->tracks_video[0], output_format);

        if (extract_subtitles)
            retcode = muxer_export_samples(input_video, input_video->tracks_subtitles[0], output_format);
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to extract from a NULL VideoFile_t struct!\n");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_close(VideoFile_t **input_video)
{
    return import_fileClose(input_video);
}

/* ************************************************************************** */
