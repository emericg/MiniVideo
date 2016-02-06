/*!
 * COPYRIGHT (C) 2015 Emeric Grange - All Rights Reserved
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
 * \file      riff.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

// minivideo headers
#include "riff.h"
#include "riff_struct.h"
#include "../../fourcc.h"
#include "../../utils.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ************************************************************************** */

/*!
 * \brief Parse a RIFF list header.
 *
 * bitstr pointer is not checked for performance reason.
 */
int parse_list_header(Bitstream_t *bitstr, RiffList_t *list_header)
{
    TRACE_3(RIF, "parse_list_header()\n");
    int retcode = SUCCESS;

    if (list_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffList_t structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse RIFF list header
        list_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        list_header->dwList       = read_bits(bitstr, 32);
        list_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
        list_header->dwFourCC     = read_bits(bitstr, 32);
        list_header->offset_end   = list_header->offset_start + list_header->dwSize + 8;

        if (list_header->dwList != fcc_RIFF &&
            list_header->dwList != fcc_LIST)
        {
            TRACE_1(RIF, "We are looking for a RIFF list, however this is neither a LIST nor a RIFF (0x%04X)\n", list_header->dwList);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print an RIFF list header.
 */
void print_list_header(RiffList_t *list_header)
{
    if (list_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffList_t structure!\n");
    }
    else
    {
        TRACE_2(RIF, "* offset_s   : %u\n", list_header->offset_start);
        TRACE_2(RIF, "* offset_e   : %u\n", list_header->offset_end);

        if (list_header->dwList == fcc_RIFF)
        {
            TRACE_2(RIF, "* RIFF header\n");
        }
        else
        {
            TRACE_2(RIF, "* LIST header\n");
        }

        char fcc[5];
        TRACE_2(RIF, "* LIST size : %u\n", list_header->dwSize);
        TRACE_2(RIF, "* LIST fcc  : 0x%08X ('%s')\n",
                list_header->dwFourCC,
                getFccString_le(list_header->dwFourCC, fcc));
    }
}

/* ************************************************************************** */

/*!
 * \brief Skip a list header and content.
 */
int skip_list(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffList_t *list_header_child)
{
    int retcode = FAILURE;

    if (list_header_child->dwSize != 0)
    {
        int64_t jump = list_header_child->dwSize * 8;
        int64_t offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check that we do not jump outside the parent list boundaries
        if ((offset + jump) > list_header_parent->offset_end)
        {
            jump = list_header_parent->offset_end - offset;
        }

        if (skip_bits(bitstr, jump) == FAILURE)
        {
            TRACE_ERROR(RIF, "> skip_list() >> Unable to skip %i bytes!\n", list_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(RIF, "> skip_list() >> %i bytes\n", list_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(RIF, "> skip_list() >> do it yourself!\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse a RIFF chunk header.
 *
 * bitstr pointer is not checked for performance reason.
 */
int parse_chunk_header(Bitstream_t *bitstr, RiffChunk_t *chunk_header)
{
    TRACE_3(RIF, "parse_chunk_header()\n");
    int retcode = SUCCESS;

    if (chunk_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffChunk_t structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse RIFF chunk header
        chunk_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        chunk_header->dwFourCC     = read_bits(bitstr, 32);
        chunk_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
        chunk_header->offset_end   = chunk_header->offset_start + chunk_header->dwSize + 8;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print a RIFF chunk header.
 */
void print_chunk_header(RiffChunk_t *chunk_header)
{
    if (chunk_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffChunk_t structure!\n");
    }
    else
    {
        TRACE_2(RIF, "* offset_s  : %u\n", chunk_header->offset_start);
        TRACE_2(RIF, "* offset_e  : %u\n", chunk_header->offset_end);

        char fcc[5];
        TRACE_2(RIF, "* CHUNK size: %u\n", chunk_header->dwSize);
        TRACE_2(RIF, "* CHUNK fcc : 0x%08X ('%s')\n",
                chunk_header->dwFourCC,
                getFccString_le(chunk_header->dwFourCC, fcc));
    }
}

/* ************************************************************************** */

/*!
 * \brief Skip a RIFF chunk header and its content.
 */
int skip_chunk(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffChunk_t *chunk_header_child)
{
    int retcode = FAILURE;

    if (chunk_header_child->dwSize != 0)
    {
        int64_t jump = chunk_header_child->dwSize * 8;
        int64_t offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check that we do not jump outside the parent list boundaries
        if ((offset + jump) > list_header_parent->offset_end)
        {
            jump = list_header_parent->offset_end - offset;
        }

        if (skip_bits(bitstr, jump) == FAILURE)
        {
            TRACE_ERROR(RIF, "> skip_chunk() >> Unable to skip %i bytes!\n", chunk_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(RIF, "> skip_chunk() >> %i bytes\n", chunk_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(RIF, "> skip_chunk() >> do it yourself!\n");
        retcode = FAILURE;
    }

    print_chunk_header(chunk_header_child);

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - RIFF edition.
 * \param parent: The RiffList_t containing the current list / chunk we're in.
 * \param offset_end: The end offset of the current list / chunk we're in.
 *
 * 'Jumpy' is in charge of checking your position into the stream after your
 * parser finish parsing a box / list / chunk / element, never leaving you
 * stranded  in the middle of nowhere with no easy way to get back on track.
 * It will check available informations to known if the current element has been
 * fully parsed, and if not perform a jump (or even a rewind) to the next known
 * element.
 */
int jumpy_riff(Bitstream_t *bitstr, RiffList_t *parent, int64_t offset_end)
{
    int retcode = FAILURE;
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);

    if (current_pos != offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);

        // If the current list/chunk has a parent, and its offset_end is 'valid' (not past file size)
        if (parent && parent->offset_end < file_size)
        {
            // If the current offset_end is past its parent offset_end, its probably
            // broken, and so we will use the one from its parent
            if (offset_end > parent->offset_end)
            {
                offset_end = parent->offset_end;
            }
        }
        else // no parent (or parent with broken offset_end)
        {
            // If the current offset_end is past file size
            if (offset_end > file_size)
                offset_end = file_size;
        }

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up
        if (offset_end >= file_size)
        {
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        // Now, do we need to go forward or backward to reach our goal?
        // Then, can we move in our current buffer or do we need to reload a new one?
        if (current_pos < offset_end)
        {
            int64_t jump = offset_end - current_pos;

            if (jump < (UINT_MAX/8))
                retcode = skip_bits(bitstr, (unsigned int)(jump*8));
            else
                retcode = bitstream_goto_offset(bitstr, offset_end);
        }
        else
        {
            int64_t rewind = current_pos - offset_end;

            if (rewind > 0)
            {
                if (rewind > (UINT_MAX/8))
                    retcode = rewind_bits(bitstr, (unsigned int)(rewind*8));
                else
                    retcode = bitstream_goto_offset(bitstr, offset_end);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
