/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      h264_parameterset.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_parameterset.h"
#include "h264_expgolomb.h"
#include "h264_transform.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "../../bitstream_utils.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/* ************************************************************************** */

//! Assignment of mnemonic names to scaling list indices (Table 7-2)
typedef enum ScalingList_e
{
    Sl_4x4_Intra_Y   = 0,
    Sl_4x4_Intra_Cb  = 1,
    Sl_4x4_Intra_Cr  = 2,
    Sl_4x4_Inter_Y   = 3,
    Sl_4x4_Inter_Cb  = 4,
    Sl_4x4_Inter_Cr  = 5,
    Sl_8x8_Intra_Y   = 6,
    Sl_8x8_Inter_Y   = 7,
    Sl_8x8_Intra_Cb  = 8,
    Sl_8x8_Inter_Cb  = 9,
    Sl_8x8_Intra_Cr  = 10,
    Sl_8x8_Inter_Cr  = 11
} ScalingList_e;

//! SEI payload type
typedef enum SEI_payloadType_e
{
    SEI_BUFFERING_PERIOD = 0,
    SEI_PIC_TIMING,
    SEI_PAN_SCAN_RECT,
    SEI_FILLER_PAYLOAD,
    SEI_USER_DATA_REGISTERED_ITU_T_T35,
    SEI_USER_DATA_UNREGISTERED,
    SEI_RECOVERY_POINT,
    SEI_DEC_REF_PIC_MARKING_REPETITION,
    SEI_SPARE_PIC,
    SEI_SCENE_INFO,
    SEI_SUB_SEQ_INFO,
    SEI_SUB_SEQ_LAYER_CHARACTERISTICS,
    SEI_SUB_SEQ_CHARACTERISTICS,
    SEI_FULL_FRAME_FREEZE,
    SEI_FULL_FRAME_FREEZE_RELEASE,
    SEI_FULL_FRAME_SNAPSHOT,
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START,
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END,
    SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET,
    SEI_FILM_GRAIN_CHARACTERISTICS,
    SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE,
    SEI_STEREO_VIDEO_INFO,
    SEI_POST_FILTER_HINTS,
    SEI_TONE_MAPPING
} SEI_payloadType_e;

//! pic_struct in picture timing SEI message
typedef enum SEI_PicStructType_e
{
    SEI_PIC_STRUCT_FRAME             = 0, //!< frame
    SEI_PIC_STRUCT_TOP_FIELD         = 1, //!< top field
    SEI_PIC_STRUCT_BOTTOM_FIELD      = 2, //!< bottom field
    SEI_PIC_STRUCT_TOP_BOTTOM        = 3, //!< top field, bottom field, in that order
    SEI_PIC_STRUCT_BOTTOM_TOP        = 4, //!< bottom field, top field, in that order
    SEI_PIC_STRUCT_TOP_BOTTOM_TOP    = 5, //!< top field, bottom field, top field repeated, in that order
    SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM = 6, //!< bottom field, top field, bottom field repeated, in that order
    SEI_PIC_STRUCT_FRAME_DOUBLING    = 7, //!< frame doubling
    SEI_PIC_STRUCT_FRAME_TRIPLING    = 8  //!< frame tripling
} SEI_PicStructType_e;

/* ************************************************************************** */

static void scaling_list_4x4(DecodingContext_t *dc, int i);
static void scaling_list_8x8(DecodingContext_t *dc, int i);
static vui_t *decodeVUI(DecodingContext_t *dc);
static hrd_t *decodeHRD(DecodingContext_t *dc);

static int checkSPS(DecodingContext_t *dc, sps_t *sps);
static int checkPPS(DecodingContext_t *dc, pps_t *pps);
static int checkSEI(DecodingContext_t *dc, sei_t *sei);
static int checkVUI(DecodingContext_t *dc, vui_t *vui);
static int checkHRD(DecodingContext_t *dc, hrd_t *hrd);

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 1 if SPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.3.2.1 Sequence parameter set RBSP syntax.
 * 7.4.2.1 Sequence parameter set RBSP semantics.
 */
