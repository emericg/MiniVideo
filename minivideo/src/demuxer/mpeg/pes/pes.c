/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \file      pes.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// minivideo headers
#include "../../../minitraces.h"
#include "../../../typedef.h"

#include "pes.h"

/* ************************************************************************** */

int parse_pes(Bitstream_t *bitstr, PesPacket_t *packet)
{
    TRACE_2(PARSER, "> parse_pes()\n");
    int retcode = SUCCESS;

    packet->PES_packet_length = read_bits(bitstr, 16);

    // Regular PES parameters
    if (packet->stream_id != SID_PADDING &&
        packet->stream_id != SID_PRIVATE_STREAM_2 &&
        packet->stream_id != SID_PROGRAM_STREAM_MAP &&
        packet->stream_id != SID_PROGRAM_STREAM_DIRECTORY)
    {
        if (read_bits(bitstr, 2) != 2)
        {
            TRACE_ERROR(PARSER, "wrong 'marker_bit'\n");
            return FAILURE;
        }
        packet->PES_scrambling_control = read_bits(bitstr, 2);
        packet->PES_priority = read_bit(bitstr);
        packet->data_alignment_indicator = read_bit(bitstr);
        packet->copyright = read_bit(bitstr);
        packet->original_or_copy = read_bit(bitstr);
        packet->PTS_DTS_flag = read_bits(bitstr, 2);
        packet->ESCR_flag = read_bit(bitstr);
        packet->ES_rate_flag = read_bit(bitstr);
        packet->DSM_trick_mode_flag = read_bit(bitstr);
        packet->additional_copy_info_flag = read_bit(bitstr);
        packet->PES_CRC_flag = read_bit(bitstr);
        packet->PES_extension_flag = read_bit(bitstr);
        packet->PES_header_data_length = read_bits(bitstr, 8);

        if (packet->PTS_DTS_flag == 2)
        {
            if (read_bits(bitstr, 4) != 2)
            {
                TRACE_ERROR(PARSER, "wrong 'marker_bit'\n");
                return FAILURE;
            }
            packet->PTS = read_bits(bitstr, 3) << 30;
            MARKER_BIT
            packet->PTS += read_bits(bitstr, 15) << 15;
            MARKER_BIT
            packet->PTS += read_bits(bitstr, 15);
            MARKER_BIT
        }
        else if (packet->PTS_DTS_flag == 3)
        {
            if (read_bits(bitstr, 4) != 3)
            {
                TRACE_ERROR(PARSER, "wrong 'marker_bit'\n");
                return FAILURE;
            }
            packet->PTS = read_bits(bitstr, 3) << 30;
            MARKER_BIT
            packet->PTS += read_bits(bitstr, 15) << 15;
            MARKER_BIT
            packet->PTS += read_bits(bitstr, 15);
            MARKER_BIT
            if (read_bits(bitstr, 4) != 1)
            {
                TRACE_ERROR(PARSER, "wrong 'marker_bit'\n");
                return FAILURE;
            }
            packet->DTS = read_bits(bitstr, 3) << 30;
            MARKER_BIT
            packet->DTS += read_bits(bitstr, 15) << 15;
            MARKER_BIT
            packet->DTS += read_bits(bitstr, 15);
            MARKER_BIT
        }

        if (packet->ESCR_flag == 1)
        {
            packet->ESCR_base = read_bits(bitstr, 3) << 30;
            MARKER_BIT
            packet->ESCR_base += read_bits(bitstr, 15) << 15;
            MARKER_BIT
            packet->ESCR_base += read_bits(bitstr, 15);
            MARKER_BIT
            packet->ESCR_extension = read_bits(bitstr, 9);
            MARKER_BIT
        }

        if (packet->ES_rate_flag == 1)
        {
            MARKER_BIT
            packet->ES_rate = read_bits(bitstr, 22);
            MARKER_BIT
        }

        if (packet->DSM_trick_mode_flag == 1)
        {
            packet->trick_mode_control = read_bits(bitstr, 3);

            if (packet->trick_mode_control == TM_FAST_FORWARD)
            {
                packet->field_id = read_bits(bitstr, 2);
                packet->intra_slice_refresh = read_bit(bitstr);
                packet->frequency_truncation = read_bits(bitstr, 2);
            }
            else if (packet->trick_mode_control == TM_SLOW_MOTION)
            {
                packet->rep_cntrl = read_bits(bitstr, 5);
            }
            else if (packet->trick_mode_control == TM_FREEZE_FRAME)
            {
                packet->field_id = read_bits(bitstr, 2);
                int reserved = read_bits(bitstr, 3);
            }
            else if (packet->trick_mode_control == TM_FAST_REVERSE)
            {
                packet->field_id = read_bits(bitstr, 2);
                packet->intra_slice_refresh = read_bit(bitstr);
                packet->frequency_truncation = read_bits(bitstr, 2);
            }
            else if (packet->trick_mode_control == TM_SLOW_REVERSE)
            {
                packet->rep_cntrl = read_bits(bitstr, 5);
            }
            else
            {
                int reserved = read_bits(bitstr, 5);
            }
        }

        if (packet->additional_copy_info_flag == 1)
        {
            MARKER_BIT
            packet->additional_copy_info = read_bits(bitstr, 7);
        }

        if (packet->PES_CRC_flag == 1)
        {
            packet->previous_PES_packet_CRC = read_bits(bitstr, 16);
        }

        if (packet->PES_extension_flag == 1)
        {
            packet->PES_private_data_flag = read_bit(bitstr);
            packet->pack_header_field_flag = read_bit(bitstr);
            packet->program_packet_sequence_counter_flag = read_bit(bitstr);
            packet->PSTD_buffer_flag = read_bit(bitstr);
            int reserved = read_bits(bitstr, 3);
            packet->PES_extension_flag_2 = read_bit(bitstr);

            if (packet->PES_private_data_flag == 1)
            {
                int i = 0;
                for (i = 0; i < 16; i++)
                {
                    packet->PES_private_data[i] = read_bits(bitstr, 8);
                }
            }

            if (packet->pack_header_field_flag == 1)
            {
                packet->pack_field_length = read_bits(bitstr, 8);
                // TODO
                //parse_pack_header(bitstr, wtf);
            }

            if (packet->program_packet_sequence_counter_flag == 1)
            {
                MARKER_BIT
                packet->program_packet_sequence_counter = read_bits(bitstr, 7);
                MARKER_BIT
                packet->MPEG1_MPEG2_identifier = read_bit(bitstr);
                packet->original_stuff_length = read_bits(bitstr, 6);
            }

            if (packet->PSTD_buffer_flag == 1)
            {
                if (read_bits(bitstr, 2) != 1)
                {
                    TRACE_ERROR(PARSER, "wrong 'marker_bit'\n");
                    return FAILURE;
                }
                packet->PSTD_buffer_scale = read_bit(bitstr);
                packet->PSTD_buffer_size = read_bits(bitstr, 13);
            }

            if (packet->PES_extension_flag_2 == 1)
            {
                MARKER_BIT
                packet->PES_extension_field_length = read_bits(bitstr, 7);
                int i = 0;
                for (i = 0; i < packet->PES_extension_field_length; i++)
                {
                    int reserved = read_bits(bitstr, 8);
                }
            }
        }

        // Skip packet data
        retcode = skip_pes_data(bitstr, packet);
/*
        // TODO
        int i = 0, N1 = 0, N2 = 0;

        for (i = 0; i < N1; i++)
        {
            if (read_bits(bitstr, 8) != 0xFF)
            {
                TRACE_ERROR(PARSER, GREEN "wrong 'stuffing_byte'\n" RESET);
                return FAILURE;
            }
        }

        for (i = 0; i < N2; i++)
        {
            uint8_t PES_packet_data_byte = read_bits(bitstr, 8);
        }
*/
    }

    return retcode;
}

