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
 * \file      pes.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "pes.h"
#include "../../../bitstream_utils.h"
#include "../../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - PES edition.
 * \param bitstr: Our bitstream reader.
 * \param header: A pointer to a PES header structure.
 * \return SUCCESS or FAILURE if the jump could not be done.
 *
 * 'Jumpy' is in charge of checking your position into the stream after your
 * parser finish parsing a box / list / chunk / element, never leaving you
 * stranded  in the middle of nowhere with no easy way to get back on track.
 * It will check available informations to known if the current element has been
 * fully parsed, and if not perform a jump (or even a rewind) to the next known
 * element.
 */
int jumpy_pes(Bitstream_t *bitstr, PesHeader_t *header)
{
    int retcode = SUCCESS;

    // Done as a precaution, because the parsing of some boxes (like ESDS...)
    // can leave us in the middle of a byte and that will never be caught by
    // offset checks (cause they works on the assumption that we are byte aligned)
    bitstream_force_alignment(bitstr);

    // Check if we need a jump
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);
    if (current_pos != header->offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up...
        if (header->offset_end >= file_size)
        {
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        // Now, do we need to go forward or backward to reach our goal?
        // Then, can we move in our current buffer or do we need to reload a new one?
        if (current_pos < header->offset_end)
        {
            int64_t jump = header->offset_end - current_pos;

            if (jump < (UINT_MAX/8))
                retcode = skip_bits(bitstr, (unsigned int)(jump*8));
            else
                retcode = bitstream_goto_offset(bitstr, header->offset_end);
        }
        else
        {
            int64_t rewind = current_pos - header->offset_end;

            if (rewind > 0)
            {
                if (rewind > (UINT_MAX/8))
                    retcode = rewind_bits(bitstr, (unsigned int)(rewind*8));
                else
                    retcode = bitstream_goto_offset(bitstr, header->offset_end);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse PES packet header, common to all PES packet types.
 * \param *bitstr: The bitstream to use.
 * \param *header: A pointer to a PES header structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.4.3.6 PES packet.
 * Table 2-17 – PES packet.
 */
int parse_pes_header(Bitstream_t *bitstr, PesHeader_t *header)
{
    TRACE_2(MPS, "> parse_pes_header()");

    header->offset_start    = bitstream_get_absolute_byte_offset(bitstr);

    header->start_code      = read_bits(bitstr, 32);
    header->stream_id       = header->start_code & 0x000000FF;
    header->payload_length  = read_bits(bitstr, 16);
    header->packet_length   = header->payload_length + 6;

    header->offset_end      = header->offset_start + header->packet_length;

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief Parse PES packet.
 * \param *bitstr: The bitstream to use.
 * \param *header: A pointer to a PES header structure.
 * \param *packet: A pointer to a PES packet structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.4.3.6 PES packet.
 * Table 2-17 – PES packet.
 *
 * Parse both MPEG-1 (ISO/IEC 11172-1) and MPEG-2 (ISO/IEC 13818-1) PES packets.
 */
int parse_pes(Bitstream_t *bitstr, PesHeader_t *header, PesPacket_t *packet)
{
    TRACE_2(MPS, "> parse_pes()");
    int retcode = SUCCESS;

    TRACE_INFO(MPS, "parse_pes() 0x%X @ %lli",
               header->start_code, bitstream_get_absolute_byte_offset(bitstr));

    // "regular" PES packet?
    if ((header->stream_id != SID_PROGRAM_STREAM_MAP) &&
        (header->stream_id != SID_PADDING) &&
        (header->stream_id != SID_PRIVATE_STREAM_2) &&
        (header->stream_id != SID_ECM_STREAM) &&
        (header->stream_id != SID_EMM_STREAM) &&
        (header->stream_id != SID_PROGRAM_STREAM_DIRECTORY) &&
        (header->stream_id != SID_DSMCC_STREAM) &&
        (header->stream_id != SID_2221E))
    {
        unsigned checkversion = next_bits(bitstr, 2);

        // Parse MPEG-2 PES header
        if ((checkversion & 0x02) == 0x02)
        {
            packet->mpeg_version = 2;

            if (read_bits(bitstr, 2) != 2)
            {
                TRACE_ERROR(MPS, "wrong 'marker_bits'");
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
                    TRACE_ERROR(MPS, "wrong 'marker_bit'");
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
                    TRACE_ERROR(MPS, "wrong 'marker_bit'");
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
                    TRACE_ERROR(MPS, "wrong 'marker_bit'");
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
                    /*int reserved =*/ read_bits(bitstr, 3);
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
                    /*int reserved =*/ read_bits(bitstr, 5);
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
                /*int reserved =*/ read_bits(bitstr, 3);
                packet->PES_extension_flag_2 = read_bit(bitstr);

                if (packet->PES_private_data_flag == 1)
                {
                    for (int i = 0; i < 16; i++)
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
                        TRACE_ERROR(MPS, "wrong 'marker_bit'");
                        return FAILURE;
                    }
                    packet->PSTD_buffer_scale = read_bit(bitstr);
                    packet->PSTD_buffer_size = read_bits(bitstr, 13);
                }

                if (packet->PES_extension_flag_2 == 1)
                {
                    MARKER_BIT
                    packet->PES_extension_field_length = read_bits(bitstr, 7);
                    for (int i = 0; i < packet->PES_extension_field_length; i++)
                    {
                        /*int reserved =*/ read_bits(bitstr, 8);
                    }
                }
            }
        }
        else // Parse MPEG-1 PES header
        {
            packet->mpeg_version = 1;
            packet->PES_header_data_length = 0; // We will emulate this field

            // MPEG-1: up to 16 stuffing bytes here (optional)
            while (next_bits(bitstr, 8) == 0xFF)
            {
                skip_bits(bitstr, 8);
                packet->PES_header_data_length++;
            }

            // Optional
            if (next_bits(bitstr, 2) == 1)
            {
                skip_bits(bitstr, 2);
                packet->PES_header_data_length += 2;

                packet->PSTD_buffer_scale = read_bit(bitstr);
                packet->PSTD_buffer_size = read_bits(bitstr, 13);
            }

            if (next_bits(bitstr, 8) == 0x0F)
            {
                skip_bits(bitstr, 8);
                packet->PES_header_data_length++;
            }
            else
            {
                if (next_bits(bitstr, 2) != 0)
                {
                    TRACE_ERROR(MPS, "wrong 'marker_bit' PESv1");
                    return FAILURE;
                }

                packet->PTS_DTS_flag = read_bits(bitstr, 2);

                if (packet->PTS_DTS_flag == 2)
                {
                    packet->PES_header_data_length += 5;

                    packet->PTS = read_bits(bitstr, 3) << 30;
                    MARKER_BIT
                    packet->PTS += read_bits(bitstr, 15) << 15;
                    MARKER_BIT
                    packet->PTS += read_bits(bitstr, 15);
                    MARKER_BIT
                }
                else if (packet->PTS_DTS_flag == 3)
                {
                    packet->PES_header_data_length += 10;

                    packet->PTS = read_bits(bitstr, 3) << 30;
                    MARKER_BIT
                    packet->PTS += read_bits(bitstr, 15) << 15;
                    MARKER_BIT
                    packet->PTS += read_bits(bitstr, 15);
                    MARKER_BIT

                    if (read_bits(bitstr, 4) != 1)
                    {
                        TRACE_ERROR(MPS, "wrong 'marker_bit' (PTS_DTS_flag==3)");
                        return FAILURE;
                    }

                    packet->DTS = read_bits(bitstr, 3) << 30;
                    MARKER_BIT
                    packet->DTS += read_bits(bitstr, 15) << 15;
                    MARKER_BIT
                    packet->DTS += read_bits(bitstr, 15);
                    MARKER_BIT
                }
                else
                {
                    packet->PES_header_data_length++;

                    if (read_bits(bitstr, 4) == 0x0F)
                    {
                        TRACE_ERROR(MPS, "wrong 'marker_bit' (PTS_DTS_flag==0)");
                        return FAILURE;
                    }
                }
            }
        }

        // Skip packet data
        //retcode = skip_pes_data(bitstr, packet);
/*
        // TODO
        int i = 0, N1 = 0, N2 = 0;

        for (i = 0; i < N1; i++)
        {
            if (read_bits(bitstr, 8) != 0xFF)
            {
                TRACE_ERROR(MPS, BLD_GREEN "wrong 'stuffing_byte'" CLR_RESET);
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

void print_pes(PesHeader_t *header, PesPacket_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "print_pes()" CLR_RESET);

    // Header
    TRACE_INFO(MPS, " packet_start_offset   = %lli", header->offset_start);
    TRACE_INFO(MPS, " packet_end_offset     = %lli", header->offset_end);

    TRACE_INFO(MPS, " packet_start_code     = 0x%06X", header->start_code);
    TRACE_INFO(MPS, " stream_id             = 0x%02X", header->start_code);
    TRACE_INFO(MPS, " PES_packet_length     = %i", header->payload_length);

    // "regular" PES packet?
    if ((header->stream_id != SID_PROGRAM_STREAM_MAP) &&
        (header->stream_id != SID_PADDING) &&
        (header->stream_id != SID_PRIVATE_STREAM_2) &&
        (header->stream_id != SID_ECM_STREAM) &&
        (header->stream_id != SID_EMM_STREAM) &&
        (header->stream_id != SID_PROGRAM_STREAM_DIRECTORY) &&
        (header->stream_id != SID_DSMCC_STREAM) &&
        (header->stream_id != SID_2221E))
    {
        TRACE_1(MPS, " PES_scrambling_control   = %i", packet->PES_scrambling_control);
        TRACE_1(MPS, " PES_priority             = %i", packet->PES_priority);
        TRACE_1(MPS, " data_alignment_indicator = %i", packet->data_alignment_indicator);
        TRACE_1(MPS, " copyright                = %i", packet->copyright);
        TRACE_1(MPS, " original_or_copy         = %i", packet->original_or_copy);
        TRACE_1(MPS, " PTS_DTS_flag             = %i", packet->PTS_DTS_flag);
        TRACE_1(MPS, " ESCR_flag                = %i", packet->ESCR_flag);
        TRACE_1(MPS, " ES_rate_flag             = %i", packet->ES_rate_flag);
        TRACE_1(MPS, " DSM_trick_mode_flag      = %i", packet->DSM_trick_mode_flag);
        TRACE_1(MPS, " additional_copy_info_flag= %i", packet->additional_copy_info_flag);
        TRACE_1(MPS, " PES_CRC_flag             = %i", packet->PES_CRC_flag);
        TRACE_1(MPS, " PES_extension_flag       = %i", packet->PES_extension_flag);
        TRACE_1(MPS, " PES_header_data_length   = %i", packet->PES_header_data_length);

        if (packet->PTS_DTS_flag == 2)
        {
            TRACE_1(MPS, " PTS      = %i", packet->PTS);
        }
        else if (packet->PTS_DTS_flag == 3)
        {
            TRACE_1(MPS, " PTS      = %i", packet->PTS);
            TRACE_1(MPS, " DTS      = %i", packet->DTS);
        }

        if (packet->ESCR_flag == 1)
        {
            TRACE_1(MPS, " ESCR_base        = %i", packet->ESCR_base);
            TRACE_1(MPS, " ESCR_extension   = %i", packet->ESCR_extension);
        }

        if (packet->ES_rate_flag == 1)
        {
            TRACE_1(MPS, " ES_rate  = %i", packet->ES_rate);
        }

        if (packet->DSM_trick_mode_flag == 1)
        {
            TRACE_1(MPS, " trick_mode_control= %i", packet->trick_mode_control);

            if (packet->trick_mode_control == TM_FAST_FORWARD)
            {
                TRACE_1(MPS, " field_id             = %i", packet->field_id);
                TRACE_1(MPS, " intra_slice_refresh  = %i", packet->intra_slice_refresh);
                TRACE_1(MPS, " frequency_truncation = %i", packet->frequency_truncation);
            }
            else if (packet->trick_mode_control == TM_SLOW_MOTION)
            {
                TRACE_1(MPS, " rep_cntrl= %i", packet->rep_cntrl);
            }
            else if (packet->trick_mode_control == TM_FREEZE_FRAME)
            {
                TRACE_1(MPS, " field_id= %i", packet->field_id);
            }
            else if (packet->trick_mode_control == TM_FAST_REVERSE)
            {
                TRACE_1(MPS, " field_id             = %i", packet->field_id);
                TRACE_1(MPS, " intra_slice_refresh  = %i", packet->intra_slice_refresh);
                TRACE_1(MPS, " frequency_truncation = %i", packet->frequency_truncation);
            }
            else if (packet->trick_mode_control == TM_SLOW_REVERSE)
            {
                TRACE_1(MPS, " rep_cntrl= %i", packet->rep_cntrl);
            }
        }

        if (packet->additional_copy_info_flag == 1)
        {
            TRACE_1(MPS, " additional_copy_info= %i", packet->additional_copy_info);
        }

        if (packet->PES_CRC_flag == 1)
        {
            TRACE_1(MPS, " previous_PES_packet_CRC= %i", packet->previous_PES_packet_CRC);
        }

        if (packet->PES_extension_flag == 1)
        {
            TRACE_1(MPS, " PES_private_data_flag    = %i", packet->PES_private_data_flag);
            TRACE_1(MPS, " pack_header_field_flag   = %i", packet->pack_header_field_flag);
            TRACE_1(MPS, " program_packet_sequence_counter_flag= %i", packet->program_packet_sequence_counter_flag);
            TRACE_1(MPS, " PSTD_buffer_flag         = %i", packet->PSTD_buffer_flag);
            TRACE_1(MPS, " PES_extension_flag_2     = %i", packet->PES_extension_flag_2);

            if (packet->PES_private_data_flag == 1)
            {
                int i = 0;
                for (i = 0; i < 16; i++)
                {
                    TRACE_1(MPS, " PES_private_data[%i]= %i", i, packet->PES_private_data[i]);
                }
            }

            if (packet->pack_header_field_flag == 1)
            {
                TRACE_1(MPS, " pack_field_length= %i", packet->pack_field_length);
                // TODO
                //parse_pack_header(bitstr, wtf);
            }

            if (packet->program_packet_sequence_counter_flag == 1)
            {
                TRACE_1(MPS, " program_packet_sequence_counter  = %i", packet->program_packet_sequence_counter);
                TRACE_1(MPS, " MPEG1_MPEG2_identifier           = %i", packet->MPEG1_MPEG2_identifier);
                TRACE_1(MPS, " original_stuff_length            = %i", packet->original_stuff_length);
            }

            if (packet->PSTD_buffer_flag == 1)
            {
                TRACE_1(MPS, " PSTD_buffer_scale= %i", packet->PSTD_buffer_scale);
                TRACE_1(MPS, " PSTD_buffer_size = %i", packet->PSTD_buffer_size);
            }

            if (packet->PES_extension_flag_2 == 1)
            {
                TRACE_1(MPS, " PES_extension_field_length= %i", packet->PES_extension_field_length);
                for (int i = 0; i < packet->PES_extension_field_length; i++)
                {
                    TRACE_1(MPS, " reserved= xx");
                }
            }
        }
    }
}

/* ************************************************************************** */

/*!
 * \brief parse_pes_padding
 * \param bitstr: Our bitstream reader.
 * \param header
 * \param packet
 * \return 1 if succeed, 0 otherwise.
 */
int parse_pes_padding(Bitstream_t *bitstr, PesHeader_t *header, PesPacket_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "  parse_pes_padding()" CLR_RESET);
    int retcode = SUCCESS;

    if (header->payload_length != 0)
    {
        skip_bits(bitstr, header->payload_length * 8);
        TRACE_INFO(MPS, "  > skip_padding_packet() >> %i bytes", header->payload_length);
        retcode = SUCCESS;
    }
    else
    {
        while (read_bits(bitstr, 8) == 0xFF)
        {
            // Manually skipping the padding bytes
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the 'body' of a PES packet containing audio data.
 * \param bitstr: Our bitstream reader.
 * \param header
 * \param packet
 * \param map
 * \return
 */
int parse_pes_a(Bitstream_t *bitstr, PesHeader_t *header, PesPacket_t *packet,
                MediaStream_t *map)
{
    TRACE_2(MPS, "> parse_pes_a()");
    int retcode = SUCCESS;

    if (header->stream_id == SID_PRIVATE_STREAM_1)
    {
        // Look for a known "non-MPEG" start code
        uint32_t startcode = read_bits(bitstr, 32);

        if ((startcode & 0xFF000000) == 0x80000000)
        {
            map->stream_codec = CODEC_AC3;

            //pc_uint16 crc1 = (read & 0x0000FFFF);
            uint8_t read = read_bits(bitstr, 8);
            uint8_t fscod = (read & 0xC0) >> 6;
            uint8_t frmsizcod = (read & 0x3F);

            switch (fscod)
            {
            case 0:
                map->sampling_rate = 48000.0;
                break;
            case 1:
                map->sampling_rate = 44100.0;
                break;
            case 2:
                map->sampling_rate = 32000.0;
                break;
            default:
                TRACE_WARNING(MPS, "Unsupported AC3 fscod %u", fscod);
                retcode = FAILURE;
                break;
            }

            switch (frmsizcod)
            {
            case 0:
            case 1:
                map->bitrate_avg = 32;
                break;
            case 2:
            case 3:
                map->bitrate_avg = 40;
                break;
            case 4:
            case 5:
                map->bitrate_avg = 48;
                break;
            case 6:
            case 7:
                map->bitrate_avg = 56;
                break;
            case 8:
            case 9:
                map->bitrate_avg = 64;
                break;
            case 10:
            case 11:
                map->bitrate_avg = 80;
                break;
            case 12:
            case 13:
                map->bitrate_avg = 96;
                break;
            case 14:
            case 15:
                map->bitrate_avg = 112;
                break;
            case 16:
            case 17:
                map->bitrate_avg = 128;
                break;
            case 18:
            case 19:
                map->bitrate_avg = 160;
                break;
            case 20:
            case 21:
                map->bitrate_avg = 192;
                break;
            case 22:
            case 23:
                map->bitrate_avg = 224;
                break;
            case 24:
            case 25:
                map->bitrate_avg = 256;
                break;
            case 26:
            case 27:
                map->bitrate_avg = 320;
                break;
            case 28:
            case 29:
                map->bitrate_avg = 384;
                break;
            case 30:
            case 31:
                map->bitrate_avg = 448;
                break;
            case 32:
            case 33:
                map->bitrate_avg = 512;
                break;
            case 34:
            case 35:
                map->bitrate_avg = 576;
                break;
            case 36:
            case 37:
                map->bitrate_avg = 640;
                break;
            default:
                TRACE_WARNING(MPS, "Unsupported AC3 frmsizcod %u", frmsizcod);
                retcode = FAILURE;
                break;
            }
        }
        else if ((startcode == 0x7FFE8001) ||
                 (startcode == 0x64582025))
        {
            map->stream_codec = CODEC_DTS;

            // 28 first bits are ignored
            uint32_t read = read_bits(bitstr, 16);
            read = read_bits(bitstr, 32);

            //uint32_t FSIZE = 0; // Might be necessary to skip the frame and go read the extension
            //uint32_t AMODE = (read & 0x000FC000) >> 18;
            uint32_t SFREQ = (read & 0x00003C00) >> 10;
            uint32_t RATE  = (read & 0x000003E0) >> 5;

            switch (SFREQ)
            {
            case 1:
                map->sampling_rate = 8000;
                break;
            case 2:
                map->sampling_rate = 16000;
                break;
            case 3:
                map->sampling_rate = 32000;
                break;
            case 6:
                map->sampling_rate = 11025;
                break;
            case 7:
                map->sampling_rate = 22050;
                break;
            case 8:
                map->sampling_rate = 44100;
                break;
            case 11:
                map->sampling_rate = 12000;
                break;
            case 12:
                map->sampling_rate = 24000;
                break;
            case 13:
                map->sampling_rate = 48000;
                break;
            default:
                TRACE_WARNING(MPS, "Unsupported DTS SFREQ %u", SFREQ);
                retcode = FAILURE;
                break;
            }

            switch (RATE)
            {
            case 0:
                map->bitrate_avg = 32000;
                break;
            case 1:
                map->bitrate_avg = 56000;
                break;
            case 2:
                map->bitrate_avg = 64000;
                break;
            case 3:
                map->bitrate_avg = 96000;
                break;
            case 4:
                map->bitrate_avg = 112000;
                break;
            case 5:
                map->bitrate_avg = 128000;
                break;
            case 6:
                map->bitrate_avg = 192000;
                break;
            case 7:
                map->bitrate_avg = 224000;
                break;
            case 8:
                map->bitrate_avg = 256000;
                break;
            case 9:
                map->bitrate_avg = 320000;
                break;
            case 10:
                map->bitrate_avg = 384000;
                break;
            case 11:
                map->bitrate_avg = 448000;
                break;
            case 12:
                map->bitrate_avg = 512000;
                break;
            case 13:
                map->bitrate_avg = 576000;
                break;
            case 14:
                map->bitrate_avg = 640000;
                break;
            case 15:
                map->bitrate_avg = 768000;
                break;
            case 16:
                map->bitrate_avg = 960000;
                break;
            case 17:
                map->bitrate_avg = 1024000;
                break;
            case 18:
                map->bitrate_avg = 1152000;
                break;
            case 19:
                map->bitrate_avg = 1280000;
                break;
            case 20:
                map->bitrate_avg = 1344000;
                break;
            case 21:
                map->bitrate_avg = 1408000;
                break;
            case 22:
                map->bitrate_avg = 1411000;
                break;
            case 23:
                map->bitrate_avg = 1472000;
                break;
            case 24:
                map->bitrate_avg = 1536000;
                break;
            case 29:
                map->bitrate_avg = 2048000; // Speficifation says "open"
                break;
            default:
                TRACE_WARNING(MPS, "Unsupported DTS RATE %u", RATE);
                retcode = FAILURE;
                break;
            }
        }
        else // audio codec in private stream could also be : LPCM, AAC, ... ?
        {
            TRACE_WARNING(MPS, "Unknown audio codec (0x%08X) inside audio private stream", startcode);
            retcode = FAILURE;
        }
    }
    else
    {
        // Look for a known MP3 start code
        uint16_t read = read_bits(bitstr, 16);

        if ((read & 0xFFF0) == 0xFFF0)
        {
            uint16_t audio_version_id = (read & 0x000C) >> 2;
            uint16_t layer_index = (read & 0x0003);

            read = read_bits(bitstr, 8);
            uint16_t bitrate_index = (read & 0x0078) >> 3;
            uint16_t samplingrate_index = (read & 0x0006) >> 1;

            uint32_t _mpeg_version = 0;
            uint32_t _mpeg_layer   = 0;

            switch (audio_version_id)
            {
            case 0x00:
                _mpeg_version = 3; // MPEG-2.5
                break;

            case 0x01:
            case 0x02:
                _mpeg_version = 2; // MPEG-2
                break;

            case 0x03:
                _mpeg_version = 1; // MPEG-1
                break;
            }

            switch (layer_index)
            {
            case 0x03:
                _mpeg_layer  = 1; // Layer 1
                map->stream_codec = CODEC_MPEG_L1;
                break;

            case 0x02:
                _mpeg_layer  = 2; // Layer 2
                map->stream_codec = CODEC_MPEG_L2;
                break;

            default:
            case 0x01:
            case 0x00: // reserved
                _mpeg_layer  = 3; // Layer 3
                map->stream_codec = CODEC_MPEG_L3;
                break;
            }

            //map->bitrate = bitrate_index_table[_mpeg_version - 1][_mpeg_layer - 1][bitrate_index];
            //map->sampling_rate = samplingrate_index_table[_mpeg_version - 1][samplingrate_index];
            //_mpeg_sampleperframe = sampleperframe_table[_mpeg_version - 1][_mpeg_layer - 1];
        }
        else
        {
            TRACE_WARNING(MPS, "Unknown ES type (0x%04X) inside PES audio packet (id: 0x%02X)",
                          read, header->stream_id);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the 'body' of a PES packet containing video data.
 * \param bitstr: Our bitstream reader.
 * \param header
 * \param packet
 * \param map
 * \return
 */
int parse_pes_v(Bitstream_t *bitstr, PesHeader_t *header, PesPacket_t *packet,
                MediaStream_t *map)
{
    TRACE_2(MPS, "> parse_pes_v()");
    int retcode = SUCCESS;

    // Avoid trying to get infos from a splitted video sample
    if (packet->PTS)
    {
        TRACE_2(MPS, "Trying get_video_infos() @ ");

        TRACE_INFO(MPS, "parse_pes_v() 0x%X @ %lli",
                   header->start_code, bitstream_get_absolute_byte_offset(bitstr));

        // Look for a sequence header start code
        uint32_t start_code = read_bits(bitstr, 32);

        // MPEG-1/2
        if (start_code == 0x000001B3)
        {
            uint32_t sizes = read_bits(bitstr, 32);
            uint32_t aspect_ratio_index = (sizes & 0x000000F0) >> 4;
            uint32_t framerate_index    = (sizes & 0x0000000F);

            map->color_matrix = CM_bt601;
            map->width  = (sizes & 0xFFF00000) >> 20;
            map->height = (sizes & 0x000FFF00) >> 8;

            uint32_t _mpeg_version = 2;

            if (_mpeg_version == 1)
            {
                map->stream_codec = CODEC_MPEG1;

                switch (aspect_ratio_index)
                {
                case 1:
                    map->display_aspect_ratio = 1.0;
                    break;
                case 8:
                case 12:
                    map->display_aspect_ratio = (4.0 / 3.0);
                    break;
                case 3:
                case 6:
                    map->display_aspect_ratio = (16.0 / 9.0);
                    break;
                default:
                    TRACE_WARNING(MPS, "Unsupported MPEG-1 aspect_ratio_index %u", aspect_ratio_index);
                    retcode = FAILURE;
                    break;
                }
            }
            else // if (_mpeg_version == 2)
            {
                map->stream_codec = CODEC_MPEG2;

                switch (aspect_ratio_index)
                {
                case 1:
                    map->video_aspect_ratio = 1.0;
                    break;
                case 2:
                    map->video_aspect_ratio = (4.0 / 3.0);
                    break;
                case 3:
                    map->video_aspect_ratio = (16.0 / 9.0);
                    break;
                case 4:
                    map->video_aspect_ratio = (2.21 / 1.0);
                    break;
                default:
                    TRACE_WARNING(MPS, "Unsupported MPEG-2 aspect_ratio_index %u", aspect_ratio_index);
                    retcode = FAILURE;
                    break;
                }
            }

            switch (framerate_index)
            {
            case 1:
                map->framerate = 23.976;
                map->framerate_num = 24000;
                map->framerate_base = 1001;
                break;
            case 2:
                map->framerate = 24.0;
                map->framerate_num = 24;
                map->framerate_base = 1;
                break;
            case 3:
                map->framerate = 25.0;
                map->framerate_num = 25;
                map->framerate_base = 1;
                break;
            case 4:
                map->framerate = 29.970;
                map->framerate_num = 30000;
                map->framerate_base = 1001;
                break;
            case 5:
                map->framerate = 30.0;
                map->framerate_num = 30;
                map->framerate_base = 1;
                break;
            case 6:
                map->framerate = 50.0;
                map->framerate_num = 50;
                map->framerate_base = 1;
                break;
            case 7:
                map->framerate = 59.940;
                map->framerate_num = 60000;
                map->framerate_base = 1001;
                break;
            case 8:
                map->framerate = 60.0;
                map->framerate_num = 60;
                map->framerate_base = 1;
                break;
            default:
                TRACE_WARNING(MPS, "Unsupported MPEG-1/2 framerate_index %u", framerate_index);
                retcode = FAILURE;
                break;
            }
        }
        else
        {
            TRACE_WARNING(MPS, "Unknown ES type (0x%08X) inside PES video packet (id: 0x%02X)",
                          start_code, header->stream_id);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */
