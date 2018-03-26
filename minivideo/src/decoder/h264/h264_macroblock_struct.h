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
 * \file      h264_macroblock_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_MACROBLOCK_STRUCT_H
#define H264_MACROBLOCK_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"

/* ************************************************************************** */

//! Luma prediction modes
typedef enum LumaPredMode_e
{
    NoPrediction      = 0,
    Intra_4x4         = 1, // I frame ↓
    Intra_8x8         = 2,
    Intra_16x16       = 3,
    Direct            = 4, // P and B frame ↓
    Pred_L0           = 5,
    Pred_L1           = 6,
    BiPred            = 7

} LumaPredMode_e;

//! Table 7-16: Relationship between IntraChromaPredMode and spatial prediction modes
typedef enum IntraChromaPredMode_e
{
    DC                = 0,
    Horizontal        = 1,
    Vertical          = 2,
    Plane             = 3

} IntraChromaPredMode_e;

//! Macroblock prediction type for I frame
typedef enum mb_type_I
{
    I_NxN             = 0,
    I_16x16_0_0_0     = 1,
    I_16x16_1_0_0     = 2,
    I_16x16_2_0_0     = 3,
    I_16x16_3_0_0     = 4,
    I_16x16_0_1_0     = 5,
    I_16x16_1_1_0     = 6,
    I_16x16_2_1_0     = 7,
    I_16x16_3_1_0     = 8,
    I_16x16_0_2_0     = 9,
    I_16x16_1_2_0     = 10,
    I_16x16_2_2_0     = 11,
    I_16x16_3_2_0     = 12,
    I_16x16_0_0_1     = 13,
    I_16x16_1_0_1     = 14,
    I_16x16_2_0_1     = 15,
    I_16x16_3_0_1     = 16,
    I_16x16_0_1_1     = 17,
    I_16x16_1_1_1     = 18,
    I_16x16_2_1_1     = 19,
    I_16x16_3_1_1     = 20,
    I_16x16_0_2_1     = 21,
    I_16x16_1_2_1     = 22,
    I_16x16_2_2_1     = 23,
    I_16x16_3_2_1     = 24,
    I_PCM             = 25

} mb_type_I;

//! Macroblock prediction type for P frame
typedef enum mb_type_P
{
    P_L0_16x16        = 26, // 0
    P_L0_L0_16x8      = 27,
    P_L0_L0_8x16      = 28,
    P_8x8             = 29,
    P_8x8ref0         = 30,
    P_Skip            = 31  // 5

} mb_type_P;

//! Sub macroblock prediction type for P frame
typedef enum sub_mb_type_P
{
    P_L0_8x8          = 32, // 0
    P_L0_8x4          = 33,
    P_L0_4x8          = 34,
    P_L0_4x4          = 35  // 3

} sub_mb_type_P;

//! Macroblock prediction type for B frame
typedef enum mb_type_B
{
    B_Direct_16x16    = 36, // 0
    B_L0_16x16        = 37,
    B_L1_16x16        = 38,
    B_Bi_16x16        = 39,
    B_L0_L0_16x8      = 40,
    B_L0_L0_8x16      = 41,
    B_L1_L1_16x8      = 42,
    B_L1_L1_8x16      = 43,
    B_L0_L1_16x8      = 44,
    B_L0_L1_8x16      = 45,
    B_L1_L0_16x8      = 46,
    B_L1_L0_8x16      = 47,
    B_L0_Bi_16x8      = 48,
    B_L0_Bi_8x16      = 49,
    B_L1_Bi_16x8      = 50,
    B_L1_Bi_8x16      = 51,
    B_Bi_L0_16x8      = 52,
    B_Bi_L0_8x16      = 53,
    B_Bi_L1_16x8      = 54,
    B_Bi_L1_8x16      = 55,
    B_Bi_Bi_16x8      = 56,
    B_Bi_Bi_8x16      = 57,
    B_8x8             = 58,
    B_Skip            = 59  // 23

} mb_type_B;

