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
 * \file      h264_parameterset_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_PARAMETER_SET_STRUCT_H
#define H264_PARAMETER_SET_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"

/* ************************************************************************** */

#define MAX_SPS     32
#define MAX_PPS     256
#define MAX_MMCO    66
#define MAX_CPB     32
#define MAX_SLICES  1

/* ************************************************************************** */

//! H.264 Profiles IDC definitions, see http://en.wikipedia.org/wiki/H264#Profiles
typedef enum H264Profiles_e
{
    BASELINE            = 66,   //!< YUV 4:2:0/8  "Baseline profile"
    MAINP               = 77,   //!< YUV 4:2:0/8  "Main profile"
    UNKNOWN1            = 83,   //!< Unknown profile
    UNKNOWN2            = 86,   //!< Unknown profile
    EXTENDED            = 88,   //!< YUV 4:2:0/8  "Extended profile"

    FREXT_CAVLC444      = 44,   //!< YUV 4:4:4/14 "CAVLC 4:4:4"
    FREXT_HiP           = 100,  //!< YUV 4:2:0/8  "High"
    FREXT_Hi10P         = 110,  //!< YUV 4:2:0/10 "High 10"
    FREXT_Hi422         = 122,  //!< YUV 4:2:2/10 "High 4:2:2"
    FREXT_Hi444         = 244,  //!< YUV 4:4:4/14 "High 4:4:4"

    MVC_HIGH            = 118,  //!< YUV 4:2:0/8  "Multiview High"
    MVC_FIELDHIGH       = 119,  //!< YUV 4:2:0/8  "Multiview Field High"
    STEREO_HIGH         = 128   //!< YUV 4:2:0/8  "Stereo High"
} H264Profiles_e;

/*
//! AVC Level IDC definitions
//! Examples for high resolution @ frame rate (max stored frames)
typedef enum LevelIDC_e
{
    LEVEL_1     = ,     //!< 128×96@30.9 (8)
    LEVEL_1B    = ,     //!< 128×96@30.9 (8)
    LEVEL_11    = 9,    //!< 176×144@30.3 (9)
    LEVEL_12    = ,     //!< 320×240@20.0 (7)
    LEVEL_13    = ,     //!< 352×288@30.0 (6)
    LEVEL_2     = ,     //!< 352×288@30.0 (6)
    LEVEL_21    = ,     //!< 352×480@30.0 (7)
    LEVEL_22    = ,     //!< 352×480@30.7 (10)
    LEVEL_3     = ,     //!< 720×480@30.0 (6)
    LEVEL_31    = ,     //!< 1280×720@30.0 (5)
    LEVEL_32    = ,     //!< 1,280×720@60.0 (5)
    LEVEL_4     = ,     //!< 1,280×720@68.3 (9)
    LEVEL_41    = ,     //!< 1,280×720@68.3 (9)
    LEVEL_42    = ,     //!< 1,920×1,080@64.0 (4)
    LEVEL_5     = ,     //!< 1,920×1,080@72.3 (13)
    LEVEL_51    =       //!< 1,920×1,080@120.5 (16)
} LevelIDC_e;
*/

/* ************************************************************************** */

/*!
 * \struct hrd_t
 * \brief HRD - Hypothetical Reference Decoder.
 *
 * From 'ITU-T H.264' recommendation:
 * - E.1.2 HRD parameters syntax.
 * - E.2.2 HRD parameters semantics.
 */
typedef struct hrd_t
{
    unsigned int cpb_cnt_minus1;
    unsigned int bit_rate_scale;
    unsigned int cpb_size_scale;

    //for (SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++)
        unsigned int bit_rate_value_minus1[MAX_CPB];
        unsigned int cpb_size_value_minus1[MAX_CPB];
        unsigned int CpbSize[MAX_CPB];
        bool cbr_flag[MAX_CPB];

    unsigned int initial_cpb_removal_delay_length_minus1;
    unsigned int cpb_removal_delay_length_minus1;
    unsigned int dpb_output_delay_length_minus1;
    unsigned int time_offset_length;
} hrd_t;

