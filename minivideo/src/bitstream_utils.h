/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \file      bitstream_utils.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef BITSTREAM_UTILS_H
#define BITSTREAM_UTILS_H

// minivideo headers
#include "typedef.h"
#include "bitstream.h"

/* ************************************************************************** */

// Debug operations
void bitstream_print_stats(Bitstream_t *bitstr);
void bitstream_print_buffer(Bitstream_t *bitstr);
void bitstream_print_absolute_byte_offset(Bitstream_t *bitstr);
void bitstream_print_absolute_bit_offset(Bitstream_t *bitstr);

// Data boundaries
bool bitstream_check_alignment(Bitstream_t *bitstr);
bool bitstream_force_alignment(Bitstream_t *bitstr);
bool more_bitstream_data(Bitstream_t *bitstr);

// H.264 specifics
bool h264_more_rbsp_data(Bitstream_t *bitstr);
bool h264_more_rbsp_trailing_data(Bitstream_t *bitstr);

// Endianness
inline uint16_t endian_flip_16(uint16_t src);
inline uint16_t endian_flip_cut_16(uint16_t src, const int n);
inline uint32_t endian_flip_32(uint32_t src);
inline uint32_t endian_flip_cut_32(uint32_t src, const int n);
inline uint64_t endian_flip_64(uint64_t src);
inline uint64_t endian_flip_cut_64(uint64_t src, const int n);

/* ************************************************************************** */
#endif /* BITSTREAM_UTILS_H */
