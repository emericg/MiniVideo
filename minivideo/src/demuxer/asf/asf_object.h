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
 * \file      asf_object.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_ASF_OBJECT_H
#define PARSER_ASF_OBJECT_H

// minivideo headers
#include "asf_struct.h"
#include "../../bitstream.h"
#include "../../minivideo_uuid.h"

/* ************************************************************************** */

uint64_t WindowsTickToUnixSeconds(int64_t windowsTicks);

/* ************************************************************************** */

int read_asf_guid(Bitstream_t *bitstr, uint8_t guid[16],
                  FILE *xml, const char *name);

int8_t read_asf_int8(Bitstream_t *bitstr, FILE *xml, const char *name);

int16_t read_asf_int16(Bitstream_t *bitstr, FILE *xml, const char *name);

int32_t read_asf_int32(Bitstream_t *bitstr, FILE *xml, const char *name);

int64_t read_asf_int64(Bitstream_t *bitstr, FILE *xml, const char *name);

int64_t read_asf_int(Bitstream_t *bitstr, const int n,
                     FILE *xml, const char *name);

uint8_t *read_asf_binary(Bitstream_t *bitstr, const int sizeBytes,
                         FILE *xml, const char *name);

char *read_asf_string_ascii(Bitstream_t *bitstr, const int sizeChar,
                            FILE *xml, const char *name);

char *read_asf_string_utf16(Bitstream_t *bitstr, const int sizeChar,
                            FILE *xml, const char *name);

/* ************************************************************************** */

int parse_asf_object(Bitstream_t *bitstr, AsfObject_t *asf_object);

void print_asf_object(AsfObject_t *asf_object);

void write_asf_object(AsfObject_t *asf_object, FILE *xml,
                      const char *title = nullptr, const char *additional = nullptr);

/* ************************************************************************** */

int parse_unknown_object(Bitstream_t *bitstr, AsfObject_t *asf_object, FILE *xml);

/* ************************************************************************** */

int jumpy_asf(Bitstream_t *bitstr, AsfObject_t *parent, AsfObject_t *current);

/* ************************************************************************** */
#endif // PARSER_ASF_OBJECT_H
