/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

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
    elementValue = read_bits_64(bitstr, elementSize) + pow(2, elementSize);
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
    sizeValue = read_bits_64(bitstr, sizeSize);

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
        element->offset_end = element->offset_start + element->eid_size + element->size_size +  element->size;
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
void write_ebml_element(EbmlElement_t *element, FILE *xml, const char *title)
{
    if (xml)
    {
        if (element == NULL)
        {
            TRACE_ERROR(MKV, "Invalid EbmlElement_t structure!");
        }
        else
        {
            if (title != NULL)
                fprintf(xml, "  <atom title=\"%s\" id=\"0x%X\" type=\"EBML element\" offset=\"%"PRId64"\" size=\"%"PRId64"\">\n",
                        title,
                        element->eid,
                        element->offset_start,
                        (element->eid_size + element->size_size +  element->size));

            else
                fprintf(xml, "  <atom title=\"Unknown\" id=\"0x%X\" type=\"EBML element\" offset=\"%"PRId64"\" size=\"%"PRId64"\">\n",
                        element->eid,
                        element->offset_start,
                        (element->eid_size + element->size_size +  element->size));
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

uint64_t read_ebml_data_uint(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_uint()");
    return read_bits_64(bitstr, size*8);
}

uint64_t read_ebml_data_uint2(Bitstream_t *bitstr, EbmlElement_t *element,
                              FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_uint()");
    uint64_t value = read_bits_64(bitstr, element->size * 8);

    if (name)
    {
        TRACE_1(MKV, "* %s  = %llu", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%"PRId64"</%s>\n", name, value, name);
        }
    }

    return value;
}

/* ************************************************************************** */

int64_t read_ebml_data_int(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_int()");
    return (int64_t)read_bits_64(bitstr, size*8);
}

int64_t read_ebml_data_int2(Bitstream_t *bitstr, EbmlElement_t *element,
                            FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_int2()");
    int64_t value = (int64_t)read_bits_64(bitstr, element->size * 8);

    if (name)
    {
        TRACE_1(MKV, "* %s  = %lli", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%"PRId64"</%s>\n", name, value, name);
        }
    }

    return value;
}

/* ************************************************************************** */

int64_t read_ebml_data_date(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_date()");
    return (int64_t)read_bits_64(bitstr, size*8);
}

int64_t read_ebml_data_date2(Bitstream_t *bitstr, EbmlElement_t *element,
                             FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_date2()");
    return 0;
}

/* ************************************************************************** */

double read_ebml_data_float(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_float()");
    return 0;
}

double read_ebml_data_float2(Bitstream_t *bitstr, EbmlElement_t *element,
                             FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_float2()");
    return 0;
}

/* ************************************************************************** */

char *read_ebml_data_string(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_string(%i)", size);

    char *value = NULL;

    value = malloc(size+1);
    if (value)
    {
        if (value)
            for (int i = 0; i < size; i++)
                value[i] = read_bits(bitstr, 8);
        value[size] = '\0';
    }

    return value;
}

char *read_ebml_data_string2(Bitstream_t *bitstr, EbmlElement_t *element,
                             FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_string(%i)", element->size);

    char *value = NULL;

    value = malloc(element->size+1);
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

uint8_t *read_ebml_data_binary(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_binary(%i)", size);

    uint8_t *value = NULL;

    value = malloc(size+1);
    if (value)
    {
        if (value)
            for (int i = 0; i < size; i++)
                value[i] = read_bits(bitstr, 8);
        value[size] = '\0';
    }

    return value;
}

uint8_t *read_ebml_data_binary2(Bitstream_t *bitstr, EbmlElement_t *element,
                                FILE *xml, const char *name)
{
    TRACE_2(MKV, "read_ebml_data_binary2(%i)", element->size);

    uint8_t *value = NULL;

    value = malloc(element->size+1);
    if (value)
    {
        for (int i = 0; i < element->size; i++)
            value[i] = read_bits(bitstr, 8);
        value[element->size] = '\0';

        if (name)
        {
#if ENABLE_DEBUG
            TRACE_1(MKV, "* %s  = 0x", name);
            for (int i = 0; i < element->size; i++)
                printf("%X", value[i]);
#endif // ENABLE_DEBUG

            if (xml)
            {
                fprintf(xml, "  <%s>0x", name);
                for (int i = 0; i < element->size; i++)
                    fprintf(xml, "%X", value[i]);
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
    TRACE_INFO(MKV, "ebml_parse_void()");

    print_ebml_element(element);
    write_ebml_element(element, xml, "Void");
    if (xml) fprintf(xml, "  </atom>\n");

    return skip_bits(bitstr, element->size*8);
}

/* ************************************************************************** */

int ebml_parse_unknown(Bitstream_t *bitstr, EbmlElement_t *element, FILE *xml)
{
    TRACE_WARNING(MKV, "ebml_parse_unknown(0x%X)", element->eid);

    print_ebml_element(element);
    write_ebml_element(element, xml, NULL);
    if (xml) fprintf(xml, "  </atom>\n");

    return skip_bits(bitstr, element->size*8);
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - MKV edition.
 * \param bitstr
 * \param parent: The element containing the current element we're in.
 * \param current: The current element we're in.
 * \return
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
    int retcode = FAILURE;
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);

    if (current_pos != current->offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);
        int64_t offset_end = current->offset_end;

        // If the current element have a parent, and its offset_end is 'valid' (not past file size)
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
