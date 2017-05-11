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
int parse_ebml_element(Bitstream_t *bitstr, EbmlElement_t *ebml_element)
{
    TRACE_3(MKV, "parse_ebml_element()");
    int retcode = SUCCESS;

    if (ebml_element == NULL)
    {
        TRACE_ERROR(MKV, "Invalid EbmlElement_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Set element offset
        ebml_element->offset_start = bitstream_get_absolute_byte_offset(bitstr);

        // Read element ID
        {
            uint32_t ebml_leadingZeroBits = 0;
            uint32_t ebml_size = 0;

            while (read_bit(bitstr) == 0 && ebml_leadingZeroBits < 4)
                ebml_leadingZeroBits++;

            ebml_size = (ebml_leadingZeroBits + 1) * 7;
            ebml_element->eid_size = (ebml_size + ebml_leadingZeroBits + 1) / 8;
            ebml_element->eid = read_bits_64(bitstr, ebml_size) + pow(2, ebml_size);
        }

        // Read element size
        {
            uint32_t ebml_leadingZeroBits = 0;
            uint32_t ebml_size = 0;

            while (read_bit(bitstr) == 0 && ebml_leadingZeroBits < 8)
                ebml_leadingZeroBits++;

            ebml_size = (ebml_leadingZeroBits + 1) * 7;
            ebml_element->size_size = (ebml_size + ebml_leadingZeroBits + 1) / 8;
            ebml_element->size = read_bits_64(bitstr, ebml_size);
        }

        // Set end offset
        ebml_element->offset_end = ebml_element->offset_start + ebml_element->eid_size + ebml_element->size;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print an element header.
 */
void print_ebml_element(EbmlElement_t *ebml_element)
{
#if ENABLE_DEBUG
    if (ebml_element == NULL)
    {
        TRACE_ERROR(RIF, "Invalid EbmlElement_t structure!");
    }
    else
    {
        TRACE_2(MKV, "* start offset  : %lli", ebml_element->offset_start);
        TRACE_2(MKV, "* end offset    : %lli", ebml_element->offset_end);

        // Print ebml element header
        TRACE_2(MKV, "* element ID    : 0x%X", ebml_element->eid);
        TRACE_2(MKV, "* element size  : %lli", ebml_element->size);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Write element header to a file for the xmlMapper.
 */
void write_ebml_element(EbmlElement_t *ebml_element, FILE *xml)
{
    if (xml)
    {
        if (ebml_element == NULL)
        {
            TRACE_ERROR(MKV, "Invalid EbmlElement_t structure!");
        }
        else
        {
            fprintf(xml, "  <atom fcc=\"%X\" type=\"EBML element\" offset=\"%"PRId64"\" size=\"%"PRId64"\">\n",
                    ebml_element->eid,
                    ebml_element->offset_start,
                    ebml_element->size);
        }
    }
}

/*!
 * \brief Write element header and title to a file for the xmlMapper.
 */
void write_ebml_element_title(EbmlElement_t *ebml_element, FILE *xml, const char *title)
{
    if (xml)
    {
        write_ebml_element(ebml_element, xml);
        fprintf(xml, "  <title>%s</title>\n", title);
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

uint64_t read_ebml_data_uint(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_uint()");
    return read_bits_64(bitstr, size*8);
}

/* ************************************************************************** */

int64_t read_ebml_data_int(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_int()");
    return (int64_t)read_bits_64(bitstr, size*8);
}

/* ************************************************************************** */

int64_t read_ebml_data_date(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_int()");
    return (int64_t)read_bits_64(bitstr, size*8);
}

/* ************************************************************************** */

double read_ebml_data_float(Bitstream_t *bitstr, int size)
{
    TRACE_2(MKV, "read_ebml_data_float()");
    return 0;
}

/* ************************************************************************** */

uint8_t *read_ebml_data_string(Bitstream_t *bitstr, int size)
{
    TRACE_1(MKV, "read_ebml_data_string(%i)", size);

    uint8_t *elementValue = NULL;

    elementValue = malloc(size+1);
    if (elementValue)
        for (int i = 0; i < size; i++)
            elementValue[i] = read_bits(bitstr, 8);
    elementValue[size] = '\0';
/*
    TRACE_1(MKV, "- elementSize     = %u", size);
    TRACE_1(MKV, "- elementValue    = ");
    if (elementValue)
        for (int i = 0; i < size; i++)
            printf("0x%X ", elementValue[i]);
*/
    return elementValue;
}

/* ************************************************************************** */

uint8_t *read_ebml_data_binary(Bitstream_t *bitstr, int size)
{
    TRACE_1(MKV, "read_ebml_data_binary(%i)", size);

    uint8_t *elementValue = NULL;

    elementValue = malloc(size+1);
    if (elementValue)
        for (int i = 0; i < size; i++)
            elementValue[i] = read_bits(bitstr, 8);
    elementValue[size] = '\0';
/*
    TRACE_1(MKV, "- elementSize     = %u", size);
    TRACE_1(MKV, "- elementValue    = ");
    if (elementValue)
        for (int i = 0; i < size; i++)
            printf("0x%X ", elementValue[i]);
*/
    return elementValue;
}

/* ************************************************************************** */
/* ************************************************************************** */

int ebml_parse_void(Bitstream_t *bitstr, EbmlElement_t *ebml_element, FILE *xml)
{
    TRACE_INFO(MKV, "ebml_parse_void()");

#if ENABLE_DEBUG
    print_ebml_element(ebml_element);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (xml)
    {
        write_ebml_element(ebml_element, xml);
        fprintf(xml, "  <title>Void</title>\n");
        fprintf(xml, "  </atom>\n");
    }

    return skip_bits(bitstr, ebml_element->size*8);
}

/* ************************************************************************** */

int ebml_parse_unknown(Bitstream_t *bitstr, EbmlElement_t *ebml_element, FILE *xml)
{
    TRACE_WARNING(MKV, "ebml_parse_unknown()");

#if ENABLE_DEBUG
    print_ebml_element(ebml_element);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (xml)
    {
        write_ebml_element(ebml_element, xml);
        fprintf(xml, "  <title>UNKNOWN</title>\n");
        fprintf(xml, "  </atom>\n");
    }

    return FAILURE; // FIXME needs to be recursive
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
    return SUCCESS; // FIXME

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
