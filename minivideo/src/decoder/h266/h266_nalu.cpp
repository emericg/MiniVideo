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
 * \file      h266_nalu.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "h266_nalu.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Allocate and initialize a new NAL Unit.
 * \return A pointer to the newly allocated NAL Unit.
 */
h266_nalu_t *h266_nalu_init(void)
{
    TRACE_INFO(NALU, BLD_GREEN "h266_nalu_init()" CLR_RESET);

    // NAL Unit allocation
    h266_nalu_t *nalu = NULL;
    if ((nalu = (h266_nalu_t*)calloc(1, sizeof(h266_nalu_t))) == NULL)
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
void h266_nalu_reset(h266_nalu_t *nalu)
{
    TRACE_INFO(NALU, BLD_GREEN "h266_nalu_reset()" CLR_RESET);

    if (nalu != NULL)
    {
        nalu->nal_offset = 0;

        // Set default params
        nalu->nuh_layer_id = 0;
        nalu->nal_unit_type = 0;
        nalu->nuh_temporal_id_plus1 = 0;
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
 * - A nuh_reserved_zero_bit. Useful to check NAL Unit validity.
 * - A nuh_layer_id.
 * - A nal_unit_type, which indicate precisly the content of the NAL Unit.
 * - A nuh_temporal_id_plus1.
 */
int h266_nalu_parse_header(Bitstream_t *bitstr, h266_nalu_t *nalu)
{
    TRACE_INFO(NALU, "> " BLD_GREEN "h266_nalu_parse_header()" CLR_RESET);
    int retcode = FAILURE;

    if (bitstr && nalu)
    {
        // Save NAL Unit header offset
        nalu->nal_offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check forbidden_zero_bit & nuh_reserved_zero_bit
        if (read_bit(bitstr) == 0 && read_bit(bitstr) == 0)
        {
            nalu->nuh_layer_id = read_bits(bitstr, 6);
            nalu->nal_unit_type = read_bits(bitstr, 5);
            nalu->nuh_temporal_id_plus1 = read_bits(bitstr, 3);

            TRACE_1(NALU, "  - nuh_layer_id         = 0x%02X", nalu->nuh_layer_id);
            TRACE_1(NALU, "  - nal_unit_type        = 0x%02X", nalu->nal_unit_type);
            TRACE_1(NALU, "  - nuh_temporal_id_plus1= 0x%02X", nalu->nuh_temporal_id_plus1);
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
int h266_nalu_clean_sample(Bitstream_t *bitstr)
{
    TRACE_INFO(NALU, BLD_GREEN "h266_nalu_clean_sample()" CLR_RESET);

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

const char *h266_nalu_get_string_type0(h266_nalu_t *nalu)
{
    return h266_nalu_get_string_type1(nalu->nal_unit_type);
}

const char *h266_nalu_get_string_type1(unsigned nal_unit_type)
{
    switch (nal_unit_type)
    {
        case NALU_TYPE_TRAIL_NUT:
            return "TRAIL_NUT (Coded slice of a trailing picture or subpicture)";
        case NALU_TYPE_STSA_NUT:
            return "STSA_NUT (Coded slice of an STSA picture or subpicture)";
        case NALU_TYPE_RADL_NUT:
            return "RADL_NUT (Coded slice of a RADL picture or subpicture)";
        case NALU_TYPE_RASL_NUT:
            return "RASL_NUT (Coded slice of a RASL picture or subpicture)";

        case NALU_TYPE_IDR_W_RADL:
            return "IDR_W_RADL (Coded slice of an IDR picture or subpicture)";
        case NALU_TYPE_IDR_N_LP:
            return "IDR_N_LP (Coded slice of an IDR picture or subpicture)";
        case NALU_TYPE_CRA_NUT:
            return "CRA_NUT (Coded slice of a CRA picture or subpicture)";
        case NALU_TYPE_GDR_NUT:
            return "GDR_NUT (Coded slice of a GDR picture or subpicture)";

        case NALU_TYPE_OPI_NUT:
            return "OPI_NUT (Operating Point Information)";
        case NALU_TYPE_DCI_NUT:
            return "DCI_NUT (Decoding Capability Information)";
        case NALU_TYPE_VPS_NUT:
            return "VPS_NUT (Video Parameter Set)";
        case NALU_TYPE_SPS_NUT:
            return "SPS_NUT (Sequence Parameter Set)";
        case NALU_TYPE_PPS_NUT:
            return "PPS_NUT (Picture Parameter Set)";
        case NALU_TYPE_PREFIX_APS_NUT:
            return "PREFIX_APS_NUT (Adaptation Parameter Set)";
        case NALU_TYPE_SUFFIX_APS_NUT:
            return "SUFFIX_APS_NUT (Adaptation Parameter Set)";
        case NALU_TYPE_PH_NUT:
            return "PH_NUT (Picture Header)";
        case NALU_TYPE_AUD_NUT:
            return "AUD_NUT (Access Unit Delimiter)";
        case NALU_TYPE_EOS_NUT:
            return "EOS_NUT (End of Sequence)";
        case NALU_TYPE_EOB_NUT:
            return "EOB_NUT (End of Bitstream)";
        case NALU_TYPE_PREFIX_SEI_NUT:
            return "PREFIX_SEI_NUT (Supplemental Enhancement Information)";
        case NALU_TYPE_SUFFIX_SEI_NUT:
            return "SUFFIX_SEI_NUT (Supplemental Enhancement Information)";
        case NALU_TYPE_FD_NUT:
            return "FD_NUT (Filler Data)";

        default:
            return "Unknown";
    }
}

/* ************************************************************************** */
