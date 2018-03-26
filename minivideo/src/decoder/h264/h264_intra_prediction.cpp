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
 * \file      h264_intra_prediction.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_intra_prediction.h"
#include "h264_transform.h"
#include "h264_spatial.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "../../minitraces.h"

// C++ standard libraries
#include <algorithm>

/* ************************************************************************** */

// 4x4
static int Intra_4x4_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb);
    static void Intra_4x4_deriv_PredMode(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma4x4BlkIdx);
    static int Intra_4x4_pred_sample(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma4x4BlkIdx);
static int Intra_4x4_Vertical(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Horizontal(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_DC(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Diagonal_Down_Left(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Diagonal_Down_Right(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Vertical_Right(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Horizontal_Down(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Vertical_Left(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);
static int Intra_4x4_Horizontal_Up(uint8_t pred4x4L[4][4], intrapred4x4_t *ip);

// 8x8
static int Intra_8x8_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb);
    static void Intra_8x8_deriv_PredMode(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma8x8BlkIdx);
    static int Intra_8x8_pred_sample(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma8x8BlkIdx);
    static void Intra_8x8_sample_filtering(intrapred8x8_t *ip, intrapred8x8_t *ipprime);
static int Intra_8x8_Vertical(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Horizontal(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_DC(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Diagonal_Down_Left(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Diagonal_Down_Right(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Vertical_Right(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Horizontal_Down(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Vertical_Left(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);
static int Intra_8x8_Horizontal_Up(uint8_t pred8x8L[8][8], intrapred8x8_t *ip);

// 16x16
static int Intra_16x16_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb);
static int Intra_16x16_Vertical(uint8_t predL[16][16], intrapred16x16_t *ip);
static int Intra_16x16_Horizontal(uint8_t predL[16][16], intrapred16x16_t *ip);
static int Intra_16x16_DC(uint8_t predL[16][16], intrapred16x16_t *ip);
static int Intra_16x16_Plane(uint8_t predL[16][16], intrapred16x16_t *ip);

// Chroma
static int Intra_Chroma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb);
static int Intra_Chroma_DC(uint8_t predC[8][8], intrapredChroma_t *ip);
static int Intra_Chroma_Horizontal(uint8_t predC[8][8], intrapredChroma_t *ip);
static int Intra_Chroma_Vertical(uint8_t predC[8][8], intrapredChroma_t *ip);
static int Intra_Chroma_Plane(uint8_t predC[8][8], intrapredChroma_t *ip);

// I_PCM
static int ipcm_construction_process(DecodingContext_t *dc, Macroblock_t *mb);

/* ************************************************************************** */

/*!
 * \brief Intra prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3 Intra prediction process.
 *
 * Inputs to this process are constructed samples prior to the deblocking filter
 * process and, for Intra_NxN prediction modes (where NxN is equal to 4x4 or 8x8),
 * the values of IntraNxNPredMode from neighbouring macroblocks.
 *
 * The transformations (idct and quantization) must be done block by block inside
 * the intra prediction process, because the results (transformed blocks) are
 * directly used by the intra prediction process.
 *
 * Outputs of this process are specified as follows:
 * If the macroblock prediction mode is Intra_4x4 or Intra_8x8, the outputs are
 *   constructed luma samples prior to the deblocking filter process and (when
 *   ChromaArrayType is not equal to 0) chroma prediction samples of the macroblock
 *   predC, where C is equal to Cb and Cr.
 * Otherwise, if mb_type is not equal to I_PCM, the outputs are luma prediction
 *   samples of the macroblock predL and (when ChromaArrayType is not equal to 0)
 *   chroma prediction samples of the macroblock predC, where C is equal to Cb and Cr.
 * Otherwise (mb_type is equal to I_PCM), the outputs are constructed luma and
 *   (when ChromaArrayType is not equal to 0) chroma samples prior to the deblocking
 *   filter process.
 */
int intra_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_INFO(INTRA, "<> " BLD_GREEN "intra_prediction_process()" CLR_RESET);
    int retcode = FAILURE;

    if (mb->mb_type == I_PCM)
    {
        ipcm_construction_process(dc, mb);
    }
    else
    {
        // Luma
        if (mb->MbPartPredMode[0] == Intra_4x4)
        {
            retcode = Intra_4x4_luma_prediction_process(dc, mb);
        }
        else if (mb->MbPartPredMode[0] == Intra_8x8)
        {
            retcode = Intra_8x8_luma_prediction_process(dc, mb);
        }
        else if (mb->MbPartPredMode[0] == Intra_16x16)
        {
            retcode = Intra_16x16_luma_prediction_process(dc, mb);
        }

        // Chroma
        if (dc->ChromaArrayType != 0)
        {
            retcode = Intra_Chroma_prediction_process(dc, mb);
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Intra_4x4 prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1 Intra_4x4 prediction process for luma samples.
 *
 * This process is invoked when the macroblock prediction mode is equal to Intra_4x4.
 */
static int Intra_4x4_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_4x4_luma_prediction_process()" CLR_RESET);
    int retcode = SUCCESS;
    int luma4x4BlkIdx = 0;

    for (luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
    {
        Intra_4x4_deriv_PredMode(dc, mb, luma4x4BlkIdx);

        Intra_4x4_pred_sample(dc, mb, luma4x4BlkIdx);

        transform4x4_luma(dc, mb, luma4x4BlkIdx);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for Intra4x4PredMode.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma4x4BlkIdx The index of a 4x4 luma block luma4x4BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.1 Derivation process for Intra4x4PredMode.
 *
 * Inputs to this process are the index of the 4x4 luma block luma4x4BlkIdx and
 * variable arrays Intra4x4PredMode (if available) and Intra8x8PredMode (if available)
 * that are previously (in decoding order) derived for adjacent macroblocks.
 *
 * Output of this process is the variable Intra4x4PredMode[luma4x4BlkIdx].
 */
static void Intra_4x4_deriv_PredMode(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma4x4BlkIdx)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_4x4_deriv_PredMode()" CLR_RESET);
/*
    // Table 8-2: Specification of Intra4x4PredMode[luma4x4BlkIdx] and associated names
    0   Intra_4x4_Vertical
    1   Intra_4x4_Horizontal
    2   Intra_4x4_DC
    3   Intra_4x4_Diagonal_Down_Left
    4   Intra_4x4_Diagonal_Down_Right
    5   Intra_4x4_Vertical_Right
    6   Intra_4x4_Horizontal_Down
    7   Intra_4x4_Vertical_Left
    8   Intra_4x4_Horizontal_Up
*/
    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];

    // Initialization
    int mbAddrA_temp = -1, mbAddrB_temp = -1;
    int blkA = 0, blkB = 0;

    deriv_4x4lumablocks(dc, luma4x4BlkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);

    TRACE_3(INTRA, "  > blkA : %i", blkA);
    TRACE_3(INTRA, "  > blkB : %i", blkB);
    TRACE_3(INTRA, "  > mbAddrA_temp : %i", mbAddrA_temp);
    TRACE_3(INTRA, "  > mbAddrB_temp : %i", mbAddrB_temp);

    // 2
    bool dcPredModePredictedFlag = false;
    if (mbAddrA_temp == -1 || mbAddrB_temp == -1)
        dcPredModePredictedFlag = true;
    else
    {
        if (dc->mb_array[mbAddrA_temp] != NULL &&
            dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] > 3 &&
            pps->constrained_intra_pred_flag == true)
            dcPredModePredictedFlag = true;

        if (dc->mb_array[mbAddrB_temp] != NULL &&
            dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] > 3 &&
            pps->constrained_intra_pred_flag == true)
            dcPredModePredictedFlag = true;
    }
    TRACE_3(INTRA, "  > dcPredModePredictedFlag: %i", dcPredModePredictedFlag);

    // 3
    unsigned intraMxMPredModeA = 2;
    unsigned intraMxMPredModeB = 2;

    if (mbAddrA_temp != -1)
    {
        if (dcPredModePredictedFlag == false)
        {
            if (dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] == Intra_4x4)
                intraMxMPredModeA = dc->mb_array[mbAddrA_temp]->Intra4x4PredMode[blkA];
            else if (dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] == Intra_8x8)
                intraMxMPredModeA = dc->mb_array[mbAddrA_temp]->Intra8x8PredMode[blkA >> 2];

            TRACE_3(INTRA, "  > intraMxMPredModeA: %u", intraMxMPredModeA);
        }
    }

    if (mbAddrB_temp != -1)
    {
        if (dcPredModePredictedFlag == false)
        {
            if (dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] == Intra_4x4)
                intraMxMPredModeB = dc->mb_array[mbAddrB_temp]->Intra4x4PredMode[blkB];
            else if (dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] == Intra_8x8)
                intraMxMPredModeB = dc->mb_array[mbAddrB_temp]->Intra8x8PredMode[blkB >> 2];

            TRACE_3(INTRA, "  > intraMxMPredModeB: %u", intraMxMPredModeB);
        }
    }

    // 4
    unsigned predIntra4x4PredMode = std::min(intraMxMPredModeA, intraMxMPredModeB);
    TRACE_3(INTRA, "  > predIntra4x4PredMode: %u", predIntra4x4PredMode);

    if (mb->prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] == true)
    {
        mb->Intra4x4PredMode[luma4x4BlkIdx] = predIntra4x4PredMode;
    }
    else
    {
        if (mb->rem_intra4x4_pred_mode[luma4x4BlkIdx] < predIntra4x4PredMode)
            mb->Intra4x4PredMode[luma4x4BlkIdx] = mb->rem_intra4x4_pred_mode[luma4x4BlkIdx];
        else
            mb->Intra4x4PredMode[luma4x4BlkIdx] = mb->rem_intra4x4_pred_mode[luma4x4BlkIdx] + 1;
    }

    TRACE_3(INTRA, "  > Intra4x4PredMode: %u", mb->Intra4x4PredMode[luma4x4BlkIdx]);
}

/* ************************************************************************** */

/*!
 * \brief Intra_4x4 sample prediction.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma4x4BlkIdx The index of a 4x4 luma block luma4x4BlkIdx.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2 Intra_4x4 sample prediction.
 *
 * This process is invoked for each 4x4 luma block of a macroblock with macroblock
 * prediction mode equal to Intra_4x4 followed by the transform decoding process
 * and picture construction process prior to deblocking for each 4x4 luma block.
 *
 * Inputs to this process are the index of a 4x4 luma block luma4x4BlkIdx,
 * an (PicWidthInSamplesL)x(PicHeightInSamplesL) array cSL containing constructed
 * luma samples prior to the deblocking filter process of neighbouring macroblocks.
 *
 * Output of this process are the prediction samples pred4x4L[x, y], with x, y = 0..3,
 * for the 4x4 luma block with index luma4x4BlkIdx.
 */
static int Intra_4x4_pred_sample(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma4x4BlkIdx)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_4x4_pred_sample()" CLR_RESET);
    TRACE_1(INTRA, "  > blkIdx %i", luma4x4BlkIdx);

    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];

    // Initialization
    int retcode = FAILURE;
    int mbAddrN;
    int xW, yW;
    int x, y;

    intrapred4x4_t ip;
    ip.blkIdx = luma4x4BlkIdx;
    ip.BitDepthY = dc->sps_array[pps->seq_parameter_set_id]->BitDepthY;
    ip.sample_left = false;
    ip.sample_up_left = false;
    ip.sample_up = false;
    ip.sample_up_right = false;

    int xO = -1, yO = -1;
    InverseLuma4x4BlkScan(luma4x4BlkIdx, &xO, &yO);

    // Up Left corner samples
    x = -1, y = -1;
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample phv[-1,-1] is NOT available for Intra_4x4 prediction (mb=%i)  (xW=%i, yW=%i)", mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_up_left = true;
            ip.pv[0] = ip.ph[0] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample phv[-1,-1] = %u      (mb=%i) (xW=%i, yW=%i)", ip.pv[0], mbAddrN, xW, yW);
        }
    }

    // Vertical samples
    x = -1, y = 0;
    for (y = 0; y < 4; y++)
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample pv[-1,%i] is NOT available for Intra_4x4 prediction (mb=%i)  (xW=%i, yW=%i)", y, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_left = true;
            ip.pv[y +1] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample pv[-1,%i] = %u       (mb=%i) (xW=%i, yW=%i)", y, ip.pv[y +1], mbAddrN, xW, yW);
        }
    }

    // Horizontal samples
    x = 0, y = -1;
    for (x = 0; x < 8; x++)
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true) ||
            (x > 3 && (luma4x4BlkIdx == 3 || luma4x4BlkIdx == 11)))
        {
            TRACE_2(INTRA, "  > sample ph[%i,-1] is NOT available for Intra_4x4 prediction (mb=%i) (xW=%i, yW=%i)", x, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            if (x < 4)
                ip.sample_up = true;
            else
                ip.sample_up_right = true;

            ip.ph[x +1] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample ph[%i,-1] = %i       (mb=%i) (xW=%i, yW=%i)", x, ip.ph[x +1], mbAddrN, xW, yW);
        }
    }

    if (ip.sample_up && !ip.sample_up_right)
    {
        for (x = 4; x < 8; x++)
        {
            ip.ph[x +1] = ip.ph[3 +1];
        }

        ip.sample_up_right = true;
    }

    // Predictions
    uint8_t pred4x4L[4][4] = {{0}};
    switch (mb->Intra4x4PredMode[luma4x4BlkIdx])
    {
        case 0:
            Intra_4x4_Vertical(pred4x4L, &ip);
            break;
        case 1:
            Intra_4x4_Horizontal(pred4x4L, &ip);
            break;
        case 2:
            Intra_4x4_DC(pred4x4L, &ip);
            break;
        case 3:
            Intra_4x4_Diagonal_Down_Left(pred4x4L, &ip);
            break;
        case 4:
            Intra_4x4_Diagonal_Down_Right(pred4x4L, &ip);
            break;
        case 5:
            Intra_4x4_Vertical_Right(pred4x4L, &ip);
            break;
        case 6:
            Intra_4x4_Horizontal_Down(pred4x4L, &ip);
            break;
        case 7:
            Intra_4x4_Vertical_Left(pred4x4L, &ip);
            break;
        case 8:
            Intra_4x4_Horizontal_Up(pred4x4L, &ip);
            break;
        default:
            TRACE_ERROR(INTRA, "Unable to understand 4x4 prediction mode!");
            break;
    }

    // Copy into prediction sample array
    for (x = 0; x < 4; x++)
        for (y = 0; y < 4; y++)
            mb->predL[xO + x][yO + y] = pred4x4L[x][y];

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 vertical prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.1 Specification of Intra_4x4_Vertical prediction mode.
 */
