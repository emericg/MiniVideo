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
 * \file      ps.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "ps.h"
#include "ps_struct.h"
#include "../pes/pes.h"
#include "../pes/pes_struct.h"
#include "../../../minivideo_typedef.h"
#include "../../../minitraces.h"
#include "../../../bitstream_utils.h"

// C standard libraries
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Parse a pack header structure.
 * \param *bitstr The bitstream to use.
 * \param *pack_header A pointer to a pack_header structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.5.3.3 Pack layer of Program Stream.
 * Table 2-33 – Program Stream pack header
 */
static int parse_pack_header(Bitstream_t *bitstr, PesHeader_t *header, PackHeader_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_pack_header()" CLR_RESET " @ %lli",
               header->offset_start);

    int retcode = SUCCESS;

    // Pack Headers do not have lengh field, rewind 2 bytes
    rewind_bits(bitstr, 16);

    if (read_bits(bitstr, 2) != 1)
    {
        TRACE_ERROR(MPS, "wrong 'marker_bits'");
        return FAILURE;
    }

    packet->system_clock_reference_base = read_bits(bitstr, 3) << 30;
    MARKER_BIT
    packet->system_clock_reference_base += read_bits(bitstr, 15) << 15;
    MARKER_BIT
    packet->system_clock_reference_base += read_bits(bitstr, 15);
    MARKER_BIT
    packet->system_clock_reference_extension = read_bits(bitstr, 9);
    MARKER_BIT
    packet->program_mux_rate = read_bits(bitstr, 22);
    MARKER_BIT
    MARKER_BIT
    /*unsigned reserved =*/ read_bits(bitstr, 5);

    packet->pack_stuffing_length = read_bits(bitstr, 3);

    // Stuffing
    for (uint8_t i = 0; i < packet->pack_stuffing_length; i++)
    {
        if (read_bits(bitstr, 8) != 0xFF)
        {
            TRACE_ERROR(MPS, "Wrong 'stuffing_byte'");
            return FAILURE;
        }
    }

    // System header // TODO split into its own function?
    if (next_bits(bitstr, 32) == 0x000001BB)
    {
        TRACE_INFO(MPS, BLD_GREEN "parse_system_header()" CLR_RESET " @ %lli",
                   bitstream_get_absolute_byte_offset(bitstr) - 4);
        skip_bits(bitstr, 48); // start code + size

        SystemHeader_t system_header;

        MARKER_BIT
        system_header.rate_bound = read_bits(bitstr, 22);
        MARKER_BIT
        system_header.audio_bound = read_bits(bitstr, 6);
        system_header.fixed_flag = read_bits(bitstr, 1);
        system_header.CSPS_flag = read_bits(bitstr, 1);
        system_header.system_audio_lock_flag = read_bits(bitstr, 1);
        system_header.system_video_lock_flag = read_bits(bitstr, 1);
        MARKER_BIT
        system_header.video_bound = read_bits(bitstr, 5);
        system_header.packet_rate_restriction_flag = read_bits(bitstr, 1);

        /*unsigned reserved_bits =*/ read_bits(bitstr, 7);

        // stack it?
        while (next_bit(bitstr) == 1)
        {
            system_header.stream_id = read_bits(bitstr, 8);
            MARKER_BIT
            MARKER_BIT
            system_header.PSTD_buffer_bound_scale = read_bits(bitstr, 1);
            system_header.PSTD_buffer_size_bound = read_bits(bitstr, 13);
        }
    }
    else
    {
        TRACE_2(MPS, " > No system_header()");
    }

    // Pack header have no length field, so we just have to parse them correctly
    header->offset_end = bitstream_get_absolute_byte_offset(bitstr);
    header->payload_length = header->offset_end - header->offset_start - 4;

    return retcode;
}

/*!
 * \brief Parse a system header structure.
 * \param *bitstr The bitstream to use.
 * \param *system_header A pointer to a system_header structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.5.3.5 System header.
 * Table 2-34 – Program Stream system header
 */
