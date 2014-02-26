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
 * \file      import.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// C POSIX library
#include <unistd.h>

// minivideo headers
#include "minitraces.h"

#include "import.h"

/* ************************************************************************** */

/*!
 * \brief Get various from a video filepath.
 * \param *video A pointer to a VideoFile_t structure, containing every informations available about the current video file.
 *
 * Get absolute file path, file directory, file name and extension.
 * This function will only work with Unix-style file systems.
 */
static void getInfosFromPath(VideoFile_t *video)
{
    TRACE_2(IO, "getInfosFromPath()\n");

    // Check if video->filepath is an absolute path
    char *pos_first_slash_p = strchr(video->file_path, '/');

    if ((pos_first_slash_p != NULL) && ((pos_first_slash_p - video->file_path) == 0))
    {
        TRACE_2(IO, "* video->file_path seems to be an absolute path already (first caracter is /)\n");
    }
    else
    {
        char cwd[4096];
        char absolute_filepath[4096];
        FILE *temp = NULL;

        // First attempt
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            strncpy(absolute_filepath, strncat(cwd, video->file_path, sizeof(cwd) - 1), sizeof(absolute_filepath) - 1);
            temp = fopen(absolute_filepath, "r");
        }

        if (temp != NULL)
        {
            TRACE_2(IO, "* New absolute file path found, new using method 1: '%s'\n", absolute_filepath);
            strncpy(video->file_path, absolute_filepath, sizeof(video->file_path) - 1);
            fclose(temp);
        }
        else
        {
            // Second attempt
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                strncpy(absolute_filepath, strncat(cwd, "/", 1), sizeof(absolute_filepath) - 1);
                strncat(absolute_filepath, video->file_path, sizeof(absolute_filepath) - 1);
                temp = fopen(absolute_filepath, "r");
            }

            if (temp != NULL)
            {
                TRACE_2(IO, "* New absolute file path found, new using method 2\n");
                strncpy(video->file_path, absolute_filepath, sizeof(video->file_path) - 1);
                fclose(temp);
            }
            else
            {
                TRACE_2(IO, "* video->file_path seems to be an absolute path already\n");
            }
        }
    }

    // Get directory
    char *pos_last_slash_p = strrchr(video->file_path, '/');
    if (pos_last_slash_p != NULL)
    {
        int pos_last_slash_i = pos_last_slash_p - video->file_path + 1;
        if (pos_last_slash_i > sizeof(video->file_directory) - 1)
        {
            pos_last_slash_i = sizeof(video->file_directory) - 1;
        }

        // Set directory
        strncpy(video->file_directory, video->file_path, pos_last_slash_i);

        // Get file name
        char *pos_last_dot_p = strrchr(video->file_path, '.');
        if (pos_last_dot_p != NULL)
        {
            int pos_last_dot_i = pos_last_dot_p - video->file_path - pos_last_slash_i;
            if (pos_last_dot_i > sizeof(video->file_name) - 1)
            {
                 pos_last_dot_i = sizeof(video->file_name) - 1;
            }

            // Set file name
            strncpy(video->file_name, pos_last_slash_p + 1, pos_last_dot_i);

            // Set file extension
            strncpy(video->file_extension, pos_last_dot_p + 1, sizeof(video->file_extension) - 1);
        }
        else
        {
            TRACE_WARNING(IO, "* Cannot find file name and extension!\n");
        }
    }
    else
    {
        TRACE_WARNING(IO, "* Cannot find file directory, name and extension!\n");
    }

    // Print results
    TRACE_1(IO, "* File path      : '%s'\n", video->file_path);
    TRACE_1(IO, "* File directory : '%s'\n", video->file_directory);
    TRACE_1(IO, "* File name      : '%s'\n", video->file_name);
    TRACE_1(IO, "* File extension : '%s'\n", video->file_extension);
}

/* ************************************************************************** */

/*!
 * \brief Get video file size.
 * \param *video A pointer to a VideoFile_t structure containing various informations about the file.
 */
static void getSize(VideoFile_t *video)
{
    TRACE_2(IO, "getSize()\n");

    fseek(video->file_pointer, 0, SEEK_END);
    video->file_size = (int64_t)ftell(video->file_pointer);
    rewind(video->file_pointer);

    if (video->file_size < 1024) // < 1 KiB
    {
        TRACE_1(IO, "* File size      : %i bytes\n", video->file_size);
    }
    else if (video->file_size < 1048576) // < 1 MiB
    {
        TRACE_1(IO, "* File size      : %.2f KiB (%.2f KB)\n", (double)video->file_size / 1024.0, (double)video->file_size / 1000.0);
    }
    else // >= 1 MiB
    {
        TRACE_1(IO, "* File size      : %.2f MiB (%.2f MB)\n", (double)video->file_size / 1024.0 / 1024.0, (double)video->file_size / 1000.0 / 1000.0);
    }
}

