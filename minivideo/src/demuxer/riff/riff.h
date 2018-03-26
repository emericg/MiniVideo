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
 * \file      riff.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_RIFF_H
#define PARSER_RIFF_H

// minivideo headers
#include "riff_struct.h"
#include "riff_common.h"
#include "../../import.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int parse_list_header(Bitstream_t *bitstr, RiffList_t *list_header);

void print_list_header(RiffList_t *list_header);

void write_list_header(RiffList_t *list_header, FILE *xml,
                       const char *title = nullptr, const char *additional = nullptr);

int skip_list(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffList_t *list_header_child);

int parse_chunk_header(Bitstream_t *bitstr, RiffChunk_t *chunk_header);

void print_chunk_header(RiffChunk_t *chunk_header);

void write_chunk_header(RiffChunk_t *chunk_header, FILE *xml,
                        const char *title = nullptr, const char *additional = nullptr);

int skip_chunk(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffChunk_t *chunk_header_child);

/* ************************************************************************** */

int parse_unkn_list(Bitstream_t *bitstr, RiffList_t *unkn_header, FILE *xml);

int parse_unkn_chunk(Bitstream_t *bitstr, RiffChunk_t *unkn_header, FILE *xml);

/* ************************************************************************** */

int jumpy_riff(Bitstream_t *bitstr, RiffList_t *parent, int64_t offset_end);

/* ************************************************************************** */
#endif // PARSER_RIFF_H