static int parse_system_header(Bitstream_t *bitstr, PesHeader_t *header, SystemHeader_t *packet)
{
    int retcode = SUCCESS;

    TRACE_INFO(MPS, BLD_GREEN "parse_system_header()" CLR_RESET " @ %lli",
               header->offset_start);

    MARKER_BIT
    packet->rate_bound = read_bits(bitstr, 22);
    MARKER_BIT
    packet->audio_bound = read_bits(bitstr, 6);
    packet->fixed_flag = read_bits(bitstr, 1);
    packet->CSPS_flag = read_bits(bitstr, 1);
    packet->system_audio_lock_flag = read_bits(bitstr, 1);
    packet->system_video_lock_flag = read_bits(bitstr, 1);
    MARKER_BIT
    packet->video_bound = read_bits(bitstr, 5);
    packet->packet_rate_restriction_flag = read_bits(bitstr, 1);

    /*unsigned reserved_bits =*/ read_bits(bitstr, 7);

    // stack it?
    while (next_bit(bitstr) == 1)
    {
        packet->stream_id = read_bits(bitstr, 8);
        MARKER_BIT
        MARKER_BIT
        packet->PSTD_buffer_bound_scale = read_bits(bitstr, 1);
        packet->PSTD_buffer_size_bound = read_bits(bitstr, 13);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse a program stream map structure.
 * \param *bitstr The bitstream to use.
 * \param *header PES packet header.
 * \param *packet A program stream map structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * \todo Parse desciptors.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.5.4 Program Stream map
 * Table 2-35 – Program Stream map
 */
int parse_program_stream_map(Bitstream_t *bitstr, PesHeader_t *header, ProgramStreamMap_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_program_stream_map()" CLR_RESET " @ %lli",
               header->offset_start);
    int retcode = SUCCESS;
    int i = 0, N1 = 0, N2 = 0;

    packet->current_next_indicator = read_bit(bitstr);
    /*int reserved1 =*/ read_bits(bitstr, 2);
    packet->program_stream_map_version = read_bits(bitstr, 5);
    /*int reserved2 =*/ read_bits(bitstr, 7);
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
 * \param *bitstr The bitstream to use.
 * \param *packet A program stream directory structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * From 'ISO/IEC 13818-1' specification:
 * 2.5.5 Program Stream directory
 * Table 2-36 – Program Stream directory packet
 */
static int parse_program_stream_directory(Bitstream_t *bitstr, PesHeader_t *header, ProgramStreamDirectory_t *packet)
{
    TRACE_INFO(MPS, BLD_GREEN "parse_program_stream_directory()" CLR_RESET " @ %lli",
               header->offset_start);
    int retcode = SUCCESS;

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

    for (uint16_t i = 0; i < packet->number_of_access_units; i++)
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
        /*unsigned reserved1 =*/ read_bits(bitstr, 3);

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
        /*unsigned reserved2 =*/ read_bits(bitstr, 4);
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int ps_fileParse(MediaFile_t *media)
{
    int retcode = SUCCESS;

    TRACE_INFO(MPS, BLD_GREEN "ps_fileParse()" CLR_RESET);

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an MpegPs structure
        MpegPs_t mpg;
        memset(&mpg, 0, sizeof(MpegPs_t));

        // A convenient way to stop the parser
        mpg.run = true;

        // stuff
        const int64_t min_packet_size = 4;

        // Loop on PES packets
        while (mpg.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - min_packet_size))
        {
            // Init
            PackHeader_t pack_header;
            memset(&pack_header, 0, sizeof(PackHeader_t));
            SystemHeader_t system_header;
            memset(&system_header, 0, sizeof(SystemHeader_t));

            PesHeader_t pes_header;
            memset(&pes_header, 0, sizeof(PesHeader_t));
            PesPacket_t pes_packet;
            memset(&pes_packet, 0, sizeof(PesPacket_t));
            ProgramStreamMap_t pes_streammap;
            memset(&pes_streammap, 0, sizeof(ProgramStreamMap_t));
            ProgramStreamDirectory_t pes_streamdirectory;
            memset(&pes_streamdirectory, 0, sizeof(ProgramStreamDirectory_t));

            // Parse packet header
            parse_pes_header(bitstr, &pes_header);

            switch (pes_header.stream_id)
            {
            case SID_PACK_HEADER:
                retcode = parse_pack_header(bitstr, &pes_header, &pack_header);
                mpg.stat_packheader++;
                break;
            case SID_SYSTEM_HEADER:
                retcode = parse_system_header(bitstr, &pes_header, &system_header);
                mpg.stat_systemheader++;
                break;

            case SID_PROGRAM_STREAM_MAP:
                retcode = parse_program_stream_map(bitstr, &pes_header, &pes_streammap);
                mpg.stat_packet_psm++;
                break;
            case SID_PROGRAM_STREAM_DIRECTORY:
                retcode = parse_program_stream_directory(bitstr, &pes_header, &pes_streamdirectory);
                mpg.stat_packet_psd++;
                break;
            case SID_PRIVATE_STREAM_2:
                TRACE_2(MPS, BLD_GREEN "Private Stream 2 PES" CLR_RESET " @ %lli", pes_header.offset_start);
                mpg.stat_packet_private++;
                break;
            case SID_PADDING:
                retcode = parse_pes_padding(bitstr, &pes_header, &pes_packet);
                mpg.stat_packet_other++;
                break;

            case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6:
            case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC:
            case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1: case 0xD2:
            case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7: case 0xD8:
            case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE:
            case 0xDF: case SID_PRIVATE_STREAM_1: case SID_AUDIO:
            {
                TRACE_INFO(MPS, BLD_GREEN "parse_pes_audio()" CLR_RESET " @ %lli", pes_header.offset_start);

                // Find a trackid
                unsigned track_id = pes_header.stream_id - 0xC0;
                if (pes_header.stream_id == SID_PRIVATE_STREAM_1)
                    track_id = 0;

                // If a private stream is in the stream, the 'regular' trackid order is shifted
                //while (media->tracks_audio[track_id] == NULL)
                //    track_id--;

                // Init bitstream_map (as needed) to store samples
                if (media->tracks_audio[track_id] == NULL)
                {
                    retcode = init_bitstream_map(&media->tracks_audio[track_id], 0, 999999);
                    media->tracks_audio_count++;
                    media->tracks_audio[track_id]->stream_type = stream_AUDIO;
                }

                retcode = parse_pes(bitstr, &pes_header, &pes_packet);
                parse_pes_a(bitstr, &pes_header, &pes_packet, media->tracks_audio[track_id]);
                mpg.stat_packet_audio++;

                // Set sample into the bitstream_map
                unsigned sample_id = media->tracks_audio[track_id]->sample_count++;
                if (sample_id < 999999)
                {
                    media->tracks_audio[track_id]->sample_type[sample_id] = sample_AUDIO;
                    media->tracks_audio[track_id]->sample_size[sample_id] = pes_header.payload_length - pes_packet.PES_header_data_length;
                    media->tracks_audio[track_id]->sample_offset[sample_id] = pes_header.offset_start + 6 + pes_packet.PES_header_data_length;
                    media->tracks_audio[track_id]->sample_pts[sample_id] = pes_packet.PTS;
                    media->tracks_audio[track_id]->sample_dts[sample_id] = pes_packet.DTS;
                }
            }
            break;

            case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:
            case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEA:
            case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
            case SID_VIDEO:
            {
                TRACE_INFO(MPS, BLD_GREEN "parse_pes_video()" CLR_RESET " @ %lli", pes_header.offset_start + 6);

                // Init bitstream_map (as needed) to store samples
                unsigned track_id = pes_header.stream_id - 0xE0;
                if (media->tracks_video[track_id] == NULL)
                {
                    retcode = init_bitstream_map(&media->tracks_video[track_id], 0, 999999);
                    media->tracks_video_count++;
                    media->tracks_video[track_id]->stream_type = stream_VIDEO;
                }

                retcode = parse_pes(bitstr, &pes_header, &pes_packet);
                parse_pes_v(bitstr, &pes_header, &pes_packet, media->tracks_video[track_id]);
                mpg.stat_packet_video++;

                // Set sample into the bitstream_map
                unsigned sample_id = media->tracks_video[track_id]->sample_count++;
                if (sample_id < 999999)
                {
                    media->tracks_video[track_id]->sample_type[sample_id] = sample_VIDEO;
                    media->tracks_video[track_id]->sample_size[sample_id] = pes_header.payload_length - pes_packet.PES_header_data_length;
                    media->tracks_video[track_id]->sample_offset[sample_id] = pes_header.offset_start + 6 + pes_packet.PES_header_data_length;
                    media->tracks_video[track_id]->sample_pts[sample_id] = pes_packet.PTS;
                    media->tracks_video[track_id]->sample_dts[sample_id] = pes_packet.DTS;
                }
            }
            break;

            case SID_PROGRAM_END:
                mpg.stat_packet_other++;
                mpg.run = false;
                break;

            default:
                TRACE_WARNING(MPS, "Unknown PES packet type (0x%02X) @ %lli",
                              pes_header.stream_id, pes_header.offset_start);
                mpg.stat_packet_other++;
                break;
            }

            retcode = jumpy_pes(bitstr, &pes_header);
        }

        // Free bitstream
        free_bitstream(&bitstr);

        // Recap
        TRACE_INFO(MPS, "MPEG PS (version %u) stats", mpg.mpeg_version);
        TRACE_INFO(MPS, "- Pack Headers:    %u", mpg.stat_packheader);
        TRACE_INFO(MPS, "- System Headers:  %u", mpg.stat_systemheader);
        TRACE_INFO(MPS, "- PSM packets:     %u", mpg.stat_packet_psm);
        TRACE_INFO(MPS, "- PSD packets:     %u", mpg.stat_packet_psd);
        TRACE_INFO(MPS, "- Private packets: %u", mpg.stat_packet_private);
        TRACE_INFO(MPS, "- Audio packets:   %u", mpg.stat_packet_audio);
        TRACE_INFO(MPS, "- Video packets:   %u", mpg.stat_packet_video);
        TRACE_INFO(MPS, "- Unknown packets: %u", mpg.stat_packet_other);
    }
    else
    {
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
