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
 * \file      bitstream.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "bitstream.h"
#include "bitstream_utils.h"
#include "minitraces.h"
#include "minivideo_typedef.h"
#include "import.h"

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
 * \brief Default size for the bitstream data buffer memory cache.
 */
#define DEFAULT_BUFFER_SIZE 1024

/* ************************************************************************** */

/*!
 * \brief init_bitstream0
 * \param media
 * \param bitstream_offset
 * \param buffer_size
 * \return
 */
Bitstream_t *init_bitstream0(MediaFile_t *media, int64_t bitstream_offset, uint32_t buffer_size)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "init_bitstream()" CLR_RESET);
    Bitstream_t *bitstr = NULL;

    if (media == NULL ||
        media->file_pointer == NULL)
    {
        TRACE_ERROR(BITS, "<b> Unable to use MediaFile_t structure!");
    }
    else
    {
        // Bitstream structure allocation
        bitstr = (Bitstream_t*)calloc(1, sizeof(Bitstream_t));

        if (bitstr == NULL)
        {
            TRACE_ERROR(BITS, "<b> Unable to allocate bitstream structure!");
        }
        else
        {
            // Bitstream structure initialization
            bitstr->bitstream_file = media->file_pointer;
            bitstr->bitstream_size = media->file_size;
            bitstr->bitstream_offset = 0;
            if (bitstream_offset > 0)
                bitstr->bitstream_offset = bitstream_offset;

            bitstr->buffer = NULL;
            bitstr->buffer_offset = 0;
            if (buffer_size > 0)
            {
                bitstr->buffer_size_saved = buffer_size;
                bitstr->buffer_size = buffer_size;
            }
            else
            {
                bitstr->buffer_size_saved = DEFAULT_BUFFER_SIZE;
                bitstr->buffer_size = DEFAULT_BUFFER_SIZE;
            }

            // Bitstream buffer allocation
            //FIXME use realloc(bitstr->buffer, bitstr->buffer_size + 4) to prevent some cases of invalid 32 bits reads near the end of the buffer
            //FIXME use realloc(bitstr->buffer, bitstr->buffer_size + 8) to prevent some cases of invalid 64 bits reads near the end of the buffer
            bitstr->buffer = (uint8_t*)calloc(bitstr->buffer_size_saved, sizeof(uint8_t));

            if (bitstr->buffer == NULL)
            {
                TRACE_ERROR(BITS, "<b> Unable to allocate the bitstream buffer!");
                free(bitstr);
                bitstr = NULL;
            }
            else
            {
                // Initial buffer filling
                buffer_feed_dynamic(bitstr, bitstr->bitstream_offset);
            }
        }
    }

    // Return the bitstream structure pointer
    return bitstr;
}

/* ************************************************************************** */

/*!
 * \brief Init a new bitstream.
 * \param *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \param *stream: A pointer to a MediaStream_t structure, containing informations about video payload datas.
 * \return *bitstr: A pointer to our newly allocated bitstream structure.
 *
 * If no MediaStream_t is available, it mean we have continuous video data,
 * starting at byte offset 0 and until the end of file.
 * Otherwise, an available MediaStream_t structure mean that we have encapsulated
 * video data, and we must bufferize data sample by sample.
 */