static int Intra_4x4_Vertical(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Vertical()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..3 are marked
    as "available for Intra_4x4 prediction".
*/
    if (ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = ip->ph[x +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Vertical prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 horizontal prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.2 Specification of Intra_4x4_Horizontal prediction mode.
 */
static int Intra_4x4_Horizontal(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Horizontal()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ -1, y ], with y = 0..3, are
    marked as "available for Intra_4x4 prediction".
*/
    if (ip->sample_left)
    {
        int x = 0, y = 0;
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = ip->pv[y +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Horizontal prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 DC prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.3 Specification of Intra_4x4_DC prediction mode.
 *
 * Note: A 4x4 luma block can always be predicted using this mode.
 */
static int Intra_4x4_DC(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_DC()" CLR_RESET);
    int retcode = SUCCESS;

    int x = 0, y = 0;
    int sumH = ip->ph[0 +1] + ip->ph[1 +1] + ip->ph[2 +1] + ip->ph[3 +1];
    int sumV = ip->pv[0 +1] + ip->pv[1 +1] + ip->pv[2 +1] + ip->pv[3 +1];
/*
    If all samples p[ x, -1 ], with x = 0..3, and p[ -1, y ], with y = 0..3, are marked as "available for Intra_4x4
    prediction", the values of the prediction samples pred4x4L[ x, y ], with x, y = 0..3, are derived by
    pred4x4L[ x, y ] = ( p[ 0, -1 ] + p[ 1, -1 ] + p[ 2, -1 ] + p[ 3, -1 ] + p[ -1, 0 ] + p[ -1, 1 ] + p[ -1, 2 ] + p[ -1, 3 ] + 4 ) >> 3
*/
    if (ip->sample_left &&
        ip->sample_up)
    {
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = (sumH + sumV + 4) >> 3;
    }
/*
    Otherwise, if any samples p[ x, -1 ], with x = 0..3, are marked as "not available for Intra_4x4 prediction" and all
    samples p[ -1, y ], with y = 0..3, are marked as "available for Intra_4x4 prediction", the values of the prediction
    samples pred4x4L[ x, y ], with x, y = 0..3, are derived by
    pred4x4L[ x, y ] = ( p[ -1, 0 ] + p[ -1, 1 ] + p[ -1, 2 ] + p[ -1, 3 ] + 2 ) >> 2
*/
    else if (!ip->sample_up &&
             ip->sample_left)
    {
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = (sumV + 2) >> 2;
    }
/*
    Otherwise, if any samples p[ -1, y ], with y = 0..3, are marked as "not available for Intra_4x4 prediction" and all
    samples p[ x, -1 ], with x = 0 .. 3, are marked as "available for Intra_4x4 prediction", the values of the prediction
    samples pred4x4L[ x, y ], with x, y = 0 .. 3, are derived by
    pred4x4L[ x, y ] = ( p[ 0, -1 ] + p[ 1, -1 ] + p[ 2, -1 ] + p[ 3, -1 ] + 2 ) >> 2
*/
    else if (ip->sample_up &&
             !ip->sample_left)
    {
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = (sumH + 2) >> 2;
    }
/*
    Otherwise (some samples p[ x, -1 ], with x = 0..3, and some samples p[ -1, y ], with y = 0..3, are marked as "not
    available for Intra_4x4 prediction"), the values of the prediction samples pred4x4L[ x, y ], with x, y = 0..3, are
    derived by
    pred4x4L[ x, y ] = ( 1 << ( BitDepthY - 1 ) )
*/
    else if (!ip->sample_left &&
             !ip->sample_up)
    {
        for (x = 0; x < 4; x++)
            for (y = 0; y < 4; y++)
                pred4x4L[x][y] = (1 << (ip->BitDepthY - 1));
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_DC prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Diagonal Down Left prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.4 Specification of Intra_4x4_Diagonal_Down_Left prediction mode.
 */
static int Intra_4x4_Diagonal_Down_Left(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Diagonal_Down_Left()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 are marked
    as "available for Intra_4x4 prediction".
*/
    if (ip->sample_up &&
        ip->sample_up_right)
    {
        int x = 0, y = 0;
        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                if (x == 3 && y == 3)
                    pred4x4L[x][y] = (ip->ph[6 +1] + 3 * ip->ph[7 +1] + 2) >> 2;
                else
                    pred4x4L[x][y] = (ip->ph[(x + y) +1] + 2 * ip->ph[(x + y + 1) +1] + ip->ph[(x + y + 2) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Diagonal_Down_Left prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Diagonal Down Right prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.5 Specification of Intra_4x4_Diagonal_Down_Right prediction mode.
 */
static int Intra_4x4_Diagonal_Down_Right(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Diagonal_Down_Right()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..3 and p[ -1, y ]
    with y = -1..3 are marked as "available for Intra_4x4 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                if (x > y)
                    pred4x4L[x][y] = (ip->ph[(x - y - 2) +1] + 2 * ip->ph[(x - y - 1) +1] + ip->ph[(x - y) +1] + 2) >> 2;
                else if (x < y)
                    pred4x4L[x][y] = (ip->pv[(y - x - 2) +1] + 2 * ip->pv[(y - x - 1) +1] + ip->pv[(y - x) +1] + 2) >> 2;
                else
                    pred4x4L[x][y] = (ip->ph[0 +1] + 2 * ip->pv[0] + ip->pv[0 +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Diagonal_Down_Right prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Vertical Right prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.6 Specification of Intra_4x4_Vertical_Right prediction mode.
 */
static int Intra_4x4_Vertical_Right(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Vertical_Right()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..3 and
    p[ -1, y ] with y = -1..3 are marked as "available for Intra_4x4 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int zVR = 0;
        int x = 0, y = 0;

        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                zVR = 2 * x - y;

                if (zVR > -1)
                {
                    if (zVR % 2 == 0)
                        pred4x4L[x][y] = (ip->ph[(x - (y >> 1) - 1) +1] + ip->ph[(x - (y >> 1)) +1] + 1) >> 1;
                    else
                        pred4x4L[x][y] = (ip->ph[(x - (y >> 1) - 2) +1] + 2 * ip->ph[(x - (y >> 1) - 1) +1] + ip->ph[(x - (y >> 1)) +1] + 2) >> 2;
                }
                else if (zVR == -1)
                    pred4x4L[x][y] = (ip->pv[0 +1] + 2 * ip->pv[0] + ip->ph[0 +1] + 2) >> 2;
                else
                    pred4x4L[x][y] = (ip->pv[(y - 1) +1] + 2 * ip->pv[(y - 2) +1] + ip->pv[(y - 3) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Vertical_Right prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Horizontal Down prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.7 Specification of Intra_4x4_Horizontal_Down prediction mode.
 */
static int Intra_4x4_Horizontal_Down(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Horizontal_Down()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..3 and
    p[ -1, y ] with y = -1..3 are marked as "available for Intra_4x4 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int zHD = 0;
        int x = 0, y = 0;

        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                zHD = 2 * y - x;

                if (zHD > -1)
                {
                    if (zHD % 2 == 0)
                        pred4x4L[x][y] = (ip->pv[(y - (x >> 1) - 1) +1] + ip->pv[(y - (x >> 1)) +1] + 1) >> 1;
                    else
                        pred4x4L[x][y] = (ip->pv[(y - (x >> 1) - 2) +1] + 2 * ip->pv[(y - (x >> 1) - 1) +1] + ip->pv[(y - (x >> 1)) +1] + 2) >> 2;
                }
                else if (zHD == -1)
                    pred4x4L[x][y] = (ip->pv[0 +1] + 2 * ip->pv[0] + ip->ph[0 +1] + 2) >> 2;
                else
                    pred4x4L[x][y] = (ip->ph[(x - 1) +1] + 2 * ip->ph[(x - 2) +1] + ip->ph[(x - 3) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Horizontal_Down prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Vertical Left prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.8 Specification of Intra_4x4_Vertical_Left prediction mode.
 */
static int Intra_4x4_Vertical_Left(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Vertical_Left()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 are marked
    as "available for Intra_4x4 prediction".
*/
    if (ip->sample_up &&
        ip->sample_up_right)
    {
        int x = 0, y = 0;
        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                if (y % 2 == 0)
                    pred4x4L[x][y] = (ip->ph[(x + (y >> 1)) +1] + ip->ph[(x + (y >> 1) + 1) +1] + 1) >> 1;
                else
                    pred4x4L[x][y] = (ip->ph[(x + (y >> 1)) +1] + 2 * ip->ph[(x + (y >> 1) + 1) +1] + ip->ph[(x + (y >> 1) + 2) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Vertical_Left prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 4x4 Horizontal Up prediction.
 * \param pred4x4L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.1.2.9 Specification of Intra_4x4_Horizontal_Up prediction mode.
 */
static int Intra_4x4_Horizontal_Up(uint8_t pred4x4L[4][4], intrapred4x4_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_4x4_Horizontal_Up()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ -1, y ] with y = 0..3 are
    marked as "available for Intra_4x4 prediction".
*/
    if (ip->sample_left)
    {
        int zHU = 0;
        int x = 0, y = 0;

        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                zHU = x + 2 * y;

                if (zHU < 5 && zHU % 2 == 0)
                    pred4x4L[x][y] = (ip->pv[(y + (x >> 1)) +1] + ip->pv[(y + (x >> 1) + 1) +1] + 1) >> 1;
                else if (zHU == 1 || zHU == 3)
                    pred4x4L[x][y] = (ip->pv[(y + (x >> 1)) +1] + 2 * ip->pv[(y + (x >> 1) + 1) +1] + ip->pv[(y + (x >> 1) + 2) +1] + 2) >> 2;
                else if (zHU == 5)
                    pred4x4L[x][y] = (ip->pv[2 +1] + 3 * ip->pv[3 +1] + 2) >> 2;
                else
                    pred4x4L[x][y] = ip->pv[3 +1];
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_4x4_Horizontal_Up prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Intra_8x8 prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2 Intra_8x8 prediction process for luma samples.
 *
 * This process is invoked when the macroblock prediction mode is equal to Intra_8x8.
 */
static int Intra_8x8_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_8x8_luma_prediction_process()" CLR_RESET);
    int retcode = SUCCESS;
    int luma8x8BlkIdx = 0;

    for (luma8x8BlkIdx = 0; luma8x8BlkIdx < 4; luma8x8BlkIdx++)
    {
        Intra_8x8_deriv_PredMode(dc, mb, luma8x8BlkIdx);

        Intra_8x8_pred_sample(dc, mb, luma8x8BlkIdx);

        transform8x8_luma(dc, mb, luma8x8BlkIdx);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for Intra8x8PredMode.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma8x8BlkIdx The index of a 8x8 luma block luma8x8BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.1 Derivation process for Intra8x8PredMode.
 *
 * Inputs to this process are the index of the 8x8 luma block luma8x8BlkIdx and
 * variable arrays Intra4x4PredMode (if available) and Intra8x8PredMode (if available)
 * that are previously (in decoding order) derived for adjacent macroblocks.
 *
 * Output of this process is the variable Intra8x8PredMode[luma8x8BlkIdx].
 */
static void Intra_8x8_deriv_PredMode(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma8x8BlkIdx)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_8x8_deriv_PredMode()" CLR_RESET);
/*
    // Table 8-3: Specification of Intra8x8PredMode[luma8x8BlkIdx] and associated names
    0   Intra_8x8_Vertical
    1   Intra_8x8_Horizontal
    2   Intra_8x8_DC
    3   Intra_8x8_Diagonal_Down_Left
    4   Intra_8x8_Diagonal_Down_Right
    5   Intra_8x8_Vertical_Right
    6   Intra_8x8_Horizontal_Down
    7   Intra_8x8_Vertical_Left
    8   Intra_8x8_Horizontal_Up
*/
    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];

    // Initialization
    int mbAddrA_temp = -1, mbAddrB_temp = -1;
    int blkA = 0, blkB = 0;

    deriv_8x8lumablocks(dc, luma8x8BlkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);

    TRACE_2(INTRA, "  > blkIdx : %u", luma8x8BlkIdx);
    TRACE_2(INTRA, "  > blkA   : %i", blkA);
    TRACE_2(INTRA, "  > blkB   : %i", blkB);
    TRACE_2(INTRA, "  > mbAddrA_temp : %i", mbAddrA_temp);
    TRACE_2(INTRA, "  > mbAddrB_temp : %i", mbAddrB_temp);

    // 2
    bool dcPredModePredictedFlag = false;
    if (mbAddrA_temp == -1 || mbAddrB_temp == -1)
        dcPredModePredictedFlag = true;
    else
    {
        if (dc->mb_array[mbAddrA_temp] != NULL &&
            dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] > 3 &&
            pps->constrained_intra_pred_flag == true)
            dcPredModePredictedFlag = true;

        if (dc->mb_array[mbAddrB_temp] != NULL &&
            dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] > 3 &&
            pps->constrained_intra_pred_flag == true)
            dcPredModePredictedFlag = true;
    }
    TRACE_3(INTRA, "  > dcPredModePredictedFlag: %i", dcPredModePredictedFlag);

    // 3
    unsigned intraMxMPredModeA = 2;
    unsigned intraMxMPredModeB = 2;

    if (mbAddrA_temp != -1)
    {
        if (dcPredModePredictedFlag == false)
        {
            if (dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] == Intra_8x8)
                intraMxMPredModeA = dc->mb_array[mbAddrA_temp]->Intra8x8PredMode[blkA];
            else if (dc->mb_array[mbAddrA_temp]->MbPartPredMode[0] == Intra_4x4)
            {
                int n = 1;

#if ENABLE_MBAFF
                // MbaffFrameFlag is equal to 1, the current macroblock is a frame coded macroblock,
                // the macroblock mbAddrN is a field coded macroblock.
                if (MbaffFrameFlag == 1 && luma8x8BlkIdx == 2)
                    n = 3;
#endif // ENABLE_MBAFF

                intraMxMPredModeA = dc->mb_array[mbAddrA_temp]->Intra4x4PredMode[blkA * 4 + n];
            }
        }
    }

    if (mbAddrB_temp != -1)
    {
        if (dcPredModePredictedFlag == false)
        {
            if (dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] == Intra_8x8)
                intraMxMPredModeB = dc->mb_array[mbAddrB_temp]->Intra8x8PredMode[blkB];
            else if (dc->mb_array[mbAddrB_temp]->MbPartPredMode[0] == Intra_4x4)
                intraMxMPredModeB = dc->mb_array[mbAddrB_temp]->Intra4x4PredMode[blkB * 4 + 2];
        }
    }

    TRACE_3(INTRA, "  > intraMxMPredModeA: %u", intraMxMPredModeA);
    TRACE_3(INTRA, "  > intraMxMPredModeB: %u", intraMxMPredModeB);

    // 4
    unsigned predIntra8x8PredMode = std::min(intraMxMPredModeA, intraMxMPredModeB);

    TRACE_3(INTRA, "  > predIntra8x8PredMode: %u", predIntra8x8PredMode);

    if (mb->prev_intra8x8_pred_mode_flag[luma8x8BlkIdx])
    {
        mb->Intra8x8PredMode[luma8x8BlkIdx] = predIntra8x8PredMode;
    }
    else
    {
        if (mb->rem_intra8x8_pred_mode[luma8x8BlkIdx] < predIntra8x8PredMode)
            mb->Intra8x8PredMode[luma8x8BlkIdx] = mb->rem_intra8x8_pred_mode[luma8x8BlkIdx];
        else
            mb->Intra8x8PredMode[luma8x8BlkIdx] = mb->rem_intra8x8_pred_mode[luma8x8BlkIdx] + 1;
    }

    TRACE_2(INTRA, "  > Intra8x8PredMode: %u", mb->Intra8x8PredMode[luma8x8BlkIdx]);
}

/* ************************************************************************** */

/*!
 * \brief Intra_8x8 sample prediction.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma8x8BlkIdx The index of a 8x8 luma block luma8x8BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2 Intra_8x8 sample prediction.
 *
 * This process is invoked for each 8x8 luma block of a macroblock with macroblock
 * prediction mode equal to Intra_8x8 followed by the transform decoding process
 * and picture construction process prior to deblocking for each 8x8 luma block.
 *
 * Inputs to this process are the index of a 8x8 luma block luma8x8BlkIdx,
 * an (PicWidthInSamplesL)x(PicHeightInSamplesL) array cSL containing constructed
 * luma samples prior to the deblocking filter process of neighbouring macroblocks.
 *
 * Output of this process are the prediction samples pred8x8L[x, y], with x, y = 0..3,
 * for the 8x8 luma block with index luma8x8BlkIdx.
 */
static int Intra_8x8_pred_sample(DecodingContext_t *dc, Macroblock_t *mb, const unsigned int luma8x8BlkIdx)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_8x8_pred_sample()" CLR_RESET);
    TRACE_1(INTRA, "  > blkIdx %i", luma8x8BlkIdx);

    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];

    // Initialization
    int retcode = FAILURE;
    int mbAddrN;
    int xW, yW;
    int x, y;

    intrapred8x8_t ip;
    ip.blkIdx = luma8x8BlkIdx;
    ip.BitDepthY = dc->sps_array[pps->seq_parameter_set_id]->BitDepthY;
    ip.sample_left = false;
    ip.sample_up_left = false;
    ip.sample_up = false;
    ip.sample_up_right = false;

    intrapred8x8_t ipprime;
    ipprime.blkIdx = luma8x8BlkIdx;
    ipprime.BitDepthY = dc->sps_array[pps->seq_parameter_set_id]->BitDepthY;
    ipprime.sample_left = false;
    ipprime.sample_up_left = false;
    ipprime.sample_up = false;
    ipprime.sample_up_right = false;

    int xO = -1, yO = -1;
    InverseLuma8x8BlkScan(luma8x8BlkIdx, &xO, &yO);

    // Up Left corner samples
    x = -1, y = -1;
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample phv[-1,-1] is NOT available for Intra_8x8 prediction (mb=%i) (xW=%i, yW=%i)", mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_up_left = true;
            ip.pv[0] = ip.ph[0] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample phv[-1,-1] = %i      (mb=%i) (xW=%i, yW=%i)", ip.pv[0], mbAddrN, xW, yW);
        }
    }

    // Vertical samples
    x = -1, y = 0;
    for (y = 0; y < 8; y++)
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample pv[-1,%i] is NOT available for Intra_8x8 prediction (mb=%i) (xW=%i, yW=%i)", y, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_left = true;
            ip.pv[y +1] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample pv[-1,%i] = %i      (mb=%i) (xW=%i, yW=%i)", y, ip.pv[y +1], mbAddrN, xW, yW);
        }
    }

    // Horizontal samples
    x = 0, y = -1;
    for (x = 0; x < 16; x++)
    {
        // 1
        int xN = xO + x;
        int yN = yO + y;

        // 2
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, xN, yN, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample ph[%i,-1] is NOT available for Intra_8x8 prediction (mb=%i) (xW=%i, yW=%i)", x, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            if (x < 8)
                ip.sample_up = true;
            else
                ip.sample_up_right = true;

            ip.ph[x +1] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample ph[%i,-1] = %i      (mb=%i) (xW=%i, yW=%i)", x, ip.ph[x +1], mbAddrN, xW, yW);
        }
    }

    if (ip.sample_up && !ip.sample_up_right)
    {
        for (x = 8; x < 16; x++)
            ip.ph[x +1] = ip.ph[7 +1];

        ip.sample_up_right = true;
    }

    // Filtering
    Intra_8x8_sample_filtering(&ip, &ipprime);

    // Predictions
    uint8_t pred8x8L[8][8] = {{0}};
    switch (mb->Intra8x8PredMode[luma8x8BlkIdx])
    {
        case 0:
            Intra_8x8_Vertical(pred8x8L, &ipprime);
            break;
        case 1:
            Intra_8x8_Horizontal(pred8x8L, &ipprime);
            break;
        case 2:
            Intra_8x8_DC(pred8x8L, &ipprime);
            break;
        case 3:
            Intra_8x8_Diagonal_Down_Left(pred8x8L, &ipprime);
            break;
        case 4:
            Intra_8x8_Diagonal_Down_Right(pred8x8L, &ipprime);
            break;
        case 5:
            Intra_8x8_Vertical_Right(pred8x8L, &ipprime);
            break;
        case 6:
            Intra_8x8_Horizontal_Down(pred8x8L, &ipprime);
            break;
        case 7:
            Intra_8x8_Vertical_Left(pred8x8L, &ipprime);
            break;
        case 8:
            Intra_8x8_Horizontal_Up(pred8x8L, &ipprime);
            break;
        default:
            TRACE_ERROR(INTRA, "Unable to understand 8x8 prediction mode!");
            break;
    }

    // Copy into prediction sample array
    for (x = 0; x < 8; x++)
        for (y = 0; y < 8; y++)
            mb->predL[xO + x][yO + y] = pred8x8L[x][y];

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra_8x8 sample filtering.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \param *ipprime A structure containing informations about filtered prediction samples and there availability.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.1 Reference sample filtering process for Intra_8x8 sample prediction.
 */
static void Intra_8x8_sample_filtering(intrapred8x8_t *ip, intrapred8x8_t *ipprime)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_8x8_sample_filtering()" CLR_RESET);

    // Parameters copy
    ipprime->sample_left = ip->sample_left;
    ipprime->sample_up = ip->sample_up;
    ipprime->sample_up_left = ip->sample_up_left;
    ipprime->sample_up_right = ip->sample_up_right;

    if (ip->sample_up &&
        ip->sample_up_right)
    {
        if (ip->sample_up_left)
            ipprime->ph[0 +1] = (ip->pv[0] + 2 * ip->ph[0 +1] + ip->ph[1 +1] + 2) >> 2;
        else
            ipprime->ph[0 +1] = (3 * ip->ph[0 +1] + ip->ph[1 +1] + 2) >> 2;

        int x = 1;
        for (x = 1; x < 15; x++)
            ipprime->ph[x +1] = (ip->ph[(x - 1) +1] + 2 * ip->ph[x +1] + ip->ph[(x + 1) +1] + 2) >> 2;

        ipprime->ph[15 +1] = (ip->ph[14 +1] + 3 * ip->ph[15 +1] + 2) >> 2;
    }

    if (ip->sample_up_left)
    {
        if (!ip->sample_up || !ip->sample_left)
        {
            if (ip->sample_up)
            {
                ipprime->pv[0] = ipprime->ph[0] = (3 * ip->pv[0] + ip->ph[0 +1] + 2) >> 2;

            }
            else if (!ip->sample_up && ip->sample_left)
            {
                ipprime->pv[0] = ipprime->ph[0] = (3 * ip->pv[0] + ip->pv[0 +1] + 2) >> 2;
            }
            else //(!ip->sample_up && !ip->sample_left)
                ipprime->pv[0] = ipprime->ph[0] = ip->pv[0];
        }
        else //if (ip->sample_up && ip->sample_left)
            ipprime->pv[0] = ipprime->ph[0] = (ip->ph[0 +1] + 2 * ip->pv[0] + ip->pv[0 +1] + 2) >> 2;
    }

    if (ip->sample_left)
    {
        if (ip->sample_up_left)
            ipprime->pv[0 +1] = (ip->pv[0] + 2 * ip->pv[0 +1] + ip->pv[1 +1] + 2) >> 2;
        else
            ipprime->pv[0 +1] = (3 * ip->pv[0 +1] + ip->pv[1 +1] + 2) >> 2;

        int y = 1;
        for (y = 1; y < 7; y++)
            ipprime->pv[y +1] = (ip->pv[(y -1) +1] + 2 * ip->pv[y +1] + ip->pv[(y + 1) +1] + 2) >> 2;

        ipprime->pv[7 +1] = (ip->pv[6 +1] + 3 * ip->pv[7 +1] + 2) >> 2;
    }
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Vertical prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.2 Specification of Intra_8x8_Vertical prediction mode.
 */
static int Intra_8x8_Vertical(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Vertical()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 are marked
    as "available for Intra_8x8 prediction".
*/
    if (ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = ip->ph[x +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Vertical prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Horizontal prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.3 Specification of Intra_8x8_Horizontal prediction mode.
 */
static int Intra_8x8_Horizontal(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Horizontal()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ -1, y ], with y = 0..7, are marked
    as "available for Intra_8x8 prediction".
*/
    if (ip->sample_left)
    {
        int x = 0, y = 0;
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = ip->pv[y +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Horizontal prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 DC prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.4 Specification of Intra_8x8_DC prediction mode.
 */
static int Intra_8x8_DC(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_DC()" CLR_RESET);
    int retcode = SUCCESS;

    int x = 0, y = 0;
    int sumH = ip->ph[0 +1] + ip->ph[1 +1] + ip->ph[2 +1] + ip->ph[3 +1] + ip->ph[4 +1] + ip->ph[5 +1] + ip->ph[6 +1] + ip->ph[7 +1];
    int sumV = ip->pv[0 +1] + ip->pv[1 +1] + ip->pv[2 +1] + ip->pv[3 +1] + ip->pv[4 +1] + ip->pv[5 +1] + ip->pv[6 +1] + ip->pv[7 +1];

/*
    If all samples p[ x, -1 ], with x = 0..7, and p[ -1, y ], with y = 0..7, are marked
    as "available for Intra_8x8 prediction,".
*/
    if (ip->sample_up &&
        ip->sample_left)
    {
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = (sumH + sumV + 8) >> 4;
    }
/*
    Otherwise, if any samples p[ x, -1 ], with x = 0..7, are marked as "not available
    for Intra_8x8 prediction" and all samples p[ -1, y ], with y = 0..7, are marked as
    "available for Intra_8x8 prediction".
*/
    else if (!ip->sample_up &&
             ip->sample_left)
    {
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = (sumV + 4) >> 3;
    }
/*
    Otherwise, if any samples p[ -1, y ], with y = 0..7, are marked as "not available
    for Intra_8x8 prediction" and all samples p[ x, -1 ], with x = 0..7, are marked as
   "available for Intra_8x8 prediction".
*/
    else if (ip->sample_up &&
             !ip->sample_left)
    {
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = (sumH + 4) >> 3;
    }
/*
    Otherwise, if any samples p[ -1, y ], with y = 0..7, are marked as "not available
    for Intra_8x8 prediction" and all samples p[ x, -1 ], with x = 0..7, are marked as
    "available for Intra_8x8 prediction".
*/
    else if (!ip->sample_up &&
             !ip->sample_left)
    {
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++)
                pred8x8L[x][y] = (1 << (ip->BitDepthY - 1));
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_DC prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Diagonal Down Left prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.5 Specification of Intra_8x8_Diagonal_Down_Left prediction mode.
 */
static int Intra_8x8_Diagonal_Down_Left(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Diagonal_Down_Left()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..15 are marked
    as "available for Intra_8x8 prediction".
*/
    if (ip->sample_up &&
        ip->sample_up_right)
    {
        int x = 0, y = 0;
        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                if (x == 7 && y == 7)
                    pred8x8L[x][y] = (ip->ph[14 +1] + 3 * ip->ph[15 +1] + 2) >> 2;
                else
                    pred8x8L[x][y] = (ip->ph[(x + y) +1] + 2 * ip->ph[(x + y + 1) +1] + ip->ph[(x + y + 2) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Diagonal_Down_Left prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}
/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Diagonal Down Right prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.6 Specification of Intra_8x8_Diagonal_Down_Right prediction mode.
 */
static int Intra_8x8_Diagonal_Down_Right(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Diagonal_Down_Right()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 and p[ -1, y ]
    with y = -1..7 are marked as "available for Intra_8x8 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                if (x > y)
                    pred8x8L[x][y] = (ip->ph[(x - y - 2) +1] + 2 * ip->ph[(x - y - 1) +1] + ip->ph[(x - y) +1] + 2) >> 2;
                else if (x < y)
                    pred8x8L[x][y] = (ip->pv[(y - x - 2) +1] + 2 * ip->pv[(y - x -1) +1] + ip->pv[(y - x) +1] + 2) >> 2;
                else
                    pred8x8L[x][y] = (ip->ph[0 +1] + 2 * ip->pv[0] + ip->pv[0 +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Diagonal_Down_Right prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Vertical Right prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.7 Specification of Intra_8x8_Vertical_Right prediction mode.
 */
static int Intra_8x8_Vertical_Right(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Vertical_Right()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 and
    p[ -1, y ] with y = -1..7 are marked as "available for Intra_8x8 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int zVR = 0;
        int x = 0, y = 0;

        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                zVR = 2 * x - y;

                if (zVR > -1)
                {
                    if (zVR % 2 == 0)
                        pred8x8L[x][y] = (ip->ph[(x - (y >> 1) - 1) +1] + ip->ph[(x - (y >> 1)) +1] + 1) >> 1;
                    else
                        pred8x8L[x][y] = (ip->ph[(x - (y >> 1) - 2) +1] + 2 * ip->ph[(x - (y >> 1) - 1) +1] + ip->ph[(x - (y >> 1)) +1] + 2) >> 2;
                }
                else if (zVR == -1)
                    pred8x8L[x][y] = (ip->pv[0 +1] + 2 * ip->pv[0] + ip->ph[0 +1] + 2) >> 2;
                else
                    pred8x8L[x][y] = (ip->pv[(y - 2*x - 1) +1] + 2 * ip->pv[(y - 2*x - 2) +1] + ip->pv[(y - 2*x - 3) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Vertical_Right prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Horizontal Down prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.8 Specification of Intra_8x8_Horizontal_Down prediction mode.
 */
static int Intra_8x8_Horizontal_Down(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Horizontal_Down()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..7 and
    p[ -1, y ] with y = -1..7 are marked as "available for Intra_8x8 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up_left &&
        ip->sample_up)
    {
        int zHD = 0;
        int x = 0, y = 0;

        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                zHD = 2 * y - x;

                if (zHD > -1)
                {
                    if (zHD % 2 == 0)
                        pred8x8L[x][y] = (ip->pv[(y - (x >> 1) - 1) +1] + ip->pv[(y - (x >> 1)) +1] + 1) >> 1;
                    else
                        pred8x8L[x][y] = (ip->pv[(y - (x >> 1) - 2) +1] + 2 * ip->pv[(y - (x >> 1) - 1) +1] + ip->pv[(y - (x >> 1)) +1] + 2) >> 2;
                }
                else if (zHD == -1)
                    pred8x8L[x][y] = (ip->pv[0 +1] + 2 * ip->pv[0] + ip->ph[0 +1] + 2) >> 2;
                else
                    pred8x8L[x][y] = (ip->ph[(x - 2*y - 1) +1] + 2 * ip->ph[(x - 2*y - 2) +1] + ip->ph[(x - 2*y - 3) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Horizontal_Down prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Vertical Left prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.9 Specification of Intra_8x8_Vertical_Left prediction mode.
 */
static int Intra_8x8_Vertical_Left(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Vertical_Left()" CLR_RESET);
    int retcode = SUCCESS;
/*
    The values of the prediction samples pred8x8L[ x, y ], with x, y = 0..7, are
    derived as follows.
*/
    if (ip->sample_up &&
        ip->sample_up_right)
    {
        int x = 0, y = 0;
        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                if (y % 2 == 0)
                    pred8x8L[x][y] = (ip->ph[(x + (y >> 1)) +1] + ip->ph[(x + (y >> 1) + 1) +1] + 1) >> 1;
                else
                    pred8x8L[x][y] = (ip->ph[(x + (y >> 1)) +1] + 2 * ip->ph[(x + (y >> 1) + 1) +1] + ip->ph[(x + (y >> 1) + 2) +1] + 2) >> 2;
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Vertical_Left prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 8x8 Horizontal Up prediction.
 * \param pred8x8L[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.2.2.10 Specification of Intra_8x8_Horizontal_Up prediction mode.
 */
static int Intra_8x8_Horizontal_Up(uint8_t pred8x8L[8][8], intrapred8x8_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_8x8_Horizontal_Up()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ -1, y ] with y = 0..7 are marked
    as "available for Intra_8x8 prediction".
*/
    if (ip->sample_left)
    {
        int zHU = 0;
        int x = 0, y = 0;

        for (x = 0; x < 8; x++)
        {
            for (y = 0; y < 8; y++)
            {
                zHU = x + 2 * y;

                if (zHU < 13)
                {
                    if (zHU % 2 == 0)
                        pred8x8L[x][y] = (ip->pv[(y + (x >> 1)) +1] + ip->pv[(y + (x >> 1) + 1) +1] + 1) >> 1;
                    else
                        pred8x8L[x][y] = (ip->pv[(y + (x >> 1)) +1] + 2 * ip->pv[(y + (x >> 1) + 1) +1] + ip->pv[(y + (x >> 1) + 2) +1] + 2) >> 2;
                }
                else if (zHU == 13)
                    pred8x8L[x][y] = (ip->pv[6 +1] + 3 * ip->pv[7 +1] + 2) >> 2;
                else
                    pred8x8L[x][y] = ip->pv[7 +1];
            }
        }
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_8x8_Horizontal_Up prediction for block n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Intra chroma prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.3 Intra_16x16 prediction process for luma samples.
 *
 * This process is invoked when the macroblock prediction mode is equal to Intra_4x4.
 */
static int Intra_16x16_luma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_16x16_luma_prediction_process()" CLR_RESET);
/*
    // Table 8-4: Specification of Intra16x16PredMode and associated names
    0   Intra_16x16_Vertical
    1   Intra_16x16_Horizontal
    2   Intra_16x16_DC
    3   Intra_16x16_Plane
*/
    // Shortcut
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];

    // Initialization
    int retcode = SUCCESS;
    int mbAddrN;
    int x, y;
    int xW, yW;

    intrapred16x16_t ip;
    ip.phv = 0;
    ip.blkIdx = mb->mbAddr;
    ip.BitDepthY = dc->sps_array[pps->seq_parameter_set_id]->BitDepthY;
    ip.sample_left = false;
    ip.sample_up = false;

    // Up Left corner sample
    x = -1, y = -1;
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, x, y, &mbAddrN, &xW, &yW);

        // 2
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample phv[-1,-1] is NOT available for Intra_16x16 prediction (mb=%i) (xW=%i, yW=%i)", mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.phv = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample phv[-1,-1] = %i      (mb=%i) (xW=%i, yW=%i)", ip.phv, mbAddrN, xW, yW);
        }
    }

    // Vertical samples
    x = -1, y = -1;
    for (y = 0; y < 16; y++)
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, x, y, &mbAddrN, &xW, &yW);

        // 2
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample pv[-1,%i] is NOT available for Intra_16x16 prediction (mb=%i) (xW=%i, yW=%i)", y, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_left = true;
            ip.pv[y] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample pv[-1,%i] = %i      (mb=%i) (xW=%i, yW=%i)", y, ip.pv[y], mbAddrN, xW, yW);
        }
    }

    // Horizontal samples
    x = 0, y = -1;
    for (x = 0; x < 16; x++)
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 1, x, y, &mbAddrN, &xW, &yW);

        // 2
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample ph[%i,-1] is NOT available for Intra_16x16 prediction (mb=%i) (xW=%i, yW=%i)", x, mbAddrN, xW, yW);
        }
        else
        {
            //int xM = 0, yM = 0;
            //InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xM, &yM);

            ip.sample_up = true;
            ip.ph[x] = dc->mb_array[mbAddrN]->SprimeL[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample ph[%i, -1] = %i      (mb=%i) (xW=%i, yW=%i)", x, ip.ph[x], mbAddrN, xW, yW);
        }
    }

    // Predictions
    switch (mb->Intra16x16PredMode)
    {
        case 0:
            Intra_16x16_Vertical(mb->predL, &ip);
            break;
        case 1:
            Intra_16x16_Horizontal(mb->predL, &ip);
            break;
        case 2:
            Intra_16x16_DC(mb->predL, &ip);
            break;
        case 3:
            Intra_16x16_Plane(mb->predL, &ip);
            break;
        default:
            TRACE_ERROR(INTRA, "Unable to understand 16x16 prediction mode!");
            break;
    }

    // Transformation
    transform16x16_luma(dc, mb);

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 16x16 vertical prediction.
 * \param predL[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.3.1 Specification of Intra_16x16_Vertical prediction mode.
 */
static int Intra_16x16_Vertical(uint8_t predL[16][16], intrapred16x16_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_16x16_Vertical()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This Intra_16x16 prediction mode shall be used only when the samples p[ x, -1 ]
    with x = 0..15 are marked as "available for Intra_16x16 prediction".
*/
    if (ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = ip->ph[x];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_16x16_Vertical prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 16x16 horizontal prediction.
 * \param predL[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.3.2 Specification of Intra_16x16_Horizontal prediction mode.
 */
static int Intra_16x16_Horizontal(uint8_t predL[16][16], intrapred16x16_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_16x16_Horizontal()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This Intra_16x16 prediction mode shall be used only when the samples p[ -1, y ]
    with x = 0..15 are marked as "available for Intra_16x16 prediction".
*/
    if (ip->sample_left)
    {
        int x = 0, y = 0;
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = ip->pv[y];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_16x16_Horizontal prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 16x16 DC prediction.
 * \param predL[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.3.3 Specification of Intra_16x16_DC prediction mode.
 *
 * Note: A 16x16 luma block can always be predicted using this mode.
 */
static int Intra_16x16_DC(uint8_t predL[16][16], intrapred16x16_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_16x16_DC()" CLR_RESET);
    int retcode = SUCCESS;

    int x = 0, y = 0;
    int sumH = 0, sumV = 0;
    for (x = 0; x < 16; x++)
    {
        sumH += ip->ph[x];
        sumV += ip->pv[x];
    }
/*
    If all neighbouring samples p[ x, -1 ], with x = 0..15, and p[ -1, y ], with y = 0..15,
    are marked as "available for Intra_16x16 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up)
    {
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = (sumH + sumV + 16) >> 5;
    }
/*
    Otherwise, if any of the neighbouring samples p[ x, -1 ], with x = 0..15, are marked
    as "not available for Intra_16x16 prediction" and all of the neighbouring samples
    p[ -1, y ], with y = 0..15, are marked as "available for Intra_16x16 prediction".
*/
    else if (ip->sample_left &&
             !ip->sample_up)
    {
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = (sumV + 8) >> 4;
    }
/*
    Otherwise, if any of the neighbouring samples p[ -1, y ], with y = 0..15, are marked
    as "not available for Intra_16x16 prediction" and all of the neighbouring samples
    p[ x, -1 ], with x = 0..15, are marked as "available for Intra_16x16 prediction".
*/
    else if (!ip->sample_left &&
             ip->sample_up)
    {
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = (sumH + 8) >> 4;
    }
/*
    Otherwise (some of the neighbouring samples p[ x, -1 ], with x = 0..15, and some
    of the neighbouring samples p[ -1, y ], with y = 0..15, are marked as
    "not available for Intra_16x16 prediction".
*/
    else if (!ip->sample_up &&
             !ip->sample_left)
    {
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = (1 << (ip->BitDepthY - 1));
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_16x16_DC prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra 16x16 plane prediction.
 * \param predL[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.3.4 Specification of Intra_16x16_Plane prediction mode.
 */
static int Intra_16x16_Plane(uint8_t predL[16][16], intrapred16x16_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_16x16_Plane()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This Intra_16x16 prediction mode shall be used only when the samples p[ x, -1 ] with
    x = -1..15 and p[ -1, y ] with y = 0..15 are marked as "available for Intra_16x16 prediction".
*/
    if (ip->sample_left &&
        ip->sample_up)
    {
        int H = 0, V = 0;
        int a = 0, b = 0, c = 0;
        int i = 0;
        int x = 0, y = 0;

        for (i = 0; i < 8; i++)
        {
            if (6-i == -1)
            {
                H += (i + 1) * (ip->ph[8 + i] - ip->phv);
                V += (i + 1) * (ip->pv[8 + i] - ip->phv);
            }
            else
            {
                H += (i + 1) * (ip->ph[8 + i] - ip->ph[6 - i]);
                V += (i + 1) * (ip->pv[8 + i] - ip->pv[6 - i]);
            }
        }

        a = 16 * (ip->pv[15] + ip->ph[15]);
        b = (5 * H + 32) >> 6;
        c = (5 * V + 32) >> 6;

        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++)
                predL[x][y] = Clip1_YCbCr_8((a + b * (x - 7) + c * (y - 7) + 16) >> 5);
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_16x16_Plane prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Intra chroma prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.4 Intra prediction process for chroma samples.
 *
 * This process is invoked when the macroblock prediction mode is equal to Intra_4x4.
 */
static int Intra_Chroma_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(INTRA, "> " BLD_GREEN "Intra_Chroma_prediction_process()" CLR_RESET);
    TRACE_1(INTRA, "  > blkIdx %i", dc->CurrMbAddr);

    // Shortcuts
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    // Initialization
    int retcode = SUCCESS;
    int mbAddrN;
    int xW, yW;
    int x, y;

    intrapredChroma_t ipCb;
    ipCb.blkIdx = dc->CurrMbAddr;
    ipCb.MbWidthC = sps->MbWidthC;
    ipCb.MbHeightC = sps->MbHeightC;
    ipCb.BitDepthC = sps->BitDepthC;
    ipCb.ChromaArrayType = dc->ChromaArrayType;
    ipCb.sample_left = false;
    ipCb.sample_up = false;

    intrapredChroma_t ipCr;
    ipCr.blkIdx = dc->CurrMbAddr;
    ipCr.MbWidthC = sps->MbWidthC;
    ipCr.MbHeightC = sps->MbHeightC;
    ipCr.BitDepthC = sps->BitDepthC;
    ipCr.ChromaArrayType = dc->ChromaArrayType;
    ipCr.sample_left = false;
    ipCr.sample_up = false;

    // Up Left corner samples
    x = -1, y = -1;
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 0, x, y, &mbAddrN, &xW, &yW);

        // 2
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample phv[-1,-1] is NOT available for Intra_Chroma prediction (mb=%i)  (xW=%i, yW=%i)", mbAddrN, xW, yW);
        }
        else
        {
/*
            int xL = 0, yL = 0;
            InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xL, &yL);

            int xM = 0, yM = 0;
            xM = (xL >> 4) * MbWidthC;
            yM = ((yL >> 4) * MbHeightC) + (yL % 2);
*/
            ipCb.pv[0] = ipCb.ph[0] = dc->mb_array[mbAddrN]->SprimeCb[/*xM + */xW][/*yM + */yW];
            ipCr.pv[0] = ipCr.ph[0] = dc->mb_array[mbAddrN]->SprimeCr[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample phv Cb[-1,-1] = %i      (mb=%i) (xW=%i, yW=%i)", ipCb.pv[0], mbAddrN, xW, yW);
            TRACE_2(INTRA, "  > sample phv Cr[-1,-1] = %i      (mb=%i) (xW=%i, yW=%i)", ipCr.pv[0], mbAddrN, xW, yW);
        }
    }

    // Vertical samples
    x = -1, y = 0;
    for (y = 0; y < (int)sps->MbHeightC; y++)
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 0, x, y, &mbAddrN, &xW, &yW);

        // 2
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample pv[-1,%i] is NOT available for Intra_4x4_chroma prediction (mb=%i)  (xW=%i, yW=%i)", y, mbAddrN, xW, yW);
        }
        else
        {
/*
            int xL = 0, yL = 0;
            InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xL, &yL);

            int xM = 0, yM = 0;
            xM = (xL >> 4) * MbWidthC;
            yM = ((yL >> 4) * MbHeightC) + (yL % 2);
*/
            ipCb.sample_left = true;
            ipCr.sample_left = true;

            ipCb.pv[y +1] = dc->mb_array[mbAddrN]->SprimeCb[/*xM + */xW][/*yM + */yW];
            ipCr.pv[y +1] = dc->mb_array[mbAddrN]->SprimeCr[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample pv Cb[-1,%i] = %i      (mb=%i) (xW=%i, yW=%i)", y, ipCb.pv[y +1], mbAddrN, xW, yW);
            TRACE_2(INTRA, "  > sample pv Cr[-1,%i] = %i      (mb=%i) (xW=%i, yW=%i)", y, ipCr.pv[y +1], mbAddrN, xW, yW);
        }
    }

    // Horizontal samples
    x = 0, y = -1;
    for (x = 0; x < (int)sps->MbWidthC; x++)
    {
        // 1
        xW = yW = mbAddrN = -1;
        deriv_neighbouringlocations(dc, 0, x, y, &mbAddrN, &xW, &yW);

        // 3
        if (mbAddrN == -1 ||
            (dc->mb_array[mbAddrN]->MbPartPredMode[0] > 3 && pps->constrained_intra_pred_flag == true))
        {
            TRACE_2(INTRA, "  > sample ph[%i,-1] is NOT available for Intra_4x4_chroma prediction (mb=%i) (xW=%i, yW=%i)", x, mbAddrN, xW, yW);
        }
        else
        {
/*
            int xL = 0, yL = 0;
            InverseMacroblockScan(mbAddrN, false, sps->PicWidthInSamplesL, &xL, &yL);

            int xM = 0, yM = 0;
            xM = (xL >> 4) * MbWidthC;
            yM = ((yL >> 4) * MbHeightC) + (yL % 2);
*/
            ipCb.sample_up = true;
            ipCr.sample_up = true;

            ipCb.ph[x +1] = dc->mb_array[mbAddrN]->SprimeCb[/*xM + */xW][/*yM + */yW];
            ipCr.ph[x +1] = dc->mb_array[mbAddrN]->SprimeCr[/*xM + */xW][/*yM + */yW];
            TRACE_2(INTRA, "  > sample ph Cb[%i,-1] = %u      (mb=%i) (xW=%i, yW=%i)", x, ipCb.ph[x +1], mbAddrN, xW, yW);
            TRACE_2(INTRA, "  > sample ph Cr[%i,-1] = %u      (mb=%i) (xW=%i, yW=%i)", x, ipCr.ph[x +1], mbAddrN, xW, yW);
        }
    }

    // Predictions
    switch (mb->IntraChromaPredMode)
    {
        case 0:
            Intra_Chroma_DC(mb->predCb, &ipCb);
            Intra_Chroma_DC(mb->predCr, &ipCr);
            break;
        case 1:
            Intra_Chroma_Horizontal(mb->predCb, &ipCb);
            Intra_Chroma_Horizontal(mb->predCr, &ipCr);
            break;
        case 2:
            Intra_Chroma_Vertical(mb->predCb, &ipCb);
            Intra_Chroma_Vertical(mb->predCr, &ipCr);
            break;
        case 3:
            Intra_Chroma_Plane(mb->predCb, &ipCb);
            Intra_Chroma_Plane(mb->predCr, &ipCr);
            break;
        default:
            TRACE_ERROR(INTRA, "Unable to understand chroma prediction mode!");
            break;
    }

    // Transformation
    if (dc->ChromaArrayType == 3)
    {
        transform4x4_chroma_cat3(dc, mb);
    }
    else
    {
        transform4x4_chroma(dc, mb);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra chroma DC prediction.
 * \param predC[][] The Cb or Cr block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.4.1 Specification of Intra_Chroma_DC prediction mode.
 *
 * Note: A chroma block can always be predicted using this mode.
 */
static int Intra_Chroma_DC(uint8_t predC[8][8], intrapredChroma_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_Chroma_DC()" CLR_RESET);

    int retcode = SUCCESS;
    int sum = 0;
    int x = 0, y = 0, i = 0;
    int xO = -1, yO = -1;
    int nbBlocks = (1 << (ip->ChromaArrayType + 1));
    int chroma4x4BlkIdx = 0;

    // Loop on each 4x4 chroma block
    for (chroma4x4BlkIdx = 0; chroma4x4BlkIdx < nbBlocks; chroma4x4BlkIdx++)
    {
        sum = 0;
        InverseChroma4x4BlkScan(chroma4x4BlkIdx, &xO, &yO);

        TRACE_1(INTRA, "xO = %i, yO = %i", xO, yO);

        if (!ip->sample_left && !ip->sample_up)
        {
            for (x = 0; x < 4; x++)
               for (y = 0; y < 4; y++)
                   predC[x + xO][y + yO] = (1 << (ip->BitDepthC - 1));
        }
        else
        {
            if ((xO == 0 && yO == 0) || (xO > 0 && yO > 0))
            {
                if (ip->sample_left && ip->sample_up)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->ph[(i + xO) +1] + ip->pv[(i + yO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 4) >> 3;
                }
                else if (ip->sample_left && !ip->sample_up)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->pv[(i + yO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
                else if (!ip->sample_left && ip->sample_up)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->ph[(i + xO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
            }
            else if (xO > 0 && yO == 0)
            {
                if (ip->sample_up)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->ph[(i + xO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
                else if (ip->sample_left)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->pv[(i + yO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
            }
            else if (xO == 0 && yO > 0)
            {
                if (ip->sample_left)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->pv[(i + yO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
                else if (ip->sample_up)
                {
                    for (i = 0; i < 4; i++)
                        sum += ip->ph[(i + xO) +1];

                    for (x = 0; x < 4; x++)
                       for (y = 0; y < 4; y++)
                           predC[x + xO][y + yO] = (sum + 2) >> 2;
                }
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra chroma horizontal prediction.
 * \param predC[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.4.2 Specification of Intra_Chroma_Horizontal prediction mode.
 */
static int Intra_Chroma_Horizontal(uint8_t predC[8][8], intrapredChroma_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_Chroma_Horizontal()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ -1, y ] with y = 0..MbHeightC - 1,
    are marked as "available for Intra chroma prediction".
*/
    if (ip->sample_left)
    {
        int x = 0, y = 0;
        for (x = 0; x < ip->MbWidthC; x++)
            for (y = 0; y < ip->MbHeightC; y++)
                predC[x][y] = ip->pv[y +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_Chroma_Horizontal prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra chroma  vertical prediction.
 * \param predC[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.4.3 Specification of Intra_Chroma_Vertical prediction mode.
 */
static int Intra_Chroma_Vertical(uint8_t predC[8][8], intrapredChroma_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_Chroma_Vertical()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ] with x = 0..MbWidthC - 1
    are marked as "available for Intra chroma prediction".
*/
    if (ip->sample_up)
    {
        int x = 0, y = 0;
        for (x = 0; x < ip->MbWidthC; x++)
            for (y = 0; y < ip->MbHeightC; y++)
                predC[x][y] = ip->ph[x +1];
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_Chroma_Vertical prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Intra chroma plane prediction.
 * \param predC[][] The block to fill with predicted coefficients.
 * \param *ip A structure containing informations about prediction samples and there availability.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.4.4 Specification of Intra_Chroma_Plane prediction mode.
 */
static int Intra_Chroma_Plane(uint8_t predC[8][8], intrapredChroma_t *ip)
{
    TRACE_1(INTRA, "  > " BLD_GREEN "Intra_Chroma_Plane()" CLR_RESET);
    int retcode = SUCCESS;
/*
    This mode shall be used only when the samples p[ x, -1 ], with x = 0..MbWidthC - 1
    and p[ -1, y ], with y = -1..MbHeightC - 1 are marked as "available for Intra
    chroma prediction".
*/
    if (ip->sample_left && ip->sample_up)
    {
        int H = 0, V = 0;
        int a = 0, b = 0, c = 0;
        int x = 0, y = 0;
        int i = 0;

        int xCF = ((ip->ChromaArrayType == 3) ? 4 : 0);
        int yCF = ((ip->ChromaArrayType != 1) ? 4 : 0);

        for (i = 0; i < 4+xCF; i++)
            H += (i+1) * (ip->ph[(4 + xCF + i) +1] - ip->ph[(2 + xCF - i) +1]);

        for (i = 0; i < 4+yCF; i++)
            V += (i+1) * (ip->pv[(4 + yCF + i) +1] - ip->pv[(2 + yCF - i) +1]);

        a = 16 * (ip->pv[(ip->MbHeightC - 1) +1] + ip->ph[(ip->MbWidthC - 1) +1]);
        b = ((34 - 29 * (ip->ChromaArrayType == 3)) * H + 32) >> 6;
        c = ((34 - 29 * (ip->ChromaArrayType != 1)) * V + 32) >> 6;

        for (x = 0; x < ip->MbWidthC; x++)
            for (y = 0; y < ip->MbHeightC; y++)
                predC[x][y] = Clip1_YCbCr_8((a + b * (x - 3 - xCF) + c * (y - 3 - yCF) + 16) >> 5);
    }
    else
    {
        TRACE_ERROR(INTRA, "Unable to perform Intra_Chroma_Plane prediction for macroblock n°%i", ip->blkIdx);
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

// 8.3.4.5 Intra prediction for chroma samples with ChromaArrayType equal to 3

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Sample construction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.3.5 Sample construction process for I_PCM macroblocks.
 *
 * This process is invoked when the macroblock type is I_PCM.
 * This feature is currently unsupported by this decoder.
 */
static int ipcm_construction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(INTRA, "> " BLD_GREEN "ipcm_construction_process()" CLR_RESET);
    int retcode = FAILURE;

#if ENABLE_IPCM
    // Shortcuts
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    // Initialization
    int i = 0;
    int dy = 1;
    int xP = 0, yP = 0;

#if ENABLE_MBAFF
    // MbaffFrameFlag is unsupported, so always "false"
    if (dc->active_slice->MbaffFrameFlag == true && dc->active_slice->field_pic_flag == true)
    {
        dy = 2;
    }
#endif // ENABLE_MBAFF

    InverseMacroblockScan(mb->mbAddr, dc->active_slice->MbaffFrameFlag, sps->PicWidthInSamplesL, &xP, &yP);

    // Luma samples construction
    for (i = 0; i < 256; i++)
    {
        mb->SprimeL[xP + (i % 16)][yP + dy * (i / 16)] = mb->pcm_sample_luma[i];
    }

    // Chroma samples construction
    if (dc->ChromaArrayType != 0)
    {
        for (i = 0; i < sps->MbWidthC * sps->MbHeightC; i++)
        {
           mb->SprimeCb[(xP / sps->SubWidthC) + (i % sps->MbWidthC)][((yP + sps->SubHeightC - 1) / sps->SubHeightC) + dy * (i / sps->MbWidthC)] = mb->pcm_sample_chroma[i];
           mb->SprimeCr[(xP / sps->SubWidthC) + (i % sps->MbWidthC)][((yP + sps->SubHeightC - 1) / sps->SubHeightC) + dy * (i / sps->MbWidthC)] = mb->pcm_sample_chroma[i + sps->MbWidthC * sps->MbHeightC];
        }
    }

    retcode = SUCCESS;
#endif // ENABLE_IPCM

    return retcode;
}

/* ************************************************************************** */
