/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
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

// minivideo headers
#include "import.h"
#include "demuxer/xml_mapper.h"
#include "bitstream_map.h"
#include "minitraces.h"
#include "minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// C POSIX library
#ifdef _MSC_VER
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <unistd.h>
#endif

/* ************************************************************************** */

/*!
 * \brief Get various from a media filepath.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 *
 * Get absolute file path, file directory, file name and extension.
 * This function will only work with Unix-style file systems.
 */
static void getInfosFromPath(MediaFile_t *media)
{
    TRACE_2(IO, "getInfosFromPath()");

    // Check if mediaFile->filepath is an absolute path
    char *pos_first_slash_p = strchr(media->file_path, '/');

    if ((pos_first_slash_p != NULL) && ((pos_first_slash_p - media->file_path) == 0))
    {
        TRACE_2(IO, "* mediaFile->file_path seems to be an absolute path already (first caracter is /)");
    }
    else
    {
        char cwd[4096];
        char absolute_filepath[4096];
        FILE *temp = NULL;

        // First attempt
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            strncpy(absolute_filepath, strncat(cwd, media->file_path, sizeof(cwd) - 1), sizeof(absolute_filepath) - 1);
            temp = fopen(absolute_filepath, "rb");
        }

        if (temp != NULL)
        {
            TRACE_2(IO, "* New absolute file path found, new using method 1: '%s'", absolute_filepath);
            strncpy(media->file_path, absolute_filepath, sizeof(media->file_path) - 1);
            fclose(temp);
        }
        else
        {
            // Second attempt
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                strncpy(absolute_filepath, strncat(cwd, "/", 1), sizeof(absolute_filepath) - 1);
                strncat(absolute_filepath, media->file_path, sizeof(absolute_filepath) - 1);
                temp = fopen(absolute_filepath, "rb");
            }

            if (temp != NULL)
            {
                TRACE_2(IO, "* New absolute file path found, new using method 2");
                strncpy(media->file_path, absolute_filepath, sizeof(media->file_path) - 1);
                fclose(temp);
            }
            else
            {
                TRACE_2(IO, "* mediaFile->file_path seems to be an absolute path already");
            }
        }
    }

    // Get directory
    char *pos_last_slash_p = strrchr(media->file_path, '/');
    if (pos_last_slash_p != NULL)
    {
        unsigned int pos_last_slash_i = pos_last_slash_p - media->file_path + 1;
        if (pos_last_slash_i > sizeof(media->file_directory) - 1)
        {
            pos_last_slash_i = sizeof(media->file_directory) - 1;
        }

        // Set directory
        strncpy(media->file_directory, media->file_path, pos_last_slash_i);

        // Get file name
        char *pos_last_dot_p = strrchr(media->file_path, '.');
        if (pos_last_dot_p != NULL)
        {
            unsigned int pos_last_dot_i = pos_last_dot_p - media->file_path - pos_last_slash_i;
            if (pos_last_dot_i > sizeof(media->file_name) - 1)
            {
                pos_last_dot_i = sizeof(media->file_name) - 1;
            }

            // Set file name
            strncpy(media->file_name, pos_last_slash_p + 1, pos_last_dot_i);

            // Set file extension (without the dot)
            strncpy(media->file_extension, pos_last_dot_p + 1, sizeof(media->file_extension) - 1);
        }
        else
        {
            TRACE_WARNING(IO, "* Cannot find file extension!");

            // Set file name (without the extension)
            strncpy(media->file_name, pos_last_slash_p + 1, 254);
        }
    }
    else
    {
        TRACE_WARNING(IO, "* Cannot find file directory, name and extension!");
    }

    // Print results
    TRACE_1(IO, "* File path      : '%s'", media->file_path);
    TRACE_1(IO, "* File directory : '%s'", media->file_directory);
    TRACE_1(IO, "* File name      : '%s'", media->file_name);
    TRACE_1(IO, "* File extension : '%s'", media->file_extension);
}

