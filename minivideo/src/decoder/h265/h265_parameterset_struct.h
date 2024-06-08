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
 * \file      h265_parameterset_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H265_PARAMETER_SET_STRUCT_H
#define H265_PARAMETER_SET_STRUCT_H
/* ************************************************************************** */

#include <cstdint>

#define H265_MAX_VPS     32
#define H265_MAX_SPS     32
#define H265_MAX_PPS     256

/* ************************************************************************** */

typedef struct h265_ptl_sublayer_t
{
    bool profile_present_flag;
    bool level_present_flag;

    //if (profile_present_flag)
    uint8_t profile_space;
    bool tier_flag;
    uint8_t profile_idc;
    //for (j = 0; j < 32; j++)
        bool profile_compatibility_flag[32];
    bool progressive_source_flag;
    bool interlaced_source_flag;
    bool non_packed_constraint_flag;
    bool frame_only_constraint_flag;

        bool max_14bit_constraint_flag;
        bool max_12bit_constraint_flag;
        bool max_10bit_constraint_flag;
        bool max_8bit_constraint_flag;
        bool max_422chroma_constraint_flag;
        bool max_420chroma_constraint_flag;
        bool max_monochrome_constraint_flag;
        bool intra_constraint_flag;
        bool one_picture_only_constraint_flag;
        bool lower_bit_rate_constraint_flag;

        bool inbld_flag;

    uint8_t level_idc;

} h265_ptl_sublayer_t;

/*!
 * \struct h265_ptl_t
 * \brief Profile, tier and level.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.3 Profile, tier and level syntax.
 * -
 */
typedef struct h265_ptl_t
{
    uint8_t general_profile_space;
    bool general_tier_flag;
    uint8_t general_profile_idc;

    //for (int j = 0; j < 32; j++)
        bool general_profile_compatibility_flag[32];

    bool general_progressive_source_flag;
    bool general_interlaced_source_flag;
    bool general_non_packed_constraint_flag;
    bool general_frame_only_constraint_flag;

        bool general_max_14bit_constraint_flag;
        bool general_max_12bit_constraint_flag;
        bool general_max_10bit_constraint_flag;
        bool general_max_8bit_constraint_flag;
        bool general_max_422chroma_constraint_flag;
        bool general_max_420chroma_constraint_flag;
        bool general_max_monochrome_constraint_flag;
        bool general_intra_constraint_flag;
        bool general_one_picture_only_constraint_flag;
        bool general_lower_bit_rate_constraint_flag;

        bool general_inbld_flag;

    uint8_t general_level_idc;

    //for (i = 0; i < maxNumSubLayersMinus1; i++)
        h265_ptl_sublayer_t *sub_layer;

} h265_ptl_t;

/*!
 * \struct h265_scalinglist_t
 * \brief Scaling lists.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.4 Scaling list data syntax.
 * -
 */
typedef struct h265_scalinglist_t
{
    //

} h265_scalinglist_t;

/*!
 * \struct h265_hrd_t
 * \brief HRD - Hypotetical Reference Decoder.
 *
 * From 'ITU-T H.265' recommendation:
 * - E.2.2 HRD parameters syntax.
 * - E.3.2 HRD parameters semantics.
 */
typedef struct h265_hrd_t
{
    //if (commonInfPresentFlag)
        bool nal_hrd_parameters_present_flag;
        bool vcl_hrd_parameters_present_flag;

        //if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
        bool sub_pic_hrd_params_present_flag;
            //if (sub_pic_hrd_params_present_flag)
                uint8_t tick_divisor_minus2;
                uint8_t du_cpb_removal_delay_increment_length_minus1;
                uint8_t sub_pic_cpb_params_in_pic_timing_sei_flag;
                uint8_t dpb_output_delay_du_length_minus1;
            uint8_t bit_rate_scale;
            uint8_t cpb_size_scale;
            //if (sub_pic_hrd_params_present_flag)
                uint8_t cpb_size_du_scale;
            uint8_t initial_cpb_removal_delay_length_minus1;
            uint8_t au_cpb_removal_delay_length_minus1;
            uint8_t dpb_output_delay_length_minus1;
} h265_hrd_t;

/*!
 * \struct h265_sublayer_hrd_t
 * \brief Sub layer HRD.
 *
 * From 'ITU-T H.265' recommendation:
 * - E.2.3 Sub-layer HRD parameters syntax.
 * - E.3.2 HRD parameters semantics.
 */
typedef struct h265_sublayer_hrd_t
{/*
    for( i = 0; i < CpbCnt; i++ ) {
        bit_rate_value_minus1[ i ]
            cpb_size_value_minus1[ i ]
            if( sub_pic_hrd_params_present_flag ) {
                cpb_size_du_value_minus1[ i ]
                bit_rate_du_value_minus1[ i ]
            }
        cbr_flag[ i ]
    }
*/
} h265_sublayer_hrd_t;


