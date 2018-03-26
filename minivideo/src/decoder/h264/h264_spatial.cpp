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
 * \file      h264_spatial.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_spatial.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cmath>
#include <climits>

/* ************************************************************************** */

/*!
 * \brief Inverse Raster Scan.
 * \param a docme.
 * \param b docme.
 * \param c docme.
 * \param d docme.
 * \param e If e == 1, y position will be returned, x otherwise.
 * \return A macroblock location (x, y) depending of e (e==1, y is returned, x is returned otherwise).
 *
 * From 'ITU-T H.264' recommendation:
 * 5.7 Mathematical functions.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample
 * for the macroblock with address mbAddr relative to the upper-left sample of
 * the picture.
 */
int InverseRasterScan(const int a,
                      const int b,
                      const int c,
                      const int d,
                      const int e)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseRasterScan()" CLR_RESET);

    if (e == 1)
    {
        // return y
        return (a / (d / b)) * c;
    }
    else
    {
        // return x
        return (a % (d / b)) * b;
    }
}

/*!
 * \brief Fast Inverse Raster Scan.
 * \return A macroblock location (x, y) depending of e (e==1, y is returned, x is returned otherwise).
 */
inline int InverseRasterScan_x(const int a,
                               const int b,
                               const int c,
                               const int d)
{
    // return x
    return (a % (d / b)) * b;
}

/*!
 * \brief Fast Inverse Raster Scan.
 * \return A y macroblock location.
 */
inline int InverseRasterScan_y(const int a,
                               const int b,
                               const int c,
                               const int d)
{
    // return y
    return (a / (d / b)) * c;
}

/* ************************************************************************** */

/*!
 * \brief Inverse macroblock scanning process.
 * \param mbAddr A macroblock address.
 * \param MbaffFrameFlag Macroblock adaptative frame field flag, true if mbaff is enabled.
 * \param PicWidthInSamplesL docme.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.1 Inverse macroblock scanning process.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample
 * for the macroblock with address mbAddr relative to the upper-left sample of
 * the picture.
 *
 * Note that MbaffFrameFlag support has been removed from this function.
 */
void InverseMacroblockScan(const int mbAddr,
                           const bool MbaffFrameFlag,
                           const int PicWidthInSamplesL,
                           int *x,
                           int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseMacroblockScan()" CLR_RESET);

#if ENABLE_MBAFF
    TRACE_ERROR(TRANS, ">>> UNSUPPORTED (MbaffFrameFlag)")
    return;
#endif // ENABLE_MBAFF

    *x = InverseRasterScan_x(mbAddr, 16, 16, PicWidthInSamplesL);
    *y = InverseRasterScan_y(mbAddr, 16, 16, PicWidthInSamplesL);
}

/* ************************************************************************** */

/*!
 * \brief Inverse macroblock partition scanning process.
 * \param mbAddr A macroblock address.
 * \param mbPartIdx A macroblock partition index.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * From 'ITU-T H.264' recommendation:
 * - 6.4.2.1 Inverse macroblock partition scanning process.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample
 * for the macroblock partition mbPartIdx relative to the upper-left sample of
 * the macroblock.
 */
void InverseMacroblockPartitionScan(const int mbAddr, const int mbPartIdx, int *x, int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseMacroblockPartitionScan()" CLR_RESET);

    TRACE_WARNING(SPATIAL, ">>> UNIMPLEMENTED (InverseMacroblockPartitionScan)");
/*
    *x = InverseRasterScan_x(mbPartIdx, MbPartWidth(mb_type), MbPartHeight(mb_type), 16);
    *y = InverseRasterScan_y(mbPartIdx, MbPartWidth(mb_type), MbPartHeight(mb_type), 16);
*/
}

/* ************************************************************************** */

/*!
 * \brief Inverse sub-macroblock partition scanning process.
 * \param mbAddr A macroblock address.
 * \param mbPartIdx A macroblock partition index.
 * \param subMbPartIdx A sub-macroblock partition index.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.2.2 Inverse sub-macroblock partition scanning process.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample
 * for the sub-macroblock partition subMbPartIdx relative to the upper-left
 * sample of the sub-macroblock.
 */
