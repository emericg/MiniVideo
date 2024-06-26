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
 * \file      h264_parameterset_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_PARAMETER_SET_STRUCT_H
#define H264_PARAMETER_SET_STRUCT_H

#include <cstdint>

/* ************************************************************************** */

#define MAX_SPS     32
#define MAX_PPS     256
#define MAX_MMCO    66
#define MAX_CPB     32
#define MAX_SLICES  1

/* ************************************************************************** */

/*!
 * \struct h264_hrd_t
 * \brief HRD - Hypothetical Reference Decoder.
 *
 * From 'ITU-T H.264' recommendation:
 * - E.1.2 HRD parameters syntax.
 * - E.2.2 HRD parameters semantics.
 */
typedef struct h264_hrd_t
{
    unsigned cpb_cnt_minus1;
    uint8_t bit_rate_scale;
    uint8_t cpb_size_scale;

    //for (SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++)
        unsigned bit_rate_value_minus1[MAX_CPB];
        unsigned cpb_size_value_minus1[MAX_CPB];
        unsigned CpbSize[MAX_CPB];
        bool cbr_flag[MAX_CPB];

    uint8_t initial_cpb_removal_delay_length_minus1;
    uint8_t cpb_removal_delay_length_minus1;
    uint8_t dpb_output_delay_length_minus1;
    uint8_t time_offset_length;

} h264_hrd_t;

/*!
 * \struct h264_vui_t
 * \brief VUI - Video Usability Information.
 *
 * From 'ITU-T H.264' recommendation:
 * - Annex E.1.1 VUI parameters syntax.
 * - Annex E.2.1 VUI parameters semantics.
 */
typedef struct h264_vui_t
{
    bool aspect_ratio_info_present_flag;
    //if (aspect_ratio_info_present_flag)
        uint8_t aspect_ratio_idc;
        //if (aspect_ratio_idc == Extended_SAR)
            uint16_t sar_width;
            uint16_t sar_height;

    bool overscan_info_present_flag;
    //if (overscan_info_present_flag)
        bool overscan_appropriate_flag;

    bool video_signal_type_present_flag;
    //if (video_signal_type_present_flag)
        uint8_t video_format;
        bool video_full_range_flag;
        bool colour_description_present_flag;
        //if (colour_description_present_flag)
            uint8_t colour_primaries;
            uint8_t transfer_characteristics;
            uint8_t matrix_coefficients;

    bool chroma_loc_info_present_flag;
    //if (chroma_loc_info_present_flag)
        unsigned chroma_sample_loc_type_top_field;
        unsigned chroma_sample_loc_type_bottom_field;

    bool timing_info_present_flag;
    //if (timing_info_present_flag)
        unsigned num_units_in_tick;
        unsigned time_scale;
        bool fixed_frame_rate_flag;

    bool nal_hrd_parameters_present_flag;
    bool vcl_hrd_parameters_present_flag;

    //if (nal_hrd_parameters_present_flag)
        h264_hrd_t *nal_hrd;
    //if (vcl_hrd_parameters_present_flag)
        h264_hrd_t *vcl_hrd;
    //if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
        bool low_delay_hrd_flag;
    bool pic_struct_present_flag;
    bool bitstream_restriction_flag;

    //if (bitstream_restriction_flag)
        bool motion_vectors_over_pic_boundaries_flag;
        unsigned max_bytes_per_pic_denom;
        unsigned max_bits_per_mb_denom;
        unsigned log2_max_mv_length_horizontal;
        unsigned log2_max_mv_length_vertical;
        unsigned num_reorder_frames;
        unsigned max_dec_frame_buffering;

} h264_vui_t;

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
    unsigned seq_parameter_set_id;
    unsigned aux_format_idc;

    //if (aux_format_idc != 0)
        unsigned bit_depth_aux_minus8;
        bool alpha_incr_flag;
        unsigned alpha_opaque_value;
        unsigned alpha_transparent_value;

    bool additional_extension_flag;

} spse_t;

/*!
 * \struct h264_ScalingStruct_t
 * \brief The scaling list, can be contained in sps or pps.
 */