/*!
 * \struct h265_vui_t
 * \brief VUI - Video Usability Information.
 *
 * From 'ITU-T H.265' recommendation:
 * - E.2.1 VUI parameters syntax.
 * - E.3.1 VUI parameters semantics.
 */
typedef struct h265_vui_t
{
    bool aspect_ratio_info_present_flag;
    //if (aspect_ratio_info_present_flag)
        uint8_t aspect_ratio_idc;
        // if (aspect_ratio_idc = = EXTENDED_SAR)
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
            uint8_t matrix_coeffs;

    bool chroma_loc_info_present_flag;
    //if (chroma_loc_info_present_flag)
        uint32_t chroma_sample_loc_type_top_field;
        uint32_t chroma_sample_loc_type_bottom_field;

    bool neutral_chroma_indication_flag;
    bool field_seq_flag;
    bool frame_field_info_present_flag;
    bool default_display_window_flag;
    //if (default_display_window_flag)
        uint32_t def_disp_win_left_offset;
        uint32_t def_disp_win_right_offset;
        uint32_t def_disp_win_top_offset;
        uint32_t def_disp_win_bottom_offset;

    bool vui_timing_info_present_flag;
    //if (vui_timing_info_present_flag)
        uint32_t vui_num_units_in_tick;
        uint32_t vui_time_scale;
        bool vui_poc_proportional_to_timing_flag;
        //if (vui_poc_proportional_to_timing_flag)
            uint32_t vui_num_ticks_poc_diff_one_minus1;
        bool vui_hrd_parameters_present_flag;
        //if (vui_hrd_parameters_present_flag)
            //hrd_parameters(1, sps_max_sub_layers_minus1)
            h265_hrd_t hrd;

    bool bitstream_restriction_flag;
    //if (bitstream_restriction_flag)
        bool tiles_fixed_structure_flag;
        bool motion_vectors_over_pic_boundaries_flag;
        bool restricted_ref_pic_lists_flag;
        uint32_t min_spatial_segmentation_idc;
        uint32_t max_bytes_per_pic_denom;
        uint32_t max_bits_per_min_cu_denom;
        uint32_t log2_max_mv_length_horizontal;
        uint32_t log2_max_mv_length_vertical;

} h265_vui_t;

/*!
 * \struct h265_sps_range_extension_t
 * \brief SPS range extension.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.2.2 Sequence parameter set range extension syntax
 * -
 */
typedef struct h265_sps_range_extension_t
{
    bool transform_skip_rotation_enabled_flag;
    bool transform_skip_context_enabled_flag;
    bool implicit_rdpcm_enabled_flag;
    bool explicit_rdpcm_enabled_flag;
    bool extended_precision_processing_flag;
    bool intra_smoothing_disabled_flag;
    bool high_precision_offsets_enabled_flag;
    bool persistent_rice_adaptation_enabled_flag;
    bool cabac_bypass_alignment_enabled_flag;

} h265_sps_range_extension_t;

/*!
 * \struct h265_sps_scc_extension_t
 * \brief SPS screen content coding extension.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.2.3 Sequence parameter set screen content coding extension syntax
 * -
 */
typedef struct h265_sps_scc_extension_t
{
    bool sps_curr_pic_ref_enabled_flag;
    bool palette_mode_enabled_flag;
    //if (palette_mode_enabled_flag)
        uint32_t palette_max_size;
        uint32_t delta_palette_max_predictor_size;

        bool sps_palette_predictor_initializers_present_flag;
        //if (sps_palette_predictor_initializers_present_flag)
        uint32_t sps_num_palette_predictor_initializers_minus1;
            //numComps = (chroma_format_idc == 0) ? 1 : 3;
            //for (comp = 0; comp < numComps; comp++)
                //for (i = 0; i <= sps_num_palette_predictor_initializers_minus1; i++)
                    //uint32_t sps_palette_predictor_initializer[ comp ][ i ];

    uint8_t motion_vector_resolution_control_idc;
    bool intra_boundary_filtering_disabled_flag;

} h265_sps_scc_extension_t;

/* ************************************************************************** */

/*!
 * \struct h265_vps_t
 * \brief VPS - Video Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.1 Video parameter set syntax.
 * - 7.4.3.1 Video parameter set semantics.
 */
