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
 * \file      minivideo_mediafile.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef MINIVIDEO_MEDIAFILE_H
#define MINIVIDEO_MEDIAFILE_H

// minivideo headers
#include "minivideo_mediastream.h"
#include "minivideo_codecs.h"
#include "minivideo_containers.h"

// C standard libraries
#include <cstdio>
#include <cstdint>

/* ************************************************************************** */

/*!
 * \brief Informations about a media file and its content.
 */
typedef struct MediaFile_t
{
    // Generic file infos
    FILE *file_pointer;                 //!< File pointer used during the life of this media file
    int64_t file_size;                  //!< File size in bytes
    char file_path[4096];               //!< Absolute path of the file (used to derive other paths/names/extention)

    // Additional file infos
    char file_directory[4096];          //!< Absolute path of the directory containing the file
    char file_name[255];                //!< File name (with file extension)
    char file_extension[255];           //!< File extension, without dot (may NOT correspond to the real file format)
    uint64_t file_creation_time;        //!< File creation time (Unix time) (if available from filesystem metadata)
    uint64_t file_modification_time;    //!< File modification time (Unix time) (if available from filesystem metadata)

    // Container
    Containers_e container;             //!< File format / container used by this video file
    ContainerProfiles_e container_profile; //!< Container profile (if applicable)
    bool container_mapper;              //!< Enables the xml container mapper
    FILE *container_mapper_fd;          //!< Direct access to xml map without the need of an actual file

    // Container generic metadata
    uint64_t duration;                  //!< Content global duration (in ms)
    uint64_t creation_time;             //!< Container creation time (Unix time, UTC) (if available)
    uint64_t modification_time;         //!< Container modification time (Unix time, UTC) (if available)
    char *creation_app;                 //!< Container creation application (C string) (if available)
    char *creation_lib;                 //!< Container creation library (C string) (if available)

    // A/V track(s) datas and infos
    unsigned int tracks_audio_count;
    MediaStream_t *tracks_audio[16];    //!< A list of parsed audio tracks

    unsigned int tracks_video_count;
    MediaStream_t *tracks_video[16];    //!< A list of parsed video tracks

    unsigned int tracks_subtitles_count;
    MediaStream_t *tracks_subt[16];     //!< A list of parsed subtitles tracks

    unsigned int tracks_others_count;
    MediaStream_t *tracks_others[16];   //!< Other "unknown" tracks found in the container (metadata, timecodes, ...)

    // Parsing statistics
    uint64_t parsingTime;               //!< Parsing time (in milliseconds) (only available in debug mode)
    uint64_t parsingMemory;             //!< Parsing memory (in bytes) (only available in debug mode)

} MediaFile_t;

/*!
 * \brief Essential informations about an output file, mostly used to export thumbnails from mini_thumbnailer.
 */
typedef struct OutputFile_t
{
    // Generic file infos
    FILE *file_pointer;                 //!< File pointer used during the life of this media file
    int64_t file_size;                  //!< File size in bytes
    char file_path[4096];               //!< Absolute path of the file (used to derive other paths/names/extention)

    // Additional file infos
    char file_directory[4096];          //!< Absolute path of the directory containing the file
    char file_name[255];                //!< File name (without file extension)
    char file_extension[255];           //!< File extension, without dot (may NOT correspond to the real file format)
    uint64_t file_creation_time;        //!< File creation time (Unix time) (if available from filesystem metadata)
    uint64_t file_modification_time;    //!< File modification time (Unix time) (if available from filesystem metadata)

    // File format
    int picture_format;
    int picture_quality;

} OutputFile_t;

/*!
 * \brief Surface used for dynamic thumbnailing, mostly used by mini_analyser.
 */
typedef struct OutputSurface_t
{
    // Surface infos
    uint8_t *surface; //!< RGB24 surface

    unsigned surface_fcc;
    unsigned width;
    unsigned height;

} OutputSurface_t;

/* ************************************************************************** */
#endif // MINIVIDEO_MEDIAFILE_H