/*!
 * \struct vui_t
 * \brief VUI - Video Usability Information.
 *
 * From 'ITU-T H.264' recommendation:
 * - Annex E.1.1 VUI parameters syntax.
 * - Annex E.2.1 VUI parameters semantics.
 */
typedef struct vui_t
{
    bool aspect_ratio_info_present_flag;
    //if (aspect_ratio_info_present_flag)
        unsigned int aspect_ratio_idc;
        //if (aspect_ratio_idc == Extended_SAR)
            unsigned int sar_width;
            unsigned int sar_height;

    bool overscan_info_present_flag;
    //if (overscan_info_present_flag)
        unsigned int overscan_appropriate_flag;

    bool video_signal_type_present_flag;
    //if (video_signal_type_present_flag)
        unsigned int video_format;
        bool video_full_range_flag;
        bool colour_description_present_flag;
        //if (colour_description_present_flag)
            unsigned int colour_primaries;
            unsigned int transfer_characteristics;
            unsigned int matrix_coefficients;

    bool chroma_loc_info_present_flag;
    //if (chroma_loc_info_present_flag)
        unsigned int chroma_sample_loc_type_top_field;
        unsigned int chroma_sample_loc_type_bottom_field;

    bool timing_info_present_flag;
    //if (timing_info_present_flag)
        unsigned int num_units_in_tick;
        unsigned int time_scale;
        bool fixed_frame_rate_flag;

    bool nal_hrd_parameters_present_flag;
    bool vcl_hrd_parameters_present_flag;

    //if (nal_hrd_parameters_present_flag)
        hrd_t *nal_hrd;
    //if (vcl_hrd_parameters_present_flag)
        hrd_t *vcl_hrd;
    //if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
        bool low_delay_hrd_flag;
    bool pic_struct_present_flag;
    bool bitstream_restriction_flag;

    //if (bitstream_restriction_flag)
        bool motion_vectors_over_pic_boundaries_flag;
        unsigned int max_bytes_per_pic_denom;
        unsigned int max_bits_per_mb_denom;
        unsigned int log2_max_mv_length_horizontal;
        unsigned int log2_max_mv_length_vertical;
        unsigned int num_reorder_frames;
        unsigned int max_dec_frame_buffering;
} vui_t;

/*!
 * \struct spse_t
 * \brief SPS E - Sequence Parameter Set Extension.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.1 Sequence parameter set extension RBSP syntax.
 * - 7.4.2.1 Sequence parameter set extension RBSP semantics.
 */
typedef struct spse_t
{
    unsigned int seq_parameter_set_id;
    unsigned int aux_format_idc;

    //if (aux_format_idc != 0)
        unsigned int bit_depth_aux_minus8;
        bool alpha_incr_flag;
        unsigned int alpha_opaque_value;
        unsigned int alpha_transparent_value;

    bool additional_extension_flag;
} spse_t;

/*!
 * \struct ScalingStruct_t
 * \brief The scaling list, can be contained in sps or pps.
 */
typedef struct ScalingStruct_t
{
    bool scaling_list_present_flag[12];
    int ScalingList4x4[6][16];
    int ScalingList8x8[6][64];
    int ScalingMatrix4x4[6][4][4]; // derived from ScalingList4x4
    int ScalingMatrix8x8[6][8][8]; // derived from ScalingList8x8
    int LevelScale4x4[3][6][4][4]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix4x4, normAdjust4x4
    int LevelScale8x8[3][6][8][8]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix8x8, normAdjust8x8

    bool UseDefaultScalingMatrix4x4Flag[6];
    bool UseDefaultScalingMatrix8x8Flag[6];
} ScalingStruct_t;

