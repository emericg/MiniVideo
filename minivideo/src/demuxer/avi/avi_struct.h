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
 * \file      avi_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_AVI_STRUCT_H
#define PARSER_AVI_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include "../../minivideo_codecs.h"
#include "../../minivideo_containers.h"

#include <cstdio>

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
    bool run;                   //!< A convenient way to stop the parser from any sublevel

    ContainerProfiles_e profile;

    AviHeader_t avih;

    unsigned int tracks_count;
    AviTrack_t *tracks[16];

    unsigned int movi_offset;   //!< DEPRECATED it was a way to stop the parser, and a qbaseoffset for indexation

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} avi_t;

/* ************************************************************************** */

typedef enum avi_fcc_e
{
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

    fcc_movi   = 0x6D6F7669,            //!< Movie Data
        fcc_rec_   = 0x72656320         //!<

} avi_fcc_e;

//! TwoCC used by indexes
typedef enum avi_tcc_e
{
    tcc_AC            = 0x4143, //!< AC : Unknow entry ?
    tcc_DIBbits       = 0x6462, //!< db : Device Independent Bitmap (uncompressed video frame)
    tcc_DIBcompressed = 0x6463, //!< dc : Device Independent Bitmap (video frame)
    tcc_INDEX         = 0x6978, //!< ix : Index entry
    tcc_PALchange     = 0x7063, //!< pc : Palette Change
    tcc_TEXT          = 0x7478, //!< tx : Subtitles entry
    tcc_SUB           = 0x7362, //!< sb : Subtitles entry ?
    tcc_WAVEbytes     = 0x7762, //!< wb : Wave Bytes (audio frame)
    tcc_xx            = 0x7878  //!< xx : Unknow entry ? Let's assume that its the same as 'dc'.

} avi_tcc_e;

//! Flags for dwFlags member of AVIMAINHEADER
typedef enum avi_flags_avih_e
{
    AVIF_HASINDEX        = 0x00000010, //!< Index at end of file?
    AVIF_MUSTUSEINDEX    = 0x00000020,
    AVIF_ISINTERLEAVED   = 0x00000100,
    AVIF_TRUSTCKTYPE     = 0x00000800, //!< Use CKType to find key frames?
    AVIF_WASCAPTUREFILE  = 0x00010000,
    AVIF_COPYRIGHTED     = 0x00020000

} avi_flags_avih_e;

typedef enum avi_flags_sf_e
{
    AVISF_DISABLED         = 0x00000001,
    AVISF_VIDEO_PALCHANGES = 0x00010000

} avi_flags_sf_e;

//! Flags for dwFlags member of AVI indexes
typedef enum avi_flags_dwCaps_e
{
    AVIFC_CANREAD        = 0x00000001,
    AVIFC_CANWRITE       = 0x00000002,
    AVIFC_ALLKEYFRAMES   = 0x00000010,
    AVIFC_NOCOMPRESSION  = 0x00000020

} avi_flags_dwCaps_e;

//! Flags for dwFlags member of old 'idx1' AVI indexes
typedef enum avi_flags_idx1_e
{
    AVIIF_LIST           = 0x00000001, //!< Chunk is a 'LIST'.
    AVIIF_KEYFRAME       = 0x00000010, //!< This frame is a key frame.
    AVIIF_NO_TIME        = 0x00000100, //!< This frame doesn't take any time.
    AVIIF_COMPRESSOR     = 0x0FFF0000, //!< These bits are for compressor use.

    AVIIF_FIRSTPART      = 0x00000000, //!< No infos...
    AVIIF_LASTPART       = 0x00000000  //!< No infos...

} avi_flags_idx1_e;

//! Flags for standart 'indx' AVI indexes
typedef enum bIndexType_e
{
    AVI_INDEX_OF_INDEXES      = 0x00,
    AVI_INDEX_OF_CHUNKS       = 0x01,
    AVI_INDEX_OF_TIMED_CHUNKS = 0x02,
    AVI_INDEX_OF_SUB_2FIELD   = 0x03,
    AVI_INDEX_IS_DATA         = 0x80

} bIndexType_e;

//! Sub-flags for standart 'indx' AVI indexes
typedef enum bIndexSubType_e
{
    AVI_INDEX_NOSUBTYPE  = 0x00,
    AVI_INDEX_2FIELD     = 0x01

} bIndexSubType_e;

/* ************************************************************************** */
#endif // PARSER_AVI_STRUCT_H