typedef struct h265_vps_t
{
    uint8_t vps_video_parameter_set_id;
    bool vps_base_layer_internal_flag;
    bool vps_base_layer_available_flag;
    uint8_t vps_max_layers_minus1;
    uint8_t vps_max_sub_layers_minus1;
    bool vps_temporal_id_nesting_flag;

    h265_ptl_t ptl;

    bool vps_sub_layer_ordering_info_present_flag;

    //for (i = (vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1);
    //     i <= vps_max_sub_layers_minus1; i++)
        uint8_t vps_max_dec_pic_buffering_minus1[16];
        uint8_t vps_max_num_reorder_pics[16];
        uint8_t vps_max_latency_increase_plus1[16];

    uint8_t vps_max_layer_id;
    uint8_t vps_num_layer_sets_minus1;
    //for (i = 1; i <= vps_num_layer_sets_minus1; i++)
        //for (j = 0; j <= vps_max_layer_id; j++)
        bool **layer_id_included_flag;

    bool vps_timing_info_present_flag;
    //if (vps_timing_info_present_flag)
        uint32_t vps_num_units_in_tick;
        uint32_t vps_time_scale;
        bool vps_poc_proportional_to_timing_flag;
        //if (vps_poc_proportional_to_timing_flag)
            uint32_t vps_num_ticks_poc_diff_one_minus1;
        uint32_t vps_num_hrd_parameters;
        //for (i = 0; i < vps_num_hrd_parameters; i++)
            uint32_t *hrd_layer_set_idx;
            //if (i > 0)
                bool *cprms_present_flag;

            //hrd_parameters( cprms_present_flag[ i ], vps_max_sub_layers_minus1 )
            h265_hrd_t *hrd;

    bool vps_extension_flag;
    //if (vps_extension_flag)
        //while (more_rbsp_data())
            bool vps_extension_data_flag;

} h265_vps_t;

/*!
 * \struct h265_sps_t
 * \brief SPS - Sequence Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.2 Sequence parameter set syntax.
 * - 7.4.3.2 Sequence parameter set semantics.
 */
typedef struct h265_sps_t
{
    uint8_t sps_video_parameter_set_id;
    uint8_t sps_max_sub_layers_minus1;
    bool sps_temporal_id_nesting_flag;

    //profile_tier_level(1, sps_max_sub_layers_minus1)
    h265_ptl_t ptl;

    uint32_t sps_seq_parameter_set_id;
    uint32_t chroma_format_idc;

    //if (chroma_format_idc == 3)
        bool separate_colour_plane_flag;

    uint32_t pic_width_in_luma_samples;
    uint32_t pic_height_in_luma_samples;

    bool conformance_window_flag;
    //if (conformance_window_flag)
        uint32_t conf_win_left_offset;
        uint32_t conf_win_right_offset;
        uint32_t conf_win_top_offset;
        uint32_t conf_win_bottom_offset;

    uint32_t bit_depth_luma_minus8;
    uint32_t bit_depth_chroma_minus8;
    uint32_t log2_max_pic_order_cnt_lsb_minus4;

    bool sps_sub_layer_ordering_info_present_flag;

    //for(i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
    //    i <= sps_max_sub_layers_minus1; i++)
        uint32_t *sps_max_dec_pic_buffering_minus1;
        uint32_t *sps_max_num_reorder_pics;
        uint32_t *sps_max_latency_increase_plus1;

    uint32_t log2_min_luma_coding_block_size_minus3;
    uint32_t log2_diff_max_min_luma_coding_block_size;
    uint32_t log2_min_luma_transform_block_size_minus2;
    uint32_t log2_diff_max_min_luma_transform_block_size;
    uint32_t max_transform_hierarchy_depth_inter;
    uint32_t max_transform_hierarchy_depth_intra;

    bool scaling_list_enabled_flag;
    //if (scaling_list_enabled_flag)
        bool sps_scaling_list_data_present_flag;
        //if (sps_scaling_list_data_present_flag)
        //scaling_list_data() // TODO

    bool amp_enabled_flag;
    bool sample_adaptive_offset_enabled_flag;
    bool pcm_enabled_flag;
    //if (pcm_enabled_flag)
        uint8_t pcm_sample_bit_depth_luma_minus1;
        uint8_t pcm_sample_bit_depth_chroma_minus1;
        uint32_t log2_min_pcm_luma_coding_block_size_minus3;
        uint32_t log2_diff_max_min_pcm_luma_coding_block_size;
        bool pcm_loop_filter_disabled_flag;

    uint32_t num_short_term_ref_pic_sets;
    //for (i = 0; i < num_short_term_ref_pic_sets; i++)
        //st_ref_pic_set(i) // TODO

    bool long_term_ref_pics_present_flag;
    //if (long_term_ref_pics_present_flag)
        uint32_t num_long_term_ref_pics_sps;
        //for (i = 0; i < num_long_term_ref_pics_sps; i++)
            //uint32_t lt_ref_pic_poc_lsb_sps[ i ]; // TODO
            //bool used_by_curr_pic_lt_sps_flag[ i ];

    bool sps_temporal_mvp_enabled_flag;
    bool strong_intra_smoothing_enabled_flag;

    bool vui_parameters_present_flag;
    //if (vui_parameters_present_flag)
        //vui_parameters() // TODO

    bool sps_extension_present_flag;
    //if (sps_extension_present_flag)
        bool sps_range_extension_flag;
        bool sps_multilayer_extension_flag;
        bool sps_3d_extension_flag;
        bool sps_scc_extension_flag;
        uint8_t sps_extension_4bits;

    //if (sps_range_extension_flag)
        //sps_range_extension()
    //if (sps_multilayer_extension_flag)
        //sps_multilayer_extension() // specified in Annex F
    //if (sps_3d_extension_flag)
        //sps_3d_extension() // specified in Annex I
    //if (sps_scc_extension_flag)
        //sps_scc_extension()
    //if (sps_extension_4bits)
        //while (more_rbsp_data())
            bool sps_extension_data_flag;

} h265_sps_t;

