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
 * \file      h264_macroblock.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_macroblock.h"
#include "h264_macroblock_utils.h"
#include "h264_expgolomb.h"
#include "h264_cavlc.h"
#include "h264_cabac.h"
#include "h264_spatial.h"
#include "h264_intra_prediction.h"
#include "h264_inter_prediction.h"
#include "../../utils.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

// C++ standard libraries
#include <algorithm>

/* ************************************************************************** */

// Prediction mode
static void mb_pred(DecodingContext_t *dc, Macroblock_t *mb);
static void sub_mb_pred(DecodingContext_t *dc, const unsigned int mb_type, unsigned int *sub_mb_type);

// Macroblock related functions
static int NumMbPart(const unsigned int slice_type, const unsigned int mb_type);
static int MbPartPredMode(Macroblock_t *mb, const unsigned int slice_type, const int mbPartIdx);
static int MbPartWidth(const unsigned int slice_type, const unsigned int mb_type);
static int MbPartHeight(const unsigned int slice_type, const unsigned int mb_type);
static void MbPosition(Macroblock_t *mb, sps_t *sps);

// Sub-macroblock related functions
static int NumSubMbPart(const unsigned int slice_type, const unsigned int sub_mb_type);
static int SubMbPredMode(const unsigned int slice_type, const unsigned int sub_mb_type);
static int SubMbPartWidth(const unsigned int slice_type, const unsigned int sub_mb_type);
static int SubMbPartHeight(const unsigned int slice_type, const unsigned int sub_mb_type);

// Residual data
static void residual_luma(DecodingContext_t *dc, const int startIdx, const int endIdx);
static void residual_chroma(DecodingContext_t *dc, const int startIdx, const int endIdx);

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param mbAddr The current macroblock address.
 * \return 0 if macroblock decoding fail, 1 otherwise.
 *
 * This function extract one macroblock from the bitstream, handle intra/inter
 * prediction for its blocks.
 */