/*!
 * \struct sps_t
 * \brief SPS - Sequence Parameter Set.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.1.1 Sequence parameter set data syntax.
 * - 7.4.2.1.1 Sequence parameter set data semantics.
 */
typedef struct sps_t
{
    unsigned int profile_idc;
    unsigned int level_idc;
    bool constraint_setX_flag[6];
    unsigned int seq_parameter_set_id;

    //if (profile_idc == x)
        unsigned int chroma_format_idc;
        unsigned int ChromaArrayType; // derived from chroma_format_idc
        unsigned int SubWidthC; // derived from chroma_format_idc
        unsigned int SubHeightC; // derived from chroma_format_idc
        unsigned int MbWidthC; // derived from SubWidthC
        unsigned int MbHeightC; // derived from  SubHeightC

        //if (chroma_format_idc == 3)
            bool separate_colour_plane_flag;

        unsigned int bit_depth_luma_minus8;
        unsigned int BitDepthY; // derived from bit_depth_luma_minus8;
        unsigned int QpBdOffsetY; // derived from bit_depth_luma_minus8;
        unsigned int bit_depth_chroma_minus8;
        unsigned int BitDepthC; // derived from bit_depth_chroma_minus8;
        unsigned int QpBdOffsetC; // derived from bit_depth_chroma_minus8;
        unsigned int RawMbBits; // derived from BitDepthY, MbWidthC, MbHeightC, BitDepth

        bool qpprime_y_zero_transform_bypass_flag;

        bool seq_scaling_matrix_present_flag;
        //if (seq_scaling_matrix_present_flag)
            bool seq_scaling_list_present_flag[12];
            int ScalingList4x4[6][16];
            int ScalingList8x8[6][64];
            int ScalingMatrix4x4[6][4][4]; // derived from ScalingList4x4
            int ScalingMatrix8x8[6][8][8]; // derived from ScalingList8x8
            int LevelScale4x4[3][6][4][4]; // derived from ScalingMatrix4x4, normAdjust4x4
            int LevelScale8x8[3][6][8][8]; // derived from ScalingMatrix8x8, normAdjust8x8
            bool UseDefaultScalingMatrix4x4Flag[6];
            bool UseDefaultScalingMatrix8x8Flag[6];

    unsigned int log2_max_frame_num_minus4;
    unsigned int MaxFrameNum; // derived from log2_max_frame_num_minus4
    unsigned int pic_order_cnt_type;
    //if (pic_order_cnt_type == 0)
        unsigned int log2_max_pic_order_cnt_lsb_minus4;
        unsigned int MaxPicOrderCntLsb; // derived from log2_max_pic_order_cnt_lsb_minus4
    //else if (pic_order_cnt_type == 1)
        bool delta_pic_order_always_zero_flag;
        int offset_for_non_ref_pic;
        int offset_for_top_to_bottom_field;
        unsigned int num_ref_frames_in_pic_order_cnt_cycle;
        //for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
            int offset_for_ref_frame[256];

    unsigned int max_num_ref_frames;
    bool gaps_in_frame_num_value_allowed_flag;
    unsigned int pic_width_in_mbs_minus1;
        unsigned int PicWidthInMbs; // derived from pic_width_in_mbs_minus1
        unsigned int PicWidthInSamplesL; // derived from PicWidthInMbs
        unsigned int PicWidthInSamplesC; // derived from PicWidthInMbs, MbWidthC
    unsigned int pic_height_in_map_units_minus1;
        unsigned int PicHeightInMapUnits; // derived from pic_height_in_map_units_minus1
        unsigned int PicSizeInMapUnits; // derived from PicWidthInMbs, PicHeightInMapUnits
    bool frame_mbs_only_flag;
        unsigned int FrameHeightInMbs; // derived from frame_mbs_only_flag, PicHeightInMapUnits
    //if (!frame_mbs_only_flag)
        bool mb_adaptive_frame_field_flag;
    bool direct_8x8_inference_flag;
    bool frame_cropping_flag;
    //if (frame_cropping_flag)
        unsigned int frame_crop_left_offset;
        unsigned int frame_crop_right_offset;
        unsigned int frame_crop_top_offset;
        unsigned int frame_crop_bottom_offset;
        unsigned int CropUnitX; // derived from SubWidthC, frame_mbs_only_flag
        unsigned int CropUnitY; // derived from SubHeightC, frame_mbs_only_flag
    bool vui_parameters_present_flag;
    vui_t *vui;
} sps_t;

