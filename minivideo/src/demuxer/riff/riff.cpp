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
 * \file      riff.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

// minivideo headers
#include "riff.h"
#include "../../minivideo_typedef.h"
#include "../../minivideo_fourcc.h"
#include "../../utils.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdint>
#include <cinttypes>

/* ************************************************************************** */

/*!
 * \brief Parse a RIFF list header.
 * \note 'bitstr' pointer is not checked for performance reasons.
 */
int parse_list_header(Bitstream_t *bitstr, RiffList_t *list_header)
{
    TRACE_3(RIF, "parse_list_header()");
    int retcode = SUCCESS;

    if (list_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffList_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Parse RIFF list header
        list_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        list_header->dwList       = read_bits(bitstr, 32);

        if (list_header->dwList == fcc_RIFF ||
            list_header->dwList == fcc_LIST)
        {
            list_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
            list_header->dwFourCC     = read_bits(bitstr, 32);
        }
        else if (list_header->dwList == fcc_FFIR ||
                 list_header->dwList == fcc_TSIL)
        {
            // HACK // Bad endianness
            TRACE_WARNING(RIF, "Invalid RiffList_t endianness... Trying to fix...");
            list_header->dwList       = endian_flip_32(list_header->dwList);
            list_header->dwSize       = read_bits(bitstr, 32);
            list_header->dwFourCC     = endian_flip_32(read_bits(bitstr, 32));
        }
        else
        {
            TRACE_WARNING(RIF, "Invalid RIFF list header (0x%04X)", list_header->dwList);
            retcode = FAILURE;
        }

        // HACK // Make sure our list doesn't go outside the file
        list_header->offset_end = list_header->offset_start + list_header->dwSize + 8;
        if (list_header->offset_end > bitstr->bitstream_size)
        {
            list_header->offset_end = bitstr->bitstream_size;
            list_header->dwSize = list_header->offset_end - list_header->offset_start;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print a RIFF list header.
 */
void print_list_header(RiffList_t *list_header)
{
#if ENABLE_DEBUG
    if (list_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffList_t structure!");
    }
    else
    {
        TRACE_2(RIF, "* offset_s   : %lli", list_header->offset_start);
        TRACE_2(RIF, "* offset_e   : %lli", list_header->offset_end);

        if (list_header->dwList == fcc_RIFF)
        {
            TRACE_2(RIF, "* RIFF header");
        }
        else
        {
            TRACE_2(RIF, "* LIST header");
        }

        char fcc[5];
        TRACE_2(RIF, "* LIST size : %u", list_header->dwSize);
        TRACE_2(RIF, "* LIST fcc  : 0x%08X ('%s')",
                list_header->dwFourCC,
                getFccString_le(list_header->dwFourCC, fcc));
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Write a RIFF list header to a file for the xmlMapper.
 */
void write_list_header(RiffList_t *list_header, FILE *xml,
                       const char *title, const char *additional)
{
    if (xml)
    {
        if (list_header == NULL)
        {
            TRACE_ERROR(RIF, "Invalid RiffList_t structure!");
        }
        else
        {
            char boxtitle[64];
            char boxtype[20];
            char boxtypeadd[16];
            char fcc[5];

            if (title != nullptr)
                snprintf(boxtitle, 63, "tt=\"%s\" ", title);
            else
                boxtitle[0] = '\0';

            if (list_header->dwList == fcc_RIFF)
                strcpy(boxtype, "tp=\"RIFF header\" ");
            else
                strcpy(boxtype, "tp=\"RIFF list\" ");

            if (additional != nullptr)
                snprintf(boxtypeadd, 15, "add=\"%s\" ", additional);
            else
                boxtypeadd[0] = '\0';

            fprintf(xml, "  <a %s%s%sfcc=\"%s\" off=\"%" PRId64 "\" sz=\"%u\">\n",
                    boxtitle, boxtype, boxtypeadd,
                    getFccString_le(list_header->dwFourCC, fcc),
                    list_header->offset_start,
                    list_header->dwSize + 8);
        }
    }
}

/* ************************************************************************** */

/*!
 * \brief Skip a RIFF list header and content.
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
            TRACE_ERROR(RIF, "> skip_list() >> Unable to skip %i bytes!", list_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(RIF, "> skip_list() >> %i bytes", list_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(RIF, "> skip_list() >> do it yourself!");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse a RIFF chunk header.
 * \note 'bitstr' pointer is not checked for performance reasons.
 */
int parse_chunk_header(Bitstream_t *bitstr, RiffChunk_t *chunk_header)
{
    TRACE_3(RIF, "parse_chunk_header()");
    int retcode = SUCCESS;

    if (chunk_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffChunk_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Parse RIFF chunk header
        chunk_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        chunk_header->dwFourCC     = read_bits(bitstr, 32);
        chunk_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
        chunk_header->offset_end   = chunk_header->offset_start + chunk_header->dwSize + 8;

        // HACK // Make sure our chunk doesn't go outside the file
        if (chunk_header->offset_end > bitstr->bitstream_size)
        {
            chunk_header->offset_end = bitstr->bitstream_size;
            chunk_header->dwSize = chunk_header->offset_end - chunk_header->offset_start;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print a RIFF chunk header.
 */
void print_chunk_header(RiffChunk_t *chunk_header)
{
#if ENABLE_DEBUG
    if (chunk_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid RiffChunk_t structure!");
    }
    else
    {
        TRACE_2(RIF, "* offset_s  : %u", chunk_header->offset_start);
        TRACE_2(RIF, "* offset_e  : %u", chunk_header->offset_end);

        char fcc[5];
        TRACE_2(RIF, "* CHUNK size: %u", chunk_header->dwSize);
        TRACE_2(RIF, "* CHUNK fcc : 0x%08X ('%s')",
                chunk_header->dwFourCC,
                getFccString_le(chunk_header->dwFourCC, fcc));
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Write a RIFF chunk header to a file for the xmlMapper.
 */
void write_chunk_header(RiffChunk_t *chunk_header, FILE *xml,
                        const char *title, const char *additional)
{
    if (xml)
    {
        if (chunk_header == NULL)
        {
            TRACE_ERROR(RIF, "Invalid RiffChunk_t structure!");
        }
        else
        {
            char fcc[5];
            char boxtitle[64];
            char boxadd[16];

            if (title != nullptr)
                snprintf(boxtitle, 63, "tt=\"%s\" ", title);
            else
                boxtitle[0] = '\0';

            if (additional != nullptr)
                snprintf(boxadd, 15, "add=\"%s\" ", additional);
            else
                boxadd[0] = '\0';

            fprintf(xml, "  <a %s%stp=\"RIFF chunk\" fcc=\"%s\" off=\"%" PRId64 "\" sz=\"%u\">\n",
                    boxtitle,
                    boxadd,
                    getFccString_le(chunk_header->dwFourCC, fcc),
                    chunk_header->offset_start,
                    chunk_header->dwSize + 8);
        }
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
            TRACE_ERROR(RIF, "> skip_chunk() >> Unable to skip %i bytes!", chunk_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(RIF, "> skip_chunk() >> %i bytes", chunk_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(RIF, "> skip_chunk() >> do it yourself!");
        retcode = FAILURE;
    }

    print_chunk_header(chunk_header_child);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse unknown list.
 */
int parse_unkn_list(Bitstream_t *bitstr, RiffList_t *unkn_header, FILE *xml)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_odml()" CLR_RESET);
    int retcode = SUCCESS;

    if (unkn_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid unkn_header structure!");
        retcode = FAILURE;
    }
    else
    {
        char fcc[5];
        TRACE_WARNING(RIF, BLD_GREEN "parse_unkn_list(list type: %s / list fcc: %s)" CLR_RESET,
                      getFccString_le(unkn_header->dwList, fcc),
                      getFccString_le(unkn_header->dwFourCC, fcc));

        // Print list header
        print_list_header(unkn_header);
        write_list_header(unkn_header, xml);

        // Bytes left in the odml list
        int byte_left = unkn_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Loop on "odml" content
        while (retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < unkn_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode |= parse_list_header(bitstr, &list_header);

                retcode |= parse_unkn_list(bitstr, &list_header, xml);

                retcode |= jumpy_riff(bitstr, unkn_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode |= parse_chunk_header(bitstr, &chunk_header);

                retcode |= parse_unkn_chunk(bitstr, &chunk_header, xml);

                retcode |= jumpy_riff(bitstr, unkn_header, chunk_header.offset_end);
            }

            // Byte left in the odml list?
            byte_left = unkn_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);
        }

        if (xml) fprintf(xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse unknown chunk.
 */
int parse_unkn_chunk(Bitstream_t *bitstr, RiffChunk_t *unkn_header, FILE *xml)
{
    int retcode = SUCCESS;

    if (unkn_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid unkn_header structure!");
        retcode = FAILURE;
    }
    else
    {
        char fcc[5];
        TRACE_WARNING(RIF, BLD_GREEN "parse_unkn_chunk(chunk fcc: %s)" CLR_RESET,
                      getFccString_le(unkn_header->dwFourCC, fcc));

#if ENABLE_DEBUG
        print_chunk_header(unkn_header);
#endif
        // xmlMapper
        if (xml)
        {
            write_chunk_header(unkn_header, xml);
            fprintf(xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - RIFF edition.
 * \param bitstr: Our bitstream reader.
 * \param parent: The RiffList_t containing the current list / chunk we're in.
 * \param offset_end: The end offset of the current list / chunk we're in.
 * \return SUCCESS or FAILURE if the jump could not be done.
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
    int retcode = SUCCESS;

    // Done as a precaution, because the parsing of some boxes (like ESDS...)
    // can leave us in the middle of a byte and that will never be caught by
    // offset checks (cause they works on the assumption that we are byte aligned)
    bitstream_force_alignment(bitstr);

    // Check if we need a jump
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);
    if (current_pos != offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);

        // Check offset_end
        if (parent && parent->offset_end < file_size) // current list/chunk has valid parent
        {
            // Validate offset_end against parent's (parent win)
            if (offset_end > parent->offset_end)
                offset_end = parent->offset_end;
        }
        else // current list/chunk has no parent (or parent with invalid offset_end)
        {
            // Validate offset_end against file's (file win)
            if (offset_end > file_size)
                offset_end = file_size;
        }

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up...
        if (offset_end >= file_size)
        {
            TRACE_WARNING(RIF, "JUMPY > going EOF (%lli)", file_size);
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        TRACE_WARNING(RIF, "JUMPY > going from %lli to %lli", current_pos, offset_end);

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
