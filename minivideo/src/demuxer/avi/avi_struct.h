/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \file      avi_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_AVI_STRUCT_H
#define PARSER_AVI_STRUCT_H

// minivideo headers
#include "../../typedef.h"

/* ************************************************************************** */

/*!
 * \struct AviList_t
 * \brief Basic data structures for AVI.
 *
 * There are 2 types of atoms in AVI files: LIST and CHUNK.
 *
 * - dwFourCC describes the type of the chunk.
 * - dwSize contains the size of the chunk or list, including the first byte
 *   after the dwSize value. In the case of Lists, this includes the 4 bytes
 *   taken by dwFourCC!
 *
 * The offset_start value indicate the start of the LIST, after dwList and
 * dwSize, but not including dwFourCC.
 *
 * The value of dwList can be 'RIFF' ('RIFF-List') or 'LIST' ('List').
 */
typedef struct AviList_t
{
    int64_t offset_start;
    int64_t offset_end;

    // List parameters
    uint32_t dwList;
    uint32_t dwSize;
    uint32_t dwFourCC;

} AviList_t;

/*!
 * \struct AviChunk_t
 * \brief Basic data structures for AVI.
 *
 * Very similar to AviList_t, but:
 * - dwSize contains the size of the chunk, including the first byte after the
 *   dwSize value.
 * - Cannot contain other AviList_t or AviChunk_t elements.
 */
typedef struct AviChunk_t
{
    int64_t offset_start;
    int64_t offset_end;

    // Chunk parameters
    uint32_t dwFourCC;
    uint32_t dwSize;

} AviChunk_t;

/* ************************************************************************** */
/* ************************************************************************** */

typedef struct AviStreamFormat_t
{
    // BITMAPINFOHEADER
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

    // WAVEFORMATEX
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;

} AviStreamFormat_t;

typedef struct AviStreamHeader_t
{
    uint32_t fccType;
    uint32_t fccHandler;
    uint32_t dwFlags;
    uint16_t wPriority;
    uint16_t wLanguage;
    uint32_t dwInitialFrames;
    uint32_t dwScale;
    uint32_t dwRate;                //!< dwRate / dwScale: samples per second
    uint32_t dwStart;
    uint32_t dwLength;              //!< In units above
    uint32_t dwSuggestedBufferSize;
    uint32_t dwQuality;
    uint32_t dwSampleSize;

    uint16_t rcFrame_x;
    uint16_t rcFrame_y;
    uint16_t rcFrame_w;
    uint16_t rcFrame_h;

} AviStreamHeader_t;

typedef struct AviHeader_t
{
    uint32_t dwMicroSecPerFrame;    //!< Frame display rate (or 0)
    uint32_t dwMaxBytesPerSec;      //!< Max. transfer rate
    uint32_t dwPaddingGranularity;  //!< Pad to multiples of this size
    uint32_t dwFlags;               //!< The ever-present flags
    uint32_t dwTotalFrames;         //!< # frames in file: not reliable!
    uint32_t dwInitialFrames;
    uint32_t dwStreams;
    uint32_t dwSuggestedBufferSize;
    uint32_t dwWidth;
    uint32_t dwHeight;

} AviHeader_t;

typedef struct AviIndexEntries_t
{
    uint32_t flags;

    int64_t offset;
    int64_t size;
    int64_t pts;

} AviIndexEntries_t;

//! Structure for AVI video tracks
typedef struct AviTrack_t
{
    unsigned int track_id;
    unsigned int track_indexed;

    AviStreamHeader_t strh;
    AviStreamFormat_t strf;

    unsigned int superindex_count;
    AviIndexEntries_t superindex_entries[16];

    unsigned int index_count;
    AviIndexEntries_t *index_entries;

} AviTrack_t;

//! Structure for AVI video infos
typedef struct avi_t
{
    AviHeader_t avih;

    unsigned int tracks_count;
    AviTrack_t *tracks[16];

    unsigned int movi_offset; //!< DEPRECATED it was a way to stop the parser, and a qbaseoffset for indexation

} avi_t;

/* ************************************************************************** */
/* ************************************************************************** */

