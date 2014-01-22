/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \date      2010
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

/*!
 * \brief Print informations about the library into standard output.
 */
void minivideo_infos(void)
{
    printf(GREEN "\nminivideo_infos()\n" RESET);
    printf("* Library version %d.%d-%d\n", minivideo_VERSION_MAJOR,
                                           minivideo_VERSION_MINOR,
                                           minivideo_VERSION_PATCH);

#if ENDIANNESS == LITTLE_ENDIAN
    printf("* ENDIANNESS is set to LITTLE_ENDIAN\n");
#else
    printf("* ENDIANNESS is set to BIG_ENDIAN\n");
#endif /* ENDIANNESS */

#if ENABLE_DEBUG
    printf("* DEBUG traces are " GREEN "ON\n" RESET);
#else
    printf("* DEBUG traces are " RED "OFF\n" RESET);
#endif /* ENABLE_DEBUG */

#if ENABLE_COLORS
    printf("* COLORS are " GREEN "ON\n" RESET);
#else
    printf("* COLORS are OFF\n" RESET);
#endif /* ENABLE_COLORS */

#if ENABLE_STDINT
    printf("* C99 integer support is " GREEN "ON\n" RESET);
#else
    printf("* C99 integer support is " RED "OFF\n" RESET);
    printf("* Integer support is emulated\n");
#endif /* ENABLE_STDINT */

#if ENABLE_STDBOOL
    printf("* C99 boolean support is " GREEN "ON\n" RESET);
#else
    printf("* C99 boolean support is " RED "OFF\n" RESET);
    printf("* boolean support is emulated\n");
#endif /* ENABLE_STDBOOL */

#if ENABLE_JPEG
    printf("* EXTERNAL JPEG SUPPORT is " GREEN "ON\n" RESET);
#else
    printf("* EXTERNAL JPEG SUPPORT is " RED "OFF\n" RESET);
#endif /* ENABLE_JPEG */

#if ENABLE_PNG
    printf("* EXTERNAL PNG SUPPORT is " GREEN "ON\n" RESET);
#else
    printf("* EXTERNAL PNG SUPPORT is " RED "OFF\n" RESET);
#endif /* ENABLE_PNG */

#if ENABLE_STBIMWRITE
    printf("* STB_IMAGE_WRITE SUPPORT is " GREEN "ON\n" RESET);
#else
    printf("* STB_IMAGE_WRITE SUPPORT is " RED "OFF\n" RESET);
#endif /* ENABLE_STBIMWRITE */

#if ENABLE_DEBUG
    printf("\n* Tracing test\n");
    TRACE_ERROR(MAIN, "TEST error\n");
    TRACE_WARNING(MAIN, "TEST warning\n");
    TRACE_INFO(MAIN, "TEST info\n");
    TRACE_1(MAIN, "TEST lvl 1\n");
    TRACE_2(MAIN, "TEST lvl 2\n");
    TRACE_3(MAIN, "TEST lvl 3\n\n");

    TRACE_INFO(MAIN, GREEN "minivideo_thumbnailer()\n" RESET);
#endif /* ENABLE_DEBUG */
}

/* ************************************************************************** */

/*!
 * \brief Determine endianness of the current system.
 * \return 4321 for big endian, 1234 for little endian, -1 if unable to determine endianness.
 *
 * To determine endianness, we use a character pointer to the bytes of an int,
 * and then check its first byte to see if it is 0 (meaning big endianness)
 * or 1 (meaning little endianness).
 */
