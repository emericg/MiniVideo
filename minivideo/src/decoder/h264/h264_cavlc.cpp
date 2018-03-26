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
 * \file      h264_cavlc.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#include "h264_cavlc.h"
#include "h264_cavlc_tables.h"

// minivideo headers
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "h264_spatial.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>

// C++ standard libraries
#include <algorithm>

/* ************************************************************************** */
/*
    // CAVLC overview

    http://en.wikipedia.org/wiki/CAVLC

    CAVLC = Context Adaptative Variable Length Coding
*/
/* ************************************************************************** */

static int read_ce_coefftoken(DecodingContext_t *dc, const int nC, int *TotalCoeffs, int *TrailingOnes);
static int read_ce_levelprefix(DecodingContext_t *dc);
static int read_ce_totalzeros(DecodingContext_t *dc, const int vlcnum, const int chromadc);
static int read_ce_runbefore(DecodingContext_t *dc, const int vlcnum);

static int code_from_bitstream_2d(DecodingContext_t *dc,
                                  const uint8_t *lentab, const uint8_t *codtab,
                                  const int tabwidth, const int tabheight,
                                  int *code);

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *coeffLevel The table of coefficients to fill.
 * \param startIdx docme.
 * \param endIdx docme.
 * \param maxNumCoeff The maximum number of coefficients to decode.
 * \param blkType The type of block to decode.
 * \param blkIdx The id of the current block.
 *
 * This process is invoked when parsing syntax elements with descriptor equal to ce(v)
 * in subclause 7.3.5.3.2 and when entropy_coding_mode_flag is equal to 0.
 *
 * Inputs to this process are bits from slice data, a maximum number of non-zero
 * transform coefficient levels maxNumCoeff, the luma block index luma4x4BlkIdx
 * or the chroma block index chroma4x4BlkIdx, cb4x4BlkIdx or cr4x4BlkIdx of the
 * current block of transform coefficient levels.
 *
 * Output of this process is the list coeffLevel containing transform coefficient
 * levels of the luma block with block index luma4x4BlkIdx or the chroma block
 * with block index chroma4x4BlkIdx, cb4x4BlkIdx or cr4x4BlkIdx.
 */
