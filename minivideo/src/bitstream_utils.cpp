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
 * \file      bitstream_utils.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "bitstream.h"
#include "minitraces.h"

// C POSIX library
#ifndef _MSC_VER
#include <unistd.h>
#endif

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Print various statistics about the current bitstream and its buffer.
 * \param *bitstr The bitstream to check.
 */
void bitstream_print_stats(Bitstream_t *bitstr)
{
    TRACE_1(BITS, "<b>   bitstream offset  (byte) : " BLD_BLUE "%lli" CLR_RESET " / " BLD_BLUE "%lli" CLR_RESET, bitstr->bitstream_offset, bitstr->bitstream_size);
    TRACE_1(BITS, "<b>   buffer offset     (byte) : " BLD_BLUE "%u" CLR_RESET " / " BLD_BLUE "%u" CLR_RESET, bitstr->buffer_offset/8, bitstr->buffer_size);
    TRACE_1(BITS, "<b>   buffer offset     (bit)  : " BLD_BLUE "%u" CLR_RESET " / " BLD_BLUE "%u" CLR_RESET, bitstr->buffer_offset, bitstr->buffer_size*8);
    TRACE_1(BITS, "<b>   buffer offset % 8 (bit)  : " BLD_BLUE "%u" CLR_RESET, bitstr->buffer_offset%8);
    TRACE_1(BITS, "<b>   buffer discarded byte    : " BLD_BLUE "%u" CLR_RESET, bitstr->buffer_discarded_bytes);
}

/* ************************************************************************** */

/*!
 * \brief Print the bitstream buffer content.
 * \param *bitstr The bitstream to use.
 *
 * Print the bitstream buffer content, by chunk of 21*16 to be easily readable.
 */
void bitstream_print_buffer(Bitstream_t *bitstr)
{
    TRACE_1(BITS, "<b> " BLD_BLUE "bitstream_print_buffer()" CLR_RESET);

    unsigned int i = 0, j = 0;
    unsigned int row = 21, line = 16;

    while (i < (bitstr->buffer_size/row))
    {
        for (j = 0; j < row; j++)
        {
            printf("%02X ", bitstr->buffer[j + i*row]);
        }

        if (i != 0 && (i % (line-1)) == 0)
        {
            printf("\n");
        }

        printf("\n");
        i++;
    }

    for (j = 0; j < (bitstr->buffer_size % row); j++)
    {
        printf("%02X ", bitstr->buffer[j + i*row]);
    }

    printf("\n");
}

/* ************************************************************************** */

/*!
 * \brief Print the absolute byte offset into the bitstream.
 * \param *bitstr The bitstream to use.
 */
void bitstream_print_absolute_byte_offset(Bitstream_t *bitstr)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "Current byte offset is %lli" CLR_RESET,
               bitstream_get_absolute_byte_offset(bitstr));
}

/* ************************************************************************** */

/*!
 * \brief Print the absolute bit offset into the bitstream.
 * \param *bitstr The bitstream to use.
 */
void bitstream_print_absolute_bit_offset(Bitstream_t *bitstr)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "Current bit offset is %lli" CLR_RESET,
               bitstream_get_absolute_bit_offset(bitstr));
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Determine if the bitstream is on a byte boundary.
 * \param *bitstr The bitstream to check.
 * \return true if the bitstream is byte-aligned, false otherwise.
 */
bool bitstream_check_alignment(Bitstream_t *bitstr)
{
    bool alignment = false;

    if ((bitstr->buffer_offset % 8) == 0)
    {
        TRACE_1(BITS, "<b> " BLD_BLUE "Bitstream is aligned" CLR_RESET " at current byte offset %lli",
                bitstream_get_absolute_byte_offset(bitstr));
        alignment = true;
    }
    else
    {
        TRACE_1(BITS, "<b> " BLD_BLUE "Bitstream is NOT aligned" CLR_RESET " at current byte offset %lli + %i bit(s)",
                bitstream_get_absolute_byte_offset(bitstr), bitstream_get_absolute_bit_offset(bitstr)%8);
    }

    return alignment;
}

/* ************************************************************************** */

/*!
 * \brief Determine if the bitstream is on a byte boundary.
 * \param *bitstr The bitstream to align.
 * \return true if the bitstream has been aligned on byte boundarie, false otherwise.
 *
 * Warning: Up to 7 bits will be lost in the process.
 * The alignment is done forward, to avoid re-read some bits, and to avoid
 * backward buffer reallocation.
 */
