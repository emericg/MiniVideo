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
 * \file      h264_transform.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_transform.h"
#include "h264_spatial.h"
#include "../../minivideo_typedef.h"
#include "../../utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* ************************************************************************** */
/*
// Default scaling lists

int Default_4x4_Intra[16] = { 6, 13, 13, 20, 20, 20, 28, 28, 28, 28, 32, 32, 32, 37, 37, 42};

int Default_4x4_Inter[16] = {10, 14, 14, 20, 20, 20, 24, 24, 24, 24, 27, 27, 27, 30, 30, 34};

int Default_8x8_Intra[64] = { 6, 10, 10, 13, 11, 13, 16, 16, 16, 16, 18, 18, 18, 18, 18, 23,
                             23, 23, 23, 23, 23, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27,
                             27, 27, 27, 27, 29, 29, 29, 29, 29, 29, 29, 31, 31, 31, 31, 31,
                             31, 33, 33, 33, 33, 33, 36, 36, 36, 36, 38, 38, 38, 40, 40, 42};

int Default_8x8_Inter[64] = { 9, 13, 13, 15, 13, 15, 17, 17, 17, 17, 19, 19, 19, 19, 19, 21,
                             21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24,
                             24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27, 27,
                             27, 28, 28, 28, 28, 28, 30, 30, 30, 30, 32, 32, 32, 33, 33, 35};
*/
/* ************************************************************************** */

const int idct_dccoeff_2x2[2][2] =
{
    { 1, 1},
    { 1,-1}
};

const int idct_dccoeff_4x4[4][4] =
{
    { 1, 1, 1, 1},
    { 1, 1,-1,-1},
    { 1,-1,-1, 1},
    { 1,-1, 1,-1}
};

/// Use Table 8-15: Specification of QPC as a function of qPI
const int QPCfunctionofqPI[22] = {29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39};

/// Figure 8-7: Assignment of the indices of dcC to chroma4x4BlkIdx when ChromaArrayType equal to 2
const int raster_chroma_cat2_2d[8][2] = {{0,0}, {0,1}, {1,0}, {1,1}, {2,0}, {2,1}, {3,0}, {3,1}};

/* ************************************************************************** */

static void print2x2(int block[2][2]);
static void print4x4(int block[4][4]);
static void print8_tx8(int block[8][8]);

static void derivChromaQP(DecodingContext_t *dc, const int iCbCr);

static int transform_16x16_lumadc(DecodingContext_t *dc, const int c[4][4], int dcY[4][4]);

static int transform_2x2_chromadc(DecodingContext_t *dc, const int YCbCr, const int c[2][2], int dcC[2][2]);
    static void quant2x2_chromadc(sps_t *sps, const int YCbCr, const int qP, const int f[2][2], int dcC[2][2]);
    static void idct2x2_chromadc(const int c[2][2], int f[2][2]);

static int transform_4x4_chromadc(DecodingContext_t *dc, const int YCbCr, const int c[4][4], int dcC[4][4]);
    static void quant4x4_chromadc(sps_t *sps, const int YCbCr, const int qP, const int f[4][4], int dcC[4][4]);
    static void idct4x4_chromadc(const int c[4][4], int f[4][4]);

static int transform_4x4_residual(DecodingContext_t *dc, const int YCbCr, const int c[4][4], int r[4][4]);
    static void quant4x4(sps_t *sps, const int YCbCr, const int mbPartPredMode, const int qP, const int c[4][4], int d[4][4]);
    static void idct4x4(const int d[4][4], int r[4][4]);

static int transform_8x8_residual(DecodingContext_t *dc, const int YCbCr, const int c[8][8], int r[8][8]);
    static void quant8x8(sps_t *sps, const int YCbCr, const int qP, const int c[8][8], int r[8][8]);
    static void idct8x8(const int d[8][8], int r[8][8]);

static int picture_construction_process_4x4(DecodingContext_t *dc, const int blkIdx, const int u[4][4]);
static int picture_construction_process_4x4chroma(DecodingContext_t *dc, const int YCbCr, const int blkIdx, const int u[4][4]);
static int picture_construction_process_8x8(DecodingContext_t *dc, const int blkIdx, const int u[8][8]);
static int picture_construction_process_8x8chroma(DecodingContext_t *dc, const int YCbCr, const int blkIdx, const int u[8][8]);
static int picture_construction_process_16x16(DecodingContext_t *dc, const int u[16][16]);

static int **transformbypass_decoding(DecodingContext_t *dc, bool horPredFlag, int **r, int nW, int nH);

/* ************************************************************************** */

/*!
 * \brief Transform decoding process for 4x4 luma blocks.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma4x4BlkIdx The index of the current 4x4 luma block.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.1 Specification of transform decoding process for 4x4 luma residual blocks.
 */
void transform4x4_luma(DecodingContext_t *dc, Macroblock_t *mb, int luma4x4BlkIdx)
{
    TRACE_1(TRANS, "<> " BLD_GREEN "transform4x4_luma()" CLR_RESET);

    int c[4][4];// = {{0}};
    int r[4][4];// = {{0}};
    int u[4][4];// = {{0}};

    // 1
    inverse_scan_4x4(mb->LumaLevel4x4[luma4x4BlkIdx], c);

    // 2
    transform_4x4_residual(dc, 0, c, r);

    // 3
    if (mb->TransformBypassModeFlag &&
        mb->MbPartPredMode[0] == Intra_4x4 &&
        (mb->Intra4x4PredMode[luma4x4BlkIdx] == 0 || mb->Intra4x4PredMode[luma4x4BlkIdx] == 1))
    {
        //transformbypass_decoding(dc, 4, 4, mb->Intra4x4PredMode[luma4x4BlkIdx], r);
        TRACE_WARNING(TRANS, ">>> UNIMPLEMENTED (transformbypass_decoding)");
    }

    // 4
    int xO = -1, yO = -1;
    InverseLuma4x4BlkScan(luma4x4BlkIdx, &xO, &yO);

    // 5
    int i = 0, j = 0;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            u[i][j] = iClip1_YCbCr_8((int)mb->predL[xO+j][yO+i] + r[i][j]);

    // 6
    picture_construction_process_4x4(dc, luma4x4BlkIdx, u);
}