Bitstream_t *init_bitstream(MediaFile_t *media, MediaStream_t *stream)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "init_bitstream()" CLR_RESET);
    Bitstream_t *bitstr = NULL;

    if (media == NULL ||
        media->file_pointer == NULL)
    {
        TRACE_ERROR(BITS, "<b> Unable to use MediaFile_t structure!");
    }
    else
    {
        // Bitstream structure allocation
        bitstr = (Bitstream_t*)calloc(1, sizeof(Bitstream_t));

        if (bitstr == NULL)
        {
            TRACE_ERROR(BITS, "<b> Unable to calloc bitstream unit!");
        }
        else
        {
            // Bitstream structure initialization
            bitstr->bitstream_file = media->file_pointer;
            bitstr->bitstream_size = media->file_size;
            bitstr->bitstream_offset = 0;

            bitstr->buffer = NULL;
            bitstr->buffer_size = DEFAULT_BUFFER_SIZE;
            bitstr->buffer_size_saved = DEFAULT_BUFFER_SIZE;
            bitstr->buffer_offset = 0;

            // Use a bitstream_map if available
            bitstr->sample_map = stream;
            bitstr->sample_index = 0;

            // Bitstream buffer allocation
            //FIXME use realloc(bitstr->buffer, bitstr->buffer_size + 4) to prevent some cases of invalid 32 bits reads near the end of the buffer
            //FIXME use realloc(bitstr->buffer, bitstr->buffer_size + 8) to prevent some cases of invalid 64 bits reads near the end of the buffer
            if ((bitstr->buffer = (uint8_t*)calloc(bitstr->buffer_size, sizeof(uint8_t))) == NULL)
            {
                TRACE_ERROR(BITS, "<b> Unable to calloc the bitstream buffer!");
                free(bitstr);
                bitstr = NULL;
            }
            else
            {
                // Initial buffer filling, only if we do not use a bitstream_map
                if (stream == NULL)
                {
                    // Rewind file pointer
                    rewind(bitstr->bitstream_file);

                    // Feed bitstream buffer
                    if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
                    {
                        free(bitstr->buffer);
                        bitstr->buffer = NULL;
                        free(bitstr);
                        bitstr = NULL;
                    }
                }
            }
        }
    }

    if (bitstr != NULL)
    {
        TRACE_1(BITS, "<b> Bitstream initialization success");
    }
    else
    {
        TRACE_ERROR(BITS, "<b> Bitstream initialization FAILED!");
    }

    // Return the bitstream structure pointer
    return bitstr;
}

/* ************************************************************************** */

/*!
 * \brief Feed the bitstream buffer with fresh data.
 * \param *bitstr The bitstream to freed.
 * \return 1 if success, 0 otherwise.
 *
 * This function is only used by the H.264 video decoder.
 */