void InverseSubMacroblockPartitionScan(const int mbAddr, const int mbPartIdx, const int subMbPartIdx, int *x, int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseSubMacroblockPartitionScan()" CLR_RESET);

    TRACE_WARNING(SPATIAL, ">>> UNIMPLEMENTED (InverseSubMacroblockPartitionScan)");
/*
    // If mb_type is equal to P_8x8, P_8x8ref0, or B_8x8
    *x = InverseRasterScan_x(subMbPartIdx, SubMbPartWidth(sub_mb_type[mbPartIdx]), SubMbPartHeight(sub_mb_type[mbPartIdx]), 8);
    *y = InverseRasterScan_y(subMbPartIdx, SubMbPartWidth(sub_mb_type[mbPartIdx]), SubMbPartHeight(sub_mb_type[mbPartIdx]), 8);

    // Otherwise (mb_type is not equal to P_8x8, P_8x8ref0, or B_8x8)
    *x = InverseRasterScan_x(subMbPartIdx, 4, 4, 8);
    *y = InverseRasterScan_y(subMbPartIdx, 4, 4, 8);
*/
}

/* ************************************************************************** */

/*!
 * \brief Inverse 4x4 luma block scanning process.
 * \param luma4x4BlkIdx The index of a 4x4 luma block.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.3 Inverse 4x4 luma block scanning process.
 * <!> 6.4.4 is the same function.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample for
 * the 4x4 luma block with index luma4x4BlkIdx relative to the upper-left luma
 * sample of the macroblock.
 */
void InverseLuma4x4BlkScan(const int luma4x4BlkIdx, int *x, int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseLuma4x4BlkScan()" CLR_RESET);
/*
    // Figure 6-10: Scan for 4x4 luma blocks
    -----------------
    | 0 | 1 | 4 | 5 |
    -----------------
    | 2 | 3 | 6 | 7 |
    -----------------
    | 8 | 9 | 12| 13|
    -----------------
    | 10| 11| 14| 15|
    -----------------
*/
    *x = InverseRasterScan_x(luma4x4BlkIdx / 4, 8, 8, 16) + InverseRasterScan_x(luma4x4BlkIdx % 4, 4, 4, 8);
    *y = InverseRasterScan_y(luma4x4BlkIdx / 4, 8, 8, 16) + InverseRasterScan_y(luma4x4BlkIdx % 4, 4, 4, 8);

    TRACE_2(SPATIAL, "   > xO : %2i", *x);
    TRACE_2(SPATIAL, "   > yO : %2i", *y);
}

/* ************************************************************************** */

/*!
 * \brief Inverse 8x8 luma block scanning process.
 * \param luma8x8BlkIdx The index of a 8x8 luma block.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.5 Inverse 8x8 luma block scanning process.
 * <!> 6.4.6 is the same function.
 *
 * Output of this process is the location (x, y) of the upper-left luma sample for
 * the 8x8 luma block with index luma8x8BlkIdx relative to the upper-left luma
 * sample of the macroblock.
 */
void InverseLuma8x8BlkScan(const int luma8x8BlkIdx, int *x, int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseLuma8x8BlkScan()" CLR_RESET);
/*
    // Figure 6-11: Scan for 8x8 luma blocks
    ---------
    | 0 | 1 |
    ---------
    | 2 | 3 |
    ---------
*/
    *x = InverseRasterScan_x(luma8x8BlkIdx, 8, 8, 16);
    *y = InverseRasterScan_y(luma8x8BlkIdx, 8, 8, 16);

    TRACE_2(SPATIAL, "   > xO : %2i", *x);
    TRACE_2(SPATIAL, "   > yO : %2i", *y);
}

/* ************************************************************************** */