typedef enum avi_fcc_e
{
    fcc_RIFF   = 0x52494646,
    fcc_LIST   = 0x4C495354,

    fcc_auds   = 0x61756473,
    fcc_vids   = 0x76696473,
    fcc_txts   = 0x74787473,

    fcc_AVI_   = 0x41564920,            //!< AVI 1.0 chunk (mandatory)
    fcc_AVIX   = 0x41564958,            //!< Open-DML AVI chunk

    fcc_hdrl   = 0x6864726C,            //!< Main AVI Header
        fcc_avih   = 0x61766968,        //!< AVI Header
        fcc_strl   = 0x7374726C,        //!< Stream List: one per stream
            fcc_strh   = 0x73747268,    //!< Stream Header
            fcc_strf   = 0x73747266,    //!< Stream Format
            fcc_strd   = 0x73747264,    //!< Stream Data (optional)
            fcc_strn   = 0x7374726E,    //!< Stream Name (optional)
            fcc_indx   = 0x696E6478,    //!<
            fcc_idx1   = 0x69647831,    //!< AVI 1.0 index (deprecated)

        fcc_odml   = 0x6F646D6C,
            fcc_dmlh   = 0x646D6C68,    //!< Extended AVI Header
            fcc_vprp   = 0x76707270,    //!< Video Properties Header

    fcc_INFO   = 0x494E464F,
        fcc_ISFT   = 0x49534654,        //!< Encoder name?

    fcc_movi   = 0x6D6F7669,            //!< Movie Data
        fcc_rec_   = 0x72656320,        //!< Movie Data

    fcc_JUNK   = 0x4A554E4B

} avi_fcc_e;


typedef enum avi_fcc_codecs_e
{
    // Audio

    // Video
    fcc_xvid = 0x78766964, //!< Mpeg 4 part 2
    fcc_XVID = 0x58564944,
    fcc_FMP4 = 0x464D5034,
    fcc_DIVX = 0x44495658, //!< DivX 4 > 6
    fcc_DX50 = 0x44583530, //!< DivX 5.x
    fcc_MP42 = 0x4D503432, //!< DivX 3.11
    fcc_dvsd = 0x64767364  //!< DV

} avi_fcc_codecs_e;

/*!
 * Good ressource about these tags (search for WAVE_FORMAT_):
 * http://www.videolan.org/developers/vlc/doc/doxygen/html/vlc__codecs_8h_source.html
 */
typedef enum wFormatTag_e
{
    wftag_UNKNOWN = 0,
    wftag_PCM    = 0x0001,
    wftag_MP1    = 0x0050,
    wftag_MP3    = 0x0055,
    wftag_AC3    = 0x2000,
    wftag_DTS    = 0x2001,
    wftag_AAC    = 0x00FF,
    wftag_WAV    = 0xFFFE

} wFormatTag_e;

typedef enum avi_flags_dwFlags_e
{
    AVIF_HASINDEX        = 0x00000010, //!< Index at end of file?
    AVIF_MUSTUSEINDEX    = 0x00000020,
    AVIF_ISINTERLEAVED   = 0x00000100,
    AVIF_TRUSTCKTYPE     = 0x00000800, //!< Use CKType to find key frames?
    AVIF_WASCAPTUREFILE  = 0x00010000,
    AVIF_COPYRIGHTED     = 0x00020000

} avi_flags_dwFlags_e;

typedef enum avi_flags_dwCaps_e
{
    AVIFC_CANREAD        = 0x00000001,
    AVIFC_CANWRITE       = 0x00000002,
    AVIFC_ALLKEYFRAMES   = 0x00000010,
    AVIFC_NOCOMPRESSION  = 0x00000020

} avi_flags_dwCaps_e;

typedef enum avi_flags_indexes_e
{
    AVIIF_LIST           = 0x00000001, //!< Chunk is a 'LIST'.
    AVIIF_KEYFRAME       = 0x00000010, //!< This frame is a key frame.
    AVIIF_NOTIME         = 0x00000100, //!< This frame doesn't take any time.
    AVIIF_COMPUSE        = 0x0FFF0000, //!< These bits are for compressor use.

    AVIIF_FIRSTPART      = 0x00000000, //!< No infos...
    AVIIF_LASTPART       = 0x00000000  //!< No infos...

} avi_flags_indexes_e;

typedef enum bIndexType_e
{
    AVI_INDEX_OF_INDEXES = 0x00,
    AVI_INDEX_OF_CHUNKS  = 0x01,
    AVI_INDEX_IS_DATA    = 0x80

} bIndexType_e;

typedef enum bIndexSubType_e
{
    AVI_INDEX_NOSUBTYPE  = 0x00,
    AVI_INDEX_2FIELD     = 0x01

} bIndexSubType_e;

/* ************************************************************************** */
#endif // PARSER_AVI_STRUCT_H
