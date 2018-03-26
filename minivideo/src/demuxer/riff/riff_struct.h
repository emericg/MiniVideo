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
 * \file      riff_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_RIFF_STRUCT_H
#define PARSER_RIFF_STRUCT_H

// C standard libraries
#include <cstdint>

/* ************************************************************************** */

/*!
 * \brief Basic container structure of RIFF formated files.
 *
 * There are 2 types of atoms in RIFF formated files:
 * - LIST can only contains other LIST or CHUNK.
 * - CHUNK can only contains datas.
 *
 * Common value of dwList can be 'RIFF' ('RIFF-List') or 'LIST' ('List').
 */
typedef struct RiffList_t
{
    int64_t offset_start;   //!< Absolute offset of the LIST beginning
    int64_t offset_end;

    // List parameters
    uint32_t dwList;        //!< Indicate the type of list we are parsing
    uint32_t dwSize;        //!< LIST payload size (NOT including dwFourCC nor dwSize, but including dwFourCC)
    uint32_t dwFourCC;      //!< LIST identifier

} RiffList_t;

/*!
 * \brief Basic data structure of RIFF formated files.
 *
 * There are 2 types of atoms in RIFF formated files:
 * - LIST can only contains other LIST or CHUNK.
 * - CHUNK can only contains datas.
 */
typedef struct RiffChunk_t
{
    int64_t offset_start;   //!< Absolute offset of the CHUNK beginning
    int64_t offset_end;

    // Chunk parameters
    uint32_t dwFourCC;      //!< CHUNK identifier
    uint32_t dwSize;        //!< CHUNK payload size (NOT including dwFourCC nor dwSize)

} RiffChunk_t;

/* ************************************************************************** */

typedef enum RIFF_fcc_e
{
    fcc_RIFF   = 0x52494646,
    fcc_LIST   = 0x4C495354,

    fcc_FFIR   = 0x46464952,    //!< Error handling (RIFF and LIST backward...)
    fcc_TSIL   = 0x5453494C,

    // RIFF Formats
    fcc_RIFX   = 0x52494658,    //!< Used by Open-DML AVI
    fcc_WAVE   = 0x57415645,    //!< WAVE audio files
    fcc_RF64   = 0x52463634,    //!< RF64 (WAVE extension)
    fcc_BW64   = 0x42573634,    //!< BW64 (WAVE extension)

} RIFF_fcc_e;

/* ************************************************************************** */

typedef enum RIFF_common_fcc_e
{
    fcc_JUNK   = 0x4A554E4B,
    fcc_PAD    = 0x50414420,

    fcc_INFO   = 0x494E464F,
        fcc_IARL   = 0x4941524C,   //!< Archive Location
        fcc_IART   = 0x49415254,   //!< Artist
        fcc_ICMS   = 0x49434D53,   //!< Commissioned
        fcc_ICMT   = 0x49434D74,   //!< Comments
        fcc_ICOP   = 0x49434F50,   //!< Copyright
        fcc_ICRD   = 0x49435244,   //!< Creation date
        fcc_ICRP   = 0x49435250,   //!< Cropped
        fcc_IDIM   = 0x4944494D,   //!< Dimensions
        fcc_IDIT   = 0x49444954,   //!< Digitization Time
        fcc_IDPI   = 0x49445049,   //!< Dots Per Inch
        fcc_IENG   = 0x49454E47,   //!< Engineer
        fcc_IGNR   = 0x49474E52,   //!< Genre
        fcc_IKEY   = 0x494B4559,   //!< Keywords
        fcc_ILGT   = 0x494C4754,   //!< Lightness
        fcc_IMED   = 0x494D4544,   //!< Medium
        fcc_INAM   = 0x494E414d,   //!< Name
        fcc_IPLT   = 0x49504C54,   //!< Palette Setting
        fcc_IPRD   = 0x49505244,   //!< Product
        fcc_ISBJ   = 0x4953424A,   //!< Subject
        fcc_ISFT   = 0x49534654,   //!< Software
        fcc_ISHP   = 0x49534850,   //!< Sharpness
        fcc_ISMP   = 0x49534D50,   //!< SMPTE time code of digitization
        fcc_ISRC   = 0x49535243,   //!< Source
        fcc_ISRF   = 0x49535246,   //!< Source Form
        fcc_ITCH   = 0x49544348,   //!< Technician

    fcc_CSET   = 0x43534554        //!< Character Set

} RIFF_common_fcc_e;

/* ************************************************************************** */
#endif // PARSER_RIFF_STRUCT_H
