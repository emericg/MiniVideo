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
 * \file      descriptor_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PROGRAM_ELEMENT_DESCRIPTORS_STRUCT_H
#define PROGRAM_ELEMENT_DESCRIPTORS_STRUCT_H

// minivideo
#include "../../minivideo_typedef.h"

/* ************************************************************************** */

/*!
 * \struct VideoStreamDescriptor_t
 * \brief Video stream descriptor structure.
 *
 * 2.6.2 Video stream descriptor.
 * H.222 table 2-40.
 */
typedef struct VideoStreamDescriptor_t
{
    uint8_t descriptor_tag;
    uint16_t descriptor_length;

    uint8_t multiple_frame_rate_flag;
    uint8_t frame_rate_code;
    uint8_t MPEG1_only_flag;
    uint8_t constrained_parameter_flag;
    uint8_t still_picture_flag;

    // MPEG1_only_flag
    uint8_t profile_and_level_indication;
    uint8_t chroma_format;
    uint8_t frame_rate_extension_flag;

} VideoStreamDescriptor_t;

/*!
 * \struct AudioStreamDescriptor_t
 * \brief Audio stream descriptor structure.
 *
 * 2.6.4 Audio stream descriptor.
 * H.222 table 2-42.
 */
typedef struct AudioStreamDescriptor_t
{
    uint8_t descriptor_tag;
    uint16_t descriptor_length;

    uint8_t free_format_flag;
    uint8_t ID;
    uint8_t layer;
    uint8_t variable_rate_audio_indicator;

} AudioStreamDescriptor_t;

/*!
 * \struct HierarchyDescriptor_t
 * \brief Hierarchy descriptor structure.
 *
 * 2.6.6 Hierarchy descriptor.
 * H.222 table 2-43.
 */
typedef struct HierarchyDescriptor_t
{
    uint8_t descriptor_tag;
    uint16_t descriptor_length;

    uint8_t hierarchy_type;
    uint8_t hierarchy_layer_index;
    uint8_t hierarchy_embedded_layer_index;
    uint8_t hierarchy_channel;

} HierarchyDescriptor_t;

/* ************************************************************************** */
#endif // PROGRAM_ELEMENT_DESCRIPTORS_STRUCT_H
