/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
 * \file      ebml.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_EBML_H
#define PARSER_EBML_H

// minivideo headers
#include "mkv_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

//! EBML element structure
typedef struct EbmlElement_t
{
    int64_t offset_start;   //!< Absolute position of the first byte of this element
    int64_t offset_end;     //!< Absolute position of the last byte of this element

    uint32_t eid;           //!< An EBML ID
    uint32_t eid_size;      //!< Size of the 'EBML ID' field ('s_ID')

    int64_t size;           //!< Element size in bytes
    uint32_t size_size;     //!< Size of the 'size' field ('s_size')

} EbmlElement_t;

/* ************************************************************************** */

int parse_ebml_element(Bitstream_t *bitstr, EbmlElement_t *ebml_element);
void print_ebml_element(EbmlElement_t *ebml_element);

void write_ebml_element(EbmlElement_t *ebml_element, FILE *xml);
void write_ebml_element_title(EbmlElement_t *ebml_element, FILE *xml, const char *title);

/* ************************************************************************** */

uint64_t read_ebml_data_uint(Bitstream_t *bitstr, int size);
int64_t read_ebml_data_int(Bitstream_t *bitstr);
double read_ebml_data_float(Bitstream_t *bitstr);

uint8_t *read_ebml_data_string(Bitstream_t *bitstr, int size);
uint8_t *read_ebml_data_binary(Bitstream_t *bitstr);
int read_ebml_data_date(Bitstream_t *bitstr);

/* ************************************************************************** */

int ebml_parse_unknown(Bitstream_t *bitstr, EbmlElement_t *ebml_element, FILE *xml);

/* ************************************************************************** */

int jumpy_mkv(Bitstream_t *bitstr, EbmlElement_t *parent, EbmlElement_t *current);

/* ************************************************************************** */
#endif // PARSER_EBML_H
