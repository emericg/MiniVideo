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
 * \file      h264_macroblock_utils.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

// minivideo headers
#include "h264_macroblock_utils.h"
#include "../../minitraces.h"
#include "../../utils.h"
#include "../../minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

/* ************************************************************************** */

/*!
 * \brief Print informations about macroblock_layer decoding.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 */
void print_macroblock_layer(DecodingContext_t *dc, Macroblock_t *mb)
{
#if ENABLE_DEBUG
    printf("[MB] <> " BLD_GREEN "print_macroblock_layer()\n" CLR_RESET);

    printf("[MB] ============" BLD_BLUE " MB %u (%2u,%2u) " CLR_RESET "============\n", mb->mbAddr, mb->mbAddr_x, mb->mbAddr_y);
    printf("[MB] - Mb position in file   : 0x%X:%u (%u bits)\n", (mb->mbFileAddrStart) / 8, (mb->mbFileAddrStart) % 8, mb->mbFileAddrStart);
    printf("[MB] - Mb size               : %u bits\n", mb->mbFileAddrStop - mb->mbFileAddrStart + 1);
    printf("[MB] - frame_num / idr_pic_id= %u / %u\n", dc->active_slice->frame_num, dc->active_slice->idr_pic_id);

    if (dc->active_slice->slice_type == 0 || dc->active_slice->slice_type == 5)
    {
        printf("[MB] - slice type   = P Slice (%u)\n", dc->active_slice->slice_type);

        switch (mb->mb_type)
        {
            case P_L0_16x16:
                printf("[MB] - mb_type  = P_L0_16x16 (%u)\n", mb->mb_type);
            break;
            case P_L0_L0_16x8:
                printf("[MB] - mb_type  = P_L0_L0_16x8 (%u)\n", mb->mb_type);
            break;
            case P_L0_L0_8x16:
                printf("[MB] - mb_type  = P_L0_L0_8x16 (%u)\n", mb->mb_type);
            break;
            case P_8x8:
                printf("[MB] - mb_type  = P_8x8 (%u)\n", mb->mb_type);
            break;
            case P_8x8ref0:
                printf("[MB] - mb_type  = P_8x8ref0 (%u)\n", mb->mb_type);
            break;
            case P_Skip:
                printf("[MB] - mb_type  = P_Skip (%u)\n", mb->mb_type);
            break;
            default:
                TRACE_ERROR(MB, "[MB] - mb_type  = unknow (%u)", mb->mb_type);
            break;
        }

        // TODO handle sub_mb_type !!
    }
    else if (dc->active_slice->slice_type == 1 || dc->active_slice->slice_type == 6)
    {
        printf("[MB] - slice type   = B Slice (%u)\n", dc->active_slice->slice_type);

        switch (mb->mb_type)
        {
            case B_Direct_16x16:
                printf("[MB] - mb_type  = B_Direct_16x16 (%u)\n", mb->mb_type);
            break;
            case B_L0_16x16:
                printf("[MB] - mb_type  = B_L0_16x16 (%u)\n", mb->mb_type);
            break;
            case B_L1_16x16:
                printf("[MB] - mb_type  = B_L1_16x16 (%u)\n", mb->mb_type);
            break;
            case B_Bi_16x16:
                printf("[MB] - mb_type  = B_Bi_16x16 (%u)\n", mb->mb_type);
            break;
            case B_L0_L0_16x8:
                printf("[MB] - mb_type  = B_L0_L0_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L0_L0_8x16:
                printf("[MB] - mb_type  = B_L0_L0_8x16 (%u)\n", mb->mb_type);
            break;
            case B_L1_L1_16x8:
                printf("[MB] - mb_type  = B_L1_L1_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L1_L1_8x16:
                printf("[MB] - mb_type  = B_L1_L1_8x16 (%u)\n", mb->mb_type);
            break;
            case B_L0_L1_16x8:
                printf("[MB] - mb_type  = B_L0_L1_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L0_L1_8x16:
                printf("[MB] - mb_type  = B_L0_L1_8x16 (%u)\n", mb->mb_type);
            break;
            case B_L1_L0_16x8:
                printf("[MB] - mb_type  = B_L1_L0_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L1_L0_8x16:
                printf("[MB] - mb_type  = B_L1_L0_8x16 (%u)\n", mb->mb_type);
            break;
            case B_L0_Bi_16x8:
                printf("[MB] - mb_type  = B_L0_Bi_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L0_Bi_8x16:
                printf("[MB] - mb_type  = B_L0_Bi_8x16 (%u)\n", mb->mb_type);
            break;
            case B_L1_Bi_16x8:
                printf("[MB] - mb_type  = B_L1_Bi_16x8 (%u)\n", mb->mb_type);
            break;
            case B_L1_Bi_8x16:
                printf("[MB] - mb_type  = B_L1_Bi_8x16 (%u)\n", mb->mb_type);
            break;
            case B_Bi_L0_16x8:
                printf("[MB] - mb_type  = B_Bi_L0_16x8 (%u)\n", mb->mb_type);
            break;
            case B_Bi_L0_8x16:
                printf("[MB] - mb_type  = B_Bi_L0_8x16 (%u)\n", mb->mb_type);
            break;
            case B_Bi_L1_16x8:
                printf("[MB] - mb_type  = B_Bi_L1_16x8 (%u)\n", mb->mb_type);
            break;
            case B_Bi_L1_8x16:
                printf("[MB] - mb_type  = B_Bi_L1_8x16 (%u)\n", mb->mb_type);
            break;
            case B_Bi_Bi_16x8:
                printf("[MB] - mb_type  = B_Bi_Bi_16x8 (%u)\n", mb->mb_type);
            break;
            case B_Bi_Bi_8x16:
                printf("[MB] - mb_type  = B_Bi_Bi_8x16 (%u)\n", mb->mb_type);
            break;
            case B_8x8:
                printf("[MB] - mb_type  = B_8x8 (%u)\n", mb->mb_type);
            break;
            case B_Skip:
                printf("[MB] - mb_type  = B_Skip (%u)\n", mb->mb_type);
            break;
            default:
                TRACE_ERROR(MB, "[MB] - mb_type= unknow (%u)", mb->mb_type);
            break;
        }
    }
    else if (dc->active_slice->slice_type == 2 || dc->active_slice->slice_type == 7)
    {
        printf("[MB] - slice type   = I Slice (%u)\n", dc->active_slice->slice_type);

        switch (mb->mb_type)
        {
            case I_NxN:
                if (mb->transform_size_8x8_flag)
                    printf("[MB] - mb_type  = I_8x8 (%u)\n", mb->mb_type);
                else
                    printf("[MB] - mb_type  = I_4x4 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_0_0:
                printf("[MB] - mb_type  = I_16x16_0_0_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_0_0:
                printf("[MB] - mb_type  = I_16x16_1_0_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_0_0:
                printf("[MB] - mb_type  = I_16x16_2_0_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_0_0:
                printf("[MB] - mb_type  = I_16x16_3_0_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_1_0:
                printf("[MB] - mb_type  = I_16x16_0_1_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_1_0:
                printf("[MB] - mb_type  = I_16x16_1_1_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_1_0:
                printf("[MB] - mb_type  = I_16x16_2_1_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_1_0:
                printf("[MB] - mb_type  = I_16x16_3_1_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_2_0:
                printf("[MB] - mb_type  = I_16x16_0_2_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_2_0:
                printf("[MB] - mb_type  = I_16x16_1_2_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_2_0:
                printf("[MB] - mb_type  = I_16x16_2_2_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_2_0:
                printf("[MB] - mb_type  = I_16x16_3_2_0 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_0_1:
                printf("[MB] - mb_type  = I_16x16_0_0_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_0_1:
                printf("[MB] - mb_type  = I_16x16_1_0_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_0_1:
                printf("[MB] - mb_type  = I_16x16_2_0_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_0_1:
                printf("[MB] - mb_type  = I_16x16_3_0_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_1_1:
                printf("[MB] - mb_type  = I_16x16_0_1_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_1_1:
                printf("[MB] - mb_type  = I_16x16_1_1_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_1_1:
                printf("[MB] - mb_type  = I_16x16_2_1_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_1_1:
                printf("[MB] - mb_type  = I_16x16_3_1_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_0_2_1:
                printf("[MB] - mb_type  = I_16x16_0_2_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_1_2_1:
                printf("[MB] - mb_type  = I_16x16_1_2_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_2_2_1:
                printf("[MB] - mb_type  = I_16x16_2_2_1 (%u)\n", mb->mb_type);
            break;
            case I_16x16_3_2_1:
                printf("[MB] - mb_type  = I_16x16_3_2_1 (%u)\n", mb->mb_type);
            break;
            case I_PCM:
                printf("[MB] - mb_type  = I_PCM (%u)\n", mb->mb_type);
            break;
        }
    }
    else
    {
        printf("[MB] - unknown slice type= %u\n", dc->active_slice->slice_type);
    }

    // TODO properly handle macroblock parts
    printf("[MB] - NumMbPart        : %u\n", mb->NumMbPart);
    //printf("[MB] - MbPartSize     : %ix%i\n", MbPartWidth(dc->active_slice->slice_type, mb->mb_type), MbPartHeight(dc->active_slice->slice_type, mb->mb_type));
    //printf("[MB] - NumSubMbPart   : %u\n", mb->NumSubMbPart);
    //printf("[MB] - SubMbPartSize  : %ix%i\n", xx, yy);

    if (mb->MbPartPredMode[0] != Intra_16x16)
    {
        if (mb->transform_size_8x8_flag)
            printf("[MB] - Luma transform size: 8x8\n");
        else
            printf("[MB] - Luma transform size: 4x4\n");

        printf("[MB] - Coded Block Pattern: %u\n", mb->coded_block_pattern);
    }
    else
    {
        printf("[MB] - Coded Block Pattern: auto\n");
    }

    printf("[MB]  - cdp LUMA    : %u\n", mb->CodedBlockPatternLuma);
    printf("[MB]  - cdp CHROMA  : %u\n", mb->CodedBlockPatternChroma);

    printf("[MB] - mb_qp_delta  = %i\n", mb->mb_qp_delta);
    printf("[MB]  - QPY         : %i\n", mb->QPY);
    printf("[MB]  - QPC         : %i, %i\n", mb->QPC[0], mb->QPC[1]);

    printf("[MB] ==============" BLD_BLUE " Neighbors " CLR_RESET "=============\n");
    if (mb->mbAddrA >= 0)
    {
        printf("[MB] - macroblock A is available at address %i\n", mb->mbAddrA);
    }
    else
    {
        printf("[MB] - macroblock A is not available\n");
    }
    if (mb->mbAddrB >= 0)
    {
        printf("[MB] - macroblock B is available at address %i\n", mb->mbAddrB);
    }
    else
    {
        printf("[MB] - macroblock B is not available\n");
    }
    if (mb->mbAddrC >= 0)
    {
        printf("[MB] - macroblock C is available at address %i\n", mb->mbAddrC);
    }
    else
    {
        printf("[MB] - macroblock C is not available\n");
    }
    if (mb->mbAddrD >= 0)
    {
        printf("[MB] - macroblock D is available at address %i\n", mb->mbAddrD);
    }
    else
    {
        printf("[MB] - macroblock D is not available\n");
    }

    printf("[MB] =============" BLD_BLUE " Predictions " CLR_RESET "============\n");
    if (mb->mb_type == I_PCM)
    {
        printf("[MB] - Luma prediction: I_PCM macroblock, no prediction\n");
    }
    else
    {
        // Luma
        for (unsigned i = 0; i < mb->NumMbPart; ++i)
        {
            if (mb->MbPartPredMode[i] == Intra_4x4)
            {
                printf("[MB] - Luma prediction: Intra_4x4\n");
                unsigned int luma4x4BlkIdx = 0;
                for (luma4x4BlkIdx = 0; luma4x4BlkIdx < 16; luma4x4BlkIdx++)
                {
                    printf("[MB]   - Intra4x4PredMode[%u]: %u\n", luma4x4BlkIdx, mb->Intra4x4PredMode[luma4x4BlkIdx]);
                }
            }
            else if (mb->MbPartPredMode[i] == Intra_16x16)
            {
                printf("[MB] - Luma prediction: Intra_16x16\n");
                printf("[MB]   - Intra16x16PredMode: %u\n", mb->Intra16x16PredMode);
            }
            else if (mb->MbPartPredMode[i] == Intra_8x8)
            {
                printf("[MB] - Luma prediction: Intra_8x8\n");
                unsigned int luma8x8BlkIdx = 0;
                for (luma8x8BlkIdx = 0; luma8x8BlkIdx < 4; luma8x8BlkIdx++)
                {
                    printf("[MB]   - Intra8x8PredMode[%u]: %u\n", luma8x8BlkIdx, mb->Intra8x8PredMode[luma8x8BlkIdx]);
                }
            }
            else if (mb->MbPartPredMode[i] == Direct ||
                     mb->MbPartPredMode[i] == Pred_L0 ||
                     mb->MbPartPredMode[i] == Pred_L1 ||
                     mb->MbPartPredMode[i] == BiPred)
            {
                printf("[MB]   - Inter prediction :\n");
                printf("[MB]     - Motion Vector 1 :\n");
                printf("[MB]     - Motion Vector 2 :\n");
            }
            else
            {
                printf("[MB] - MbPartPredMode[%u]: %u\n", i, mb->MbPartPredMode[i]);
                TRACE_WARNING(MB, "Unknown luma prediction mode\n");
            }
        }

        // Chroma
        if (dc->ChromaArrayType != 0)
        {
            printf("[MB] - Chroma prediction mode: %u\n", mb->IntraChromaPredMode);
        }
    }

    if (dc->entropy_coding_mode_flag)
    {
        printf("[MB] ===========" BLD_BLUE " coded_block_flag " CLR_RESET "==========\n");
        int a = 0;

        if (mb->MbPartPredMode[0] == Intra_16x16)
        {
            printf("[MB]  - [luma] [DC]: %i\n", mb->coded_block_flag[0][16]);
        }
        for (a = 0; a < ((mb->MbPartPredMode[0] == Intra_8x8) ? 4 : 16) ; a++)
        {
            printf("[MB]  - [luma] [%i]: %i\n", a, mb->coded_block_flag[0][a]);
        }

        printf("[MB]  -  [cb]  [DC]: %i\n", mb->coded_block_flag[1][4]);
        for (a = 0; a < 4; a++)
        {
            printf("[MB]  -  [cb]  [%i]: %i\n", a, mb->coded_block_flag[1][a]);
        }

        printf("[MB]  -  [cr]  [DC]: %i\n", mb->coded_block_flag[2][4]);
        for (a = 0; a < 4; a++)
        {
            printf("[MB]  -  [cr]  [%i]: %i\n", a, mb->coded_block_flag[2][a]);
        }
    }
    printf("[MB] ======================================\n\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Print parsed residual coefficient for current macroblock.
 * \param *mb The current macroblock.
 */
void print_macroblock_pixel_residual(Macroblock_t *mb)
{
#if ENABLE_DEBUG
    int blkGrp = 0;
    int linePerBlk = 0;
    int ra = 0;
    int zz = 0;

    printf("[MB] ==============" BLD_BLUE " RESIDUAL Y " CLR_RESET "==============\n");
    if (mb->MbPartPredMode[0] == Intra_4x4)
    {
        for (blkGrp = 0; blkGrp < 4; blkGrp++)
        {
            printf("+-------------------+-------------------+-------------------+-------------------+\n");

            for (linePerBlk = 0; linePerBlk < 4; linePerBlk++)
            {
                for (ra = (0 + 4*blkGrp); ra < (4 + 4*blkGrp); ra++)
                {
                    for (zz = (0 + 4*linePerBlk); zz < (4 + 4*linePerBlk); zz++)
                    {
                        if (zz%4 == 0)
                            printf("|");
                        else
                            printf(",");

                        printf("%4i", mb->LumaLevel4x4[raster_4x4[ra]][zigzag_4x4[zz]]);
                    }
                }
                printf("|\n");
            }
        }
        printf("+-------------------+-------------------+-------------------+-------------------+\n\n");
    }
    else if (mb->MbPartPredMode[0] == Intra_8x8)
    {
        for (blkGrp = 0; blkGrp < 2; blkGrp++)
        {
            printf("+---------------------------------------+---------------------------------------+\n");

            for (linePerBlk = 0; linePerBlk < 8; linePerBlk++)
            {
                for (ra = (0 + 2*blkGrp); ra < (2 + 2*blkGrp); ra++)
                {
                    for (zz = (0 + 8*linePerBlk); zz < (8 + 8*linePerBlk); zz++)
                    {
                        if (zz%8 == 0)
                            printf("|");
                        else
                            printf(",");

                        printf("%4i", mb->LumaLevel8x8[raster_8x8[ra]][zigzag_8x8[zz]]);
                    }
                }
                printf("|\n");
            }
        }
        printf("+---------------------------------------+---------------------------------------+\n\n");
    }
    else if (mb->MbPartPredMode[0] == Intra_16x16)
    {
        for (blkGrp = 0; blkGrp < 4; blkGrp++)
        {
            printf("+-------------------+-------------------+-------------------+-------------------+\n");

            for (linePerBlk = 0; linePerBlk < 4; linePerBlk++)
            {
                for (ra = (0 + 4*blkGrp); ra < (4 + 4*blkGrp); ra++)
                {
                    for (zz = (0 + 4*linePerBlk); zz < (4 + 4*linePerBlk); zz++)
                    {

                        if (zz%4 == 0)
                            printf("|");
                        else
                            printf(",");

                        if (zigzag_4x4[zz] == 0)
                            printf(" DC "); /*("%4i", mb->Intra16x16DCLevel[raster_4x4[ra]]);*/ //FIXME DC coefficient
                        else
                            printf("%4i", mb->Intra16x16ACLevel[raster_4x4[ra]][zigzag_4x4[zz]-1]);
                    }
                }
                printf("|\n");
            }
        }
        printf("+-------------------+-------------------+-------------------+-------------------+\n\n");
    }

    printf("[MB] ==============" BLD_BLUE " RESIDUAL Cb " CLR_RESET "=============\n");
    {
        blkGrp = 0;
        linePerBlk = 0;
        ra = 0;
        zz = 0;

        for (blkGrp = 0; blkGrp < 2; blkGrp++)
        {
            printf("+-------------------+-------------------+\n");

            for (linePerBlk = 0; linePerBlk < 4; linePerBlk++)
            {
                for (ra = (0 + 2*blkGrp); ra < (2 + 2*blkGrp); ra++)
                {
                    for (zz = (0 + 4*linePerBlk); zz < (4 + 4*linePerBlk); zz++)
                    {

                        if (zz%4 == 0)
                            printf("|");
                        else
                            printf(",");

                        if (zigzag_4x4[zz] == 0)
                            printf(" DC "); /*("%4i", mb->ChromaDCLevel[0][raster_8x8[ra]]);*/ //FIXME DC coefficient
                        else
                            printf("%4i", mb->ChromaACLevel[0][raster_8x8[ra]][zigzag_4x4[zz]-1]);
                    }
                }
                printf("|\n");
            }
        }
        printf("+-------------------+-------------------+\n\n");
    }

    printf("[MB] ==============" BLD_BLUE " RESIDUAL Cr " CLR_RESET "=============\n");
    {
        blkGrp = 0;
        linePerBlk = 0;
        ra = 0;
        zz = 0;

        for (blkGrp = 0; blkGrp < 2; blkGrp++)
        {
            printf("+-------------------+-------------------+\n");

            for (linePerBlk = 0; linePerBlk < 4; linePerBlk++)
            {
                for (ra = (0 + 2*blkGrp); ra < (2 + 2*blkGrp); ra++)
                {
                    for (zz = (0 + 4*linePerBlk); zz < (4 + 4*linePerBlk); zz++)
                    {

                        if (zz%4 == 0)
                            printf("|");
                        else
                            printf(",");

                        if (zigzag_4x4[zz] == 0)
                            printf(" DC "); /*("%4i", mb->ChromaDCLevel[1][raster_8x8[ra]]);*/ //FIXME DC coefficient
                        else
                            printf("%4i", mb->ChromaACLevel[1][raster_8x8[ra]][zigzag_4x4[zz]-1]);
                    }
                }
                printf("|\n");
            }
        }
        printf("+-------------------+-------------------+\n\n");
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Print predicted coefficient for current macroblock.
 * \param *mb The current macroblock.
 */
void print_macroblock_pixel_predicted(Macroblock_t *mb)
{
#if ENABLE_DEBUG
    int blkSize = 4;
    int x = 0, y = 0;

    if (mb->MbPartPredMode[0] == Intra_8x8)
        blkSize = 8;

    printf("[MB] =============" BLD_BLUE " PREDICTED Y " CLR_RESET "=============\n");
    for (y = 0; y < 16; y++)
    {
        if (y % blkSize == 0)
            printf("+-------------------+-------------------+-------------------+-------------------+\n");

        for (x = 0; x < 16; x++)
        {
            if (x % blkSize == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->predL[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+-------------------+-------------------+\n\n");

    printf("[MB] =============" BLD_BLUE " PREDICTED Cb " CLR_RESET "============\n");
    for (y = 0; y < 8; y++)
    {
        if (y % 4 == 0)
            printf("+-------------------+-------------------+\n");

        for (x = 0; x < 8; x++)
        {
            if (x % 4 == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->predCb[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+\n\n");

    printf("[MB] =============" BLD_BLUE " PREDICTED Cr " CLR_RESET "============\n");
    for (y = 0; y < 8; y++)
    {
        if (y % 4 == 0)
            printf("+-------------------+-------------------+\n");

        for (x = 0; x < 8; x++)
        {
            if (x % 4 == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->predCr[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+\n\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Print final decoded coefficient for current macroblock.
 * \param *mb The current macroblock.
 */
void print_macroblock_pixel_final(Macroblock_t *mb)
{
#if ENABLE_DEBUG
    int blkSize = 4;
    int x = 0, y = 0;

    if (mb->MbPartPredMode[0] == Intra_8x8)
        blkSize = 8;

    printf("[MB] ==============" BLD_BLUE " FINAL Y " CLR_RESET "==============\n");
    for (y = 0; y < 16; y++)
    {
        if (y % blkSize == 0)
            printf("+-------------------+-------------------+-------------------+-------------------+\n");

        for (x = 0; x < 16; x++)
        {
            if (x % blkSize == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->SprimeL[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+-------------------+-------------------+\n\n");

    printf("[MB] ==============" BLD_BLUE " FINAL Cb " CLR_RESET "==============\n");
    for (y = 0; y < 8; y++)
    {
        if (y % 4 == 0)
            printf("+-------------------+-------------------+\n");

        for (x = 0; x < 8; x++)
        {
            if (x % 4 == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->SprimeCb[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+\n\n");

    printf("[MB] ==============" BLD_BLUE " FINAL Cr " CLR_RESET "==============\n");
    for (y = 0; y < 8; y++)
    {
        if (y % 4 == 0)
            printf("+-------------------+-------------------+\n");

        for (x = 0; x < 8; x++)
        {
            if (x % 4 == 0)
                printf("|");
            else
                printf(",");

            printf("%4i", mb->SprimeCr[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+-------------------+\n\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