int buffer_feed_manual(Bitstream_t *bitstr, int64_t bitstream_offset, int64_t size)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "buffer_feed_manual()" CLR_RESET);
    int retcode = SUCCESS;

    // Print stats?
    //TRACE_1(BITS, "<b> Status before reallocation:");
    //bitstream_print_stats(bitstr);

    // Reset parameters
    bitstr->buffer_offset = 0;
    bitstr->buffer_discarded_bytes = 0;
    // Deallocate buffer (if needed)
    if (bitstr->buffer != NULL &&
        bitstr->buffer_size != size)
    {
        free(bitstr->buffer);
        bitstr->buffer = NULL;
    }
    // Allocate new buffer (if needed)
    if (bitstr->buffer == NULL)
    {
        bitstr->buffer_size = size;
        bitstr->buffer = (uint8_t *)malloc(bitstr->buffer_size);
    }

    if (bitstr->buffer == NULL)
    {
        TRACE_ERROR(BITS, "<b> Unable to realloc bitstream buffer!");
        retcode = FAILURE;
    }
    else
    {
        // Move file pointer
        bitstr->bitstream_offset = bitstream_offset;
        if (fseek(bitstr->bitstream_file, bitstr->bitstream_offset, SEEK_SET) != 0)
        {
            TRACE_ERROR(BITS, "<b> Unable to seek through the input file!");
            retcode = FAILURE;
        }
        else
        {
            // Feed buffer
            if (fread(bitstr->buffer, sizeof(uint8_t), bitstr->buffer_size, bitstr->bitstream_file) != bitstr->buffer_size)
            {
                TRACE_ERROR(BITS, "<b> Unable to read from the input file!");
                retcode = FAILURE;
            }
            else
            {
                TRACE_1(BITS, "<b> Bitstream buffer reallocation succeed!");
                retcode = SUCCESS;
            }
        }

        // Print stats?
        //TRACE_1(BITS, "<b> Status after reallocation:");
        //bitstream_print_stats(bitstr);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Feed the bitstream buffer with fresh data.
 * \param *bitstr The bitstream to freed.
 * \param new_bitstream_offset The byte offset of the data we want to bufferize.
 * \return 1 if success, 0 otherwise.
 *
 * If the bitstream_offset is a negative value, just load the data following the
 * data from the current buffer.
 * Otherwise, load data starting at the given new_bitstream_offset.
 */
int buffer_feed_dynamic(Bitstream_t *bitstr, int64_t new_bitstream_offset)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "buffer_feed_dynamic()" CLR_RESET);
    int retcode = FAILURE;

    // Print stats?
    //TRACE_1(BITS, "<b> Status before reallocation:");
    //bitstream_print_stats(bitstr);

    // Update current offset into the bitstream
    if (new_bitstream_offset > -1)
    {
        bitstr->bitstream_offset = new_bitstream_offset;
        bitstr->buffer_offset = 0;
    }
    else
    {
        bitstr->bitstream_offset += (int64_t)(bitstr->buffer_offset/8) + bitstr->buffer_discarded_bytes;
        bitstr->buffer_offset %= 8;
    }

    // Check for premature end of file
    if (bitstr->bitstream_offset >= bitstr->bitstream_size)
    {
        TRACE_ERROR(BITS, "<b> Fatal error: premature end of file reached!");
        retcode = FAILURE;
    }
    else
    {
        // Reset buffer size (necessary if some data have been dynamically removed from previous buffer)
        bitstr->buffer_size = bitstr->buffer_size_saved;
        bitstr->buffer_discarded_bytes = 0;

        // Cut buffer size if the end of file is almost reached
        if ((bitstr->bitstream_offset + bitstr->buffer_size) > bitstr->bitstream_size)
        {
            unsigned int buffer_size_saved = bitstr->buffer_size;
            bitstr->buffer_size = (unsigned int)(bitstr->bitstream_size - bitstr->bitstream_offset);
            bitstr->buffer = (uint8_t*)realloc(bitstr->buffer, bitstr->buffer_size);

            if (bitstr->buffer == NULL)
            {
                TRACE_ERROR(BITS, "<b> Unable to realloc bitstream buffer!");
                retcode = FAILURE;
            }
            else
            {
                TRACE_1(BITS, "<b> Bitstream buffer resized from %uB to %uB", buffer_size_saved, bitstr->buffer_size);
            }
        }

        // Move file pointer
        if (fseek(bitstr->bitstream_file, bitstr->bitstream_offset, SEEK_SET) != 0)
        {
            TRACE_ERROR(BITS, "<b> Unable to seek through the input file!");
            retcode = FAILURE;
        }
        else
        {
            // Feed buffer
            size_t b = fread(bitstr->buffer, sizeof(uint8_t), bitstr->buffer_size, bitstr->bitstream_file);

            if (b != bitstr->buffer_size)
            {
                TRACE_ERROR(BITS, "<b> Unable to read from the input file! (%u read instead of %u)", b, bitstr->buffer_size);
                retcode = FAILURE;
            }
            else
            {
                TRACE_1(BITS, "<b> Bitstream buffer feeded!");
                retcode = SUCCESS;
            }

            // Print stats?
            //TRACE_1(BITS, "<b> Status after reallocation:");
            //bitstream_print_stats(bitstr);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Destroy a bitstream and it's buffer.
 * \param **bitstr_ptr The bitstream to freed.
 *
 * This function do not freed MediaFile_t structure.
 */
void free_bitstream(Bitstream_t **bitstr_ptr)
{
    TRACE_INFO(BITS, "<b> " BLD_BLUE "free_bitstream()" CLR_RESET);

    if (*bitstr_ptr != NULL)
    {
        if ((*bitstr_ptr)->buffer != NULL)
        {
            free((*bitstr_ptr)->buffer);
            (*bitstr_ptr)->buffer = NULL;

            TRACE_1(BITS, "<b> Bitstream buffer freed");
        }

        {
            free(*bitstr_ptr);
            *bitstr_ptr = NULL;

            TRACE_1(BITS, "<b> Bitstream freed");
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Read 1 bit from a bitstream, return it then move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \return bit The bit read from the bitstream.
 *
 * First step is to check if reading 1 bit is possible, then if there is 1 bit
 * left in the bitstream buffer.
 */
uint32_t read_bit(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "read_bit()" CLR_RESET " starting at bit offset %lli", bitstream_get_absolute_bit_offset(bitstr));

    uint32_t start_byte = (uint32_t)(bitstr->buffer_offset / 8);

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if (start_byte >= bitstr->buffer_size)
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }

        start_byte = (uint32_t)(bitstr->buffer_offset / 8);
    }

    // Read one bit
    ////////////////////////////////////////////////////////////////////////

    uint32_t bp = 7 - (uint32_t)(bitstr->buffer_offset % 8); // back padding, in bit

    uint32_t bit = (uint32_t)(bitstr->buffer[start_byte] >> bp);
    bit &= 0x01;

    // Update bit offset
    bitstr->buffer_offset++;

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   bit     : %i", bit);

    return bit;
}

/* ************************************************************************** */

/*!
 * \brief Read n bit(s) from a bitstream, return it then move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \param n The number of bits to read, up to 32.
 * \return bits The content read from the bitstream.
 *
 * First step is to check if reading n bits is possible, then if there is n bits
 * left in the bitstream buffer.
 */
uint32_t read_bits(Bitstream_t *bitstr, const unsigned int n)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "read_bits(%u)" CLR_RESET " starting at bit offset %lli", n, bitstream_get_absolute_bit_offset(bitstr));

    uint32_t bits = 0;
    uint32_t fp = (uint32_t)(bitstr->buffer_offset % 8); // front padding, in bit
    uint32_t byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
    uint32_t tbr = (uint32_t)ceil((n + fp) / 8.0);
    uint32_t tbr_current = tbr;

    // Debug traces
    ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
    TRACE_2(BITS, "<b>   n       : %u", n);
    TRACE_2(BITS, "<b>   fp      : %u", fp);
    TRACE_2(BITS, "<b>   bo      : %u", bitstr->buffer_offset);
    TRACE_2(BITS, "<b>   start   : %u", byte_current);
    TRACE_2(BITS, "<b>   tbr     : %u", tbr);

    // Check if we can read n bits
    ////////////////////////////////////////////////////////////////////////

    if (n == 0)
    {
        TRACE_WARNING(BITS, "This function cannot read 0 bits!");
        return FAILURE;
    }
    else if (n > 32)
    {
        TRACE_WARNING(BITS, "You want to read %i bits, but this function can only read up to 32 bits!", n);
        return FAILURE;
    } else
#endif // ENABLE_DEBUG

    if (n == 1)
    {
        return read_bit(bitstr);
    }

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if ((byte_current + tbr) > bitstr->buffer_size)
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }

        fp = bitstr->buffer_offset % 8; // front padding, in bit
        byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
        tbr = (uint32_t)ceil((n + fp) / 8.0);
        tbr_current = tbr;
    }

    // Read
    ////////////////////////////////////////////////////////////////////////

    if (fp > 0)
    {
        // Read un-aligned bits
        bits += bitstr->buffer[byte_current++];
        bits &= 0xFF >> fp;
        tbr_current--;

        while (tbr_current > 0)
        {
            if (tbr > 4 &&
                tbr_current == 1)
            {
                bits <<= fp;
                bits += bitstr->buffer[byte_current++] & (0xFF >> fp);
            }
            else
            {
                bits <<= 8;
                bits += bitstr->buffer[byte_current++];
            }

            tbr_current--;
        }

        bits >>= ((tbr*8) - n - fp);
    }
    else
    {
        // Read aligned bits
        while (tbr_current > 0)
        {
            bits <<= 8;
            bits += bitstr->buffer[byte_current++];
            tbr_current--;
        }

        bits >>= (32 - n) % 8;
    }

    // Update bit offset
    bitstr->buffer_offset += n;

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   content = 0d%u", bits);
    TRACE_2(BITS, "<b>   content = 0x%08X", bits);

    return bits;
}