bool bitstream_force_alignment(Bitstream_t *bitstr)
{
    TRACE_1(BITS, "<b> " BLD_BLUE "bitstream_force_alignment()" CLR_RESET);
    bool alignment = true;

    // Check if bitstream is already aligned
    if ((bitstr->buffer_offset % 8) != 0)
    {
        // Compute bit alignment
        int bit_alignment = 8 - (bitstr->buffer_offset % 8);
        TRACE_2(BITS, "<b> +%i bit(s) alignment needed", bit_alignment);

        // Load next data if needed
        if ((bitstr->buffer_offset + bit_alignment) > (bitstr->buffer_size * 8))
        {
            if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
            {
                TRACE_ERROR(BITS, "<b> Bitstream cannot be aligned");
                alignment = false;
            }
        }
        else
        {
            // Stats
            bitstream_print_stats(bitstr);

            // Alignment operation
            bitstr->buffer_offset += bit_alignment;
            TRACE_1(BITS, "<b> Bitstream now aligned: %i bit(s) offset applied", bit_alignment);
        }
    }

    return alignment;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Determine if there is additional data in the bitstream.
 * \param *bitstr The bitstream to check.
 * \return true if at least one byte of data follow in the bitstream, false otherwise.
 *
 * This function do not read bitstream content, but just check if the current
 * offset into the bitstream is smaller than the bitstream size.
 * More than 8 bits must follow in order to return true.
 */
bool more_bitstream_data(Bitstream_t *bitstr)
{
    TRACE_2(BITS, "<b> " BLD_BLUE "more_bitstream_data()" CLR_RESET);
    bool retcode = true;

    if (bitstr->sample_map == NULL)
    {
        if ((bitstr->bitstream_offset + (bitstr->buffer_offset / 8)) >= bitstr->bitstream_size)
        {
            TRACE_2(BITS, "<b> No more data in the bitstream!");
            retcode = false;
        }
    }
    else
    {
        if ((uint32_t)bitstr->sample_index == bitstr->sample_map->sample_count)
        {
            if ((bitstr->buffer_size - bitstr->buffer_offset) < 8)
            {
                TRACE_2(BITS, "<b> No more data in the bitstream!");
                retcode = false;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief h264_rbsp_trailing_bits
 * \param *bitstr The bitstream to check.
 * \return true everything went as expected, false otherwise.
 *
 * More informations available at 7.3.2.11 'RBSP trailing bits syntax' of H.264 standard.
 */
bool h264_rbsp_trailing_bits(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "h264_rbsp_trailing_bits()" CLR_RESET);
    bool retcode = true;

    if (read_bit(bitstr) == 1) // rbsp_stop_one_bit
    {
        while (bitstream_check_alignment(bitstr) == false)
        {
            if (read_bit(bitstr) != 0) // rbsp_alignment_zero_bit
            {
                retcode = false;
                break;
            }
        }
    }
    else
    {
        retcode = false;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Determine if there is additional data in the RBSP.
 * \param *bitstr The bitstream to check.
 * \return true if more data have been found in the RBSP, false otherwise.
 *
 * This function check for the immediate presence of a start_code_prefix, then
 * for a trailing bits structure (1 rbsp_stop_one_bit + less than 7 rbsp_alignment_zero_bit).
 * In this function trailing bits are NOT considered as data.
 *
 * More informations available at 7.4.1 'NAL unit semantics' of H.264 standard.
 */
bool h264_more_rbsp_data(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "h264_more_rbsp_data()" CLR_RESET);
    bool retcode = true;

    if (bitstr->sample_map == NULL)
    {
        if (more_bitstream_data(bitstr))
        {
            // Load next data if needed
            if ((bitstr->buffer_offset + 48) > (bitstr->buffer_size * 8))
            {
                if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
                {
                    return FAILURE;
                }
            }

            // Save current position
            unsigned int buffer_offset_saved = bitstr->buffer_offset;

            // Try to find a start_code_prefix in the next 6 byte, which would mean that we are near the end of the NAL Unit
            unsigned int current_byte = 0;
            unsigned int search_window = 6;
            unsigned int startcode_pos = 0;
            unsigned int startcode_size = 0;

            bitstream_force_alignment(bitstr);

            while (startcode_pos == 0 && search_window > 0)
            {
                current_byte = read_byte_aligned(bitstr);

                if (current_byte == 0)
                {
                    startcode_size++;
                }
                else if ((startcode_size > 1) && (current_byte == 0x01))
                {
                    startcode_size++;
                    startcode_pos = bitstream_get_absolute_byte_offset(bitstr) - startcode_size;
                    TRACE_3(BITS, "<b> %uB start_code_prefix found at byte offset %u", startcode_size, startcode_pos);
                }
                else
                {
                    startcode_size = 0;
                }

                search_window--;
            }

            // Rewind to original position
            bitstr->buffer_offset = buffer_offset_saved;

            // Try to find trailing bits
            if (startcode_pos > 0)
            {
                // Count bits left in NAL Unit
                unsigned int bits_left = startcode_pos*8 - buffer_offset_saved;

                if (bits_left < 8)
                {
                    TRACE_3(BITS, "<b> %u bits left in current NAL Unit! Checking for trailing data.", bits_left);

                    // Count trailing bits
                    if (next_bits(bitstr, bits_left) == 1) // or == (pow(2, bits_left) -1) ??
                    {
                        TRACE_3(BITS, "<b> These bits are trailing bits: no more useful data in current NAL Unit");
                        retcode = false;
                    }
                }
                else
                {
                    TRACE_3(BITS, "<b> %u bits left in current NAL Unit! No need to check for trailing data", bits_left);
                }
            }
        }
        else
        {
            retcode = false;
        }
    }
    else
    {
        // Count bits left in NAL Unit
        unsigned int bits_left = bitstr->buffer_size*8 - bitstr->buffer_offset;

        if (bits_left < 8)
        {
            if (bits_left == 0)
            {
                retcode = false;
            }
            else // Count trailing bits
            {
                TRACE_3(BITS, "<b> %u bits left in current NAL Unit! Checking for trailing data.", bits_left);

                if (next_bits(bitstr, bits_left) == 1) // or == (pow(2, bits_left) -1) ??
                {
                    TRACE_3(BITS, "<b> These bits are trailing bits: no more useful data in current NAL Unit");
                    retcode = false;
                }
            }
        }
        else
        {
            TRACE_3(BITS, "<b> %u bits left in current NAL Unit! No need to check for trailing data", bits_left);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Determine if there is additional data or trailing data in the RBSP.
 * \param *bitstr The bitstream to check.
 * \return true if more data have been found, false otherwise.
 *
 * This function check for the immediate presence of a start_code_prefix. If there
 * is no 3 or 4 bytes start_code_prefix, we can conclude that there is more data
 * left in the RBSP.
 * In this function trailing bits are considered as data.
 *
 * More informations available at 7.4.1 'NAL unit semantics' of H.264 standard.
 */
bool h264_more_rbsp_trailing_data(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "h264_more_rbsp_trailing_data()" CLR_RESET);
    bool retcode = true;

    if (bitstr->sample_map == NULL)
    {
        if (bitstream_check_alignment(bitstr))
        {
            if (next_bits(bitstr, 24) == 0x000001 && next_bits(bitstr, 32) == 0x00000001)
            {
                TRACE_3(BITS, "<b> No more data or trailing data in rbsp");
                retcode = false;
            }
        }
    }
    else
    {
        if (bitstr->buffer_offset >= bitstr->buffer_size*8)
        {
            TRACE_3(BITS, "<b> No more data or trailing data in rbsp");
            retcode = false;
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint16_t endian_flip_16(uint16_t src)
{
    TRACE_3(BITS, "    endian_flip_16()");

    return ( ((src & 0x00FF) << 8)
           | ((src & 0xFF00) >> 8) );
}

/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \param n The number size in bits.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint16_t endian_flip_cut_16(uint16_t src, const int n)
{
    TRACE_3(BITS, "    endian_flip_cut_16()");

    if (n > 0 && n < 16)
    {
        return ( ((src & 0x00FF) << 8)
               | ((src & 0xFF00) >> 8) ) >> (16 - n);
    }
    else
    {
        return src;
    }
}

/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint32_t endian_flip_32(uint32_t src)
{
    TRACE_3(BITS, "    endian_flip_32()");

    return ( ((src & 0x000000FF) << 24)
           | ((src & 0x0000FF00) <<  8)
           | ((src & 0x00FF0000) >>  8)
           | ((src & 0xFF000000) >> 24) );
}

/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \param n The size in bits of the wanted value.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint32_t endian_flip_cut_32(uint32_t src, const  int n)
{
    TRACE_3(BITS, "    endian_flip_cut_32()");

    if (n > 0 && n < 32)
    {
        return ( ((src & 0x000000FF) << 24)
               | ((src & 0x0000FF00) <<  8)
               | ((src & 0x00FF0000) >>  8)
               | ((src & 0xFF000000) >> 24) ) >> (32 - n);
    }
    else
    {
        return src;
    }
}

/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint64_t endian_flip_64(uint64_t src)
{
    TRACE_3(BITS, "    endian_flip_64()");

    return ( ((src & 0x00000000000000FFULL) << 56)
           | ((src & 0x000000000000FF00ULL) << 40)
           | ((src & 0x0000000000FF0000ULL) << 24)
           | ((src & 0x00000000FF000000ULL) <<  8)
           | ((src & 0x000000FF00000000ULL) >>  8)
           | ((src & 0x0000FF0000000000ULL) >> 24)
           | ((src & 0x00FF000000000000ULL) >> 40)
           | ((src & 0xFF00000000000000ULL) >> 56) );
}

/* ************************************************************************** */

/*!
 * \brief flip endianness variable content.
 * \param src The source variable to flip.
 * \param n The size in bits of the wanted value.
 * \return The destination variable, with flipped endianness.
 *
 * Only useful for variable bigger than 1 byte!
 */
uint64_t endian_flip_cut_64(uint64_t src, const int n)
{
    TRACE_3(BITS, "    endian_flip_cut_64()");

    if (n > 0 && n < 64)
    {
        return ( ((src & 0x00000000000000FFULL) << 56)
               | ((src & 0x000000000000FF00ULL) << 40)
               | ((src & 0x0000000000FF0000ULL) << 24)
               | ((src & 0x00000000FF000000ULL) <<  8)
               | ((src & 0x000000FF00000000ULL) >>  8)
               | ((src & 0x0000FF0000000000ULL) >> 24)
               | ((src & 0x00FF000000000000ULL) >> 40)
               | ((src & 0xFF00000000000000ULL) >> 56) ) >> (64 - n);
    }
    else
    {
        return src;
    }
}

/* ************************************************************************** */
