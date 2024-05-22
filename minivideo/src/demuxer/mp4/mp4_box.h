/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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

void write_box_header(Mp4Box_t *box_header, FILE *xml,
                      const char *title = nullptr, const char *additional = nullptr);

/* ************************************************************************** */

int parse_unknown_box(Bitstream_t *bitstr, Mp4Box_t *box_header, FILE *xml);

/* ************************************************************************** */

uint32_t read_mp4_uint(Bitstream_t *bitstr, int bits, FILE *xml, const char *name = nullptr);

bool read_mp4_flag(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);
uint8_t read_mp4_uint8(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);
uint16_t read_mp4_uint16(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);
uint64_t read_mp4_uint64(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);
uint32_t read_mp4_uint32(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);

float read_mp4_f1616(Bitstream_t *bitstr, int precision, FILE *xml, const char *name);

uint32_t read_mp4_fcc(Bitstream_t *bitstr, FILE *xml, const char *name = nullptr);
uint8_t *read_mp4_data(Bitstream_t *bitstr, int bytes, FILE *xml, const char *name = nullptr);
char *read_mp4_string(Bitstream_t *bitstr, int bytes, FILE *xml, const char *name = nullptr);

/* ************************************************************************** */

int jumpy_mp4(Bitstream_t *bitstr, Mp4Box_t *parent, Mp4Box_t *current);

/* ************************************************************************** */
#endif // PARSER_MP4_BOX_H