/* ************************************************************************** */

/*!
 * \brief Transform decoding process for 16x16 luma blocks.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.2 Specification of transform decoding process for luma samples of Intra_16x16 macroblock prediction mode.
 */
void transform16x16_luma(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(TRANS, "<> " BLD_GREEN "transform16x16_luma()" CLR_RESET);

    // 1 (DC coefficients)
    int c1[4][4];// = {{0}};
    int dcY[4][4];// = {{0}};
    int rMb[16][16];// = {{0}};
    int u[16][16];// = {{0}};
    int lumaList[16];// = {0};
    int luma4x4BlkIdx = 0;

    inverse_scan_4x4(mb->Intra16x16DCLevel, c1);
    transform_16x16_lumadc(dc, c1, dcY);

    // 2 (AC coefficients)
    for (luma4x4BlkIdx = 0; luma4x4BlkIdx<16; luma4x4BlkIdx++)
    {
        lumaList[0] = dcY[raster_4x4_2d[luma4x4BlkIdx][0]][raster_4x4_2d[luma4x4BlkIdx][1]];

        int k = 1;
        for (k = 1; k < 16; k++)
            lumaList[k] = mb->Intra16x16ACLevel[luma4x4BlkIdx][k-1];

        int c[4][4];// = {{0}};
        int r[4][4];// = {{0}};

        inverse_scan_4x4(lumaList, c);
        transform_4x4_residual(dc, 0, c, r);

        int xO = -1, yO = -1;
        InverseLuma4x4BlkScan(luma4x4BlkIdx, &xO, &yO);

        int i = 0, j = 0;
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                rMb[xO + i][yO + j] = r[j][i];
    }

    // 3
    if (mb->TransformBypassModeFlag &&
        (mb->Intra16x16PredMode == 0 || mb->Intra16x16PredMode == 1))
    {
        //transformbypass_decoding(dc, mb->Intra16x16PredMode, rMb, 16, 16);
        TRACE_WARNING(TRANS, ">>> UNIMPLEMENTED (transformbypass_decoding)");
    }

    // 4
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            u[i][j] = iClip1_YCbCr_8((int)mb->predL[j][i] + rMb[j][i]);

    // 5
    picture_construction_process_16x16(dc, u);
}

/* ************************************************************************** */

/*!
 * \brief Transform decoding process for 8x8 luma blocks.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \param luma8x8BlkIdx The index of the current 8x8 luma block.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.3 Specification of transform decoding process for 8x8 luma residual blocks.
 */
void transform8x8_luma(DecodingContext_t *dc, Macroblock_t *mb, int luma8x8BlkIdx)
{
    TRACE_1(TRANS, "<> " BLD_GREEN "transform8x8_luma()" CLR_RESET);

    int c[8][8];// = {{0}};
    int r[8][8];// = {{0}};
    int u[8][8];// = {{0}};

    // 1
    inverse_scan_8x8(mb->LumaLevel8x8[luma8x8BlkIdx], c);

    // 2
    transform_8x8_residual(dc, 0, c, r);

    // 3
    if (mb->TransformBypassModeFlag &&
        mb->MbPartPredMode[0] == Intra_8x8 &&
        (mb->Intra8x8PredMode[luma8x8BlkIdx] == 0 || mb->Intra8x8PredMode[luma8x8BlkIdx] == 1))
     {
         //transformbypass_decoding(dc, 8, 8, mb->Intra8x8PredMode[luma8x8BlkIdx], r);
         TRACE_WARNING(TRANS, ">>> UNIMPLEMENTED (transformbypass_decoding)");
     }

    // 4
    int xO = -1, yO = -1;
    InverseLuma8x8BlkScan(luma8x8BlkIdx, &xO, &yO);

    // 5
    int i = 0, j = 0;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            u[i][j] = iClip1_YCbCr_8((int)mb->predL[xO+j][yO+i] + r[i][j]);

    // 6
    picture_construction_process_8x8(dc, luma8x8BlkIdx, u);
}

/* ************************************************************************** */

/*!
 * \brief Transform decoding process for 4x4 chroma blocks.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.4 Specification of transform decoding process for chroma samples.
 *
 * Note that only ChromaArrayType == 1 is currently supported by this decoder,
 * every other value will be caught by decodeSPS().
 */
