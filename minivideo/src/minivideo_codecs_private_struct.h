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
 * \file      minivideo_codecs_private_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef PARSER_CODEC_PRIVATE_STRUCT_H
#define PARSER_CODEC_PRIVATE_STRUCT_H
/* ************************************************************************** */

// minivideo headers
#include "decoder/h264/h264_parameterset_struct.h"
#include "decoder/h265/h265_parameterset_struct.h"
#include "decoder/h266/h266_parameterset_struct.h"

/* ************************************************************************** */

typedef struct codecprivate_avcC_t
{
    uint8_t configurationVersion;
    uint8_t AVCProfileIndication;
    uint8_t profile_compatibility;
    uint8_t AVCLevelIndication;
    uint8_t lengthSizeMinusOne;

    uint8_t sps_count;
    int32_t *sps_sample_size;
    int64_t *sps_sample_offset;
    h264_sps_t *sps_array[MAX_SPS];

    uint8_t pps_count;
    int32_t *pps_sample_size;
    int64_t *pps_sample_offset;
    h264_pps_t *pps_array[MAX_PPS];

} codecprivate_avcC_t;

/* ************************************************************************** */

typedef struct codecprivate_hvcC_t
{
    uint8_t configurationVersion;
    uint8_t general_profile_space;
    bool general_tier_flag;
    uint8_t general_profile_idc;
    uint32_t general_profile_compatibility_flags;
    uint64_t general_constraint_indicator_flags;
    uint8_t general_level_idc;

    uint16_t min_spatial_segmentation_idc;
    uint8_t parallelismType;
    uint8_t chromaFormat;
    uint8_t bitDepthLumaMinus8;
    uint8_t bitDepthChromaMinus8;
    uint16_t avgFrameRate;
    uint8_t constantFrameRate;
    uint8_t numTemporalLayers;
    bool temporalIdNested;
    uint8_t lengthSizeMinusOne;

    uint8_t numOfArrays;
    uint8_t *array_completeness;
    uint8_t *NAL_unit_type;
    uint16_t *numNalus;
    uint16_t **nalUnitLength;
    uint8_t ***nalUnit;

} codecprivate_hvcC_t;

/* ************************************************************************** */

typedef struct codecprivate_vvcC_t
{
    uint8_t profile_idc;
    uint8_t tier_flag;
    uint8_t general_level_idc;
    uint8_t ptl_frame_only_constraint_flag;
    uint8_t ptl_multilayer_enabled_flag;
    uint8_t gci_present_flag;
    //if (gci_present_flag)
        uint8_t gci_general_constraints[9];
        uint8_t gci_num_reserved_bits;
    uint8_t *ptl_sublayer_level_present_flag;
    //if (ptl_sublayer_level_present_flag)
        uint8_t *sublayer_level_idc;
    uint8_t ptl_num_sub_profiles;
    uint32_t *general_sub_profile_idc;

    // TODO // NALs

} codecprivate_vvcC_t;

/* ************************************************************************** */

typedef struct codecprivate_vpcC_t
{
    uint8_t profile;
    uint8_t level;
    uint8_t bitDepth;
    uint8_t chromaSubsampling;
    bool videoFullRangeFlag;
    uint8_t colourPrimaries;
    uint8_t transferCharacteristics;
    uint8_t matrixCoefficients;

    uint16_t codecIntializationDataSize;

} codecprivate_vpcC_t;

/* ************************************************************************** */

typedef struct codecprivate_av1C_t
{
    bool marker;
    uint8_t version;
    uint8_t seq_profile;
    uint8_t seq_level_idx_0;
    bool seq_tier_0;
    bool high_bitdepth;
    bool twelve_bit;
    bool monochrome;
    bool chroma_subsampling_x;
    bool chroma_subsampling_y;
    uint8_t chroma_sample_position;
    bool initial_presentation_delay_present;
    //if (bl_present_flag)
        uint8_t initial_presentation_delay_minus_one;

    // TODO // OBUs

} codecprivate_av1C_t;

/* ************************************************************************** */

typedef struct codecprivate_dvcC_t
{
    uint8_t dv_version_major;
    uint8_t dv_version_minor;
    uint8_t dv_profile;
    uint8_t dv_level;
    uint8_t rpu_present_flag;
    uint8_t el_present_flag;
    uint8_t bl_present_flag;
    //if (initial_presentation_delay_present)
        uint8_t dv_bl_signal_compatibility_id;

} codecprivate_dvcC_t;

/* ************************************************************************** */

typedef struct codecprivate_mvcC_t
{
    //

} codecprivate_mvcC_t;

/* ************************************************************************** */
#endif // PARSER_CODEC_PRIVATE_STRUCT_H
