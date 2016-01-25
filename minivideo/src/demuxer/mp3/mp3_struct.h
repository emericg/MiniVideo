/*!
 * COPYRIGHT (C) 2015 Emeric Grange - All Rights Reserved
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
#include "../../typedef.h"

/* ************************************************************************** */

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