/* ************************************************************************** */

void print_pes(PesPacket_t *packet)
{
    TRACE_INFO(PARSER, GREEN "print_pes()\n" RESET);

    TRACE_INFO(PARSER, " packet_start_offset\t= 0x%08X\n", packet->packet_start_offset);

    TRACE_INFO(PARSER, " packet_start_code\t\t= 0x%06X\n", packet->packet_start_code);
    TRACE_INFO(PARSER, " stream_id\t\t\t= 0x%02X\n", packet->packet_start_code);
    TRACE_INFO(PARSER, " PES_packet_length\t\t= %i\n", packet->PES_packet_length);

    // Regular PES parameters
    if (packet->stream_id != SID_PADDING &&
        packet->stream_id != SID_PRIVATE_STREAM_2 &&
        packet->stream_id != SID_PROGRAM_STREAM_MAP &&
        packet->stream_id != SID_PROGRAM_STREAM_DIRECTORY)
    {
        TRACE_1(PARSER, " PES_scrambling_control\t= %i\n", packet->PES_scrambling_control);
        TRACE_1(PARSER, " PES_priority\t\t= %i\n", packet->PES_priority);
        TRACE_1(PARSER, " data_alignment_indicator\t= %i\n", packet->data_alignment_indicator);
        TRACE_1(PARSER, " copyright\t\t\t= %i\n", packet->copyright);
        TRACE_1(PARSER, " original_or_copy\t\t= %i\n", packet->original_or_copy);
        TRACE_1(PARSER, " PTS_DTS_flag\t\t= %i\n", packet->PTS_DTS_flag);
        TRACE_1(PARSER, " ESCR_flag\t\t\t= %i\n", packet->ESCR_flag);
        TRACE_1(PARSER, " ES_rate_flag\t\t= %i\n", packet->ES_rate_flag);
        TRACE_1(PARSER, " DSM_trick_mode_flag\t= %i\n", packet->DSM_trick_mode_flag);
        TRACE_1(PARSER, " additional_copy_info_flag\t= %i\n", packet->additional_copy_info_flag);
        TRACE_1(PARSER, " PES_CRC_flag\t\t= %i\n", packet->PES_CRC_flag);
        TRACE_1(PARSER, " PES_extension_flag\t\t= %i\n", packet->PES_extension_flag);
        TRACE_1(PARSER, " PES_header_data_length\t= %i\n", packet->PES_header_data_length);

        if (packet->PTS_DTS_flag == 2)
        {
            TRACE_1(PARSER, " PTS\t\t\t= %i\n", packet->PTS);
        }
        else if (packet->PTS_DTS_flag == 3)
        {
            TRACE_1(PARSER, " PTS\t\t\t= %i\n", packet->PTS);
            TRACE_1(PARSER, " DTS\t\t\t= %i\n", packet->DTS);
        }

        if (packet->ESCR_flag == 1)
        {
            TRACE_1(PARSER, " ESCR_base\t\t\t= %i\n", packet->ESCR_base);
            TRACE_1(PARSER, " ESCR_extension\\tt= %i\n", packet->ESCR_extension);
        }

        if (packet->ES_rate_flag == 1)
        {
            TRACE_1(PARSER, " ES_rate\t= %i\n", packet->ES_rate);
        }

        if (packet->DSM_trick_mode_flag == 1)
        {
            TRACE_1(PARSER, " trick_mode_control\t= %i\n", packet->trick_mode_control);

            if (packet->trick_mode_control == TM_FAST_FORWARD)
            {
                TRACE_1(PARSER, " field_id\t= %i\n", packet->field_id);
                TRACE_1(PARSER, " intra_slice_refresh\t= %i\n", packet->intra_slice_refresh);
                TRACE_1(PARSER, " frequency_truncation\t= %i\n", packet->frequency_truncation);
            }
            else if (packet->trick_mode_control == TM_SLOW_MOTION)
            {
                TRACE_1(PARSER, " rep_cntrl\t\t= %i\n", packet->rep_cntrl);
            }
            else if (packet->trick_mode_control == TM_FREEZE_FRAME)
            {
                TRACE_1(PARSER, " field_id\t\t= %i\n", packet->field_id);
            }
            else if (packet->trick_mode_control == TM_FAST_REVERSE)
            {
                TRACE_1(PARSER, " field_id\t\t= %i\n", packet->field_id);
                TRACE_1(PARSER, " intra_slice_refresh\t= %i\n", packet->intra_slice_refresh);
                TRACE_1(PARSER, " frequency_truncation\t= %i\n", packet->frequency_truncation);
            }
            else if (packet->trick_mode_control == TM_SLOW_REVERSE)
            {
                TRACE_1(PARSER, " rep_cntrl\t\t= %i\n", packet->rep_cntrl);
            }
        }

        if (packet->additional_copy_info_flag == 1)
        {
            TRACE_1(PARSER, " additional_copy_info\t= %i\n", packet->additional_copy_info);
        }

        if (packet->PES_CRC_flag == 1)
        {
            TRACE_1(PARSER, " previous_PES_packet_CRC\t= %i\n", packet->previous_PES_packet_CRC);
        }

        if (packet->PES_extension_flag == 1)
        {
            TRACE_1(PARSER, " PES_private_data_flag\t= %i\n", packet->PES_private_data_flag);
            TRACE_1(PARSER, " pack_header_field_flag\t= %i\n", packet->pack_header_field_flag);
            TRACE_1(PARSER, " program_packet_sequence_counter_flag = %i\n", packet->program_packet_sequence_counter_flag);
            TRACE_1(PARSER, " PSTD_buffer_flag\t\t= %i\n", packet->PSTD_buffer_flag);
            TRACE_1(PARSER, " PES_extension_flag_2\t= %i\n", packet->PES_extension_flag_2);

            if (packet->PES_private_data_flag == 1)
            {
                int i = 0;
                for (i = 0; i < 16; i++)
                {
                    TRACE_1(PARSER, " PES_private_data[%i]\t= %i\n", i, packet->PES_private_data[i]);
                }
            }

            if (packet->pack_header_field_flag == 1)
            {
                TRACE_1(PARSER, " pack_field_length\t= %i\n", packet->pack_field_length);
                // TODO
                //parse_pack_header(bitstr, wtf);
            }

            if (packet->program_packet_sequence_counter_flag == 1)
            {
                TRACE_1(PARSER, " program_packet_sequence_counter\t= %i\n", packet->program_packet_sequence_counter);
                TRACE_1(PARSER, " MPEG1_MPEG2_identifier\t= %i\n", packet->MPEG1_MPEG2_identifier);
                TRACE_1(PARSER, " original_stuff_length\t= %i\n", packet->original_stuff_length);
            }

            if (packet->PSTD_buffer_flag == 1)
            {
                TRACE_1(PARSER, " PSTD_buffer_scale\t\t= %i\n", packet->PSTD_buffer_scale);
                TRACE_1(PARSER, " PSTD_buffer_size\t\t= %i\n", packet->PSTD_buffer_size);
            }

            if (packet->PES_extension_flag_2 == 1)
            {
                TRACE_1(PARSER, " PES_extension_field_length\t= %i\n", packet->PES_extension_field_length);
                int i = 0;
                for (i = 0; i < packet->PES_extension_field_length; i++)
                {
                    TRACE_1(PARSER, " reserved\t\t= xx\n");
                }
            }
        }
    }
}

