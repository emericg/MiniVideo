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
 * \file      riff_common.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef PARSER_RIFF_COMMON_H
#define PARSER_RIFF_COMMON_H

// minivideo headers
#include "riff_struct.h"
#include "../../import.h"
#include "../../bitstream.h"

/* ************************************************************************** */

/*!
 * Character Set Chunk.
 */
typedef struct CsetChunk_t
{
    uint32_t wCodePage;
    uint32_t wCountryCode;
    uint32_t wLanguage;
    uint32_t wDialect;

} CsetChunk_t;

/* ************************************************************************** */

int parse_JUNK(Bitstream_t *bitstr, RiffChunk_t *JUNK_header, FILE *xml);

int parse_PAD(Bitstream_t *bitstr, RiffChunk_t *PAD_header, FILE *xml);

int parse_CSET(Bitstream_t *bitstr, RiffChunk_t *CEST_header, CsetChunk_t *cset, FILE *xml);

/* ************************************************************************** */
#endif // PARSER_RIFF_COMMON_H