/*!
 * \struct pps_t
 * \brief PPS - Picture Parameter Set.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.2 Picture parameter set RBSP syntax.
 * - 7.4.2.2 Picture parameter set RBSP semantics.
 */
typedef struct pps_t
{
    unsigned int pic_parameter_set_id;
    unsigned int seq_parameter_set_id;
    bool entropy_coding_mode_flag;
    bool bottom_field_pic_order_in_frame_present_flag;
    unsigned int num_slice_groups_minus1;
    //if (num_slice_groups_minus1 > 0)
        unsigned int slice_group_map_type;
        //if (slice_group_map_type == 0)
            //for (iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
                unsigned int run_length_minus1[8];
        //else if (slice_group_map_type == 2)
            //for (iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++)
                unsigned int top_left[8];
                unsigned int bottom_right[8];
    //else if (slice_group_map_type == 3 || 4 || 5)
        bool slice_group_change_direction_flag;
        unsigned int slice_group_change_rate_minus1;
        //else if (slice_group_map_type == 6)
            unsigned int pic_size_in_map_units_minus1;
            //for (i = 0; i <= pic_size_in_map_units_minus1; i++) // value of pic_size_in_map_units_minus1, see p76
                unsigned int slice_group_id[MAX_SLICES];

    unsigned int num_ref_idx_l0_default_active_minus1;
    unsigned int num_ref_idx_l1_default_active_minus1;
    bool weighted_pred_flag;
    unsigned int weighted_bipred_idc;
    int pic_init_qp_minus26;
    int pic_init_qs_minus26;
    int chroma_qp_index_offset;
    bool deblocking_filter_control_present_flag;
    bool constrained_intra_pred_flag;
    bool redundant_pic_cnt_present_flag;
    //if (h264_more_rbsp_data())
        bool transform_8x8_mode_flag;
        bool pic_scaling_matrix_present_flag;
        //if (pic_scaling_matrix_present_flag)
            bool pic_scaling_list_present_flag[12];
            //int ScalingList4x4[6][16];
            //int ScalingList8x8[6][64];
            //int ScalingMatrix4x4[6][4][4]; // derived from ScalingList4x4
            //int ScalingMatrix8x8[6][8][8]; // derived from ScalingList8x8
            //int LevelScale4x4[3][6][4][4]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix4x4, normAdjust4x4
            //int LevelScale8x8[3][6][8][8]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix8x8, normAdjust8x8
            //bool UseDefaultScalingMatrix4x4Flag[6];
            //bool UseDefaultScalingMatrix8x8Flag[6];

        int second_chroma_qp_index_offset;
} pps_t;

/*!
 * \struct sei_t
 * \brief SEI - Supplemental Enhancement Information.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.3 Supplemental Enhancement Information message syntax.
 * - Annex D.1 SEI payload syntax.
 * - Annex D.2 SEI payload semantics.
 */
typedef struct sei_t
{
    unsigned int payloadType;
    unsigned int payloadSize;
} sei_t;

/*!
 * \struct aud_t
 * \brief AUD - Access Unit Delimiter.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.4 Access unit delimiter RBSP syntax.
 * - 7.4.2.4 Access unit delimiter RBSP semantics.
 */
typedef struct aud_t
{
    unsigned int primary_pic_type;
} aud_t;

/* ************************************************************************** */
#endif // H264_PARAMETER_SET_STRUCT_H
