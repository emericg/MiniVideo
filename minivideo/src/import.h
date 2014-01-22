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
 * \file      import.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef IMPORT_H
#define IMPORT_H

// C standard library
#include <stdio.h>

// minivideo headers
#include "avcodecs.h"
#include "bitstream_map.h"

/* ************************************************************************** */

/*!
 * \struct VideoFile_t
 * \brief Essential informations about a video file and its content.
 */
typedef struct VideoFile_t
{
    // File
    FILE *file_pointer;             //!< File pointer

    // File info
    char file_path[4096];           //!< Absolute path of the file
    char file_directory[4096];      //!< Absolute path of the directory containing the file

    int64_t file_size;              //!< File size in bytes
    char file_name[255];            //!< File name with file extension
    char file_extension[255];       //!< File extension, without dot

    // File format
    ContainerFormat_e container;    //!< Video file container
    AVCodec_e codec_audio;          //!< First audio codec
    AVCodec_e codec_video;          //!< First video codec

    // A/V data
    int tracks_audio_count;
    BitstreamMap_t *tracks_audio[16];  //!< A list of all audio tracks

    int tracks_video_count;
    BitstreamMap_t *tracks_video[16];  //!< A list of all video tracks

    int tracks_subtitles_count;
    BitstreamMap_t *tracks_subtitles[16]; //!< A list of all subtitles tracks

} VideoFile_t;

/*!
 * \struct OutputFile_t
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

VideoFile_t *import_fileOpen(const char *filepath);

int import_fileClose(VideoFile_t **video_ptr);

/* ************************************************************************** */
#endif /* IMPORT_H */
