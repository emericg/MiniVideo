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
 * \file      mp4_box.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "mp4_box.h"
#include "mp4_struct.h"

#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_uuid.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>
#include <cinttypes>

/* ************************************************************************** */

#define SEC_TO_UNIX_EPOCH 2082844800LL

/*!
 * \param LabVIEWTime: Seconds since January 1, 1904
 * \return Unix time
 *
 * For infos on "LabVIEW time":
 * - https://en.wikipedia.org/wiki/Epoch_(reference_date)
 */
uint64_t LabVIEWTimeToUnixSeconds(int64_t LabVIEWTime)
{
    return (uint64_t)(LabVIEWTime - SEC_TO_UNIX_EPOCH);
}

/* ************************************************************************** */
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

    if (box_header == nullptr)
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
            read_uuid_be(bitstr, box_header->usertype);
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

    if (box_header == nullptr)
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
    if (box_header == nullptr)
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
            char guid_str[39];
            getGuidString(box_header->usertype, guid_str);
            TRACE_2(MP4, "* box usertype  : %s", guid_str);
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
void write_box_header(Mp4Box_t *box_header, FILE *xml,
                      const char *title, const char *additional)
{
    if (xml)
    {
        if (box_header == nullptr)
        {
            TRACE_ERROR(MP4, "Invalid Mp4Box_t structure!");
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

            if (box_header->version == 0xFF && box_header->flags == 0xFFFFFFFF)
                strcpy(boxtype, "tp=\"MP4 box\" ");
            else
                strcpy(boxtype, "tp=\"MP4 fullbox\" ");

            if (additional != nullptr)
                snprintf(boxtypeadd, 15, "add=\"%s\" ", additional);
            else
                boxtypeadd[0] = '\0';

            fprintf(xml, "  <a %s%s%sfcc=\"%s\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                    boxtitle, boxtype, boxtypeadd,
                    getFccString_le(box_header->boxtype, fcc),
                    box_header->offset_start,
                    box_header->size);

            if (box_header->boxtype == BOX_UUID)
            {
                char guid_str[39];
                getGuidString(box_header->usertype, guid_str);
                fprintf(xml, "  <usertype>%s</usertype>\n", guid_str);
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
#if ENABLE_DEBUG
    char fcc[5];
    TRACE_WARNING(MP4, BLD_GREEN "parse_unknown_box('%s' @ %lli; size is %u)" CLR_RESET,
                  getFccString_le(box_header->boxtype, fcc), box_header->offset_start,
                  box_header->offset_end - box_header->offset_start);

    print_box_header(box_header);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (xml)
    {
        write_box_header(box_header, xml, nullptr);
        fprintf(xml, "  </a>\n");
    }

    return SUCCESS;
}

/* ************************************************************************** */
/* ************************************************************************** */

uint64_t read_mp4_uint64(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_uint64()");
    uint64_t value = read_bits_64(bitstr, 64);

    if (name)
    {
        TRACE_1(MP4, "* %s  = %u", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;
}
uint32_t read_mp4_uint32(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_uint32()");
    uint32_t value = read_bits(bitstr, 32);

    if (name)
    {
        TRACE_1(MP4, "* %s  = %u", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%u</%s>\n", name, value, name);
        }
    }

    return value;
}
uint16_t read_mp4_uint16(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_uint16()");
    uint16_t value = read_bits(bitstr, 16);

    if (name)
    {
        TRACE_1(MP4, "* %s  = %u", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%u</%s>\n", name, value, name);
        }
    }

    return value;
}
uint8_t read_mp4_uint8(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_uint8()");
    uint8_t value = read_bits(bitstr, 8);

    if (name)
    {
        TRACE_1(MP4, "* %s  = %u", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%u</%s>\n", name, value, name);
        }
    }

    return value;
}
uint32_t read_mp4_uint(Bitstream_t *bitstr, int bits, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_uint()");
    uint32_t value = read_bits(bitstr, bits);

    if (name)
    {
        TRACE_1(MP4, "* %s  = %u", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%u</%s>\n", name, value, name);
        }
    }

    return value;
}
uint8_t *read_mp4_data(Bitstream_t *bitstr, int bytes, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_data()");

    uint8_t *value = (uint8_t *)malloc(bytes+1);
    if (value)
    {
        for (int i = 0; i < bytes; i++)
            value[i] = read_bits(bitstr, 8);
        value[bytes] = '\0';

        if (name)
        {
#if ENABLE_DEBUG
            TRACE_1(MP4, "* %s  = 0x", name);

            if (bytes > 1023)
                TRACE_1(MP4, "* %s  = (first 1024B) 0x", name);
            else
                TRACE_1(MP4, "* %s  = 0x", name);
            for (int i = 0; i < bytes && i < 1024; i++)
                printf("%02X", value[i]);
#endif // ENABLE_DEBUG

            if (xml)
            {
                if (bytes > 1023)
                    fprintf(xml, "  <%s>(first 1024B) 0x", name);
                else
                    fprintf(xml, "  <%s>0x", name);
                for (int i = 0; i < bytes && i < 1024; i++)
                    fprintf(xml, "%02X", value[i]);
                fprintf(xml, "</%s>\n", name);
            }
        }
    }

    return value;
}

char *read_mp4_string(Bitstream_t *bitstr, int bytes, FILE *xml, const char *name)
{
    TRACE_2(MP4, "read_mp4_string()");

    char *value = (char *)malloc(bytes+1);
    if (value)
    {
        for (int i = 0; i < bytes; i++)
            value[i] = (char)read_bits(bitstr, 8);
        value[bytes] = '\0';

        if (name)
        {
            TRACE_1(MKV, "* %s  = '%s'", name, value);

            if (xml)
            {
                fprintf(xml, "  <%s>%s</%s>\n", name, value, name);
            }
        }
    }

    return value;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - MP4 edition.
 * \param bitstr: Our bitstream reader.
 * \param parent: The box containing the current box we're in.
 * \param current: The current box we're in.
 * \return SUCCESS or FAILURE if the jump could not be done.
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
    int retcode = SUCCESS;

    // Done as a precaution, because the parsing of some boxes (like ESDS...)
    // can leave us in the middle of a byte and that will never be caught by
    // offset checks (cause they works on the assumption that we are byte aligned)
    bitstream_force_alignment(bitstr);

    // Check if we need a jump
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);
    if (current_pos != current->offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);
        int64_t offset_end = current->offset_end;

        // Check offset_end
        if (parent && parent->offset_end < file_size) // current box has valid parent
        {
            // Validate offset_end against parent's (parent win)
            if (offset_end > parent->offset_end)
                offset_end = parent->offset_end;
        }
        else // current box has no parent (or parent with invalid offset_end)
        {
            // Validate offset_end against file's (file win)
            if (offset_end > file_size)
                offset_end = file_size;
        }

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up...
        if (offset_end >= file_size)
        {
            TRACE_WARNING(MP4, "JUMPY > going EOF (%lli)", file_size);
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        TRACE_WARNING(MP4, "JUMPY > going from %lli to %lli", current_pos, offset_end);

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
