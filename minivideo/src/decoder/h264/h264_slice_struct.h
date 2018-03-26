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
 * \file      h264_slice_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_SLICE_STRUCT_H
#define H264_SLICE_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"

/* ************************************************************************** */

#define MAX_REF_FRAMES 16
#define MAX_REF_FIELDS 32

/* ************************************************************************** */

/*
//! Name of slice_type
typedef enum SliceType_e
{
    SLICE_P     = 0, 5,
    SLICE_B     = 1, 6,
    SLICE_I     = 2, 7,
    SLICE_SP    = 3, 8,
    SLICE_SI    = 4, 9
} SliceType_e;
*/

/*!
 * \struct rplm_t
 * \brief RPLM - Reference Picture List Modification.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.3.1 Reference picture list modification syntax.
 * - 7.4.3.1 Reference picture list modification semantic.
 */
typedef struct rplm_t
{
    bool ref_pic_list_modification_flag_l0;
    bool ref_pic_list_modification_flag_l1;

    unsigned int modification_of_pic_nums_idc;
    unsigned int abs_diff_pic_num_minus1;
    unsigned int long_term_pic_num;
} rplm_t;

/*!
 * \struct pwt_t
 * \brief PWT - Prediction Weight Table.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.3.2 Prediction weight table syntax.
 * - 7.4.3.2 Prediction weight table semantics.
 */
typedef struct pwt_t
{
    unsigned int luma_log2_weight_denom;
    unsigned int chroma_log2_weight_denom;
    bool luma_weight_l0_flag;
//  int luma_weight_l0[i];              //FIXME range
//  int luma_offset_l0[i];              //FIXME range
    bool chroma_weight_l0_flag;
//  int chroma_weight_l0[i][j];         //FIXME range
//  int chroma_offset_l0[i][j];         //FIXME range
    bool luma_weight_l1_flag;
//  int luma_weight_l1[i];              //FIXME range
//  int luma_offset_l1[i];              //FIXME range
    bool chroma_weight_l1_flag;
//  int chroma_weight_l1[i][j];         //FIXME range
//  int chroma_offset_l1[i][j];         //FIXME range
} pwt_t;

/*!
 * \struct drpm_t
 * \brief DRPM - Decoded Reference Picture Marking.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.3.3 syntax.
 * - 7.4.3.3 semantics.
 */
typedef struct drpm_t
{
    //if (IdrPicFlag)
        bool no_output_of_prior_pics_flag;
        bool long_term_reference_flag;
    //else
        bool adaptive_ref_pic_marking_mode_flag;
        //if (adaptive_ref_pic_marking_mode_flag)
        //do {
            unsigned int memory_management_control_operation;
            //if (memory_management_control_operation == 1 ||
            //    memory_management_control_operation == 3)
                unsigned int difference_of_pic_nums_minus1;
            //if (memory_management_control_operation == 2)
                unsigned int long_term_pic_num;
            //if (memory_management_control_operation == 3 ||
            //    memory_management_control_operation == 6)
                unsigned int long_term_frame_idx;
            //if (memory_management_control_operation == 4)
                unsigned int max_long_term_frame_idx_plus1;
                unsigned int MaxLongTermFrameIdx;
        //} while (memory_management_control_operation != 0)
} drpm_t;

//! CABAC decoding context
typedef struct CabacContext_t
{
    uint8_t pStateIdx[460]; //!< Corresponds to a probability state index
    uint8_t valMPS[460];    //!< Corresponds to the value of the most probable symbol

    // The status of the arithmetic decoding engine is represented by the variables codIRange and codIOffset.
    uint16_t codIRange;
    uint16_t codIOffset;
} CabacContext_t;

/*!
 * \struct slice_t
 * \brief A slice is a group of adjacent macroblocks.
 *
 * Slice header, from 'ITU-T H.264' recommendation:
 * - 7.3.3 Slice header syntax.
 * - 7.4.3 Slice header semantics.
 *
 * Slice data, from 'ITU-T H.264' recommendation:
 * - 7.3.4 Slice data syntax.
 * - 7.4.4 Slice data semantics.
 */
