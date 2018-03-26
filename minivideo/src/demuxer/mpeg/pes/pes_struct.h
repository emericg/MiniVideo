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
 * \file      pes_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_MPEG_PES_STRUCT_H
#define PARSER_MPEG_PES_STRUCT_H

// minivideo headers
#include "../../../minivideo_typedef.h"

/* ************************************************************************** */

/*!
 * \struct PesHeader_t
 * \brief PES packet header structure.
 *
 * From 'ISO/IEC 13818-1' specification:
 * Table 2-17 – PES packet.
 */
typedef struct PesHeader_t
{
    int64_t offset_start;   //!< Absolute position of the very first byte of this PES (including start code)
    int64_t offset_end;     //!< Absolute position of the last byte of this PES

    uint32_t start_code;    //!< should be 0x000001
    uint8_t stream_id;
    int16_t packet_length;  //!< Full packet length
    int16_t payload_length; //!< Payload length only

} PesHeader_t;

/*!
 * \struct PesPacket_t
 * \brief PES packet structure.
 *
 * From 'ISO/IEC 13818-1' specification:
 * Table 2-17 – PES packet.
 */
typedef struct PesPacket_t
{
    uint32_t mpeg_version;

    // "Regular" PES parameters
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

    // Trick mode
    uint8_t trick_mode_control;
    uint8_t field_id;
    uint8_t intra_slice_refresh;
    uint8_t frequency_truncation;
    uint8_t rep_cntrl;

    // Additional_copy_info_flag
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
 * \enum PesStreamId_e
 * \brief Specify the type of Packetized Elementary Stream.
 *
 * 8b stream_id only.
 * H.222 / table 2-18.
 */
typedef enum PesStreamId_e
{
    SID_PROGRAM_END              = 0xB9,
    SID_PACK_HEADER              = 0xBA,
    SID_SYSTEM_HEADER            = 0xBB,
    SID_PROGRAM_STREAM_MAP       = 0xBC,
    SID_PRIVATE_STREAM_1         = 0xBD,
    SID_PADDING                  = 0xBE,
    SID_PRIVATE_STREAM_2         = 0xBF,

    SID_AUDIO                    = 0xC0, //!< audio: C0 to DF
    SID_VIDEO                    = 0xE0, //!< video: E0 to EF

    SID_ECM_STREAM               = 0xF0,
    SID_EMM_STREAM               = 0xF1,
    SID_DSMCC_STREAM             = 0xF2,
    SID_CEI13522_STREAM          = 0xF3,
    SID_2221A                    = 0xF4,
    SID_2221B                    = 0xF5,
    SID_2221C                    = 0xF6,
    SID_2221D                    = 0xF7,
    SID_2221E                    = 0xF8,
    SID_ANC_DATA                 = 0xF9,
    SID_SLPACKETIZED_STREAM      = 0xFA,
    SID_FLEXMUX_STREAM           = 0xFB,

    SID_RESERVED                 = 0xFC, //!< reserved: FC to FE

    SID_PROGRAM_STREAM_DIRECTORY = 0xFF

} PesStreamId_e;

/* ************************************************************************** */
#endif // PARSER_MPEG_PES_STRUCT_H
