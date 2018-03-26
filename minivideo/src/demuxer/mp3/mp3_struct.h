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
 * \file      mp3_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_MP3_STRUCT_H
#define PARSER_MP3_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"

#include <cstdio>

/* ************************************************************************** */

//! Structure for MP3 audio infos
typedef struct mp3_t
{
    bool run;                  //!< A convenient way to stop the parser from any sublevel

    uint32_t mpeg_version;
    uint32_t mpeg_layer;
    uint32_t mpeg_sampleperframe;

    uint32_t audio_bitrate_cbr; //!< Audio bitrate of the first frame (if CBR, otherwise use audio_bitrate_vbr)
    uint32_t audio_bitrate_vbr; //!< If VBR, we store the average bitrate here
    uint32_t audio_samplingrate;//!< In Hertz
    double   audio_pts_tick;    //!< In milliseconds
    bool     audio_vbr;         //!< True if VBR, false if CBR
    uint32_t audio_channels;    //!< Indicates channel mode

    double   media_duration_s;  //!< Media duration in seconds

    uint64_t sample_count;      //!< Total size of all the audio samples of the track (not including tags)
    uint64_t sample_size_total; //!< Total size of all the audio samples of the track (not including tags)
    uint32_t sample_size_max;   //!< Size of the biggest audio sample of the track (not including tags)

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} mp3_t;

/* ************************************************************************** */

//! MPEG audio channel modes
typedef enum MPEGChannelMode_e
{
    CHAN_MONO           = 0,
    CHAN_STEREO         = 1,
    CHAN_STEREO_JOINT   = 2,
    CHAN_DUAL           = 3

} MPEGChannelMode_e;

//! Audio version ID // Sampling Rate Index
static const uint32_t samplingrate_index_table[3][4] =
{
    { 44100, 48000, 32000, 0 }, // MPEG-1
    { 22050, 24000, 16000, 0 }, // MPEG-2
    { 11025, 12000,  8000, 0 }, // MPEG-2.5
};

//! Audio version ID // Layer_index
static const uint32_t sampleperframe_table[3][3] =
{
    { 384, 1152, 1152 }, // MPEG-1
    { 384, 1152,  576 }, // MPEG-2
    { 384, 1152,  576 }, // MPEG-2.5
};

//! Audio version ID // Layer_index // Bitrate Index
static const uint32_t bitrate_index_table[3][3][16] =
{
    { // MPEG-1
        { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }, // Layer 1
        { 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 }, // Layer 2
        { 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 }, // Layer 3
    },
    { // MPEG-2
        { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }, // Layer 1
        { 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
        { 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
    },
    { // MPEG-2.5
        { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }, // Layer 1
        { 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
        { 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
    }
};

/* ************************************************************************** */
#endif // PARSER_MP3_STRUCT_H