void residual_block_cavlc(DecodingContext_t *dc, int *coeffLevel, const int startIdx, const int endIdx, const int maxNumCoeff, const int blkType, const int blkIdx)
{
    TRACE_1(CAVLC, "> " BLD_GREEN "residual_block_cavlc()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int mbAddrA_temp = -1, mbAddrB_temp = -1;
    int blkA = 0, blkB = 0;
    int nA = 0, nB = 0, nC = 0; // Number of coefficients (TotalCoeff) in neighboring blocks
    int i = 0;

    // 9.2.1 Parsing process for total number of transform coefficient levels and trailing ones
    if (blkType == blk_CHROMA_DC_Cb || blkType == blk_CHROMA_DC_Cr)
    {
        if (dc->ChromaArrayType == 1)
            nC = -1;
        else // if (dc->ChromaArrayType == 2)
            nC = -2;
    }
    else
    {
        // 4
        if (blkType == blk_LUMA_16x16_DC || blkType == blk_LUMA_16x16_AC || blkType == blk_LUMA_4x4 || blkType == blk_LUMA_8x8)
        {
            deriv_4x4lumablocks(dc, blkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);
        }
/*
        else if (blkType == CAVLC_CB_INTRA16x16DC || blkType == CAVLC_CB_INTRA16x16AC || blkType == CAVLC_CB)
        {
            deriv_4x4chromablocks_cat3(dc, blkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);
        }
        else if (blkType == CAVLC_CR_INTRA16x16DC || blkType == CAVLC_CR_INTRA16x16AC || blkType == CAVLC_CR)
        {
            deriv_4x4chromablocks_cat3(dc, blkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);
        }
*/
        else if (blkType == blk_CHROMA_AC_Cb || blkType == blk_CHROMA_AC_Cr)
        {
            deriv_4x4chromablocks(dc, blkIdx, &mbAddrA_temp, &blkA, &mbAddrB_temp, &blkB);
        }

        TRACE_2(CAVLC, "  > blkA : %i", blkA);
        TRACE_2(CAVLC, "  > blkB : %i", blkB);
        TRACE_2(CAVLC, "  > mbAddrA_temp : %i", mbAddrA_temp);
        TRACE_2(CAVLC, "  > mbAddrB_temp : %i", mbAddrB_temp);

        if (mbAddrA_temp != -1) // 5 6 - A
        {
            if (((dc->active_slice->slice_type == 0 || dc->active_slice->slice_type == 5) && dc->mb_array[mbAddrA_temp]->mb_type == P_Skip) ||
                ((dc->active_slice->slice_type == 1 || dc->active_slice->slice_type == 6) && dc->mb_array[mbAddrA_temp]->mb_type == B_Skip))
            {
                nA = 0;
            }
            else if ((dc->active_slice->slice_type == 2 || dc->active_slice->slice_type == 7) && dc->mb_array[mbAddrA_temp]->mb_type == I_PCM)
            {
                nA = 16;
            }
            else
            {
                if (blkType < 4)
                    nA = dc->mb_array[mbAddrA_temp]->TotalCoeffs_luma[blkA];
                else if (blkType == blk_CHROMA_AC_Cb)
                    nA = dc->mb_array[mbAddrA_temp]->TotalCoeffs_chroma[0][blkA];
                else if (blkType == blk_CHROMA_AC_Cr)
                    nA = dc->mb_array[mbAddrA_temp]->TotalCoeffs_chroma[1][blkA];
                else
                {
                    TRACE_WARNING(CAVLC, ">>> UNIMPLEMENTED (blkType > 5)");
                }
            }

            TRACE_2(CAVLC, "  > nA : %i", nA);
        }

        if (mbAddrB_temp != -1) // 5 6 - B
        {
            if (((dc->active_slice->slice_type == 0 || dc->active_slice->slice_type == 5) && dc->mb_array[mbAddrA_temp]->mb_type == P_Skip) ||
                ((dc->active_slice->slice_type == 1 || dc->active_slice->slice_type == 6) && dc->mb_array[mbAddrA_temp]->mb_type == B_Skip))
            {
                nB = 0;
            }
            else if ((dc->active_slice->slice_type == 2 || dc->active_slice->slice_type == 7) && dc->mb_array[mbAddrB_temp]->mb_type == I_PCM)
            {
                nB = 16;
            }
            else
            {
                if (blkType < 4)
                    nB = dc->mb_array[mbAddrB_temp]->TotalCoeffs_luma[blkB];
                else if (blkType  == blk_CHROMA_AC_Cb)
                    nB = dc->mb_array[mbAddrB_temp]->TotalCoeffs_chroma[0][blkB];
                else if (blkType  == blk_CHROMA_AC_Cr)
                    nB = dc->mb_array[mbAddrB_temp]->TotalCoeffs_chroma[1][blkB];
                else
                {
                    TRACE_WARNING(CAVLC, ">>> UNIMPLEMENTED (blkType > 5)");
                }
            }

            TRACE_2(CAVLC, "  > nB : %i", nB);
        }

        // 7
        if (mbAddrA_temp != -1 && mbAddrB_temp != -1)
            nC = (nA + nB + 1) >> 1;
        else if (mbAddrA_temp != -1 && mbAddrB_temp == -1)
            nC = nA;
        else if (mbAddrA_temp == -1 && mbAddrB_temp != -1)
            nC = nB;
        else
            nC = 0;
    }

    // Print nC
    TRACE_1(CAVLC, "  > nC : %i", nC);

    int TotalCoeffs = 0, TrailingOnes = 0;
    int CoeffToken = read_ce_coefftoken(dc, nC, &TotalCoeffs, &TrailingOnes);

    if (CoeffToken == -1)
    {
        TRACE_WARNING(CAVLC, "Fatal error: could not compute CoeffToken!");
        exit(EXIT_FAILURE);
    }

    if (blkType < 4)
        mb->TotalCoeffs_luma[blkIdx] = TotalCoeffs;
    else if (blkType == blk_CHROMA_AC_Cb)
        mb->TotalCoeffs_chroma[0][blkIdx] = TotalCoeffs;
    else if (blkType == blk_CHROMA_AC_Cr)
        mb->TotalCoeffs_chroma[1][blkIdx] = TotalCoeffs;
    else
    {
        //TRACE_WARNING(CAVLC, "Could not save TotalCoeffs!");
    }

    if (TotalCoeffs > 0 && TotalCoeffs <= maxNumCoeff)
    {
        // 9.2.2 Parsing process for level information
        int level[16]; // maximum value for TotalCoeffs
        int run[16]; // maximum value for TotalCoeffs
        int suffixLength = 0;

        if (TotalCoeffs > 10 && TrailingOnes < 3)
        {
            suffixLength = 1;
        }

        for (i = 0; i < TotalCoeffs; i++)
        {
            if (i < TrailingOnes)
            {
                bool trailing_ones_sign_flag = read_bit(dc->bitstr);
                level[i] = 1 - 2 * trailing_ones_sign_flag;

                TRACE_3(CAVLC, "trailing_ones_sign_flag = %i", trailing_ones_sign_flag);
                TRACE_3(CAVLC, "level[%i]               : %i", i, level[i]);
            }
            else
            {
                int level_prefix = read_ce_levelprefix(dc);
                int levelCode = std::min(15, level_prefix) << suffixLength;

                if (suffixLength > 0 || level_prefix >= 14)
                {
                    int levelSuffixSize = suffixLength;

                    if (level_prefix == 14 && suffixLength == 0)
                    {
                        levelSuffixSize = 4;
                    }
                    else if (level_prefix > 14)
                    {
                        levelSuffixSize = level_prefix - 3;
                    }

                    int level_suffix = read_bits(dc->bitstr, levelSuffixSize);
                    levelCode += level_suffix;

                    TRACE_3(CAVLC, "> level_suffix  : %i", level_suffix);
                    TRACE_3(CAVLC, "levelSuffixSize : %i", levelSuffixSize);
                    TRACE_3(CAVLC, "levelCode       : %i", levelCode);
                }

                if (level_prefix >= 15 && suffixLength == 0)
                    levelCode += 15;

                if (level_prefix >= 16)
                    levelCode += (1 << (level_prefix - 3)) - 4096;

                if (i == TrailingOnes && TrailingOnes < 3)
                    levelCode += 2;

                if (levelCode % 2 == 0)
                    level[i] = (levelCode + 2) >> 1;
                else
                    level[i] = (-levelCode - 1) >> 1;

                if (suffixLength == 0)
                {
                    suffixLength = 1;
                }

                {
                    int vtest = (3 << (suffixLength - 1));
                    if ((abs(level[i]) > vtest) && (suffixLength < 6))
                    {
                        suffixLength++;
                    }
                }

                TRACE_3(CAVLC, "suffixLength = %i", suffixLength);
                TRACE_3(CAVLC, "levelCode    : %i", levelCode);
                TRACE_3(CAVLC, "level[%i]    : %i", i, level[i]);
            }
        }

        // 9.2.3 Parsing process for run information
        int zerosLeft = 0;
        if (TotalCoeffs < endIdx - startIdx + 1)
        {
            int vlcnum = TotalCoeffs - 1;
            int total_zeros = 0;

            if (blkType != blk_CHROMA_DC_Cb && blkType != blk_CHROMA_DC_Cr)
                total_zeros = read_ce_totalzeros(dc, vlcnum, 0);
            else
                total_zeros = read_ce_totalzeros(dc, vlcnum, 1);

            zerosLeft = total_zeros;
        }
        else
        {
            zerosLeft = 0;
        }

        for (i = 0; i < TotalCoeffs - 1; i++)
        {
            if (zerosLeft > 0)
            {
                int vlcnum = imin(zerosLeft - 1, RUNBEFORE_NUM_M1);
                int run_before = read_ce_runbefore(dc, vlcnum);
                run[i] = run_before;
            }
            else
            {
                run[i] = 0;
            }

            zerosLeft -= run[i];
        }

        // Decoded coefficients :
        run[TotalCoeffs - 1] = zerosLeft;
        int coeffNum = -1;
        for (i = TotalCoeffs - 1; i >= 0; i--)
        {
            coeffNum += run[i] + 1;
            coeffLevel[startIdx + coeffNum] = level[i];
            TRACE_1(CAVLC, "# " BLD_PURPLE "coeffLevel[%i]" CLR_RESET " = %i", startIdx + coeffNum, coeffLevel[startIdx + coeffNum]);
        }
    }

    TRACE_2(CAVLC, "---- residual_block_cavlc - the end");
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parsing process for coeff_token.
 * \param *dc The current DecodingContext.
 * \param nC The number of coefficient in the previous block.
 * \param *TotalCoeffs The number of coefficients in the current block.
 * \param *TrailingOnes The number of trailing one's in the current block, in zig zag scan order.
 * \return coeff_token.
 *
 * coeff_token specifies the total number of non-zero transform coefficient levels
 * and the number of trailing one transform coefficient levels in a transform
 * coefficient level scan.
 * The range of coeff_token is specified in subclause 9.2.1.

 * A trailing one transform coefficient level is one of up to three consecutive
 * non-zero transform coefficient levels having an absolute value equal to 1 at
 * the end of a scan of non-zero transform coefficient levels.
 */
int read_ce_coefftoken(DecodingContext_t *dc, const int nC, int *TotalCoeffs, int *TrailingOnes)
{
    TRACE_3(CAVLC, BLD_GREEN "read_ce_coefftoken()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    // VLC decoding
    ////////////////////////////////////////////////////////////////////////////

    // Value of coeff_token will not be 100% accurate after reading: need to add 4, 8 or 16, but we do not use it so it's ok.
    unsigned int coeff_token = 0;
    unsigned int b = 0;
    int leadingZeroBits = -1;

    if (nC > 16 || nC < -2)
    {
        TRACE_ERROR(CAVLC, "nC is %i but must be in range [-2;16]", nC);
        return -1;
    }
    else if (nC > 7) // nC == [8,16]
    {
        // Count leadingZeroBits
        for (b = 0; !b && leadingZeroBits < 6; leadingZeroBits++)
            b = read_bit(dc->bitstr);

        // Coeff token have a fixed size (6bits) when nC == [8,16]
        if ((5 - leadingZeroBits) < 0)
            rewind_bits(dc->bitstr, 1);
        else if ((5 - leadingZeroBits) > 0)
            coeff_token = read_bits(dc->bitstr, 5 - leadingZeroBits);

        b = 0;
        while (coeff_token != coefftoken_table[3][leadingZeroBits][b][0] && b < 32)
            b++;

        *TotalCoeffs = coefftoken_table[3][leadingZeroBits][b][1];
        *TrailingOnes = coefftoken_table[3][leadingZeroBits][b][2];
    }
    else if (nC > 3) // nC == [4,8[
    {
        // Count leadingZeroBits
        for (b = 0; !b; leadingZeroBits++)
            b = read_bit(dc->bitstr);

        // Deduce coeff_token value
        if (leadingZeroBits == 9)
        {
            *TotalCoeffs = 16;
            *TrailingOnes = 0;
        }
        else if (leadingZeroBits == 6 && next_bits(dc->bitstr, 2) == 3)
        {
            skip_bits(dc->bitstr, 2);
            *TotalCoeffs = 13;
            *TrailingOnes = 1;
        }
        else
        {
            // Compute coeff_token size
            int n = 3;
            if (leadingZeroBits == 8)
                n = 1;
            else if (leadingZeroBits == 7)
                n = 2;

            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, n);
            b = 0;

            while (coeff_token != coefftoken_table[2][leadingZeroBits][b][0] && b < 8)
                b++;

            *TotalCoeffs = coefftoken_table[2][leadingZeroBits][b][1];
            *TrailingOnes = coefftoken_table[2][leadingZeroBits][b][2];
        }
    }
    else if (nC > 1) // nC == [2,4[
    {
        // Count leadingZeroBits
        for (b = 0; !b; leadingZeroBits++)
            b = read_bit(dc->bitstr);

        // Find coeff_token value
        int nb = next_bit(dc->bitstr);

        if (leadingZeroBits == 12)
        {
            *TotalCoeffs = 15;
            *TrailingOnes = 3;
        }
        else if (leadingZeroBits == 1 && nb == 1)
        {
            skip_bits(dc->bitstr, 1);
            *TotalCoeffs = 2;
            *TrailingOnes = 2;
        }
        else if (leadingZeroBits == 2 && nb == 1)
        {
            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, 2);

            if (coeff_token == 2)
            {
                *TotalCoeffs = 5;
                *TrailingOnes = 3;
            }
            else
            {
                *TotalCoeffs = 2;
                *TrailingOnes = 1;
            }
        }
        else if (leadingZeroBits == 10 && nb == 1)
        {
            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, 2);

            if (coeff_token == 2)
            {
                *TotalCoeffs = 14;
                *TrailingOnes = 2;
            }
            else
            {
                *TotalCoeffs = 14;
                *TrailingOnes = 0;
            }
        }
        else
        {
            // Compute coeff_token size
            int n = 2;
            if (leadingZeroBits == 0)
                n = 1;
            else if (leadingZeroBits == 2 || (leadingZeroBits > 6 && leadingZeroBits < 11))
                n = 3;

            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, n);
            b = 0;

            while (coeff_token != coefftoken_table[1][leadingZeroBits][b][0] && b < 8)
                b++;

            *TotalCoeffs = coefftoken_table[1][leadingZeroBits][b][1];
            *TrailingOnes = coefftoken_table[1][leadingZeroBits][b][2];
        }
    }
    else if (nC > -1) // nC == [0,2[
    {
        // Count leadingZeroBits
        for (b = 0; !b; leadingZeroBits++)
            b = read_bit(dc->bitstr);

        // Find coeff_token value
        int nb = next_bit(dc->bitstr);

        if (leadingZeroBits < 3)
        {
            *TotalCoeffs = *TrailingOnes = leadingZeroBits;
        }
        else if (leadingZeroBits == 3 && nb == 1)
        {
            skip_bits(dc->bitstr, 1);
            *TotalCoeffs = 3;
            *TrailingOnes = 3;
        }
        else if (leadingZeroBits == 4 && nb == 1)
        {
            skip_bits(dc->bitstr, 1);
            *TotalCoeffs = 4;
            *TrailingOnes = 3;
        }
        else if (leadingZeroBits == 14)
        {
            *TotalCoeffs = 13;
            *TrailingOnes = 1;
        }
        else
        {
            // Compute coeff_token size
            int n = 2;
            if (leadingZeroBits > 8 && leadingZeroBits < 13)
                n = 3;

            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, n);
            b = 0;

            while (coeff_token != coefftoken_table[0][leadingZeroBits][b][0] && b < 8)
                b++;

            *TotalCoeffs = coefftoken_table[0][leadingZeroBits][b][1];
            *TrailingOnes = coefftoken_table[0][leadingZeroBits][b][2];
        }
    }
    else if (nC == -1)
    {
        if (next_bits(dc->bitstr, 7) == 0)
        {
            skip_bits(dc->bitstr, 7);
            *TotalCoeffs = 4;
            *TrailingOnes = 3;
        }
        else
        {
            // Count leadingZeroBits
            for (b = 0; !b; leadingZeroBits++)
                b = read_bit(dc->bitstr);

            // Find coeff_token value
            if (leadingZeroBits < 3)
            {
                if (leadingZeroBits == 2)
                    *TotalCoeffs = *TrailingOnes = 2;
                else if  (leadingZeroBits == 1)
                    *TotalCoeffs = *TrailingOnes = 0;
                else
                    *TotalCoeffs = *TrailingOnes = 1;
            }
            else
            {
                // Compute coeff_token size
                int n = 1;
                if (leadingZeroBits == 3)
                    n = 2;

                // Read coeff_token value
                coeff_token = read_bits(dc->bitstr, n);
                b = 0;

                while (coeff_token != coefftoken_table[4][leadingZeroBits][b][0] && b < 4)
                    b++;

                *TotalCoeffs = coefftoken_table[4][leadingZeroBits][b][1];
                *TrailingOnes = coefftoken_table[4][leadingZeroBits][b][2];
            }
        }
    }
    else if (nC == -2)
    {
        // Count leadingZeroBits
        for (b = 0; !b; leadingZeroBits++)
            b = read_bit(dc->bitstr);

        // Find coeff_token value
        if (leadingZeroBits < 3)
        {
            *TotalCoeffs = *TrailingOnes = leadingZeroBits;
        }
        else if (leadingZeroBits == 4)
        {
            *TotalCoeffs = 3;
            *TrailingOnes = 3;
        }
        else if (leadingZeroBits == 5)
        {
            *TotalCoeffs = 4;
            *TrailingOnes = 3;
        }
        else
        {
            // Compute coeff_token size
            int n = 2;
            if (leadingZeroBits == 3)
                n = 3;

            // Read coeff_token value
            coeff_token = read_bits(dc->bitstr, n);
            b = 0;

            while (coeff_token != coefftoken_table[5][leadingZeroBits][b][0] && b < 8)
                b++;

            *TotalCoeffs = coefftoken_table[5][leadingZeroBits][b][1];
            *TrailingOnes = coefftoken_table[5][leadingZeroBits][b][2];
        }
    }

    // Print & return results
    ////////////////////////////////////////////////////////////////////////////

    TRACE_1(CAVLC, "> coeff_token found : %i (lzb=%i, b=%i)", coeff_token, leadingZeroBits, b);
    TRACE_1(CAVLC, "> TotalCoeff        : %i", *TotalCoeffs);
    TRACE_1(CAVLC, "> TrailingOnes      : %i", *TrailingOnes);

    return coeff_token;
}

