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
 * \file      videofile_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef VIDEOFILE_STRUCT_H
#define VIDEOFILE_STRUCT_H

// C standard library
#include <stdio.h>

// minivideo headers
#include "avcodecs.h"
#include "bitstream_map_struct.h"

/* ************************************************************************** */

/*!
 * \brief Essential informations about a video file and its content.
 */
typedef struct VideoFile_t
{
    FILE *file_pointer;             //!< File pointer used during the life of this video file

    // Generic file infos
    int64_t file_size;              //!< File size in bytes
    char file_path[4096];           //!< Absolute path of the file (used to derive other paths/names/extention)
    char file_directory[4096];      //!< Absolute path of the directory containing the file
    char file_name[255];            //!< File name with file extension
    char file_extension[255];       //!< File extension, without dot (may NOT correspond to the file container)
    unsigned int file_creation_time;     //!< File creation time, from filesystem metadatas
    unsigned int file_modification_time; //!< File modification time, from filesystem metadatas

    ContainerFormat_e container;    //!< File format / container used by this video file

    // Meta datas // TODO dedicated metadatas structures
    char *creation_app;             //!< Container creation application
    unsigned int creation_time;     //!< Container creation time in milliseconds
    unsigned int modification_time; //!< Container modification time in milliseconds
    unsigned int duration;          //!< Content duration in milliseconds

    // A/V track(s) datas and infos
    int tracks_audio_count;
    BitstreamMap_t *tracks_audio[16];     //!< A list of parsed audio tracks

    int tracks_video_count;
    BitstreamMap_t *tracks_video[16];     //!< A list of parsed video tracks

    int tracks_subtitles_count;
    BitstreamMap_t *tracks_subtitles[16]; //!< A list of parsed subtitles tracks

    int tracks_others; //!< Other tracks found in the container but left unparsed (metadatas, timecodes, ...)

} VideoFile_t;

/*!
 * \brief Essential informations about a file.
 */
typedef struct OutputFile_t
{
    // File
    FILE *file_pointer;             //!< File pointer

    // File info
    char file_path[4096];           //!< Absolute path of the file
    char file_directory[4096];      //!< Absolute path of the directory containing the file

    int64_t file_size;              //!< File size in bytes
    char file_name[255];            //!< File name with file extension
    char file_extension[255];       //!< File extension, without dot

} OutputFile_t;

/* ************************************************************************** */
#endif /* VIDEOFILE_STRUCT_H */
