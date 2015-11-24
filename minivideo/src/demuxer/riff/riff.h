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
 * \file      riff.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_RIFF_H
#define PARSER_RIFF_H

// minivideo headers
#include "riff_struct.h"
#include "../../import.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int parse_list_header(Bitstream_t *bitstr, RiffList_t *list_header);

void print_list_header(RiffList_t *list_header);

int skip_list(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffList_t *list_header_child);

int parse_chunk_header(Bitstream_t *bitstr, RiffChunk_t *chunk_header);

void print_chunk_header(RiffChunk_t *chunk_header);

int skip_chunk(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffChunk_t *chunk_header_child);

/* ************************************************************************** */
#endif // PARSER_RIFF_H
