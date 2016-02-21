/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      h264_nalu.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_nalu.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ************************************************************************** */

/*!
 * \brief Allocate and initialize a new NAL Unit.
 * \return A pointer to the newly allocated NAL Unit.
 */
nalu_t *initNALU(void)
{
    TRACE_INFO(NALU, BLD_GREEN "initNALU()\n" CLR_RESET);

    // NAL Unit allocation
    nalu_t *nalu = NULL;
    if ((nalu = (nalu_t*)calloc(1, sizeof(nalu_t))) == NULL)
    {
        TRACE_ERROR(NALU, "Unable to alloc new NAL Unit!\n");
    }
    else
    {
        TRACE_1(NALU, "* NAL Unit allocation success\n");
    }

    // Return the NALU
    return nalu;
}

/* ************************************************************************** */

/*!
 * \brief Set default values for a given NAL Unit.
 * \param *nalu The NAL Unit to initialize.
 * \return 1 if success, 0 otherwise.
 */
void nalu_reset(nalu_t *nalu)
{
    TRACE_INFO(NALU, BLD_GREEN "nalu_reset()\n\n" CLR_RESET);

    if (nalu != NULL)
    {
        // Set default params
        nalu->nal_offset = 0;

        nalu->nal_ref_idc = 0;
        nalu->nal_unit_type = 0;

        nalu->idr_flag = 0;
        nalu->priority_id = 0;
        nalu->no_inter_layer_pred_flag = 0;
        nalu->dependency_id = 0;
        nalu->quality_id = 0;
        nalu->temporal_id = 0;
        nalu->use_ref_base_pic_flag = 0;
        nalu->discardable_flag = 0;
        nalu->output_flag = 0;

        TRACE_2(NALU, "* NAL Unit re-initialization success\n");
    }
}

/* ************************************************************************** */

/*!
 * \brief Parse the content of a NAL Unit header.
 * \param *bitstr The bitstream to use.
 * \param *nalu A NAL Unit structure.
 * \return 0 if no NAL Unit is found, 1 otherwise.
 *
 * A NAL Unit header contain (at least) 3 informations:
 * - A forbidden_zero_bit equal to 0. Useful to check NAL Unit validity.
 * - A nal_ref_idc, which can be used to indicate NAL Unit priority and content.
 * - A nal_unit_type, which indicate precisly the content of the NAL Unit.
 *
 * If nal_unit_type is 14 or 20, it indicate the presence of three additional
 * octets in the NAL unit header:
 * http://r2d2n3po.tistory.com/26
 */