typedef struct h264_ScalingStruct_t
{
    bool seq_scaling_list_present_flag[12];

    int ScalingList4x4[6][16];
    int ScalingList8x8[6][64];
    int ScalingMatrix4x4[6][4][4]; // derived from ScalingList4x4
    int ScalingMatrix8x8[6][8][8]; // derived from ScalingList8x8

    int LevelScale4x4[3][6][4][4]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix4x4, normAdjust4x4
    int LevelScale8x8[3][6][8][8]; // [YCbCr][qP%6][i][j] derived from ScalingMatrix8x8, normAdjust8x8

    bool UseDefaultScalingMatrix4x4Flag[6];
    bool UseDefaultScalingMatrix8x8Flag[6];

} h264_ScalingStruct_t;

/*!
 * \struct h264_sps_t
 * \brief SPS - Sequence Parameter Set.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.1.1 Sequence parameter set data syntax.
 * - 7.4.2.1.1 Sequence parameter set data semantics.
 */
typedef struct h264_sps_t
{
    unsigned profile_idc;
    unsigned level_idc;
    bool constraint_setX_flag[6];
    unsigned seq_parameter_set_id;

    //if (profile_idc == x)
        unsigned chroma_format_idc;
        unsigned ChromaArrayType; // derived from chroma_format_idc
        unsigned SubWidthC; // derived from chroma_format_idc
        unsigned SubHeightC; // derived from chroma_format_idc
        unsigned MbWidthC; // derived from SubWidthC
        unsigned MbHeightC; // derived from  SubHeightC

        //if (chroma_format_idc == 3)
            bool separate_colour_plane_flag;

        unsigned bit_depth_luma_minus8;
        unsigned BitDepthY; // derived from bit_depth_luma_minus8;
        unsigned QpBdOffsetY; // derived from bit_depth_luma_minus8;
        unsigned bit_depth_chroma_minus8;
        unsigned BitDepthC; // derived from bit_depth_chroma_minus8;
        unsigned QpBdOffsetC; // derived from bit_depth_chroma_minus8;
        unsigned RawMbBits; // derived from BitDepthY, MbWidthC, MbHeightC, BitDepth

        bool qpprime_y_zero_transform_bypass_flag;

        bool seq_scaling_matrix_present_flag;
        //if (seq_scaling_matrix_present_flag)
            bool seq_scaling_list_present_flag[12];

            bool UseDefaultScalingMatrix4x4Flag[6];
            int ScalingList4x4[6][16];
            int ScalingMatrix4x4[6][4][4]; // derived from ScalingList4x4

            bool UseDefaultScalingMatrix8x8Flag[6];
            int ScalingList8x8[6][64];
            int ScalingMatrix8x8[6][8][8]; // derived from ScalingList8x8

            int LevelScale4x4[3][6][4][4]; // derived from ScalingMatrix4x4, normAdjust4x4
            int LevelScale8x8[3][6][8][8]; // derived from ScalingMatrix8x8, normAdjust8x8

    unsigned log2_max_frame_num_minus4;
    unsigned MaxFrameNum; // derived from log2_max_frame_num_minus4
    unsigned pic_order_cnt_type;
    //if (pic_order_cnt_type == 0)
        unsigned log2_max_pic_order_cnt_lsb_minus4;
        unsigned MaxPicOrderCntLsb; // derived from log2_max_pic_order_cnt_lsb_minus4
    //else if (pic_order_cnt_type == 1)
        bool delta_pic_order_always_zero_flag;
        int offset_for_non_ref_pic;
        int offset_for_top_to_bottom_field;
        unsigned num_ref_frames_in_pic_order_cnt_cycle;
        //for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
            int offset_for_ref_frame[256];

    unsigned max_num_ref_frames;
    bool gaps_in_frame_num_value_allowed_flag;
    unsigned pic_width_in_mbs_minus1;
        unsigned PicWidthInMbs; // derived from pic_width_in_mbs_minus1
        unsigned PicWidthInSamplesL; // derived from PicWidthInMbs
        unsigned PicWidthInSamplesC; // derived from PicWidthInMbs, MbWidthC
    unsigned pic_height_in_map_units_minus1;
        unsigned PicHeightInMapUnits; // derived from pic_height_in_map_units_minus1
        unsigned PicSizeInMapUnits; // derived from PicWidthInMbs, PicHeightInMapUnits
    bool frame_mbs_only_flag;
        unsigned FrameHeightInMbs; // derived from frame_mbs_only_flag, PicHeightInMapUnits
    //if (!frame_mbs_only_flag)
        bool mb_adaptive_frame_field_flag;
    bool direct_8x8_inference_flag;
    bool frame_cropping_flag;
    //if (frame_cropping_flag)
        unsigned frame_crop_left_offset;
        unsigned frame_crop_right_offset;
        unsigned frame_crop_top_offset;
        unsigned frame_crop_bottom_offset;
        unsigned CropUnitX; // derived from SubWidthC, frame_mbs_only_flag
        unsigned CropUnitY; // derived from SubHeightC, frame_mbs_only_flag

    bool vui_parameters_present_flag;
    h264_vui_t *vui;

} h264_sps_t;

