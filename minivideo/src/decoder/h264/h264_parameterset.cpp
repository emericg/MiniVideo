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
 * \file      h264_parameterset.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_parameterset.h"
#include "h264_expgolomb.h"
#include "h264_transform.h"
#include "../../demuxer/xml_mapper.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "../../bitstream_utils.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <cinttypes>

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

static vui_t *decodeVUI(Bitstream_t *bitstr);
static hrd_t *decodeHRD(Bitstream_t *bitstr);

static void mapVUI(vui_t *vui, FILE *xml);
static void mapHRD(hrd_t *hrd, FILE *xml);

static void printVUI(vui_t *vui);
static void printHRD(hrd_t *hrd);

static int checkSPS(sps_t *sps);
static int checkPPS(pps_t *pps, sps_t *sps);
static int checkSEI(sei_t *sei);
static int checkVUI(vui_t *vui, sps_t *sps);
static int checkHRD(hrd_t *hrd);

/* ************************************************************************** */
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
static void scaling_list_4x4(Bitstream_t *bitstr, sps_t *sps, int i)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "scaling_list_4x4()" CLR_RESET);

    int lastScale = 8;
    int nextScale = 8;

    if (sps)
    {
        for (int j = 0; j < 16; j++)
        {
            if (nextScale != 0)
            {
                int delta_scale = read_se(bitstr);
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
static void scaling_list_8x8(Bitstream_t *bitstr, sps_t *sps, int i)
{
    TRACE_INFO(PARAM, "> " BLD_GREEN "scaling_list_8x8()" CLR_RESET);

    int lastScale = 8;
    int nextScale = 8;

    if (sps)
    {
        for (int j = 0; j < 64; j++)
        {
            if (nextScale != 0)
            {
                int delta_scale = read_se(bitstr);
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
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 1 if SPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.3.2.1 Sequence parameter set RBSP syntax.
 * 7.4.2.1 Sequence parameter set RBSP semantics.
 */
int decodeSPS_legacy(DecodingContext_t *dc)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeSPS_legacy()" CLR_RESET);
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
            sps->profile_idc == STEREO_HIGH || sps->profile_idc == 138 || sps->profile_idc == 139 ||
            sps->profile_idc == 134 || sps->profile_idc == 135)
        {
            sps->separate_colour_plane_flag = false;
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
                            scaling_list_4x4(dc->bitstr, sps, i);
                        else
                            scaling_list_8x8(dc->bitstr, sps, i - 6);
                    }
                }
            }
        }
        else // Set default parameters for profiles < HIGH
        {
            dc->ChromaArrayType = 1;
            sps->ChromaArrayType = 1;
            sps->chroma_format_idc = 1;
            sps->separate_colour_plane_flag = false;

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
        else if (sps->pic_order_cnt_type == 1)
        {
            sps->delta_pic_order_always_zero_flag = read_bit(dc->bitstr);
            sps->offset_for_non_ref_pic = read_se(dc->bitstr);
            sps->offset_for_top_to_bottom_field = read_se(dc->bitstr);
            sps->num_ref_frames_in_pic_order_cnt_cycle = read_ue(dc->bitstr);

            for (unsigned i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
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
            sps->mb_adaptive_frame_field_flag = read_bit(dc->bitstr);
        else
            sps->mb_adaptive_frame_field_flag = false;

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
            sps->vui = decodeVUI(dc->bitstr);
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int decodeSPS(Bitstream_t *bitstr, sps_t *sps)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeSPS()" CLR_RESET);
    int retcode = SUCCESS;

    if (sps == NULL)
    {
        TRACE_ERROR(PARAM, "NULL SPS!");
        retcode = FAILURE;
    }
    else
    {
        // SPS decoding
        ////////////////////////////////////////////////////////////////////////

        sps->profile_idc = read_bits(bitstr, 8);

        sps->constraint_setX_flag[0] = read_bit(bitstr);
        sps->constraint_setX_flag[1] = read_bit(bitstr);
        sps->constraint_setX_flag[2] = read_bit(bitstr);
        sps->constraint_setX_flag[3] = read_bit(bitstr);
        sps->constraint_setX_flag[4] = read_bit(bitstr);
        sps->constraint_setX_flag[5] = read_bit(bitstr);

        if (read_bits(bitstr, 2) != 0)
        {
            TRACE_ERROR(PARAM, "  Error while reading reserved_zero_2bits: must be 0!");
            freeSPS(&sps);
            return FAILURE;
        }

        sps->level_idc = read_bits(bitstr, 8);
        sps->seq_parameter_set_id = read_ue(bitstr);
        sps->separate_colour_plane_flag = false;

        // Handle parameters for profiles >= HIGH
        if (sps->profile_idc == FREXT_HiP || sps->profile_idc == FREXT_Hi10P ||
            sps->profile_idc == FREXT_Hi422 || sps->profile_idc == FREXT_Hi444 || sps->profile_idc == FREXT_CAVLC444 ||
            sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == MVC_HIGH ||
            sps->profile_idc == STEREO_HIGH || sps->profile_idc == 138 || sps->profile_idc == 139 ||
            sps->profile_idc == 134 || sps->profile_idc == 135)
        {
            sps->chroma_format_idc = read_ue(bitstr);
            sps->ChromaArrayType = sps->chroma_format_idc;

            if (sps->chroma_format_idc == 0) // 4:0:0 subsampling
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc == 0, yuv 4:0:0)");

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

                sps->SubHeightC = 1;
                sps->SubWidthC = 2;
            }
            else if (sps->chroma_format_idc == 3) // 4:4:4 subsampling
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (chroma_format_idc == 3, yuv 4:4:4)");

                sps->separate_colour_plane_flag = read_bit(bitstr);
                if (sps->separate_colour_plane_flag)
                {
                    TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == 1)");

                    sps->ChromaArrayType = 0;

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

            sps->bit_depth_luma_minus8 = read_ue(bitstr);
            sps->BitDepthY = 8 + sps->bit_depth_luma_minus8;
            sps->QpBdOffsetY = 6 * sps->bit_depth_luma_minus8;

            sps->bit_depth_chroma_minus8 = read_ue(bitstr);
            sps->BitDepthC = 8 + sps->bit_depth_chroma_minus8;
            sps->QpBdOffsetC = 6 * sps->bit_depth_chroma_minus8;

            sps->RawMbBits = 256 * sps->BitDepthY + 2 * sps->MbWidthC * sps->MbHeightC * sps->BitDepthC;

            sps->qpprime_y_zero_transform_bypass_flag = read_bit(bitstr);

            // Extract scaling list from bitstream
            sps->seq_scaling_matrix_present_flag = read_bit(bitstr);
            if (sps->seq_scaling_matrix_present_flag)
            {
                for (unsigned i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++)
                {
                    sps->seq_scaling_list_present_flag[i] = read_bit(bitstr);
                    if (sps->seq_scaling_list_present_flag[i])
                    {
                        if (i < 6)
                            scaling_list_4x4(bitstr, sps, i);
                        else
                            scaling_list_8x8(bitstr, sps, i - 6);
                    }
                }
            }
        }
        else // Set default parameters for profiles < HIGH
        {
            sps->ChromaArrayType = 1;
            sps->chroma_format_idc = 1;
            sps->separate_colour_plane_flag = false;

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
            for (int i = 0; i < 6; i++)
            {
                for (int k = 0; k < 16; k++)
                {
                    sps->ScalingList4x4[i][k] = 16;
                }

                for (int k = 0; k < 64; k++)
                {
                    sps->ScalingList8x8[i][k] = 16;
                }

                // Transform the list into a matrix
                inverse_scan_4x4(sps->ScalingList4x4[i], sps->ScalingMatrix4x4[i]);
                inverse_scan_8x8(sps->ScalingList8x8[i], sps->ScalingMatrix8x8[i]);
            }
        }

        sps->log2_max_frame_num_minus4 = read_ue(bitstr);
        sps->MaxFrameNum = pow(2, sps->log2_max_frame_num_minus4 + 4);

        sps->pic_order_cnt_type = read_ue(bitstr);
        if (sps->pic_order_cnt_type == 0)
        {
            sps->log2_max_pic_order_cnt_lsb_minus4 = read_ue(bitstr);
            sps->MaxPicOrderCntLsb = pow(2, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        }
        else if (sps->pic_order_cnt_type == 1)
        {
            sps->delta_pic_order_always_zero_flag = read_bit(bitstr);
            sps->offset_for_non_ref_pic = read_se(bitstr);
            sps->offset_for_top_to_bottom_field = read_se(bitstr);
            sps->num_ref_frames_in_pic_order_cnt_cycle = read_ue(bitstr);

            for (unsigned i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                sps->offset_for_ref_frame[i] = read_se(bitstr);
            }
        }

        sps->max_num_ref_frames = read_ue(bitstr);
        sps->gaps_in_frame_num_value_allowed_flag = read_bit(bitstr);

        sps->pic_width_in_mbs_minus1 = read_ue(bitstr);
        sps->PicWidthInMbs = sps->pic_width_in_mbs_minus1 + 1;
        sps->PicWidthInSamplesL = sps->PicWidthInMbs * 16;
        sps->PicWidthInSamplesC = sps->PicWidthInMbs * sps->MbWidthC;

        sps->pic_height_in_map_units_minus1 = read_ue(bitstr);
        sps->PicHeightInMapUnits = sps->pic_height_in_map_units_minus1 + 1;
        sps->PicSizeInMapUnits = sps->PicWidthInMbs * sps->PicHeightInMapUnits;

        sps->frame_mbs_only_flag = read_bit(bitstr);
        if (sps->frame_mbs_only_flag == false)
            sps->mb_adaptive_frame_field_flag = read_bit(bitstr);
        else
            sps->mb_adaptive_frame_field_flag = false;

        sps->FrameHeightInMbs = (2 - sps->frame_mbs_only_flag) * sps->PicHeightInMapUnits;

        sps->direct_8x8_inference_flag = read_bit(bitstr);

        sps->frame_cropping_flag = read_bit(bitstr);
        if (sps->frame_cropping_flag)
        {
            sps->frame_crop_left_offset = read_ue(bitstr);
            sps->frame_crop_right_offset = read_ue(bitstr);
            sps->frame_crop_top_offset = read_ue(bitstr);
            sps->frame_crop_bottom_offset = read_ue(bitstr);

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

        sps->vui_parameters_present_flag = read_bit(bitstr);
        if (sps->vui_parameters_present_flag)
        {
            // Decode VUI
            sps->vui = decodeVUI(bitstr);
        }

        // SPS check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkSPS(sps);
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
            free((*sps_ptr)->vui->nal_hrd);
            (*sps_ptr)->vui->nal_hrd = NULL;

            free((*sps_ptr)->vui->vcl_hrd);
            (*sps_ptr)->vui->vcl_hrd = NULL;

            free((*sps_ptr)->vui);
            (*sps_ptr)->vui = NULL;
        }

        free(*sps_ptr);
        *sps_ptr = NULL;
    }
}

/* ************************************************************************** */

/*!
 * \param *sps (sequence_parameter_set) data structure.
 * \return 1 if SPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.1 Sequence parameter set RBSP semantics.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkSPS(sps_t *sps)
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
            }

            if (sps->separate_colour_plane_flag)
            {
                TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == true)");
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
            // always true if sps->offset_for_non_ref_pic is an int32

            if (sps->num_ref_frames_in_pic_order_cnt_cycle > 255)
            {
                TRACE_WARNING(PARAM, "  - num_ref_frames_in_pic_order_cnt_cycle is %i but should be in range [0,255]", sps->num_ref_frames_in_pic_order_cnt_cycle);
                retcode = FAILURE;
            }
            for (unsigned i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                // always true if sps->offset_for_ref_frame[i] is an int32
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
            retcode = checkVUI(sps->vui, sps);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print informations about sequence_parameter_set decoding.
 * \param *sps: .
 */
void printSPS(sps_t *sps)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printSPS()" CLR_RESET);

    unsigned int i = 0;

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
         for (i = 0; i < ((sps->ChromaArrayType != 3) ? 8 : 12); i++)
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
        TRACE_1(PARAM, "  - num_ref_frames_in_pic_order_cnt_cycle= %u", sps->num_ref_frames_in_pic_order_cnt_cycle);

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

void mapSPS(sps_t *sps, int64_t offset, int64_t size, FILE *xml)
{
    if (sps && xml)
    {
        fprintf(xml, "  <a tt=\"SPS\" add=\"private\" tp=\"datas\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                offset, size);

        xmlSpacer(xml, "Sequence Parameter Set", -1);

        fprintf(xml, "  <profile_idc>%u</profile_idc>\n", sps->profile_idc);
        for (unsigned i = 0; i < 6; i++)
        {
            fprintf(xml, "  <constraint_setX_flag_%u>%u</constraint_setX_flag_%u>\n", i, sps->constraint_setX_flag[i], i);
        }
        fprintf(xml, "  <level_idc>%u</level_idc>\n", sps->level_idc);
        fprintf(xml, "  <seq_parameter_set_id>%u</seq_parameter_set_id>\n", sps->seq_parameter_set_id);

        fprintf(xml, "  <chroma_format_idc>%u</chroma_format_idc>\n", sps->chroma_format_idc);
        fprintf(xml, "  <ChromaArrayType>%u</ChromaArrayType>\n", sps->ChromaArrayType);
        fprintf(xml, "  <SubWidthC>%u</SubWidthC>\n", sps->SubWidthC);
        fprintf(xml, "  <SubHeightC>%u</SubHeightC>\n", sps->SubHeightC);
        fprintf(xml, "  <MbWidthC>%u</MbWidthC>\n", sps->MbWidthC);
        fprintf(xml, "  <MbHeightC>%u</MbHeightC>\n", sps->MbHeightC);

        if (sps->chroma_format_idc == 3)
        {
            fprintf(xml, "  <separate_colour_plane_flag>%u</separate_colour_plane_flag>\n", sps->separate_colour_plane_flag);
        }

        fprintf(xml, "  <bit_depth_luma_minus8>%u</bit_depth_luma_minus8>\n", sps->bit_depth_luma_minus8);
        fprintf(xml, "  <BitDepthY>%u</BitDepthY>\n", sps->BitDepthY);
        fprintf(xml, "  <QpBdOffsetY>%u</QpBdOffsetY>\n", sps->QpBdOffsetY);
        fprintf(xml, "  <bit_depth_chroma_minus8>%u</bit_depth_chroma_minus8>\n", sps->bit_depth_chroma_minus8);
        fprintf(xml, "  <BitDepthC>%u</BitDepthC>\n", sps->BitDepthC);
        fprintf(xml, "  <QpBdOffsetC>%u</QpBdOffsetC>\n", sps->QpBdOffsetC);
        fprintf(xml, "  <RawMbBits>%u</RawMbBits>\n", sps->RawMbBits);
        fprintf(xml, "  <qpprime_y_zero_transform_bypass_flag>%u</qpprime_y_zero_transform_bypass_flag>\n", sps->qpprime_y_zero_transform_bypass_flag);

        fprintf(xml, "  <seq_scaling_matrix_present_flag>%u</seq_scaling_matrix_present_flag>\n", sps->seq_scaling_matrix_present_flag);
        if (sps->seq_scaling_matrix_present_flag)
        {
             for (unsigned i = 0; i < ((sps->ChromaArrayType != 3) ? 8 : 12); i++)
             {
                 fprintf(xml, "  <seq_scaling_list_present_flag_%u>%u</seq_scaling_list_present_flag_%u>\n", i, sps->seq_scaling_list_present_flag[i], i);
             }
        }
        fprintf(xml, "  <log2_max_frame_num_minus4>%u</log2_max_frame_num_minus4>\n", sps->log2_max_frame_num_minus4);
        fprintf(xml, "  <MaxFrameNum>%u</MaxFrameNum>\n", sps->MaxFrameNum);
        fprintf(xml, "  <pic_order_cnt_type>%u</pic_order_cnt_type>\n", sps->pic_order_cnt_type);
        if (sps->pic_order_cnt_type == 0)
        {
            fprintf(xml, "  <log2_max_pic_order_cnt_lsb_minus4>%u</log2_max_pic_order_cnt_lsb_minus4>\n", sps->log2_max_pic_order_cnt_lsb_minus4);
            fprintf(xml, "  <MaxPicOrderCntLsb>%u</MaxPicOrderCntLsb>\n", sps->MaxPicOrderCntLsb);
        }
        else if (sps->pic_order_cnt_type == 1)
        {
            fprintf(xml, "  <delta_pic_order_always_zero_flag>%u</delta_pic_order_always_zero_flag>\n", sps->delta_pic_order_always_zero_flag);
            fprintf(xml, "  <offset_for_non_ref_pic>%i</offset_for_non_ref_pic>\n", sps->offset_for_non_ref_pic);
            fprintf(xml, "  <offset_for_top_to_bottom_field>%i</offset_for_top_to_bottom_field>\n", sps->offset_for_top_to_bottom_field);
            fprintf(xml, "  <num_ref_frames_in_pic_order_cnt_cycle>%u</num_ref_frames_in_pic_order_cnt_cycle>\n", sps->num_ref_frames_in_pic_order_cnt_cycle);

            for (unsigned i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                fprintf(xml, "  <offset_for_ref_frame_%u>%i</offset_for_ref_frame_%u>\n", i, sps->offset_for_ref_frame[i], i);
            }
        }

        fprintf(xml, "  <max_num_ref_frames>%u</max_num_ref_frames>\n", sps->max_num_ref_frames);
        fprintf(xml, "  <gaps_in_frame_num_value_allowed_flag>%u</gaps_in_frame_num_value_allowed_flag>\n", sps->gaps_in_frame_num_value_allowed_flag);
        fprintf(xml, "  <pic_width_in_mbs_minus1>%u</pic_width_in_mbs_minus1>\n", sps->pic_width_in_mbs_minus1);
        fprintf(xml, "  <PicWidthInMbs>%u</PicWidthInMbs>\n", sps->PicWidthInMbs);
        fprintf(xml, "  <PicWidthInSamplesL>%u</PicWidthInSamplesL>\n", sps->PicWidthInSamplesL);
        fprintf(xml, "  <PicWidthInSamplesC>%u</PicWidthInSamplesC>\n", sps->PicWidthInSamplesC);
        fprintf(xml, "  <pic_height_in_map_units_minus1>%u</pic_height_in_map_units_minus1>\n", sps->pic_height_in_map_units_minus1);
        fprintf(xml, "  <frame_mbs_only_flag>%u</frame_mbs_only_flag>\n", sps->frame_mbs_only_flag);
        fprintf(xml, "  <FrameHeightInMbs>%u</FrameHeightInMbs>\n", sps->FrameHeightInMbs);

        if (sps->frame_mbs_only_flag == false)
        {
            fprintf(xml, "  <mb_adaptive_frame_field_flag>%u</mb_adaptive_frame_field_flag>\n", sps->mb_adaptive_frame_field_flag);
        }

        fprintf(xml, "  <direct_8x8_inference_flag>%u</direct_8x8_inference_flag>\n", sps->direct_8x8_inference_flag);

        fprintf(xml, "  <frame_cropping_flag>%u</frame_cropping_flag>\n", sps->frame_cropping_flag);
        if (sps->frame_cropping_flag)
        {
            fprintf(xml, "  <frame_crop_left_offset>%u</frame_crop_left_offset>\n", sps->frame_crop_left_offset);
            fprintf(xml, "  <frame_crop_right_offset>%u</frame_crop_right_offset>\n", sps->frame_crop_right_offset);
            fprintf(xml, "  <frame_crop_top_offset>%u</frame_crop_top_offset>\n", sps->frame_crop_top_offset);
            fprintf(xml, "  <frame_crop_bottom_offset>%u</frame_crop_bottom_offset>\n", sps->frame_crop_bottom_offset);
            fprintf(xml, "  <CropUnitX>%u</CropUnitX>\n", sps->CropUnitX);
            fprintf(xml, "  <CropUnitY>%u</CropUnitY>\n", sps->CropUnitY);
        }

        TRACE_1(PARAM, "  - vui_parameters_present_flag= %u", sps->vui_parameters_present_flag);
        if (sps->vui_parameters_present_flag)
        {
            mapVUI(sps->vui, xml);
        }

        fprintf(xml, "  </a>\n");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

int decodePPS(Bitstream_t *bitstr, pps_t *pps, sps_t **sps_array)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodePPS()" CLR_RESET);
    int retcode = SUCCESS;

    if (pps == NULL)
    {
        TRACE_ERROR(PARAM, "NULL PPS!");
        retcode = FAILURE;
    }
    else
    {
        // PPS decoding
        ////////////////////////////////////////////////////////////////////////

        pps->pic_parameter_set_id = read_ue(bitstr);
        pps->seq_parameter_set_id = read_ue(bitstr);

        pps->entropy_coding_mode_flag = read_bit(bitstr);
        pps->bottom_field_pic_order_in_frame_present_flag = read_bit(bitstr);

        pps->num_slice_groups_minus1 = read_ue(bitstr);
        if (pps->num_slice_groups_minus1 > 0)
        {
            pps->slice_group_map_type = read_ue(bitstr);
            if (pps->slice_group_map_type == 0)
            {
                for (unsigned iGroup = 0; iGroup <= pps->num_slice_groups_minus1 && iGroup < 8; iGroup++)
                {
                    pps->run_length_minus1[iGroup] = read_ue(bitstr);
                }
            }
            else if (pps->slice_group_map_type == 2)
            {
                for (unsigned iGroup = 0; iGroup <= pps->num_slice_groups_minus1 && iGroup < 8; iGroup++)
                {
                    pps->top_left[iGroup] = read_ue(bitstr);
                    pps->bottom_right[iGroup] = read_ue(bitstr);
                }
            }
            else if (pps->slice_group_map_type == 3 ||
                     pps->slice_group_map_type == 4 ||
                     pps->slice_group_map_type == 5)
            {
                pps->slice_group_change_direction_flag = read_bit(bitstr);
                pps->slice_group_change_rate_minus1 = read_ue(bitstr);
            }
            else if (pps->slice_group_map_type == 6)
            {
                pps->pic_size_in_map_units_minus1 = read_ue(bitstr);
                for (unsigned i = 0; i <= pps->pic_size_in_map_units_minus1; i++)
                {
                    //pps->slice_group_id[i] = read_bits(bitstr, ceil(log2(pps->num_slice_groups_minus1 + 1)));
                    //pps->slice_group_id[i] = read_bit(bitstr);
                    TRACE_ERROR(PARAM, "UNIMPLEMENTED read_u(v) !!!"); //FIXME
                }
            }
        }

        pps->num_ref_idx_l0_default_active_minus1 = read_ue(bitstr);
        pps->num_ref_idx_l1_default_active_minus1 = read_ue(bitstr);
        pps->weighted_pred_flag = read_bit(bitstr);
        pps->weighted_bipred_idc = read_bits(bitstr, 2);
        pps->pic_init_qp_minus26 = read_se(bitstr);
        pps->pic_init_qs_minus26 = read_se(bitstr);
        pps->chroma_qp_index_offset = read_se(bitstr);
        pps->deblocking_filter_control_present_flag = read_bit(bitstr);
        pps->constrained_intra_pred_flag = read_bit(bitstr);
        pps->redundant_pic_cnt_present_flag = read_bit(bitstr);

        if (h264_more_rbsp_data(bitstr) == true &&
            sps_array[pps->seq_parameter_set_id]->profile_idc >= FREXT_HiP)
        {
            pps->transform_8x8_mode_flag = read_bit(bitstr);

            pps->pic_scaling_matrix_present_flag = read_bit(bitstr);
            if (pps->pic_scaling_matrix_present_flag)
            {
                for (int i = 0; i < (6 + ((sps_array[pps->seq_parameter_set_id]->chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag); i++)
                {
                    pps->pic_scaling_list_present_flag[i] = read_bit(bitstr);
                    if (pps->pic_scaling_list_present_flag[i])
                    {
                        TRACE_ERROR(PARAM, "UNIMPLEMENTED PPS scaling list !!!"); //FIXME
/*
                        // TODO
                        if (i < 6)
                            scaling_list_4x4(sps_array[pps->seq_parameter_set_id]->ScalingList4x4[i], 16,
                                             sps_array[pps->seq_parameter_set_id]->UseDefaultScalingMatrix4x4Flag[i]);
                        else
                            scaling_list_8x8(sps_array[pps->seq_parameter_set_id]->ScalingList8x8[i-6], 64,
                                             sps_array[pps->seq_parameter_set_id]->UseDefaultScalingMatrix8x8Flag[i-6]);
*/
                    }
                }
            }

            pps->second_chroma_qp_index_offset = read_se(bitstr);
        }
        else
        {
            pps->transform_8x8_mode_flag = false;
            pps->pic_scaling_matrix_present_flag = false;
            pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
        }

        retcode = h264_rbsp_trailing_bits(bitstr);

        // PPS check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkPPS(pps, sps_array[pps->seq_parameter_set_id]);
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
    }
}

/* ************************************************************************** */

/*!
 * \param *pps picture_parameter_set.
 * \param *sps the associated sequence_parameter_set.
 * \return 1 if PPS seems consistent, 0 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.2.2 Picture parameter set RBSP semantics.
 *
 * Check parsed values (and not derived ones) for inconsistencies.
 */
static int checkPPS(pps_t *pps, sps_t *sps)
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
            retcode = FAILURE;
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
void printPPS(pps_t *pps, sps_t **sps_array)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printPPS()" CLR_RESET);

    unsigned int i = 0;

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
         for (i = 0; i < ((sps_array[pps->seq_parameter_set_id]->ChromaArrayType != 3) ? 8 : 12); i++)
         {
             TRACE_1(PARAM, "  - pic_scaling_list_present_flag[%i]= %i", i, pps->pic_scaling_list_present_flag[i]);
         }
    }
    TRACE_1(PARAM, "  - second_chroma_qp_index_offset   = %i", pps->second_chroma_qp_index_offset);
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

void mapPPS(pps_t *pps, sps_t **sps, int64_t offset, int64_t size, FILE *xml)
{
    if (pps && sps && xml)
    {
        fprintf(xml, "  <a tt=\"PPS\" add=\"private\" tp=\"datas\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                offset, size);

        xmlSpacer(xml, "Picture Parameter Set", -1);

        // Print PPS values
        fprintf(xml, "  <pic_parameter_set_id>%u</pic_parameter_set_id>\n", pps->pic_parameter_set_id);
        fprintf(xml, "  <seq_parameter_set_id>%u</seq_parameter_set_id>\n", pps->seq_parameter_set_id);
        fprintf(xml, "  <entropy_coding_mode_flag>%u</entropy_coding_mode_flag>\n", pps->entropy_coding_mode_flag);
        fprintf(xml, "  <bottom_field_pic_order_in_frame_present_flag>%u</bottom_field_pic_order_in_frame_present_flag>\n",
                pps->bottom_field_pic_order_in_frame_present_flag);
        fprintf(xml, "  <num_slice_groups_minus1>%u</num_slice_groups_minus1>\n", pps->num_slice_groups_minus1);
        if (pps->num_slice_groups_minus1 > 0)
        {
            fprintf(xml, "  <slice_group_map_type>%u</slice_group_map_type>\n", pps->slice_group_map_type);
            if (pps->slice_group_map_type == 0)
            {
                for (unsigned iGroup = 0; iGroup <= pps->num_slice_groups_minus1; iGroup++)
                {
//                    fprintf(xml, "  <run_length_minus1_%u>%u</run_length_minus1_%u>\n",
//                            iGroup, pps->run_length_minus1[iGroup], iGroup);
                }
            }
            else if (pps->slice_group_map_type == 2)
            {
                for (unsigned iGroup = 0; iGroup < pps->num_slice_groups_minus1; iGroup++)
                {
//                    fprintf(xml, "  <top_left_%u>%u</top_left_%u>\n",
//                            iGroup, pps->top_left[iGroup], iGroup);
//                    fprintf(xml, "  <bottom_right_%u>%u</bottom_right_%u>\n",
//                            iGroup, pps->bottom_right[iGroup], iGroup);
                }
            }
            else if (pps->slice_group_map_type == 3 ||
                     pps->slice_group_map_type == 4 ||
                     pps->slice_group_map_type == 5)
            {
                fprintf(xml, "  <slice_group_change_direction_flag>%u</slice_group_change_direction_flag>\n", pps->slice_group_change_direction_flag);
                fprintf(xml, "  <slice_group_change_rate_minus1>%u</slice_group_change_rate_minus1>\n", pps->slice_group_change_rate_minus1);
            }
            else if (pps->slice_group_map_type == 6)
            {
                fprintf(xml, "  <pic_size_in_map_units_minus1>%u</pic_size_in_map_units_minus1>\n", pps->pic_size_in_map_units_minus1);
                for (unsigned i = 0; i <= pps->pic_size_in_map_units_minus1; i++)
                {
//                  fprintf(xml, "  <slice_group_id_%u>%u</slice_group_id_%u>\n", i, pps->slice_group_id[i], i);
                }
            }

            fprintf(xml, "  <num_ref_idx_l0_default_active_minus1>%u</num_ref_idx_l0_default_active_minus1>\n", pps->num_ref_idx_l0_default_active_minus1);
            fprintf(xml, "  <num_ref_idx_l1_default_active_minus1>%u</num_ref_idx_l1_default_active_minus1>\n", pps->num_ref_idx_l1_default_active_minus1);
            fprintf(xml, "  <weighted_pred_flag>%u</weighted_pred_flag>\n", pps->weighted_pred_flag);
            fprintf(xml, "  <weighted_bipred_idc>%u</weighted_bipred_idc>\n", pps->weighted_bipred_idc);
            fprintf(xml, "  <pic_init_qp_minus26>%i</pic_init_qp_minus26>\n", pps->pic_init_qp_minus26);
            fprintf(xml, "  <pic_init_qs_minus26>%i</pic_init_qs_minus26>\n", pps->pic_init_qs_minus26);
            fprintf(xml, "  <chroma_qp_index_offset>%i</chroma_qp_index_offset>\n", pps->chroma_qp_index_offset);
            fprintf(xml, "  <deblocking_filter_control_present_flag>%u</deblocking_filter_control_present_flag>\n", pps->deblocking_filter_control_present_flag);
            fprintf(xml, "  <constrained_intra_pred_flag>%u</constrained_intra_pred_flag>\n", pps->constrained_intra_pred_flag);
            fprintf(xml, "  <redundant_pic_cnt_present_flag>%u</redundant_pic_cnt_present_flag>\n", pps->redundant_pic_cnt_present_flag);

            fprintf(xml, "  <transform_8x8_mode_flag>%u</transform_8x8_mode_flag>\n", pps->transform_8x8_mode_flag);
            fprintf(xml, "  <pic_scaling_matrix_present_flag>%u</pic_scaling_matrix_present_flag>\n", pps->pic_scaling_matrix_present_flag);
            if (pps->pic_scaling_matrix_present_flag)
            {
                 for (unsigned i = 0; i < ((sps[pps->seq_parameter_set_id]->ChromaArrayType != 3) ? 8 : 12); i++)
                 {
                     fprintf(xml, "  <pic_scaling_list_present_flag_%u>%u</pic_scaling_list_present_flag_%u>\n", i, pps->pic_scaling_list_present_flag[i], i);
                 }
            }
            fprintf(xml, "  <second_chroma_qp_index_offset>%i</second_chroma_qp_index_offset>\n", pps->second_chroma_qp_index_offset);
        }

        fprintf(xml, "  </a>\n");
    }
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
int decodeAUD(Bitstream_t *bitstr, aud_t *aud)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeAUD()" CLR_RESET);
    int retcode = SUCCESS;

    if (aud == NULL)
    {
        TRACE_ERROR(PARAM, "NULL AUD!");
        retcode = FAILURE;
    }
    else
    {
        aud->primary_pic_type = read_bits(bitstr, 3);
        TRACE_2(PARAM, "<> " BLD_GREEN "primary_pic_type: %i" CLR_RESET, aud->primary_pic_type);

        retcode = h264_rbsp_trailing_bits(bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int decodeSEI(Bitstream_t *bitstr, sei_t *sei)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "decodeSEI()" CLR_RESET);
    int retcode = SUCCESS;

    if (sei == NULL)
    {
        TRACE_ERROR(PARAM, "NULL SEI!");
        retcode = FAILURE;
    }
    else
    {
        // SEI decoding
        ////////////////////////////////////////////////////////////////////////

        TRACE_WARNING(PARAM, ">>> UNIMPLEMENTED (SEI decoding)");
        retcode = FAILURE;

        // SEI check
        ////////////////////////////////////////////////////////////////////////

        if (retcode == SUCCESS)
        {
            retcode = checkSEI(sei);
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
static int checkSEI(sei_t *sei)
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
void printSEI(sei_t *sei)
{
#if ENABLE_DEBUG
    TRACE_INFO(PARAM, "> " BLD_GREEN "printSEI()" CLR_RESET);

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
 * \param bitstr: Our bitstream reader.
 * \return vui_t initialized data structure.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.1 VUI parameters syntax.
 * E.2.1 VUI parameters semantics.
 *
 * VUI can only be found inside SPS structure.
 * This function must only be called by decodeSPS().
 */
static vui_t *decodeVUI(Bitstream_t *bitstr)
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
        ////////////////////////////////////////////////////////////////////////

        vui->aspect_ratio_info_present_flag = read_bit(bitstr);
        if (vui->aspect_ratio_info_present_flag)
        {
            vui->aspect_ratio_idc = read_bits(bitstr, 8);
            if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
            {
                vui->sar_width = read_bits(bitstr, 16);
                vui->sar_height = read_bits(bitstr, 16);
            }
        }

        vui->overscan_info_present_flag = read_bit(bitstr);
        if (vui->overscan_info_present_flag)
        {
            vui->overscan_appropriate_flag = read_bit(bitstr);
        }

        vui->video_signal_type_present_flag = read_bit(bitstr);
        if (vui->video_signal_type_present_flag)
        {
            vui->video_format = read_bits(bitstr, 3);
            vui->video_full_range_flag = read_bit(bitstr);
            vui->colour_description_present_flag = read_bit(bitstr);
            if (vui->colour_description_present_flag)
            {
                vui->colour_primaries = read_bits(bitstr, 8);
                vui->transfer_characteristics = read_bits(bitstr, 8);
                vui->matrix_coefficients = read_bits(bitstr, 8);
            }
        }

        vui->chroma_loc_info_present_flag = read_bit(bitstr);
        if (vui->chroma_loc_info_present_flag)
        {
            vui->chroma_sample_loc_type_top_field = read_ue(bitstr);
            vui->chroma_sample_loc_type_bottom_field = read_ue(bitstr);
        }

        vui->timing_info_present_flag = read_bit(bitstr);
        if (vui->timing_info_present_flag)
        {
            vui->num_units_in_tick = read_bits(bitstr, 32);
            vui->time_scale = read_bits(bitstr, 32);
            vui->fixed_frame_rate_flag = read_bit(bitstr);
        }

        vui->nal_hrd_parameters_present_flag = read_bit(bitstr);
        if (vui->nal_hrd_parameters_present_flag)
        {
            // Hypothetical Reference Decoder
            vui->nal_hrd = decodeHRD(bitstr);
        }

        vui->vcl_hrd_parameters_present_flag = read_bit(bitstr);
        if (vui->vcl_hrd_parameters_present_flag)
        {
            // Hypothetical Reference Decoder
            vui->vcl_hrd = decodeHRD(bitstr);
        }

        if (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true)
        {
            vui->low_delay_hrd_flag = read_bit(bitstr);
        }

        vui->pic_struct_present_flag = read_bit(bitstr);
        vui->bitstream_restriction_flag = read_bit(bitstr);

        if (vui->bitstream_restriction_flag)
        {
            vui->motion_vectors_over_pic_boundaries_flag = read_bit(bitstr);
            vui->max_bytes_per_pic_denom = read_ue(bitstr);
            vui->max_bits_per_mb_denom = read_ue(bitstr);
            vui->log2_max_mv_length_horizontal = read_ue(bitstr);
            vui->log2_max_mv_length_vertical = read_ue(bitstr);
            vui->num_reorder_frames = read_ue(bitstr);
            vui->max_dec_frame_buffering = read_ue(bitstr);
        }
    }

    return vui;
}

/* ************************************************************************** */

/*!
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
static int checkVUI(vui_t *vui, sps_t *sps)
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
        if (retcode == SUCCESS)
        {
            if (vui->nal_hrd_parameters_present_flag == true)
                retcode = checkHRD(vui->nal_hrd);
            if (vui->vcl_hrd_parameters_present_flag == true)
                retcode = checkHRD(vui->vcl_hrd);
        }

        if (vui->chroma_loc_info_present_flag)
        {
            if (sps->ChromaArrayType != 1)
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
static void printVUI(vui_t *vui)
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
    TRACE_1(PARAM, "    - aspect_ratio_info_present_flag= %u", vui->aspect_ratio_info_present_flag);
    if (vui->aspect_ratio_info_present_flag)
    {
        TRACE_1(PARAM, "    - aspect_ratio_idc  = %u", vui->aspect_ratio_idc);
        if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
        {
            TRACE_1(PARAM, "    - sar_width         = %u", vui->sar_width);
            TRACE_1(PARAM, "    - sar_height        = %u", vui->sar_height);
        }
    }

    TRACE_1(PARAM, "    - overscan_info_present_flag= %u", vui->overscan_info_present_flag);
    if (vui->overscan_info_present_flag)
    {
        TRACE_1(PARAM, "    - overscan_appropriate_flag= %u", vui->overscan_appropriate_flag);
    }

    TRACE_1(PARAM, "    - video_signal_type_present_flag= %u", vui->video_signal_type_present_flag);
    if (vui->video_signal_type_present_flag)
    {
        TRACE_1(PARAM, "    - video_format          = %u", vui->video_format);
        TRACE_1(PARAM, "    - video_full_range_flag = %u", vui->video_full_range_flag);
        TRACE_1(PARAM, "    - colour_description_present_flag= %u", vui->colour_description_present_flag);
        if (vui->colour_description_present_flag)
        {
            TRACE_1(PARAM, "    - colour_primaries          = %u", vui->colour_primaries);
            TRACE_1(PARAM, "    - transfer_characteristics  = %u", vui->transfer_characteristics);
            TRACE_1(PARAM, "    - matrix_coefficients       = %u", vui->matrix_coefficients);
        }
    }

    TRACE_1(PARAM, "    - chroma_loc_info_present_flag          = %u", vui->chroma_loc_info_present_flag);
    if (vui->chroma_loc_info_present_flag)
    {
        TRACE_1(PARAM, "    - chroma_sample_loc_type_top_field      = %u", vui->chroma_sample_loc_type_top_field);
        TRACE_1(PARAM, "    - chroma_sample_loc_type_bottom_field   = %u", vui->chroma_sample_loc_type_bottom_field);
    }

    TRACE_1(PARAM, "    - timing_info_present_flag      = %u", vui->timing_info_present_flag);
    if (vui->timing_info_present_flag)
    {
        TRACE_1(PARAM, "    - num_units_in_tick     = %u", vui->num_units_in_tick);
        TRACE_1(PARAM, "    - time_scale            = %u", vui->time_scale);
        TRACE_1(PARAM, "    - fixed_frame_rate_flag = %u", vui->fixed_frame_rate_flag);
    }

    TRACE_1(PARAM, "    - nal_hrd_parameters_present_flag= %u", vui->nal_hrd_parameters_present_flag);
    TRACE_1(PARAM, "    - vcl_hrd_parameters_present_flag= %u", vui->vcl_hrd_parameters_present_flag);

    // HRD
    if (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true)
    {
        if (vui->nal_hrd_parameters_present_flag == true)
            printHRD(vui->nal_hrd);
        if (vui->vcl_hrd_parameters_present_flag == true)
            printHRD(vui->vcl_hrd);

        TRACE_1(PARAM, "    - low_delay_hrd_flag        = %u", vui->low_delay_hrd_flag);
    }

    TRACE_1(PARAM, "    - pic_struct_present_flag       = %u", vui->pic_struct_present_flag);
    TRACE_1(PARAM, "    - bitstream_restriction_flag    = %u", vui->bitstream_restriction_flag);

    if (vui->bitstream_restriction_flag)
    {
        TRACE_1(PARAM, "    - motion_vectors_over_pic_boundaries_flag = %u", vui->motion_vectors_over_pic_boundaries_flag);
        TRACE_1(PARAM, "    - max_bytes_per_pic_denom       = %u", vui->max_bytes_per_pic_denom);
        TRACE_1(PARAM, "    - max_bits_per_mb_denom         = %u", vui->max_bits_per_mb_denom);
        TRACE_1(PARAM, "    - log2_max_mv_length_horizontal = %u", vui->log2_max_mv_length_horizontal);
        TRACE_1(PARAM, "    - log2_max_mv_length_vertical   = %u", vui->log2_max_mv_length_vertical);
        TRACE_1(PARAM, "    - num_reorder_frames            = %u", vui->num_reorder_frames);
        TRACE_1(PARAM, "    - max_dec_frame_buffering       = %u", vui->max_dec_frame_buffering);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

static void mapVUI(vui_t *vui, FILE *xml)
{
    if (vui && xml)
    {
        xmlSpacer(xml, "Video Usability Information", -1);

        fprintf(xml, "  <aspect_ratio_info_present_flag>%i</aspect_ratio_info_present_flag>\n", vui->aspect_ratio_info_present_flag);
        if (vui->aspect_ratio_info_present_flag)
        {
            fprintf(xml, "  <aspect_ratio_idc>%u</aspect_ratio_idc>\n", vui->aspect_ratio_idc);
            if (vui->aspect_ratio_idc == 255) // 255 : Extended_SAR
            {
                fprintf(xml, "  <sar_width>%u</sar_width>\n", vui->sar_width);
                fprintf(xml, "  <sar_width>%u</sar_width>\n", vui->sar_width);
            }
        }

        fprintf(xml, "  <overscan_info_present_flag>%i</overscan_info_present_flag>\n", vui->overscan_info_present_flag);
        if (vui->overscan_info_present_flag)
        {
            fprintf(xml, "  <overscan_appropriate_flag>%u</overscan_appropriate_flag>\n", vui->overscan_appropriate_flag);
        }

        fprintf(xml, "  <video_signal_type_present_flag>%i</video_signal_type_present_flag>\n", vui->video_signal_type_present_flag);
        if (vui->video_signal_type_present_flag)
        {
            fprintf(xml, "  <video_format>%u</video_format>\n", vui->video_format);
            fprintf(xml, "  <video_full_range_flag>%i</video_full_range_flag>\n", vui->video_full_range_flag);
            fprintf(xml, "  <colour_description_present_flag>%i</colour_description_present_flag>\n", vui->colour_description_present_flag);
            if (vui->colour_description_present_flag)
            {
                fprintf(xml, "  <colour_primaries>%u</colour_primaries>\n", vui->colour_primaries);
                fprintf(xml, "  <transfer_characteristics>%u</transfer_characteristics>\n", vui->transfer_characteristics);
                fprintf(xml, "  <matrix_coefficients>%u</matrix_coefficients>\n", vui->matrix_coefficients);
            }
        }

        fprintf(xml, "  <chroma_loc_info_present_flag>%i</chroma_loc_info_present_flag>\n", vui->chroma_loc_info_present_flag);
        if (vui->chroma_loc_info_present_flag)
        {
            fprintf(xml, "  <chroma_sample_loc_type_top_field>%u</chroma_sample_loc_type_top_field>\n", vui->chroma_sample_loc_type_top_field);
            fprintf(xml, "  <chroma_sample_loc_type_bottom_field>%u</chroma_sample_loc_type_bottom_field>\n", vui->chroma_sample_loc_type_bottom_field);
        }

        fprintf(xml, "  <timing_info_present_flag>%i</timing_info_present_flag>\n", vui->timing_info_present_flag);
        if (vui->timing_info_present_flag)
        {
            fprintf(xml, "  <timing_info_present_flag>%u</timing_info_present_flag>\n", vui->timing_info_present_flag);
            fprintf(xml, "  <time_scale>%u</time_scale>\n", vui->time_scale);
            fprintf(xml, "  <fixed_frame_rate_flag>%u</fixed_frame_rate_flag>\n", vui->fixed_frame_rate_flag);
        }

        fprintf(xml, "  <nal_hrd_parameters_present_flag>%i</nal_hrd_parameters_present_flag>\n", vui->nal_hrd_parameters_present_flag);
        fprintf(xml, "  <vcl_hrd_parameters_present_flag>%i</vcl_hrd_parameters_present_flag>\n", vui->vcl_hrd_parameters_present_flag);

        // HRD
        if (vui->nal_hrd_parameters_present_flag == true || vui->vcl_hrd_parameters_present_flag == true)
        {
            if (vui->nal_hrd_parameters_present_flag == true)
                mapHRD(vui->nal_hrd, xml);
            if (vui->vcl_hrd_parameters_present_flag == true)
                mapHRD(vui->vcl_hrd, xml);
            fprintf(xml, "  <vcl_hrd_parameters_present_flag>%i</vcl_hrd_parameters_present_flag>\n", vui->vcl_hrd_parameters_present_flag);
        }

        fprintf(xml, "  <pic_struct_present_flag>%i</pic_struct_present_flag>\n", vui->pic_struct_present_flag);
        fprintf(xml, "  <bitstream_restriction_flag>%i</bitstream_restriction_flag>\n", vui->bitstream_restriction_flag);

        if (vui->bitstream_restriction_flag)
        {
            fprintf(xml, "  <motion_vectors_over_pic_boundaries_flag>%u</motion_vectors_over_pic_boundaries_flag>\n", vui->motion_vectors_over_pic_boundaries_flag);
            fprintf(xml, "  <max_bytes_per_pic_denom>%u</max_bytes_per_pic_denom>\n", vui->max_bytes_per_pic_denom);
            fprintf(xml, "  <max_bits_per_mb_denom>%u</max_bits_per_mb_denom>\n", vui->max_bits_per_mb_denom);
            fprintf(xml, "  <log2_max_mv_length_horizontal>%u</log2_max_mv_length_horizontal>\n", vui->log2_max_mv_length_horizontal);
            fprintf(xml, "  <log2_max_mv_length_vertical>%u</log2_max_mv_length_vertical>\n", vui->log2_max_mv_length_vertical);
            fprintf(xml, "  <num_reorder_frames>%u</num_reorder_frames>\n", vui->num_reorder_frames);
            fprintf(xml, "  <max_dec_frame_buffering>%u</max_dec_frame_buffering>\n", vui->max_dec_frame_buffering);
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param bitstr: Our bitstream reader.
 * \return hrd_t initialized data structure.
 *
 * From 'ITU-T H.264' recommendation:
 * E.1.2 HRD parameters syntax.
 * E.2.2 HRD parameters semantics.
 *
 * HRD can only be found inside VUI structure.
 * This function must only be called by checkVUI().
 */
hrd_t *decodeHRD(Bitstream_t *bitstr)
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
        ////////////////////////////////////////////////////////////////////////

        hrd->cpb_cnt_minus1 = read_ue(bitstr);
        hrd->bit_rate_scale = read_bits(bitstr, 4);
        hrd->cpb_size_scale = read_bits(bitstr, 4);

        for (unsigned SchedSelIdx = 0;
             SchedSelIdx <= hrd->cpb_cnt_minus1 && SchedSelIdx < MAX_CPB;
             SchedSelIdx++)
        {
            hrd->bit_rate_value_minus1[SchedSelIdx] = read_ue(bitstr);
            hrd->cpb_size_value_minus1[SchedSelIdx] = read_ue(bitstr);
            hrd->CpbSize[SchedSelIdx] = (hrd->cpb_size_value_minus1[SchedSelIdx] + 1) * pow(2, 4 + hrd->cpb_size_scale);
            hrd->cbr_flag[SchedSelIdx] = read_bit(bitstr);
        }

        hrd->initial_cpb_removal_delay_length_minus1 = read_bits(bitstr, 5);
        hrd->cpb_removal_delay_length_minus1 = read_bits(bitstr, 5);
        hrd->dpb_output_delay_length_minus1 = read_bits(bitstr, 5);
        hrd->time_offset_length = read_bits(bitstr, 5);
    }

    return hrd;
}

/* ************************************************************************** */

/*!
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
static int checkHRD(hrd_t *hrd)
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
        if (hrd->cpb_cnt_minus1 > (MAX_CPB - 1))
        {
            TRACE_WARNING(PARAM, "      - cpb_cnt_minus1 is %i, but should be between 0 and 31", hrd->cpb_cnt_minus1);
            retcode = FAILURE;
        }

        //FIXME No boundaries specified?
        //TRACE_1(PARAM, "      - bit_rate_scale    : %i", hrd->bit_rate_scale);
        //TRACE_1(PARAM, "      - cpb_size_scale    : %i", hrd->cpb_size_scale);

        for (unsigned SchedSelIdx = 0;
             SchedSelIdx <= hrd->cpb_cnt_minus1 && SchedSelIdx < MAX_CPB;
             SchedSelIdx++)
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
static void printHRD(hrd_t *hrd)
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
    TRACE_1(PARAM, "      - cpb_cnt_minus1  = %u", hrd->cpb_cnt_minus1);
    TRACE_1(PARAM, "      - bit_rate_scale  = %u", hrd->bit_rate_scale);
    TRACE_1(PARAM, "      - cpb_size_scale  = %u", hrd->cpb_size_scale);

    for (unsigned SchedSelIdx = 0;
         SchedSelIdx <= hrd->cpb_cnt_minus1 && SchedSelIdx < MAX_CPB;
         SchedSelIdx++)
    {
        TRACE_1(PARAM, "      - bit_rate_value_minus1[%u]       = %u", SchedSelIdx, hrd->bit_rate_value_minus1[SchedSelIdx]);
        TRACE_1(PARAM, "      - cpb_size_value_minus1[%u]       = %u", SchedSelIdx, hrd->cpb_size_value_minus1[SchedSelIdx]);
        TRACE_1(PARAM, "      - cbr_flag[%u]                    = %u", SchedSelIdx, hrd->cbr_flag[SchedSelIdx]);
    }

    TRACE_1(PARAM, "      - initial_cpb_removal_delay_length_minus1 = %u", hrd->initial_cpb_removal_delay_length_minus1);
    TRACE_1(PARAM, "      - cpb_removal_delay_length_minus1 = %u", hrd->cpb_removal_delay_length_minus1);
    TRACE_1(PARAM, "      - dpb_output_delay_length_minus1  = %u", hrd->dpb_output_delay_length_minus1);
    TRACE_1(PARAM, "      - time_offset_length              = %u", hrd->time_offset_length);
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

static void mapHRD(hrd_t *hrd, FILE *xml)
{
    if (hrd && xml)
    {
        xmlSpacer(xml, "Hypothetical Reference Decoder", -1);

        fprintf(xml, "  <cpb_cnt_minus1>%u</cpb_cnt_minus1>\n", hrd->cpb_cnt_minus1);
        fprintf(xml, "  <bit_rate_scale>%u</bit_rate_scale>\n", hrd->bit_rate_scale);
        fprintf(xml, "  <cpb_size_scale>%u</cpb_size_scale>\n", hrd->cpb_size_scale);

        for (unsigned SchedSelIdx = 0;
             SchedSelIdx <= hrd->cpb_cnt_minus1 && SchedSelIdx <= (MAX_CPB - 1);
             SchedSelIdx++)
        {
            fprintf(xml, "  <bit_rate_value_minus1_%u>%u</bit_rate_value_minus1_%u>\n", SchedSelIdx, hrd->bit_rate_value_minus1[SchedSelIdx], SchedSelIdx);
            fprintf(xml, "  <cpb_size_value_minus1_%u>%u</cpb_size_value_minus1_%u>\n", SchedSelIdx, hrd->cpb_size_value_minus1[SchedSelIdx], SchedSelIdx);
            fprintf(xml, "  <cbr_flag_%u>%u</cbr_flag_%u>\n", SchedSelIdx, hrd->cbr_flag[SchedSelIdx], SchedSelIdx);
        }

        fprintf(xml, "  <initial_cpb_removal_delay_length_minus1>%u</initial_cpb_removal_delay_length_minus1>\n", hrd->initial_cpb_removal_delay_length_minus1);
        fprintf(xml, "  <cpb_removal_delay_length_minus1>%u</cpb_removal_delay_length_minus1>\n", hrd->cpb_removal_delay_length_minus1);
        fprintf(xml, "  <dpb_output_delay_length_minus1>%u</dpb_output_delay_length_minus1>\n", hrd->dpb_output_delay_length_minus1);
        fprintf(xml, "  <time_offset_length>%u</time_offset_length>\n", hrd->time_offset_length);
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