/* ************************************************************************** */

/*!
 * \brief Get media file size.
 * \param[in] *media: A pointer to a MediaFile_t structure containing various informations about the file.
 */
static void getSize(MediaFile_t *media)
{
    TRACE_2(IO, "getSize()");

    fseek(media->file_pointer, 0, SEEK_END);
    media->file_size = (int64_t)ftell(media->file_pointer);
    rewind(media->file_pointer);

    if (media->file_size < 1024) // < 1 KiB
    {
        TRACE_1(IO, "* File size      : %i bytes", media->file_size);
    }
    else if (media->file_size < 1048576) // < 1 MiB
    {
        TRACE_1(IO, "* File size      : %.2f KiB (%.2f KB)", (double)media->file_size / 1024.0, (double)media->file_size / 1000.0);
    }
    else // >= 1 MiB
    {
        TRACE_1(IO, "* File size      : %.2f MiB (%.2f MB)", (double)media->file_size / 1024.0 / 1024.0, (double)media->file_size / 1000.0 / 1000.0);
    }
}

/* ************************************************************************** */

/*!
 * \brief Detect the container used by a multimedia file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \return container: A ContainerFormat_e value.
 *
 * This function check the first 16 bytes of a media file in order to find
 * evidence of a known file container format.
 */
static Containers_e getContainerUsingStartcodes(MediaFile_t *media)
{
    TRACE_2(IO, "getContainerUsingStartcodes()");

    // Set container to unknown
    Containers_e container = CONTAINER_UNKNOWN;

    // Read the first bytes of the file
    rewind(media->file_pointer);
    uint8_t buffer[16];
    size_t bytes_read = fread(buffer, sizeof(uint8_t), sizeof(buffer), media->file_pointer);
    if (bytes_read != sizeof(buffer))
        return container;

    // Parse the file to find evidence of a container format
    if (buffer[0] == 0x47)
    {
        TRACE_1(IO, "* File type      : TS (MPEG 'Transport Stream') container detected");
        container = CONTAINER_MPEG_TS;
    }
    else if (buffer[0] == 0x1A && buffer[1] == 0x45 && buffer[2] == 0xDF && buffer[3] == 0xA3)
    {
        TRACE_1(IO, "* File type      : EBML file detected, possibly MKV or WebM container");
        container = CONTAINER_MKV;
    }
    else if (buffer[0] == 0x52 && buffer[1] == 0x49 && buffer[2] == 0x46 && buffer[3] == 0x46)
    {
        // RIFF header:
        // 52 49 46 46 xx xx xx xx // R I F F (size)
        // then:
        // 41 56 49 20 4C 49 53 54 // A V I   L I S T
        // 57 41 56 45 66 6D 74 20 // W A V E f m t

        if (buffer[8] == 0x41 && buffer[9] == 0x56 && buffer[10] == 0x49 && buffer[11] == 0x20)
        {
            TRACE_1(IO, "* File type      : AVI container detected");
            container = CONTAINER_AVI;
        }
        else if (buffer[8] == 0x57 && buffer[9] == 0x41 && buffer[10] == 0x56 && buffer[11] == 0x45)
        {
            TRACE_1(IO, "* File type      : WAVE container detected");
            container = CONTAINER_WAVE;
        }
        else
        {
            TRACE_WARNING(IO, "* File type      : Unknown RIFF container detected");
        }
    }
    else if (buffer[0] == 0x00 && buffer[1] == 0x00)
    {
        if (buffer[2] == 0x01)
        {
            if (buffer[3] == 0xBA)
            {
                TRACE_1(IO, "* File type      : PS (MPEG 'Program Stream') container detected");
                container = CONTAINER_MPEG_PS;
            }
            else if (buffer[3] == 0xB3)
            {
                TRACE_1(IO, "* File type      : MPEG-1/2 / H.262 Elementary Stream detected (raw video datas)");
                container = CONTAINER_ES;
                //media->codec_video = CODEC_MPEG12;
            }
            else if (buffer[3] == 0x67)
            {
                TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (raw video datas)");
                container = CONTAINER_ES;
                //media->codec_video = CODEC_H264;
            }
        }
        else if (buffer[2] == 0x00 && buffer[3] == 0x01)
        {
            if (buffer[4] == 0xBA)
            {
                TRACE_1(IO, "* File type      : MPEG-PS (MPEG 'Program Stream') container detected");
                container = CONTAINER_MPEG_PS;
            }
            else if (buffer[4] == 0xB3)
            {
                TRACE_1(IO, "* File type      : MPEG-1/2 / H.262 Elementary Stream detected (raw video datas)");
                container = CONTAINER_ES;
                //media->codec_video = CODEC_MPEG12;
            }
            else if (buffer[4] == 0x67)
            {
                TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (raw video datas)");
                container = CONTAINER_ES;
                //media->codec_video = CODEC_H264;
            }
        }

        if (buffer[4] == 0x66 && buffer[5] == 0x74 && buffer[6] == 0x79 && buffer[7] == 0x70)
        {
            // MP4: 00 00 xx xx 66 74 79 70 // (size) f t y p

            TRACE_1(IO, "* File type      : ISO BMF (MOV,MP4, ...) container detected");
            container = CONTAINER_MP4;
        }
        else if (buffer[4] == 0x73 && buffer[5] == 0x74 && buffer[6] == 0x79 && buffer[7] == 0x70)
        {
            // MP4: 00 00 xx xx 73 74 79 70 // (size) s t y p

            TRACE_1(IO, "* File type      : ISO BMF (MOV,MP4, ...) container detected");
            container = CONTAINER_MP4;
        }
    }
    else if (buffer[0] == 0x30 && buffer[1] == 0x26 && buffer[2] == 0xB2 && buffer[3] == 0x75)
    {
        TRACE_1(IO, "* File type      : ASF container detected");
        container = CONTAINER_ASF;
    }
    else if (buffer[0] == 0x4F && buffer[1] == 0x67 && buffer[2] == 0x67 && buffer[3] == 0x53)
    {
        TRACE_1(IO, "* File type      : OGG container detected");
        container = CONTAINER_OGG;
    }
    else if (buffer[0] == 0x66 && buffer[1] == 0x61 && buffer[2] == 0x4C && buffer[3] == 0x43)
    {
        TRACE_1(IO, "* File type      : FLAC container detected");
        container = CONTAINER_FLAC;
    }
    else if (buffer[0] == 0x06 && buffer[1] == 0x0E && buffer[2] == 0x2B && buffer[3] == 0x34)
    {
        TRACE_1(IO, "* File type      : MXF container detected");
        container = CONTAINER_MXF;
    }
    else if (buffer[0] == 0x46 && buffer[1] == 0x4C && buffer[2] == 0x56 && buffer[3] == 0x01)
    {
        TRACE_1(IO, "* File type      : FLV container detected");
        container = CONTAINER_FLV;
    }
    else if (buffer[0] == 0xFF &&
             (buffer[1] == 0xFE || buffer[1] == 0xFD ||buffer[1] == 0xFB))
    {
        TRACE_1(IO, "* File type      : MP1/2/3 Elementary Stream detected");
        container = CONTAINER_ES_MP3;
    }

    return container;
}

