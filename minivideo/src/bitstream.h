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
 * \file      bitstream.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef BITSTREAM_H
#define BITSTREAM_H

// minivideo headers
#include "typedef.h"
#include "import.h"
#include "bitstream_map.h"

/* ************************************************************************** */

/*!
 * \brief Size used for the bitstream data buffer.
 *
 * Other buffer sizes possible:
 *  32768   //  32 KiB
 *  65536   //  64 KiB
 * 131072   // 128 KiB
 * 262144   // 256 KiB
 * 524288   // 512 KiB
 */
#define BITSTREAM_BUFFER_SIZE 131072

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
 * During a bitstream initialization, a bitstream_map structure can be used. It
 * allows the bitstream to easily move the current bitstream offset from the end
 * of a frame directly to the start of the next frame one.
 */
typedef struct Bitstream_t
{
    FILE *bitstream_file;                //!< File pointer

    BitstreamMap_t *bitstream_map;       //!< Bitstream map of existing A/V samples inside the file
    int32_t bitstream_sample_index;      //!< Id of the A/V sample currently in use

    int64_t bitstream_size;              //!< Total bitstream size (in byte)
    int64_t bitstream_offset;            //!< Current offset into the bitstream (in byte)

    uint8_t *buffer;                     //!< Data buffer pointer
    uint32_t buffer_size;                //!< Data buffer size (in byte)

    uint32_t buffer_offset;              //!< Current offset into the buffer (in bit)
    uint32_t buffer_discarded_bytes;     //!< Current number of byte removed during read ahead

} Bitstream_t;

/* ************************************************************************** */

Bitstream_t *init_bitstream(VideoFile_t *video, BitstreamMap_t *bitstream_map);
void free_bitstream(Bitstream_t **bitstr_ptr);

int buffer_feed_next_sample(Bitstream_t *bitstr);
int buffer_feed_dynamic(Bitstream_t *bitstr, int64_t new_bitstream_offset);

// Bits operations
uint32_t read_bit(Bitstream_t *bitstr);
uint32_t read_bits(Bitstream_t *bitstr, const unsigned int n);
uint64_t read_bits_64(Bitstream_t *bitstr, const unsigned int n);

uint32_t next_bit(Bitstream_t *bitstr);
uint32_t next_bits(Bitstream_t *bitstr, const unsigned int n);

uint32_t read_byte_aligned(Bitstream_t *bitstr);
uint32_t next_byte_aligned(Bitstream_t *bitstr);

int skip_bits(Bitstream_t *bitstr, const unsigned int n);
int rewind_bits(Bitstream_t *bitstr, const unsigned int n);

// Various operations
int64_t bitstream_get_absolute_byte_offset(Bitstream_t *bitstr);
int64_t bitstream_get_absolute_bit_offset(Bitstream_t *bitstr);
int bitstream_goto_offset(Bitstream_t *bitstr, const int64_t n);

/* ************************************************************************** */
#endif // BITSTREAM_H