void transform4x4_chroma(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(TRANS, "<> " BLD_GREEN "transform4x4_chroma()" CLR_RESET);

    // Shortcuts
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    // Init
    unsigned int i = 0, j = 0, k = 0;
    int iCbCr = 0;
    int numChroma4x4Blks = (sps->MbWidthC/4) * (sps->MbHeightC/4);

    for (iCbCr = 0; iCbCr < 2; iCbCr++)
    {
        // Deriv chroma quantization parameter qP
        derivChromaQP(dc, iCbCr);

        // 1 (DC coefficients)
        int u[8][8];// = {{0}};
/*
        if (dc->ChromaArrayType == 1)
        {
*/          int cc[2][2];// = {{0}};
            int dcC[2][2];// = {{0}};

            // c[2][2]
            cc[0][0] = mb->ChromaDCLevel[iCbCr][0];
            cc[0][1] = mb->ChromaDCLevel[iCbCr][1];
            cc[1][0] = mb->ChromaDCLevel[iCbCr][2];
            cc[1][1] = mb->ChromaDCLevel[iCbCr][3];

            transform_2x2_chromadc(dc, iCbCr+1, cc, dcC);
/*
        }
        else if (dc->ChromaArrayType == 2)
        {
            int cc[4][4] = {{0}};
            int dcC[4][4] = {{0}};

            // c[2][4]
            // need mb->ChromaDCLevel[2][8]
            cc[0][0] = mb->ChromaDCLevel[iCbCr][0];
            cc[0][1] = mb->ChromaDCLevel[iCbCr][2];
            cc[1][0] = mb->ChromaDCLevel[iCbCr][1];
            cc[1][1] = mb->ChromaDCLevel[iCbCr][5];
            cc[2][0] = mb->ChromaDCLevel[iCbCr][3];
            cc[2][1] = mb->ChromaDCLevel[iCbCr][6];
            cc[3][0] = mb->ChromaDCLevel[iCbCr][4];
            cc[3][1] = mb->ChromaDCLevel[iCbCr][7];

            transform_4x4_chromadc(dc, iCbCr+1, cc, dcC);
        }
*/

        // 2 (AC coefficients)
        int rMb[8][8];
        int chromaList[16];
        int chroma4x4BlkIdx = 0;

        for (chroma4x4BlkIdx = 0; chroma4x4BlkIdx<numChroma4x4Blks; chroma4x4BlkIdx++)
        {
            int c[4][4];
            int r[4][4];
/*
            if (dc->ChromaArrayType == 1)
            {
*/
                chromaList[0] = dcC[raster_8x8_2d[chroma4x4BlkIdx][0]][raster_8x8_2d[chroma4x4BlkIdx][1]];
/*
            }
            else if (dc->ChromaArrayType == 2)
            {
                chromaList[0] = dcC[raster_chroma_cat2_2d[chroma4x4BlkIdx][0]][raster_chroma_cat2_2d[chroma4x4BlkIdx][1]];
            }
*/
            for (k = 1; k < 16; k++)
            {
                chromaList[k] = mb->ChromaACLevel[iCbCr][chroma4x4BlkIdx][k-1];
            }

            inverse_scan_4x4(chromaList, c);
            transform_4x4_residual(dc, iCbCr+1, c, r);

            int xO = -1, yO = -1;
            InverseChroma4x4BlkScan(chroma4x4BlkIdx, &xO, &yO);

            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    rMb[xO + j][yO + i] = r[i][j];
        }

        // 3
        if (mb->TransformBypassModeFlag &&
            (mb->MbPartPredMode[0] == Intra_4x4 || mb->MbPartPredMode[0] == Intra_8x8 || mb->MbPartPredMode[0] == Intra_16x16) &&
            (mb->IntraChromaPredMode == 1 || mb->IntraChromaPredMode == 2))
        {
            //transformbypass_decoding(dc, (2 - mb->IntraChromaPredMode), rMb, sps->MbWidthC, sps->MbHeightC);
            TRACE_WARNING(TRANS, ">>> UNIMPLEMENTED (transformbypass_decoding)");
        }

        // 4
        for (i = 0; i < sps->MbWidthC; i++)
        {
            for (j = 0; j < sps->MbHeightC; j++)
            {
                if (iCbCr == 0)
                    u[i][j] = iClip1_YCbCr_8((int)mb->predCb[j][i] + rMb[j][i]);
                else
                    u[i][j] = iClip1_YCbCr_8((int)mb->predCr[j][i] + rMb[j][i]);
            }
        }

        // 5
        picture_construction_process_8x8chroma(dc, iCbCr+1, 0, u);
    }
}

/* ************************************************************************** */

/*!
 * \brief Transform decoding process for 4x4 chroma blocks with ChromaArrayType 3.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.5 Specification of transform decoding process for chroma samples with ChromaArrayType equal to 3.
 */