int minivideo_endianness(void)
{
    int retcode = -1;
    int i = 1;
    char *p = (char *)&i;

#if ENABLE_DEBUG
    printf(GREEN "\nminivideo_endianness()\n" RESET);

    if (p[0] == 1)
    {
        printf("* LITTLE_ENDIAN\n");
        retcode = LITTLE_ENDIAN; // 1234
    }
    else
    {
        printf("* BIG_ENDIAN\n");
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

/*!
 * \brief Open a file, check what's in it, parse it, then close it.
 * \param *input_filepath The file path of the video we want to decode.
 * \param *output_directory The directory where we want to save decoded thumbnail(s).
 * \param picture_format The picture format for thumbnail(s) we want to extract.
 * \param picture_quality The quality of thumbnail(s) we want to extract.
 * \param picture_number The number of thumbnail(s) we want to extract.
 * \param picture_extractionmode The method of distribution for thumbnails extraction.
 * \return 0 if picture(s) extraction is a success, 1 otherwise.
 *
 * The first step is to open the file with the given fileptath.
 * If the file is successfully opened, the program start gathering informations about
 * the file, print them, then try to parse it's container (if appropriate parser is
 * available) and start decoding one or more picture(s) from the video (if appropriate
 * decoder is available). Picture(s) is/are exported into selected file format.
 * The video is then closed, and the library is exited with return code.
 */
int minivideo_thumbnailer(const char *input_filepath,
                          const char *output_directory,
                          const int picture_format,
                          const int picture_quality,
                          const int picture_number,
                          const int picture_extractionmode)
{
    int retcode = FAILURE;
    int picture_number_filtered = picture_number;

    // Open the video file
    VideoFile_t *input_video = import_fileOpen(input_filepath);

    if (input_video != NULL)
    {
        // Start container parsing
        switch (input_video->container)
        {
            case CONTAINER_UNKNOWN:
                TRACE_ERROR(MAIN, "Unknown container format. Unable to parse that file!\n");
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
                TRACE_ERROR(MAIN, "Unable to parse given container format (id %i): no parser available!\n", input_video->container);
                break;
        }

        if (retcode == SUCCESS)
        {
            // IDR frame filtering
            picture_number_filtered = idr_filtering(&input_video->tracks_video[0],
                                                    picture_number, picture_extractionmode);

            if (picture_number_filtered == 0)
                retcode = FAILURE;
        }

        if (retcode == SUCCESS)
        {
            // Start video decoding
            switch (input_video->codec_video)
            {
                case CODEC_UNKNOWN:
                    //TRACE_ERROR(MAIN, "Unknown video format. Unable to decode that file!\n");
                    //break;
                case CODEC_H264:
                    retcode = h264_decode(input_video, output_directory, picture_format, picture_quality, picture_number_filtered, picture_extractionmode);
                    break;
                default:
                    TRACE_ERROR(MAIN, "Unable to decode given file format (id %i): no decoder available!\n", input_video->codec_video);
                    break;
            }
        }

        // Close file
        import_fileClose(&input_video);
    }

    // Convert return code into exit code
    if (retcode == SUCCESS)
    {
        retcode = EXIT_SUCCESS;
        TRACE_INFO(MAIN, YELLOW "Exiting minivideo thumbnailing function without errors.\n" RESET);
    }
    else
    {
        retcode = EXIT_FAILURE;
        TRACE_INFO(MAIN, YELLOW "Exiting minivideo thumbnailing function " RED "with errors!\n" RESET);
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_extractor(const char *input_filepath,
                        const char *output_directory,
                        const bool extract_audio,
                        const bool extract_video,
                        const bool extract_subtitles,
                        const int output_format)
{
    int retcode = FAILURE;

    // Open the video file
    VideoFile_t *input_video = import_fileOpen(input_filepath);

    if (input_video != NULL)
    {
        // Start container parsing
        switch (input_video->container)
        {
            case CONTAINER_UNKNOWN:
                TRACE_ERROR(MAIN, "Unknown container format. Unable to parse that file!\n");
                break;
            case CONTAINER_PS:
                retcode = ps_fileParse(input_video);
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
                // Guess its H.264 codec, the only one supported anyway
                input_video->codec_video = CODEC_H264;
                retcode = bruteforce_fileParse(input_video, CODEC_H264);
                break;
            default:
                TRACE_ERROR(MAIN, "Unable to parse given container format (id %i): no parser available!\n", input_video->container);
                break;
        }

        // Export audio and video PES stream
        if (retcode == SUCCESS)
        {
            if (extract_audio)
                retcode = muxer_export_samples(input_video, input_video->tracks_audio[0], output_format);

            if (extract_video)
                retcode = muxer_export_samples(input_video, input_video->tracks_video[0], output_format);

            if (extract_subtitles)
                retcode = muxer_export_samples(input_video, input_video->tracks_subtitles[0], output_format);
        }

        // Close file
        import_fileClose(&input_video);
    }

    // Convert return code into exit code
    if (retcode == SUCCESS)
    {
        retcode = EXIT_SUCCESS;
        TRACE_INFO(MAIN, YELLOW "Exiting minivideo extracting function without errors.\n" RESET);
    }
    else
    {
        retcode = EXIT_FAILURE;
        TRACE_INFO(MAIN, YELLOW "Exiting minivideo extracting " RED "with errors!\n" RESET);
    }

    return retcode;
}

/* ************************************************************************** */
