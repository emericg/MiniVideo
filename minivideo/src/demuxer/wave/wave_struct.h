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
    uint32_t nAvgBytesPerSec;     //!< Data rate (in bytes)
    uint16_t nBlockAlign;         //!< Data block size (in bytes)
    uint16_t wBitsPerSample;      //!< Bits per sample (in bits)

    // fmt chunk extension
    uint16_t cbSize;              //!< Size of the extension (in bytes)

        // PCM
        uint16_t wValidBitsPerSample; //!< Number of valid bits
        uint32_t dwChannelMask;       //!< Speaker position mask
        uint8_t SubFormat[16];        //!< GUID, including the data format code

        // MP1
        uint16_t fwHeadLayer;
        uint32_t dwHeadBitrate;
        uint16_t fwHeadMode;
        uint16_t fwHeadModeExt;
        uint16_t wHeadEmphasis;
        uint16_t fwHeadFlag;
        uint32_t dwPTSLow;
        uint32_t dwPTSHigh;

        // MP3
        uint16_t wID;
        uint32_t fdwFlags;
        uint16_t nBlockSize;
        uint16_t nFramesPerBlock;
        uint16_t nCodecDelay;

        // WAVE extension
        uint16_t wSamplesPerBlock;
        uint16_t wReserved;

} fmtChunk_t;

typedef struct factChunk_t
{
    uint32_t dwSampleLength;      //!< Number of samples (per channel)

} factChunk_t;

typedef struct dataChunk_t
{
    int64_t datasOffset;          //!< Offset of the first sample of this data chunk
    int64_t datasSize;            //!< Size of all the samples of this data chunk (in bytes)

} dataChunk_t;

typedef struct cueChunk_t
{
    uint32_t what;                //!< ?

} cueChunk_t;

//! Structure for WAVE audio infos
typedef struct wave_t
{
    bool run; //!< A convenient way to stop the parser from any sublevel

    fmtChunk_t fmt;
    factChunk_t fact;
    dataChunk_t data;
    cueChunk_t cue;

} wave_t;

/* ************************************************************************** */

//SUBTYPE_AMBISONIC_B_FORMAT_PCM {00000001-0721-11d3-8644-C8C1CA000000}
//SUBTYPE_AMBISONIC_B_FORMAT_IEEE_FLOAT {00000003-0721-11d3-8644-C8C1CA000000}

/*
// GUID SubFormat IDs
ATRAC3P                         {0xE923AABF, 0xCB58, 0x4471, {0xA1, 0x19, 0xFF, 0xFA, 0x01, 0xE4, 0xCE, 0x62}}
GUIDBASE                        {0x00000000, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}}
AMBISONIC_SUBTYPE_PCM           {0x00000001, 0x0721, 0x11D3, {0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00}}
AMBISONIC_SUBTYPE_IEEE_FLOAT    {0x00000003, 0x0721, 0x11D3, {0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00}}
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
