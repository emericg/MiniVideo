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
 * \file      pes_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_MPEG_PES_STRUCT_H
#define PARSER_MPEG_PES_STRUCT_H

// minivideo headers
#include "../../../typedef.h"

/* ************************************************************************** */

/*!
 * \struct PesPacket_t
 * \brief PES packet structure.
 *
 * H.222 / table 2-17.
 */
typedef struct PesPacket_t
{
    uint32_t packet_start_code;  //!< 0x000001
    uint32_t packet_start_offset;
    uint8_t stream_id;
    uint16_t PES_packet_length;

    // Regular PES parameters
    uint8_t PES_scrambling_control;
    uint8_t PES_priority;
    uint8_t data_alignment_indicator;
    uint8_t copyright;
    uint8_t original_or_copy;
    uint8_t PTS_DTS_flag;
    uint8_t ESCR_flag;
    uint8_t ES_rate_flag;
    uint8_t DSM_trick_mode_flag;
    uint8_t additional_copy_info_flag;
    uint8_t PES_CRC_flag;
    uint8_t PES_extension_flag;
    uint8_t PES_header_data_length;

    uint64_t PTS;
    uint64_t DTS;
    uint64_t ESCR_base;
    uint16_t ESCR_extension;
    uint32_t ES_rate;

    // trick mode
    uint8_t trick_mode_control;
    uint8_t field_id;
    uint8_t intra_slice_refresh;
    uint8_t frequency_truncation;
    uint8_t rep_cntrl;

    // additional_copy_info_flag
    uint8_t additional_copy_info;

    // PES_CRC_flag
    uint16_t previous_PES_packet_CRC;

    // PES_extension_flag
    uint8_t PES_private_data_flag;
    uint8_t pack_header_field_flag;
    uint8_t program_packet_sequence_counter_flag;
    uint8_t PSTD_buffer_flag;
    uint8_t PES_extension_flag_2;

        // PES_private_data_flag
        uint8_t PES_private_data[16];

        // pack_header_field_flag
        uint8_t pack_field_length;

        // program_packet_sequence_counter_flag
        uint8_t program_packet_sequence_counter;
        uint8_t MPEG1_MPEG2_identifier;
        uint8_t original_stuff_length;

        // PSTD_buffer_flag
        uint8_t PSTD_buffer_scale;
        uint16_t PSTD_buffer_size;

        // PES_extension_flag_2
        uint8_t PES_extension_field_length;

} PesPacket_t;

/* ************************************************************************** */

/*!
 * \enum trickMode_e
 * \brief Can modify stream speed.
 *
 * H.222 / table 2-20.
 */
typedef enum trickMode_e
{
    TM_FAST_FORWARD = 0x0,
    TM_SLOW_MOTION  = 0x1,
    TM_FREEZE_FRAME = 0x2,
    TM_FAST_REVERSE = 0x3,
    TM_SLOW_REVERSE = 0x4
} trickMode_e;

/*!
 * \enum fieldId_e
 * \brief Indicate which field(s) should be displayed.
 *
 * H.222 / table 2-21.
 */
typedef enum fieldId_e
{
    FID_TOP      = 0x0,
    FID_BOTTOM   = 0x1,
    FID_FRAME    = 0x2,
    FID_RESERVED = 0x3
} fieldId_e;

/*!
 * \enum pesFullStartCodes_e
 * \brief Specify the type of Packetized Elementary Stream.
 *
 * 32b full start code = 24b packet_start_code + 8b stream_id.
 * H.222 / table 2-18.
 */
typedef enum pesFullStartCodes_e
{
    PES_PACKETSTARTCODE          = 0x000001,

    PES_PROGRAM_END              = 0x000001B9,
    PES_PACK_HEADER              = 0x000001BA,
    PES_SYSTEM_HEADER            = 0x000001BB,
    PES_PROGRAM_STREAM_MAP       = 0x000001BC,
    PES_PRIVATE_STREAM_1         = 0x000001BD,
    PES_PADDING                  = 0x000001BE,
    PES_PRIVATE_STREAM_2         = 0x000001BF,
    PES_VIDEO                    = 0x000001E0,
    PES_AUDIO                    = 0x000001C0,
    // stuff...
    PES_PROGRAM_STREAM_DIRECTORY = 0x000001FF
} pesFullStartCodes_e;

/*!
 * \enum pesStreamId_e
 * \brief Specify the type of Packetized Elementary Stream.
 *
 * 8b stream_id only.
 * H.222 / table 2-18.
 */
typedef enum pesStreamId_e
{
    SID_PROGRAM_END              = 0xB9,
    SID_PACK_HEADER              = 0xBA,
    SID_SYSTEM_HEADER            = 0xBB,
    SID_PROGRAM_STREAM_MAP       = 0xBC,
    SID_PRIVATE_STREAM_1         = 0xBD,
    SID_PADDING                  = 0xBE,
    SID_PRIVATE_STREAM_2         = 0xBF,
    SID_VIDEO                    = 0xE0,
    SID_AUDIO                    = 0xC0,
    // stuff...
    SID_PROGRAM_STREAM_DIRECTORY = 0xFF
} pesStreamId_e;

/* ************************************************************************** */
#endif /* PARSER_MPEG_PES_STRUCT_H */
