/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "import.h"
#include "demuxer/xml_mapper.h"
#include "bitstream_map.h"
#include "minitraces.h"
#include "minivideo_typedef.h"
#include "minivideo_containers.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

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
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every information available about the current media file.
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
        unsigned pos_last_slash_i = pos_last_slash_p - media->file_path + 1;
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
            unsigned pos_last_dot_i = pos_last_dot_p - media->file_path - pos_last_slash_i;
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
 * \param[in] *media: A pointer to a MediaFile_t structure containing various information about the file.
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
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every information available about the current media file.
 */
static void getContainer(MediaFile_t *media)
{
    media->container_ext = CONTAINER_UNKNOWN;
    media->container_sc = CONTAINER_UNKNOWN;
    media->container = CONTAINER_UNKNOWN;

    // Detect container format using start codes, by readind the first bytes of the file
    rewind(media->file_pointer);
    uint8_t buffer[16];
    if (fread(buffer, sizeof(uint8_t), sizeof(buffer), media->file_pointer) == sizeof(buffer))
    {
        media->container_sc = getContainerUsingStartcodes(buffer);
    }

    // Detect container format using file extension
    std::string ext = media->file_extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    media->container_ext = getContainerUsingExtension(ext);

    if (media->container_sc > CONTAINER_UNKNOWN)
    {
        media->container = media->container_sc;
    }
    else if (media->container_ext > CONTAINER_UNKNOWN)
    {
        media->container = media->container_ext;
    }
    else
    {
        TRACE_WARNING(IO, "* Unknown container format: startcodes and file extension detection failed...");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Open a file and check what's inside it.
 * \param[in] *filepath: The path of the file to load.
 * \param[in,out] **media_ptr: A pointer to a MediaFile_t structure, containing every information available about the current media file.
 *
 * Some more information about supported files:
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

                // Extract some information from the media file
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

        // Tracks
        for (unsigned i = 0; i < (*media_ptr)->tracks_audio_count; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_audio[i]);
        }
        for (unsigned i = 0; i < (*media_ptr)->tracks_video_count; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_video[i]);
        }
        for (unsigned i = 0; i < (*media_ptr)->tracks_subtitles_count; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_subt[i]);
        }
        for (unsigned i = 0; i < (*media_ptr)->tracks_others_count; i++)
        {
            free_bitstream_map(&(*media_ptr)->tracks_others[i]);
        }
/*
        // Chapters
        for (unsigned i = 0; i < (*media_ptr)->chapters_count; i++)
        {
            if (&(*media_ptr)->chapters[i])
            {
                free((*media_ptr)->chapters[i].name);
                free(&(*media_ptr)->chapters[i]);
            }
        }
        free((*media_ptr)->chapters);
*/
        // Vendors
        free((*media_ptr)->metadata_gopro);

        // MediaFile_t
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
 * \brief Print various information about a file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every information available about the current media file.
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
