/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      mp4_box.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef PARSER_MP4_BOX_H
#define PARSER_MP4_BOX_H

// minivideo headers
#include "mp4_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

uint64_t LabVIEWTimeToUnixSeconds(int64_t LabVIEWTime);

/* ************************************************************************** */

int parse_box_header(Bitstream_t *bitstr, Mp4Box_t *box_header);

int parse_fullbox_header(Bitstream_t *bitstr, Mp4Box_t *box_header);

void print_box_header(Mp4Box_t *box_header);

void write_box_header(Mp4Box_t *box_header, FILE *xml);

/* ************************************************************************** */

int parse_unknown_box(Bitstream_t *bitstr, Mp4Box_t *box_header, FILE *xml);

/* ************************************************************************** */

int jumpy_mp4(Bitstream_t *bitstr, Mp4Box_t *parent, Mp4Box_t *current);

/* ************************************************************************** */
#endif // PARSER_MP4_BOX_H
