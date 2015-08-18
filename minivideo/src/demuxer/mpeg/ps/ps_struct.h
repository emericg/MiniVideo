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
 * \file      ps_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_MPEG_PS_STRUCT_H
#define PARSER_MPEG_PS_STRUCT_H

// minivideo headers
#include "../../../typedef.h"

/* ************************************************************************** */

/*!
 * \struct PackHeader_t
 * \brief PS "pack header" structure.
 *
 * H.222 / table 2-33.
 */
typedef struct PackHeader_t
{
    uint32_t pack_start_code;
    uint64_t system_clock_reference_base;
    uint16_t system_clock_reference_extension;
    uint32_t program_mux_rate;
    uint8_t pack_stuffing_length;

} PackHeader_t;

/*!
 * \struct SystemHeader_t
 * \brief PS "system header" structure.
 *
 * H.222 / table 2-34.
 */
typedef struct SystemHeader_t
{
    uint32_t system_header_start_code;
    uint16_t header_length;
    uint32_t rate_bound;
    uint8_t audio_bound;
    uint8_t fixed_flag;
    uint8_t CSPS_flag;
    uint8_t system_audio_lock_flag;
    uint8_t system_video_lock_flag;
    uint8_t video_bound;
    uint8_t packet_rate_restriction_flag;

    // stack it?
    uint8_t stream_id;
    uint8_t PSTD_buffer_bound_scale;
    uint16_t PSTD_buffer_size_bound;

} SystemHeader_t;

/*!
 * \struct ProgramStreamMap_t
 * \brief PS "program stream map" structure.
 *
 * H.222 / table 2-35.
 */
typedef struct ProgramStreamMap_t
{
    uint32_t packet_start_code_prefix;
    uint8_t map_stream_id;
    uint16_t program_stream_map_length;
    uint8_t current_next_indicator;
    uint8_t program_stream_map_version;
    uint16_t program_stream_map_info_length;
        // descriptor

    uint16_t elementary_stream_map_length;
        // stack it?
        uint8_t stream_type;
        uint8_t elementary_stream_id;
        uint16_t elementary_stream_info_length;
            // descriptor

    uint32_t CRC_32;

} ProgramStreamMap_t;

/*!
 * \struct ProgramStreamDirectory_t
 * \brief PS "program stream directory" structure.
 *
 * H.222 / table 2-36.
 */
typedef struct ProgramStreamDirectory_t
{
    uint32_t packet_start_code_prefix;
    uint8_t directory_stream_id;
    uint16_t PES_packet_length;
    uint16_t number_of_access_units;

    uint64_t prev_directory_offset;
    uint64_t next_directory_offset;

        // stack it?
        uint8_t packet_stream_id;
        uint8_t PES_header_position_offset_sign;
        uint64_t PES_header_position_offset;
        uint16_t reference_offset;
        uint64_t PTS;
        uint16_t byes_to_read;
        uint8_t intra_coded_indicator;
        uint8_t coding_parameters_indicator;

} ProgramStreamDirectory_t;

/* ************************************************************************** */
#endif // PARSER_MPEG_PS_STRUCT_H
