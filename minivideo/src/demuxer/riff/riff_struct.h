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
 * \struct RiffList_t
 * \brief Basic data structures for RIFF formated files.
 *
 * There are 2 types of atoms in RIFF formated files: LIST and CHUNK.
 *
 * - 'dwFourCC' describes the type of the chunk.
 * - 'dwSize' contains the size of the chunk or list, including the first byte
 *   after the dwSize value. In the case of Lists, this includes the 4 bytes
 *   taken by the 'dwFourCC' field.
 *
 * The offset_start value indicate the start of the LIST, after dwList and
 * dwSize, but not including dwFourCC.
 *
 * The value of dwList can be 'RIFF' ('RIFF-List') or 'LIST' ('List').
 */
typedef struct RiffList_t
{
    int64_t offset_start;
    int64_t offset_end;

    // List parameters
    uint32_t dwList;
    uint32_t dwSize;
    uint32_t dwFourCC;

} RiffList_t;

/*!
 * \struct RiffChunk_t
 * \brief Basic data structures for RIFF formated files.
 *
 * Very similar to RiffList_t, but:
 * - 'dwSize' contains the size of the chunk, including the first byte after the
 *   'dwSize' value.
 * - Cannot contain other RiffList_t nor RiffChunk_t elements.
 */
typedef struct RiffChunk_t
{
    int64_t offset_start;
    int64_t offset_end;

    // Chunk parameters
    uint32_t dwFourCC;
    uint32_t dwSize;

} RiffChunk_t;

/* ************************************************************************** */

typedef enum RIFF_fcc_e
{
    fcc_RIFF   = 0x52494646,
    fcc_LIST   = 0x4C495354,

    fcc_RIFX   = 0x52494658,
    fcc_FFIR   = 0x46464952

} RIFF_fcc_e;

/* ************************************************************************** */
#endif // PARSER_RIFF_STRUCT_H