/* ************************************************************************** */

/*!
 * \brief Parsing process for level_prefix.
 * \param *dc The current DecodingContext.
 * \return leadingZeroBits.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.2.2.1 Parsing process for level_prefix
 *
 * The parsing process for this syntax element consists in reading the bits
 * starting at the current location in the bitstream up to and including the
 * first non-zero bit, and counting the number of leading bits that are equal to 0.
 *
 * level_prefix and level_suffix specify the value of a non-zero transform coefficient level.
 * The range of level_prefix and level_suffix is specified in subclause 9.2.2.
 */
int read_ce_levelprefix(DecodingContext_t *dc)
{
    TRACE_3(CAVLC, BLD_GREEN "read_ce_levelprefix()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    int leadingZeroBits = -1;

    int b = 0;
    for (b = 0; !b; leadingZeroBits++)
    {
        b = read_bit(dc->bitstr);
    }

    // Return level_prefix syntax element value
    TRACE_3(CAVLC, "> level_prefix: %i", leadingZeroBits);
    return leadingZeroBits;
}

/* ************************************************************************** */

/*!
 * \brief Parsing process for total_zeros.
 * \param *dc The current DecodingContext.
 * \param vlcnum docme.
 * \param chromadc Set to 1 if we are decoding chroma dc coefficient, 0 otherwise.
 * \return total_zeros.
 *
 * total_zeros specifies the total number of zero-valued transform coefficient
 * levels that are located before the position of the last non-zero transform
 * coefficient level in a scan of transform coefficient levels.
 * The range of total_zeros is specified in subclause 9.2.3.
 */
int read_ce_totalzeros(DecodingContext_t *dc, const int vlcnum, const int chromadc)
{
    TRACE_3(CAVLC, BLD_GREEN "read_ce_totalzeros()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    int total_zeros = 0;
    int retval = 0;

    if (!chromadc)
        retval = code_from_bitstream_2d(dc, &totalzeros_lentab[vlcnum][0], &totalzeros_codtab[vlcnum][0], 16, 1, &total_zeros);
    else
        retval = code_from_bitstream_2d(dc, &totalzeros_chromadc_lentab[0][vlcnum][0], &totalzeros_chromadc_codtab[0][vlcnum][0], 4, 1, &total_zeros);

    if (retval)
    {
        TRACE_ERROR(CAVLC, "> Error wile decoding total_zeros!");
        total_zeros = -1;
    }

    // Return total_zeros syntax element value
    TRACE_3(CAVLC, "> total_zeros: %i", total_zeros);
    return total_zeros;
}

/* ************************************************************************** */

/*!
 * \brief Parsing process for run_before.
 * \param *dc The current DecodingContext.
 * \param vlcnum docme.
 * \return run_before.
 *
 * run_before specifies the number of consecutive transform coefficient levels in
 * the scan with zero value before a non-zero valued transform coefficient level.
 * The range of run_before is specified in subclause 9.2.3.
 */
int read_ce_runbefore(DecodingContext_t *dc, const int vlcnum)
{
    TRACE_3(CAVLC, BLD_GREEN "read_ce_runbefore()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    int run_before = 0;
    int retval = code_from_bitstream_2d(dc, &runbefore_lentab[vlcnum][0], &runbefore_codtab[vlcnum][0], 16, 1, &run_before);

    if (retval)
    {
        TRACE_ERROR(CAVLC, "> Error wile decoding run_before!");
        run_before = -1;
    }

    // Return run_before syntax element value
    TRACE_3(CAVLC, "> run_before: %i", run_before);
    return run_before;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief code from bitstream (2d tables).
 * \param *dc The current DecodingContext.
 * \param *lentab The table containing code lenght.
 * \param *codtab The table containing code value.
 * \param tabwidth The with of the tables.
 * \param tabheight The height of the tables.
 * \param *code The syntax element value, returned by pointer.
 * \return -1 if failed to find code.
 *
 * Original version of this vlc decoding function can be found in JM 17.1 - vlc.c.
 */
static int code_from_bitstream_2d(DecodingContext_t *dc,
                                  const uint8_t *lentab, const uint8_t *codtab,
                                  const int tabwidth, const int tabheight,
                                  int *code)
{
    //TRACE_3(CAVLC, BLD_GREEN "code_from_bitstream_2d()" CLR_RESET);

    const uint8_t *len = &lentab[0];
    const uint8_t *cod = &codtab[0];
    int retcode = -1;

    int i, j;
    for (j = 0; j < tabheight; j++)
    {
        for (i = 0; i < tabwidth; i++)
        {
/*
            TRACE_3(CAVLC, " > current cod : %i", *cod);
            TRACE_3(CAVLC, " > current len : %i", *len);
            TRACE_3(CAVLC, " > current bits(%i) : %i", *len, next_bits(dc->bitstr, *len));
*/
            if ((*len == 0) || (next_bits(dc->bitstr, *len) != *cod))
            {
                ++len;
                ++cod;
            }
            else
            {
                // Move bitstream pointer
                skip_bits(dc->bitstr, *len);

                // The syntax element value
                *code =  i;
/*
                TRACE_2(CAVLC, " > [B2D] code : %i", *code);
                TRACE_2(CAVLC, " > [B2D]  len : %i", *len);
                TRACE_2(CAVLC, " > [B2D]    i : %i", i);
                TRACE_2(CAVLC, " > [B2D]    j : %i", j);
*/
                return 0;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