int macroblock_layer(DecodingContext_t *dc, const int mbAddr)
{
    TRACE_INFO(MB, "<> " BLD_GREEN "macroblock_layer(" CLR_RESET "%i" BLD_GREEN ")" CLR_RESET, mbAddr);
    int retcode = FAILURE;

    // Macroblock allocation
    ////////////////////////////////////////////////////////////////////////////

    dc->mb_array[mbAddr] = (Macroblock_t*)calloc(1, sizeof(Macroblock_t));

    if (dc->mb_array[mbAddr] == NULL)
    {
        TRACE_ERROR(MB, "Unable to alloc new macroblock!");
    }
    else
    {
        // Set macroblock address
        dc->mb_array[mbAddr]->mbAddr = mbAddr;

        // Shortcuts
        pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
        sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];
        slice_t *slice = dc->active_slice;
        Macroblock_t *mb = dc->mb_array[mbAddr];

        // Macroblock decoding
        ////////////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
        mb->mbFileAddrStart = bitstream_get_absolute_bit_offset(dc->bitstr);
#endif // ENABLE_DEBUG

        deriv_macroblockneighbours_availability(dc, mbAddr);
        MbPosition(mb, sps);

        if (pps->entropy_coding_mode_flag)
            mb->mb_type = read_ae(dc, SE_mb_type);
        else
            mb->mb_type = read_ue(dc->bitstr);

        mb->MbPartPredMode[0] = MbPartPredMode(mb, slice->slice_type, 0);
        mb->NumMbPart = NumMbPart(slice->slice_type, mb->mb_type);

        if (mb->mb_type == I_PCM)
        {
#if ENABLE_IPCM
            TRACE_3(MB, "---- macroblock_layer - I PCM macroblock");

            while (bitstream_check_alignment(dc->bitstr) == false)
            {
                if (read_bit(dc->bitstr) != 0) // pcm_alignment_zero_bit
                {
                    TRACE_ERROR(MB, "  Error while reading pcm_alignment_zero_bit: must be 0!");
                    return FAILURE;
                }
            }

            // CABAC initialization process //FIXME needed? See 'ITU-T H.264' recommendation 9.3.1.2
            initCabacDecodingEngine(dc);

            int i = 0;
            for (i = 0; i < 256; i++)
            {
                mb->pcm_sample_luma[i] = (uint8_t)read_bits(dc->bitstr, sps->BitDepthY);
            }

            // CABAC initialization process //FIXME needed? See 'ITU-T H.264' recommendation 9.3.1.2
            initCabacDecodingEngine(dc);

            for (i = 0; i < 2 * sps->MbWidthC * sps->MbHeightC; i++)
            {
                mb->pcm_sample_chroma[i] = (uint8_t)read_bits(dc->bitstr, sps->BitDepthC);
            }

            // CABAC initialization process //FIXME needed? See 'ITU-T H.264' recommendation 9.3.1.2
            initCabacDecodingEngine(dc);
#else // ENABLE_IPCM
            TRACE_ERROR(MB, "I_PCM decoding is currently disabled!");
            return UNSUPPORTED;
#endif // ENABLE_IPCM
        }
        else
        {
#if ENABLE_INTER_PRED
            bool noSubMbPartSizeLessThan8x8Flag = true;

            if (mb->mb_type != I_NxN &&
                mb->MbPartPredMode[0] != Intra_16x16 &&
                mb->NumMbPart == 4)
            {
                TRACE_3(MB, "---- macroblock_layer - mb partition & related");

                int mbPartIdx = 0;
                for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
                {
                    if (mb->sub_mb_type[mbPartIdx] != B_Direct_8x8)
                    {
                        if (NumSubMbPart(slice->slice_type, mb->sub_mb_type[mbPartIdx]) > 1)
                        {
                            noSubMbPartSizeLessThan8x8Flag = false;
                        }
                    }
                    else if (sps->direct_8x8_inference_flag == false)
                    {
                        noSubMbPartSizeLessThan8x8Flag = false;
                    }
                }

                // Read sub macroblock prediction mode
                sub_mb_pred(dc, mb->mb_type, mb->sub_mb_type);
            }
            else
#endif // ENABLE_INTER_PRED
            {
                TRACE_3(MB, "---- macroblock_layer - transform_size_8x8_flag & prediction modes");

                if (pps->transform_8x8_mode_flag == true && mb->mb_type == I_NxN)
                {
                    if (pps->entropy_coding_mode_flag)
                        mb->transform_size_8x8_flag = read_ae(dc, SE_transform_size_8x8_flag);
                    else
                        mb->transform_size_8x8_flag = read_bit(dc->bitstr);

                    // Need to update MbPartPredMode in order to detect I_8x8 prediction mode
                    mb->MbPartPredMode[0] = MbPartPredMode(mb, slice->slice_type, 0);
                }

                // Read macroblock prediction mode
                mb_pred(dc, mb);
            }

            if (mb->MbPartPredMode[0] != Intra_16x16)
            {
                TRACE_3(MB, "---- macroblock_layer - coded block pattern & transform_size_8x8_flag");

                if (pps->entropy_coding_mode_flag)
                    mb->coded_block_pattern = read_ae(dc, SE_coded_block_pattern);
                else
                    mb->coded_block_pattern = read_me(dc->bitstr, sps->ChromaArrayType, dc->IdrPicFlag);

                mb->CodedBlockPatternLuma = mb->coded_block_pattern % 16;
                mb->CodedBlockPatternChroma = mb->coded_block_pattern / 16;
#if ENABLE_INTER_PRED
                if (mb->CodedBlockPatternLuma > 0 &&
                    pps->transform_8x8_mode_flag == true &&
                    mb->mb_type != I_NxN &&
                    noSubMbPartSizeLessThan8x8Flag == true &&
                    (mb->mb_type != B_Direct_16x16 || sps->direct_8x8_inference_flag == true))
                {
                    if (pps->entropy_coding_mode_flag)
                        mb->transform_size_8x8_flag = read_ae(dc, SE_transform_size_8x8_flag);
                    else
                        mb->transform_size_8x8_flag = read_bit(dc->bitstr);

                    // Need to update MbPartPredMode in order to account for I_8x8 prediction mode
                    if (transform_size_8x8_flag)
                        mb->MbPartPredMode[0] = MbPartPredMode(mb, slice->slice_type, 0);
                }
#endif // ENABLE_INTER_PRED
            }

            if (mb->CodedBlockPatternLuma > 0 ||
                mb->CodedBlockPatternChroma > 0 ||
                mb->MbPartPredMode[0] == Intra_16x16)
            {
                TRACE_3(MB, "---- macroblock_layer - quantization parameter & residual datas");

                // Read QP delta
                if (pps->entropy_coding_mode_flag)
                    mb->mb_qp_delta = read_ae(dc, SE_mb_qp_delta);
                else
                    mb->mb_qp_delta = read_se(dc->bitstr);

                // Parse the residual coefficients
                ////////////////////////////////////////////////////////////////

                // Luma levels
                residual_luma(dc, 0, 15);

                // Chroma levels
                residual_chroma(dc, 0, 15);
            }
            else
            {
                TRACE_3(MB, "---- macroblock_layer - No residual datas to decode in this macroblock");
            }

            // Compute luma Quantization Parameters
            if (mb->mb_qp_delta)
                mb->QPY = ((slice->QPYprev + mb->mb_qp_delta + 52 + sps->QpBdOffsetY*2) % (52 + sps->QpBdOffsetY)) - sps->QpBdOffsetY;
            else
                mb->QPY = slice->QPYprev;

            mb->QPprimeY = mb->QPY + sps->QpBdOffsetY;
            slice->QPYprev = mb->QPY;

            // Set Transform Bypass Mode
            if (sps->qpprime_y_zero_transform_bypass_flag == true && mb->QPprimeY == 0)
                mb->TransformBypassModeFlag = true;

            // Prediction process (include quantization and transformation stages)
            ////////////////////////////////////////////////////////////////

            if (dc->IdrPicFlag)
            {
                retcode = intra_prediction_process(dc, mb);
            }
            else
            {
                retcode = inter_prediction_process(dc, mb);
            }

            // Print macroblock(s) header and block data ?
            ////////////////////////////////////////////////////////////////

#if ENABLE_DEBUG
            mb->mbFileAddrStop = bitstream_get_absolute_bit_offset(dc->bitstr) - 1;

            int frame_debug_range[2] = {-1, -1}; // Range of (idr) frame(s) to debug/analyse
            int mb_debug_range[2] = {-1, -1}; // Range of macroblock(s) to debug/analyse

            if ((int)dc->idrCounter >= frame_debug_range[0] &&
                (int)dc->idrCounter <= frame_debug_range[1])
            {
                if ((int)mb->mbAddr >= mb_debug_range[0] &&
                    (int)mb->mbAddr <= mb_debug_range[1])
                {
                    print_macroblock_layer(dc, mb);
                    print_macroblock_pixel_residual(mb);
                    print_macroblock_pixel_predicted(mb);
                    print_macroblock_pixel_final(mb);
                }
            }
#endif // ENABLE_DEBUG
        }

        TRACE_3(MB, "---- macroblock_layer - the end");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The DecodingContext containing macroblock array we want to freed.
 *
 * This function free **mb_array, which is an array of pointers to macroblock elements.
 * mb_array is in a DecodingContext structure. There can only be one mb_array at a time.
 * Please note that in order to avoid memory leak, this function begin by calling freeMbArrayContent().
 */
void freeMbArray(DecodingContext_t *dc)
{
    if (dc->mb_array != NULL)
    {
        freeMbArrayContent(dc);

        free(dc->mb_array);
        dc->mb_array = NULL;

        TRACE_1(MB, ">> mb_array freed");
    }
}

/* ************************************************************************** */

/*!
 * \param *dc The DecodingContext containing macroblock array we want to freed.
 *
 * This function try to freed every macroblock of the picture stored in mb_array.
 * The total number of macroblocks is equal to (picture width / 16) * (picture height / 16).
 */
void freeMbArrayContent(DecodingContext_t *dc)
{
    if (dc->mb_array != NULL)
    {
        unsigned int i = 0;
        for (i = 0; i < dc->PicSizeInMbs; i++)
        {
            if (dc->mb_array[i] != NULL)
            {
                free(dc->mb_array[i]);
                dc->mb_array[i] = NULL;
            }
        }

        TRACE_2(MB, ">> mb_array content freed");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param CurrMbAddr The address of the current macroblock.
 * \return The address of the next macroclock.
 *
 * Because this decoder do not support interlaced mode or slice partitioning or
 * flexible macroblock ordering (FMO) the NextMbAddress returned by this function
 * is always CurrMbAddr + 1.
 */
unsigned int NextMbAddress(DecodingContext_t *dc, const unsigned int CurrMbAddr)
{
    TRACE_3(MB, "> " BLD_GREEN "NextMbAddress()" "= %i" CLR_RESET, CurrMbAddr + 1);

    return CurrMbAddr + 1;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Read prediction informations for current macroblock.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 *
 * Intra prediction infos are a table containing prediction mode for each blocks.
 * Inter prediction infos are motion vectors.
 */
static void mb_pred(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "mb_pred()" CLR_RESET);

    if (mb->MbPartPredMode[0] == Intra_4x4 ||
        mb->MbPartPredMode[0] == Intra_8x8 ||
        mb->MbPartPredMode[0] == Intra_16x16)
    {
        if (mb->MbPartPredMode[0] == Intra_4x4)
        {
            // Read intra prediction mode for all 16 4x4 luma blocks
            unsigned int luma4x4BlkIdx = 0;
            for (luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
            {
                if (dc->entropy_coding_mode_flag)
                    mb->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = read_ae(dc, SE_prev_intraxxx_pred_mode_flag);
                else
                    mb->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = read_bit(dc->bitstr);

                if (mb->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == false)
                {
                    if (dc->entropy_coding_mode_flag)
                        mb->rem_intra4x4_pred_mode[luma4x4BlkIdx] = read_ae(dc, SE_rem_intraxxx_pred_mode);
                    else
                        mb->rem_intra4x4_pred_mode[luma4x4BlkIdx] = read_bits(dc->bitstr, 3);
                }
            }
        }
        else if (mb->MbPartPredMode[0] == Intra_8x8)
        {
            // Read intra prediction mode for all 4 8x8 luma blocks
            unsigned int luma8x8BlkIdx = 0;
            for (luma8x8BlkIdx = 0; luma8x8BlkIdx < 4; luma8x8BlkIdx++)
            {
                if (dc->entropy_coding_mode_flag)
                    mb->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx] = read_ae(dc, SE_prev_intraxxx_pred_mode_flag);
                else
                    mb->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx] = read_bit(dc->bitstr);

                if (mb->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx] == false)
                {
                    if (dc->entropy_coding_mode_flag)
                        mb->rem_intra8x8_pred_mode[luma8x8BlkIdx] = read_ae(dc, SE_rem_intraxxx_pred_mode);
                    else
                        mb->rem_intra8x8_pred_mode[luma8x8BlkIdx] = read_bits(dc->bitstr, 3);
                }
            }
        }

        // Read intra prediction mode for chroma blocks
        if (dc->ChromaArrayType == 1 || dc->ChromaArrayType == 2)
        {
            if (dc->entropy_coding_mode_flag)
                mb->IntraChromaPredMode = read_ae(dc, SE_intra_chroma_pred_mode);
            else
                mb->IntraChromaPredMode = read_ue(dc->bitstr);
        }
    }
    else if (mb->MbPartPredMode[0] != Direct)
    {
        // ref_idx_l0
        unsigned int mbPartIdx = 0;
        for (mbPartIdx = 0; mbPartIdx < mb->NumMbPart; mbPartIdx++)
        {
            if ((dc->active_slice->num_ref_idx_l0_active_minus1 > 0 ||
                 dc->active_slice->mb_field_decoding_flag != dc->active_slice->field_pic_flag) &&
                MbPartPredMode(mb, dc->active_slice->slice_type, mbPartIdx) != Pred_L1)
            {
                if (dc->entropy_coding_mode_flag)
                    mb->ref_idx_l0[mbPartIdx] = read_ae(dc, SE_ref_idx_lx);
                else
                    mb->ref_idx_l0[mbPartIdx] = read_te(dc->bitstr, 0);
            }
        }

        // ref_idx_l1
        for (mbPartIdx = 0; mbPartIdx < mb->NumMbPart; mbPartIdx++)
        {
            if ((dc->active_slice->num_ref_idx_l1_active_minus1 > 0 ||
                 dc->active_slice->mb_field_decoding_flag != dc->active_slice->field_pic_flag) &&
                MbPartPredMode(mb, dc->active_slice->slice_type, mbPartIdx) != Pred_L0)
            {
                if (dc->entropy_coding_mode_flag)
                    mb->ref_idx_l1[mbPartIdx] = read_ae(dc, SE_ref_idx_lx);
                else
                    mb->ref_idx_l1[mbPartIdx] = read_te(dc->bitstr, 0);
            }
        }

        // mvd_l0
        for (mbPartIdx = 0; mbPartIdx < mb->NumMbPart; mbPartIdx++)
        {
            if (MbPartPredMode(mb, dc->active_slice->slice_type, mbPartIdx) != Pred_L1)
            {
                if (dc->entropy_coding_mode_flag)
                {
                    mb->mvd_l0[mbPartIdx][0][0] = read_ae(dc, SE_mvd_lx0);
                    mb->mvd_l0[mbPartIdx][0][1] = read_ae(dc, SE_mvd_lx1);
                }
                else
                {
                    mb->mvd_l0[mbPartIdx][0][0] = read_te(dc->bitstr, 0);
                    mb->mvd_l0[mbPartIdx][0][1] = read_te(dc->bitstr, 0);
                }
            }
        }

        // mvd_l1
        for (mbPartIdx = 0; mbPartIdx < mb->NumMbPart; mbPartIdx++)
        {
            if (MbPartPredMode(mb, dc->active_slice->slice_type, mbPartIdx) != Pred_L0)
            {
                if (dc->entropy_coding_mode_flag)
                {
                    mb->mvd_l1[mbPartIdx][0][0] = read_ae(dc, SE_mvd_lx0);
                    mb->mvd_l1[mbPartIdx][0][1] = read_ae(dc, SE_mvd_lx1);
                }
                else
                {
                    mb->mvd_l1[mbPartIdx][0][0] = read_te(dc->bitstr, 0);
                    mb->mvd_l1[mbPartIdx][0][1] = read_te(dc->bitstr, 0);
                }
            }
        }
    }
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param mb_type The macroblock prediction type.
 * \param *sub_mb_type The sub macroblock prediction type.
 *
 * Find the sub macroblock prediction type (intra prediction mode or motion vectors exctraction).
 */
static void sub_mb_pred(DecodingContext_t *dc, const unsigned int mb_type, unsigned int *sub_mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "sub_mb_pred()" CLR_RESET);

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];
    slice_t *slice = dc->active_slice;

    // Read sub_mb_type
    int mbPartIdx = 0;
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if (dc->entropy_coding_mode_flag)
            sub_mb_type[mbPartIdx] = read_ae(dc, SE_sub_mb_type);
        else
            sub_mb_type[mbPartIdx] = read_ue(dc->bitstr);
    }

    // ref_idx_l0
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if ((slice->num_ref_idx_l0_active_minus1 > 0 ||
             slice->mb_field_decoding_flag != slice->field_pic_flag) &&
            mb_type != P_8x8ref0 &&
            sub_mb_type[mbPartIdx] != B_Direct_8x8 &&
            SubMbPredMode(slice->slice_type, sub_mb_type[mbPartIdx]) != Pred_L1)
        {
            if (dc->entropy_coding_mode_flag)
                mb->ref_idx_l0[mbPartIdx] = read_ae(dc, SE_ref_idx_lx);
            else
                mb->ref_idx_l0[mbPartIdx] = read_te(dc->bitstr, 0);
        }
    }

    // ref_idx_l1
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if ((slice->num_ref_idx_l1_active_minus1 > 0 ||
             slice->mb_field_decoding_flag != slice->field_pic_flag) &&
            sub_mb_type[mbPartIdx] != B_Direct_8x8 &&
            SubMbPredMode(slice->slice_type, sub_mb_type[mbPartIdx]) != Pred_L0)
        {
            if (dc->entropy_coding_mode_flag)
                mb->ref_idx_l1[mbPartIdx] = read_ae(dc, SE_ref_idx_lx);
            else
                mb->ref_idx_l1[mbPartIdx] = read_te(dc->bitstr, 0);
        }
    }

    // mvd_l0
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if (sub_mb_type[mbPartIdx] != B_Direct_8x8 &&
            SubMbPredMode(slice->slice_type, sub_mb_type[mbPartIdx]) != Pred_L1)
        {
            int subMbPartIdx = 0;
            for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(slice->slice_type, sub_mb_type[mbPartIdx]); subMbPartIdx++)
            {
                if (dc->entropy_coding_mode_flag)
                {
                    mb->mvd_l0[mbPartIdx][subMbPartIdx][0] = read_ae(dc, SE_mvd_lx0);
                    mb->mvd_l0[mbPartIdx][subMbPartIdx][1] = read_ae(dc, SE_mvd_lx1);
                }
                else
                {
                    mb->mvd_l0[mbPartIdx][subMbPartIdx][0] = read_se(dc->bitstr);
                    mb->mvd_l0[mbPartIdx][subMbPartIdx][1] = read_se(dc->bitstr);
                }
            }
        }
    }

    // mvd_l1
    for (mbPartIdx = 0; mbPartIdx < 4; mbPartIdx++)
    {
        if (sub_mb_type[mbPartIdx] != B_Direct_8x8 &&
            SubMbPredMode(slice->slice_type, sub_mb_type[mbPartIdx]) != Pred_L0)
        {
            int subMbPartIdx = 0;
            for (subMbPartIdx = 0; subMbPartIdx < NumSubMbPart(slice->slice_type, sub_mb_type[mbPartIdx]); subMbPartIdx++)
            {
                if (dc->entropy_coding_mode_flag)
                {
                    mb->mvd_l1[mbPartIdx][subMbPartIdx][0] = read_ae(dc, SE_mvd_lx0);
                    mb->mvd_l1[mbPartIdx][subMbPartIdx][1] = read_ae(dc, SE_mvd_lx1);
                }
                else
                {
                    mb->mvd_l1[mbPartIdx][subMbPartIdx][0] = read_se(dc->bitstr);
                    mb->mvd_l1[mbPartIdx][subMbPartIdx][1] = read_se(dc->bitstr);
                }
            }
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param slice_type The slice type (I, SI, P, SP, B).
 * \param mb_type The macroblock prediction type.
 * \return The number of partitions used in this macroblock.
 *
 * Return the number of partitions used in this macroblock.
 * Macroblock partitions are only used in P/SP and B slice.
 * Table 7-13: Macroblock type values 0 to 4 for P and SP slices.
 * Table 7-14: Macroblock type values 0 to 22 for B slices.
 */
static int NumMbPart(const unsigned int slice_type, const unsigned int mb_type)
{
    TRACE_2(MB, "  > " BLD_GREEN "NumMbPart()" CLR_RESET);
    int retcode = 1;

    if (slice_type == 0 || slice_type == 5 ||
        slice_type == 3 || slice_type == 8) // P or SP slice
    {
        // P and SP slice
        switch (mb_type)
        {
        case P_L0_16x16:
            retcode = 1;
            break;
        case P_L0_L0_16x8:
            retcode = 2;
            break;
        case P_L0_L0_8x16:
            retcode = 2;
            break;
        case P_8x8:
            retcode = 4;
            break;
        case P_8x8ref0:
            retcode = 4;
            break;
        case P_Skip:
            retcode = 1;
            break;
        }
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        // B slice
        switch (mb_type)
        {
        case B_Direct_16x16:
            retcode = 0; // na
            break;
        case B_L0_16x16:
            retcode = 1;
            break;
        case B_L1_16x16:
            retcode = 1;
            break;
        case B_Bi_16x16:
            retcode = 1;
            break;
        case B_L0_L0_16x8:
            retcode = 2;
            break;
        case B_L0_L0_8x16:
            retcode = 2;
            break;
        case B_L1_L1_16x8:
            retcode = 2;
            break;
        case B_L1_L1_8x16:
            retcode = 2;
            break;
        case B_L0_L1_16x8:
            retcode = 2;
            break;
        case B_L0_L1_8x16:
            retcode = 2;
            break;
        case B_L1_L0_16x8:
            retcode = 2;
            break;
        case B_L1_L0_8x16:
            retcode = 2;
            break;
        case B_L0_Bi_16x8:
            retcode = 2;
            break;
        case B_L0_Bi_8x16:
            retcode = 2;
            break;
        case B_L1_Bi_16x8:
            retcode = 2;
            break;
        case B_L1_Bi_8x16:
            retcode = 2;
            break;
        case B_Bi_L0_16x8:
            retcode = 2;
            break;
        case B_Bi_L0_8x16:
            retcode = 2;
            break;
        case B_Bi_L1_16x8:
            retcode = 2;
            break;
        case B_Bi_L1_8x16:
            retcode = 2;
            break;
        case B_Bi_Bi_16x8:
            retcode = 2;
            break;
        case B_Bi_Bi_8x16:
            retcode = 2;
            break;
        case B_8x8:
            retcode = 4;
            break;
        case B_Skip:
            retcode = 0; // na
            break;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param mb The macroblock data structure.
 * \param slice_type The slice type.
 * \param mbPartIdx The index of the macroblock partition.
 * \return The prediction mode of the macroblock (when it is not partitioned) or the first partition.
 *
 * <!> Only I frames are supported.
 * Return the prediction mode of the macroblock or of the given partition, if
 * mbPartIdx != 1. The prediction mode can be Intra_4x4, Intra_8x8 or Intra_16x16.
 * I_PCM macroblocks do not use prediction, therefore 0 is returned.
 * Use Table 7-11, Macroblock types for I slices.
 */
static int MbPartPredMode(Macroblock_t *mb, const unsigned int slice_type, const int mbPartIdx)
{
    TRACE_2(MB, "  > " BLD_GREEN "MbPartPredMode()" CLR_RESET);
    int retcode = 0;

    if (slice_type == 2 || slice_type == 7) // I slice
    {
        if (mbPartIdx != 1)
        {
            if (mb->mb_type == I_NxN)
            {
                if (mb->transform_size_8x8_flag)
                    retcode = Intra_8x8;
                else
                    retcode = Intra_4x4;
            }
            else if (mb->mb_type < I_PCM)
            {
                retcode = Intra_16x16;

                // Intra 16x16 Prediction Mode, see Table 7-11
                unsigned int i = 1, j = 0;
                for (i = 1; i < 5; i++)
                {
                    for (j = 0; j < 6; j++)
                    {
                        if (mb->mb_type == (i + j*4))
                        {
                            mb->Intra16x16PredMode = i-1;
                            i = j = 9;
                        }
                    }
                }

                // Coded Block Pattern chroma/luma, see Table 7-11
                if (mb->mb_type > 4 /*&& mb->mb_type < I_PCM*/)
                {
                    if (mb->mb_type > 12)
                    {
                        mb->CodedBlockPatternLuma = 15;

                        if (mb->mb_type > 20)
                            mb->CodedBlockPatternChroma = 2;
                        else if (mb->mb_type > 16)
                            mb->CodedBlockPatternChroma = 1;
                    }
                    else
                    {
                        if (mb->mb_type > 8)
                            mb->CodedBlockPatternChroma = 2;
                        else if (mb->mb_type > 4)
                            mb->CodedBlockPatternChroma = 1;
                    }
                }
            }
            else //if (mb->mb_type == I_PCM)
            {
                retcode = 0;
            }
        }
    }
    else if (slice_type == 4 || slice_type == 9) // SI slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED (SI slice)");
    }
    else if (slice_type == 0 || slice_type == 5 ||
             slice_type == 3 || slice_type == 8) // P or SP slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED (P and SP slice)");
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED (B slice)");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param mb_type The macroblock prediction type.
 *
 * docme.
 */
static int MbPartWidth(const unsigned int slice_type, const unsigned int mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "MbPartWidth()" CLR_RESET);
    int retcode = 0;

    TRACE_WARNING(MB, ">>> UNIMPLEMENTED (MbPartWidth)");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param mb_type The macroblock prediction type.
 *
 * docme.
 */
static int MbPartHeight(const unsigned int slice_type, const unsigned int mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "MbPartHeight()" CLR_RESET);
    int retcode = 0;

    TRACE_WARNING(MB, ">>> UNIMPLEMENTED (MbPartHeight)");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Compute the horizontal and vertical position of the macroblock in the mb_array.
 * \param *mb The macroblock.
 * \param *sps The SPS currently in use.
 */
static void MbPosition(Macroblock_t *mb, sps_t *sps)
{
    TRACE_2(MB, "  > " BLD_GREEN "MbPosition()" CLR_RESET);

    if (mb->mbAddr <= (sps->PicWidthInMbs * sps->FrameHeightInMbs))
    {
        mb->mbAddr_x = mb->mbAddr % sps->PicWidthInMbs;
        mb->mbAddr_y = mb->mbAddr / sps->PicWidthInMbs;
    }
    else
    {
        TRACE_ERROR(MB, "Macroblock %i is out of range! There is only %i macroblocks in this picture!",
                    mb->mbAddr, sps->PicWidthInMbs * sps->FrameHeightInMbs);
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param sub_mb_type The sub-macroblock type.
 * \return The number of partitions used in this sub-macroblock.
 *
 * Give the number of sub-macroblock partitions used.
 * Table 7-17: Sub-macroblock types in P macroblocks.
 * Table 7-18: Sub-macroblock types in B macroblocks.
 */
static int NumSubMbPart(const unsigned int slice_type, const unsigned int sub_mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "NumSubMbPart()" CLR_RESET);
    int retcode = 0;

    if (slice_type == 0 || slice_type == 5) // P slice
    {
        switch (sub_mb_type)
        {
        case P_L0_8x8:
            retcode = 1;
            break;
        case P_L0_8x4:
            retcode = 2;
            break;
        case P_L0_4x8:
            retcode = 2;
            break;
        case P_L0_4x4:
            retcode = 4;
            break;
        }
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        switch (sub_mb_type)
        {
        case B_Direct_8x8:
            retcode = 4;
            break;
        case B_L0_8x8:
            retcode = 1;
            break;
        case B_L1_8x8:
            retcode = 1;
            break;
        case B_Bi_8x8:
            retcode = 1;
            break;
        case B_L0_8x4:
            retcode = 2;
            break;
        case B_L0_4x8:
            retcode = 2;
            break;
        case B_L1_8x4:
            retcode = 2;
            break;
        case B_L1_4x8:
            retcode = 2;
            break;
        case B_Bi_8x4:
            retcode = 2;
            break;
        case B_Bi_4x8:
            retcode = 2;
            break;
        case B_L0_4x4:
            retcode = 4;
            break;
        case B_L1_4x4:
            retcode = 4;
            break;
        case B_Bi_4x4:
            retcode = 4;
            break;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param sub_mb_type The sub-macroblock type.
 * \return SubMbPredMode Prediction mode for the-sub macroblock.
 *
 * From 'ITU-T H.264' recommendation:
 * 7.4.5.2 Sub-macroblock prediction semantics
 *
 * Table 7-17: Sub-macroblock types in P macroblocks.
 * Table 7-18: Sub-macroblock types in B macroblocks.
 */
static int SubMbPredMode(const unsigned int slice_type, const unsigned int sub_mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "SubMbPredMode()" CLR_RESET);
    int retcode = 0;

    if (slice_type == 0 || slice_type == 5) // P slice
    {
        retcode = Pred_L0;
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        if (sub_mb_type == B_Direct_8x8)
        {
            retcode = Direct;
        }
        else if (sub_mb_type == B_L0_8x8 || sub_mb_type == B_L0_8x4 ||
                 sub_mb_type == B_L0_4x8 || sub_mb_type == B_L0_4x4)
        {
            retcode = Pred_L0;
        }
        else if (sub_mb_type == B_L1_8x8 || sub_mb_type == B_L1_8x4 ||
                 sub_mb_type == B_L1_4x8 || sub_mb_type == B_L1_4x4)
        {
            retcode = Pred_L1;
        }
        else if (sub_mb_type == B_Bi_8x8 || sub_mb_type == B_Bi_8x4 ||
                 sub_mb_type == B_Bi_4x8 || sub_mb_type == B_Bi_4x4)
        {
            retcode = BiPred;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param sub_mb_type docme.
 *
 * Table 7-17: Sub-macroblock types in P macroblocks.
 * Table 7-18: Sub-macroblock types in B macroblocks.
 */
static int SubMbPartWidth(const unsigned int slice_type, const unsigned int sub_mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "SubMbPartWidth()" CLR_RESET);
    int retcode = 0;

    if (slice_type == 0 || slice_type == 5) // P slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED SubMbPartWidth(P slice)");
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED SubMbPartWidth(B slice)");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param slice_type The slice type.
 * \param sub_mb_type docme.
 *
 * Table 7-17: Sub-macroblock types in P macroblocks.
 * Table 7-18: Sub-macroblock types in B macroblocks.
 */
static int SubMbPartHeight(const unsigned int slice_type, const unsigned int sub_mb_type)
{
    TRACE_INFO(MB, "  > " BLD_GREEN "SubMbPartHeight()" CLR_RESET);
    int retcode = 0;

    if (slice_type == 0 || slice_type == 5) // P slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED SubMbPartHeight(P slice)");
    }
    else if (slice_type == 1 || slice_type == 6) // B slice
    {
        TRACE_WARNING(MB, ">>> UNIMPLEMENTED SubMbPartHeight(B slice)");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param startIdx docme.
 * \param endIdx docme.
 *
 * 7.4.5.3.1 Residual luma data semantics.
 * Output of this syntax structure are the variables i16x16DClevel, i16x16AClevel, level, and level8x8.
 *
 * This function decode the luma coefficients contained in a macroblock. It can
 * handle 16x16 block (composed of one 4x4 DC block and 16 4x4 AC blocks)
 * 4x4 blocks (16 4x4 blocks) and 8x8 blocks (4 8x8 blocks).
 */
static void residual_luma(DecodingContext_t *dc, const int startIdx, const int endIdx)
{
    TRACE_INFO(MB, "<> " BLD_GREEN "residual_luma()" CLR_RESET);

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    //
    if (startIdx == 0 && mb->MbPartPredMode[0] == Intra_16x16)
    {
        TRACE_2(MB, "---- residual_luma 4x4 DC (mb %i - blk 0) ---------------- START - 16x16 DC", mb->mbAddr);

        if (dc->entropy_coding_mode_flag)
            residual_block_cabac(dc, mb->Intra16x16DCLevel, 0, 15, 16, blk_LUMA_16x16_DC, 0);
        else
            residual_block_cavlc(dc, mb->Intra16x16DCLevel, 0, 15, 16, blk_LUMA_16x16_DC, 0);
    }

    int blkIdx = 0;
    int i8x8 = 0;
    for (i8x8 = 0; i8x8 < 4; i8x8++)
    {
        if (mb->transform_size_8x8_flag == false || dc->entropy_coding_mode_flag == false)
        {
            int i4x4 = 0;
            for (i4x4 = 0; i4x4 < 4; i4x4++)
            {
                blkIdx = i8x8*4 + i4x4;

                if (mb->CodedBlockPatternLuma & (1 << i8x8))
                {
                    if (mb->MbPartPredMode[0] == Intra_16x16)
                    {
                        TRACE_2(MB, "---- residual_luma 16x16 AC (mb %i - blk %i/15) ---------------- START - 16x16 AC", mb->mbAddr, blkIdx);

                        if (dc->entropy_coding_mode_flag)
                            residual_block_cabac(dc, mb->Intra16x16ACLevel[blkIdx], std::max(0, startIdx - 1), endIdx - 1, 15, blk_LUMA_16x16_AC, blkIdx);
                        else
                            residual_block_cavlc(dc, mb->Intra16x16ACLevel[blkIdx], std::max(0, startIdx - 1), endIdx - 1, 15, blk_LUMA_16x16_AC, blkIdx);
                    }
                    else
                    {
                        TRACE_2(MB, "---- residual_luma 4x4 (mb %i - blk %i/15) ---------------- START", mb->mbAddr, blkIdx);

                        if (dc->entropy_coding_mode_flag)
                            residual_block_cabac(dc, mb->LumaLevel4x4[blkIdx], startIdx, endIdx, 16, blk_LUMA_4x4, blkIdx);
                        else
                            residual_block_cavlc(dc, mb->LumaLevel4x4[blkIdx], startIdx, endIdx, 16, blk_LUMA_4x4, blkIdx);
                    }
                }
                else if (mb->MbPartPredMode[0] == Intra_16x16)
                {
                    TRACE_2(MB, "---- residual_luma 16x16 AC (mb %i - blk %i/15) ---------------- EMPTY - no 16x16 AC coeff", mb->mbAddr, blkIdx);

                    int i = 0;
                    for (i = 0; i < 15; i++)
                    {
                        mb->Intra16x16ACLevel[blkIdx][i] = 0;
                        mb->TotalCoeffs_luma[blkIdx] = 0;
                    }
                }
                else
                {
                    TRACE_2(MB, "---- residual_luma 4x4 (mb %i - blk %i/15) ---------------- EMPTY", mb->mbAddr, blkIdx);

                    int i = 0;
                    for (i = 0; i < 16; i++)
                    {
                        mb->LumaLevel4x4[blkIdx][i] = 0;
                        mb->TotalCoeffs_luma[blkIdx] = 0;
                    }
                }

                if (!dc->entropy_coding_mode_flag && mb->transform_size_8x8_flag)
                {
                    TRACE_2(MB, "---- residual_luma 8x8 from 4x4 (mb %i) ----------------", mb->mbAddr, blkIdx);

                    int i = 0;
                    for (i = 0; i < 16; i++)
                    {
                        mb->LumaLevel8x8[i8x8][4 * i + i4x4] = mb->LumaLevel4x4[blkIdx][i];
                    }
                }
            }
        }
        else if (mb->CodedBlockPatternLuma & (1 << i8x8))
        {
            TRACE_2(MB, "---- residual_luma 8x8 (mb %i - blk %i/3) ---------------- START", mb->mbAddr, i8x8);

            if (dc->entropy_coding_mode_flag)
                residual_block_cabac(dc, mb->LumaLevel8x8[i8x8], 4 * startIdx, 4 * endIdx + 3, 64, blk_LUMA_8x8, i8x8);
            else
                residual_block_cavlc(dc, mb->LumaLevel8x8[i8x8], 4 * startIdx, 4 * endIdx + 3, 64, blk_LUMA_8x8, i8x8);
        }
        else
        {
            TRACE_2(MB, "---- residual_luma 8x8 (mb %i - blk %i/3) ---------------- EMPTY", mb->mbAddr, i8x8);

            int i = 0;
            for (i = 0; i < 64; i++)
            {
                mb->LumaLevel8x8[i8x8][i] = 0;
            }
        }
    }

    TRACE_3(MB, "---- residual_luma - the end");
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param startIdx docme.
 * \param endIdx docme.
 *
 * Decode the chroma coefficients of the macroblock. There can only be 4x4 block size.
 * Start by decoding DC coefficients for Cr block, then Cb block.
 * Then decode 4 AC blocks of Cb and 4 AC blocks of Cr.
 */
static void residual_chroma(DecodingContext_t *dc, const int startIdx, const int endIdx)
{
    TRACE_INFO(MB, "<> " BLD_GREEN "residual_chroma()" CLR_RESET);

    // Shortcuts
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];

    //
    if (dc->ChromaArrayType == 1 || dc->ChromaArrayType == 2)
    {
        int iCbCr = 0;
        int NumC8x8 = 4 / (sps->SubWidthC * sps->SubHeightC);

        for (iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            if ((mb->CodedBlockPatternChroma & 3) && (startIdx == 0))
            {
                TRACE_2(MB, "---- residual_chroma 4x4 DC (mb %i - iCbCr %i - blk 0) ---------------- START", mb->mbAddr, iCbCr);

                if (dc->entropy_coding_mode_flag)
                    residual_block_cabac(dc, mb->ChromaDCLevel[iCbCr], 0, 4 * NumC8x8 - 1, 4 * NumC8x8, blk_CHROMA_DC_Cb + iCbCr, 0);
                else
                    residual_block_cavlc(dc, mb->ChromaDCLevel[iCbCr], 0, 4 * NumC8x8 - 1, 4 * NumC8x8, blk_CHROMA_DC_Cb + iCbCr, 0);
            }
            else
            {
                TRACE_2(MB, "---- residual_chroma 4x4 DC (mb %i - iCbCr %i - blk 0) ---------------- EMPTY", mb->mbAddr, iCbCr);

                int i = 0;
                for (i = 0; i < (4 * NumC8x8); i++)
                {
                    mb->ChromaDCLevel[iCbCr][i] = 0;
                }
            }
        }

        for (iCbCr = 0; iCbCr < 2; iCbCr++)
        {
            int blkIdx = 0;
            int i8x8 = 0;
            for (i8x8 = 0; i8x8 < NumC8x8; i8x8++)
            {
                int i4x4 = 0;
                for (i4x4 = 0; i4x4 < 4; i4x4++)
                {
                    blkIdx = i8x8*4 + i4x4;

                    if (mb->CodedBlockPatternChroma & 2)
                    {
                        TRACE_2(MB, "---- residual_chroma 4x4 AC (mb %i - iCbCr %i - blk %i/3) ---------------- START", mb->mbAddr, iCbCr, blkIdx);

                        if (dc->entropy_coding_mode_flag)
                            residual_block_cabac(dc, mb->ChromaACLevel[iCbCr][blkIdx], std::max(0, startIdx - 1), endIdx - 1, 15, blk_CHROMA_AC_Cb + iCbCr, blkIdx);
                        else
                            residual_block_cavlc(dc, mb->ChromaACLevel[iCbCr][blkIdx], std::max(0, startIdx - 1), endIdx - 1, 15, blk_CHROMA_AC_Cb + iCbCr, blkIdx);
                    }
                    else
                    {
                        TRACE_2(MB, "---- residual_chroma 4x4 AC (mb %i - iCbCr %i - blk %i/3) ---------------- EMPTY", mb->mbAddr, iCbCr, blkIdx);

                        int i = 0;
                        for (i = 0; i < 15; i++)
                        {
                            mb->ChromaACLevel[iCbCr][blkIdx][i] = 0;
                        }
                    }
                }
            }
        }
    }

    TRACE_3(MB, "---- residual_chroma - the end");
}

/* ************************************************************************** */
