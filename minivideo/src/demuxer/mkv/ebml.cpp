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
 * \file      ebml.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "ebml.h"
#include "mkv_struct.h"

#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <cinttypes>

/* ************************************************************************** */

#define EBML_TICK 1000000000LL
#define SEC_TO_UNIX_EPOCH 978264000LL // 978285600LL

/*!
 * \param ebmlTime: Seconds since January 1, 2000
 * \return Unix time
 *
 * For infos on "EBML time":
 * - https://www.matroska.org/technical/specs/index.html
 * - https://en.wikipedia.org/wiki/Epoch_(reference_date)
 */
uint64_t EbmlTimeToUnixSeconds(int64_t ebmlTime)
{
     return (uint64_t)(ebmlTime / EBML_TICK + SEC_TO_UNIX_EPOCH);
}

/* ************************************************************************** */

/*!
 * \brief Read an EBML element ID.
 * \param *bitstr The bitstream to read.
 * \return The element ID.
 */
uint32_t read_ebml_eid(Bitstream_t *bitstr)
{
    uint32_t leadingZeroBits = 0;
    uint32_t elementSize = 0;
    uint32_t elementValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 4)
        leadingZeroBits++;

    elementSize = (leadingZeroBits + 1) * 7;
    elementValue = read_bits(bitstr, elementSize) + pow(2, elementSize);
/*
    TRACE_2(MKV, "read_ebml_eid()");
    bitstream_print_absolute_bit_offset(bitstr);
    TRACE_3(MKV, "- leadingZeroBits = %u", leadingZeroBits);
    TRACE_2(MKV, "- elementSize     = %u", elementSize);
    TRACE_2(MKV, "- elementValue    = 0x%0X", elementValue);
*/
    return elementValue;
}

/* ************************************************************************** */

/*!
 * \brief Read an EBML element size.
 * \param *bitstr The bitstream to read.
 * \return The element size.
 */
int64_t read_ebml_size(Bitstream_t *bitstr)
{
    int32_t leadingZeroBits = 0;
    int32_t sizeSize = 0;
    int64_t sizeValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 8)
        leadingZeroBits++;

    sizeSize = (leadingZeroBits + 1) * 7;
    sizeValue = (int64_t)read_bits_64(bitstr, sizeSize);

    TRACE_2(MKV, "read_ebml_size()");
    TRACE_3(MKV, "- leadingZeroBits = %i", leadingZeroBits);
    TRACE_2(MKV, "- sizeSize        = %i", sizeSize);
    TRACE_2(MKV, "- sizeValue       = %lli", sizeValue);

    return sizeValue;
}

/*!
 * \brief parse_ebml_element
 * \note 'bitstr' pointer is not checked for performance reasons.
 *
 * https://matroska.org/technical/specs/index.html
 * https://matroska.org/technical/specs/rfc/index.html
 */
