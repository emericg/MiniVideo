/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
#include "../../minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

#define start_code_prefix                   0x000001
#define emulation_prevention_three_byte     0x000003

/* ************************************************************************** */

/*!
 * \brief Allocate and initialize a new NAL Unit.
 * \return A pointer to the newly allocated NAL Unit.
 */
h264_nalu_t *h264_nalu_init(void)
{
    TRACE_INFO(NALU, BLD_GREEN "h264_nalu_init()" CLR_RESET);

    // NAL Unit allocation
    h264_nalu_t *nalu = NULL;
    if ((nalu = (h264_nalu_t*)calloc(1, sizeof(h264_nalu_t))) == NULL)
    {
        TRACE_ERROR(NALU, "Unable to alloc new NAL Unit!");
    }
    else
    {
        TRACE_1(NALU, "* NAL Unit allocation success");
    }

    return nalu;
}

/* ************************************************************************** */

/*!
 * \brief Set default values for a given NAL Unit.
 * \param *nalu The NAL Unit to initialize.
 * \return 1 if success, 0 otherwise.
 */
void h264_nalu_reset(h264_nalu_t *nalu)
{
    TRACE_INFO(NALU, BLD_GREEN "h264_nalu_reset()" CLR_RESET);

    if (nalu != NULL)
    {
        nalu->nal_offset = 0;

        // Set default params
        nalu->nal_ref_idc = 0;
        nalu->nal_unit_type = 0;
        // Extensions
        nalu->idr_flag = 0;
        nalu->priority_id = 0;
        nalu->no_inter_layer_pred_flag = 0;
        nalu->dependency_id = 0;
        nalu->quality_id = 0;
        nalu->temporal_id = 0;
        nalu->use_ref_base_pic_flag = 0;
        nalu->discardable_flag = 0;
        nalu->output_flag = 0;

        TRACE_2(NALU, "* NAL Unit re-initialization success");
    }
}

/* ************************************************************************** */

/*!
 * \brief Parse the content of a NAL Unit header.
 * \param *bitstr The bitstream to use.
 * \param *nalu A NAL Unit structure.
 * \return 0 if no NAL Unit is found, 1 otherwise.
 *
 * A NAL Unit header contain (at least) 3 information:
 * - A forbidden_zero_bit equal to 0. Useful to check NAL Unit validity.
 * - A nal_ref_idc, which can be used to indicate NAL Unit priority and content.
 * - A nal_unit_type, which indicate precisly the content of the NAL Unit.
 *
 * If nal_unit_type is 14 or 20, it indicate the presence of three additional
 * bytes in the NAL unit header, corresponding to annex G or H of the specification.
 */