int decodeSPS(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeSPS()" CLR_RESET);
    int retcode = SUCCESS;

    // SPS allocation
    ////////////////////////////////////////////////////////////////////////////

    sps_t *sps = (sps_t*)calloc(1, sizeof(sps_t));
    if (sps == NULL)
    {
        TRACE_ERROR(PARAM, "Unable to alloc new SPS!");
        retcode = FAILURE;
    }
    else
    {
        // SPS decoding
        ////////////////////////////////////////////////////////////////////////

        sps->profile_idc = dc->profile_idc = read_bits(dc->bitstr, 8);

        sps->constraint_setX_flag[0] = read_bit(dc->bitstr);
        sps->constraint_setX_flag[1] = read_bit(dc->bitstr);
        sps->constraint_setX_flag[2] = read_bit(dc->bitstr);
        sps->constraint_setX_flag[3] = read_bit(dc->bitstr);
        sps->constraint_setX_flag[4] = read_bit(dc->bitstr);
        sps->constraint_setX_flag[5] = read_bit(dc->bitstr);

        if (read_bits(dc->bitstr, 2) != 0)
        {
            TRACE_ERROR(PARAM, "  Error while reading reserved_zero_2bits: must be 0!");
            freeSPS(&sps);
            return FAILURE;
        }

        sps->level_idc = read_bits(dc->bitstr, 8);
        sps->seq_parameter_set_id = read_ue(dc->bitstr);

        // Put SPS in decoding context
        freeSPS(&dc->sps_array[sps->seq_parameter_set_id]);
        dc->sps_array[sps->seq_parameter_set_id] = sps;
        dc->active_sps = sps->seq_parameter_set_id;

        // Handle parameters for profiles >= HIGH
        if (sps->profile_idc == FREXT_HiP || sps->profile_idc == FREXT_Hi10P ||
            sps->profile_idc == FREXT_Hi422 || sps->profile_idc == FREXT_Hi444 || sps->profile_idc == FREXT_CAVLC444 ||
            sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == MVC_HIGH ||
            sps->profile_idc == STEREO_HIGH)
        {
            sps->chroma_format_idc = read_ue(dc->bitstr);
            dc->ChromaArrayType = sps->ChromaArrayType = sps->chroma_format_idc;

            if (sps->chroma_format_idc == 0) // 4:0:0 subsampling
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc == 0, yuv 4:0:0)");
                return UNSUPPORTED;

                sps->SubHeightC = 0;
                sps->SubWidthC = 0;
            }
            else if (sps->chroma_format_idc == 1) // 4:2:0 subsampling
            {
                sps->SubHeightC = 2;
                sps->SubWidthC = 2;
            }
            else if (sps->chroma_format_idc == 2) // 4:2:2 subsampling
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc == 2, yuv 4:2:1)");
                return UNSUPPORTED;

                sps->SubHeightC = 1;
                sps->SubWidthC = 2;
            }
            else if (sps->chroma_format_idc == 3) // 4:4:4 subsampling
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc == 3, yuv 4:4:4)");
                return UNSUPPORTED;

                sps->separate_colour_plane_flag = read_bit(dc->bitstr);

                if (sps->separate_colour_plane_flag)
                {
                    TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == 1)");
                    return UNSUPPORTED;

                    sps->ChromaArrayType = dc->ChromaArrayType = 0;

                    sps->SubHeightC = 0;
                    sps->SubWidthC = 0;
                }
                else
                {
                    sps->SubHeightC = 1;
                    sps->SubWidthC = 1;
                }
            }

            sps->MbWidthC = 16 / sps->SubWidthC;
            sps->MbHeightC = 16 / sps->SubHeightC;

            sps->bit_depth_luma_minus8 = read_ue(dc->bitstr);
            sps->BitDepthY = 8 + sps->bit_depth_luma_minus8;
            sps->QpBdOffsetY = 6 * sps->bit_depth_luma_minus8;

            sps->bit_depth_chroma_minus8 = read_ue(dc->bitstr);
            sps->BitDepthC = 8 + sps->bit_depth_chroma_minus8;
            sps->QpBdOffsetC = 6 * sps->bit_depth_chroma_minus8;

            sps->RawMbBits = 256 * sps->BitDepthY + 2 * sps->MbWidthC * sps->MbHeightC * sps->BitDepthC;

            sps->qpprime_y_zero_transform_bypass_flag = read_bit(dc->bitstr);

            // Extract scaling list from bitstream
            sps->seq_scaling_matrix_present_flag = read_bit(dc->bitstr);
            if (sps->seq_scaling_matrix_present_flag)
            {
                int i = 0;
                for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++)
                {
                    sps->seq_scaling_list_present_flag[i] = read_bit(dc->bitstr);
                    if (sps->seq_scaling_list_present_flag[i])
                    {
                        if (i < 6)
                        {
                            scaling_list_4x4(dc, i);
                        }
                        else
                        {
                            scaling_list_8x8(dc, i - 6);
                        }
                    }
                }
            }
        }
        else // Set default parameters for profiles < HIGH
        {
            dc->ChromaArrayType = 1;
            sps->ChromaArrayType = 1;
            sps->chroma_format_idc = 1;
            sps->SubHeightC = sps->SubWidthC = 2;
            sps->MbHeightC = sps->MbWidthC = 8;

            sps->bit_depth_luma_minus8 = 0;
            sps->BitDepthY = 8;
            sps->QpBdOffsetY = 0;

            sps->bit_depth_chroma_minus8 = 0;
            sps->BitDepthC = 8;
            sps->QpBdOffsetC = 0;

            sps->RawMbBits = 3072;

            sps->qpprime_y_zero_transform_bypass_flag = false;
            sps->seq_scaling_matrix_present_flag = false;
        }

        // Initialize flat scaling list
        if (sps->seq_scaling_matrix_present_flag == false)
        {
            int i = 0, k = 0;
            for (i = 0; i < 6; i++)
            {
                for (k = 0; k < 16; k++)
                {
                    sps->ScalingList4x4[i][k] = 16;
                }

                for (k = 0; k < 64; k++)
                {
                    sps->ScalingList8x8[i][k] = 16;
                }

                // Transform the list into a matrix
                inverse_scan_4x4(sps->ScalingList4x4[i], sps->ScalingMatrix4x4[i]);
                inverse_scan_8x8(sps->ScalingList8x8[i], sps->ScalingMatrix8x8[i]);
            }
        }

        // Init some quantization tables
        computeLevelScale4x4(dc, sps);
        computeLevelScale8x8(dc, sps);

        sps->log2_max_frame_num_minus4 = read_ue(dc->bitstr);
        sps->MaxFrameNum = pow(2, sps->log2_max_frame_num_minus4 + 4);

        sps->pic_order_cnt_type = read_ue(dc->bitstr);
        if (sps->pic_order_cnt_type == 0)
        {
            sps->log2_max_pic_order_cnt_lsb_minus4 = read_ue(dc->bitstr);
            sps->MaxPicOrderCntLsb = pow(2, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        }
        else
        {
            sps->delta_pic_order_always_zero_flag = read_bit(dc->bitstr);
            sps->offset_for_non_ref_pic = read_se(dc->bitstr);
            sps->offset_for_top_to_bottom_field = read_se(dc->bitstr);
            sps->num_ref_frames_in_pic_order_cnt_cycle = read_ue(dc->bitstr);

            unsigned int i = 0;
            for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                sps->offset_for_ref_frame[i] = read_se(dc->bitstr);
            }
        }

        sps->max_num_ref_frames = read_ue(dc->bitstr);
        sps->gaps_in_frame_num_value_allowed_flag = read_bit(dc->bitstr);

        sps->pic_width_in_mbs_minus1 = read_ue(dc->bitstr);
        sps->PicWidthInMbs = sps->pic_width_in_mbs_minus1 + 1;
        sps->PicWidthInSamplesL = sps->PicWidthInMbs * 16;
        sps->PicWidthInSamplesC = sps->PicWidthInMbs * sps->MbWidthC;

        sps->pic_height_in_map_units_minus1 = read_ue(dc->bitstr);
        sps->PicHeightInMapUnits = sps->pic_height_in_map_units_minus1 + 1;
        sps->PicSizeInMapUnits = sps->PicWidthInMbs * sps->PicHeightInMapUnits;

        sps->frame_mbs_only_flag = read_bit(dc->bitstr);
        if (sps->frame_mbs_only_flag == false)
        {
            sps->mb_adaptive_frame_field_flag = read_bit(dc->bitstr);
        }
        sps->FrameHeightInMbs = (2 - sps->frame_mbs_only_flag) * sps->PicHeightInMapUnits;

        // Macroblocks number
        dc->PicSizeInMbs = sps->FrameHeightInMbs * sps->PicWidthInMbs;

            //FIXME desalloc on a new SPS

            // Macroblocks "table" allocation (on macroblock **mbs_data):
            dc->mb_array = (Macroblock_t**)calloc(dc->PicSizeInMbs, sizeof(Macroblock_t*));

            // Macroblocks "chunk" allocation (on Macroblock_t *mbs_data):
            //dc->mbs_data = (macroblock*)calloc(dc->PicSizeInMbs, sizeof(macroblock));

        sps->direct_8x8_inference_flag = read_bit(dc->bitstr);

        sps->frame_cropping_flag = read_bit(dc->bitstr);
        if (sps->frame_cropping_flag)
        {
            sps->frame_crop_left_offset = read_ue(dc->bitstr);
            sps->frame_crop_right_offset = read_ue(dc->bitstr);
            sps->frame_crop_top_offset = read_ue(dc->bitstr);
            sps->frame_crop_bottom_offset = read_ue(dc->bitstr);

            if (!sps->ChromaArrayType)
            {
                sps->CropUnitX = 1;
                sps->CropUnitY = 2 - sps->frame_mbs_only_flag;
            }
            else
            {
                sps->CropUnitX = sps->SubWidthC;
                sps->CropUnitY = sps->SubHeightC * (2 - sps->frame_mbs_only_flag);
            }
        }

        sps->vui_parameters_present_flag = read_bit(dc->bitstr);
        if (sps->vui_parameters_present_flag)
        {
            // Decode VUI
            sps->vui = decodeVUI(dc);
        }

        // SPS check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkSPS(dc, sps);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param **sps_ptr A pointer to the SPS structure we want to freed.
 */