/* ************************************************************************** */

/*!
 * \brief Read n bit(s) from a bitstream, return it then move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \param n The number of bits to read, up to 64.
 * \return bits The content read from the bitstream.
 *
 * First step is to check if reading n bits is possible, then if there is n bits
 * left in the bitstream buffer.
 */
uint64_t read_bits_64(Bitstream_t *bitstr, const unsigned int n)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "read_bits_64(%u)" CLR_RESET " starting at bit offset %lli", n, bitstream_get_absolute_bit_offset(bitstr));

    uint64_t bits = 0;
    uint32_t fp = (uint32_t)(bitstr->buffer_offset % 8); // front padding, in bit
    uint32_t byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
    uint32_t tbr = (uint32_t)ceil((n + fp) / 8.0);
    uint32_t tbr_current = tbr;

    // Debug traces
    ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
    TRACE_2(BITS, "<b>   n       : %u", n);
    TRACE_2(BITS, "<b>   fp      : %u", fp);
    TRACE_2(BITS, "<b>   bo      : %u", bitstr->buffer_offset);
    TRACE_2(BITS, "<b>   start   : %u", byte_current);
    TRACE_2(BITS, "<b>   tbr     : %u", tbr);

    // Check if we can read n bits
    ////////////////////////////////////////////////////////////////////////

    if (n == 0)
    {
        TRACE_WARNING(BITS, "This function cannot read 0 bits!");
        return FAILURE;
    }
    else if (n > 64)
    {
        TRACE_WARNING(BITS, "You want to read %i bits, but this function can only read up to 64 bits!", n);
        return FAILURE;
    } else