int h264_nalu_parse_header(Bitstream_t *bitstr, h264_nalu_t *nalu)
{
    TRACE_INFO(NALU, "> " BLD_GREEN "h264_nalu_parse_header()" CLR_RESET);
    int retcode = FAILURE;

    if (bitstr && nalu)
    {
        // Save NAL Unit header offset
        nalu->nal_offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check forbidden_zero_bit
        if (read_bit(bitstr) == 0)
        {
            nalu->nal_ref_idc = read_bits(bitstr, 2);
            nalu->nal_unit_type = read_bits(bitstr, 5);

            TRACE_1(NALU, "  - nal_ref_idc   = 0x%02X", nalu->nal_ref_idc);
            TRACE_1(NALU, "  - nal_unit_type = 0x%02X", nalu->nal_unit_type);

            if (nalu->nal_unit_type == 14 ||
                nalu->nal_unit_type == 20 ||
                nalu->nal_unit_type == 21)
            {
                bool svc_extension_flag = false;
                bool avc_3d_extension_flag = false;

                if (nalu->nal_unit_type != 21)
                    svc_extension_flag = read_bit(bitstr);
                else
                    avc_3d_extension_flag = read_bit(bitstr);

                if (svc_extension_flag)
                {
                    // nal_unit_header_svc_extension() // specified in Annex G
                    nalu->idr_flag = read_bit(bitstr);
                    nalu->priority_id = read_bits(bitstr, 6);
                    nalu->no_inter_layer_pred_flag = read_bit(bitstr);
                    nalu->dependency_id = read_bits(bitstr, 3);
                    nalu->quality_id = read_bits(bitstr, 4);
                    nalu->temporal_id = read_bits(bitstr, 3);
                    nalu->use_ref_base_pic_flag = read_bit(bitstr);
                    nalu->discardable_flag = read_bit(bitstr);
                    nalu->output_flag = read_bit(bitstr);

                    TRACE_1(NALU, "  * nal_unit_header_svc_extension");
                    TRACE_1(NALU, "  - idr_flag      = %u", nalu->idr_flag);
                    TRACE_1(NALU, "  - priority_id   = %u", nalu->priority_id);
                    TRACE_1(NALU, "  - no_inter_layer_pred_flag = %u", nalu->no_inter_layer_pred_flag);
                    TRACE_1(NALU, "  - dependency_id = %u", nalu->dependency_id);
                    TRACE_1(NALU, "  - quality_id    = %u", nalu->quality_id);
                    TRACE_1(NALU, "  - temporal_id   = %u", nalu->temporal_id);
                    TRACE_1(NALU, "  - use_ref_base_pic_flag    = %u", nalu->use_ref_base_pic_flag);
                    TRACE_1(NALU, "  - discardable_flag = %u", nalu->discardable_flag);
                    TRACE_1(NALU, "  - output_flag      = %u", nalu->output_flag);

                    // Check reserved_three_2bits
                    if (read_bits(bitstr, 2) != 3)
                    {
                        TRACE_ERROR(NALU, "  * Invalid NAL Unit at byte offset %i (wrong reserved_three_2bits value)", nalu->nal_offset);
                    }
                }
                else if (avc_3d_extension_flag)
                {
                    // nal_unit_header_3davc_extension() // specified in Annex J
                    nalu->view_idx = read_bits(bitstr, 8);
                    nalu->depth_flag = read_bit(bitstr);
                    nalu->non_idr_flag = read_bit(bitstr);
                    nalu->temporal_id = read_bits(bitstr, 3);
                    nalu->anchor_pic_flag = read_bit(bitstr);
                    nalu->inter_view_flag = read_bit(bitstr);

                    TRACE_1(NALU, "  * nal_unit_header_3davc_extension");
                    TRACE_1(NALU, "  - view_idx     = %u", nalu->view_idx);
                    TRACE_1(NALU, "  - depth_flag   = %u", nalu->depth_flag);
                    TRACE_1(NALU, "  - non_idr_flag = %u", nalu->non_idr_flag);
                    TRACE_1(NALU, "  - temporal_id  = %u", nalu->temporal_id);
                    TRACE_1(NALU, "  - anchor_pic_flag = %u", nalu->anchor_pic_flag);
                    TRACE_1(NALU, "  - inter_view_flag = %u", nalu->inter_view_flag);
                }
                else
                {
                    // nal_unit_header_mvc_extension() // specified in Annex H
                    nalu->non_idr_flag = read_bit(bitstr);
                    nalu->priority_id = read_bits(bitstr, 6);
                    nalu->view_id = read_bits(bitstr, 10);
                    nalu->temporal_id = read_bits(bitstr, 3);
                    nalu->anchor_pic_flag = read_bit(bitstr);
                    nalu->inter_view_flag = read_bit(bitstr);

                    TRACE_1(NALU, "  * nal_unit_header_mvc_extension");
                    TRACE_1(NALU, "  - non_idr_flag = %u", nalu->non_idr_flag);
                    TRACE_1(NALU, "  - priority_id  = %u", nalu->priority_id);
                    TRACE_1(NALU, "  - view_id      = %u", nalu->view_id);
                    TRACE_1(NALU, "  - temporal_id  = %u", nalu->temporal_id);
                    TRACE_1(NALU, "  - anchor_pic_flag = %u", nalu->anchor_pic_flag);
                    TRACE_1(NALU, "  - inter_view_flag = %u", nalu->inter_view_flag);

                    // Check reserved_one_bit
                    if (read_bit(bitstr) != 1)
                    {
                        TRACE_ERROR(NALU, "  * Invalid NAL Unit at byte offset %i (wrong reserved_one_bit value)", nalu->nal_offset);
                    }
                }
            }
            else
            {
                retcode = SUCCESS;
                TRACE_1(NALU, "  * NAL Unit confirmed at byte offset %i", nalu->nal_offset);
            }
        }
        else
        {
            TRACE_ERROR(NALU, "  * There is no valid NAL Unit at byte offset %i (wrong forbidden_zero_bit value)", nalu->nal_offset);
        }
    }
    else
    {
        TRACE_ERROR(NALU, "  * Empty NAL Unit or bitstream!");
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
 * for more information about that issue.
 */
int h264_nalu_clean_sample(Bitstream_t *bitstr)
{
    TRACE_INFO(NALU, BLD_GREEN "h264_nalu_clean_sample()" CLR_RESET);

    unsigned int buffer_offset_saved = bitstr->buffer_offset;

    unsigned int consecutive_zeros = 0;
    unsigned int current_byte = 0;
    unsigned int current_byte_offset = std::ceil(bitstr->buffer_offset/8.0);

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

                // Move data
                memmove((bitstr->buffer + current_byte_offset), (bitstr->buffer + current_byte_offset + 1), (bitstr->buffer_size - current_byte_offset - 1));

                // Discard byte(s)
                bitstr->buffer_size--;
                bitstr->buffer_discarded_bytes++;

                TRACE_2(NALU, "emulation_prevention_three_byte removed at buffer offset: %i", current_byte_offset);
                TRACE_2(NALU, "emulation_prevention_three_byte removed at bitstream offset: %i", bitstream_get_absolute_byte_offset(bitstr));
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

const char *h264_nalu_get_string_type0(h264_nalu_t *nalu)
{
    return h264_nalu_get_string_type1(nalu->nal_unit_type);
}

const char *h264_nalu_get_string_type1(unsigned nal_unit_type)
{
    switch (nal_unit_type)
    {
        case NALU_TYPE_SLICE:
            return "SLICE (coded slice)";
        case NALU_TYPE_DPA:
            return "DPA (data partition, type A)";
        case NALU_TYPE_DPB:
            return "DPB (data partition, type B)";
        case NALU_TYPE_DPC:
            return "DPC (data partition, type C)";
        case NALU_TYPE_IDR:
            return "IDR (Instantaneous Decode Refresh frame)";
        case NALU_TYPE_SEI:
            return "SEI (Supplemental Enhancement Information)";
        case NALU_TYPE_SPS:
            return "SPS (Sequence Parameter Set)";
        case NALU_TYPE_SPS_EXT:
            return "SPS EXT (Sequence Parameter Set Extension)";
        case NALU_TYPE_SPS_SUBSET:
            return "SPS SUBSET (Subset Sequence Parameter Set)";
        case NALU_TYPE_PPS:
            return "PPS (Picture Parameter Set)";
        case NALU_TYPE_AUD:
            return "AUD (Access Unit Delimiter)";
        case NALU_TYPE_END_SEQUENCE:
            return "END_SEQUENCE (end of sequence)";
        case NALU_TYPE_END_STREAM:
            return "END_STREAM (end of stream)";
        case NALU_TYPE_FILL:
            return "FILL (filler data)";
        case NALU_TYPE_PREFIX_NAL:
            return "Prefix NAL unit";
        case NALU_TYPE_DPS:
            return "DPS (Depth Parameter Set)";
        case NALU_TYPE_SLICE_AUX:
            return "SLICE AUX (coded slice of an auxiliary coded picture)";
        case NALU_TYPE_SLICE_EXT:
            return "SLICE EXT (coded slice extension)";
        case NALU_TYPE_SLICE_EXT_3D:
            return "SLICE EXT 3D (coded slice extension for depth view / 3D-AVC texture view)";

        default:
            return "Unknown";
    }
}

/* ************************************************************************** */