/*!
 * \brief Inverse 4x4 chroma block scanning process.
 * \param chroma4x4BlkIdx The index of a 4x4 chroma block.
 * \param *x A horizontal luma sample location.
 * \param *y A vertical luma sample location.
 *
 * Output of this process is the location (x, y) of the upper-left chroma sample for
 * the 4x4 chroma block with index chroma4x4BlkIdx relative to the upper-left chroma
 * sample of the macroblock.
 */
void InverseChroma4x4BlkScan(const int chroma4x4BlkIdx, int *x, int *y)
{
    TRACE_1(SPATIAL, BLD_GREEN "   InverseChroma4x4BlkScan()" CLR_RESET);
/*
    // Figure x: Scan for 4x4 chroma blocks
    ---------
    | 0 | 1 |
    ---------
    | 2 | 3 |
    ---------
*/
    *x = InverseRasterScan_x(chroma4x4BlkIdx, 4, 4, 8);
    *y = InverseRasterScan_y(chroma4x4BlkIdx, 4, 4, 8);

    TRACE_2(SPATIAL, "   > xO : %2i", *x);
    TRACE_2(SPATIAL, "   > yO : %2i", *y);
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Derivation process of the availability for macroblock addresses.
 * \param *dc The current DecodingContext.
 * \param mbAddr A macroblock address.
 * \return true if macroblock is available at mbAddr, false otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.7 Derivation process of the availability for macroblock addresses.
 * Output of this process is the availability of the macroblock mbAddr.
 */
bool deriv_macroblock_availability(DecodingContext_t *dc, const int mbAddr)
{
    TRACE_1(SPATIAL, BLD_GREEN "deriv_macroblock_availability()" CLR_RESET);

    if (mbAddr < 0 || (unsigned int)mbAddr > dc->CurrMbAddr || dc->mb_array[mbAddr] == NULL)
    {
        return false;
    }

    return true;
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring macroblock addresses and their availability.
 * \param *dc The current DecodingContext.
 * \param *mbAddr The (current?) macroblock address.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.8 Derivation process for neighbouring macroblock addresses and their availability.
 *
 * <!> This process can only be invoked when MbaffFrameFlag is equal to 0.
 */
void deriv_macroblockneighbours_availability(DecodingContext_t *dc, const int mbAddr)
{
    TRACE_1(SPATIAL, BLD_GREEN "     deriv_macroblockneighbours_availability()" CLR_RESET);
/*
    ----------------------------------------
    |  mbAddr D  |  mbAddr B  |  mbAddr C  |
    ----------------------------------------
    |  mbAddr A  | CurrMbAddr |            |
    ----------------------------------------
*/
    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[mbAddr];
    int PicWidthInMbs = (int)sps->PicWidthInMbs;

    // Input to the process in subclause 6.4.7 is mbAddrA = CurrMbAddr - 1 and
    // the output is whether the macroblock mbAddrA is available.
    // In addition, mbAddrA is marked as not available when CurrMbAddr % PicWidthInMbs is equal to 0.
    if (mbAddr % PicWidthInMbs != 0)
    {
        mb->mbAddrA = mbAddr - 1;
    }
    else
    {
        mb->mbAddrA = -1;
    }

    // Input to the process in subclause 6.4.7 is mbAddrB = CurrMbAddr - PicWidthInMbs
    // and the output is whether the macroblock mbAddrB is available.
    if (mbAddr >= PicWidthInMbs)
    {
        mb->mbAddrB = mbAddr - PicWidthInMbs;
    }
    else
    {
        mb->mbAddrB = -1;
    }

    // Input to the process in subclause 6.4.7 is mbAddrC = CurrMbAddr - PicWidthInMbs + 1
    // and the output is whether the macroblock mbAddrC is available.
    // In addition, mbAddrC is marked as not available when (CurrMbAddr + 1) % PicWidthInMbs is equal to 0.
    if (mbAddr >= PicWidthInMbs &&
        (mbAddr + 1) % PicWidthInMbs != 0)
    {
        mb->mbAddrC = mbAddr - PicWidthInMbs + 1;
    }
    else
    {
        mb->mbAddrC = -1;
    }

    // Input to the process in subclause 6.4.7 is mbAddrD = CurrMbAddr - PicWidthInMbs - 1
    // and the output is whether the macroblock mbAddrD is available.
    // In addition, mbAddrD is marked as not available when CurrMbAddr % PicWidthInMbs is equal to 0.
    if (mbAddr > PicWidthInMbs &&
        mbAddr % PicWidthInMbs != 0)
    {
        mb->mbAddrD = mbAddr - PicWidthInMbs - 1;
    }
    else
    {
        mb->mbAddrD = -1;
    }

#if ENABLE_DEBUG
    // Check neighbors availability in memory
    if (mb->mbAddrA >= 0 && dc->mb_array[mb->mbAddrA] == NULL)
    {
        TRACE_ERROR(SPATIAL, "     - macroblock A (mbAddr %i) should be available", mb->mbAddrA);
    }
    if (mb->mbAddrB >= 0 && dc->mb_array[mb->mbAddrB] == NULL)
    {
        TRACE_ERROR(SPATIAL, "     - macroblock B (mbAddr %i) should be available", mb->mbAddrB);
    }
    if (mb->mbAddrC >= 0 && dc->mb_array[mb->mbAddrC] == NULL)
    {
        TRACE_ERROR(SPATIAL, "     - macroblock C (mbAddr %i) should be available", mb->mbAddrC);
    }
    if (mb->mbAddrD >= 0 && dc->mb_array[mb->mbAddrD] == NULL)
    {
        TRACE_ERROR(SPATIAL, "     - macroblock D (mbAddr %i) should be available", mb->mbAddrD);
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring macroblocks.
 * \param *dc The current DecodingContext.
 * \param *mbAddr The (current?) macroblock address.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.1 Derivation process for neighbouring macroblocks
 *
 * <!> This process can only be invoked when MbaffFrameFlag is equal to 0.
 */
void deriv_macroblockneighbours(DecodingContext_t *dc, const int mbAddr)
{
    TRACE_1(SPATIAL, BLD_GREEN "     deriv_macroblockneighbours()" CLR_RESET);
/*
    ----------------------------------------
    |  mbAddr D  |  mbAddr B  |  mbAddr C  |
    ----------------------------------------
    |  mbAddr A  | CurrMbAddr |            |
    ----------------------------------------
*/
    TRACE_WARNING(SPATIAL, ">>> UNIMPLEMENTED");
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring 8x8 luma blocks.
 * \param *dc The current decoding context.
 * \param luma8x8BlkIdx 8x8 luma block index.
 * \param *mbAddrA Either equal to CurrMbAddr or the address of the macroblock to the left of the current macroblock and its availability status.
 * \param *luma8x8BlkIdxA The index of the 8x8 luma block to the left of the 8x8 block with index luma8x8BlkIdx and its availability status.
 * \param *mbAddrB Either equal to CurrMbAddr or the address of the macroblock above the current macroblock and its availability status.
 * \param *luma8x8BlkIdxB The index of the 8x8 luma block above the 8x8 block with index luma8x8BlkIdx and its availability status.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.2 Derivation process for neighbouring 8x8 luma blocks.
 *
 * Input to this process is a 8x8 luma block index luma8x8BlkIdx.
 * Other parameters are output.
 */
void deriv_8x8lumablocks(DecodingContext_t *dc, const int luma8x8BlkIdx,
                         int *mbAddrA, int *luma8x8BlkIdxA,
                         int *mbAddrB, int *luma8x8BlkIdxB)
{
    TRACE_1(SPATIAL, BLD_GREEN "  deriv_8x8lumablocks()" CLR_RESET);
/*
    // Table 6-2: Specification of input and output assignments for subclauses 6.4.11.1 to 6.4.11.7
    N      xD       yD
    A      -1        0
    B       0       -1
    C predPartWidth -1
    D      -1       -1
*/
    //int x = 0, y = 0;
    //InverseLuma8x8BlkScan(luma8x8BlkIdx, &x, &y);

    int xA = (luma8x8BlkIdx % 2) * 8 - 1;
    int yA = (luma8x8BlkIdx / 2) * 8;

    int xB = (luma8x8BlkIdx % 2) * 8;
    int yB = (luma8x8BlkIdx / 2) * 8 - 1;

    int xW = 0, yW = 0;

    // luma8x8BlkIdxA derivation
    deriv_neighbouringlocations(dc, 1, xA, yA, mbAddrA, &xW, &yW);
    if (*mbAddrA > -1)
    {
        *luma8x8BlkIdxA = deriv_8x8lumablock_indices(xW, yW);
    }
    else
    {
        *luma8x8BlkIdxA = -1;
    }

    // luma8x8BlkIdxB derivation
    deriv_neighbouringlocations(dc, 1, xB, yB, mbAddrB, &xW, &yW);
    if (*mbAddrB > -1)
    {
        *luma8x8BlkIdxB = deriv_8x8lumablock_indices(xW, yW);
    }
    else
    {
        *luma8x8BlkIdxB = -1;
    }

    TRACE_2(SPATIAL, "  luma8x8BlkIdxA : %i", *luma8x8BlkIdxA);
    TRACE_2(SPATIAL, "  from mbAddrA   : %i", *mbAddrA);

    TRACE_2(SPATIAL, "  luma8x8BlkIdxB : %i", *luma8x8BlkIdxB);
    TRACE_2(SPATIAL, "  from mbAddrB   : %i", *mbAddrB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring 8x8 chroma blocks.
 * \param *dc The current decoding context.
 * \param chroma8x8BlkIdx 8x8 chroma block index.
 * \param *mbAddrA Either equal to CurrMbAddr or the address of the macroblock to the left of the current macroblock and its availability status.
 * \param *chroma8x8BlkIdxA The index of the 8x8 chroma block to the left of the 8x8 block with index chroma8x8BlkIdx and its availability status.
 * \param *mbAddrB Either equal to CurrMbAddr or the address of the macroblock above the current macroblock and its availability status.
 * \param *chroma8x8BlkIdxB The index of the 8x8 chroma block above the 8x8 block with index chroma8x8BlkIdx and its availability status.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.3 Derivation process for neighbouring 8x8 chroma blocks for ChromaArrayType equal to 3.
 *
 * This process is IDENTICAL to the derivation process for neighbouring 8x8 LUMA
 * block as specified in subclause 6.4.11.2.
 *
 * <!> This process is only invoked when ChromaArrayType is equal to 3.
 */
void deriv_8x8chromablocks_cat3(DecodingContext_t *dc, const int chroma8x8BlkIdx,
                                int *mbAddrA, int *chroma8x8BlkIdxA,
                                int *mbAddrB, int *chroma8x8BlkIdxB)
{
    TRACE_1(SPATIAL, BLD_GREEN "  deriv_8x8chromablocks()" CLR_RESET);

    deriv_8x8lumablocks(dc, chroma8x8BlkIdx, mbAddrA, chroma8x8BlkIdxA, mbAddrB, chroma8x8BlkIdxB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring 4x4 luma blocks.
 * \param *dc The current decoding context.
 * \param luma4x4BlkIdx 4x4 luma block index.
 * \param *mbAddrA Either equal to CurrMbAddr or the address of the macroblock to the left of the current macroblock and its availability status.
 * \param *luma4x4BlkIdxA The index of the 4x4 luma block to the left of the 4x4 block with index luma4x4BlkIdx and its availability status.
 * \param *mbAddrB Either equal to CurrMbAddr or the address of the macroblock above the current macroblock and its availability status.
 * \param *luma4x4BlkIdxB The index of the 4x4 luma block above the 4x4 block with index luma4x4BlkIdx and its availability status.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.4 Derivation process for neighbouring 4x4 luma blocks.
 *
 * Input to this process is a 4x4 luma block index luma4x4BlkIdx.
 * Other parameters are output.
 */
void deriv_4x4lumablocks(DecodingContext_t *dc, const int luma4x4BlkIdx,
                         int *mbAddrA, int *luma4x4BlkIdxA,
                         int *mbAddrB, int *luma4x4BlkIdxB)
{
    TRACE_1(SPATIAL, BLD_GREEN "  deriv_4x4lumablocks()" CLR_RESET);
/*
    // Table 6-2: Specification of input and output assignments for subclauses 6.4.11.1 to 6.4.11.7
    N      xD       yD
    A      -1        0
    B       0       -1
    C predPartWidth -1
    D      -1       -1
*/
    int x = 0, y = 0;
    InverseLuma4x4BlkScan(luma4x4BlkIdx, &x, &y);

    int xA = x - 1;
    int yA = y;

    int xB = x;
    int yB = y - 1;

    int xW = 0, yW = 0;

    // luma4x4BlkIdxA derivation
    deriv_neighbouringlocations(dc, 1, xA, yA, mbAddrA, &xW, &yW);
    if (*mbAddrA > -1)
    {
        *luma4x4BlkIdxA = deriv_4x4lumablock_indices(xW, yW);
    }
    else
    {
        *luma4x4BlkIdxA = -1;
    }

    // luma4x4BlkIdxB derivation
    deriv_neighbouringlocations(dc, 1, xB, yB, mbAddrB, &xW, &yW);
    if (*mbAddrB > -1)
    {
        *luma4x4BlkIdxB = deriv_4x4lumablock_indices(xW, yW);
    }
    else
    {
        *luma4x4BlkIdxB = -1;
    }

    TRACE_2(SPATIAL, "  luma4x4BlkIdxA : %i", *luma4x4BlkIdxA);
    TRACE_2(SPATIAL, "  from mbAddrA   : %i", *mbAddrA);

    TRACE_2(SPATIAL, "  luma4x4BlkIdxB : %i", *luma4x4BlkIdxB);
    TRACE_2(SPATIAL, "  from mbAddrB   : %i", *mbAddrB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring 4x4 chroma blocks.
 * \param *dc The current decoding context.
 * \param chroma4x4BlkIdx 4x4 chroma block index.
 * \param *mbAddrA Either equal to CurrMbAddr or the address of the macroblock to the left of the current macroblock and its availability status.
 * \param *chroma4x4BlkIdxA The index of the 4x4 chroma block to the left of the 4x4 block with index chroma4x4BlkIdx and its availability status.
 * \param *mbAddrB Either equal to CurrMbAddr or the address of the macroblock above the current macroblock and its availability status.
 * \param *chroma4x4BlkIdxB The index of the 4x4 chroma block above the 4x4 block with index chroma4x4BlkIdx and its availability status.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.5 Derivation process for neighbouring 4x4 chroma blocks.
 *
 * Input to this process is a 4x4 chroma block index chroma4x4BlkIdx.
 * Other parameters are output.
 *
 * <!> This process can only be invoked when ChromaArrayType is equals to 1 or 2.
 */
void deriv_4x4chromablocks(DecodingContext_t *dc, const int chroma4x4BlkIdx,
                           int *mbAddrA, int *chroma4x4BlkIdxA,
                           int *mbAddrB, int *chroma4x4BlkIdxB)
{
    TRACE_1(SPATIAL, BLD_GREEN "  deriv_4x4chromablocks()" CLR_RESET);
/*
    // Table 6-2: Specification of input and output assignments for subclauses 6.4.11.1 to 6.4.11.7
    N      xD       yD
    A      -1        0
    B       0       -1
    C predPartWidth -1
    D      -1       -1
*/
    int x = InverseRasterScan_x(chroma4x4BlkIdx, 4, 4, 8);
    int y = InverseRasterScan_y(chroma4x4BlkIdx, 4, 4, 8);

    int xA = x - 1;
    int yA = y;

    int xB = x;
    int yB = y - 1;

    int xW = 0, yW = 0;

    // chroma4x4BlkIdxA derivation
    deriv_neighbouringlocations(dc, 0, xA, yA, mbAddrA, &xW, &yW);
    if (*mbAddrA > -1)
    {
        *chroma4x4BlkIdxA = deriv_4x4chromablock_indices(xW, yW);
    }
    else
    {
        *chroma4x4BlkIdxA = -1;
    }

    // chroma4x4BlkIdxB derivation
    deriv_neighbouringlocations(dc, 0, xB, yB, mbAddrB, &xW, &yW);
    if (*mbAddrB > -1)
    {
        *chroma4x4BlkIdxB = deriv_4x4chromablock_indices(xW, yW);
    }
    else
    {
        *chroma4x4BlkIdxB = -1;
    }

    TRACE_2(SPATIAL, "  chroma4x4BlkIdxA : %i", *chroma4x4BlkIdxA);
    TRACE_2(SPATIAL, "  from mbAddrA     : %i", *mbAddrA);

    TRACE_2(SPATIAL, "  chroma4x4BlkIdxB : %i", *chroma4x4BlkIdxB);
    TRACE_2(SPATIAL, "  from mbAddrB     : %i", *mbAddrB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring 4x4 chroma blocks.
 * \param *dc The current decoding context.
 * \param chroma4x4BlkIdx 4x4 chroma (cb or cr) block index.
 * \param *mbAddrA Either equal to CurrMbAddr or the address of the macroblock to the left of the current macroblock and its availability status.
 * \param *chroma4x4BlkIdxA The index of a 4x4 chroma (cb or cr) block to the left of the 4x4 block with index chroma4x4BlkIdx and its availability status.
 * \param *mbAddrB Either equal to CurrMbAddr or the address of the macroblock above the current macroblock and its availability status.
 * \param *chroma4x4BlkIdxB The index of a 4x4 chroma (cb or cr) block above the 4x4 block with index chroma4x4BlkIdx and its availability status.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11.6 Derivation process for neighbouring 4x4 chroma blocks for ChromaArrayType equal to 3.
 *
 * This process is IDENTICAL to the derivation process for neighbouring 4x4 LUMA
 * block as specified in subclause 6.4.11.4.
 *
 * <!> ChromaArrayType must be equal to 3 in order to call this function.
 */
void deriv_4x4chromablocks_cat3(DecodingContext_t *dc, const int chroma4x4BlkIdx,
                                int *mbAddrA, int *chroma4x4BlkIdxA,
                                int *mbAddrB, int *chroma4x4BlkIdxB)
{
    TRACE_1(SPATIAL, BLD_GREEN "  deriv_4x4chromablocks_cat3()" CLR_RESET);

    deriv_4x4lumablocks(dc, chroma4x4BlkIdx, mbAddrA, chroma4x4BlkIdxA, mbAddrB, chroma4x4BlkIdxB);
}

/* ************************************************************************** */

// 6.4.11.7 Derivation process for neighbouring partitions
// Unused because macroblock partitions are not supported

/* ************************************************************************** */

/*!
 * \brief Derivation process for neighbouring locations.
 * \param *dc The current decoding context.
 * \param lumaBlock true if this is a luma block, false otherwise.
 * \param xN The location (xN, yN) expressed relative to the upper left corner of the current macroblock.
 * \param yN The location (xN, yN) expressed relative to the upper left corner of the current macroblock.
 * \param *mbAddrN The address of neighbouring macroblock that contains (xN, yN).
 * \param *xW The location (xW, yW) expressed relative to the upper-left corner of the macroblock mbAddrN.
 * \param *yW The location (xW, yW) expressed relative to the upper-left corner of the macroblock mbAddrN.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.11 Derivation process for neighbouring locations.
 *
 * - Outputs of this process are:
 * mbAddrN: either equal to CurrMbAddr or to the address of neighbouring macroblock
 * that contains (xN, yN) and its availability status.
 * (xW, yW): the location (xN, yN) expressed relative to the upper-left corner
 * of the macroblock mbAddrN (rather than relative to the upper-left corner of
 * the current macroblock).
 */
void deriv_neighbouringlocations(DecodingContext_t *dc, const bool lumaBlock,
                                 const int xN, const int yN, int *mbAddrN, int *xW, int *yW)
{
    TRACE_1(SPATIAL, BLD_GREEN "   deriv_neighbouringlocations()" CLR_RESET);

    int maxW = 16;
    int maxH = 16;

    if (lumaBlock == false)
    {
        maxW = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id]->MbWidthC;
        maxH = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id]->MbHeightC;
    }

    // Checking neighbour's availability with neighbourmacroblock_availability()
    // Now use info contained in each macroblock, to avoid useless computation

    // Specification of mbAddrN (Table 6-3 with optimizations)
    if (yN > -1)
    {
        if (xN < 0)
            *mbAddrN = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
        else if (xN < maxW)
            *mbAddrN = dc->CurrMbAddr;
        else
            return;
    }
    else
    {
        if (xN < 0)
            *mbAddrN = dc->mb_array[dc->CurrMbAddr]->mbAddrD;
        else if (xN < maxW)
            *mbAddrN = dc->mb_array[dc->CurrMbAddr]->mbAddrB;
        else
            *mbAddrN = dc->mb_array[dc->CurrMbAddr]->mbAddrC;
    }

    // The neighbouring location (xW, yW) relative to the upper-left corner of mbAddrN is derived as:
    *xW = (xN + maxW) % maxW;
    *yW = (yN + maxH) % maxH;

    // Print
    TRACE_2(SPATIAL, "   xN      : %i", xN);
    TRACE_2(SPATIAL, "   yN      : %i", yN);
    TRACE_2(SPATIAL, "   xW      : %i", *xW);
    TRACE_2(SPATIAL, "   yW      : %i", *yW);
    TRACE_2(SPATIAL, "   mbAddrN : %i", *mbAddrN);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for 4x4 luma block indices.
 * \param xP luma location relative to the upper-left luma sample of a macroblock.
 * \param yP luma location relative to the upper-left luma sample of a macroblock.
 * \return A 4x4 luma block index luma4x4BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.12.1 Derivation process for 4x4 luma block indices.
 */
inline int deriv_4x4lumablock_indices(const int xP, const int yP)
{
    return (int)(8 * (yP / 8) + 4 * (xP / 8) + 2 * ((yP % 8) / 4) + ((xP % 8) / 4));
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for 4x4 chroma block indices.
 * \param xP chroma location relative to the upper-left chroma sample of a macroblock.
 * \param yP chroma location relative to the upper-left chroma sample of a macroblock.
 * \return A 4x4 chroma block index chroma4x4BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.12.2 Derivation process for 4x4 chroma block indices.
 *
 * This subclause is only invoked when ChromaArrayType is equal to 1 or 2.
 * Be carefull, the equation used in this function is not the same as the one
 * described in the specification.
 */
inline int deriv_4x4chromablock_indices(const int xP, const int yP)
{
    //FIXME why this is not the same equation as in the 'ITU-T H.264' recommendation?
    return (int)(2.0*floor((yP / 8.0) + 0.5) + floor((xP / 8.0) + 0.5));
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for 8x8 luma block indices.
 * \param xP luma location relative to the upper-left luma sample of a macroblock.
 * \param yP luma location relative to the upper-left luma sample of a macroblock.
 * \return A 8x8luma block index luma8x8BlkIdx.
 *
 * From 'ITU-T H.264' recommendation:
 * 6.4.12.3 Derivation process for 8x8 luma block indices.
 */
inline int deriv_8x8lumablock_indices(const int xP, const int yP)
{
    return (int)(2*(yP / 8) + (xP / 8));
}

/* ************************************************************************** */
