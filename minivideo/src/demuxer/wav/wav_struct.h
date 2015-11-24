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
 * \file      wave_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_WAVE_STRUCT_H
#define PARSER_WAVE_STRUCT_H

// minivideo headers
#include "../../typedef.h"

/* ************************************************************************** */

typedef struct fmtChunk_t
{
    uint16_t wFormatTag;          //!< Format code
    uint16_t nChannels;           //!< Number of interleaved channels
    uint32_t nSamplesPerSec;      //!< Sampling rate (blocks per second)
    uint32_t nAvgBytesPerSec;     //!< Data rate
    uint16_t nBlockAlign;         //!< Data block size (bytes)
    uint16_t wBitsPerSample;      //!< Bits per sample
    uint16_t cbSize;              //!< Size of the extension (0 or 22)

    // fmt chunk extension
    uint16_t wValidBitsPerSample; //!< Number of valid bits
    uint32_t dwChannelMask;       //!< Speaker position mask
    uint8_t SubFormat[16];        //!< GUID, including the data format code

} fmtChunk_t;

typedef struct factChunk_t
{
    uint32_t dwSampleLength;      //!< Number of samples (per channel)

} factChunk_t;

typedef struct dataChunk_t
{
    int64_t datasOffset;          //!< Offset of the first sample of this data chunk
    int64_t datasSize;            //!< Size of all the samples of this data chunk

} dataChunk_t;

typedef struct cueChunk_t
{
    uint32_t what;                //!< ?

} cueChunk_t;

//! Structure for WAVE audio infos
typedef struct wave_t
{
    fmtChunk_t fmt;
    factChunk_t fact;
    dataChunk_t data;
    cueChunk_t cue;

    //WaveTrack_t *tracks[16];

} wave_t;

/* ************************************************************************** */

/*!
 * Good ressource about these tags (search for WAVE_FORMAT_):
 * http://www.videolan.org/developers/vlc/doc/doxygen/html/vlc__codecs_8h_source.html
 */
typedef enum WaveFormatTag_e
{
    WAVE_FORMAT_UNKNOWN    = 0,

    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_ADPCM      = 0x0002,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_MULAW      = 0x0007,
    WAVE_FORMAT_DTS_MS     = 0x0008,

    WAVE_FORMAT_MP1        = 0x0050,
    WAVE_FORMAT_MP3        = 0x0055,

    WAVE_FORMAT_AC3        = 0x2000,
    WAVE_FORMAT_DTS        = 0x2001,
    WAVE_FORMAT_AAC        = 0x00FF,

    WAVE_FORMAT_EXTENSIBLE = 0xFFFE

} WaveFormatTag_e;
/*
// GUID SubFormat IDs
ATRAC3P {0xE923AABF, 0xCB58, 0x4471, {0xA1, 0x19, 0xFF, 0xFA, 0x01, 0xE4, 0xCE, 0x62}}
GUIDBASE        {0x00000000, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}
AMBISONIC_SUBTYPE_PCM        {0x00000001, 0x0721, 0x11D3, {0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00}}
AMBISONIC_SUBTYPE_IEEE_FLOAT {0x00000003, 0x0721, 0x11D3, {0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00}}
*/
/* ************************************************************************** */

typedef enum wave_fcc_e
{
    fcc_WAVE   = 0x57415645,

    fcc_fmt_   = 0x666D7420,
    fcc_fact   = 0x66616374,
    fcc_data   = 0x64617461,
    fcc_cue_   = 0x63756520

} wave_fcc_e;

/* ************************************************************************** */
#endif // PARSER_WAVE_STRUCT_H
