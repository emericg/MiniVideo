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
 * \file      h264_slice.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_slice.h"
#include "h264_slice_struct.h"
#include "h264_expgolomb.h"
#include "h264_cabac.h"
#include "h264_macroblock.h"
#include "h264.h"
#include "../../minivideo_typedef.h"
#include "../../export.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

/* ************************************************************************** */

static int decodeSliceHeader(DecodingContext_t *dc, slice_t *slice);
static int decodeSliceData(DecodingContext_t *dc, slice_t *slice);

static rplm_t *decodeRPLM(DecodingContext_t *dc, slice_t *slice);
static pwt_t *decodePWT(DecodingContext_t *dc, slice_t *slice);
static drpm_t *decodeDRPM(DecodingContext_t *dc, slice_t *slice);

static void printSliceHeader(DecodingContext_t *dc);
static void printRPLM(DecodingContext_t *dc, rplm_t *rplm);
static void printDRPM(DecodingContext_t *dc, drpm_t *drpm);

static int checkSliceHeader(DecodingContext_t *dc);
static int checkRPLM(DecodingContext_t *dc, rplm_t *rplm);
static int checkDRPM(DecodingContext_t *dc, drpm_t *drpm);

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 0 if slice allocation fail, 1 otherwise.
 */
int decode_slice(DecodingContext_t *dc)
{
    TRACE_INFO(SLICE, "> " BLD_GREEN "decodeSlice()" CLR_RESET);
    int retcode = SUCCESS;

    // New slice allocation
    dc->active_slice = (slice_t*)calloc(1, sizeof(slice_t));

    if (dc->active_slice == NULL)
    {
        TRACE_ERROR(SLICE, "Unable to alloc new Slice!");
        retcode = FAILURE;
    }
    else
    {
        // First, check for valid SPS and PPS
        retcode = checkDecodingContext(dc);

        // Slice header
        if (retcode == SUCCESS)
        {
            retcode = decodeSliceHeader(dc, dc->active_slice);
            printSliceHeader(dc);
        }

        // Slice datas
        if (retcode == SUCCESS)
        {
            retcode = decodeSliceData(dc, dc->active_slice);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param **slice_ptr A pointer to the slice structure we want to freed.
 */
void free_slice(slice_t **slice_ptr)
{
    if (*slice_ptr != NULL)
    {
        if ((*slice_ptr)->rplm != NULL)
        {
            free((*slice_ptr)->rplm);
        }

        if ((*slice_ptr)->pwt != NULL)
        {
            free((*slice_ptr)->pwt);
        }

        if ((*slice_ptr)->drpm != NULL)
        {
            free((*slice_ptr)->drpm);
        }

        if ((*slice_ptr)->cc != NULL)
        {
            free((*slice_ptr)->cc);
        }

        free(*slice_ptr);
        *slice_ptr = NULL;
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *slice The current Slice.
 */
static int decodeSliceHeader(DecodingContext_t *dc, slice_t *slice)
{
    TRACE_INFO(SLICE, "> " BLD_GREEN "decodeSliceHeader()" CLR_RESET);

    // Slice header decoding
    ////////////////////////////////////////////////////////////////////////////

    slice->first_mb_in_slice = read_ue(dc->bitstr);
    slice->slice_type = read_ue(dc->bitstr);
    slice->pic_parameter_set_id = read_ue(dc->bitstr);

    // Shortcuts
    pps_t *pps = dc->pps_array[slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    if (sps->separate_colour_plane_flag)
    {
        slice->colour_plane_id = read_bits(dc->bitstr, 2);
    }

    slice->frame_num = read_bits(dc->bitstr, sps->log2_max_frame_num_minus4 + 4);

    if (sps->frame_mbs_only_flag == false)
    {
        slice->field_pic_flag = read_bit(dc->bitstr);
        if (slice->field_pic_flag)
        {
            slice->bottom_field_flag = read_bit(dc->bitstr);

            slice->MaxPicNum = sps->MaxFrameNum*2;
            slice->CurrPicNum = slice->frame_num*2 + 1;
        }
        else
        {
            slice->MaxPicNum = sps->MaxFrameNum;
            slice->CurrPicNum = slice->frame_num;
        }
    }

    slice->MbaffFrameFlag = sps->mb_adaptive_frame_field_flag && !slice->field_pic_flag;
    slice->PicHeightInMbs = sps->FrameHeightInMbs / (1 + slice->field_pic_flag);
    slice->PicHeightInSamplesL = slice->PicHeightInMbs * 16;
    slice->PicHeightInSamplesC = slice->PicHeightInMbs * sps->MbHeightC;
    slice->PicSizeInMbs = sps->PicWidthInMbs * slice->PicHeightInMbs;

    if (dc->IdrPicFlag)
    {
        slice->PrevRefFrameNum = 0;
        slice->idr_pic_id = read_ue(dc->bitstr);
    }

    if (sps->pic_order_cnt_type == 0)
    {
        slice->pic_order_cnt_lsb = read_bits(dc->bitstr, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (pps->bottom_field_pic_order_in_frame_present_flag && slice->field_pic_flag == false)
        {
            slice->delta_pic_order_cnt_bottom = read_se(dc->bitstr);
        }
    }
    else if (sps->pic_order_cnt_type == 1 && sps->delta_pic_order_always_zero_flag == false)
    {
        slice->delta_pic_order_cnt[0] = read_se(dc->bitstr);
        if (pps->bottom_field_pic_order_in_frame_present_flag && slice->field_pic_flag == false)
        {
            slice->delta_pic_order_cnt[1] = read_se(dc->bitstr);
        }
    }

    if (pps->redundant_pic_cnt_present_flag)
    {
        slice->redundant_pic_cnt = read_ue(dc->bitstr);
    }

    if (slice->slice_type == 1 || slice->slice_type == 6) // B frame
    {
        TRACE_ERROR(SLICE, ">>> UNSUPPORTED (B frame)");
        return UNSUPPORTED;

        slice->direct_spatial_mv_pred_flag = read_bit(dc->bitstr);
    }

    if (slice->slice_type == 0 || slice->slice_type == 5 ||
        slice->slice_type == 3 || slice->slice_type == 8 ||
        slice->slice_type == 1 || slice->slice_type == 6) // P, SP, B frame
    {
        TRACE_ERROR(SLICE, ">>> UNSUPPORTED (P, SP, B frame)");
        return UNSUPPORTED;
/*
        slice->num_ref_idx_active_override_flag = read_bit(dc->bitstr);

        if (slice->num_ref_idx_active_override_flag)
        {
            slice->num_ref_idx_l0_active_minus1 = read_ue(dc->bitstr);

            if (slice->slice_type == 1 || slice->slice_type == 6) // B frame
            {
                slice->num_ref_idx_l1_active_minus1 = read_ue(dc->bitstr);
            }
        }
*/
    }

    if (dc->active_nalu->nal_unit_type == 20)
    {
        TRACE_ERROR(SLICE, ">>> UNSUPPORTED (unit_type == 20: MVC extension)");
        return UNSUPPORTED;
    }
    else
    {
        // RPLM
        slice->rplm = decodeRPLM(dc, slice);
    }

    if ((pps->weighted_pred_flag == true &&
         (slice->slice_type == 0 || slice->slice_type == 5 ||
          slice->slice_type == 3 || slice->slice_type == 8)) ||
        (pps->weighted_bipred_idc == 1  &&
         (slice->slice_type == 1 || slice->slice_type == 6))) // P, SP, B frame
    {
        // PWT
        slice->pwt = decodePWT(dc, slice);
    }

    if (dc->active_nalu->nal_ref_idc != 0)
    {
        // DRPM
        slice->drpm = decodeDRPM(dc, slice);
    }

    if (pps->entropy_coding_mode_flag == true &&
        ((slice->slice_type != 2 && slice->slice_type != 7) &&
         (slice->slice_type != 4 && slice->slice_type != 9))) // Not I or SI frame
    {
        slice->cabac_init_idc = read_ue(dc->bitstr);
    }

    slice->slice_qp_delta = read_se(dc->bitstr);
    slice->SliceQPY = 26 + pps->pic_init_qp_minus26 + slice->slice_qp_delta;
    slice->QPYprev = slice->SliceQPY; // Set QPYprev value to use it in the first macroblock

    if (slice->slice_type == 3 || slice->slice_type == 8 ||
        slice->slice_type == 4 || slice->slice_type == 9) // SP, SI frame
    {
#if ENABLE_SWITCHING_SLICE
        if (slice->slice_type == 4 || slice->slice_type == 9)
        {
            slice->sp_for_switch_flag = read_bit(dc->bitstr);
        }

        slice->slice_qs_delta = read_se(dc->bitstr);
        slice->SliceQSY = 26 + pps->pic_init_qs_minus26 + slice->slice_qs_delta;
#else // ENABLE_SWITCHING_SLICE
        TRACE_ERROR(SLICE, ">>> UNSUPPORTED (slice_type == SP || slice_type == SI)");
        return UNSUPPORTED;
#endif // ENABLE_SWITCHING_SLICE
    }

    if (pps->deblocking_filter_control_present_flag)
    {
        slice->disable_deblocking_filter_idc = read_ue(dc->bitstr);
        if (slice->disable_deblocking_filter_idc != 1)
        {
            slice->slice_alpha_c0_offset_div2 = read_se(dc->bitstr);
            slice->slice_beta_offset_div2 = read_se(dc->bitstr);

            slice->FilterOffsetA = slice->slice_alpha_c0_offset_div2 << 1;
            slice->FilterOffsetB = slice->slice_beta_offset_div2 << 1;
        }
    }

    if (pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
    {
        TRACE_ERROR(SLICE, ">>> UNSUPPORTED (FMO)");
        return UNSUPPORTED;
    }

    // Check content
    return checkSliceHeader(dc);
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \brief Print informations about slice header decoding.
 */
static void printSliceHeader(DecodingContext_t *dc)
{
#if ENABLE_DEBUG
    TRACE_INFO(SLICE, "> " BLD_GREEN "printSliceHeader()" CLR_RESET);

    // Shortcut
    slice_t *slice = dc->active_slice;

    // Check structure
    if (slice == NULL)
    {
        TRACE_ERROR(SLICE, "  Invalid slice structure!");
        return;
    }

    // Shortcuts
    pps_t *pps = dc->pps_array[slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    // Print values
    TRACE_1(SLICE, "  - first_mb_in_slice   = %i", slice->first_mb_in_slice);
    TRACE_1(SLICE, "  - slice_type          = %i", slice->slice_type);
    TRACE_1(SLICE, "  - pic_parameter_set_id= %i", slice->pic_parameter_set_id);

    if (sps->separate_colour_plane_flag)
    {
        TRACE_1(SLICE, "  - colour_plane_id     = %i", slice->colour_plane_id);
    }

    TRACE_1(SLICE, "  - frame_num           = %i", slice->frame_num);
    TRACE_1(SLICE, "  - MaxPicNum           : %i", slice->MaxPicNum);
    TRACE_1(SLICE, "  - CurrPicNum          : %i", slice->CurrPicNum);

    if (sps->frame_mbs_only_flag == false)
    {
        TRACE_1(SLICE, "  - field_pic_flag      = %i", slice->field_pic_flag);

        if (slice->field_pic_flag)
        {
            TRACE_1(SLICE, "  - bottom_field_flag   = %i", slice->bottom_field_flag);
        }
    }

    TRACE_1(SLICE, "  - MbaffFrameFlag      : %i", slice->MbaffFrameFlag);
    TRACE_1(SLICE, "  - PicHeightInMbs      : %i", slice->PicHeightInMbs);
    TRACE_1(SLICE, "  - PicHeightInSamplesL : %i", slice->PicHeightInSamplesL);
    TRACE_1(SLICE, "  - PicHeightInSamplesC : %i", slice->PicHeightInSamplesC);
    TRACE_1(SLICE, "  - PicSizeInMbs        : %i", slice->PicSizeInMbs);

    if (dc->IdrPicFlag)
    {
        TRACE_1(SLICE, "  - idr_pic_id          = %i", slice->idr_pic_id);
    }

    if (sps->pic_order_cnt_type == 0)
    {
        TRACE_1(SLICE, "  - pic_order_cnt_lsb       = %i", slice->pic_order_cnt_lsb);

        if (pps->bottom_field_pic_order_in_frame_present_flag && slice->field_pic_flag == false)
        {
            TRACE_1(SLICE, "  - delta_pic_order_cnt_bottom  = %i", slice->delta_pic_order_cnt_bottom);
        }
    }
    else if (sps->pic_order_cnt_type == 1 && sps->delta_pic_order_always_zero_flag == false)
    {
        TRACE_1(SLICE, "  - delta_pic_order_cnt[0]  = %i", slice->delta_pic_order_cnt[0]);
        if (pps->bottom_field_pic_order_in_frame_present_flag && slice->field_pic_flag == false)
        {
            TRACE_1(SLICE, "  - delta_pic_order_cnt[1]= %i", slice->delta_pic_order_cnt[1]);
        }
    }

    if (dc->active_nalu->nal_unit_type != 20)
    {
        // RPLM
        printRPLM(dc, slice->rplm);
    }

    if ((pps->weighted_pred_flag && (slice->slice_type == 0 || slice->slice_type == 5 ||slice->slice_type == 3 ||slice->slice_type == 8)) ||
        (pps->weighted_bipred_idc == 1 && (slice->slice_type == 1 || slice->slice_type == 6)))
    {
        // PWT
        //printPWT(dc, slice->pwt);
    }

    if (dc->active_nalu->nal_ref_idc != 0)
    {
        // DRPM
        printDRPM(dc, slice->drpm);
    }

    TRACE_1(SLICE, "  - slice_qp_delta      = %i", slice->slice_qp_delta);
    TRACE_1(SLICE, "  - SliceQPY            : %i", slice->SliceQPY);

    if (slice->slice_type == 3 || slice->slice_type == 8 ||
        slice->slice_type == 4 || slice->slice_type == 9) // SP, SI frame
    {
        if (slice->slice_type == 4 || slice->slice_type == 9) // SI frame
        {
            TRACE_1(SLICE, "  - sp_for_switch_flag  = %i", slice->sp_for_switch_flag);
        }
        TRACE_1(SLICE, "  - slice_qs_delta      = %i", slice->slice_qs_delta);
        TRACE_1(SLICE, "  - SliceQSY            : %i", slice->SliceQSY);
    }

    if (pps->deblocking_filter_control_present_flag)
    {
        TRACE_1(SLICE, "  - disable_deblocking_filter_idc= %i", slice->disable_deblocking_filter_idc);

        if (slice->disable_deblocking_filter_idc != 1)
        {
            TRACE_1(SLICE, "  - slice_alpha_c0_offset_div2= %i", slice->slice_alpha_c0_offset_div2);
            TRACE_1(SLICE, "  - slice_beta_offset_div2= %i", slice->slice_beta_offset_div2);
            TRACE_1(SLICE, "  - FilterOffsetA       : %i", slice->FilterOffsetA);
            TRACE_1(SLICE, "  - FilterOffsetB       : %i", slice->FilterOffsetB);
        }
    }

    if (pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
    {
        TRACE_1(SLICE, "  - slice_group_change_cycle    = %i", slice->slice_group_change_cycle);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \brief Check conformance of values decoded from slice header.
 * \return 1 if slice header seems consistent, 0 otherwise.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 * See H264 specification subclause 7.4.3.
 */
static int checkSliceHeader(DecodingContext_t *dc)
{
    TRACE_INFO(SLICE, "  > " BLD_GREEN "checkSliceHeader()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    slice_t *slice = dc->active_slice;

    // Check structure
    if (slice == NULL)
    {
        TRACE_ERROR(SLICE, "  Invalid slice_header structure!");
        retcode = FAILURE;
    }
    else // Check values
    {
        // Shortcuts
        pps_t *pps = dc->pps_array[slice->pic_parameter_set_id];
        sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

        if (slice->slice_type > 9)
        {
            TRACE_WARNING(SLICE, "  - slice_type is %i but should be in range [0,9]", slice->slice_type);
            retcode = FAILURE;
        }
        else if (dc->active_nalu->nal_unit_type == 5 &&
                 (slice->slice_type != 2 && slice->slice_type != 7 &&
                  slice->slice_type != 4 && slice->slice_type != 9)) // Not I or SI slice
        {
            TRACE_WARNING(SLICE, "  - slice_type is %i but should be 2,4,7,9 (because this frame is an IDR)", slice->slice_type);
            retcode = FAILURE;
        }

        if (slice->pic_parameter_set_id > 255)
        {
            TRACE_WARNING(SLICE, "  - pic_parameter_set_id is %i but should be in range [0,255]", slice->pic_parameter_set_id);
            retcode = FAILURE;
        }
        else if (dc->pps_array[slice->pic_parameter_set_id] == NULL)
        {
            TRACE_WARNING(SLICE, "  - this slice refere to a PPS (nb %i) which doesn't exist", slice->pic_parameter_set_id);
            retcode = FAILURE;
        }

        if (sps->separate_colour_plane_flag)
        {
            if (slice->colour_plane_id > 2)
            {
                TRACE_WARNING(SLICE, "  - colour_plane_id is %i but should be in range [0,2]", slice->colour_plane_id);
                retcode = FAILURE;
            }
        }

        if (dc->IdrPicFlag)
        {
            if (slice->frame_num != 0)
            {
                TRACE_WARNING(SLICE, "  - frame_num is %i but should be 0 (because this frame is an IDR)", slice->frame_num);
                retcode = FAILURE;
            }
        }
        else
        {
            // not needed to check frame_num for non IDR frame
        }

        if (sps->frame_mbs_only_flag == false)
        {
            TRACE_1(SLICE, "  - field_pic_flag  : %i", slice->field_pic_flag);

            if (slice->field_pic_flag)
            {
                TRACE_1(SLICE, "  - bottom_field_flag: %i", slice->bottom_field_flag);
            }
        }

        if (dc->IdrPicFlag)
        {
            if (slice->idr_pic_id > 65535)
            {
                TRACE_WARNING(SLICE, "  - idr_pic_id is %i but should be in range [0,65535]", slice->idr_pic_id);
                retcode = FAILURE;
            }
        }

        if (sps->pic_order_cnt_type == 0)
        {
            if (slice->pic_order_cnt_lsb > sps->MaxPicOrderCntLsb-1)
            {
                TRACE_WARNING(SLICE, "  - pic_order_cnt_lsb is %i but should be in range [0,MaxPicOrderCntLsb-1=%i]", slice->pic_order_cnt_lsb, sps->MaxPicOrderCntLsb-1);
                retcode = FAILURE;
            }

            if (pps->bottom_field_pic_order_in_frame_present_flag == true &&
                slice->field_pic_flag == false)
            {/*
                if ()
                {
                    TRACE_WARNING(SLICE, "  - delta_pic_order_cnt_bottom is %i but ", slice->delta_pic_order_cnt_bottom);
                    retcode = FAILURE;
                }
            */}
        }
        else if (sps->pic_order_cnt_type == 1 && sps->delta_pic_order_always_zero_flag == false)
        {
            // always true if delta_pic[0]_order_cnt is an int32

            if (pps->bottom_field_pic_order_in_frame_present_flag == true && slice->field_pic_flag == false)
            {
                // always true if delta_pic_order_cnt[1] is an int32
            }
        }

        if (retcode == SUCCESS && dc->active_nalu->nal_unit_type != 20)
        {
            // Check RPLM
            retcode = checkRPLM(dc, slice->rplm);
        }

        if (retcode == SUCCESS &&
            ((pps->weighted_pred_flag == true &&
              (slice->slice_type == 0 || slice->slice_type == 5 ||
               slice->slice_type == 3 || slice->slice_type == 8)) ||
            (pps->weighted_bipred_idc == 1 &&
             (slice->slice_type == 1 || slice->slice_type == 6))))
        {
            // Check PWT
            //retcode = checkPWT(dc, slice->pwt);
        }

        if (retcode == SUCCESS && dc->active_nalu->nal_ref_idc != 0)
        {
            // Check DRPM
            retcode = checkDRPM(dc, slice->drpm);
        }

        if (slice->SliceQPY < -((int)sps->QpBdOffsetY) || slice->SliceQPY > 51)
        {
            TRACE_WARNING(SLICE, "  - SliceQPY is %i but should be in range [-QpBdOffsetY=%i,51] %i", slice->SliceQPY, sps->QpBdOffsetY);
            retcode = FAILURE;
        }

        if (pps->deblocking_filter_control_present_flag)
        {
            if (slice->disable_deblocking_filter_idc > 2)
            {
                TRACE_WARNING(SLICE, "  - disable_deblocking_filter_idc is %i but should be in range [0-2]", slice->disable_deblocking_filter_idc);
                retcode = FAILURE;
            }
            else if (slice->disable_deblocking_filter_idc != 1)
            {
                if (slice->slice_alpha_c0_offset_div2 < -6 || slice->slice_alpha_c0_offset_div2 > 6)
                {
                    TRACE_WARNING(SLICE, "  - slice_alpha_c0_offset_div2 is %i but should be in range [-6,6]", slice->slice_alpha_c0_offset_div2);
                    retcode = FAILURE;
                }

                if (slice->slice_beta_offset_div2 < -6 || slice->slice_beta_offset_div2 > 6)
                {
                    TRACE_WARNING(SLICE, "  - slice_beta_offset_div2 is %i but should be in range [-6,6]", slice->slice_beta_offset_div2);
                    retcode = FAILURE;
                }
            }
        }
/*
        if (pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
        {
            // The value of slice_group_change_cycle shall to ceil(PicSizeInMapUnits÷SliceGroupChangeRate), inclusive.
            TRACE_1(SLICE, "  - slice_group_change_cycle: %i", slice->slice_group_change_cycle);
        }
*/
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *slice structure.
 * \return *rplm_t initialized data structure.
 */
static rplm_t *decodeRPLM(DecodingContext_t *dc, slice_t *slice)
{
    TRACE_INFO(SLICE, "  > " BLD_GREEN "decodeRPLM()" CLR_RESET);

    // RPLM allocation
    ////////////////////////////////////////////////////////////////////////////

    rplm_t *rplm = (rplm_t*)calloc(1, sizeof(rplm_t));
    if (rplm == NULL)
    {
        TRACE_ERROR(SLICE, "Unable to alloc new RPLM!");
    }
    else
    {
        // RPLM decoding
        ////////////////////////////////////////////////////////////////////////

        if (slice->slice_type % 5 != 2 && slice->slice_type % 5 != 4)
        {
            rplm->ref_pic_list_modification_flag_l0 = read_bit(dc->bitstr);
            if (rplm->ref_pic_list_modification_flag_l0)
            {
                do {
                    rplm->modification_of_pic_nums_idc = read_ue(dc->bitstr);
                    if (rplm->modification_of_pic_nums_idc == 0 ||
                        rplm->modification_of_pic_nums_idc == 1)
                    {
                        rplm->abs_diff_pic_num_minus1 = read_ue(dc->bitstr);
                    }
                    else if (rplm->modification_of_pic_nums_idc == 2)
                    {
                        rplm->long_term_pic_num = read_ue(dc->bitstr);
                    }
                } while (rplm->modification_of_pic_nums_idc != 3);
            }
        }

        if (slice->slice_type % 5 == 1)
        {
            rplm->ref_pic_list_modification_flag_l1 = read_bit(dc->bitstr);
            if (rplm->ref_pic_list_modification_flag_l1)
            {
                do {
                    rplm->modification_of_pic_nums_idc = read_ue(dc->bitstr);
                    if (rplm->modification_of_pic_nums_idc == 0 ||
                        rplm->modification_of_pic_nums_idc == 1)
                    {
                        rplm->abs_diff_pic_num_minus1 = read_ue(dc->bitstr);
                    }
                    else if (rplm->modification_of_pic_nums_idc == 2)
                    {
                        rplm->long_term_pic_num = read_ue(dc->bitstr);
                    }
                } while (rplm->modification_of_pic_nums_idc != 3);
            }
        }
    }

    return rplm;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about reference_picture_list_modification decoding.
 * \param *dc The current DecodingContext.
 * \param *rplm (rplm_t) data structure.
 */
static void printRPLM(DecodingContext_t *dc, rplm_t *rplm)
{
#if ENABLE_DEBUG
    TRACE_INFO(SLICE, "  > " BLD_GREEN "printRPLM()" CLR_RESET);

    // Check structure
    if (rplm == NULL)
    {
        TRACE_ERROR(SLICE, "    Invalid RPML structure!");
        return;
    }

    // Print values
    if (rplm->ref_pic_list_modification_flag_l0)
    {
        TRACE_1(SLICE, "    - l0  modification_of_pic_nums_idc= %i", rplm->modification_of_pic_nums_idc);

        TRACE_1(SLICE, "    - l0  abs_diff_pic_num_minus1   = %i", rplm->abs_diff_pic_num_minus1);
        TRACE_1(SLICE, "    - l0  long_term_pic_num         = %i", rplm->long_term_pic_num);
    }
    else
    {
        TRACE_1(SLICE, "    - ref_pic_list_modification_flag_l0 is not defined");
    }

    if (rplm->ref_pic_list_modification_flag_l1)
    {
        TRACE_1(SLICE, "    - l1  modification_of_pic_nums_idc= %i", rplm->modification_of_pic_nums_idc);

        TRACE_1(SLICE, "    - l1  abs_diff_pic_num_minus1   = %i", rplm->abs_diff_pic_num_minus1);
        TRACE_1(SLICE, "    - l1  long_term_pic_num         = %i", rplm->long_term_pic_num);
    }
    else
    {
        TRACE_1(SLICE, "    - ref_pic_list_modification_flag_l1 is not defined");
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Check conformance of values decoded from reference_picture_list_modification.
 * \param *dc The current DecodingContext.
 * \param *rplm (reference_picture_list_modification) data structure.
 * \return 1 if RPLM seems consistent, 0 otherwise.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 * See H264 specification subclause 7.4.3.1.
 */
static int checkRPLM(DecodingContext_t *dc, rplm_t *rplm)
{
    TRACE_INFO(SLICE, "    > " BLD_GREEN "checkRPLM()" CLR_RESET);
    int retcode = SUCCESS;

    // Check structure
    if (rplm == NULL)
    {
        TRACE_ERROR(SLICE, "    Invalid RPML structure!");
        retcode = FAILURE;
    }
    else // Check values
    {
        TRACE_WARNING(SLICE, "    >>> UNIMPLEMENTED (checkRPLM)");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *slice structre.
 * \return *pwt_t initialized data structure.
 */
static pwt_t *decodePWT(DecodingContext_t *dc, slice_t *slice)
{
    TRACE_INFO(SLICE, "  > " BLD_GREEN "decodePWT()" CLR_RESET);

    // PWT allocation
    ////////////////////////////////////////////////////////////////////////////

    pwt_t *pwt = (pwt_t*)calloc(1, sizeof(pwt_t));
    if (pwt == NULL)
    {
        TRACE_ERROR(SLICE, "Unable to alloc new PWT!");
    }
    else
    {
        // PWT decoding
        ////////////////////////////////////////////////////////////////////////

        TRACE_WARNING(SLICE, ">>> UNIMPLEMENTED (prediction_weight_table)");
    }

    return pwt;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *slice structre.
 * \return *drpm_t initialized data structure.
 */
static drpm_t *decodeDRPM(DecodingContext_t *dc, slice_t *slice)
{
    TRACE_INFO(SLICE, "  > " BLD_GREEN "decodeDRPM()" CLR_RESET);

    // DRPM allocation
    ////////////////////////////////////////////////////////////////////////////

    drpm_t *drpm = (drpm_t*)calloc(1, sizeof(drpm_t));
    if (drpm == NULL)
    {
        TRACE_ERROR(SLICE, "Unable to alloc new DRPM!");
    }
    else
    {
        // DRPM decoding
        ////////////////////////////////////////////////////////////////////////

        if (dc->IdrPicFlag)
        {
            drpm->no_output_of_prior_pics_flag = read_bit(dc->bitstr);
            drpm->long_term_reference_flag = read_bit(dc->bitstr);
        }
        else
        {
            drpm->adaptive_ref_pic_marking_mode_flag = read_bit(dc->bitstr);
            if (drpm->adaptive_ref_pic_marking_mode_flag)
            {
                do {
                    drpm->memory_management_control_operation = read_ue(dc->bitstr);
                    if (drpm->memory_management_control_operation == 1 ||
                        drpm->memory_management_control_operation == 3)
                    {
                        drpm->difference_of_pic_nums_minus1 = read_ue(dc->bitstr);
                    }
                    else if (drpm->memory_management_control_operation == 2)
                    {
                        drpm->long_term_pic_num = read_ue(dc->bitstr);
                    }
                    else if (drpm->memory_management_control_operation == 3 ||
                             drpm->memory_management_control_operation == 6)
                    {
                        drpm->long_term_frame_idx = read_ue(dc->bitstr);
                    }
                    else if (drpm->memory_management_control_operation == 4)
                    {
                        drpm->max_long_term_frame_idx_plus1 = read_ue(dc->bitstr);
                        if (drpm->max_long_term_frame_idx_plus1 > 0)
                        {
                            drpm->MaxLongTermFrameIdx = drpm->max_long_term_frame_idx_plus1 - 1;
                        }
                        else
                        {
                            //FIXME 8.2.5.4.4 Decoding process for MaxLongTermFrameIdx
                        }
                    }
                } while (drpm->memory_management_control_operation != 0);
            }
        }
    }

    return drpm;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about decoded_reference_picture_marking decoding.
 * \param *dc The current DecodingContext.
 * \param *drpm (drpm_t) data structre.
 */
static void printDRPM(DecodingContext_t *dc, drpm_t *drpm)
{
#if ENABLE_DEBUG
    TRACE_INFO(SLICE, "  > " BLD_GREEN "printDRPM()" CLR_RESET);

    // Check structure
    if (drpm == NULL)
    {
        TRACE_ERROR(SLICE, "    Invalid DRPM structure!");
        return;
    }

    // Print values
    if (dc->IdrPicFlag)
    {
        TRACE_1(SLICE, "    - no_output_of_prior_pics_flag  = %i", drpm->no_output_of_prior_pics_flag);
        TRACE_1(SLICE, "    - long_term_reference_flag      = %i", drpm->long_term_reference_flag);
    }
    else
    {
        TRACE_1(SLICE, "    - adaptive_ref_pic_marking_mode_flag= %i", drpm->adaptive_ref_pic_marking_mode_flag);
        if (drpm->adaptive_ref_pic_marking_mode_flag)
        {
            TRACE_1(SLICE, "    - (last) memory_management_control_operation= %i", drpm->memory_management_control_operation);

            TRACE_1(SLICE, "    - difference_of_pic_nums_minus1 = %i", drpm->difference_of_pic_nums_minus1);
            TRACE_1(SLICE, "    - long_term_pic_num             = %i", drpm->long_term_pic_num);
            TRACE_1(SLICE, "    - long_term_frame_idx           = %i", drpm->long_term_frame_idx);
            TRACE_1(SLICE, "    - max_long_term_frame_idx_plus1 = %i", drpm->max_long_term_frame_idx_plus1);
        }
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Check conformance of values decoded from decoded_reference_picture_marking.
 * \param *dc The current DecodingContext.
 * \param *drpm (decoded_reference_picture_marking) data structre.
 * \return 1 if DRPM seems consistent, 0 otherwise.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 * See H264 specification subclause 7.4.3.3.
 */
static int checkDRPM(DecodingContext_t *dc, drpm_t *drpm)
{
    TRACE_INFO(SLICE, "    > " BLD_GREEN "checkDRPM()" CLR_RESET);
    int retcode = SUCCESS;

    // Check structure
    if (drpm == NULL)
    {
        TRACE_ERROR(SLICE, "    Invalid DRPM structure!");
        retcode = FAILURE;
    }
    else // Check values
    {
        if (dc->IdrPicFlag == false)
        {
            if (drpm->adaptive_ref_pic_marking_mode_flag)
            {
                if (drpm->memory_management_control_operation > 6)
                {
                    TRACE_WARNING(SLICE, "    - memory_management_control_operation is %i but should be in range [0,6]", drpm->memory_management_control_operation);
                    retcode = FAILURE;
                }
                else
                {
                    if (drpm->long_term_frame_idx > drpm->MaxLongTermFrameIdx)
                    {
                        TRACE_WARNING(SLICE, "    - long_term_frame_idx is %i but should be in range [0,MaxLongTermFrameIdx=%i]", drpm->long_term_frame_idx, drpm->MaxLongTermFrameIdx);
                        retcode = FAILURE;
                    }

                    unsigned int max_num_ref_frames = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id]->max_num_ref_frames;
                    if (drpm->max_long_term_frame_idx_plus1 > max_num_ref_frames)
                    {
                        TRACE_WARNING(SLICE, "    - max_long_term_frame_idx_plus1 is %i but should be in range [0,max_num_ref_frames=%i]", drpm->max_long_term_frame_idx_plus1, max_num_ref_frames);
                        retcode = FAILURE;
                    }
                }
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *slice The current Slice.
 * \return 0 if an error occurred while decoding slice datas, 1 otherwise.
 */
static int decodeSliceData(DecodingContext_t *dc, slice_t *slice)
{
    TRACE_INFO(SLICE, "> " BLD_GREEN "decodeSliceData()" CLR_RESET);

    // Initialization
    int retcode = SUCCESS;
    dc->CurrMbAddr = 0;
    slice->moreDataFlag = true;
    slice->prevMbSkipped = false;

    // Shortcut
    pps_t *pps = dc->pps_array[slice->pic_parameter_set_id];

    // Slice data decoding
    ////////////////////////////////////////////////////////////////////////////

    if (pps->entropy_coding_mode_flag)
    {
        // CABAC alignment
        while (bitstream_check_alignment(dc->bitstr) == false)
        {
            if (read_bit(dc->bitstr) == 0) // cabac_alignment_one_bit
            {
                TRACE_ERROR(SLICE, "cabac_alignment_one_bit must be 1");
                return FAILURE;
            }
        }

        // CABAC initialization process
        initCabacContextVariables(dc);
        initCabacDecodingEngine(dc);
    }

    while ((retcode == SUCCESS) &&
           (slice->moreDataFlag == true) &&
           (dc->CurrMbAddr < dc->PicSizeInMbs))
    {
#if ENABLE_INTER_PRED
        // Only I frames are supported anyway
        if (slice->slice_type != 2 && slice->slice_type != 7 &&
            slice->slice_type != 4 && slice->slice_type != 9) // Not I or SI slice
        {
            if (pps->entropy_coding_mode_flag)
            {
                slice->mb_skip_flag = read_ae(dc, SE_mb_skip_flag);
                slice->moreDataFlag = !slice->mb_skip_flag;
            }
            else
            {
                slice->mb_skip_run = read_ue(dc->bitstr);
                slice->prevMbSkipped = (slice->mb_skip_run > 0);

                for (int i = 0; i < slice->mb_skip_run; i++)
                {
                    dc->CurrMbAddr = NextMbAddress(dc, dc->CurrMbAddr);
                }

                if (slice->mb_skip_run > 0)
                {
                    slice->moreDataFlag = h264_more_rbsp_data(dc->bitstr);
                }
            }
        }

        // If there is still some data in the slice
        if (slice->moreDataFlag)
        {
#if ENABLE_MBAFF
            if (slice->MbaffFrameFlag == true &&
                (dc->CurrMbAddr % 2 == 0 || (dc->CurrMbAddr % 2 == 1 && slice->prevMbSkipped == true)))
            {
                if (pps->entropy_coding_mode_flag)
                    slice->mb_field_decoding_flag = read_ae(dc, SE_mb_field_decoding_flag);
                else
                    slice->mb_field_decoding_flag = read_bit(dc->bitstr);

                TRACE_ERROR(SLICE, ">>> UNSUPPORTED (interlaced mode)");
                return UNSUPPORTED;
            }
#endif // ENABLE_MBAFF

            // Macroblock decoding
            retcode = macroblock_layer(dc, dc->CurrMbAddr);
        }
#endif // ENABLE_INTER_PRED

        // Macroblock decoding
        retcode = macroblock_layer(dc, dc->CurrMbAddr);

        // Check for end of slice
        if (pps->entropy_coding_mode_flag)
        {
#if ENABLE_INTER_PRED
            // Only I frames are supported anyway
            if (slice->slice_type != 2 && slice->slice_type != 7 &&
                slice->slice_type != 4 && slice->slice_type != 9) // Not I or SI slice
            {
                slice->prevMbSkipped = slice->mb_skip_flag;
            }

#if ENABLE_MBAFF
            // MbaffFrameFlag is unsupported, so always "false"
            if (slice->MbaffFrameFlag == true && dc->CurrMbAddr % 2 == 0)
            {
                slice->moreDataFlag = true;
            }
            else
#endif // ENABLE_MBAFF
#endif // ENABLE_INTER_PRED
            {
                slice->end_of_slice_flag = read_ae(dc, SE_end_of_slice_flag);
                if (slice->end_of_slice_flag)
                {
                    TRACE_INFO(SLICE, "end_of_slice_flag detected!");
                    slice->moreDataFlag = false;
                }
            }
        }
        else
        {
            slice->moreDataFlag = h264_more_rbsp_data(dc->bitstr);
        }

        // Get next macroblock address
        dc->CurrMbAddr = NextMbAddress(dc, dc->CurrMbAddr);
    }

    return retcode;
}

/* ************************************************************************** */