void freeSPS(sps_t **sps_ptr)
{
    if (*sps_ptr != NULL)
    {
        if ((*sps_ptr)->vui != NULL)
        {
            if ((*sps_ptr)->vui->hrd != NULL)
            {
                free((*sps_ptr)->vui->hrd);
            }

            free((*sps_ptr)->vui);
        }

        {
            free(*sps_ptr);
            *sps_ptr = NULL;

            TRACE_1(PARAM, ">> SPS freed");
        }
    }
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *sps (sequence_parameter_set) data structure.
 * \return 1 if SPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.1 Sequence parameter set RBSP semantics.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkSPS(DecodingContext_t *dc, sps_t *sps)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "checkSPS()" CLR_RESET);
    int retcode = SUCCESS;

    // Check SPS structure
    if (sps == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid SPS structure!");
        return FAILURE;
    }
    else // Check SPS values
    {
        if (sps->seq_parameter_set_id > 31)
        {
            TRACE_WARNING(PARAM, "  - seq_parameter_set_id is %i but should be in range [0,31]", sps->seq_parameter_set_id);
            retcode = FAILURE;
        }

        if (sps->profile_idc != MAINP && sps->profile_idc != FREXT_HiP && sps->profile_idc != BASELINE)
        {
            TRACE_WARNING(PARAM, "  - profile_idc is %i", sps->profile_idc);
            TRACE_WARNING(PARAM, "  <!> Only MAIN and HIGH profiles are supported by this decoder");

            TRACE_ERROR(PARAM, ">>> UNSUPPORTED (profile_idc != 77 && profile_idc != 100)");
            return UNSUPPORTED;
        }

        if (sps->profile_idc == FREXT_HiP || sps->profile_idc == FREXT_Hi10P ||
            sps->profile_idc == FREXT_Hi422 || sps->profile_idc == FREXT_Hi444 || sps->profile_idc == FREXT_CAVLC444 ||
            sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == MVC_HIGH ||
            sps->profile_idc == STEREO_HIGH)
        {
            if (sps->chroma_format_idc > 3)
            {
                TRACE_WARNING(PARAM, "  - chroma_format_idc is %i but should be in range [0,3]", sps->chroma_format_idc);
                retcode = FAILURE;
            }
            else if (sps->chroma_format_idc > 1)
            {
                TRACE_WARNING(PARAM, "  - chroma_format_idc is %i", sps->chroma_format_idc);
                TRACE_ERROR(PARAM, "  <!> Only 4:0:0 (0) and 4:2:0 (1) chroma_format_idc are supported by this decoder");

                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc > 1)");
                return UNSUPPORTED;
            }

            if (sps->separate_colour_plane_flag)
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == true)");
                return UNSUPPORTED;
            }

            if (sps->bit_depth_luma_minus8 > 6)
            {
                TRACE_WARNING(PARAM, "  - bit_depth_luma_minus8 is %i but should be in range [0,6]", sps->bit_depth_luma_minus8);
                retcode = FAILURE;
            }

            if (sps->bit_depth_chroma_minus8 > 6)
            {
                TRACE_WARNING(PARAM, "  - bit_depth_chroma_minus8 is %i but should be in range [0,6]", sps->bit_depth_luma_minus8);
                retcode = FAILURE;
            }
        }

        if (sps->log2_max_frame_num_minus4 > 12)
        {
            TRACE_WARNING(PARAM, "  - log2_max_frame_num_minus4 is %i but should be in range [0,12]", sps->log2_max_frame_num_minus4);
            retcode = FAILURE;
        }

        if (sps->pic_order_cnt_type == 0)
        {
            if  (sps->log2_max_pic_order_cnt_lsb_minus4 > 12)
            {
                TRACE_WARNING(PARAM, "  - log2_max_pic_order_cnt_lsb_minus4 is %i but should be in range [0,12]", sps->log2_max_pic_order_cnt_lsb_minus4);
                retcode = FAILURE;
            }
        }
        else if (sps->pic_order_cnt_type == 1)
        {
            if (sps->offset_for_non_ref_pic < (pow(-2, 31) + 1) ||  sps->offset_for_non_ref_pic < (pow(2, 31) - 1))
            {
                TRACE_WARNING(PARAM, "  - offset_for_non_ref_pic is %i but should be in range [-2147483647,2147483647]", sps->offset_for_non_ref_pic);
                retcode = FAILURE;
            }

            if (sps->offset_for_top_to_bottom_field < (pow(-2, 31) + 1) ||  sps->offset_for_top_to_bottom_field < (pow(2, 31) - 1))
            {
                TRACE_WARNING(PARAM, "  - offset_for_top_to_bottom_field is %i but should be in range [-2147483647,2147483647]", sps->offset_for_top_to_bottom_field);
                retcode = FAILURE;
            }

            if (sps->num_ref_frames_in_pic_order_cnt_cycle > 255)
            {
                TRACE_WARNING(PARAM, "  - num_ref_frames_in_pic_order_cnt_cycle is %i but should be in range [0,255]", sps->num_ref_frames_in_pic_order_cnt_cycle);
                retcode = FAILURE;
            }

            unsigned int i = 0;
            for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                if (sps->offset_for_ref_frame[i] < (pow(-2, 31) + 1) ||  sps->offset_for_ref_frame[i] < (pow(2, 31) - 1))
                {
                    TRACE_WARNING(PARAM, "  - offset_for_ref_frame[i] is %i but should be in range [-2147483647,2147483647]", sps->offset_for_top_to_bottom_field);
                    retcode = FAILURE;
                }
            }
        }
        else if (sps->pic_order_cnt_type > 2)
        {
            TRACE_WARNING(PARAM, "  - pic_order_cnt_type is %i but should be in range [0,2]", sps->pic_order_cnt_type);
            retcode = FAILURE;
        }

        //FIXME The value of max_num_ref_frames shall be in the range of 0 to MaxDpbFrames (as specified insubclause A.3.1 or A.3.2)
        //TRACE_WARNING(PARAM, "  - max_num_ref_frames is %i but should be in range [1,x]", sps->max_num_ref_frames);

        //FIXME allowed range of values specified by constraints in Annex A.
        //TRACE_1(PARAM, "  - pic_width_in_mbs_minus1       : %i", sps->pic_width_in_mbs_minus1);
        //TRACE_1(PARAM, "  - pic_height_in_map_units_minus1: %i", sps->pic_height_in_map_units_minus1);

        if (sps->frame_mbs_only_flag == false && sps->direct_8x8_inference_flag == false)
        {
            TRACE_WARNING(PARAM, "  - direct_8x8_inference_flag is set to 0, but should be 1, because mb_adaptive_frame_field_flag is set to 1");
            retcode = FAILURE;
        }

        if (sps->mb_adaptive_frame_field_flag)
        {
            TRACE_ERROR(PARAM, ">>> UNSUPPORTED (mb_adaptive_frame_field_flag)");
            return UNSUPPORTED;
        }

        if (sps->frame_cropping_flag)
        {
            //FIXME check that
            //TRACE_1(PARAM, "  - frame_crop_left_offset    : %i", sps->frame_crop_left_offset);
            //TRACE_1(PARAM, "  - frame_crop_right_offset   : %i", sps->frame_crop_right_offset);
            //TRACE_1(PARAM, "  - frame_crop_top_offset     : %i", sps->frame_crop_top_offset);
            //TRACE_1(PARAM, "  - frame_crop_bottom_offset  : %i", sps->frame_crop_bottom_offset);
        }

        // Check VUI content
        if (retcode == SUCCESS && sps->vui_parameters_present_flag == true)
        {
            retcode = checkVUI(dc, sps->vui);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about sequence_parameter_set decoding.
 * \param *dc The current DecodingContext.
 */
void printSPS(DecodingContext_t *dc)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printSPS()" CLR_RESET);

    unsigned int i = 0;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];

    // Check SPS structure
    if (sps == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid SPS structure!");
        return;
    }

    // Print SPS values
    TRACE_1(PARAM, "  - profile_idc     = %i", sps->profile_idc);
    for (i = 0; i < 6; i++)
    {
        TRACE_1(PARAM, "  - constraint_setX_flag[%i]= %i", i, sps->constraint_setX_flag[i]);
    }
    TRACE_1(PARAM, "  - level_idc       = %i", sps->level_idc);
    TRACE_1(PARAM, "  - seq_parameter_set_id= %i", sps->seq_parameter_set_id);

    TRACE_1(PARAM, "  - chroma_format_idc   = %i", sps->chroma_format_idc);
    TRACE_1(PARAM, "  - ChromaArrayType : %i", sps->ChromaArrayType);
    TRACE_1(PARAM, "  - SubWidthC       : %i", sps->SubWidthC);
    TRACE_1(PARAM, "  - SubHeightC      : %i", sps->SubHeightC);
    TRACE_1(PARAM, "  - MbWidthC        : %i", sps->MbWidthC);
    TRACE_1(PARAM, "  - MbHeightC       : %i", sps->MbHeightC);

    if (sps->chroma_format_idc == 3)
    {
        TRACE_1(PARAM, "  - separate_colour_plane_flag= %i", sps->separate_colour_plane_flag);
    }

    TRACE_1(PARAM, "  - bit_depth_luma_minus8   = %i", sps->bit_depth_luma_minus8);
    TRACE_1(PARAM, "  - BitDepthY               : %i", sps->BitDepthY);
    TRACE_1(PARAM, "  - QpBdOffsetY             : %i", sps->QpBdOffsetY);
    TRACE_1(PARAM, "  - bit_depth_chroma_minus8 = %i", sps->bit_depth_chroma_minus8);
    TRACE_1(PARAM, "  - BitDepthC               : %i", sps->BitDepthC);
    TRACE_1(PARAM, "  - QpBdOffsetC             : %i", sps->QpBdOffsetC);
    TRACE_1(PARAM, "  - RawMbBits               : %i", sps->RawMbBits);
    TRACE_1(PARAM, "  - qpprime_y_zero_transform_bypass_flag= %i", sps->qpprime_y_zero_transform_bypass_flag);

    TRACE_1(PARAM, "  - seq_scaling_matrix_present_flag     = %i", sps->seq_scaling_matrix_present_flag);
    if (sps->seq_scaling_matrix_present_flag)
    {
         for (i = 0; i < ((dc->ChromaArrayType != 3) ? 8 : 12); i++)
         {
             TRACE_1(PARAM, "  - seq_scaling_list_present_flag[%i]= %i", i, sps->seq_scaling_list_present_flag[i]);
         }
    }

    TRACE_1(PARAM, "  - log2_max_frame_num_minus4   = %i", sps->log2_max_frame_num_minus4);
    TRACE_1(PARAM, "  - MaxFrameNum                 : %i", sps->MaxFrameNum);
    TRACE_1(PARAM, "  - pic_order_cnt_type          = %i", sps->pic_order_cnt_type);
    if (sps->pic_order_cnt_type == 0)
    {
        TRACE_1(PARAM, "  - log2_max_pic_order_cnt_lsb_minus4= %i", sps->log2_max_pic_order_cnt_lsb_minus4);
        TRACE_1(PARAM, "  - MaxPicOrderCntLsb           : %i", sps->MaxPicOrderCntLsb);
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        TRACE_1(PARAM, "  - delta_pic_order_always_zero_flag= %i", sps->delta_pic_order_always_zero_flag);
        TRACE_1(PARAM, "  - offset_for_non_ref_pic          = %i", sps->offset_for_non_ref_pic);
        TRACE_1(PARAM, "  - offset_for_top_to_bottom_field  = %i", sps->offset_for_top_to_bottom_field);
        TRACE_1(PARAM, "  - num_ref_frames_in_pic_order_cnt_cycle= %i", sps->num_ref_frames_in_pic_order_cnt_cycle);

        for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            TRACE_1(PARAM, "  - offset_for_ref_frame[i]         = %i", i, sps->offset_for_ref_frame[i]);
        }
    }

    TRACE_1(PARAM, "  - max_num_ref_frames      = %i", sps->max_num_ref_frames);
    TRACE_1(PARAM, "  - gaps_in_frame_num_value_allowed_flag= %i", sps->gaps_in_frame_num_value_allowed_flag);
    TRACE_1(PARAM, "  - pic_width_in_mbs_minus1 = %i", sps->pic_width_in_mbs_minus1);
    TRACE_1(PARAM, "  - PicWidthInMbs           : %i", sps->PicWidthInMbs);
    TRACE_1(PARAM, "  - PicWidthInSamplesL      : %i", sps->PicWidthInSamplesL);
    TRACE_1(PARAM, "  - PicWidthInSamplesC      : %i", sps->PicWidthInSamplesC);
    TRACE_1(PARAM, "  - pic_height_in_map_units_minus1= %i", sps->pic_height_in_map_units_minus1);
    TRACE_1(PARAM, "  - frame_mbs_only_flag     = %i", sps->frame_mbs_only_flag);
    TRACE_1(PARAM, "  - FrameHeightInMbs        : %i", sps->FrameHeightInMbs);

    if (sps->frame_mbs_only_flag == false)
    {
        TRACE_1(PARAM, "  - mb_adaptive_frame_field_flag= %i", sps->mb_adaptive_frame_field_flag);
    }

    TRACE_1(PARAM, "  - direct_8x8_inference_flag= %i", sps->direct_8x8_inference_flag);

    TRACE_1(PARAM, "  - frame_cropping_flag     = %i", sps->frame_cropping_flag);
    if (sps->frame_cropping_flag)
    {
        TRACE_1(PARAM, "  - frame_crop_left_offset      = %i", sps->frame_crop_left_offset);
        TRACE_1(PARAM, "  - frame_crop_right_offset     = %i", sps->frame_crop_right_offset);
        TRACE_1(PARAM, "  - frame_crop_top_offset       = %i", sps->frame_crop_top_offset);
        TRACE_1(PARAM, "  - frame_crop_bottom_offset    = %i", sps->frame_crop_bottom_offset);
        TRACE_1(PARAM, "  - CropUnitX   : %i", sps->CropUnitX);
        TRACE_1(PARAM, "  - CropUnitY   : %i", sps->CropUnitY);
    }

    TRACE_1(PARAM, "  - vui_parameters_present_flag= %i", sps->vui_parameters_present_flag);
    if (sps->vui_parameters_present_flag)
    {
        printVUI(sps->vui);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Get custom scaling list from the bitstream.
 * \param *dc The current DecodingContext.
 * \param i The scaling list id [1;6].
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.1.1.1 Scaling list semantics.
 * 8.5.9 Derivation process for scaling functions.
 */
static void scaling_list_4x4(DecodingContext_t *dc, int i)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "scaling_list_4x4()" CLR_RESET);

    sps_t *sps = NULL;
    int lastScale = 8;
    int nextScale = 8;
    int delta_scale = 0;
    int j = 0;

    if (dc->pps_array[0] == NULL || dc->active_slice == NULL)
        sps = dc->sps_array[0];
    else
        sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];

    for (j = 0; j < 16; j++)
    {
        if (nextScale != 0)
        {
            delta_scale = read_se(dc->bitstr);
            nextScale = (lastScale + delta_scale + 256) % 256;
            sps->UseDefaultScalingMatrix4x4Flag[i] = (bool)(j == 0 && nextScale == 0);
        }

        sps->ScalingList4x4[i][j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = sps->ScalingList4x4[i][j];

        //TRACE_3(PARAM, "ScalingList4x4 : %i", sps->ScalingList4x4[i][j])
    }

    // Transform the list into a matrix
    inverse_scan_4x4(sps->ScalingList4x4[i], sps->ScalingMatrix4x4[i]);
}

