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
 * \file      wave_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_WAVE_STRUCT_H
#define PARSER_WAVE_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include "../../minivideo_codecs.h"
#include "../../minivideo_containers.h"

#include <cstdio>

/* ************************************************************************** */

/*!
 * Format Chunk.
 */
typedef struct fmtChunk_t
{
    uint16_t wFormatTag;        //!< Format code
    uint16_t nChannels;         //!< Number of interleaved channels
    uint32_t nSamplesPerSec;    //!< Sampling rate (blocks per second)
    uint32_t nAvgBytesPerSec;   //!< Data rate (in bytes)
    uint16_t nBlockAlign;       //!< Data block size (in bytes)

    // PCM extension
    uint16_t wBitsPerSample;    //!< Bits per sample (in bits)

    // fmt chunk extension
    uint16_t cbSize;             //!< Size of the extension (in bytes)

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

/*!
 * Fact Chunk.
 */
typedef struct factChunk_t
{
    uint32_t dwSampleLength;        //!< Number of samples (per channel)

} factChunk_t;

/*!
 * Data Chunk.
 */
typedef struct dataChunk_t
{
    int64_t datasOffset;            //!< Offset of the first sample of this data chunk
    int64_t datasSize;              //!< Size of all the samples of this data chunk (in bytes)

} dataChunk_t;

/*!
 * Cue Points Chunk.
 */
typedef struct cueChunk_t
{
    uint32_t dwCuePoints;           //!< Cue points count

    uint32_t *dwName;
    uint32_t *dwPosition;
    uint32_t *fccChunk;
    uint32_t *dwChunkStart;
    uint32_t *dwBlockStart;
    uint32_t *dwSampleOffset;

} cueChunk_t;

/*!
 * Playlist Chunk.
 */
typedef struct plstChunk_t
{
    uint32_t dwSegments;           //!< Play segment count

    uint32_t *dwName;
    uint32_t *dwLength;
    uint32_t *dwLoops;

} plstChunk_t;

/*!
 * Broadcast Audio Extension Chunk.
 */
typedef struct bwfChunk_t
{
    uint8_t Description[256];       //!< Description of the sound sequence (ASCII string)
    uint8_t Originator[32];         //!< Name of the originator (ASCII string)
    uint8_t OriginatorReference[32];//!< Reference of the originator (ASCII string)
    uint8_t OriginatorDate[10];     //!< yyyy:mm:dd (ASCII string)
    uint8_t OriginationTime[8];     //!< hh:mm:ss (ASCII string)
    uint32_t TimeReferenceLow;
    uint32_t TimeReferenceHigh;
    uint16_t Version;               //!< Version of the BWF format
    uint8_t UMID[64];               //!< SMPTE UMID

    uint16_t LoudnessVal;
    uint16_t LoudnessRange;
    uint16_t MaxTruePeakLevel;
    uint16_t MaxMomentaryLoudnes;
    uint16_t MaxShortTermLoudness;

    // (reserved bytes)

    uint8_t *CodingHistory;          //!< ASCII string

} bwfChunk_t;

/*!
 * ChunkSize64 Chunk.
 */
typedef struct big1Chunk_t
{
    uint64_t chunkSize;

} big1Chunk_t;

/*!
 * DataSize64 Chunk.
 */
typedef struct ds64Chunk_t
{
    uint64_t riffSize;
    uint64_t dataSize;
    uint64_t sampleCount;
    uint32_t tableLength;

    // data table?

} ds64Chunk_t;

//! Structure for WAVE audio infos
typedef struct wave_t
{
    bool run;           //!< A convenient way to stop the parser from any sublevel

    ContainerProfiles_e profile;

    fmtChunk_t fmt;
    factChunk_t fact;
    dataChunk_t data;
    cueChunk_t cue;
    plstChunk_t plst;
    bwfChunk_t bwf;

    FILE *xml;          //!< Temporary file used by the xmlMapper

} wave_t;

/* ************************************************************************** */

static const uint8_t WAVE_SubFormat_GUIDs[4][16] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71},   // GUIDBASE
    {0xE9, 0x23, 0xAA, 0xBF, 0xCB, 0x58, 0x44, 0x71, 0xA1, 0x19, 0xFF, 0xFA, 0x01, 0xE4, 0xCE, 0x62},   // ATRAC3P
    {0x00, 0x00, 0x00, 0x01, 0x07, 0x21, 0x11, 0xD3, 0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00},   // AMBISONIC_SUBTYPE_PCM
    {0x00, 0x00, 0x00, 0x03, 0x07, 0x21, 0x11, 0xD3, 0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00},   // AMBISONIC_SUBTYPE_IEEE_FLOAT
};

typedef enum WAVE_SubFormat_Names_e
{
    WAVE_GUIDBASE,
    WAVE_ATRAC3P,
    WAVE_AMBISONIC_SUBTYPE_PCM,
    WAVE_AMBISONIC_SUBTYPE_IEEE_FLOAT,

} WAVE_SubFormat_Names_e;

/* ************************************************************************** */

typedef enum wave_fcc_e
{
    fcc_fmt_    = 0x666D7420,   //!< Format
    fcc_fact    = 0x66616374,   //!< Facts
    fcc_data    = 0x64617461,   //!< Datas
    fcc_cue_    = 0x63756520,   //!< Cue Points
    fcc_plst    = 0x706C7374,   //!< Playlist
    fcc_labl    = 0x6C61626C,   //!< Label
    fcc_note    = 0x6E6F7465,   //!< Note

    // Used by BWF
    fcc_bext    = 0x62657874,   //!< Broadcast Audio Extension

    // Used by RF64
    fcc_big1    = 0x62696731,   //!< Chunk Size 64
        fcc_ds64= 0x64733634,   //!< Data Size 64
        fcc_r64m= 0x7236346D    //!< Marker

} wave_fcc_e;

/* ************************************************************************** */
#endif // PARSER_WAVE_STRUCT_H