/*!
 * \struct h264_pps_t
 * \brief PPS - Picture Parameter Set.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.2 Picture parameter set RBSP syntax.
 * - 7.4.2.2 Picture parameter set RBSP semantics.
 */
typedef struct h264_pps_t
{
    unsigned pic_parameter_set_id;
    unsigned seq_parameter_set_id;
    bool entropy_coding_mode_flag;
    bool bottom_field_pic_order_in_frame_present_flag;
    unsigned num_slice_groups_minus1;
    //if (num_slice_groups_minus1 > 0)
        unsigned slice_group_map_type;
        //if (slice_group_map_type == 0)
            //for (iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
                unsigned run_length_minus1[8];
        //else if (slice_group_map_type == 2)
            //for (iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++)
                unsigned top_left[8];
                unsigned bottom_right[8];
    //else if (slice_group_map_type == 3 || 4 || 5)
        bool slice_group_change_direction_flag;
        unsigned slice_group_change_rate_minus1;
        //else if (slice_group_map_type == 6)
            unsigned pic_size_in_map_units_minus1;
            //for (i = 0; i <= pic_size_in_map_units_minus1; i++) // value of pic_size_in_map_units_minus1, see p76
                unsigned slice_group_id[MAX_SLICES];

    unsigned num_ref_idx_l0_default_active_minus1;
    unsigned num_ref_idx_l1_default_active_minus1;
    bool weighted_pred_flag;
    uint8_t weighted_bipred_idc;
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
            // TODO

        int second_chroma_qp_index_offset;

} h264_pps_t;

/*!
 * \struct h264_aud_t
 * \brief AUD - Access Unit Delimiter.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.4 Access unit delimiter RBSP syntax.
 * - 7.4.2.4 Access unit delimiter RBSP semantics.
 */
typedef struct h264_aud_t
{
    uint8_t primary_pic_type;

} h264_aud_t;

/* ************************************************************************** */

/*!
 * \struct h264_sei_t
 * \brief SEI - Supplemental Enhancement Information.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.2.3 Supplemental Enhancement Information message syntax.
 * - Annex D.1 SEI payload syntax.
 * - Annex D.2 SEI payload semantics.
 */
