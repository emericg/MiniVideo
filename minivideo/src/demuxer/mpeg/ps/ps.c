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
 * \file      ps.c
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

#include "ps.h"
#include "ps_struct.h"
#include "../pes/pes.h"
#include "../pes/pes_struct.h"

/* ************************************************************************** */

/*!
 * \brief Parse a pack header structure.
 * \param *bitstr The bitstream to use.
 * \param *pack_header A pointer to a pack_header structure.
 * \param *system_header A pointer to a system_header structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * This parser is based on the 'ISO/IEC 13818-1' international standard, part 1:
 * 'Transmission multiplexing and synchronization'.
 */
static int parse_pack_header(Bitstream_t *bitstr, PackHeader_t *pack_header, SystemHeader_t *system_header)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_pack_header()" CLR_RESET " @ %i\n", bitstream_get_absolute_byte_offset(bitstr) - 4);
    int retcode = SUCCESS;
    int i = 0;

    pack_header->pack_start_code = PES_PACK_HEADER;

    if (read_bits(bitstr, 2) != 1)
    {
        TRACE_ERROR(MPS, "wrong 'marker_bit'\n");
        return FAILURE;
    }

    pack_header->system_clock_reference_base = read_bits(bitstr, 3) << 30;
    MARKER_BIT
    pack_header->system_clock_reference_base += read_bits(bitstr, 15) << 15;
    MARKER_BIT
    pack_header->system_clock_reference_base += read_bits(bitstr, 15);
    MARKER_BIT
    pack_header->system_clock_reference_extension = read_bits(bitstr, 9);
    MARKER_BIT
    pack_header->program_mux_rate = read_bits(bitstr, 22);
    MARKER_BIT
    MARKER_BIT
    int reserved = read_bits(bitstr, 5);

    pack_header->pack_stuffing_length = read_bits(bitstr, 3);

    // Stuffing
    for (i = 0; i < pack_header->pack_stuffing_length; i++)
    {
        if (read_bits(bitstr, 8) != 0xFF)
        {
            TRACE_ERROR(MPS, "Wrong 'stuffing_byte'\n");
            return FAILURE;
        }
    }

    // System header
    if (next_bits(bitstr, 32) == PES_SYSTEM_HEADER)
    {
        TRACE_INFO(MPS, BLD_GREEN "parse_system_header()" CLR_RESET " @ %i\n", bitstream_get_absolute_byte_offset(bitstr) - 4);
        skip_bits(bitstr, 32);

        system_header->header_length = read_bits(bitstr, 16);
        MARKER_BIT
        system_header->rate_bound = read_bits(bitstr, 22);
        MARKER_BIT
        system_header->audio_bound = read_bits(bitstr, 6);
        system_header->fixed_flag = read_bits(bitstr, 1);
        system_header->CSPS_flag = read_bits(bitstr, 1);
        system_header->system_audio_lock_flag = read_bits(bitstr, 1);
        system_header->system_video_lock_flag = read_bits(bitstr, 1);
        MARKER_BIT
        system_header->video_bound = read_bits(bitstr, 5);
        system_header->packet_rate_restriction_flag = read_bits(bitstr, 1);

        int reserved_bits = read_bits(bitstr, 7);

        // stack it?
        while (next_bit(bitstr) == 1)
        {
            system_header->stream_id = read_bits(bitstr, 8);
            MARKER_BIT
            MARKER_BIT
            system_header->PSTD_buffer_bound_scale = read_bits(bitstr, 1);
            system_header->PSTD_buffer_size_bound = read_bits(bitstr, 13);
        }
    }
    else
    {
        TRACE_2(MPS, " > No system_header()\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse a program stream map structure.
 * \param *bitstr A bitstream.
 * \param *packet A program stream map structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * \todo Parse desciptors.
 *
 * H.222 / 2.5.4 Program Stream map.
 */
int parse_program_stream_map(Bitstream_t *bitstr, ProgramStreamMap_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_program_stream_map()" CLR_RESET " @ %i\n", bitstream_get_absolute_byte_offset(bitstr) - 4);
    int retcode = SUCCESS;
    int i = 0, N = 0, N1 = 0, N2 = 0;

    packet->packet_start_code_prefix = 0x000001;
    packet->map_stream_id = 0xBC;

    packet->program_stream_map_length = read_bits(bitstr, 16);
    packet->current_next_indicator = read_bit(bitstr);
    int reserved1 = read_bits(bitstr, 2);
    packet->program_stream_map_version = read_bits(bitstr, 5);
    int reserved2 = read_bits(bitstr, 7);
    MARKER_BIT
    packet->program_stream_map_info_length = read_bits(bitstr, 16);
    for (i = 0; i < N1; i++)
    {
        // descriptor()
    }

    packet->elementary_stream_map_length = read_bits(bitstr, 16);
    for (i = 0; i < N1; i++)
    {
        // Stack it?
        packet->stream_type = read_bits(bitstr, 8);
        packet->elementary_stream_id = read_bits(bitstr, 8);
        packet->elementary_stream_info_length = read_bits(bitstr, 16);
        for (i = 0; i < N2; i++)
        {
            // descriptor()
        }
    }

    packet->CRC_32 = read_bits(bitstr, 32);

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse a program stream directory structure.
 * \param *bitstr A bitstream.
 * \param *packet A program stream directory structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * H.222 / 2.5.5 Program Stream directory.
 */
static int parse_program_stream_directory(Bitstream_t *bitstr, ProgramStreamDirectory_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_program_stream_directory()" CLR_RESET " @ %i\n", bitstream_get_absolute_byte_offset(bitstr) - 4);
    int retcode = SUCCESS;
    int i = 0;

    packet->packet_start_code_prefix = 0x000001;
    packet->directory_stream_id = 0xFF;
    packet->PES_packet_length = read_bits(bitstr, 16);
    packet->number_of_access_units = read_bits(bitstr, 15);
    MARKER_BIT

    packet->prev_directory_offset = read_bits(bitstr, 15) << 30;
    MARKER_BIT
    packet->prev_directory_offset += read_bits(bitstr, 15) << 15;
    MARKER_BIT
    packet->prev_directory_offset += read_bits(bitstr, 15);
    MARKER_BIT

    packet->next_directory_offset = read_bits(bitstr, 15) << 30;
    MARKER_BIT
    packet->next_directory_offset += read_bits(bitstr, 15) << 15;
    MARKER_BIT
    packet->next_directory_offset += read_bits(bitstr, 15);
    MARKER_BIT

    for (i = 0; i < packet->number_of_access_units; i++)
    {
        // TODO stack it?
        packet->packet_stream_id = read_bits(bitstr, 8);
        packet->PES_header_position_offset_sign = read_bit(bitstr);

        packet->PES_header_position_offset = read_bits(bitstr, 14) << 30;
        MARKER_BIT
        packet->PES_header_position_offset += read_bits(bitstr, 15) << 15;
        MARKER_BIT
        packet->PES_header_position_offset += read_bits(bitstr, 15);
        MARKER_BIT

        packet->reference_offset = read_bits(bitstr, 16);
        MARKER_BIT
        int reserved1 = read_bits(bitstr, 3);

        packet->PTS = read_bits(bitstr, 3) << 30;
        MARKER_BIT
        packet->PTS += read_bits(bitstr, 15) << 15;
        MARKER_BIT
        packet->PTS += read_bits(bitstr, 15);
        MARKER_BIT

        packet->byes_to_read = read_bits(bitstr, 15) << 15;
        MARKER_BIT
        packet->byes_to_read += read_bits(bitstr, 8);
        MARKER_BIT

        packet->intra_coded_indicator = read_bit(bitstr);
        packet->coding_parameters_indicator = read_bits(bitstr, 2);
        int reserved2 = read_bits(bitstr, 4);
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse a mp4 file.
 * \param *video A pointer to a VideoFile_t structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * This parser is based on the 'ISO/IEC 13818-1' international standard, part 1:
 * 'Transmission multiplexing and synchronization'.
 */
int ps_fileParse(VideoFile_t *video)
{
    TRACE_INFO(MPS, BLD_GREEN "ps_fileParse()\n" CLR_RESET);
    int retcode = SUCCESS;
    int sid = 0;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(video, NULL);

    if (bitstr != NULL)
    {
        // Init bitstream_map to store container infos
        retcode = init_bitstream_map(&video->tracks_audio[0], 999999);
        retcode = init_bitstream_map(&video->tracks_video[0], 999999);

        if (retcode == SUCCESS)
        {
            video->tracks_audio[0]->stream_type = stream_AUDIO;
            video->tracks_audio[0]->stream_level = stream_level_PES;
            video->tracks_audio[0]->stream_codec = CODEC_MPEG_L3;
            video->tracks_audio[0]->sample_alignment = false;

            video->tracks_video[0]->stream_type = stream_VIDEO;
            video->tracks_video[0]->stream_level = stream_level_PES;
            video->tracks_video[0]->stream_codec = CODEC_MPEG12;
            video->tracks_video[0]->sample_alignment = false;

            // Read bitstream
            while (retcode == SUCCESS &&
                   (bitstream_get_absolute_byte_offset(bitstr) + 4) < video->file_size &&
                   next_bits(bitstr, 32) != PES_PROGRAM_END)
            {
                PackHeader_t pack_header;
                SystemHeader_t system_header;

                if (read_bits(bitstr, 32) == PES_PACK_HEADER)
                {
                    // Parse pack & system header
                    retcode = parse_pack_header(bitstr, &pack_header, &system_header);

                    // Then loop on PES
                    while (retcode == SUCCESS &&
                           (bitstream_get_absolute_byte_offset(bitstr) + 4) < video->file_size &&
                           next_bits(bitstr, 32) != PES_PACK_HEADER &&
                           next_bits(bitstr, 32) != PES_PROGRAM_END)
                    {
                        // Init
                        PesPacket_t pes_packet;
                        ProgramStreamMap_t pes_streammap;
                        ProgramStreamDirectory_t pes_streamdirectory;

                        // Parse start code
                        pes_packet.packet_start_offset = bitstream_get_absolute_byte_offset(bitstr);
                        pes_packet.packet_start_code   = read_bits(bitstr, 24);
                        pes_packet.stream_id           = (uint8_t)read_bits(bitstr, 8);

                        if (pes_packet.packet_start_code == PES_PACKETSTARTCODE)
                        {
                            switch (pes_packet.stream_id)
                            {
                            case SID_PROGRAM_STREAM_MAP:
                                retcode = parse_program_stream_map(bitstr, &pes_streammap);
                                break;
                            case SID_PROGRAM_STREAM_DIRECTORY:
                                retcode = parse_program_stream_directory(bitstr, &pes_streamdirectory);
                                break;
                            case SID_PADDING:
                                retcode = parse_pes_padding(bitstr, &pes_packet);
                                break;
                            case SID_PRIVATE_STREAM_1:
                                TRACE_2(MPS, BLD_GREEN "Private Stream 1 PES" CLR_RESET " @ %i\n", pes_packet.packet_start_offset);
                                retcode = skip_pes(bitstr, &pes_packet);
                                break;
                            case SID_PRIVATE_STREAM_2:
                                TRACE_2(MPS, BLD_GREEN "Private Stream 2 PES" CLR_RESET " @ %i\n", pes_packet.packet_start_offset);
                                retcode = skip_pes(bitstr, &pes_packet);
                                break;
                            case SID_VIDEO:
                                TRACE_INFO(MPS, BLD_GREEN "parse_pes_video()" CLR_RESET " @ %i\n", pes_packet.packet_start_offset);
                                retcode = parse_pes(bitstr, &pes_packet);
                                //print_pes(&pes_packet);

                                // Set sample into the bitstream_map
                                video->tracks_video[0]->sample_count++;
                                sid = video->tracks_video[0]->sample_count;
                                if (sid < 999999)
                                {
                                    video->tracks_video[0]->sample_type[sid] = sample_VIDEO;
                                    video->tracks_video[0]->sample_size[sid] = pes_packet.PES_packet_length + 6;
                                    video->tracks_video[0]->sample_offset[sid] = pes_packet.packet_start_offset;
                                    video->tracks_video[0]->sample_pts[sid] = pes_packet.PTS;
                                    video->tracks_video[0]->sample_dts[sid] = pes_packet.DTS;
                                }
                                break;
                            case SID_AUDIO:
                                TRACE_INFO(MPS, BLD_GREEN "parse_pes_audio()" CLR_RESET " @ %i\n", pes_packet.packet_start_offset);
                                retcode = parse_pes(bitstr, &pes_packet);
                                //print_pes(&pes_packet);

                                // Set sample into the bitstream_map
                                video->tracks_audio[0]->sample_count++;
                                sid = video->tracks_audio[0]->sample_count;
                                if (sid < 999999)
                                {
                                    video->tracks_audio[0]->sample_type[sid] = sample_AUDIO;
                                    video->tracks_audio[0]->sample_size[sid] = pes_packet.PES_packet_length + 6;
                                    video->tracks_audio[0]->sample_offset[sid] = pes_packet.packet_start_offset;
                                    video->tracks_audio[0]->sample_pts[sid] = pes_packet.PTS;
                                    video->tracks_audio[0]->sample_dts[sid] = pes_packet.DTS;
                                }
                                break;
                            default:
                                TRACE_WARNING(MPS, "Unknown PES type (0x%06X%02X) @ %i\n", pes_packet.packet_start_code, pes_packet.stream_id, pes_packet.packet_start_offset);
                                retcode = skip_pes(bitstr, &pes_packet);
                                break;
                            }
                        }
                        else
                        {
                            TRACE_ERROR(MPS, "No valid packet_start_code at %i\n", pes_packet.packet_start_offset);
                            retcode = FAILURE;
                        }
                    }
                }
                else
                {
                    TRACE_ERROR(MPS, "No pack header\n");
                    retcode = FAILURE;
                }
            }
        }

        // Free bitstream
        free_bitstream(&bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