#endif // ENABLE_DEBUG

    if (n == 1)
    {
        return read_bit(bitstr);
    }

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if ((byte_current + tbr) > bitstr->buffer_size)
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }

        fp = (uint32_t)(bitstr->buffer_offset % 8); // front padding, in bit
        byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
        tbr = (uint32_t)ceil((n + fp) / 8.0);
        tbr_current = tbr;
    }

    // Read
    ////////////////////////////////////////////////////////////////////////

    if (fp > 0)
    {
        // Read un-aligned bits
        bits += bitstr->buffer[byte_current++];
        bits &= 0xFF >> fp;
        tbr_current--;

        while (tbr_current > 0)
        {
            if (tbr > 4 &&
                tbr_current == 1)
            {
                bits <<= fp;
                bits += bitstr->buffer[byte_current++] & (0xFF >> fp);
            }
            else
            {
                bits <<= 8;
                bits += bitstr->buffer[byte_current++];
            }

            tbr_current--;
        }

        bits >>= ((tbr*8) - n - fp);
    }
    else
    {
        // Read aligned bits
        while (tbr_current > 0)
        {
            bits <<= 8;
            bits += bitstr->buffer[byte_current++];
            tbr_current--;
        }

        bits >>= (64 - n) % 8;
    }

    // Update bit offset
    bitstr->buffer_offset += n;

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   content = 0d%LLu", bits);
    TRACE_2(BITS, "<b>   content = 0x%16X", bits);

    return bits;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Read 1 byte from a bitstream, return it, then move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \return bit The bit read from the bitstream.
 *
 * First step is to check if reading one byte is possible, if there is 8 bits
 * left in the bitstream buffer, then move the bitstream offset.
 */
uint32_t read_byte_aligned(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "read_byte_aligned()" CLR_RESET " starting at bit offset %lli", bitstream_get_absolute_bit_offset(bitstr));
    uint32_t byte = 0;

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if ((bitstr->buffer_offset + 8) > (bitstr->buffer_size * 8))
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }
    }

    // Check byte alignment
    ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
    if ((bitstr->buffer_offset % 8) != 0)
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "read_byte_aligned() on unaligned offset" CLR_RESET " at current byte offset %lli + %i bit(s)", bitstream_get_absolute_byte_offset(bitstr), bitstream_get_absolute_bit_offset(bitstr)%8);
        return FAILURE;
    }
#endif // ENABLE_DEBUG

    // Read one byte
    ////////////////////////////////////////////////////////////////////////

    byte = (uint32_t)(bitstr->buffer[bitstr->buffer_offset / 8]);

    // Update bit offset
    bitstr->buffer_offset += 8;

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   byte    : %02X", byte);

    return byte;
}

/* ************************************************************************** */

/*!
 * \brief Read one byte from a bitstream and return it, but DO NOT move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \return bit The bit read from the bitstream.
 *
 * This function is basically the same as read_byte_aligned() but it DO NOT move
 * the buffer position after reading one byte.
 */
uint32_t next_byte_aligned(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "next_byte_aligned()" CLR_RESET " starting at bit offset %lli", bitstream_get_absolute_bit_offset(bitstr));
    uint32_t byte = 0;

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if ((bitstr->buffer_offset + 8) > (bitstr->buffer_size * 8))
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }
    }

    // Check byte alignment
    ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
    if ((bitstr->buffer_offset % 8) != 0)
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "read_byte_aligned() on unaligned offset" CLR_RESET " at current byte offset %lli + %i bit(s)", bitstream_get_absolute_byte_offset(bitstr), bitstream_get_absolute_bit_offset(bitstr)%8);
        return FAILURE;
    }
#endif // ENABLE_DEBUG

    // Read one byte
    ////////////////////////////////////////////////////////////////////////

    byte = (uint32_t)(bitstr->buffer[bitstr->buffer_offset / 8]);

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   byte    : %02X", byte);

    return byte;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Read one bit from a bitstream and return it, but DO NOT move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \return bit The bit read from the bitstream.
 *
 * This function is basically the same as read_bit() but it DO NOT move the
 * buffer position after reading one bit.
 */
uint32_t next_bit(Bitstream_t *bitstr)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "next_bit()" CLR_RESET " starting at bit offset %lli", bitstream_get_absolute_bit_offset(bitstr));

    uint32_t start_byte = (uint32_t)(bitstr->buffer_offset / 8);

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if (start_byte >= bitstr->buffer_size)
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }

        start_byte = (uint32_t)(bitstr->buffer_offset / 8);
    }

    // Read one bit
    ////////////////////////////////////////////////////////////////////////

    uint32_t bp = 7 - (uint32_t)(bitstr->buffer_offset % 8); // back padding, in bit

    uint32_t bit = (uint32_t)(bitstr->buffer[start_byte] >> bp);
    bit &= 0x01;

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   bit     = %i", bit);

    return bit;
}

/* ************************************************************************** */

/*!
 * \brief Read n bit(s) from a bitstream and return it, but DO NOT move the bitstream position.
 * \param *bitstr The bitstream to read.
 * \param n The number of bit(s) to read. Up to 32 bits.
 * \return bits The content read from the bitstream.
 *
 * This function is basically the same as read_bits() but it DO NOT move the
 * buffer position after reading n bits.
 */