//! Sub macroblock prediction type for B frame
typedef enum sub_mb_type_B
{
    B_Direct_8x8      = 60, // 0
    B_L0_8x8          = 61,
    B_L1_8x8          = 62,
    B_Bi_8x8          = 63,
    B_L0_8x4          = 64,
    B_L0_4x8          = 65,
    B_L1_8x4          = 66,
    B_L1_4x8          = 67,
    B_Bi_8x4          = 68,
    B_Bi_4x8          = 69,
    B_L0_4x4          = 70,
    B_L1_4x4          = 71,
    B_Bi_4x4          = 72  // 12

} sub_mb_type_B;

//! Block types
typedef enum BlockType_e
{
    blk_LUMA_8x8      = 0,
    blk_LUMA_4x4      = 1,
    blk_LUMA_16x16_DC = 2,
    blk_LUMA_16x16_AC = 3,
    blk_CHROMA_DC_Cb  = 4,
    blk_CHROMA_DC_Cr  = 5,
    blk_CHROMA_AC_Cb  = 6,
    blk_CHROMA_AC_Cr  = 7,

    blk_UNKNOWN       = 999
} BlockType_e;

/*!
 * \struct Block_t
 */
typedef struct Block_t
{
    int blkType;   // ctxBlockCat
    int YCbCr;
    int blkIdx;
    int maxBlkIdx;
    int maxNumCoeff;

    // CAVLC semantics (7.4.5.3.2)
    int TotalCoeffs_luma[16];
    int TotalCoeffs_chroma[2][4];

    // CABAC semantics (7.4.5.3.3)
    bool coded_block_flag[3][16];
    int numDecodAbsLevelEq1;
    int numDecodAbsLevelGt1;
    int levelListIdx;

} Block_t;

/*!
 * \struct Macroblock_t
 * \todo massive memory optimizations.
 *
 * A square group of 16x16 pixels, that is treated as an entity for video compression
 * and decompression. Macroblocks are usually composed of two or more blocks of pixels.
 * Each macroblock contains 4 Y (luminance) block, 1 Cb (blue color difference) and
 * 1 Cr (red color difference) block (for 4:2:0 subsampling).
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.5 Macroblock layer syntax.
 * - 7.4.5 Macroblock layer semantic.
 */