void transform4x4_chroma_cat3(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_1(TRANS, "<> " BLD_GREEN "transform4x4_chroma_cat3()" CLR_RESET);

    TRACE_ERROR(TRANS, ">>> UNSUPPORTED (ChromaArrayType == 3)");
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Inverse scanning process for 4x4 transform coefficients and scaling lists.
 * \param list[] A list of 16 coefficients.
 * \param matrix[][] A two-dimensional array of 4x4 coefficients.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.6 Inverse scanning process for 4x4 transform coefficients and scaling lists.
 *
 * Output of this process is a variable "matrix" containing a two-dimensional array of 4x4 values.
 * In the case of transform coefficients, these 4x4 values represent levels assigned to
 * locations in the transform block. In the case of applying the inverse scanning process
 * to a scaling list, the output variable "matrix" contains a two-dimensional array representing
 * a 4x4 scaling matrix.
 *
 * See table 8-13: implemented in utils.h.
 */
void inverse_scan_4x4(int list[16], int matrix[4][4])
{
    TRACE_2(TRANS, " inverse_scan_4x4()");

    // Table transfer
    for (int zz = 0; zz < 16; zz++)
    {
        matrix[zigzag_4x4_2d[zz][0]][zigzag_4x4_2d[zz][1]] = list[zz];
    }
}

/* ************************************************************************** */

/*!
 * \brief Inverse scanning process for 8x8 transform coefficients and scaling lists.
 * \param list[] A list of 64 luma coefficients.
 * \param matrix[][] A two-dimensional array of 8x8 luma coefficients.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.7 Inverse scanning process for 8x8 transform coefficients and scaling lists.
 *
 * Output of this process is a variable "matrix" containing a two-dimensional array of 8x8 values.
 * In the case of transform coefficients, these 8x8 values represent levels assigned to
 * locations in the transform block. In the case of applying the inverse scanning process
 * to a scaling list, the output variable "matrix" contains a two-dimensional array representing
 * a 8x8 scaling matrix.
 *
 * See table 8-14: implemented in utils.h.
 */
void inverse_scan_8x8(int list[64], int matrix[8][8])
{
    TRACE_2(TRANS, " inverse_scan_8x8()");

    // Table transfer
    for (int zz = 0; zz < 64; zz++)
    {
        matrix[zigzag_8x8_2d[zz][0]][zigzag_8x8_2d[zz][1]] = list[zz];
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param block[][] A two-dimensional array of 2x2 residual coefficients.
 */
static void print2x2(int block[2][2])
{
#if ENABLE_DEBUG
    TRACE_2(TRANS, " print2x2()");

    // Print residual block
    printf("+-------------------+\n");
    int i = 0, j = 0;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (j%2 == 0)
            {
                printf("|");
            }
            else
            {
                printf(",");
            }

            printf("%4i", block[i][j]); // print residual coeff
        }
        printf("|\n");
    }
    printf("+-------------------+\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \param block[][] A two-dimensional array of 4x4 residual coefficients.
 */
static void print4x4(int block[4][4])
{
#if ENABLE_DEBUG
    TRACE_2(TRANS, " print4x4()");

    // Print residual block
    printf("+-------------------+\n");
    int i = 0, j = 0;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (j%4 == 0)
            {
                printf("|");
            }
            else
            {
                printf(",");
            }

            printf("%4i", block[i][j]); // print residual coeff
        }
        printf("|\n");
    }
    printf("+-------------------+\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \param block[][] A two-dimensional array of 8x8 residual coefficients.
 */
static void print8_tx8(int block[8][8])
{
#if ENABLE_DEBUG
    TRACE_2(TRANS, " print8_tx8()");

    // Print residual block
    printf("+---------------------------------------+\n");
    int i = 0, j = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (j%8 == 0)
            {
                printf("|");
            }
            else
            {
                printf(",");
            }

            printf("%4i", block[i][j]); // print residual coeff
        }
        printf("|\n");
    }
    printf("+---------------------------------------+\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param iCbCr Indicate if we are dealing with a Cb (0) or Cr (1) block.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.8 Derivation process for chroma quantisation parameters.
 *
 * Note that this function do not implement the QSC computation in case the current
 * slice is an SI or SP slice.
 */
static void derivChromaQP(DecodingContext_t *dc, const int iCbCr)
{
    TRACE_2(TRANS, " derivChromaQP()");

    // Shortcuts
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];
    pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
    sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];

    // Initialization
    int qPOffset = 0;
    int qPI = 0;

    if (iCbCr == 0)
    {
        qPOffset = pps->chroma_qp_index_offset;
    }
    else //if (iCbCr == 1)
    {
        qPOffset = pps->second_chroma_qp_index_offset;
    }

    // Compute qPI, QPC and QPprimeC
    int low = sps->QpBdOffsetC;
    qPI = iClip3(-low, 51, mb->QPY + qPOffset);

    if (qPI > 29)
    {
        mb->QPC[iCbCr] = QPCfunctionofqPI[qPI-30];
    }
    else
    {
        mb->QPC[iCbCr] = qPI;
    }

    mb->QPprimeC[iCbCr] = mb->QPC[iCbCr] + sps->QpBdOffsetC;
/*
    NOTE 1: QP quantisation parameter values QPY and QSY are always in the range
    of -QpBdOffsetY to 51, inclusive. QP quantisation parameter values QPC and
    QSC are always in the range of -QpBdOffsetC to 39, inclusive.
*/
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *sps The SPS currently in use.
 */
void computeLevelScale4x4(DecodingContext_t *dc, sps_t *sps)
{
    // Initialization
    int YCbCr = 0, q = 0, i = 0, j = 0;

#if ENABLE_INTER_PRED
    bool mbIsInterFlag = false;

    if (mb->MbPartPredMode[0] > 3)
    {
        mbIsInterFlag = true;
        TRACE_ERROR(SPATIAL, ">>> UNSUPPORTED (MbPartPredMode > 3)");
        return UNSUPPORTED;
    }
#endif // ENABLE_INTER_PRED

#if ENABLE_SEPARATE_COLOUR_PLANES
    if (sps->separate_colour_plane_flag)
    {
        YCbCr = sps->separate_colour_plane_flag;
        TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == true)");
        return UNSUPPORTED;
    }
#endif // ENABLE_SEPARATE_COLOUR_PLANES

    // Compute
    for (YCbCr = 0; YCbCr < 3; YCbCr++)
        for (q = 0; q < 6; q++)
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    sps->LevelScale4x4[YCbCr][q][i][j] = sps->ScalingMatrix4x4[YCbCr /*+ ((mbIsInterFlag == true) ? 3 : 0)*/][i][j] * dc->normAdjust4x4[q][i][j];
/*
    // Print
    for (YCbCr = 0; YCbCr < 3; YCbCr++)
    {
        for (q = 0; q < 6; q++)
        {
            TRACE_1(TRANS, "YCbCr: %i / Qp: %i", YCbCr, q);
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    TRACE_1(TRANS, "levelscale_4x4: %i", sps->LevelScale4x4[YCbCr][q][i][j]);
        }
    }
*/
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *sps The SPS currently in use.
 */
void computeLevelScale8x8(DecodingContext_t *dc, sps_t *sps)
{
    // Initialization
    int YCbCr = 0, q = 0, i = 0, j = 0;

#if ENABLE_INTER_PRED
    bool mbIsInterFlag = false;

    if (mb->MbPartPredMode[0] > 3)
    {
        mbIsInterFlag = true;
        TRACE_ERROR(SPATIAL, ">>> UNSUPPORTED (MbPartPredMode > 3)");
        return UNSUPPORTED;
    }
#endif // ENABLE_INTER_PRED

#if ENABLE_SEPARATE_COLOUR_PLANES
    if (sps->separate_colour_plane_flag)
    {
        YCbCr = sps->separate_colour_plane_flag;
        TRACE_ERROR(PARAM, ">>> UNSUPPORTED (separate_colour_plane_flag == true)");
        return UNSUPPORTED;
    }
#endif // ENABLE_SEPARATE_COLOUR_PLANES

    // Compute
    for (YCbCr = 0; YCbCr < 3; YCbCr++)
        for (q = 0; q < 6; q++)
            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++)
                    sps->LevelScale8x8[YCbCr][q][i][j] = sps->ScalingMatrix8x8[YCbCr /*+ ((mbIsInterFlag == true) ? 3 : 0)*/][i][j] * dc->normAdjust8x8[q][i][j];
/*
    // Print
    for (YCbCr = 0; YCbCr < 3; YCbCr++)
    {
        for (q = 0; q < 6; q++)
        {
            TRACE_1(TRANS, "YCbCr: %i / Qp: %i", YCbCr, q);
            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++)
                    TRACE_1(TRANS, "  levelscale_8x8[%i][%i]: %i", i, j, sps->LevelScale8x8[YCbCr][q][i][j]);
        }
    }
*/
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param c[][] A residual block of luma or chroma component.
 * \param dcY[][] A residual sample array quantized and transformed.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.10 Scaling and transformation process for DC transform coefficients for Intra_16x16 macroblock type.
 *
 * Note: For DC coefficients, the quantization is done after the idct.
 */
static int transform_16x16_lumadc(DecodingContext_t *dc, const int c[4][4], int dcY[4][4])
{
    TRACE_2(TRANS, " transform_16x16_lumadc()");
    int retcode = SUCCESS;

    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int qP = mb->QPprimeY;
    int qPmod6 = qP % 6;
    int qPdiv6 = qP / 6;
    int i = 0, j = 0, k = 0;

    if (mb->TransformBypassModeFlag)
    {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                dcY[i][j] = c[i][j];
    }
    else
    {
        int f1[4][4] = {{0}};
        int f2[4][4] = {{0}};

        // Transform - round 1
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                for (k = 0; k < 4; k++)
                    f1[i][j] = idct_dccoeff_4x4[i][k] * c[k][j] + f1[i][j];

        // Transform - round 2
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                for (k = 0; k < 4; k++)
                    f2[i][j] = f1[i][k] * idct_dccoeff_4x4[k][j] + f2[i][j];

        //print4x4(f2); // DC transformed coefficients

        // Quantization
        if (qP > 36)
        {
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    dcY[i][j] = (f2[i][j] * sps->LevelScale4x4[0][qPmod6][0][0]) << (qPdiv6 - 6);
        }
        else
        {
            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    dcY[i][j] = (f2[i][j] * sps->LevelScale4x4[0][qPmod6][0][0] + (1 << (5 - qPdiv6))) >> (6 - qPdiv6);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param c[][] A residual block of chroma component.
 * \param dcC[][] An array residual sample array **r, quantized and transformed.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11 Scaling and transformation process for chroma DC transform coefficients.
 *
 * Note: For DC coefficients, the quantization is done after the idct.
 */
static int transform_2x2_chromadc(DecodingContext_t *dc, const int YCbCr,
                                  const int c[2][2], int dcC[2][2])
{
    TRACE_2(TRANS, " transform_2x2_chromadc()");
    int retcode = SUCCESS;

    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int qP = mb->QPprimeC[YCbCr-1];

    // Scale and transform
    if (mb->TransformBypassModeFlag)
    {
        int i = 0, j = 0;

        for (i = 0; i < 2; i++)
            for (j = 0; j < 2; j++)
                dcC[i][j] = c[i][j];
    }
    else
    {
        int f[2][2] = {{0}};
        idct2x2_chromadc(c, f);
        //print2x2(f); // DC transformed coefficients
        quant2x2_chromadc(sps, YCbCr, qP, f, dcC);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param c[][] A residual block of chroma component.
 * \param dcC[][] An array residual sample array **r, quantized and transformed.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11 Scaling and transformation process for chroma DC transform coefficients.
 *
 * Note: For DC coefficients, the quantization is done after the idct.
 */
static int transform_4x4_chromadc(DecodingContext_t *dc, const int YCbCr,
                                  const int c[4][4], int dcC[4][4])
{
    TRACE_2(TRANS, " transform_4x4_chromadc()");
    int retcode = SUCCESS;

    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int qP = mb->QPprimeC[YCbCr-1];

    // Scale and transform
    if (mb->TransformBypassModeFlag)
    {
        int i = 0, j = 0;

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                dcC[i][j] = c[i][j];
    }
    else
    {
        int f[4][4] = {{0}};
        idct4x4_chromadc(c, f);
        //print4x4(f); // DC transformed coefficients
        quant4x4_chromadc(sps, YCbCr, qP, f, dcC);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *sps The SPS currently in use.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param qP The quantization parameter.
 * \param f[][] A residual block of luma or chroma component.
 * \param dcC[][] An quantized residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11.2 Scaling process for chroma DC transform coefficients.
 *
 * Quantification for 2x2 chroma dc blocks, ChromaArrayType == 1, 4:2:0 subsampling.
 */
static void quant2x2_chromadc(sps_t *sps, const int YCbCr, const int qP,
                              const int f[2][2], int dcC[2][2])
{
    TRACE_2(TRANS, "quant2x2_chromadc()");
    int i = 0, j = 0;

    // Scale
    int qPdiv6 = qP / 6;
    int myscale = sps->LevelScale4x4[YCbCr][qP % 6][0][0];

    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            dcC[i][j] = ((f[i][j] * myscale) << (qPdiv6)) >> 5;
}

/* ************************************************************************** */

/*!
 * \param *sps The SPS currently in use.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param qP The quantization parameter.
 * \param f[][] A residual block of luma or chroma component.
 * \param dcC[][] An quantized residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11.2 Scaling process for chroma DC transform coefficients.
 *
 * Quantification for 4x4 chroma dc blocks, ChromaArrayType == 2, 4:2:0 subsampling.
 */
static void quant4x4_chromadc(sps_t *sps, const int YCbCr, const int qP,
                              const int f[4][4], int dcC[4][4])
{
    TRACE_2(TRANS, "quant4x4_chromadc()");
    int i = 0, j = 0;

    // Scale
    int qPDCdiv6 = (int)((qP+3) / 6.0);
    int myscale = sps->LevelScale4x4[YCbCr][(qP+3) % 6][0][0];

    if (qP > 32)
    {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 2; j++)
                dcC[i][j] = (f[i][j] * myscale) << (qPDCdiv6 - 6);
    }
    else
    {
        int mypow = (int)pow(2, 5 - qPDCdiv6);

        for (i = 0; i < 4; i++)
            for (j = 0; j < 2; j++)
                dcC[i][j] = (f[i][j] * myscale + mypow) >> (6 - qPDCdiv6);
    }
}

/* ************************************************************************** */

/*!
 * \param c[][] A quantized residual block of luma or chroma component.
 * \param f[][] An quantized and transformed residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11.1 Transformation process for chroma DC transform coefficients.
 *
 * Transformation for 2x2 chroma dc blocks, ChromaArrayType == 1, 4:2:0 subsampling.
 */
static void idct2x2_chromadc(const int c[2][2], int f[2][2])
{
    TRACE_2(TRANS, "idct2x2_chromadc()");
    int i = 0, j = 0, k = 0;
    int t1[2][2] = {{0}};

    // Transform - round 1
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            for (k = 0; k < 2; k++)
                t1[i][j] = idct_dccoeff_2x2[i][k] * c[k][j] + t1[i][j];

    // Transform - round 2
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            for (k = 0; k < 2; k++)
                f[i][j] = t1[i][k] * idct_dccoeff_2x2[k][j] + f[i][j];
}

/* ************************************************************************** */

/*!
 * \param c[][] A quantized residual block of luma or chroma component.
 * \param f[][] An quantized and transformed residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.11.1 Transformation process for chroma DC transform coefficients.
 *
 * Transformation for 4x4 chroma dc blocks, ChromaArrayType == 2, 4:2:2 subsampling.
 */
static void idct4x4_chromadc(const int c[4][4], int f[4][4])
{
    TRACE_2(TRANS, "idct4x4_chromadc()");
    int i = 0, j = 0, k = 0;
    int t1[2][2] = {{0}};

    // Transform - round 1
    for (i = 0; i < 2; i++)
        for (j = 0; j < 4; j++)
            for (k = 0; k < 4; k++)
                t1[i][j] = idct_dccoeff_4x4[i][k] * c[k][j] + t1[i][j];

    // Transform - round 2 //FIXME untested
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            for (k = 0; k < 2; k++)
                f[i][j] = t1[i][k] * idct_dccoeff_2x2[k][j] + f[i][j];
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param c[][] A residual block of luma or chroma component.
 * \param r[][] A residual sample array quantized and transformed.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.12 Scaling and transformation process for residual 4x4 blocks.
 */
static int transform_4x4_residual(DecodingContext_t *dc, const int YCbCr,
                                  const int c[4][4], int r[4][4])
{
    TRACE_2(TRANS, " transform_4x4_residual()");
    int retcode = SUCCESS;

    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int qP = mb->QPprimeY;

    if (YCbCr != 0)
        qP = mb->QPprimeC[YCbCr-1];;

    // Scale and transform
    if (mb->TransformBypassModeFlag)
    {
        int i = 0, j = 0;

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                r[i][j] = c[i][j];
    }
    else
    {
        int d[4][4];// = {{0}};
        quant4x4(sps, YCbCr, mb->MbPartPredMode[0], qP, c, d);
        //print4x4(d); // Print residual block
        idct4x4(d, r);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *sps The SPS currently in use.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param mbPartPredMode The block prediction mode.
 * \param qP The quantization parameter.
 * \param c A residual block of luma or chroma component.
 * \param d A quantized residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.12.1 Scaling process for residual 4x4 blocks.
 */
static void quant4x4(sps_t *sps, const int YCbCr, const int mbPartPredMode,
                     const int qP, const int c[4][4], int d[4][4])
{
    TRACE_2(TRANS, " quant4x4()");

    // Initialization
    int i = 0, j = 0;
    int qPmod6 = qP % 6;
    int qPdiv6 = qP / 6;

    // Quantization
    if (qP > 23)
    {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                d[i][j] = (c[i][j] * sps->LevelScale4x4[YCbCr][qPmod6][i][j]) << (qPdiv6 - 4);
    }
    else
    {
        int mypow = (int)pow(2, 3 - qPdiv6);

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                d[i][j] = (c[i][j] * sps->LevelScale4x4[YCbCr][qPmod6][i][j] + mypow) >> (4 - qPdiv6);
    }

    // Save DC coefficient for Intra_16x16 prediction mode
    if (mbPartPredMode == Intra_16x16 || YCbCr != 0)
    {
        d[0][0] = c[0][0];
    }
/*
    The bitstream shall not contain data that result in any element dij of d with i, j = 0..3
    that exceeds the range of integer values from -2(7 + bitDepth) to 2(7 + bitDepth) - 1, inclusive.
*/
}

/* ************************************************************************** */

/*!
 * \param d[][] A quantized residual block of luma or chroma component.
 * \param r[][] A quantized and transformed residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.12.2 Transformation process for residual 4x4 blocks.
 */
static void idct4x4(const int d[4][4], int r[4][4])
{
    TRACE_2(TRANS, " idct4x4()");

    int e[4][4];// = {{0}};
    int f[4][4];// = {{0}};
    int g[4][4];// = {{0}};
    int h[4][4];// = {{0}};

    int i = 0, j = 0;

    for (i = 0; i < 4; i++)
    {
        e[i][0] = d[i][0] + d[i][2];
        e[i][1] = d[i][0] - d[i][2];
        e[i][2] = (d[i][1] >> 1) - d[i][3];
        e[i][3] = d[i][1] + (d[i][3] >> 1);
    }

    for (i = 0; i < 4; i++)
    {
        f[i][0] = e[i][0] + e[i][3];
        f[i][1] = e[i][1] + e[i][2];
        f[i][2] = e[i][1] - e[i][2];
        f[i][3] = e[i][0] - e[i][3];
    }

    for (j = 0; j < 4; j++)
    {
        g[0][j] = f[0][j] + f[2][j];
        g[1][j] = f[0][j] - f[2][j];
        g[2][j] = (f[1][j] >> 1) - f[3][j];
        g[3][j] = f[1][j] + (f[3][j] >> 1);
    }

    for (j = 0; j < 4; j++)
    {
        h[0][j] = g[0][j] + g[3][j];
        h[1][j] = g[1][j] + g[2][j];
        h[2][j] = g[1][j] - g[2][j];
        h[3][j] = g[0][j] - g[3][j];
    }

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = (h[i][j] + 32) >> 6;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param c[][] A residual block of luma or chroma component.
 * \param r[][] A residual sample array quantized and transformed.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.13 Scaling and transformation process for residual 8x8 blocks.
 */
static int transform_8x8_residual(DecodingContext_t *dc, const int YCbCr,
                                  const int c[8][8], int r[8][8])
{
    TRACE_2(TRANS, " transform_8x8_residual()");
    int retcode = SUCCESS;

    // Shortcuts
    sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int qP = mb->QPprimeY;

    if (YCbCr != 0)
    {
        qP = mb->QPprimeC[YCbCr-1];
    }

    // Scale and transform
    if (mb->TransformBypassModeFlag)
    {
        int i = 0, j = 0;
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                r[i][j] = c[i][j];
    }
    else
    {
        int d[8][8];// = {{0}};
        quant8x8(sps, YCbCr, qP, c, d);
        //print8_tx8(d); // Print residual block
        idct8x8(d, r);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *sps The SPS currently in use.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param qP The quantization parameter.
 * \param c[][] A residual block of luma or chroma component.
 * \param d[][] A quantized residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.13.1 Scaling process for residual 8x8 blocks.
 */
static void quant8x8(sps_t *sps, const int YCbCr, const int qP,
                     const int c[8][8], int d[8][8])
{
    TRACE_2(TRANS, " quant8x8()");

    // Initialization
    int i = 0, j = 0;
    int qPmod6 = qP % 6;
    int qPdiv6 = qP / 6;

    // Quantization
    if (qP > 35)
    {
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                d[i][j] = (c[i][j] * sps->LevelScale8x8[YCbCr][qPmod6][i][j]) << (qPdiv6 - 6);
    }
    else
    {
        int mypow = (int)pow(2, 5 - qPdiv6);

        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                d[i][j] = (c[i][j] * sps->LevelScale8x8[YCbCr][qPmod6][i][j] + mypow) >> (6 - qPdiv6);
    }
/*
    The bitstream shall not contain data that result in any element dij of d with i, j = 0..7 that exceeds the range of integer
    values from -2(7 + bitDepth) to 2(7 + bitDepth) - 1, inclusive.
*/
}

/* ************************************************************************** */

/*!
 * \param d[][] A quantized residual block of luma or chroma component.
 * \param r[][] A quantized and transformed residual sample array.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.13.2 Transformation process for residual 8x8 blocks.
 */
static void idct8x8(const int d[8][8], int r[8][8])
{
    TRACE_2(TRANS, " idct8x8()");

    int e[8][8];
    int f[8][8];
    int g[8][8];
    int h[8][8];
    int m[8][8];
    int k[8][8];

    int i = 0, j = 0;

    for (i = 0; i < 8; i++)
    {
        e[i][0] = d[i][0] + d[i][4];
        e[i][1] = -d[i][3] + d[i][5] - d[i][7] - (d[i][7] >> 1);
        e[i][2] = d[i][0] - d[i][4];
        e[i][3] = d[i][1] + d[i][7] - d[i][3] - (d[i][3] >> 1);
        e[i][4] = (d[i][2] >> 1) - d[i][6];
        e[i][5] = -d[i][1] + d[i][7] + d[i][5] + (d[i][5] >> 1);
        e[i][6] = d[i][2] + (d[i][6] >> 1);
        e[i][7] = d[i][3] + d[i][5] + d[i][1] + (d[i][1] >> 1);
    }

    for (i = 0; i < 8; i++)
    {
        f[i][0] = e[i][0] + e[i][6];
        f[i][1] = e[i][1] + (e[i][7] >> 2);
        f[i][2] = e[i][2] + e[i][4];
        f[i][3] = e[i][3] + (e[i][5] >> 2);
        f[i][4] = e[i][2] - e[i][4];
        f[i][5] = (e[i][3] >> 2) - e[i][5];
        f[i][6] = e[i][0] - e[i][6];
        f[i][7] = e[i][7] - (e[i][1] >> 2);
    }

    for (i = 0; i < 8; i++)
    {
        g[i][0] = f[i][0] + f[i][7];
        g[i][1] = f[i][2] + f[i][5];
        g[i][2] = f[i][4] + f[i][3];
        g[i][3] = f[i][6] + f[i][1];
        g[i][4] = f[i][6] - f[i][1];
        g[i][5] = f[i][4] - f[i][3];
        g[i][6] = f[i][2] - f[i][5];
        g[i][7] = f[i][0] - f[i][7];
    }

    for (j = 0; j < 8; j++)
    {
        h[0][j] = g[0][j] + g[4][j];
        h[1][j] = - g[3][j] + g[5][j] - g[7][j] - (g[7][j] >> 1);
        h[2][j] = g[0][j] - g[4][j];
        h[3][j] = g[1][j] + g[7][j] - g[3][j] - (g[3][j] >> 1);
        h[4][j] = (g[2][j] >> 1) - g[6][j];
        h[5][j] = - g[1][j] + g[7][j] + g[5][j] + (g[5][j] >> 1);
        h[6][j] = g[2][j] + (g[6][j] >> 1);
        h[7][j] = g[3][j] + g[5][j] + g[1][j] + (g[1][j] >> 1);
    }

    for (j = 0; j < 8; j++)
    {
        k[0][j] = h[0][j] + h[6][j];
        k[1][j] = h[1][j] + (h[7][j] >> 2);
        k[2][j] = h[2][j] + h[4][j];
        k[3][j] = h[3][j] + (h[5][j] >> 2);
        k[4][j] = h[2][j] - h[4][j];
        k[5][j] = (h[3][j] >> 2) - h[5][j];
        k[6][j] = h[0][j] - h[6][j];
        k[7][j] = h[7][j] - (h[1][j] >> 2);
    }

    for (j = 0; j < 8; j++)
    {
        m[0][j] = k[0][j] + k[7][j];
        m[1][j] = k[2][j] + k[5][j];
        m[2][j] = k[4][j] + k[3][j];
        m[3][j] = k[6][j] + k[1][j];
        m[4][j] = k[6][j] - k[1][j];
        m[5][j] = k[4][j] - k[3][j];
        m[6][j] = k[2][j] - k[5][j];
        m[7][j] = k[0][j] - k[7][j];
    }

    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            r[i][j] = (m[i][j] + 32) >> 6;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param blkIdx The index of the current block.
 * \param u A transformed block of luma or chroma component.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.14 Picture construction process prior to deblocking filter process.
 *
 * Note that MbaffFrameFlag support has been removed from this function.
 */
static int picture_construction_process_4x4(DecodingContext_t *dc, const int blkIdx, const int u[4][4])
{
    TRACE_1(TRANS, "> " BLD_GREEN "picture_construction_process_4x4()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int i = 0, j = 0;
    int xO = 0, yO = 0;

    InverseLuma4x4BlkScan(blkIdx, &xO, &yO);
    TRACE_2(TRANS, "xO yO : %i - %i", xO, yO);

    // Derivation of upper-left luma sample of the current macroblock
    //int xP = 0, yP = 0;
    //InverseMacroblockScan(mb->mbAddr, false, sps->PicWidthInSamplesL, &xP, &yP);
    //TRACE_3(TRANS, "xP yP : %i - %i", xP, yP);

    // Picture construction
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            mb->SprimeL[/*xP + */xO + j][/*yP + */yO + i] = (uint8_t)u[i][j];

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param blkIdx The index of the current block.
 * \param u A transformed block of luma or chroma component.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.14 Picture construction process prior to deblocking filter process.
 *
 * Note that MbaffFrameFlag support has been removed from this function.
 * Note that ChromaArrayType == 3 support has been removed from this function.
 */
static int picture_construction_process_4x4chroma(DecodingContext_t *dc, const int YCbCr,
                                                  const int blkIdx, const int u[4][4])
{
    TRACE_1(TRANS, "> " BLD_GREEN "picture_construction_process_4x4chroma()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int i = 0, j = 0;
    int xO = 0, yO = 0;

    InverseChroma4x4BlkScan(blkIdx, &xO, &yO);
    TRACE_2(TRANS, "xO yO : %i - %i", xO, yO);
/*
    // Derivation of upper-left luma sample of the current macroblock
    int xP = 0, yP = 0;
    InverseMacroblockScan(mb->mbAddr, false, sps->PicWidthInSamplesL, &xP, &yP);
    TRACE_3(TRANS, "xP yP : %i - %i", xP, yP);
*/
    // Picture construction
    if (YCbCr == 1)
    {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                mb->SprimeCb[/*xP / sps->subWidthC + */xO + j][/*yP + sps->subHeightC + */yO + i] = (uint8_t)u[i][j];
    }
    else if (YCbCr == 2)
    {
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                mb->SprimeCr[/*xP / sps->subWidthC + */xO + j][/*yP + sps->subHeightC + */yO + i] = (uint8_t)u[i][j];
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param blkIdx The index of the current block.
 * \param u[][] A transformed block of luma or chroma component.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.14 Picture construction process prior to deblocking filter process.
 *
 * Note that MbaffFrameFlag support has been removed from this function.
 */
static int picture_construction_process_8x8(DecodingContext_t *dc, const int blkIdx,
                                            const int u[8][8])
{
    TRACE_1(TRANS, "> " BLD_GREEN "picture_construction_process_8x8()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int i = 0, j = 0;
    int xO = 0, yO = 0;

    InverseLuma8x8BlkScan(blkIdx, &xO, &yO);
    TRACE_2(TRANS, "xO yO : %i - %i", xO, yO);
/*
    // Derivation of upper-left luma sample of the current macroblock
    int xP = 0, yP = 0;
    InverseMacroblockScan(mb->mbAddr, false, sps->PicWidthInSamplesL, &xP, &yP);
    TRACE_3(TRANS, "xP yP : %i - %i", xP, yP);
*/
    // Picture construction
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            mb->SprimeL[/*xP + */xO + j][/*yP + */yO + i] = (uint8_t)u[i][j];

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param YCbCr Indicate if we are dealing with a Y (0), Cb (1) or Cr (2) block.
 * \param blkIdx The index of the current block.
 * \param u A transformed block of luma or chroma component.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.14 Picture construction process prior to deblocking filter process.
 *
 * Note that MbaffFrameFlag support has been removed from this function.
 * Note that ChromaArrayType == 3 support has been removed from this function.
 */
static int picture_construction_process_8x8chroma(DecodingContext_t *dc, const int YCbCr,
                                                  const int blkIdx, const int u[8][8])
{
    TRACE_1(TRANS, "> " BLD_GREEN "picture_construction_process_8x8chroma()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    int i = 0, j = 0;
    int xO = 0, yO = 0;

    InverseChroma4x4BlkScan(blkIdx, &xO, &yO);
    TRACE_2(TRANS, "xO yO : %i - %i", xO, yO);
/*
    // Derivation of upper-left luma sample of the current macroblock
    int xP = 0, yP = 0;
    InverseMacroblockScan(mb->mbAddr, false, sps->PicWidthInSamplesL, &xP, &yP);
    TRACE_3(TRANS, "xP yP : %i - %i", xP, yP);
*/
    // Picture construction
    if (YCbCr == 1)
    {
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                mb->SprimeCb[/*xP / sps->subWidthC + */xO + j][/*yP + sps->subHeightC + */yO + i] = (uint8_t)u[i][j];
    }
    else if (YCbCr == 2)
    {
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                mb->SprimeCr[/*xP / sps->subWidthC + */xO + j][/*yP + sps->subHeightC + */yO + i] = (uint8_t)u[i][j];
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param u[][] A transformed block of luma or chroma component.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.14 Picture construction process prior to deblocking filter process.
 *
 * Note that MbaffFrameFlag and ChromaArrayType == 3 support has been removed from this function.
 */
static int picture_construction_process_16x16(DecodingContext_t *dc, const int u[16][16])
{
    TRACE_1(TRANS, "> " BLD_GREEN "picture_construction_process_16x16()" CLR_RESET);
    int retcode = SUCCESS;

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];
/*
    // Derivation of upper-left luma sample of the current macroblock
    int xP = 0, yP = 0;
    InverseMacroblockScan(mb->mbAddr, false, sps->PicWidthInSamplesL, &xP, &yP);
    TRACE_3(TRANS, "xP yP : %i - %i", xP, yP);
*/

    // Picture construction
#if ENABLE_MBAFF
    if (dc->active_slice->MbaffFrameFlag)
    {
        TRACE_ERROR(TRANS, ">>> UNSUPPORTED (MbaffFrameFlag)")
        return UNSUPPORTED;
    }
    else
#endif // ENABLE_MBAFF
    {
        int i = 0, j = 0;
        for (i = 0; i < 16; i++)
            for (j = 0; j < 16; j++)
                mb->SprimeL[/*xP + */ + j][/*yP + */ + i] = (uint8_t)u[i][j];
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current Decoding Context.
 * \param horPredFlag docme.
 * \param **r docme.
 * \param nW docme.
 * \param nH docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.5.15 Intra residual transform-bypass decoding process.
 */
static int **transformbypass_decoding(DecodingContext_t *dc, bool horPredFlag,
                                      int **r, int nW, int nH)
{
    TRACE_2(TRANS, " transformbypass_decoding()");

    TRACE_WARNING(TRANS, ">>> UNIMPLEMENTED (transformbypass_decoding)");

    return NULL;
}

/* ************************************************************************** */
/* ************************************************************************** */