typedef struct slice_t
{
    // Slice header
    ////////////////////////////////////////////////////////////////////////////

    unsigned int first_mb_in_slice;
    unsigned int slice_type;
    unsigned int pic_parameter_set_id;

    //if (separate_colour_plane_flag)
        unsigned int colour_plane_id;

    unsigned int frame_num;
    unsigned int PrevRefFrameNum;           // derived from frame_num

    //if (!frame_mbs_only_flag)
        bool field_pic_flag;
        //if (field_pic_flag)
            bool bottom_field_flag;
            unsigned int MaxPicNum;         // derived from MaxFrameNum, field_pic_flag
            unsigned int CurrPicNum;        // derived from frame_num, field_pic_flag
        bool MbaffFrameFlag;                // derived from mb_adaptive_frame_field_flag, field_pic_flag
        unsigned int PicHeightInMbs;        // derived from FrameHeightInMbs, field_pic_flag
        unsigned int PicHeightInSamplesL;   // derived from PicHeightInMbs
        unsigned int PicHeightInSamplesC;   // derived from PicHeightInMbs, MbHeightC
        unsigned int PicSizeInMbs;          // derived from PicWidthInMbs, PicHeightInMbs

    //if (IdrPicFlag)
        unsigned int idr_pic_id;

    //if (pic_order_cnt_type == 0)
        unsigned int pic_order_cnt_lsb;
        //if (bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
            int delta_pic_order_cnt_bottom;

    //if (pic_order_cnt_type == 1 && !delta_pic_order_always_zero_flag)
        int delta_pic_order_cnt[2];
        //if (bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
            //delta_pic_order_cnt[1];

    //if (redundant_pic_cnt_present_flag)
        unsigned int redundant_pic_cnt;

    //if (slice_type == B)
        bool direct_spatial_mv_pred_flag;

    //if (slice_type == P || slice_type == SP || slice_type == B)
        bool num_ref_idx_active_override_flag;
        //if (num_ref_idx_active_override_flag)
            unsigned int num_ref_idx_l0_active_minus1;
            //if (slice_type == B)
                unsigned int num_ref_idx_l1_active_minus1;

    unsigned int RefPicList0[MAX_REF_FRAMES];    //!< A list of reference pictures used for inter prediction of a P or SP slice
    unsigned int RefPicList1[MAX_REF_FRAMES];    //!< A list of reference pictures used for inter prediction of a B slice

    rplm_t *rplm;
    pwt_t *pwt;
    drpm_t *drpm;

    //if (entropy_coding_mode_flag && slice_type != I && slice_type != SI)
        unsigned int cabac_init_idc;
    int slice_qp_delta;
    int SliceQPY;                           // derived from slice_qp_delta
    int QPYprev;                            // keep this for use in macroblock + 1

    //if (slice_type == SP || slice_type == SI)
        //if (slice_type == SP)
            bool sp_for_switch_flag;
        int slice_qs_delta;
        int SliceQSY;                       // derived from slice_qs_delta

    //if (deblocking_filter_control_present_flag)
        unsigned int disable_deblocking_filter_idc;
        //if (disable_deblocking_filter_idc != 1)
            int slice_alpha_c0_offset_div2;
            int slice_beta_offset_div2;
            int FilterOffsetA;              // derived from slice_alpha_c0_offset_div2
            int FilterOffsetB;              // derived from slice_beta_offset_div2

    //if (num_slice_groups_minus1 > 0 && slice_group_map_type >= 3 && slice_group_map_type <= 5)
        unsigned int slice_group_change_cycle;

    //  Slice datas
    ////////////////////////////////////////////////////////////////////////////

    bool moreDataFlag;
    bool prevMbSkipped;
    unsigned int mb_skip_run;

    bool mb_skip_flag;              // ae(v)
    bool mb_field_decoding_flag;    // u(1) | ae(v)
    bool end_of_slice_flag;         // ae(v)

    // CABAC
    ////////////////////////////////////////////////////////////////////////////

    CabacContext_t *cc;

} slice_t;


/* ************************************************************************** */
#endif // H264_SLICE_STRUCT_H