typedef struct Macroblock_t
{
    // Macroblock level
    ////////////////////////////////////////////////////////////////////////////

    // Macroblock address
    unsigned int mbAddr;                    //!< Current macroblock address in mb_array
    unsigned int mbAddr_x, mbAddr_y;        //!< Current macroblock horizontal and vertical position in mb_array

    // Neighbouring macroblocks addresses
    int mbAddrA, mbAddrB, mbAddrC, mbAddrD; //!< The neighbouring macroblocks addresses

    // Partition(s)
    unsigned int mb_type;                   //!< Specifies the macroblock type
    unsigned int NumMbPart;
    unsigned int MbPartPredMode[4];         //!< Prediction method used by the macroblock

    // Sub-partition(s)
    unsigned int sub_mb_type[4];            //!< Specifies the sub-macroblock types
    unsigned int NumSubMbPart;
    unsigned int SubMbPartPredMode[4];      //!< Prediction method used by each sub-macroblock
    unsigned int SubMbPartWidth[4];
    unsigned int SubMbPartHeight[4];

    // Parameters
    bool transform_size_8x8_flag;           //!< Specifies the size of the blocks (8x8 if true, otherwise default 4x4 size)

    unsigned int coded_block_pattern;       //!< Indicates which blocks within a macroblock contain coded coefficients
    unsigned int CodedBlockPatternLuma;     // derived from coded_block_pattern
    unsigned int CodedBlockPatternChroma;   // derived from coded_block_pattern

    // Quantization parameters
    int mb_qp_delta;                        //!< Can change the value of QPY, only for the macroblock layer
    int QPY;                                //!< The luma quantization parameter, derived from QPY, mb_qp_delta, QpBdOffsetY
    int QPprimeY;                           //!< The previous luma quantization parameter, derived from QPY, QpBdOffsetY
    int QPC[2];                             //!< The chroma quantization parameter, derived from ???
    int QPprimeC[2];                        //!< The previous chroma quantization parameter, derived from QPC, ???
    bool TransformBypassModeFlag;           // derived from qpprime_y_zero_transform_bypass_flag, QPprimeY


    // Residual data semantics
    //FIXME Use dynamic allocations to avoid wasting precious memory space
    ////////////////////////////////////////////////////////////////////////////

#if ENABLE_IPCM
    // I_PCM sample levels
    uint8_t pcm_sample_luma[256];            // 16*16 pixels, the entire luma macroblock
    uint8_t pcm_sample_chroma[256];          // up to 16*16 pixels with 4:4:4 color subsampling
#endif // ENABLE_IPCM

    // Luma sample levels (zig zag scanned)
    int LumaLevel4x4[16][16];               //!< An array of 16 blocks of (4x4) 16 coefficients
    int LumaLevel8x8[4][64];                //!< An array of 4 blocks of (8x8) 64 coefficients
    int Intra16x16DCLevel[16];              //!< An array of 16 luma DC coeff
    int Intra16x16ACLevel[16][15];          //!< An array of 16 blocks of (4*4 - 1) 15 AC coefficients

    // Chroma sample levels (zig zag scanned). Use [2][8] for 4:4:4 subsampling
    int ChromaDCLevel[2][4];                //!< Store chroma DC coeff
    int ChromaACLevel[2][4][15];            //!< Store chroma AC coeff

    // CAVLC semantics (7.4.5.3.2)
    int TotalCoeffs_luma[16];               //!< Store total number of coefficients for each luma block
    int TotalCoeffs_chroma[2][4];           //!< Store total number of coefficients for each chroma block

    // CABAC semantics (7.4.5.3.3)
    bool coded_block_flag[3][17];           //!< Specifies whether the transform block contains non-zero transform coefficient levels
    int numDecodAbsLevelEq1;                //!< Accumulated number of decoded transform coefficient levels with absolute value equal to 1
    int numDecodAbsLevelGt1;                //!< Accumulated number of decoded transform coefficient levels with absolute value greater than 1
    int levelListIdx;


    // Intra predictions
    ////////////////////////////////////////////////////////////////////////////

    bool prev_intra4x4_pred_mode_flag[16];  //!< Most probable mode prediction mode for 4x4 blocks
    bool prev_intra8x8_pred_mode_flag[4];   //!< Most probable mode prediction mode for 8x8 blocks
    unsigned int rem_intra4x4_pred_mode[16];
    unsigned int rem_intra8x8_pred_mode[4];

    unsigned int Intra4x4PredMode[16];      //!< Final table of prediction modes for each 4x4 block of the macroblock
    unsigned int Intra8x8PredMode[4];       //!< Final table of prediction modes for each 8x8 block of the macroblock
    unsigned int Intra16x16PredMode;        //!< Prediction mode for 16x16 macroblock
    unsigned int IntraChromaPredMode;       //!< Prediction mode for chroma blocks

    uint8_t predL[16][16];                  //!< Prediction array for luma samples
    uint8_t predCb[8][8];                   //!< Prediction array for chroma Cb samples
    uint8_t predCr[8][8];                   //!< Prediction array for vhroma Cr samples


    // Inter predictions
    ////////////////////////////////////////////////////////////////////////////

    unsigned int ref_idx_l0[4];             //!< Reference frame index list 0
    unsigned int ref_idx_l1[4];             //!< Reference frame index list 1
    int mvd_l0[4][4][2];                    //!< Motion vectors differences list 0
    int mvd_l1[4][4][2];                    //!< Motion vectors differences list 1


    // Constructed sample arrays
    ////////////////////////////////////////////////////////////////////////////

    uint8_t SprimeL[16][16];                //!< Luma samples
    uint8_t SprimeCb[8][8];                 //!< Chroma Cb samples
    uint8_t SprimeCr[8][8];                 //!< Chroma Cr samples

#if ENABLE_DEBUG
    unsigned mbFileAddrStart;               //!< The macroblock start address in bit
    unsigned mbFileAddrStop;                //!< The macroblock stop address in bit
#endif // ENABLE_DEBUG

} Macroblock_t;

/* ************************************************************************** */
#endif // H264_MACROBLOCK_STRUCT_H