int nalu_parse_header(Bitstream_t *bitstr, nalu_t *nalu)
{
    TRACE_INFO(NALU, "> " BLD_GREEN "nalu_parse_header()\n" CLR_RESET);
    int retcode = FAILURE;

    // Set NAL Unit header offset
    nalu->nal_offset = bitstream_get_absolute_byte_offset(bitstr);

    // Check forbidden_zero_bit
    if (read_bit(bitstr) == 0)
    {
        nalu->nal_ref_idc = read_bits(bitstr, 2);
        nalu->nal_unit_type = read_bits(bitstr, 5);

        TRACE_1(NALU, "  - nal_ref_idc\t= 0x%02X\n", nalu->nal_ref_idc);
        TRACE_1(NALU, "  - nal_unit_type\t= 0x%02X\n", nalu->nal_unit_type);

        if (nalu->nal_unit_type == 14 || nalu->nal_unit_type == 20)
        {
            // Check reserved_one_bit
            if (read_bit(bitstr) == 0)
            {
                nalu->idr_flag = read_bit(bitstr);
                nalu->priority_id = read_bits(bitstr, 6);
                nalu->no_inter_layer_pred_flag = read_bit(bitstr);
                nalu->dependency_id = read_bits(bitstr, 3);
                nalu->quality_id = read_bits(bitstr, 4);
                nalu->temporal_id = read_bits(bitstr, 3);
                nalu->use_ref_base_pic_flag = read_bit(bitstr);
                nalu->discardable_flag = read_bit(bitstr);
                nalu->output_flag = read_bit(bitstr);

                TRACE_1(NALU, "  - idr_flag \t\t= %u\n", nalu->idr_flag);
                TRACE_1(NALU, "  - priority_id \t\t= %u\n", nalu->priority_id);
                TRACE_1(NALU, "  - no_inter_layer_pred_flag= %u\n", nalu->no_inter_layer_pred_flag);
                TRACE_1(NALU, "  - dependency_id \t\t= %u\n", nalu->dependency_id);
                TRACE_1(NALU, "  - quality_id \t\t= %u\n", nalu->quality_id);
                TRACE_1(NALU, "  - temporal_id \t\t= %u\n", nalu->temporal_id);
                TRACE_1(NALU, "  - use_ref_base_pic_flag \t= %u\n", nalu->use_ref_base_pic_flag);
                TRACE_1(NALU, "  - discardable_flag \t= %u\n", nalu->discardable_flag);
                TRACE_1(NALU, "  - output_flag \t\t= %u\n", nalu->output_flag);

                // Check reserved_three_2bits
                if (read_bits(bitstr, 2) == 3)
                {
                    retcode = SUCCESS;
                    TRACE_1(NALU, "  * NAL Unit confirmed at byte offset %i\n", nalu->nal_offset);
                }
                else
                {
                    TRACE_ERROR(NALU, "  * There is no valid NAL Unit at byte offset %i (wrong reserved_three_2bits value)\n", nalu->nal_offset);
                }
            }
            else
            {
                TRACE_ERROR(NALU, "  * There is no valid NAL Unit at byte offset %i (wrong reserved_one_bit value)\n", nalu->nal_offset);
            }
        }
        else
        {
            retcode = SUCCESS;
            TRACE_1(NALU, "  * NAL Unit confirmed at byte offset %i\n", nalu->nal_offset);
        }
    }
    else
    {
        TRACE_ERROR(NALU, "  * There is no valid NAL Unit at byte offset %i (wrong forbidden_zero_bit value)\n", nalu->nal_offset);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Clean one nalu sample from emulation_prevention_three_byte codes.
 * \param *bitstr The bitstream to use.
 * \return The number of discarded bytes.
 *
 * Clean current nalu sample from emulation_prevention_three_byte 4 byte codes
 * (00 00 03 0x) and replaces them by original 3 byte codes (00 00 0x).
 *
 * See "emulation_prevention_three_byte" section in "7.4.1 NAL unit semantics"
 * and "7.4.1.1 Encapsulation of an SODB within an RBSP"
 * for more informations about that issue.
 */
int nalu_clean_sample(Bitstream_t *bitstr)
{
    TRACE_INFO(NALU, BLD_GREEN "nalu_clean_sample()\n" CLR_RESET);

    unsigned int buffer_offset_saved = bitstr->buffer_offset;

    unsigned int consecutive_zeros = 0;
    unsigned int current_byte = 0;
    unsigned int current_byte_offset = ceil(bitstr->buffer_offset/8.0);

    // Search for start_code_prefix & emulation_prevention_three_byte
    while (current_byte_offset < bitstr->buffer_size)
    {
        // Read next byte
        current_byte = bitstr->buffer[current_byte_offset];

        if (current_byte == 0x00)
        {
            consecutive_zeros++;
            current_byte_offset++;
        }
        else if (consecutive_zeros > 1)
        {
            if (current_byte == 0x03)
            {
                //bitstream_print_stats(bitstr);
                //bitstream_print_buffer(bitstr);

                // Move datas
                memmove((bitstr->buffer + current_byte_offset), (bitstr->buffer + current_byte_offset + 1), (bitstr->buffer_size - current_byte_offset - 1));

                // Discard byte(s)
                bitstr->buffer_size--;
                bitstr->buffer_discarded_bytes++;

                TRACE_2(NALU, "emulation_prevention_three_byte removed at buffer offset: %i\n", current_byte_offset);
                TRACE_2(NALU, "emulation_prevention_three_byte removed at bitstream offset: %i\n", bitstream_get_absolute_byte_offset(bitstr));
            }

            // Reset search
            consecutive_zeros = 0;
        }
        else
        {
            // Reset search
            consecutive_zeros = 0;
            current_byte_offset++;
        }
    }

    // Reset position into the buffer
    bitstr->buffer_offset = buffer_offset_saved;

    return bitstr->buffer_discarded_bytes;
}

/* ************************************************************************** */