uint32_t next_bits(Bitstream_t *bitstr, const unsigned int n)
{
    TRACE_3(BITS, "<b> " BLD_BLUE "next_bits(%u)" CLR_RESET " starting at bit offset %lli", n, bitstream_get_absolute_bit_offset(bitstr));

    uint32_t bits = 0;
    uint32_t fp = (uint32_t)(bitstr->buffer_offset % 8); // front padding, in bit
    uint32_t byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
    uint32_t tbr = (uint32_t)ceil((n + fp) / 8.0);
    uint32_t tbr_current = tbr;

    // Debug traces
    ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
    TRACE_2(BITS, "<b>   n       : %u", n);
    TRACE_2(BITS, "<b>   fp      : %u", fp);
    TRACE_2(BITS, "<b>   bo      : %u", bitstr->buffer_offset);
    TRACE_2(BITS, "<b>   start   : %u", byte_current);
    TRACE_2(BITS, "<b>   tbr     : %u", tbr);

    // Check if we can read n bits
    ////////////////////////////////////////////////////////////////////////

    if (n == 0)
    {
        TRACE_WARNING(BITS, "This function cannot read 0 bits!");
        return FAILURE;
    }
    else if (n > 32)
    {
        TRACE_WARNING(BITS, "You want to read %i bits, but this function can only read up to 32 bits!", n);
        return FAILURE;
    }
    else
#endif // ENABLE_DEBUG

    if (n == 1)
    {
        return next_bit(bitstr);
    }

    // Fill new data into the bitstream buffer, if needed
    ////////////////////////////////////////////////////////////////////////

    if ((byte_current + tbr) > bitstr->buffer_size)
    {
        if (buffer_feed_dynamic(bitstr, -1) == FAILURE)
        {
            return FAILURE;
        }

        fp = (uint32_t)(bitstr->buffer_offset % 8); // front padding, in bit
        byte_current = (uint32_t)floor(bitstr->buffer_offset / 8.0);
        tbr = (uint32_t)ceil((n + fp) / 8.0);
        tbr_current = tbr;
    }

    // Read
    ////////////////////////////////////////////////////////////////////////

    if (fp > 0)
    {
        // Read un-aligned bits
        bits += bitstr->buffer[byte_current++];
        bits &= 0xFF >> fp;
        tbr_current--;

        while (tbr_current > 0)
        {
            if (tbr > 4 &&
                tbr_current == 1)
            {
                bits <<= fp;
                bits += bitstr->buffer[byte_current++] & (0xFF >> fp);
            }
            else
            {
                bits <<= 8;
                bits += bitstr->buffer[byte_current++];
            }

            tbr_current--;
        }

        bits >>= ((tbr*8) - n - fp);
    }
    else
    {
        // Read aligned bits
        while (tbr_current > 0)
        {
            bits <<= 8;
            bits += bitstr->buffer[byte_current];
            byte_current++;
            tbr_current--;
        }

        bits >>= (32 - n) % 8;
    }

    // Return result
    ////////////////////////////////////////////////////////////////////////

    TRACE_2(BITS, "<b>   content = 0d%u", bits);
    TRACE_2(BITS, "<b>   content = 0x%08X", bits);

    return bits;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Skip n bits in a bitstream.
 * \param *bitstr The bitstream to use.
 * \param n The number of bits to skip.
 * \return 1 if success, 0 otherwise.
 *
 * skip_bits() is designed and optimized to handle small bit jumps INSIDE the bitstream buffer.
 *
 * However:
 * - If jumping 'n' bits takes us outside of the remaining bitstream buffer space,
 * bitstream_goto_offset() will be used (triggering a buffer refresh).
 * - If your jump is bigger than the bitsteam buffer size, save some time and use
 * bitstream_goto_offset() directy.
 * - If your jump is bigger than 2^32 bits, don't trigger an integer overflow
 * and use bitstream_goto_offset() directy.
 */
int skip_bits(Bitstream_t *bitstr, const unsigned int n)
{
    int retcode = FAILURE;

    // Check if destination is outside the current buffer
    if ((bitstr->buffer_offset + n) > (bitstr->buffer_size * 8))
    {
        // If it is, check if its in the next one
        if (n < (bitstr->buffer_size * 8))
        {
            // Refresh buffer
            retcode = buffer_feed_dynamic(bitstr, -1);

            // Skip n bits
            bitstr->buffer_offset += n;
        }
        else // Or somewhere else entierly
        {
            int64_t new_bit_offset = (int64_t)(bitstr->bitstream_offset*8 + bitstr->buffer_offset + n);

            // Do not jump to the last byte of the file?
            //if (new_bit_offset/8 >= bitstr->bitstream_size)
            {
                // Reload bitstream buffer from the byte offset we want
                retcode = bitstream_goto_offset(bitstr, new_bit_offset/8);

                // Then skip x bits ?
                if ((new_bit_offset % 8) != 0)
                {
                    bitstr->buffer_offset += new_bit_offset % 8;
                }
            }
        }
    }
    else
    {
        // Skip n bits
        bitstr->buffer_offset += n;
        retcode = SUCCESS;
    }

#if ENABLE_DEBUG
    if (retcode == SUCCESS)
    {
        TRACE_2(BITS, "<b> " BLD_BLUE "skip_bits(%u)" CLR_RESET " SUCCESS", n);
    }
    else
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "skip_bits(%u)" CLR_RESET " Cannot skip bits at bit offset", n, bitstr->buffer_offset);
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Rewind n bits in a bitstream.
 * \param *bitstr The bitstream to use.
 * \param n The number of bits to rewind.
 * \return 1 if success, 0 otherwise.
 *
 * If rewinding is impossible due to the current buffer_offset being smaller
 * than 'n', we do a jump directly to the offset we want. That will trigger a
 * buffer refresh.
 */
int rewind_bits(Bitstream_t *bitstr, const unsigned int n)
{
    int retcode = FAILURE;

    // Check if rewinding inside the buffer is possible
    if (n < bitstr->buffer_offset)
    {
        // Rewind n bits
        bitstr->buffer_offset -= n;
        retcode = SUCCESS;
    }
    else
    {
        // Must reload previous data and go directly to the offset we want
        int64_t new_offset = (int64_t)(bitstr->bitstream_offset*8 + bitstr->buffer_offset - n);
        retcode = bitstream_goto_offset(bitstr, new_offset);
    }

#if ENABLE_DEBUG
    if (retcode == SUCCESS)
    {
        TRACE_2(BITS, "<b> " BLD_BLUE "rewind_bits(%u)" CLR_RESET " SUCCESS", n);
    }
    else
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "rewind_bits(%u)" CLR_RESET " Cannot rewind bits at bit offset %u", n, bitstr->buffer_offset);
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Go to the n byte of a file, if possible.
 * \param *bitstr The bitstream to use.
 * \param n The position to go to, in byte.
 * \return 1 if success, 0 otherwise.
 *
 * Note that it is NOT possible to jump to the last byte of the bitstream,
 * because it would cause a buffer reload, and incidentally a "premature end
 * of file" error.
 */
int bitstream_goto_offset(Bitstream_t *bitstr, const int64_t n)
{
    int retcode = FAILURE;

    if (n > 0 && n < bitstr->bitstream_size)
    {
        retcode = buffer_feed_dynamic(bitstr, n);
    }
    else
    {
        retcode = FAILURE;
    }

#if ENABLE_DEBUG
    if (retcode == SUCCESS)
    {
        TRACE_2(BITS, "<b> " BLD_BLUE "bitstream_goto_offset(%lli)" CLR_RESET " Success", n);
    }
    else if (bitstream_get_absolute_byte_offset(bitstr) != n)
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "bitstream_goto_offset() at %lli instead of %lli" CLR_RESET, bitstream_get_absolute_byte_offset(bitstr), n);
    }
    else
    {
        TRACE_ERROR(BITS, "<b> " BLD_BLUE "bitstream_goto_offset(%lli)" CLR_RESET " Cannot jump outside bitstream boundaries!", n);
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int64_t bitstream_get_full_size(Bitstream_t *bitstr)
{
    return bitstr->bitstream_size;
}

/* ************************************************************************** */

/*!
 * \brief Return the current byte offset in the bitstream (may left a few bits behind...).
 * \param *bitstr The bitstream to use.
 * \return The absolute byte offset into the bitstream.
 */
int64_t bitstream_get_absolute_byte_offset(Bitstream_t *bitstr)
{
    return (int64_t)(bitstr->bitstream_offset + bitstr->buffer_offset/8 + bitstr->buffer_discarded_bytes);
}

/* ************************************************************************** */

/*!
 * \brief Return the current absolute bit offset in the bitstream.
 * \param *bitstr The bitstream to use.
 * \return The absolute bit offset into the bitstream.
 *
 * Be careful of integer overflow if file is more than 134217728 GiB?
 */
int64_t bitstream_get_absolute_bit_offset(Bitstream_t *bitstr)
{
    return (int64_t)(bitstr->bitstream_offset*8 + bitstr->buffer_offset + bitstr->buffer_discarded_bytes*8);
}

/* ************************************************************************** */