/* ************************************************************************** */

/*!
 * \brief Get custom scaling list from the bitstream.
 * \param *dc The current DecodingContext.
 * \param i The scaling list id [1;6].
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.1.1.1 Scaling list semantics.
 * 8.5.9 Derivation process for scaling functions.
 */
static void scaling_list_8x8(DecodingContext_t *dc, int i)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "scaling_list_8x8()" CLR_RESET);

    sps_t *sps = NULL;
    int lastScale = 8;
    int nextScale = 8;
    int j = 0;

    if (dc->pps_array[0] == NULL || dc->active_slice == NULL)
        sps = dc->sps_array[0];
    else
        sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];

    for (j = 0; j < 64; j++)
    {
        if (nextScale != 0)
        {
            int delta_scale = read_se(dc->bitstr);
            nextScale = (lastScale + delta_scale + 256) % 256;
            sps->UseDefaultScalingMatrix8x8Flag[i] = (bool)(j == 0 && nextScale == 0);
        }

        sps->ScalingList8x8[i][j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = sps->ScalingList8x8[i][j];

        //TRACE_3(PARAM, "ScalingList8x8 : %i", sps->ScalingList8x8[i][j])
    }

    // Transform the list into a matrix
    inverse_scan_8x8(sps->ScalingList8x8[i], sps->ScalingMatrix8x8[i]);
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 1 if PPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.3.2.2 Picture parameter set RBSP syntax.
 * 7.4.2.2 Picture parameter set RBSP semantics.
 */
int decodePPS(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodePPS()" CLR_RESET);
    int retcode = SUCCESS;

    // PPS allocation
    ////////////////////////////////////////////////////////////////////////////

    pps_t *pps = (pps_t*)calloc(1, sizeof(pps_t));
    if (pps == NULL)
    {
        TRACE_ERROR(PARAM, "Unable to alloc new PPS!");
        retcode = FAILURE;
    }
    else
    {
        // PPS decoding
        ////////////////////////////////////////////////////////////////////////

        pps->pic_parameter_set_id = read_ue(dc->bitstr);
        pps->seq_parameter_set_id = read_ue(dc->bitstr);

        // Put pps in decoding context
        dc->active_pps = pps->seq_parameter_set_id;
        freePPS(&dc->pps_array[dc->active_pps]);
        dc->pps_array[dc->active_pps] = pps;

        pps->entropy_coding_mode_flag = read_bit(dc->bitstr);
        dc->entropy_coding_mode_flag = pps->entropy_coding_mode_flag; // Shortcut
        pps->bottom_field_pic_order_in_frame_present_flag = read_bit(dc->bitstr);

        pps->num_slice_groups_minus1 = read_ue(dc->bitstr);
        if (pps->num_slice_groups_minus1 > 0)
        {
#if ENABLE_FMO
            pps->slice_group_map_type = read_ue(dc->bitstr);
            if (pps->slice_group_map_type == 0)
            {
                int iGroup = 0;
                for (iGroup = 0; iGroup <= pps->num_slice_groups_minus1; iGroup++)
                {
                    pps->run_length_minus1[iGroup] = read_ue(dc->bitstr);
                }
            }
            else if (pps->slice_group_map_type == 2)
            {
                int iGroup = 0;
                for (iGroup = 0; iGroup < pps->num_slice_groups_minus1; iGroup++)
                {
                    pps->top_left[iGroup] = read_ue(dc->bitstr);
                    pps->bottom_right[iGroup] = read_ue(dc->bitstr);
                }
            }
            else if (pps->slice_group_map_type == 3 ||
                     pps->slice_group_map_type == 4 ||
                     pps->slice_group_map_type == 5)
            {
                pps->slice_group_change_direction_flag = read_bit(dc->bitstr);
                pps->slice_group_change_rate_minus1 = read_ue(dc->bitstr);
            }
            else if (pps->slice_group_map_type == 6)
            {
                pps->pic_size_in_map_units_minus1 = read_ue(dc->bitstr);
                int i = 0;
                for (i = 0; i <= pps->pic_size_in_map_units_minus1; i++)
                {
                    //pps->slice_group_id[i] = read_bits(dc->bitstr, ceil(log2(pps->num_slice_groups_minus1 + 1)));
                }
            }
#else // ENABLE_FMO
            TRACE_ERROR(PARAM, ">>> UNSUPPORTED (FMO)");
            return UNSUPPORTED;
#endif // ENABLE_FMO
        }

        pps->num_ref_idx_l0_default_active_minus1 = read_ue(dc->bitstr);
        pps->num_ref_idx_l1_default_active_minus1 = read_ue(dc->bitstr);
        pps->weighted_pred_flag = read_bit(dc->bitstr);
        pps->weighted_bipred_idc = read_bits(dc->bitstr, 2);
        pps->pic_init_qp_minus26 = read_se(dc->bitstr);
        pps->pic_init_qs_minus26 = read_se(dc->bitstr);
        pps->chroma_qp_index_offset = read_se(dc->bitstr);
        pps->deblocking_filter_control_present_flag = read_bit(dc->bitstr);
        pps->constrained_intra_pred_flag = read_bit(dc->bitstr);
        pps->redundant_pic_cnt_present_flag = read_bit(dc->bitstr);

        if (h264_more_rbsp_data(dc->bitstr) == true &&
            dc->sps_array[pps->seq_parameter_set_id]->profile_idc >= FREXT_HiP)
        {
            pps->transform_8x8_mode_flag = read_bit(dc->bitstr);

            pps->pic_scaling_matrix_present_flag = read_bit(dc->bitstr);
            if (pps->pic_scaling_matrix_present_flag)
            {
#if ENABLE_FMO
                int i = 0;
                for (i = 0; i < 6 + ((dc->sps_array[pps->seq_parameter_set_id]->chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag; i++)
                {
                    pps->pic_scaling_list_present_flag[i] = read_bit(dc->bitstr); //FIXME?
                    if (pps->pic_scaling_list_present_flag[i])
                    {
                        if (i < 6)
                            scaling_list(dc->sps_array[pps->seq_parameter_set_id]->ScalingList4x4[i], 16,
                                         dc->sps_array[pps->seq_parameter_set_id]->UseDefaultScalingMatrix4x4Flag[i]);
                        else
                            scaling_list(dc->sps_array[pps->seq_parameter_set_id]->ScalingList8x8[i-6], 64,
                                         dc->sps_array[pps->seq_parameter_set_id]->UseDefaultScalingMatrix8x8Flag[i-6]);
                    }
                }
#else // ENABLE_FMO
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (FMO)");
                return UNSUPPORTED;
#endif // ENABLE_FMO
            }

            pps->second_chroma_qp_index_offset = read_se(dc->bitstr);
        }
        else
        {
            pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
        }

        // PPS check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkPPS(dc, pps);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param **pps_ptr A pointer to the pps structure we want to freed.
 */
void freePPS(pps_t **pps_ptr)
{
    if (*pps_ptr != NULL)
    {
        free(*pps_ptr);
        *pps_ptr = NULL;

        TRACE_1(PARAM, ">> PPS freed");
    }
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *pps picture_parameter_set.
 * \return 1 if PPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.2 Picture parameter set RBSP semantics.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkPPS(DecodingContext_t *dc, pps_t *pps)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "checkPPS()" CLR_RESET);
    int retcode = SUCCESS;

    // Check PPS structure
    if (pps == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid PPS structure!");
        retcode = FAILURE;
    }
    else // Check PPS values
    {
        if (pps->pic_parameter_set_id > 255)
        {
            TRACE_WARNING(PARAM, "  - pic_parameter_set_id is %i but should be in range [0,255]", pps->seq_parameter_set_id);
            retcode = FAILURE;
        }

        if (pps->seq_parameter_set_id > 31)
        {
            TRACE_WARNING(PARAM, "  - seq_parameter_set_id is %i but should be in range [0,31]", pps->seq_parameter_set_id);
            retcode = FAILURE;
        }

        if (pps->num_slice_groups_minus1 > 7)
        {
            //FIXME should be 0 for main & high profile

            TRACE_WARNING(PARAM, "  - num_slice_groups_minus1 is %i but should be in range [0,7]", pps->num_slice_groups_minus1);
            retcode = FAILURE;
        }
        else if (pps->num_slice_groups_minus1 > 0)
        {
            TRACE_ERROR(PARAM, "  - FMO is not implemented. Decoding aborted!");
            return UNSUPPORTED;
        }

        if (pps->num_ref_idx_l0_default_active_minus1 > 31)
        {
            TRACE_WARNING(PARAM, "  - num_ref_idx_l0_default_active_minus1 is %i but should be in range [0,31]", pps->num_ref_idx_l0_default_active_minus1);
            retcode = FAILURE;
        }

        if (pps->num_ref_idx_l1_default_active_minus1 > 31)
        {
            TRACE_WARNING(PARAM, "  - num_ref_idx_l1_default_active_minus1 is %i but should be in range [0,31]", pps->num_ref_idx_l1_default_active_minus1);
            retcode = FAILURE;
        }

        if (pps->weighted_bipred_idc > 2)
        {
            TRACE_WARNING(PARAM, "  - weighted_bipred_idc is %i but should be in range [0,2]", pps->weighted_bipred_idc);
            retcode = FAILURE;
        }
/*
        if ((pps->pic_init_qp_minus26 < -(26 + dc->sps_array[pps->seq_parameter_set_id]->QpBdOffsetY)) || pps->pic_init_qp_minus26 > 25)
        {
            TRACE_WARNING(PARAM, "  - pic_init_qp_minus26 is %i but should be in range [-(26 + QpBdOffsetY),25]", pps->pic_init_qp_minus26);
            retcode = FAILURE;
        }
*/
        if (pps->pic_init_qs_minus26 < -26 || pps->pic_init_qs_minus26  > 25)
        {
            TRACE_WARNING(PARAM, "  - pic_init_qs_minus26 is %i but should be in range [-26,+25]", pps->pic_init_qs_minus26);
            retcode = FAILURE;
        }

        if (pps->chroma_qp_index_offset < -12 || pps->chroma_qp_index_offset  > 12)
        {
            TRACE_WARNING(PARAM, "  - chroma_qp_index_offset is %i but should be in range [-12,+12]", pps->chroma_qp_index_offset);
            retcode = FAILURE;
        }

        if (pps->second_chroma_qp_index_offset < -12 || pps->second_chroma_qp_index_offset  > 12)
        {
            TRACE_WARNING(PARAM, "  - second_chroma_qp_index_offset is %i but should be in range [-12,+12]", pps->second_chroma_qp_index_offset);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about picture_parameter_set decoding.
 * \param *dc The current DecodingContext.
 */
void printPPS(DecodingContext_t *dc)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printPPS()" CLR_RESET);

    unsigned int i = 0;

    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_pps];

    // Check PPS structure
    if (pps == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid PPS structure!");
        return;
    }

    // Print PPS values
    TRACE_1(PARAM, "  - pic_parameter_set_id        = %i", pps->pic_parameter_set_id);
    TRACE_1(PARAM, "  - seq_parameter_set_id        = %i", pps->seq_parameter_set_id);
    TRACE_1(PARAM, "  - entropy_coding_mode_flag    = %i", pps->entropy_coding_mode_flag);
    TRACE_1(PARAM, "  - bottom_field_pic_order_in_frame_present_flag = %i", pps->bottom_field_pic_order_in_frame_present_flag);
    TRACE_1(PARAM, "  - num_slice_groups_minus1     = %i", pps->num_slice_groups_minus1);
    if (pps->num_slice_groups_minus1 > 0)
    {
        unsigned int iGroup = 0;
        TRACE_1(PARAM, "  - slice_group_map_type    = %i", pps->slice_group_map_type);
        if (pps->slice_group_map_type == 0)
        {
            for (iGroup = 0; iGroup <= pps->num_slice_groups_minus1; iGroup++)
            {
//                TRACE_1(PARAM, "  - run_length_minus1[%i]= %i", iGroup, pps->run_length_minus1[iGroup]);
            }
        }
        else if (pps->slice_group_map_type == 2)
        {
            for (iGroup = 0; iGroup < pps->num_slice_groups_minus1; iGroup++)
            {
//                TRACE_1(PARAM, "  - top_left[%i]      = %i", iGroup, pps->top_left[iGroup]);
//                TRACE_1(PARAM, "  - bottom_right[%i]  = %i", iGroup, pps->bottom_right[iGroup]);
            }
        }
        else if (pps->slice_group_map_type == 3 ||
                 pps->slice_group_map_type == 4 ||
                 pps->slice_group_map_type == 5)
        {
            TRACE_1(PARAM, "  - slice_group_change_direction_flag= %i", pps->slice_group_change_direction_flag);
            TRACE_1(PARAM, "  - slice_group_change_rate_minus1   = %i", pps->slice_group_change_rate_minus1);
        }
        else if (pps->slice_group_map_type == 6)
        {
            TRACE_1(PARAM, "  - pic_size_in_map_units_minus1    = %i", pps->pic_size_in_map_units_minus1);
            for (i = 0; i <= pps->pic_size_in_map_units_minus1; i++)
            {
//                TRACE_1(PARAM, "  - slice_group_id[%i]        = %i", i, pps->slice_group_id[i]);
            }
        }
    }

    TRACE_1(PARAM, "  - num_ref_idx_l0_default_active_minus1= %i", pps->num_ref_idx_l0_default_active_minus1);
    TRACE_1(PARAM, "  - num_ref_idx_l1_default_active_minus1= %i", pps->num_ref_idx_l1_default_active_minus1);
    TRACE_1(PARAM, "  - weighted_pred_flag      = %i", pps->weighted_pred_flag);
    TRACE_1(PARAM, "  - weighted_bipred_idc     = %i", pps->weighted_bipred_idc);
    TRACE_1(PARAM, "  - pic_init_qp_minus26     = %i", pps->pic_init_qp_minus26);
    TRACE_1(PARAM, "  - pic_init_qs_minus26     = %i", pps->pic_init_qs_minus26);
    TRACE_1(PARAM, "  - chroma_qp_index_offset  = %i", pps->chroma_qp_index_offset);
    TRACE_1(PARAM, "  - deblocking_filter_control_present_flag= %i", pps->deblocking_filter_control_present_flag);
    TRACE_1(PARAM, "  - constrained_intra_pred_flag     = %i", pps->constrained_intra_pred_flag);
    TRACE_1(PARAM, "  - redundant_pic_cnt_present_flag  = %i", pps->redundant_pic_cnt_present_flag);

    TRACE_1(PARAM, "  - transform_8x8_mode_flag         = %i", pps->transform_8x8_mode_flag);
    TRACE_1(PARAM, "  - pic_scaling_matrix_present_flag = %i", pps->pic_scaling_matrix_present_flag);
    if (pps->pic_scaling_matrix_present_flag)
    {
         for (i = 0; i < ((dc->ChromaArrayType != 3) ? 8 : 12); i++)
         {
             TRACE_1(PARAM, "  - pic_scaling_list_present_flag[%i]= %i", i, pps->pic_scaling_list_present_flag[i]);
         }
    }
    TRACE_1(PARAM, "  - second_chroma_qp_index_offset   = %i", pps->second_chroma_qp_index_offset);
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse Access Unit Delimiter.
 * \param *dc The current DecodingContext.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.3.2.4 Access unit delimiter RBSP syntax.
 * 7.4.2.4 Access unit delimiter RBSP semantics.
 */
int decodeAUD(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeAUD()" CLR_RESET);
    int retcode = SUCCESS;

    unsigned int primary_pic_type = read_bits(dc->bitstr, 3);
    TRACE_2(PARAM, "<> " BLD_GREEN "primary_pic_type: %i" CLR_RESET, primary_pic_type);

    retcode = h264_rbsp_trailing_bits(dc->bitstr);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 */
int decodeSEI(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeSEI()" CLR_RESET);
    int retcode = SUCCESS;

    // SEI allocation
    ////////////////////////////////////////////////////////////////////////////

    sei_t *sei = (sei_t*)calloc(1, sizeof(sei_t));
    if (sei == NULL)
    {
        TRACE_ERROR(PARAM, "Unable to alloc new SEI!");
        retcode = FAILURE;
    }
    else
    {
        // SEI decoding
        ////////////////////////////////////////////////////////////////////////

        // Put sei in decoding context
        dc->active_sei = sei;

        TRACE_WARNING(PARAM, ">>> UNIMPLEMENTED (SEI decoding)");
        retcode = FAILURE;

        // SEI check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkSEI(dc, sei);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *sei (supplemental_enhancement_information) data structure.
 * \return 1 if SEI seems consistent, 0 otherwise.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkSEI(DecodingContext_t *dc, sei_t *sei)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "checkSEI()" CLR_RESET);
    int retcode = SUCCESS;

    // Check SEI structure
    if (sei == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid SEI structure!");
        retcode = FAILURE;
    }
    else // Check SEI values
    {
        TRACE_WARNING(PARAM, ">>> UNIMPLEMENTED (checkSEI)");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param **sei_ptr A pointer to the SEI structure we want to freed.
 */
void freeSEI(sei_t  **sei_ptr)
{
    if (*sei_ptr != NULL)
    {
        free(*sei_ptr);
        *sei_ptr = NULL;

        TRACE_1(PARAM, ">> SEI freed");
    }
}

/* ************************************************************************** */

/*!
 * \brief Print informations about supplemental_enhancement_information decoding.
 * \param *dc The current DecodingContext.
 */
void printSEI(DecodingContext_t *dc)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printSEI()" CLR_RESET);

    // Shortcut
    sei_t *sei = dc->active_sei;

    // Check SEI structure
    if (sei == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid SEI structure!");
        return;
    }

    // Print SEI values
    TRACE_WARNING(PARAM, ">>> UNIMPLEMENTED (printSEI)");

#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return vui_t initialized data structure.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.1 VUI parameters syntax.
 * E.2.1 VUI parameters semantics.
 *
 * VUI can only be found inside SPS structure.
 * This function must only be called by decodeSPS().
 */
static vui_t *decodeVUI(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "  > " BLD_GREEN "decodeVUI()" CLR_RESET);

    // VUI allocation
    ////////////////////////////////////////////////////////////////////////////
    vui_t *vui = (vui_t*)calloc(1, sizeof(vui_t));

    if (vui == NULL)
    {
        TRACE_ERROR(PARAM, "Unable to alloc new VUI!");
    }
    else
    {
        // VUI decoding
        ////////////////////////////////////////////////////////////////////////////

        vui->aspect_ratio_info_present_flag = read_bit(dc->bitstr);
        if (vui->aspect_ratio_info_present_flag)
        {
            vui->aspect_ratio_idc = read_bits(dc->bitstr, 8);
            if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
            {
                vui->sar_width = read_bits(dc->bitstr, 16);
                vui->sar_height = read_bits(dc->bitstr, 16);
            }
        }

        vui->overscan_info_present_flag = read_bit(dc->bitstr);
        if (vui->overscan_info_present_flag)
        {
            vui->overscan_appropriate_flag = read_bit(dc->bitstr);
        }

        vui->video_signal_type_present_flag = read_bit(dc->bitstr);
        if (vui->video_signal_type_present_flag)
        {
            vui->video_format = read_bits(dc->bitstr, 3);
            vui->video_full_range_flag = read_bit(dc->bitstr);
            vui->colour_description_present_flag = read_bit(dc->bitstr);
            if (vui->colour_description_present_flag)
            {
                vui->colour_primaries = read_bits(dc->bitstr, 8);
                vui->transfer_characteristics = read_bits(dc->bitstr, 8);
                vui->matrix_coefficients = read_bits(dc->bitstr, 8);
            }
        }

        vui->chroma_loc_info_present_flag = read_bit(dc->bitstr);
        if (vui->chroma_loc_info_present_flag)
        {
            vui->chroma_sample_loc_type_top_field = read_ue(dc->bitstr);
            vui->chroma_sample_loc_type_bottom_field = read_ue(dc->bitstr);
        }

        vui->timing_info_present_flag = read_bit(dc->bitstr);
        if (vui->timing_info_present_flag)
        {
            vui->num_units_in_tick = read_bits(dc->bitstr, 32);
            vui->time_scale = read_bits(dc->bitstr, 32);
            vui->fixed_frame_rate_flag = read_bit(dc->bitstr);
        }

        vui->nal_hrd_parameters_present_flag = read_bit(dc->bitstr);
        if (vui->nal_hrd_parameters_present_flag)
        {
            // Hypothetical Reference Decoder
            vui->hrd = decodeHRD(dc);
        }

        vui->vcl_hrd_parameters_present_flag = read_bit(dc->bitstr);
        if (vui->vcl_hrd_parameters_present_flag)
        {
            // Decode HRD
            vui->hrd = decodeHRD(dc);
        }

        if (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true)
        {
            vui->low_delay_hrd_flag = read_bit(dc->bitstr);
        }

        vui->pic_struct_present_flag = read_bit(dc->bitstr);
        vui->bitstream_restriction_flag = read_bit(dc->bitstr);

        if (vui->bitstream_restriction_flag)
        {
            vui->motion_vectors_over_pic_boundaries_flag = read_bit(dc->bitstr);
            vui->max_bytes_per_pic_denom = read_ue(dc->bitstr);
            vui->max_bits_per_mb_denom = read_ue(dc->bitstr);
            vui->log2_max_mv_length_horizontal = read_ue(dc->bitstr);
            vui->log2_max_mv_length_vertical = read_ue(dc->bitstr);
            vui->num_reorder_frames = read_ue(dc->bitstr);
            vui->max_dec_frame_buffering = read_ue(dc->bitstr);
        }
    }

    // Return VUI structure
    return vui;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *vui (vui_t) data structure.
 * \return 1 if VUI seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.1 VUI parameters syntax.
 * E.2.1 VUI parameters semantics.
 *
 * VUI can only be found inside SPS structure.
 * This function must only be called by checkSPS().
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkVUI(DecodingContext_t *dc, vui_t *vui)
{
    TRACE_INFO(PARAM, "  > " BLD_GREEN "checkVUI()" CLR_RESET);
    int retcode = SUCCESS;

    // Check VUI structure
    if (vui == NULL)
    {
        TRACE_ERROR(PARAM, "    Invalid VUI structure!");
        retcode = FAILURE;
    }
    else // Check VUI values
    {
        if (vui->aspect_ratio_info_present_flag)
        {
            if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
            {
                if (is_prime(vui->sar_width) == 0)
                {
                    if (vui->sar_width != 0)
                    {
                        TRACE_WARNING(PARAM, "    - sar_width is %i, but should be 0 or a prime number", vui->sar_width);
                        retcode = FAILURE;
                    }
                }

                if (is_prime(vui->sar_height) == 0)
                {
                    if (vui->sar_height != 0)
                    {
                        TRACE_WARNING(PARAM, "    - sar_height is %i, but should be 0 or a prime number", vui->sar_height);
                        retcode = FAILURE;
                    }
                }
            }
            else if (vui->aspect_ratio_idc > 255)
            {
                TRACE_WARNING(PARAM, "    - aspect_ratio_idc is %i but should be in range [0,255]", vui->aspect_ratio_idc);
                retcode = FAILURE;
            }
        }

        if (vui->video_signal_type_present_flag)
        {
            if (vui->video_format > 7)
            {
                TRACE_WARNING(PARAM, "    - video_format is %i but should be in range [0,7]", vui->video_format);
                retcode = FAILURE;
            }

            if (vui->colour_description_present_flag)
            {
                if (vui->colour_primaries > 255)
                {
                    TRACE_WARNING(PARAM, "    - colour_primaries is %i but should be in range [0,255]", vui->colour_primaries);
                    retcode = FAILURE;
                }

                if (vui->transfer_characteristics > 255)
                {
                    TRACE_WARNING(PARAM, "    - transfer_characteristics is %i but should be in range [0,255]", vui->colour_primaries);
                    retcode = FAILURE;
                }

                if (vui->matrix_coefficients > 255)
                {
                    //FIXME better check
                    TRACE_WARNING(PARAM, "    - matrix_coefficients is %i but should be in range [0,255]", vui->colour_primaries);
                    retcode = FAILURE;
                }
            }
        }

        // Check HRD content
        if (retcode == SUCCESS && (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true))
        {
            retcode = checkHRD(dc, vui->hrd);
        }

        if (vui->chroma_loc_info_present_flag)
        {
            if (dc->ChromaArrayType != 1)
            {
                TRACE_WARNING(PARAM, "    - chroma_loc_info_present_flag is 1 but should be 0, because chroma_format_idc value is different than 1");
                retcode = FAILURE;
            }

            if (vui->chroma_sample_loc_type_top_field > 5)
            {
                TRACE_WARNING(PARAM, "    - chroma_sample_loc_type_top_field is %i but should be in range [0,5]", vui->chroma_sample_loc_type_top_field);
                retcode = FAILURE;
            }

            if (vui->chroma_sample_loc_type_bottom_field > 5)
            {
                TRACE_WARNING(PARAM, "    - chroma_sample_loc_type_bottom_field is %i but should be in range [0,5]", vui->chroma_sample_loc_type_bottom_field);
                retcode = FAILURE;
            }
        }

        if (vui->bitstream_restriction_flag)
        {
            if (vui->max_bytes_per_pic_denom > 16)
            {
                TRACE_WARNING(PARAM, "    - max_bytes_per_pic_denom is %i but should be in range [0,16]", vui->max_bytes_per_pic_denom);
                retcode = FAILURE;
            }

            if (vui->max_bits_per_mb_denom > 16)
            {
                TRACE_WARNING(PARAM, "    - max_bits_per_mb_denom is %i but should be in range [0,16]", vui->max_bits_per_mb_denom);
                retcode = FAILURE;
            }


            if (vui->log2_max_mv_length_horizontal > 16)
            {
                TRACE_WARNING(PARAM, "    - log2_max_mv_length_horizontal is %i but should be in range [0,16]", vui->log2_max_mv_length_horizontal);
                retcode = FAILURE;
            }

            if (vui->log2_max_mv_length_vertical > 16)
            {
                TRACE_WARNING(PARAM, "    - log2_max_mv_length_vertical is %i but should be in range [0,16]",
                              vui->log2_max_mv_length_vertical);
                retcode = FAILURE;
            }

            if (vui->num_reorder_frames > vui->max_dec_frame_buffering)
            {
                TRACE_WARNING(PARAM, "    - num_reorder_frames is %i but should be in range [0,max_dec_frame_buffering=%i]",
                              vui->num_reorder_frames, vui->max_dec_frame_buffering);
                retcode = FAILURE;
            }

            if (vui->max_dec_frame_buffering < vui->num_reorder_frames)
            {
                //FIXME max level: specified in subclauses A.3.1, A.3.2, G.10.2.1, and H.10.2.
                TRACE_WARNING(PARAM, "    - max_dec_frame_buffering is %i but should be greater than num_reorder_frames=%i",
                              vui->max_dec_frame_buffering, vui->num_reorder_frames);
                retcode = FAILURE;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about video_usability_information decoding.
 * \param *vui (vui_t) data structure.
 *
 * VUI can only be found inside SPS structure.
 * This function must only be called by printSPS().
 */
void printVUI(vui_t *vui)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "  > " BLD_GREEN "printVUI()" CLR_RESET);

    // Check VUI structure
    if (vui == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid VUI structure!");
        return;
    }

    // Print VUI values
    TRACE_1(PARAM, "    - aspect_ratio_info_present_flag= %i", vui->aspect_ratio_info_present_flag);
    if (vui->aspect_ratio_info_present_flag)
    {
        TRACE_1(PARAM, "    - aspect_ratio_idc  = %i", vui->aspect_ratio_idc);
        if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
        {
            TRACE_1(PARAM, "    - sar_width         = %i", vui->sar_width);
            TRACE_1(PARAM, "    - sar_height        = %i", vui->sar_height);
        }
    }

    TRACE_1(PARAM, "    - overscan_info_present_flag= %i", vui->overscan_info_present_flag);
    if (vui->overscan_info_present_flag)
    {
        TRACE_1(PARAM, "    - overscan_appropriate_flag= %i", vui->overscan_appropriate_flag);
    }

    TRACE_1(PARAM, "    - video_signal_type_present_flag= %i", vui->video_signal_type_present_flag);
    if (vui->video_signal_type_present_flag)
    {
        TRACE_1(PARAM, "    - video_format          = %i", vui->video_format);
        TRACE_1(PARAM, "    - video_full_range_flag = %i", vui->video_full_range_flag);
        TRACE_1(PARAM, "    - colour_description_present_flag= %i", vui->colour_description_present_flag);
        if (vui->colour_description_present_flag)
        {
            TRACE_1(PARAM, "    - colour_primaries          = %i", vui->colour_primaries);
            TRACE_1(PARAM, "    - transfer_characteristics  = %i", vui->transfer_characteristics);
            TRACE_1(PARAM, "    - matrix_coefficients       = %i", vui->matrix_coefficients);
        }
    }

    TRACE_1(PARAM, "    - chroma_loc_info_present_flag          = %i", vui->chroma_loc_info_present_flag);
    if (vui->chroma_loc_info_present_flag)
    {
        TRACE_1(PARAM, "    - chroma_sample_loc_type_top_field      = %i", vui->chroma_sample_loc_type_top_field);
        TRACE_1(PARAM, "    - chroma_sample_loc_type_bottom_field   = %i", vui->chroma_sample_loc_type_bottom_field);
    }

    TRACE_1(PARAM, "    - timing_info_present_flag      = %i", vui->timing_info_present_flag);
    if (vui->timing_info_present_flag)
    {
        TRACE_1(PARAM, "    - num_units_in_tick     = %i", vui->num_units_in_tick);
        TRACE_1(PARAM, "    - time_scale            = %i", vui->time_scale);
        TRACE_1(PARAM, "    - fixed_frame_rate_flag = %i", vui->fixed_frame_rate_flag);
    }

    TRACE_1(PARAM, "    - nal_hrd_parameters_present_flag= %i", vui->nal_hrd_parameters_present_flag);
    TRACE_1(PARAM, "    - vcl_hrd_parameters_present_flag= %i", vui->vcl_hrd_parameters_present_flag);

    // HRD
    if (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true)
    {
        TRACE_1(PARAM, "    - low_delay_hrd_flag        = %i", vui->low_delay_hrd_flag);
        printHRD(vui->hrd);
    }

    TRACE_1(PARAM, "    - pic_struct_present_flag       = %i", vui->pic_struct_present_flag);
    TRACE_1(PARAM, "    - bitstream_restriction_flag    = %i", vui->bitstream_restriction_flag);

    if (vui->bitstream_restriction_flag)
    {
        TRACE_1(PARAM, "    - motion_vectors_over_pic_boundaries_flag = %i", vui->motion_vectors_over_pic_boundaries_flag);
        TRACE_1(PARAM, "    - max_bytes_per_pic_denom       = %i", vui->max_bytes_per_pic_denom);
        TRACE_1(PARAM, "    - max_bits_per_mb_denom         = %i", vui->max_bits_per_mb_denom);
        TRACE_1(PARAM, "    - log2_max_mv_length_horizontal = %i", vui->log2_max_mv_length_horizontal);
        TRACE_1(PARAM, "    - log2_max_mv_length_vertical   = %i", vui->log2_max_mv_length_vertical);
        TRACE_1(PARAM, "    - num_reorder_frames            = %i", vui->num_reorder_frames);
        TRACE_1(PARAM, "    - max_dec_frame_buffering       = %i", vui->max_dec_frame_buffering);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return hrd_t initialized data structure.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.2 HRD parameters syntax.
 * E.2.2 HRD parameters semantics.
 *
 * HRD can only be found inside VUI structure.
 * This function must only be called by checkVUI().
 */
hrd_t *decodeHRD(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "  > " BLD_GREEN "decodeHRD()" CLR_RESET);

    // HRD allocation
    ////////////////////////////////////////////////////////////////////////////
    hrd_t *hrd = (hrd_t*)calloc(1, sizeof(hrd_t));

    if (hrd == NULL)
    {
        TRACE_ERROR(PARAM, "Unable to alloc new HRD!");
    }
    else
    {
        // HRD decoding
        ////////////////////////////////////////////////////////////////////////////

        hrd->cpb_cnt_minus1 = read_ue(dc->bitstr);
        hrd->bit_rate_scale = read_bits(dc->bitstr, 4);
        hrd->cpb_size_scale = read_bits(dc->bitstr, 4);

        unsigned int SchedSelIdx = 0;
        for (SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++)
        {
            hrd->bit_rate_value_minus1[SchedSelIdx] = read_ue(dc->bitstr);
            hrd->cpb_size_value_minus1[SchedSelIdx] = read_ue(dc->bitstr);
            hrd->CpbSize[SchedSelIdx] = (hrd->cpb_size_value_minus1[SchedSelIdx] + 1) * pow(2, 4 + hrd->cpb_size_scale);
            hrd->cbr_flag[SchedSelIdx] = read_bit(dc->bitstr);
        }

        hrd->initial_cpb_removal_delay_length_minus1 = read_bits(dc->bitstr, 5);
        hrd->cpb_removal_delay_length_minus1 = read_bits(dc->bitstr, 5);
        hrd->dpb_output_delay_length_minus1 = read_bits(dc->bitstr, 5);
        hrd->time_offset_length = read_bits(dc->bitstr, 5);
    }

    // Return HRD structure
    return hrd;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *hrd (hypothetical_reference_decoder) data structure.
 * \return 1 if HRD seems consistant, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.2 HRD parameters syntax.
 * E.2.2 HRD parameters semantics.
 *
 * HRD can only be found inside VUI structure.
 * This function must only be called by checkVUI().
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkHRD(DecodingContext_t *dc, hrd_t *hrd)
{
    TRACE_INFO(PARAM, "  > " BLD_GREEN "checkHRD()" CLR_RESET);
    int retcode = SUCCESS;

    // Check HRD structure
    if (hrd == NULL)
    {
        TRACE_ERROR(PARAM, "  Invalid HRD structure!");
        retcode = FAILURE;
    }
    else // Check HRD values
    {
        if (hrd->cpb_cnt_minus1 > 31)
        {
            TRACE_WARNING(PARAM, "      - cpb_cnt_minus1 is %i, but should be between 0 and 31", hrd->cpb_cnt_minus1);
            retcode = FAILURE;
        }

        //FIXME No boundaries specified?
        //TRACE_1(PARAM, "      - bit_rate_scale    : %i", hrd->bit_rate_scale);
        //TRACE_1(PARAM, "      - cpb_size_scale    : %i", hrd->cpb_size_scale);

        unsigned int SchedSelIdx = 0;
        for (SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++)
        {
            if (hrd->bit_rate_value_minus1[SchedSelIdx] > (pow(2, 32)-2))
            {
                TRACE_WARNING(PARAM, "      - bit_rate_value_minus1[%i] is %i, but should in range [0,2^32-2]",
                              SchedSelIdx, hrd->bit_rate_value_minus1[SchedSelIdx]);
                retcode = FAILURE;
            }
            else if (SchedSelIdx > 0 && hrd->bit_rate_value_minus1[SchedSelIdx] <= hrd->bit_rate_value_minus1[SchedSelIdx-1])
            {
                TRACE_WARNING(PARAM, "      - bit_rate_value_minus1[%i] is %i, but should be greater than bit_rate_value_minus1[%i] = %i",
                              SchedSelIdx, hrd->bit_rate_value_minus1[SchedSelIdx], SchedSelIdx-1, hrd->bit_rate_value_minus1[SchedSelIdx-1]);
                retcode = FAILURE;
            }

            if (hrd->cpb_size_value_minus1[SchedSelIdx] > (pow(2, 32)-2))
            {
                TRACE_WARNING(PARAM, "      - cpb_size_value_minus1[%i] is %i, but should in range [0,2^32-2]",
                              SchedSelIdx, hrd->cpb_size_value_minus1[SchedSelIdx]);
                retcode = FAILURE;
            }
            else if (SchedSelIdx > 0 && hrd->cpb_size_value_minus1[SchedSelIdx] > hrd->cpb_size_value_minus1[SchedSelIdx-1])
            {
                TRACE_WARNING(PARAM, "      - cpb_size_value_minus1[%i] is %i, but should be less than or equal to cpb_size_value_minus1[%i] = %i",
                              SchedSelIdx, hrd->cpb_size_value_minus1[SchedSelIdx], SchedSelIdx-1, hrd->cpb_size_value_minus1[SchedSelIdx-1]);
                retcode = FAILURE;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about hypothetical_reference_decoder decoding.
 * \param *hrd (hrd_t) data structure.
 *
 * HRD can only be found inside VUI structure.
 * This function must only be called by printVUI().
 */
void printHRD(hrd_t *hrd)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "  > " BLD_GREEN "printHRD()" CLR_RESET);

    // Check HRD structure
    if (hrd == NULL)
    {
        TRACE_ERROR(PARAM, "      Invalid HRD structure!");
        return;
    }

    // Print HRD values
    TRACE_1(PARAM, "      - cpb_cnt_minus1  = %i", hrd->cpb_cnt_minus1);
    TRACE_1(PARAM, "      - bit_rate_scale  = %i", hrd->bit_rate_scale);
    TRACE_1(PARAM, "      - cpb_size_scale  = %i", hrd->cpb_size_scale);

    unsigned int SchedSelIdx = 0;
    for (SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++)
    {
        TRACE_1(PARAM, "      - bit_rate_value_minus1[%i]       = %i", SchedSelIdx, hrd->bit_rate_value_minus1[SchedSelIdx]);
        TRACE_1(PARAM, "      - cpb_size_value_minus1[%i]       = %i", SchedSelIdx, hrd->cpb_size_value_minus1[SchedSelIdx]);
        TRACE_1(PARAM, "      - cbr_flag[%i]                    = %i", SchedSelIdx, hrd->cbr_flag[SchedSelIdx]);
    }

    TRACE_1(PARAM, "      - initial_cpb_removal_delay_length_minus1 = %i", hrd->initial_cpb_removal_delay_length_minus1);
    TRACE_1(PARAM, "      - cpb_removal_delay_length_minus1 = %i", hrd->cpb_removal_delay_length_minus1);
    TRACE_1(PARAM, "      - dpb_output_delay_length_minus1  = %i", hrd->dpb_output_delay_length_minus1);
    TRACE_1(PARAM, "      - time_offset_length          = %i", hrd->time_offset_length);
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
