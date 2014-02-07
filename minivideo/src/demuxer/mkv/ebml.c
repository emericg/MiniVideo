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

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// minivideo headers
#include "../../minitraces.h"
#include "../../typedef.h"

#include "ebml.h"
#include "mkv_struct.h"

/* ************************************************************************** */

int ebml_parse_header(Bitstream_t *bitstr)
{
    TRACE_INFO(MKV, GREEN "ebml_parse_header()\n" RESET);
    int retcode = FAILURE;

    uint64_t headerSize = 0;
    uint64_t headerOffset = 0;

    uint64_t EBMLVersion = 1;
    uint64_t EBMLReadVersion = 1;
    uint64_t EBMLMaxIDLength = 4;
    uint64_t EBMLMaxSizeLength = 8;
    uint64_t DocType = 0;
    uint64_t DocTypeVersion = 1;
    uint64_t DocTypeReadVersion = 1;

    const uint8_t DocType_mkv[8] = {'m', 'a', 't', 'r', 'o', 's', 'k', 'a'};
    const uint8_t DocType_webm[4] = {'w', 'e', 'b', 'm'};

    if (read_bits(bitstr, 32) == element_EBML)
    {
        headerSize = read_ebml_size(bitstr);
        headerOffset = bitstream_get_absolute_byte_offset(bitstr);

        while (bitstream_get_absolute_byte_offset(bitstr) < (headerSize + headerOffset))
        {
            //switch (read_ebml_eid(bitstr))
            switch (read_bits(bitstr, 16))
            {
                case element_EBMLVersion:
                    EBMLVersion = read_ebml_data_uint(bitstr);
                    break;
                case element_EBMLReadVersion:
                    EBMLReadVersion = read_ebml_data_uint(bitstr);
                    break;
                case element_EBMLMaxIDLength:
                    EBMLMaxIDLength = read_ebml_data_uint(bitstr);
                    break;
                case element_EBMLMaxSizeLength:
                    EBMLMaxSizeLength = read_ebml_data_uint(bitstr);
                    break;
                case element_DocType:
                    DocType = doctype_matroska;
                    read_bits(bitstr, 8);
                    read_bits_64(bitstr, 64);
                    retcode = SUCCESS;
                    break;
                case element_DocTypeVersion:
                    DocTypeVersion = read_ebml_data_uint(bitstr);
                    break;
                case element_DocTypeReadVersion:
                    DocTypeReadVersion = read_ebml_data_uint(bitstr);
                    break;
                default:
                    TRACE_WARNING(MKV, "Unknown ebml element inside header\n");
                    bitstream_force_alignment(bitstr);
                    break;
            }
        }

        // Goto the end of the element

        TRACE_2(MKV, "header size        = %llu\n", headerSize);
        TRACE_2(MKV, "header offset      = %llu\n", headerOffset);
        TRACE_1(MKV, "EBMLVersion        = %llu\n", EBMLVersion);
        TRACE_1(MKV, "EBMLReadVersion    = %llu\n", EBMLReadVersion);
        TRACE_1(MKV, "EBMLMaxIDLength    = %llu\n", EBMLMaxIDLength);
        TRACE_1(MKV, "EBMLMaxSizeLength  = %llu\n", EBMLMaxSizeLength);
        TRACE_1(MKV, "DocType            = %llu\n", DocType);
        TRACE_1(MKV, "DocTypeVersion     = %llu\n", DocTypeVersion);
        TRACE_1(MKV, "DocTypeReadVersion = %llu\n", DocTypeReadVersion);
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Read an EBML element.
 * \param *bitstr The bitstream to read.
 * \return The value of the next element.
 */
uint32_t read_ebml_eid(Bitstream_t *bitstr)
{
    TRACE_2(MKV, "read_ebml_eid()\n");
    bitstream_print_absolute_bit_offset(bitstr);

    uint32_t leadingZeroBits = 0;
    uint32_t elementSize = 0;
    uint32_t elementValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 4)
        leadingZeroBits++;

    elementSize = (leadingZeroBits + 1) * 7;
    elementValue = read_bits_64(bitstr, elementSize) + pow(2, elementSize);
/*
    TRACE_3(MKV, "- leadingZeroBits = %u\n", leadingZeroBits);
    TRACE_2(MKV, "- elementSize     = %u\n", elementSize);
    TRACE_2(MKV, "- elementValue    = 0x%0X\n\n", elementValue);
*/
    return elementValue;
}

/* ************************************************************************** */

/*!
 * \brief Read an EBML element.
 * \param *bitstr The bitstream to read.
 * \return The value of the next element.
 */
uint64_t read_ebml_size(Bitstream_t *bitstr)
{
    TRACE_2(MKV, "read_ebml_size()\n");

    uint32_t leadingZeroBits = 0;
    uint32_t sizeSize = 0;
    uint64_t sizeValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 8)
        leadingZeroBits++;

    sizeSize = (leadingZeroBits + 1) * 7;
    sizeValue = read_bits_64(bitstr, sizeSize);
/*
    TRACE_3(MKV, "- leadingZeroBits = %u\n", leadingZeroBits);
    TRACE_2(MKV, "- sizeSize        = %u\n", sizeSize);
    TRACE_2(MKV, "- sizeValue       = %llu\n\n", sizeValue);
*/
    return sizeValue;
}

/* ************************************************************************** */

/*!
 * \brief Read an EBML element.
 * \param *bitstr The bitstream to read.
 * \return The value of the next element.
 */
uint64_t read_ebml_data_uint(Bitstream_t *bitstr)
{
    TRACE_2(MKV, "read_ebml_data_uint()\n");

    uint32_t leadingZeroBits = 0;
    uint32_t elementSizeSize = 0;
    uint32_t elementSize = 0;
    uint64_t elementValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 8)
        leadingZeroBits++;

    elementSizeSize = (leadingZeroBits + 1) * 7;
    elementSize = read_bits(bitstr, elementSizeSize) * 8;
    elementValue = read_bits_64(bitstr, elementSize);
/*
    TRACE_3(MKV, "- leadingZeroBits = %u\n", leadingZeroBits);
    TRACE_3(MKV, "- elementSizeSize = %u\n", elementSizeSize);
    TRACE_3(MKV, "- elementSize     = %u\n", elementSize);
    TRACE_2(MKV, "- elementValue    = %u\n\n", elementValue);
*/
    return elementValue;
}

/* ************************************************************************** */

/*!
 * \brief Read an EBML element.
 * \param *bitstr The bitstream to read.
 * \return The value of the next element.
 */
uint64_t read_ebml_data_binary(Bitstream_t *bitstr)
{
    TRACE_2(MKV, "read_ebml_data_binary()\n");

    uint32_t leadingZeroBits = 0;
    uint32_t elementSize = 0;
    uint64_t elementValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 8)
        leadingZeroBits++;

    elementSize = (leadingZeroBits + 1) * 7;
    elementValue = read_bits_64(bitstr, elementSize);
/*
    TRACE_3(MKV, "- leadingZeroBits = %u\n", leadingZeroBits);
    TRACE_3(MKV, "- elementSize     = %u\n", elementSize);
    TRACE_2(MKV, "- elementValue    = %u\n\n", elementValue);
*/
    return elementValue;
}

/* ************************************************************************** */