/*!
 * \brief Detect the container used by a multimedia file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \return container: A ContainerFormat_e value.
 *
 * This function check the file extension to guess the container. As this method
 * is *definitely* not reliable (file extensions are set by users, and users
 * make mistakes) this code is here mostly for fun. And to list extensions for
 * whoever is interested.
 */
static Containers_e getContainerUsingExtension(MediaFile_t *media)
{
    TRACE_2(IO, "getContainerUsingExtension()");

    // Set container to unknown
    Containers_e container = CONTAINER_UNKNOWN;

    if (strlen(media->file_extension) > 0 && strlen(media->file_extension) < 255)
    {
        if (strncmp(media->file_extension, "avi", 255) == 0 ||
            strncmp(media->file_extension, "divx", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : AVI container detected");
            container = CONTAINER_AVI;
        }
        else if (strncmp(media->file_extension, "webm", 255) == 0 ||
                 strncmp(media->file_extension, "mkv", 255) == 0 ||
                 strncmp(media->file_extension, "mka", 255) == 0 ||
                 strncmp(media->file_extension, "mks", 255) == 0 ||
                 strncmp(media->file_extension, "mk3d", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : MKV container detected");
            container = CONTAINER_MKV;
        }
        else if (strncmp(media->file_extension, "mov", 255) == 0 || // QuickTime file format
                 strncmp(media->file_extension, "qt", 255)  == 0 ||
                 strncmp(media->file_extension, "mp4", 255) == 0 || // ISO Base Media file format
                 strncmp(media->file_extension, "m4v", 255) == 0 ||
                 strncmp(media->file_extension, "m4a", 255) == 0 ||
                 strncmp(media->file_extension, "m4p", 255) == 0 ||
                 strncmp(media->file_extension, "m4b", 255) == 0 ||
                 strncmp(media->file_extension, "m4s", 255) == 0 ||
                 strncmp(media->file_extension, "mp4v", 255)== 0 ||
                 strncmp(media->file_extension, "mp4a", 255)== 0 ||
                 strncmp(media->file_extension, "3gp", 255) == 0 || // ISO BMF / 3GP profile
                 strncmp(media->file_extension, "3g2", 255) == 0 ||
                 strncmp(media->file_extension, "3gpp", 255)== 0 ||
                 strncmp(media->file_extension, "f4v", 255) == 0 || // ISO BMF / Flash Video
                 strncmp(media->file_extension, "mj2", 255) == 0 || // ISO BMF / Motion JPEG2000 profile
                 strncmp(media->file_extension, "mjp2", 255)== 0)
        {
            TRACE_1(IO, "* File extension  : ISO BMF (MOV,MP4, ...) container detected");
            container = CONTAINER_MP4;
        }
        else if (strncmp(media->file_extension, "ps", 255) == 0  ||
                 strncmp(media->file_extension, "vob", 255) == 0 ||
                 strncmp(media->file_extension, "evo", 255) == 0 ||
                 strncmp(media->file_extension, "m2p", 255) == 0 ||
                 strncmp(media->file_extension, "m2v", 255) == 0 ||
                 strncmp(media->file_extension, "mpg", 255) == 0 ||
                 strncmp(media->file_extension, "mpeg", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : PS (MPEG 'Program Stream') container detected");
            container = CONTAINER_MPEG_PS;
        }
        else if (strncmp(media->file_extension, "ts", 255) == 0  ||
                 strncmp(media->file_extension, "trp", 255) == 0 ||
                 strncmp(media->file_extension, "mts", 255) == 0 ||
                 strncmp(media->file_extension, "m2ts", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : TS (MPEG 'Transport Stream') container detected");
            container = CONTAINER_MPEG_TS;
        }
        else if (strncmp(media->file_extension, "asf", 255) == 0 ||
                 strncmp(media->file_extension, "wma", 255) == 0 ||
                 strncmp(media->file_extension, "wmv", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : ASF container detected");
            container = CONTAINER_ASF;
        }
        else if (strncmp(media->file_extension, "ogg", 255) == 0 ||
                 strncmp(media->file_extension, "ogv", 255) == 0 ||
                 strncmp(media->file_extension, "oga", 255) == 0 ||
                 strncmp(media->file_extension, "ogx", 255) == 0 ||
                 strncmp(media->file_extension, "ogm", 255) == 0 ||
                 strncmp(media->file_extension, "spx", 255) == 0 ||
                 strncmp(media->file_extension, "opus", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : OGG container detected");
            container = CONTAINER_OGG;
        }
        else if (strncmp(media->file_extension, "mxf", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : MXF container detected");
            container = CONTAINER_MXF;
        }
        else if (strncmp(media->file_extension, "flac", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : FLAC container detected");
            container = CONTAINER_FLAC;
        }
        else if (strncmp(media->file_extension, "wav", 255) == 0 ||
                 strncmp(media->file_extension, "wave", 255) == 0 ||
                 strncmp(media->file_extension, "amb", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : WAVE container detected");
            container = CONTAINER_WAVE;
        }
        else if (strncmp(media->file_extension, "flv", 255) == 0 ||
                 strncmp(media->file_extension, "f4v", 255) == 0 ||
                 strncmp(media->file_extension, "f4p", 255) == 0 ||
                 strncmp(media->file_extension, "f4a", 255) == 0 ||
                 strncmp(media->file_extension, "f4b", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : FLV container detected");
            container = CONTAINER_FLV;
        }
        else if (strncmp(media->file_extension, "rm", 255) == 0 ||
                 strncmp(media->file_extension, "rmvb", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : RealMedia container detected");
            container = CONTAINER_RM;
        }
        else if (strncmp(media->file_extension, "264", 255) == 0 ||
                 strncmp(media->file_extension, "h264", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : H.264 ES detected");
            container = CONTAINER_ES;
            //codec = CODEC_H264;
        }
        else if (strncmp(media->file_extension, "265", 255) == 0 ||
                 strncmp(media->file_extension, "h265", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : H.265 ES detected");
            container = CONTAINER_ES;
            //codec = CODEC_H265;
        }
        else if (strncmp(media->file_extension, "aac", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : AAC ES detected");
            container = CONTAINER_ES_AAC;
            //codec = CODEC_AAC;
        }
        else if (strncmp(media->file_extension, "ac3", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : AC3 ES detected");
            container = CONTAINER_ES_AC3;
            //codec = CODEC_AC3;
        }
        else if (strncmp(media->file_extension, "mp1", 255) == 0 ||
                 strncmp(media->file_extension, "mp2", 255) == 0 ||
                 strncmp(media->file_extension, "mp3", 255) == 0)
        {
            TRACE_1(IO, "* File extension  : MP3 ES detected");
            container = CONTAINER_ES_MP3;
            //codec = CODEC_MP3;
        }
    }

    return container;
}

/*!
 * \brief Detect the container used by a multimedia file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 */
static void getContainer(MediaFile_t *media)
{
    media->container = CONTAINER_UNKNOWN;

    // Detect container format using start codes
    media->container = getContainerUsingStartcodes(media);

    if (media->container == CONTAINER_UNKNOWN)
    {
        TRACE_WARNING(IO, "* Unknown container format: startcodes detection failed...");

        // Fallback: detect container format using file extension
        media->container = getContainerUsingExtension(media);

        if (media->container == CONTAINER_UNKNOWN)
        {
            TRACE_ERROR(IO, "* Unknown container format: file extension detection failed...");
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Open a file and check what's inside it.
 * \param[in] *filepath: The path of the file to load.
 * \param[in,out] **media_ptr: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
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
int import_fileOpen(const char *filepath, MediaFile_t **media_ptr)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileOpen()" CLR_RESET);

    int retcode = FAILURE;

    if (filepath == NULL)
    {
        TRACE_ERROR(IO, "* File path is invalid");
    }
    else
    {
        // Allocate media structure and create a shortcut
        *media_ptr = (MediaFile_t*)calloc(1, sizeof(MediaFile_t));

        if (*media_ptr == NULL)
        {
            TRACE_ERROR(IO, "* Unable to allocate a MediaFile_t structure!");
        }
        else
        {
            MediaFile_t *media = (*media_ptr);

            // Set filepath in MediaFile_t
            strncpy(media->file_path, filepath, sizeof(media->file_path) - 1);
            TRACE_INFO(IO, "* File path (raw): '%s'", filepath);

            // Open file, read only
            media->file_pointer = fopen(filepath, "rb");

            if (media->file_pointer == NULL)
            {
                TRACE_ERROR(IO, "Unable to open the media file: '%s'!", filepath);
                free(*media_ptr);
                *media_ptr = NULL;
            }
            else
            {
                TRACE_1(IO, "* File successfully opened");

                // Extract some informations from the media file
                getInfosFromPath(media);
                getSize(media);
                getContainer(media);

                retcode = SUCCESS;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Close a file.
 * \param[in,out] **media_ptr: A pointer of pointer to a MediaFile_t structure.
 * \return 1 if success, 0 otherwise.
 */
int import_fileClose(MediaFile_t **media_ptr)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileClose()" CLR_RESET);
    int retcode = SUCCESS;
    int i = 0;

    if ((*media_ptr) != NULL)
    {
        if ((*media_ptr)->file_pointer != NULL)
        {
            if (fclose((*media_ptr)->file_pointer) == 0)
            {
                TRACE_1(IO, "* File successfully closed");
                retcode = SUCCESS;
            }
            else
            {
                TRACE_ERROR(IO, "Unable to close that file!");
                retcode = FAILURE;
            }
        }

        xmlMapperClose(&(*media_ptr)->container_mapper_fd);

        free((*media_ptr)->creation_app);
        free((*media_ptr)->creation_lib);

        for (i = 0; i < 16 /*(*media_ptr)->tracks_audio_count*/; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_audio[i]);
        }

        for (i = 0; i < 16/*(*media_ptr)->tracks_video_count*/; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_video[i]);
        }

        for (i = 0; i < 16/*(*media_ptr)->tracks_subtitles_count*/; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_subt[i]);
        }

        for (i = 0; i < 16/*(*media_ptr)->tracks_others_count*/; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_others[i]);
        }

        {
            free(*media_ptr);
            *media_ptr = NULL;

            TRACE_1(IO, ">> MediaFile freed");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print various informations about a file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 */
void import_fileStatus(MediaFile_t *media)
{
    TRACE_INFO(IO, BLD_GREEN "import_fileStatus()" CLR_RESET);

    unsigned i = 0;

    // File
    if (media->file_pointer)
    {
        TRACE_1(IO, "file_pointer is " BLD_GREEN "open" CLR_RESET);
    }
    else
    {
        TRACE_1(IO, "file_pointer is " BLD_RED "closed" CLR_RESET);
    }

    // File info
    TRACE_1(IO, "* File path      : '%s'", media->file_path);
    TRACE_1(IO, "* File directory : '%s'", media->file_directory);
    TRACE_1(IO, "* File name      : '%s'", media->file_name);
    TRACE_1(IO, "* File extension : '%s'", media->file_extension);
    TRACE_1(IO, "* File size      : %i MiB  /  %i MB",
            media->file_size / 1024 / 1024,
            media->file_size / 1000 / 1000);

    // File format
    TRACE_1(IO, "* File container :  '%s'", getContainerString(media->container, 1));

    // Audio track(s)
    TRACE_1(IO, "* %i audio track(s)", media->tracks_audio_count);
    for (i = 0; i < media->tracks_audio_count; i++)
    {
        if (media->tracks_audio[i])
            print_bitstream_map(media->tracks_audio[i]);
    }

    // Video track(s)
    TRACE_1(IO, "* %i video track(s)", media->tracks_video_count);
    for (i = 0; i < media->tracks_video_count; i++)
    {
        if (media->tracks_video[i])
            print_bitstream_map(media->tracks_video[i]);
    }

    // Subtitles track(s)
    TRACE_1(IO, "* %i subtitles track(s)", media->tracks_subtitles_count);
    for (i = 0; i < media->tracks_audio_count; i++)
    {
        if (media->tracks_subt[i])
            print_bitstream_map(media->tracks_subt[i]);
    }
}

/* ************************************************************************** */
