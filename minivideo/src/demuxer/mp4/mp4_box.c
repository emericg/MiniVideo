/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      mp4_box.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "mp4_box.h"
#include "mp4_struct.h"

#include "../xml_mapper.h"
#include "../../fourcc.h"
#include "../../typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

/*!
 * \brief Parse box header.
 * \note 'bitstr' pointer is not checked for performance reasons.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 4.2 Object Structure.
 */
int parse_box_header(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_3(MP4, "parse_box_header()");
    int retcode = SUCCESS;

    if (box_header == NULL)
    {
        TRACE_ERROR(MP4, "Invalid Mp4Box_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Set box offset
        box_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);

        // Read box size
        box_header->size = (int64_t)read_bits(bitstr, 32);

        // Read box type
        box_header->boxtype = read_bits(bitstr, 32);

        if (box_header->size == 0)
        {
            // the size is the remaining space in the file
            box_header->size = bitstr->bitstream_size - box_header->offset_start;
        }
        else if (box_header->size == 1)
        {
            // the size is actually a 64b field coded right after the box type
            box_header->size = (int64_t)read_bits_64(bitstr, 64);
        }

        // Set end offset
        box_header->offset_end = box_header->offset_start + box_header->size;

        if (box_header->boxtype == BOX_UUID)
        {
            //box_header->usertype = malloc(16);
            //if (box_header->usertype)
            {
                int i = 0;
                for (i = 0; i < 16; i++)
                {
                    box_header->usertype[i] = (uint8_t)read_bits(bitstr, 8);
                }
            }
        }

        // Init "FullBox" parameters
        box_header->version = 0xFF;
        box_header->flags = 0xFFFFFFFF;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse fullbox header.
 * \note 'bitstr' pointer is not checked for performance reasons.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 4.2 Object Structure.
 */
int parse_fullbox_header(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_3(MP4, "parse_fullbox_header()");
    int retcode = SUCCESS;

    if (box_header == NULL)
    {
        TRACE_ERROR(MP4, "Invalid Mp4Box_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Read FullBox attributs
        box_header->version = (uint8_t)read_bits(bitstr, 8);
        box_header->flags = read_bits(bitstr, 24);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print a box header content.
 */
void print_box_header(Mp4Box_t *box_header)
{
#if ENABLE_DEBUG
    if (box_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid Mp4Box_t structure!");
    }
    else
    {
        TRACE_2(MP4, "* start offset  : %lli", box_header->offset_start);
        TRACE_2(MP4, "* end offset    : %lli", box_header->offset_end);

        // Print Box header
        if (box_header->size == 1)
        {
            TRACE_2(MP4, "* box largesize : %lli", box_header->size);
        }
        else
        {
            TRACE_2(MP4, "* box size      : %lli", box_header->size);
        }

        TRACE_2(MP4, "* box type      : 0x%X", box_header->boxtype);
        if (box_header->boxtype == BOX_UUID)
        {
            TRACE_2(MP4, "* box usertype  : '%s'", box_header->usertype);
        }

        // Print FullBox header
        if (box_header->version != 0xFF || box_header->flags != 0xFFFFFFFF)
        {
            TRACE_2(MP4, "* version       : %u", box_header->version);
            TRACE_2(MP4, "* flags         : 0x%X", box_header->flags);
        }
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Write box header content to a file for the xmlMapper.
 */
void write_box_header(Mp4Box_t *box_header, FILE *xml)
{
    if (xml)
    {
        if (box_header == NULL)
        {
            TRACE_ERROR(MP4, "Invalid Mp4Box_t structure!");
        }
        else
        {
            char fcc[5];

            if (box_header->version == 0xFF && box_header->flags == 0xFFFFFFFF)
            {
                fprintf(xml, "  <atom fcc=\"%s\" type=\"MP4 box\" offset=\"%li\" size=\"%li\">\n",
                        getFccString_le(box_header->boxtype, fcc),
                        box_header->offset_start,
                        box_header->size);
            }
            else
            {
                fprintf(xml, "  <atom fcc=\"%s\" type=\"MP4 fullbox\" offset=\"%li\" size=\"%li\">\n",
                        getFccString_le(box_header->boxtype, fcc),
                        box_header->offset_start,
                        box_header->size);
            }
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Unknown box, just parse header.
 *
 * When encountering an unknown box type, just print the header infos, the box
 * will be automatically skipped.
 */
int parse_unknown_box(Bitstream_t *bitstr, Mp4Box_t *box_header, FILE *xml)
{
    char fcc[5];

#if ENABLE_DEBUG
    TRACE_WARNING(MP4, BLD_GREEN "parse_unknown_box('%s' @ %lli; size is %u)" CLR_RESET,
                  getFccString_le(box_header->boxtype, fcc), box_header->offset_start,
                  box_header->offset_end - box_header->offset_start);

    print_box_header(box_header);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (xml)
    {
        write_box_header(box_header, xml);
        fprintf(xml, "  </atom>\n");
    }

    return SUCCESS;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - MP4 edition.
 * \param parent: The box containing the current box we're in.
 * \param current: The current box we're in.
 *
 * 'Jumpy' is in charge of checking your position into the stream after your
 * parser finish parsing a box / list / chunk / element, never leaving you
 * stranded  in the middle of nowhere with no easy way to get back on track.
 * It will check available informations to known if the current element has been
 * fully parsed, and if not perform a jump (or even a rewind) to the next known
 * element.
 */
int jumpy_mp4(Bitstream_t *bitstr, Mp4Box_t *parent, Mp4Box_t *current)
{
    int retcode = FAILURE;
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);

    if (current_pos != current->offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);
        int64_t offset_end = current->offset_end;

        // If the current box have a parent, and its offset_end is 'valid' (not past file size)
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