/*!
 * \struct h265_pps_t
 * \brief PPS - Picture Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.3 Picture parameter set syntax.
 * - 7.4.3.3 Picture parameter set semantics.
 */
typedef struct h265_pps_t
{
    uint32_t pps_pic_parameter_set_id;
    uint32_t pps_seq_parameter_set_id;
    bool dependent_slice_segments_enabled_flag;
    bool output_flag_present_flag;
    uint8_t num_extra_slice_header_bits;
    bool sign_data_hiding_enabled_flag;
    bool cabac_init_present_flag;
    uint32_t num_ref_idx_l0_default_active_minus1;
    uint32_t num_ref_idx_l1_default_active_minus1;
    int32_t init_qp_minus26;
    bool constrained_intra_pred_flag;
    bool transform_skip_enabled_flag;
    bool cu_qp_delta_enabled_flag;
    //if (cu_qp_delta_enabled_flag)
        uint32_t diff_cu_qp_delta_depth;

    int32_t pps_cb_qp_offset;
    int32_t pps_cr_qp_offset;
    bool pps_slice_chroma_qp_offsets_present_flag;
    bool weighted_pred_flag;
    bool weighted_bipred_flag;
    bool transquant_bypass_enabled_flag;
    bool tiles_enabled_flag;
    bool entropy_coding_sync_enabled_flag;
    //if (tiles_enabled_flag)
        uint32_t num_tile_columns_minus1;
        uint32_t num_tile_rows_minus1;
        bool uniform_spacing_flag;
        //if (!uniform_spacing_flag)
            //for (i = 0; i < num_tile_columns_minus1; i++)
                uint32_t *column_width_minus1;
            //for (i = 0; i < num_tile_rows_minus1; i++)
                uint32_t *row_height_minus1;
        bool loop_filter_across_tiles_enabled_flag;
    bool pps_loop_filter_across_slices_enabled_flag;
    bool deblocking_filter_control_present_flag;
    //if (deblocking_filter_control_present_flag)
        bool deblocking_filter_override_enabled_flag;
        bool pps_deblocking_filter_disabled_flag;
        //if (!pps_deblocking_filter_disabled_flag)
            int32_t pps_beta_offset_div2;
            int32_t pps_tc_offset_div2;

    bool pps_scaling_list_data_present_flag;
    //if (pps_scaling_list_data_present_flag)
        //scaling_list_data()

    bool lists_modification_present_flag;
    uint32_t log2_parallel_merge_level_minus2;
    bool slice_segment_header_extension_present_flag;

    bool pps_extension_present_flag;
    //if (pps_extension_present_flag)
        bool pps_range_extension_flag;
        bool pps_multilayer_extension_flag;
        bool pps_3d_extension_flag;
        bool pps_scc_extension_flag;
        uint8_t pps_extension_4bits;

    //if (pps_range_extension_flag)
        //pps_range_extension()
    //if (pps_multilayer_extension_flag)
        //pps_multilayer_extension() // specified in Annex F
    //if (pps_3d_extension_flag)
        //pps_3d_extension() // specified in Annex I
    //if (pps_scc_extension_flag)
        //pps_scc_extension()
    //if (pps_extension_4bits)
        //while (more_rbsp_data())
            bool pps_extension_data_flag;

} h265_pps_t;

/*!
 * \struct h265_aud_t
 * \brief AUD - Access Unit Delimiter.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.5 Access unit delimiter syntax.
 * - 7.4.3.5 Access unit delimiter semantics.
 */
typedef struct h265_aud_t
{
    uint8_t pic_type;

} h265_aud_t;

/* ************************************************************************** */
#endif // H265_PARAMETER_SET_STRUCT_H
