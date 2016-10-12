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
 * \file      riff_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_RIFF_STRUCT_H
#define PARSER_RIFF_STRUCT_H

// minivideo headers
#include "../../typedef.h"

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
    int64_t offset_start;   //!< absolute offset of the LIST beginning
    int64_t offset_end;

    // List parameters
    uint32_t dwList;        //!< indicate the type of list we are parsing
    uint32_t dwSize;        //!< LIST size, not including dwFourCC nor dwSize (but including dwFourCC)
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
    int64_t offset_start;   //!< absolute offset of the CHUNK beginning
    int64_t offset_end;

    // Chunk parameters
    uint32_t dwFourCC;      //!< CHUNK identifier
    uint32_t dwSize;        //!< CHUNK size, not including dwFourCC nor dwSize

} RiffChunk_t;

/* ************************************************************************** */

typedef enum RIFF_fcc_e
{
    fcc_RIFF   = 0x52494646,
    fcc_LIST   = 0x4C495354,

    fcc_FFIR   = 0x46464952,
    fcc_TSIL   = 0x5453494C,

    fcc_RIFX   = 0x52494658,    //!< Used by Open-DML AVI
    fcc_RF64   = 0x52463634,    //!< Used by RIFF64

} RIFF_fcc_e;

/* ************************************************************************** */

typedef enum RIFF_common_fcc_e
{
    fcc_JUNK   = 0x4A554E4B

} RIFF_common_fcc_e;

/* ************************************************************************** */
#endif // PARSER_RIFF_STRUCT_H
