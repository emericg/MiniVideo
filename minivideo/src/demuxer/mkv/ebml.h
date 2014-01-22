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
#include "../../bitstream.h"

/* ************************************************************************** */

int ebml_parse_header(Bitstream_t *bitstr);

uint32_t read_ebml_eid(Bitstream_t *bitstr);
uint64_t read_ebml_size(Bitstream_t *bitstr);
uint64_t read_ebml_data_uint(Bitstream_t *bitstr);
int64_t read_ebml_data_int(Bitstream_t *bitstr);

/* ************************************************************************** */
#endif /* PARSER_EBML_H */
