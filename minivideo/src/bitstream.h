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
 * \file      bitstream.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef BITSTREAM_H
#define BITSTREAM_H

// minivideo headers
#include "minivideo_typedef.h"
#include "bitstream_map.h"

/* ************************************************************************** */

/*!
 * \struct Bitstream_t
 * \brief Bitstream structure.
 * \todo Massive cleanup of this API, need to address the bit/byte ambiguity of the variable names and function arguments.
 *
 * The "Bitstream_t" structure and associated APU is used to map a file to a
 * bitstream data buffer that can be read bit by bit instead of byte by byte.
 * That capability is requiered to extract compressed data that aren't aligned
 * with bytes boundaries.
 *
 * Data can be read from 1 to 64 unaligned bits from a file. These datas are
 * bufferized by slices of size defined by the "BITSTREAM_BUFFER_SIZE" variable.
 *
 * Size and offset of the bitstream are stored using int64_t. This structure
 * should be able to handle files up to 1073741824 GiB if "Large File Support"
 * is enabled during compilation with -D_LARGEFILE64_SOURCE. Overwise only files
 * of 2 GiB will work.
 *
 * During a bitstream initialization, a MediaStream_t structure can be used. It
 * allows the bitstream to easily move the current bitstream offset from the end
 * of a frame directly to the start of the next frame one.
 */
typedef struct Bitstream_t
{
    FILE *bitstream_file;               //!< File pointer
    int64_t bitstream_size;             //!< Bitstream size (in byte)
    int64_t bitstream_offset;           //!< Current offset into the bitstream (in byte)

    uint32_t buffer_size_saved;         //!< Data buffer size (in byte)

    uint8_t *buffer;                    //!< Current data buffer
    uint32_t buffer_size;               //!< Current data buffer size (in byte)
    uint32_t buffer_offset;             //!< Current offset into the buffer (in bit)
    uint32_t buffer_discarded_bytes;    //!< Current number of byte(s) removed during NAL Unit read ahead

    // (if used with a sample track)
    MediaStream_t *sample_map;          //!< Bitstream map of existing A/V samples inside the file
    uint32_t sample_index;              //!< ID of the A/V sample currently in use

} Bitstream_t;

/* ************************************************************************** */

Bitstream_t *init_bitstream0(MediaFile_t *media, int64_t bitstream_offset, uint32_t buffer_size);
Bitstream_t *init_bitstream(MediaFile_t *media, MediaStream_t *stream);
void free_bitstream(Bitstream_t **bitstr_ptr);

int buffer_feed_manual(Bitstream_t *bitstr, int64_t bitstream_offset, int64_t size);
int buffer_feed_dynamic(Bitstream_t *bitstr, int64_t new_bitstream_offset);

// Bits operations
uint32_t read_bit(Bitstream_t *bitstr);
uint32_t read_bits(Bitstream_t *bitstr, const unsigned int n);
uint64_t read_bits_64(Bitstream_t *bitstr, const unsigned int n);

uint32_t next_bit(Bitstream_t *bitstr);
uint32_t next_bits(Bitstream_t *bitstr, const unsigned int n);

uint32_t read_byte_aligned(Bitstream_t *bitstr);
uint32_t next_byte_aligned(Bitstream_t *bitstr);

// Move into the bitstream
int skip_bits(Bitstream_t *bitstr, const unsigned int n);
int rewind_bits(Bitstream_t *bitstr, const unsigned int n);
int bitstream_goto_offset(Bitstream_t *bitstr, const int64_t n);

// Various operations
int64_t bitstream_get_full_size(Bitstream_t *bitstr);
int64_t bitstream_get_absolute_byte_offset(Bitstream_t *bitstr);
int64_t bitstream_get_absolute_bit_offset(Bitstream_t *bitstr);

/* ************************************************************************** */
#endif // BITSTREAM_H