int parse_ebml_element(Bitstream_t *bitstr, EbmlElement_t *element)
{
    TRACE_3(MKV, "parse_ebml_element()");
    int retcode = SUCCESS;

    if (element == NULL)
    {
        TRACE_ERROR(MKV, "Invalid EbmlElement_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Set element offset
        element->offset_start = bitstream_get_absolute_byte_offset(bitstr);

        // Read element ID
        {
            uint32_t ebml_leadingZeroBits = 0;
            uint32_t ebml_size = 0;

            while (read_bit(bitstr) == 0 && ebml_leadingZeroBits < 4)
                ebml_leadingZeroBits++;

            ebml_size = (ebml_leadingZeroBits + 1) * 7;
            element->eid_size = (ebml_size + ebml_leadingZeroBits + 1) / 8;
            element->eid = read_bits_64(bitstr, ebml_size) + pow(2, ebml_size);
        }

        // Read element size
        {
            uint32_t ebml_leadingZeroBits = 0;
            uint32_t ebml_size = 0;

            while (read_bit(bitstr) == 0 && ebml_leadingZeroBits < 8)
                ebml_leadingZeroBits++;

            ebml_size = (ebml_leadingZeroBits + 1) * 7;
            element->size_size = (ebml_size + ebml_leadingZeroBits + 1) / 8;
            element->size = read_bits_64(bitstr, ebml_size);
        }

        // Set end offset
        element->offset_end = element->offset_start + (element->eid_size + element->size_size + element->size);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print an element header.
 */
void print_ebml_element(EbmlElement_t *element)
{
#if ENABLE_DEBUG
    if (element == NULL)
    {
        TRACE_ERROR(RIF, "Invalid EbmlElement_t structure!");
    }
    else
    {
        TRACE_2(MKV, "* start offset  : %lli", element->offset_start);
        TRACE_2(MKV, "* end offset    : %lli", element->offset_end);

        // Print ebml element header
        TRACE_2(MKV, "* element ID    : 0x%X", element->eid);
        TRACE_2(MKV, "* element size  : %lli", element->size);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Write element header to a file for the xmlMapper.
 */
void write_ebml_element(EbmlElement_t *element, FILE *xml,
                        const char *title, const char *additional)
{
    if (xml)
    {
        if (element == nullptr)
        {
            TRACE_ERROR(MKV, "Invalid EbmlElement_t structure!");
        }
        else
        {
            char boxtitle[64];
            char boxtypeadd[16];

            if (title != nullptr)
                snprintf(boxtitle, 63, "tt=\"%s\" ", title);
            else
                boxtitle[0] = '\0';

            if (additional != nullptr)
                snprintf(boxtypeadd, 15, "add=\"%s\" ", additional);
            else
                boxtypeadd[0] = '\0';

            fprintf(xml, "  <a %s%stp=\"EBML\" id=\"0x%X\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                    boxtitle,
                    boxtypeadd,
                    element->eid,
                    element->offset_start,
                    (element->eid_size + element->size_size +  element->size));
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

uint64_t read_ebml_data_uint(Bitstream_t *bitstr, EbmlElement_t *element,
                             FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_uint(%i bits)", element->size*8);
    uint64_t value = read_bits_64(bitstr, element->size * 8);

    if (name)
    {
        TRACE_1(MKV, "* %s  = %llu", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;
}

uint64_t read_ebml_data_uint_UID(Bitstream_t *bitstr, EbmlElement_t *element,
                                 FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_uint_UID(%i bits)", element->size*8);
    uint64_t value = read_bits_64(bitstr, element->size * 8);

    if (name)
    {
        TRACE_1(MKV, "* %08X  = %llu", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>0x%" PRIX64 "</%s>\n", name, value, name);
        }
    }

    return value;
}

/* ************************************************************************** */

int64_t read_ebml_data_int(Bitstream_t *bitstr, EbmlElement_t *element,
                           FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_int2(%i bits)", element->size*8);
    int64_t value = (int64_t)read_bits_64(bitstr, element->size*8);

    if (name)
    {
        TRACE_1(MKV, "* %s  = %lli", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;
}

/* ************************************************************************** */

int64_t read_ebml_data_date(Bitstream_t *bitstr, EbmlElement_t *element,
                            FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_date2(%i bits)", element->size*8);
    int64_t value = (int64_t)read_bits_64(bitstr, 64);

    if (name)
    {
        TRACE_1(MKV, "* %s  = %lli", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;

}

/* ************************************************************************** */

double read_ebml_data_float(Bitstream_t *bitstr, EbmlElement_t *element,
                            FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_float2(%i bits)", element->size*8);
    double x = 0;

    if (element->size == 4)
    {
        char buf[4];
        for (int i = 3; i >= 0; i--)
            buf[i] = read_bits(bitstr, 8);
        x = *((float *)buf);
    }
    else if (element->size == 8)
    {
        char buf[8];
        for (int i = 7; i >= 0; i--)
            buf[i] = read_bits(bitstr, 8);
        x = *((double *)buf);
    }

    if (name)
    {
        TRACE_1(MKV, "* %s  = %f", name, x);
        if (xml)
        {
            fprintf(xml, "  <%s>%f</%s>\n", name, x, name);
        }
    }

    return x;
}

/* ************************************************************************** */

char *read_ebml_data_string(Bitstream_t *bitstr, EbmlElement_t *element,
                            FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_string(%i bytes)", element->size);

    char *value = new char [element->size+1];
    if (value)
    {
        for (int i = 0; i < element->size; i++)
            value[i] = read_bits(bitstr, 8);
        value[element->size] = '\0';

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

uint8_t *read_ebml_data_binary(Bitstream_t *bitstr, EbmlElement_t *element,
                               FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_binary2(%i bytes)", element->size);

    uint8_t *value = new uint8_t [element->size+1];
    if (value)
    {
        for (int i = 0; i < element->size; i++)
            value[i] = read_bits(bitstr, 8);
        value[element->size] = '\0';

        if (name)
        {
#if ENABLE_DEBUG
            TRACE_1(MKV, "* %s  = 0x", name);

            if (element->size > 1023)
                TRACE_1(MKV, "* %s  = (first 1024B) 0x", name);
            else
                TRACE_1(MKV, "* %s  = 0x", name);
            for (int i = 0; i < element->size && i < 1024; i++)
                printf("%02X", value[i]);
#endif // ENABLE_DEBUG

            if (xml)
            {
                if (element->size > 1023)
                    fprintf(xml, "  <%s>(first 1024B) 0x", name);
                else
                    fprintf(xml, "  <%s>0x", name);
                for (int i = 0; i < element->size && i < 1024; i++)
                    fprintf(xml, "%02X", value[i]);
                fprintf(xml, "</%s>\n", name);
            }
        }
    }

    return value;
}

/* ************************************************************************** */
/* ************************************************************************** */

int ebml_parse_void(Bitstream_t *bitstr, EbmlElement_t *element, FILE *xml)
{
    TRACE_INFO(MKV, "ebml_parse_void(%i bytes)", element->size);

    print_ebml_element(element);
    write_ebml_element(element, xml, "Void");
    if (xml) fprintf(xml, "  </a>\n");

    return SUCCESS;
}

/* ************************************************************************** */

int ebml_parse_unknown(Bitstream_t *bitstr, EbmlElement_t *element, FILE *xml)
{
    TRACE_WARNING(MKV, "ebml_parse_unknown(0x%X / %i bytes)", element->eid, element->size);

    print_ebml_element(element);
    write_ebml_element(element, xml, NULL);
    if (xml) fprintf(xml, "  </a>\n");

    return SUCCESS;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - MKV edition.
 * \param bitstr: Our bitstream reader.
 * \param parent: The element containing the current element we're in.
 * \param current: The current element we're in.
 * \return SUCCESS or FAILURE if the jump could not be done.
 *
 * 'Jumpy' is in charge of checking your position into the stream after your
 * parser finish parsing a box / list / chunk / element, never leaving you
 * stranded  in the middle of nowhere with no easy way to get back on track.
 * It will check available informations to known if the current element has been
 * fully parsed, and if not perform a jump (or even a rewind) to the next known
 * element.
 */
int jumpy_mkv(Bitstream_t *bitstr, EbmlElement_t *parent, EbmlElement_t *current)
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
        if (parent && parent->offset_end < file_size) // current element has valid parent
        {
            // Validate offset_end against parent's (parent win)
            if (offset_end > parent->offset_end)
                offset_end = parent->offset_end;
        }
        else // current element has no parent (or parent with invalid offset_end)
        {
            // Validate offset_end against file's (file win)
            if (offset_end > file_size)
                offset_end = file_size;
        }

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up...
        if (offset_end >= file_size)
        {
            TRACE_WARNING(MKV, "JUMPY > going EOF (%lli)", file_size);
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        //TRACE_WARNING(MKV, "JUMPY > going from %lli to %lli", current_pos, offset_end);
        //TRACE_WARNING(MKV, "JUMPY > FIXME FIXME FIXME");

        return bitstream_goto_offset(bitstr, offset_end); // FIXME

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