typedef struct h264_sei_t
{
    // D.1.2 // Buffering period SEI message syntax
    uint8_t eq_parameter_set_id;
    //if(NalHrdBpPresentFlag || VclHrdBpPresentFlag)
        //for( SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++ )
        uint8_t initial_cpb_removal_delay[MAX_CPB];
        uint8_t initial_cpb_removal_delay_offset[MAX_CPB];

    // D.1.3 // Picture timing SEI message syntax
    // D.1.4 // Pan-scan rectangle SEI message syntax
    // D.1.5 // Filler payload SEI message syntax

    // D.1.6 // User data registered by Rec. ITU-T T.35 SEI message syntax
    uint8_t itu_t_t35_country_code;
    uint8_t itu_t_t35_country_code_extension_byte;
    char *itu_t_t35_payload;

    // D.1.7 // User data unregistered SEI message syntax
    char uuid_iso_iec_11578[16];
    char *user_data_payload;

    // D.1.8 // Recovery point SEI message syntax
    uint32_t recovery_frame_cnt;
    uint8_t exact_match_flag;
    uint8_t broken_link_flag;
    uint8_t changing_slice_group_idc;

    // D.1.9 // Decoded reference picture marking repetition SEI message syntax
    // D.1.10 // Spare picture SEI message syntax
    // D.1.11 // Scene information SEI message syntax
    // D.1.12 // Sub-sequence information SEI message syntax
    // D.1.13 // Sub-sequence layer characteristics SEI message syntax
    // D.1.14 // Sub-sequence characteristics SEI message syntax

    // D.1.15 // Full-frame freeze SEI message syntax
    unsigned full_frame_freeze_repetition_period;

    // D.1.16 // Full-frame freeze release SEI message syntax

    // D.1.17 // Full-frame snapshot SEI message syntax
    uint32_t snapshot_id;

    // D.1.18 // Progressive refinement segment start SEI message syntax
    uint32_t progressive_refinement_id_start;
    uint32_t num_refinement_steps_minus1;

    // D.1.19 // Progressive refinement segment end SEI message syntax
    uint32_t progressive_refinement_id_end;

    // D.1.20 // Motion-constrained slice group set SEI message syntax
    // D.1.21 // Film grain characteristics SEI message syntax

    // D.1.22 // Deblocking filter display preference SEI message syntax
    bool deblocking_display_preference_cancel_flag;
        //if (!deblocking_display_preference_cancel_flag)
        bool display_prior_to_deblocking_preferred_flag;
        bool dec_frame_buffering_constraint_flag;
        uint32_t deblocking_display_preference_repetition_period;

    // D.1.23 // Stereo video information SEI message syntax
    bool field_views_flag;
    // if (field_views_flag)
        bool top_field_is_left_view_flag;
    //else
        bool current_frame_is_left_view_flag;
        bool next_frame_is_second_view_flag;
    bool left_view_self_contained_flag;
    bool right_view_self_contained_flag;

    // D.1.24 // Post-filter hint SEI message syntax
    uint32_t filter_hint_size_y;
    uint32_t filter_hint_size_x;
    uint8_t filter_hint_type;
    //int filter_hint[3][cy][cx];
    bool additional_extension_flag;

    // D.1.25 // Tone mapping information SEI message syntax
    // D.1.26 // Frame packing arrangement SEI message syntax

    // D.1.27 // Display orientation SEI message syntax
    bool display_orientation_cancel_flag;
    //if (!display_orientation_cancel_flag)
        bool hor_flip;
        bool ver_flip;
        uint16_t anticlockwise_rotation;
        uint32_t display_orientation_repetition_period;
        bool display_orientation_extension_flag;

    // D.1.28 // Green metadata SEI message syntax
    // ISO/IEC 23001-11 (Green metadata)

    // D.1.29 // Mastering display colour volume SEI message syntax
    uint16_t display_primaries_x[3];
    uint16_t display_primaries_y[3];
    uint16_t white_point_x;
    uint16_t white_point_y;
    uint32_t max_display_mastering_luminance;
    uint32_t min_display_mastering_luminance;

    // D.1.30 // Colour remapping information SEI message syntax

    // D.1.31 // Content light level information SEI message syntax
    uint16_t max_content_light_level;
    uint16_t max_pic_average_light_level;

    // D.1.32 // Alternative transfer characteristics SEI message syntax
    uint8_t preferred_transfer_characteristics;

    // D.1.33 // Content colour volume SEI message syntax
    bool ccv_cancel_flag;
        //if (!ccv_cancel_flag)
        bool ccv_persistence_flag;
        bool ccv_primaries_present_flag;
        bool ccv_min_luminance_value_present_flag;
        bool ccv_max_luminance_value_present_flag;
        bool ccv_avg_luminance_value_present_flag;
        //if (ccv_primaries_present_flag)
            int32_t ccv_primaries_x[3];
            int32_t ccv_primaries_y[3];
        //if (ccv_min_luminance_value_present_flag)
            uint32_t ccv_min_luminance_value;
        //if (ccv_max_luminance_value_present_flag)
            uint32_t ccv_max_luminance_value;
        //if (ccv_avg_luminance_value_present_flag)
            uint32_t ccv_avg_luminance_value;

    // D.1.34 // Ambient viewing environment SEI message syntax
    uint32_t ambient_illuminance;
    uint16_t ambient_light_x;
    uint16_t ambient_light_y;

    // D.1.35 // Syntax of omnidirectional video specific SEI messages
    // D.1.36 // SEI manifest SEI message syntax
    // D.1.37 // SEI prefix indication SEI message syntax
    // D.1.38 // Annotated regions SEI message syntax
    // D.1.39 // Shutter interval information SEI message syntax
    // D.1.40 // Reserved SEI message syntax

} h264_sei_t;

/* ************************************************************************** */
#endif // H264_PARAMETER_SET_STRUCT_H
