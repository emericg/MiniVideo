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

/*!
 * AVC Decoder Configuration Record, from ISO/IEC 14496-15 - 5.3.2
 */
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
    h264_sps_t *sps_array[H264_MAX_SPS];

    uint8_t pps_count;
    int32_t *pps_sample_size;
    int64_t *pps_sample_offset;
    h264_pps_t *pps_array[H264_MAX_PPS];

} codecprivate_avcC_t;

/* ************************************************************************** */

/*!
 * HEVC Decoder Configuration Record, from ISO/IEC 14496-15 - 8.3.2
 */
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

    uint32_t vps_count;
    int32_t vps_sample_size[H265_MAX_VPS];
    int64_t vps_sample_offset[H265_MAX_VPS];
    h265_vps_t *vps_array[H265_MAX_VPS];

    uint32_t sps_count;
    int32_t sps_sample_size[H265_MAX_SPS];
    int64_t sps_sample_offset[H265_MAX_SPS];
    h265_sps_t *sps_array[H265_MAX_SPS];

    uint32_t pps_count;
    int32_t pps_sample_size[H265_MAX_PPS];
    int64_t pps_sample_offset[H265_MAX_PPS];
    h265_pps_t *pps_array[H265_MAX_PPS];

} codecprivate_hvcC_t;

/* ************************************************************************** */

/*!
 * VVC Decoder Configuration Record, from ISO/IEC 14496-15 - 11.2.4
 */
typedef struct codecprivate_vvcC_t
{
    uint8_t lengthSizeMinusOne;
    bool ptl_present_flag;

    uint16_t ols_idx;
    uint8_t num_sublayers;
    uint8_t constant_frame_rate;
    uint8_t chroma_format_idc;
    uint8_t bit_depth_minus8;

    uint8_t num_bytes_constraint_info;
    uint8_t general_profile_idc;
    bool general_tier_flag;
    uint8_t general_level_idc;
    bool ptl_frame_only_constraint_flag;
    bool ptl_multilayer_enabled_flag;
    uint8_t general_constraint_info[63];
    bool ptl_sublayer_level_present_flag[8];
    uint8_t sublayer_level_idc[8];
    uint8_t ptl_num_sub_profiles;
    uint32_t general_sub_profile_idc[256];

    uint16_t max_picture_width;
    uint16_t max_picture_height;
    uint16_t avg_frame_rate;

    uint8_t num_of_arrays;

    uint32_t vps_count;
    int32_t vps_sample_size[H266_MAX_VPS];
    int64_t vps_sample_offset[H266_MAX_VPS];
    h266_vps_t *vps_array[H266_MAX_VPS];

    uint32_t sps_count;
    int32_t sps_sample_size[H266_MAX_SPS];
    int64_t sps_sample_offset[H266_MAX_SPS];
    h266_sps_t *sps_array[H266_MAX_SPS];

    uint32_t pps_count;
    int32_t pps_sample_size[H266_MAX_PPS];
    int64_t pps_sample_offset[H266_MAX_PPS];
    h266_pps_t *pps_array[H266_MAX_PPS];

} codecprivate_vvcC_t;

/* ************************************************************************** */

/*!
 * https://www.webmproject.org/vp9/mp4/
 */
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

} codecprivate_vpcC_t;

/* ************************************************************************** */

/*!
 * https://aomediacodec.github.io/av1-isobmff/
 */
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