/* ************************************************************************** */

/*!
 * \brief Get video container.
 * \param *video A pointer to a VideoFile_t structure, containing every informations available about the current video file.
 *
 * This function check the first 8 bytes of a video file in order to find sign of
 * a known file container format.
 */
static void getContainer(VideoFile_t *video)
{
    TRACE_2(IO, "getContainer()\n");

    // Set container to unknown
    video->container = CONTAINER_UNKNOWN;

    // Read the first 8 bytes of the file
    rewind(video->file_pointer);
    uint8_t buffer[8];
    fread(buffer, sizeof(uint8_t), sizeof(buffer), video->file_pointer);

    // Parse the file to find evidence of a container format
    if (buffer[0] == 0x47)
    {
        TRACE_1(IO, "* File type      : TS (MPEG Transport Stream) container detected\n");
        video->container = CONTAINER_MPEG_TS;
    }
    else if (buffer[0] == 0x1A && buffer[1] == 0x45 && buffer[2] == 0xDF && buffer[3] == 0xA3)
    {
        TRACE_1(IO, "* File type      : EBML file detected, possibly MKV or WebM container\n");
        video->container = CONTAINER_MKV;
    }
    else if (buffer[0] == 0x00 && buffer[1] == 0x00)
    {
        if (buffer[2] == 0x01)
        {
            if (buffer[3] == 0xBA)
            {
                TRACE_1(IO, "* File type      : PS (MPEG Program Stream) container detected\n");
                video->container = CONTAINER_MPEG_PS;
            }
            else if (buffer[3] == 0xB3)
            {
                TRACE_1(IO, "* File type      : MPEG12 / H.262 Elementary Stream detected (video data without container)\n");
                video->container = CONTAINER_ES;
                //video->codec_video = CODEC_MPEG12;
            }
            else if (buffer[3] == 0x67)
            {
                TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (video data without container)\n");
                video->container = CONTAINER_ES;
                //video->codec_video = CODEC_H264;
            }
        }
        else if (buffer[2] == 0x00 && buffer[3] == 0x01)
        {
            if (buffer[4] == 0xBA)
            {
                TRACE_1(IO, "* File type      : PS (MPEG Program Stream) container detected\n");
                video->container = CONTAINER_MPEG_PS;
            }
            else if (buffer[4] == 0xB3)
            {
                TRACE_1(IO, "* File type      : MPEG12 / H.262 Elementary Stream detected (video data without container)\n");
                video->container = CONTAINER_ES;
                //video->codec_video = CODEC_MPEG12;
            }
            else if (buffer[4] == 0x67)
            {
                TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (video data without container)\n");
                video->container = CONTAINER_ES;
                //video->codec_video = CODEC_H264;
            }
        }

        if (buffer[4] == 0x66 && buffer[5] == 0x74 && buffer[6] == 0x79 && buffer[7] == 0x70)
        {
            TRACE_1(IO, "* File type      : MP4 container detected\n");
            video->container = CONTAINER_MP4;

            // Box size on 4 bytes (ignored), then
            // Box type on 4 bytes = "ftyp"
        }
    }
    else if (buffer[0] == 0x46 && buffer[1] == 0x4C && buffer[2] == 0x56 && buffer[3] == 0x01)
    {
        TRACE_1(IO, "* File type      : FLV container detected\n");
        //video->container = CONTAINER_FLV;
    }
    else if (buffer[0] == 0x4F && buffer[1] == 0x67 && buffer[2] == 0x67 && buffer[3] == 0x53)
    {
        TRACE_1(IO, "* File type      : OGG container detected\n");
        //video->container = CONTAINER_OGG;
    }
    else if (buffer[0] == 0x52 && buffer[1] == 0x49 && buffer[2] == 0x46 && buffer[3] == 0x46)
    {
        TRACE_1(IO, "* File type      : AVI container detected\n");
        video->container = CONTAINER_AVI;

        // AVI: 52 49 46 46 xx xx xx xx
        //       41 56 49 20 4C 49 53 54
    }

    if (video->container == CONTAINER_UNKNOWN)
    {
        TRACE_ERROR(IO, "* Unknown file type: no known container detected\n");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Open a file and check what's inside it.
 * \param *filepath The path of the file to load.
 * \return *video A pointer to a VideoFile_t structure, containing every informations available about the current video file.
 *
 * Some more informations about supported files:
 * Size and offset are coded on int64_t (signed long long), so this library should
 * be able to handle file of 1073741824 GiB if Large File Support is enabled during
 * compilation, 2 GiB otherwise.
 * The filename is limited to 255 caracters (including file extension) and the
 * complete filepath is limited to 4096 caracters.
 *
 * These parameters will only work on POSIX compliant operating system.
 */
VideoFile_t *import_fileOpen(const char *filepath)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileOpen()\n" CLR_RESET);

    // VideoFile allocation
    VideoFile_t *video = (VideoFile_t*)calloc(1, sizeof(VideoFile_t));

    if (video == NULL)
    {
        TRACE_ERROR(IO, "Unable to alloc new VideoFile_t structure!\n");
    }
    else
    {
        if (filepath == NULL)
        {
            TRACE_ERROR(IO, "* File path is invalid\n");
            free(video);
            video = NULL;
        }
        else
        {
            // Set filepath in VideoFile_t
            strncpy(video->file_path, filepath, sizeof(video->file_path) - 1);
            TRACE_2(IO, "* File path (brut): '%s'\n", filepath);

            // Open file, read only
            video->file_pointer = fopen(filepath, "r");

            if (video->file_pointer == NULL)
            {
                TRACE_ERROR(IO, "Unable to open the video file!\n");
                free(video);
                video = NULL;
            }
            else
            {
                TRACE_1(IO, "* File successfully opened\n");

                // Get some informations from the video file
                getInfosFromPath(video);
                getSize(video);
                getContainer(video);
            }
        }
    }

    return video;
}

/* ************************************************************************** */

/*!
 * \brief Close a file.
 * \param **video_ptr A pointer of pointer to a VideoFile_t structure, containing every informations available about the current video file.
 * \return 1 if success, 0 otherwise.
 */
int import_fileClose(VideoFile_t **video_ptr)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileClose()\n" CLR_RESET);
    int retcode = SUCCESS;
    int i = 0;

    if ((*video_ptr) != NULL)
    {
        if ((*video_ptr)->file_pointer != NULL)
        {
            if (fclose((*video_ptr)->file_pointer) == 0)
            {
                TRACE_1(IO, "* File successfully closed\n");
                retcode = SUCCESS;
            }
            else
            {
                TRACE_ERROR(IO, "Unable to close that file!\n");
                retcode = FAILURE;
            }
        }

        for (i = 0; i < 16 /*(*video_ptr)->tracks_audio_count*/; i++)
        {
            free_bitstream_map(&(*video_ptr)->tracks_audio[i]);
        }

        for (i = 0; i < 16/*(*video_ptr)->tracks_video_count*/; i++)
        {
            free_bitstream_map(&(*video_ptr)->tracks_video[i]);
        }

        for (i = 0; i < 16/*(*video_ptr)->tracks_subtitles_count*/; i++)
        {
            free_bitstream_map(&(*video_ptr)->tracks_subtitles[i]);
        }

        {
            free(*video_ptr);
            *video_ptr = NULL;

            TRACE_1(IO, ">> VideoFile freed\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

void import_fileStatus(VideoFile_t *videoFile)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileStatus()\n" CLR_RESET);

    int i = 0;

    // File
    if (videoFile->file_pointer)
        TRACE_1(IO, "file_pointer is " BLD_GREEN "open\n" CLR_RESET);
    else
        TRACE_1(IO, "file_pointer is " BLD_RED "closed\n" CLR_RESET);


    // File info
    TRACE_1(IO, "* File path      : '%s'\n", videoFile->file_path);
    TRACE_1(IO, "* File directory : '%s'\n", videoFile->file_directory);
    TRACE_1(IO, "* File name      : '%s'\n", videoFile->file_name);
    TRACE_1(IO, "* File extension : '%s'\n", videoFile->file_extension);
    TRACE_1(IO, "* File size      : %i MiB  /  %i MB\n", videoFile->file_size / 1024 / 1024, videoFile->file_size / 1000 / 1000);

    // File format
    TRACE_1(IO, "* File container :  '%s'\n", getContainerString(videoFile->container));

    // Audio track(s)
    TRACE_1(IO, "* %i audio track(s)\n", videoFile->tracks_audio_count);
    for (i = 0; i < videoFile->tracks_audio_count; i++)
    {
        if (videoFile->tracks_audio[i])
            print_bitstream_map(videoFile->tracks_audio[i]);
    }

    // Video track(s)
    TRACE_1(IO, "* %i video track(s)\n", videoFile->tracks_video_count);
    for (i = 0; i < videoFile->tracks_video_count; i++)
    {
        if (videoFile->tracks_video[i])
            print_bitstream_map(videoFile->tracks_video[i]);
    }

    // Subtitles track(s)
    TRACE_1(IO, "* %i subtitles track(s)\n", videoFile->tracks_subtitles_count);
    for (i = 0; i < videoFile->tracks_audio_count; i++)
    {
        if (videoFile->tracks_subtitles[i])
            print_bitstream_map(videoFile->tracks_subtitles[i]);
    }
}

/* ************************************************************************** */