/* ************************************************************************** */

int parse_pes_padding(Bitstream_t *bitstr, PesPacket_t *packet)
{
    TRACE_INFO(PARSER, GREEN "  parse_pes_padding()\n" RESET);
    int retcode = SUCCESS;
    packet->PES_packet_length = read_bits(bitstr, 16);

    if (packet->PES_packet_length != 0)
    {
        skip_bits(bitstr, packet->PES_packet_length * 8);
        TRACE_INFO(PARSER, "  > skip_padding_packet() >> %i bytes\n", packet->PES_packet_length);
        retcode = SUCCESS;
    }
    else
    {
        while (read_bits(bitstr, 8) == 0xFF)
        {
            // Manual skip the padding bytes
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Skip a PES packet content.
 */
int skip_pes(Bitstream_t *bitstr, PesPacket_t *packet)
{
    int retcode = FAILURE;
    packet->PES_packet_length = read_bits(bitstr, 16);

    if (packet->PES_packet_length != 0)
    {
        skip_bits(bitstr, (packet->PES_packet_length) * 8);
        TRACE_INFO(PARSER, " > skip_pes() >> %i bytes\n", packet->PES_packet_length - 6);
        retcode = SUCCESS;
    }
    else
    {
        TRACE_WARNING(PARSER, " > skip_pes() >> do it yourself\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Skip PES packet data.
 */
int skip_pes_data(Bitstream_t *bitstr, PesPacket_t *packet)
{
    int retcode = FAILURE;

    if (packet->PES_packet_length != 0)
    {
        int curr_offset = bitstream_get_absolute_byte_offset(bitstr);
        int skipsize = packet->PES_packet_length - (curr_offset - packet->packet_start_offset - 6);
        skip_bits(bitstr, skipsize * 8);
/*
        TRACE_1(PARSER, "  star_offset :: %i\n", packet->packet_start_offset);
        TRACE_1(PARSER, "  curr_offset :: %i\n", curr_offset);
        TRACE_1(PARSER, "  skip_data() >> %i bytes\n", packet->PES_packet_length);
        TRACE_1(PARSER, "  skip_data() >> %i bytes\n", packet->PES_header_data_length);
*/
        TRACE_1(PARSER, " > skip_pes_data() >> %i bytes\n", skipsize);
        retcode = SUCCESS;
    }
    else
    {
        TRACE_WARNING(PARSER, " > skip_pes_data() >> do it yourself!\n");
    }

    return retcode;
}

/* ************************************************************************** */
