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
 * \file      h264_cabac.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "h264_cabac.h"
#include "h264_cabac_tables.h"
#include "h264_spatial.h"
#include "../../utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

// C++ standard libraries
#include <algorithm>

/* ************************************************************************** */
/*
    // CABAC overview

    http://en.wikipedia.org/wiki/CABAC
    http://en.wikipedia.org/wiki/Arithmetic_coding

    CABAC = Context Adaptative Binary Arithmetic Coding
    C'est un codage arithmétique particulier (codage arithmétique binaire). Il
    ne fonctionne qu'avec deux subdivisions dans chaque intervalle. Entre chaque
    sub-division d'un intervalle les probabilités sont réajustées (adaptation au contexte).

    Alors ce qu'il faut savoir c'est que le décodage d'un élement de syntaxe est
    très dépendant de son type.
    Pour chaque appel a CABAC il faut donc spécifier quel est le type de l'élément
    à décoder, car les étapes 3/ et 4/ sont complétements dépendantes de cette information.
    La partie décodage arithmétique en elle même (étape 5/) est assez concise et
    générique avec une centaine de lignes de C. Les parties binarization et surtout
    récupération des contextes de probabilité feront quand à elles plus de 2000
    à 3000 lignes de codes.

    // CABAC decoding process

    (Si on est au début d'une slice)
    1/ On initialise les contextes de probabilités du décodeur
    2/ On initialise le décodeur

    Pour un élement de syntaxe donné :
    3/ On génére tous les résultats possibles ("binarization") sous forme de
       chaines de binaires ("bins strings")

    On boucle sur le processus de décodage d'un bin (un élément de la bins string) :
    4/ On récupère le contexte de probabilité du bin
    5/ On décode le bin
    6/ On compare la bins string obtenu avec les différents résultats de la binarization,
       et on continu dans la boucle tant que l'on n'a pas qu'une seul égalité.

    (Si on est à la fin d'une slice)
    7/ On ré-initialise le décodeur
*/
/* ************************************************************************** */

static int getBinarization(DecodingContext_t *dc,
                           SyntaxElementType_e seType, BlockType_e blkType,
                           binarization_t *prefix,
                           binarization_t *suffix);

    static int bp_U(void);
    static int bp_TU(const int cMax);
    static int bp_UEGk(int k, const bool signedValFlag, const int uCoff);
    static int bp_FL(const int cMax);
    static int bp_mbType(void);
    static int bp_CBP(void);
    static int bp_mbQPd(void);

static int decodingProcessFlow(DecodingContext_t *dc,
                               SyntaxElementType_e seType, BlockType_e blkType, const int blkIdx,
                               binarization_t *bin);

static int getCtxIdx(DecodingContext_t *dc, SyntaxElementType_e seType, BlockType_e blkType, const int blkIdx, const uint8_t decodedSE[32], int binIdx, const int maxBinIdxCtx, const int ctxIdxOffset);
        static int deriv_ctxIdxInc(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx, const int ctxIdxOffset);
        static int deriv_ctxIdxInc_mbtype(DecodingContext_t *dc, const int ctxIdxOffset);
        static int deriv_ctxIdxInc_cbp_luma(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx);
        static int deriv_ctxIdxInc_cbp_chroma(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx);
        static int deriv_ctxIdxInc_mbQPd(DecodingContext_t *dc);
        static int deriv_ctxIdxInc_intrachromapredmode(DecodingContext_t *dc);
        static int deriv_ctxIdxInc_transformsize8x8flag(DecodingContext_t *dc);
        static int deriv_ctxIdxInc_codedblockflag(DecodingContext_t *dc, const int blkType, const int blkIdx);
        static int assign_ctxIdxInc_priorvalues(const uint8_t decodedSE[32], const int binIdx, const int ctxIdxOffset);
        static int assign_ctxIdxInc_se(DecodingContext_t *dc, const SyntaxElementType_e seType, const BlockType_e blkType, const int binIdx);

    static int decodeBin(DecodingContext_t *dc, const int ctxIdx, const bool bypassFlag);
    static uint8_t DecodeDecision(DecodingContext_t *dc, const int ctxIdx);
    static void RenormD(CabacContext_t *cc, Bitstream_t *bitstr);
    static uint8_t DecodeBypass(CabacContext_t *cc, Bitstream_t *bitstr);
    static uint8_t DecodeTerminate(CabacContext_t *cc, Bitstream_t *bitstr);

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *coeffLevel docme.
 * \param startIdx docme.
 * \param endIdx docme.
 * \param maxNumCoeff The maximum number of coefficients to decode.
 * \param blkType The type of block to decode.
 * \param blkIdx The id of the current block.
 *
 * Syntax element details from 'ITU-T H.264' recommendation:
 * 7.4.5.3.3 Residual block CABAC semantics.
 *
 * coded_block_flag specifies whether the transform block contains non-zero
 * transform coefficient levels.
 *
 * significant_coeff_flag[i] specifies whether the transform coefficient level
 * at scanning position i is non-zero as follows.
 *
 * last_significant_coeff_flag[i] specifies for the scanning position i whether
 * there are non-zero transform coefficient levels for subsequent scanning
 * positions i + 1 to maxNumCoeff - 1 as follows.
 */
void residual_block_cabac(DecodingContext_t *dc, int *coeffLevel,
                          const int startIdx, const int endIdx,
                          const int maxNumCoeff,
                          const int blkType, const int blkIdx)
{
    TRACE_1(CABAC, "> " BLD_GREEN "residual_block_cabac()" CLR_RESET);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    // Shortcut
    Macroblock_t *mb = dc->mb_array[dc->CurrMbAddr];

    // Initialization
    bool significant_coeff_flag[64];
    bool last_significant_coeff_flag[64];
    bool coeff_sign_flag[64]; //[endIdx + 1]
    int coeff_abs_level_minus1[64]; //[endIdx + 1]
    bool cbf = false;
    int numCoeff = 0;
    int mycoefflevel = 0;
    int i = 0;

    // Init or reset some variables
    for (i = 0; i < maxNumCoeff; i++)
    {
        //coeffLevel[i] = 0;
        significant_coeff_flag[i] = false;
        last_significant_coeff_flag[i] = false;
        coeff_sign_flag[i] = false;
        coeff_abs_level_minus1[i] = 0;
    }

    mb->numDecodAbsLevelEq1 = 0;
    mb->numDecodAbsLevelGt1 = 0;
    mb->levelListIdx = 0;

    // Read and store coded_block_flag, if present
    if (maxNumCoeff != 64 || dc->ChromaArrayType == 3)
    {
        cbf = read_ae_blk(dc, SE_coded_block_flag, (BlockType_e)blkType, blkIdx);

        switch (blkType)
        {
            case blk_LUMA_16x16_DC:
                mb->coded_block_flag[0][maxNumCoeff] = cbf;
            break;
            case blk_CHROMA_DC_Cb:
                mb->coded_block_flag[1][maxNumCoeff] = cbf;
            break;
            case blk_CHROMA_DC_Cr:
                mb->coded_block_flag[2][maxNumCoeff] = cbf;
            break;
            case blk_CHROMA_AC_Cb:
                mb->coded_block_flag[1][blkIdx] = cbf;
            break;
            case blk_CHROMA_AC_Cr:
                mb->coded_block_flag[2][blkIdx] = cbf;
            break;
            default: // blk_LUMA_4x4 // blk_LUMA_16x16_AC
                mb->coded_block_flag[0][blkIdx] = cbf;
            break;
        }
    }
    else
    {
        cbf = true;
        mb->coded_block_flag[0][blkIdx] = true;
    }

    // Decode block if coefficients are present
    if (cbf == true)
    {
        numCoeff = endIdx + 1;
        i = startIdx;
        mb->levelListIdx = startIdx;

        // Find the number and position of coded coefficients
        while (i < (numCoeff - 1))
        {
           significant_coeff_flag[i] = read_ae_blk(dc, SE_significant_coeff_flag, (BlockType_e)blkType, blkIdx);
           if (significant_coeff_flag[i] == true)
           {
               last_significant_coeff_flag[i] = read_ae_blk(dc, SE_last_significant_coeff_flag, (BlockType_e)blkType, blkIdx);
               if (last_significant_coeff_flag[i] == true)
               {
                   numCoeff = i + 1;
               }
           }

           i++;
           mb->levelListIdx++;
        }

        // Decode last coefficient
        {
            // decode coeff
            mb->levelListIdx = numCoeff - 1;
            coeff_abs_level_minus1[numCoeff - 1] = read_ae_blk(dc, SE_coeff_abs_level_minus1, (BlockType_e)blkType, blkIdx);
            coeff_sign_flag[numCoeff - 1] = read_ae_blk(dc, SE_coeff_sign_flag, (BlockType_e)blkType, blkIdx);

            // update stats
            if ((coeff_abs_level_minus1[numCoeff - 1] + 1) == 1)
                mb->numDecodAbsLevelEq1++;
            else if ((coeff_abs_level_minus1[numCoeff - 1] + 1) > 1)
                mb->numDecodAbsLevelGt1++;

            // set last coeff
            mycoefflevel = (coeff_abs_level_minus1[numCoeff - 1] + 1) * (1 - 2*coeff_sign_flag[numCoeff - 1]);
            coeffLevel[numCoeff - 1] = mycoefflevel;
        }

        // Decode other coefficient
        for (i = numCoeff - 2; i >= startIdx; i--)
        {
            if (significant_coeff_flag[i] == true)
            {
                // decode coeff
                mb->levelListIdx = i;
                coeff_abs_level_minus1[i] = read_ae_blk(dc, SE_coeff_abs_level_minus1, (BlockType_e)blkType, blkIdx);
                coeff_sign_flag[i] = read_ae_blk(dc, SE_coeff_sign_flag, (BlockType_e)blkType, blkIdx);

                // update stats
                if ((coeff_abs_level_minus1[i] + 1) == 1)
                    mb->numDecodAbsLevelEq1++;
                else if ((coeff_abs_level_minus1[i] + 1) > 1)
                    mb->numDecodAbsLevelGt1++;

                // set coeff
                mycoefflevel = (coeff_abs_level_minus1[i] + 1) * (1 - 2*coeff_sign_flag[i]);
                coeffLevel[i] = mycoefflevel;
            }
        }
    }

#if ENABLE_DEBUG
    int frame_debug_range[2] = {-1, -1}; // Range of (idr) frame(s) to debug/analyse
    int mb_debug_range[2] = {-1, -1}; // Range of macroblock(s) to debug/analyse

    if ((int)(dc->idrCounter) >= frame_debug_range[0] && (int)(dc->idrCounter) <= frame_debug_range[1])
    {
        if ((int)(mb->mbAddr) >= mb_debug_range[0] && (int)(mb->mbAddr) <= mb_debug_range[1])
        {
            int a = 0;
            int iYCbCr = 0;

            if (blkType > blk_LUMA_16x16_AC)
            {
                if (blkType == blk_CHROMA_DC_Cb || blkType == blk_CHROMA_AC_Cb)
                    iYCbCr = 1;
                else
                    iYCbCr = 2;
            }

            printf("[CABAC] " BLD_BLUE "CABAC RESIDUAL BLOCK\n" CLR_RESET);

            printf("[CABAC] - from macroblock: %u\n", mb->mbAddr);
            printf("[CABAC] - blkIdx   :  %i/x\n", blkIdx);
            printf("[CABAC] - blkType  :  %i\n", blkType);
            printf("[CABAC] - iYCbCr   :  %i\n", iYCbCr);

            printf("[CABAC] - maxNumCoeff      = %i\n", maxNumCoeff);
            printf("[CABAC] - coded_block_flag = %i\n", cbf);

            if (cbf == true)
            {
                printf("[CABAC] - significant_coeff_flag\n");
                for (a = 0; a < numCoeff; a++)
                    printf("[CABAC]   - [idx:%2i] = %i\n", a, significant_coeff_flag[a]);

                printf("[CABAC] - last_significant_coeff_flag\n");
                for (a = 0; a < numCoeff; a++)
                    printf("[CABAC]   - [idx:%2i] = %i\n", a, last_significant_coeff_flag[a]);
            }

            if (numCoeff > 0)
            {
                printf("[CABAC]  - coefficients:\n");
                for (a = 0; a < numCoeff; a++)
                {
                    printf("[CABAC]   - [idx:%2i] coeff_abs_level_minus1 = %i\n", a, coeff_abs_level_minus1[a]);
                    printf("[CABAC]   -          coeff_sign_flag         = %i\n", coeff_sign_flag[a]);
                    printf("[CABAC]   -          coeff final             = %i\n", coeffLevel[a]);
                }
            }
            else
            {
                printf("[CABAC]  - no coefficients for this block\n");
            }
        }
    }
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief CABAC decoding process.
 * \param *dc The current DecodingContext.
 * \param seType The type of the syntax element we want to decode.
 * \return The value of the requested syntax element.
 *
 * This process is invoked when parsing syntax elements with descriptor ae(v) in
 * subclauses 7.3.4 and 7.3.5 when entropy_coding_mode_flag is equal to 1.
 *
 * Inputs to this process are a request for a value of a syntax element and
 * values of prior parsed syntax elements.
 *
 * Output of this process is the value of the syntax element.
 */
int read_ae(DecodingContext_t *dc, SyntaxElementType_e seType)
{
    TRACE_1(CABAC, BLD_GREEN "read_ae (" CLR_RESET "SE_Type %i" BLD_GREEN ") =============================" CLR_RESET, seType);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    // Initialization
    ////////////////////////////////////////////////////////////////////////////

    int SyntaxElementValue = -1;

    binarization_t prefix;
    prefix.SyntaxElementValue = -1;
    prefix.maxBinIdxCtx = -1;
    prefix.ctxIdxOffset = -1;
    prefix.bintable = NULL;
    prefix.bintable_x = -1;
    prefix.bintable_y = -1;
    prefix.bypassFlag = false;

    binarization_t suffix;
    suffix.SyntaxElementValue = -1;
    suffix.maxBinIdxCtx = -1;
    suffix.ctxIdxOffset = -1;
    suffix.bintable = NULL;
    suffix.bintable_x = -1;
    suffix.bintable_y = -1;
    suffix.bypassFlag = false;

    // Binarization process
    ////////////////////////////////////////////////////////////////////////////

    if (getBinarization(dc, seType, blk_UNKNOWN, &prefix, &suffix) == FAILURE)
    {
        TRACE_ERROR(CABAC, "Fatal error during Binarization process");
        exit(EXIT_FAILURE);
    }

    // Arithmetic decoding process
    ////////////////////////////////////////////////////////////////////////////

    // Prefix
    if (decodingProcessFlow(dc, seType, blk_UNKNOWN, blk_UNKNOWN, &prefix) == FAILURE)
    {
        TRACE_ERROR(CABAC, "Fatal error during Arithmetic (prefix) decoding process");
        exit(EXIT_FAILURE);
    }

    // Suffix?
    if (suffix.maxBinIdxCtx != -1)
    {
        if (decodingProcessFlow(dc, seType, blk_UNKNOWN, blk_UNKNOWN, &suffix) == FAILURE)
        {
            TRACE_ERROR(CABAC, "Fatal error during Arithmetic (suffix) decoding process");
            exit(EXIT_FAILURE);
        }

        if (seType == SE_coded_block_pattern)
        {
            SyntaxElementValue = prefix.SyntaxElementValue + (16 * suffix.SyntaxElementValue);
        }
        else
        {
            SyntaxElementValue = prefix.SyntaxElementValue + suffix.SyntaxElementValue;
        }
    }
    else
    {
        if (seType == SE_mb_qp_delta)
        {
            SyntaxElementValue = std::pow(-1.0, prefix.SyntaxElementValue+1) * std::ceil(prefix.SyntaxElementValue/2.0);
        }
        else
        {
            SyntaxElementValue = prefix.SyntaxElementValue;
        }
    }

    // Print
    TRACE_1(CABAC, "> read_ae = %i", SyntaxElementValue);

    // Return the Syntax Element value
    return SyntaxElementValue;
}

/* ************************************************************************** */

/*!
 * \brief CABAC decoding process.
 * \param *dc The current DecodingContext.
 * \param seType The type of the syntax element we want to decode.
 * \param blkType The type of the block we are currently decoding.
 * \param blkIdx The block index into the macroblock.
 * \return The value of the requested syntax element.
 *
 * This process is invoked when parsing syntax elements with descriptor ae(v) in
 * subclauses 7.3.4 and 7.3.5 when entropy_coding_mode_flag is equal to 1.
 *
 * Inputs to this process are a request for a value of a syntax element and
 * values of prior parsed syntax elements.
 *
 * Output of this process is the value of the syntax element.
 */
int read_ae_blk(DecodingContext_t *dc, SyntaxElementType_e seType, BlockType_e blkType, const int blkIdx)
{
    TRACE_1(CABAC, BLD_GREEN "read_ae_blk (" CLR_RESET "SE_Type %i" BLD_GREEN ") (" CLR_RESET "BLK_Type %i" BLD_GREEN ") (" CLR_RESET "BLK_Idx %i" BLD_GREEN ") =============================" CLR_RESET, seType, blkType, blkIdx);
    //bitstream_print_absolute_bit_offset(dc->bitstr);

    // Initialization
    ////////////////////////////////////////////////////////////////////////////

    int SyntaxElementValue = 0;

    binarization_t prefix;
    prefix.SyntaxElementValue = -1;
    prefix.maxBinIdxCtx = -1;
    prefix.ctxIdxOffset = -1;
    prefix.bintable = NULL;
    prefix.bintable_x = -1;
    prefix.bintable_y = -1;
    prefix.bypassFlag = false;

    binarization_t suffix;
    suffix.SyntaxElementValue = -1;
    suffix.maxBinIdxCtx = -1;
    suffix.ctxIdxOffset = -1;
    suffix.bintable = NULL;
    suffix.bintable_x = -1;
    suffix.bintable_y = -1;
    suffix.bypassFlag = false;

    // Binarization process
    ////////////////////////////////////////////////////////////////////////////

    if (getBinarization(dc, seType, blkType, &prefix, &suffix) == FAILURE)
    {
        TRACE_ERROR(CABAC, "Fatal error during Binarization process");
        exit(EXIT_FAILURE);
    }

    // Arithmetic decoding process
    ////////////////////////////////////////////////////////////////////////////

    // Prefix
    if (decodingProcessFlow(dc, seType, blkType, blkIdx, &prefix) == FAILURE)
    {
        TRACE_ERROR(CABAC, "Fatal error during Arithmetic (prefix) decoding process");
        exit(EXIT_FAILURE);
    }

    // Suffix?
    if (seType == SE_coeff_abs_level_minus1 && prefix.SyntaxElementValue >= 14)
    {
        if (decodingProcessFlow(dc, seType, blkType, blkIdx, &suffix) == FAILURE)
        {
            TRACE_ERROR(CABAC, "Fatal error during Arithmetic (suffix) decoding process");
            exit(EXIT_FAILURE);
        }

        SyntaxElementValue = prefix.SyntaxElementValue + suffix.SyntaxElementValue;
    }
    else
    {
        SyntaxElementValue = prefix.SyntaxElementValue;
    }

    // Print
    TRACE_1(CABAC, "> read_ae = %i", SyntaxElementValue);

    // Return the Syntax Element value
    return SyntaxElementValue;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Initialisation process for context variables.
 * \param *dc The current DecodingContext.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.1.1 Initialisation process for context variables.
 *
 * Outputs of this process are the initialised CABAC context variables indexed by ctxIdx.
 * For each context variable, the two variables pStateIdx and valMPS are initialised.
 */
int initCabacContextVariables(DecodingContext_t *dc)
{
    TRACE_1(CABAC, BLD_GREEN "initCabacContextVariables()" CLR_RESET);
    int retcode = SUCCESS;

    if ((dc->active_slice->cc = (CabacContext_t*)calloc(1, sizeof(CabacContext_t))) == NULL)
    {
        TRACE_ERROR(SLICE, "Unable to alloc new CabacContext!");
        retcode = FAILURE;
    }
    else
    {
        int ctxIdx = 0;
        int preCtxState = 0;

        for (ctxIdx = 0; ctxIdx < 460; ctxIdx++)
        {
            preCtxState = iClip3(1, 126, ((cabac_context_init_I[ctxIdx][0] * iClip3(0, 51, dc->active_slice->SliceQPY)) >> 4) + cabac_context_init_I[ctxIdx][1]);

            if (preCtxState < 64)
            {
                dc->active_slice->cc->pStateIdx[ctxIdx] = (63 - preCtxState);
                dc->active_slice->cc->valMPS[ctxIdx] = 0;
            }
            else
            {
                dc->active_slice->cc->pStateIdx[ctxIdx] = (preCtxState - 64);
                dc->active_slice->cc->valMPS[ctxIdx] = 1;
            }
        }

        // Note: ctxIdx equal to 276 is associated with the end_of_slice_flag and the bin of mb_type
        dc->active_slice->cc->pStateIdx[276] = 63;
        dc->active_slice->cc->valMPS[276] = 0;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Initialisation process for the arithmetic decoding engine.
 * \param *dc The current DecodingContext.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.1.2 Initialisation process for the arithmetic decoding engine.
 *
 * Outputs of this process are the initialised decoding engine registers
 * codIRange and codIOffset both in 16 bit register precision, respresenting the
 * status of the arithmetic decoding engine.
 */
int initCabacDecodingEngine(DecodingContext_t *dc)
{
    TRACE_1(CABAC, BLD_GREEN "initCabacDecodingEngine()" CLR_RESET);
    int retcode = SUCCESS;

    dc->active_slice->cc->codIRange = 510;
    dc->active_slice->cc->codIOffset = (int)read_bits(dc->bitstr, 9);

    if (dc->active_slice->cc->codIOffset == 510 ||
        dc->active_slice->cc->codIOffset == 511)
    {
        TRACE_ERROR(CABAC, "Error while loading codIOffset: must not be 510 nor 511!");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Binarization process.
 * \param *dc The current DecodingContext.
 * \param seType The type of syntax element we want to decode.
 * \param blkType The type of block we are decoding, correspond to ctxBlkCat.
 * \param *prefix Structure containing necessary informations about decoding prefix bins string.
 * \param *suffix Structure containing necessary informations about decoding suffix bins string.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2 Binarization process.
 *
 * Please note that this function can only binarize syntax elements from I slice.
 *
 * Input to this process is a request for a syntax element.
 * Output of this process is the binarization of the syntax element,
 * maxBinIdxCtx, ctxIdxOffset, and bypassFlag.
 */
static int getBinarization(DecodingContext_t *dc,
                           SyntaxElementType_e seType, BlockType_e blkType,
                           binarization_t *prefix, binarization_t *suffix)
{
    TRACE_1(CABAC, BLD_GREEN "getBinarization()" CLR_RESET);
    int retcode = FAILURE;

    switch (seType)
    {
        case SE_mb_type:
            prefix->maxBinIdxCtx = 6;
            prefix->ctxIdxOffset = 3;

            prefix->bintable = (uint8_t **)binarization_mbtype_I;
            prefix->bintable_x = 7;
            prefix->bintable_y = 26;

            retcode = SUCCESS; // bp_MBt(dc, prefix->maxBinIdxCtx, prefix->ctxIdxOffset);
        break;

        case SE_mvd_lx0:
            prefix->maxBinIdxCtx = 4;
            prefix->ctxIdxOffset = 40;
/*
            prefix->bintable_x = -1;
            prefix->bintable_y = -1;

            suffix->bypassFlag = true;

            retcode = bp_UEGk(3, 1, 9);
*/
        break;

        case SE_mvd_lx1:
            prefix->maxBinIdxCtx = 4;
            prefix->ctxIdxOffset = 47;
/*
            prefix->bintable_x = -1;
            prefix->bintable_y = -1;

            suffix->bypassFlag = true;

            retcode = bp_UEGk(3, 1, 9);
*/
        break;

        case SE_ref_idx_lx:
            prefix->maxBinIdxCtx = 2;
            prefix->ctxIdxOffset = 54;

            prefix->bintable = (uint8_t **)binarization_u;
            prefix->bintable_x = 32;
            prefix->bintable_y = 32;

            retcode = SUCCESS; // bp_U();
        break;

        case SE_mb_qp_delta:
            prefix->maxBinIdxCtx = 2;
            prefix->ctxIdxOffset = 60;

            prefix->bintable = (uint8_t **)binarization_u;
            prefix->bintable_x = 32;
            prefix->bintable_y = 32;

            retcode = SUCCESS; // bp_mbQPd();
        break;

        case SE_intra_chroma_pred_mode:
            prefix->maxBinIdxCtx = 1;
            prefix->ctxIdxOffset = 64;

            prefix->bintable = (uint8_t **)binarization_tu3;
            prefix->bintable_x = 3;
            prefix->bintable_y = 4;

            retcode = SUCCESS; // bp_TU(3);
        break;

        case SE_prev_intraxxx_pred_mode_flag:
            prefix->maxBinIdxCtx = 0;
            prefix->ctxIdxOffset = 68;

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_rem_intraxxx_pred_mode:
            prefix->maxBinIdxCtx = 0;
            prefix->ctxIdxOffset = 69;

            prefix->bintable = (uint8_t **)binarization_fl7;
            prefix->bintable_x = 3;
            prefix->bintable_y = 8;

            retcode = SUCCESS; // bp_FL(7);
        break;

        case SE_mb_field_decoding_flag:
            prefix->maxBinIdxCtx = 0;
            prefix->ctxIdxOffset = 70;

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_coded_block_pattern:
            prefix->maxBinIdxCtx = 3;
            prefix->ctxIdxOffset = 73;
            prefix->bintable = (uint8_t **)binarization_fl15;
            prefix->bintable_x = 4;
            prefix->bintable_y = 16;

            suffix->maxBinIdxCtx = 1;
            suffix->ctxIdxOffset = 77;
            suffix->bintable = (uint8_t **)binarization_tu2;
            suffix->bintable_x = 2;
            suffix->bintable_y = 3;

            retcode = SUCCESS; // bp_CBP();
        break;

        case SE_coded_block_flag:
            prefix->maxBinIdxCtx = 0;

            if (blkType == blk_LUMA_8x8)
                prefix->ctxIdxOffset = 1012;
            else
                prefix->ctxIdxOffset = 85;

#if ENABLE_SEPARATE_COLOUR_PLANES
            //UNSUPPORTED separate_colour_plane_flag
            if (ctxBlockCat == 5 || ctxBlockCat == 9 || ctxBlockCat == 13)
                prefix->ctxIdxOffset = 1012;
            else if (ctxBlockCat < 5)
                prefix->ctxIdxOffset = 85;
            else if (ctxBlockCat < 9)
                prefix->ctxIdxOffset = 460;
            else if (ctxBlockCat < 13)
                prefix->ctxIdxOffset = 472;
#endif // ENABLE_SEPARATE_COLOUR_PLANES

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_significant_coeff_flag:
            prefix->maxBinIdxCtx = 0;

            if (dc->active_slice->mb_field_decoding_flag == false)
            {
                // For frame coded block only
                if (blkType == blk_LUMA_8x8)
                    prefix->ctxIdxOffset = 402;
                else
                    prefix->ctxIdxOffset = 105;

#if ENABLE_SEPARATE_COLOUR_PLANES
                //UNSUPPORTED separate_colour_plane_flag
                if (ctxBlockCat < 5)
                    prefix->ctxIdxOffset = 105;
                else if (ctxBlockCat == 5)
                    prefix->ctxIdxOffset = 402;
                else if (ctxBlockCat < 9)
                    prefix->ctxIdxOffset = 484;
                else if (ctxBlockCat == 9)
                    prefix->ctxIdxOffset = 660;
                else if (ctxBlockCat < 13)
                    prefix->ctxIdxOffset = 528;
                else if (ctxBlockCat == 13)
                    prefix->ctxIdxOffset = 718;
#endif // ENABLE_SEPARATE_COLOUR_PLANES
            }
            else
            {
                // For field coded block only
                if (blkType == blk_LUMA_8x8)
                    prefix->ctxIdxOffset = 436;
                else
                    prefix->ctxIdxOffset = 277;

#if ENABLE_SEPARATE_COLOUR_PLANES
                //UNSUPPORTED separate_colour_plane_flag
                if (ctxBlockCat < 5)
                    prefix->ctxIdxOffset = 277;
                else if (ctxBlockCat == 5)
                    prefix->ctxIdxOffset = 436;
                else if (ctxBlockCat < 9)
                    prefix->ctxIdxOffset = 776;
                else if (ctxBlockCat == 9)
                    prefix->ctxIdxOffset = 675;
                else if (ctxBlockCat < 13)
                    prefix->ctxIdxOffset = 820;
                else if (ctxBlockCat == 13)
                    prefix->ctxIdxOffset = 733;
#endif // ENABLE_SEPARATE_COLOUR_PLANES
            }

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_last_significant_coeff_flag:
            prefix->maxBinIdxCtx = 0;

            if (dc->active_slice->mb_field_decoding_flag == false)
            {
                // For frame coded block only
                if (blkType == blk_LUMA_8x8)
                    prefix->ctxIdxOffset = 417;
                else
                    prefix->ctxIdxOffset = 166;

#if ENABLE_SEPARATE_COLOUR_PLANES
                //UNSUPPORTED separate_colour_plane_flag
                if (ctxBlockCat < 5)
                    prefix->ctxIdxOffset = 166;
                else if (ctxBlockCat == 5)
                    prefix->ctxIdxOffset = 417;
                else if (ctxBlockCat < 9)
                    prefix->ctxIdxOffset = 572;
                else if (ctxBlockCat == 9)
                    prefix->ctxIdxOffset = 690;
                else if (ctxBlockCat < 13)
                    prefix->ctxIdxOffset = 616;
                else if (ctxBlockCat == 13)
                    prefix->ctxIdxOffset = 748;
#endif // ENABLE_SEPARATE_COLOUR_PLANES
            }
            else
            {
                // For field coded block only
                if (blkType == blk_LUMA_8x8)
                    prefix->ctxIdxOffset = 451;
                else
                    prefix->ctxIdxOffset = 338;

#if ENABLE_SEPARATE_COLOUR_PLANES
                //UNSUPPORTED separate_colour_plane_flag
                if (ctxBlockCat < 5)
                    prefix->ctxIdxOffset = 338;
                else if (ctxBlockCat == 5)
                    prefix->ctxIdxOffset = 451;
                else if (ctxBlockCat < 9)
                    prefix->ctxIdxOffset = 864;
                else if (ctxBlockCat == 9)
                    prefix->ctxIdxOffset = 699;
                else if (ctxBlockCat < 13)
                    prefix->ctxIdxOffset = 908;
                else if (ctxBlockCat == 13)
                    prefix->ctxIdxOffset = 757;
#endif // ENABLE_SEPARATE_COLOUR_PLANES
            }

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_coeff_abs_level_minus1:
            prefix->maxBinIdxCtx = 1;

            if (blkType == blk_LUMA_8x8)
                prefix->ctxIdxOffset = 426;
            else
                prefix->ctxIdxOffset = 227;

#if ENABLE_SEPARATE_COLOUR_PLANES
            //UNSUPPORTED separate_colour_plane_flag
            if (ctxBlockCat < 5)
                prefix->ctxIdxOffset = 227;
            else if (ctxBlockCat == 5)
                prefix->ctxIdxOffset = 426;
            else if (ctxBlockCat < 9)
                prefix->ctxIdxOffset = 952;
            else if (ctxBlockCat == 9)
                prefix->ctxIdxOffset = 708;
            else if (ctxBlockCat < 13)
                prefix->ctxIdxOffset = 982;
            else if (ctxBlockCat == 13)
                prefix->ctxIdxOffset = 766;
#endif // ENABLE_SEPARATE_COLOUR_PLANES

            prefix->bintable = (uint8_t **)binarization_tu14;
            prefix->bintable_x = 14;
            prefix->bintable_y = 15;

            suffix->bintable = (uint8_t **)binarization_eg0;
            suffix->bintable_x = 19;
            suffix->bintable_y = 1023;
            suffix->bypassFlag = true;

            retcode = SUCCESS; // bp_UEGk(0, 0, 14);
        break;

        case SE_coeff_sign_flag:
            prefix->maxBinIdxCtx = 0;
            prefix->bypassFlag = true;

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_transform_size_8x8_flag:
            prefix->maxBinIdxCtx = 0;
            prefix->ctxIdxOffset = 399;

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        case SE_end_of_slice_flag:
            prefix->maxBinIdxCtx = 0;
            prefix->ctxIdxOffset = 276;

            prefix->bintable = (uint8_t **)binarization_fl1;
            prefix->bintable_x = 1;
            prefix->bintable_y = 2;

            retcode = SUCCESS; // bp_FL(1);
        break;

        default:
            TRACE_ERROR(CABAC, "Unable to select binarization method!");
            break;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Unary (U) binarization process.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.1 Unary (U) binarization process.
 *
 * Input to this process is a request for a U binarization for a syntax element.
 * Output of this process is the U binarization of the syntax element.
 */
static int bp_U(void)
{
    TRACE_2(CABAC, BLD_GREEN " bp_U()" CLR_RESET);

    // Use static table

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief TU binarization process.
 * \param cMax docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.2 Truncated unary (TU) binarization process.
 *
 * Input to this process is a request for a TU binarization for a syntax element and cMax.
 * Output of this process is the TU binarization of the syntax element.
 */
static int bp_TU(const int cMax)
{
    TRACE_2(CABAC, BLD_GREEN " bp_TU()" CLR_RESET);

    // Use static tables

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief UEGk binarization process.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.3 Concatenated unary/ k-th order Exp-Golomb (UEGk) binarization process.
 *
 * UEGk binarization process is used for the binarization of the syntax elements
 * mvd_lX (X = 0, 1) and coeff_abs_level_minus1.
 * Input to this process is a request for a UEGk binarization for a syntax element, signedValFlag and uCoff.
 * Output of this process is the UEGk binarization of the syntax element.
 */
static int bp_UEGk(int k, const bool signedValFlag, const int uCoff)
{
    TRACE_2(CABAC, BLD_GREEN " bp_UEGk()" CLR_RESET);
    int retcode = FAILURE;

    // Prefix
    // TU binarization with cMax = uCoff

    // Suffix
    // Exponentiel Golomb code, k order

    // Suffix string generation example
    int synElVal = 0;
    while (synElVal < 1024)
    {
        if (synElVal >= uCoff)
        {
            int sufS = synElVal - uCoff;
            bool stopLoop = false;

            //printf("%i : ", synElVal+1);
            printf("{");

            do
            {
                if (sufS >= (1 << k))
                {
                    // prefix
                    printf("1, ");//binstring[pos] = 1;//put(1);
                    sufS -= (1 << k);
                    k++;
                }
                else
                {
                    // end prefix
                    printf("0, ");//binstring[pos] = 0;//put(0);

                    // suffix
                    while (k > 0)
                    {
                        k--;
                        printf("%i, ", ((sufS >> k) & 0x01));//binstring[pos] = (sufS >> k) & 1;//put((sufS >> k) & 1);
                    }

                    stopLoop = true;
                }
            } while (!stopLoop);

            printf("}\n");
        }

        synElVal++;
    }
/*
    // Sign bit happend to the suffix
    if (signedValFlag == true && synElVal != 0)
    {
        if (synElVal > 0)
            printf("0t ");//binstring[pos] = 0;//put(0);
        else
            printf("1t ");//binstring[pos] = 1;//put(1);
    }
*/
    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief FL binarization process.
 * \param cMax docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.4 Fixed-length (FL) binarization process
 *
 * Input to this process is a request for a FL binarization for a syntax element and cMax.
 * Output of this process is the FL binarization of the syntax element.
 */
static int bp_FL(const int cMax)
{
    TRACE_2(CABAC, BLD_GREEN " bp_FL()" CLR_RESET);

    // Use static table for cMax=1, cMax=3, cMax=7, cMax=15

    // To compute the length of the bin strings
    //int fixedLength = (int)ceil(log2((double)(cMax + 1)));

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief  binarization process.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.5 Binarization process for macroblock type and sub-macroblock type.
 *
 * Input to this process is a request for a binarization for syntax elements mb_type or sub_mb_type[].
 * Output of this process is the binarization of the syntax element.
 */
static int bp_mbType(void)
{
    TRACE_2(CABAC, BLD_GREEN " bp_mbType()" CLR_RESET);

    // Use static table

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief coded block pattern binarization process.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.6 Binarization process for coded block pattern.
 *
 * Input to this process is a request for a binarization for the syntax element coded_block_pattern.
 * Output of this process is the binarization of the syntax element.
 */
static int bp_CBP(void)
{
    TRACE_2(CABAC, BLD_GREEN " bp_CBP()" CLR_RESET);

    // (prefix)
    // Use static table of bp_FL(), cMax=15

    // (suffix, only when ChromaArrayType is 1 or 2)
    // Use static table of bp_TU(), cMax=2

    return SUCCESS;
}

/* ************************************************************************** */

/*!
 * \brief mb_qp_delta binarization process.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.2.7 Binarization process for mb_qp_delta.
 *
 * Input to this process is a request for a binarization for the syntax element mb_qp_delta.
 * Output of this process is the binarization of the syntax element.
 */
static int bp_mbQPd(void)
{
    TRACE_2(CABAC, BLD_GREEN " bp_mbQPd()" CLR_RESET);
    int retcode = SUCCESS;

    // Use static table (U binarization) mapped by Table 9-3.

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Decoding process flow.
 * \param *dc The current DecodingContext.
 * \param seType The type of syntax element we want to decode.
 * \param blkType The type of coefficient block we are decoding.
 * \param blkIdx The id of the coefficient block inside its macroblock.
 * \param *bin docme.
 * \return SUCCESS or FAILURE.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3 Decoding process flow.
 *
 * Input to this process is a binarization of the requested syntax element,
 * maxBinIdxCtx, bypassFlag and ctxIdxOffset as specified in subclause 9.3.2.
 * Output of this process is the value of the syntax element.
 */
static int decodingProcessFlow(DecodingContext_t *dc,
                               SyntaxElementType_e seType,
                               BlockType_e blkType,
                               const int blkIdx,
                               binarization_t *bin)
{
    TRACE_1(CABAC, BLD_GREEN "decodingProcessFlow()" CLR_RESET);
    int retcode = FAILURE;

    int binIdx = -1;
    int ctxIdx = -1;
    int match = 999;

    uint8_t decodedSE[32];
    //memset(decodedSE, 255, sizeof(decodedSE));

    {
        // Print status
        TRACE_1(CABAC, "maxBinIdxCtx (%i)", bin->maxBinIdxCtx);
        TRACE_1(CABAC, "ctxIdxOffset (%i)", bin->ctxIdxOffset);
        TRACE_1(CABAC, "bypassFlag (%i)", bin->bypassFlag);
        TRACE_1(CABAC, "levelListIdx (%i)", dc->mb_array[dc->CurrMbAddr]->levelListIdx);
    }

    // Start decoding bin string
    while (match > 1)
    {
        // Next bin
        binIdx++;

        // Decode bin
        ////////////////////////////////////////////////////////////////////////

        if (bin->bypassFlag == true)
        {
            decodedSE[binIdx] = DecodeBypass(dc->active_slice->cc, dc->bitstr);
        }
        else
        {
            //1. Given binIdx, maxBinIdxCtx and ctxIdxOffset, ctxIdx is derived.
            ctxIdx = getCtxIdx(dc, seType, blkType, blkIdx, decodedSE, binIdx, bin->maxBinIdxCtx, bin->ctxIdxOffset);

            //2. Given ctxIdx, the value of the bin from the bitstream is decoded.
            if (ctxIdx == -1)
            {
                TRACE_ERROR(CABAC, "Warning, computation of ctxIdx failed!");
                return FAILURE;
            }
            else if (ctxIdx == 276)
            {
                // Use the special '9.3.3.2.1' process (DecodeTerminate) instead of regular 9.3.3.2.4 process (DecodeDecision)
                decodedSE[binIdx] = DecodeTerminate(dc->active_slice->cc, dc->bitstr);
            }
#if ENABLE_DEBUG
            else if (ctxIdx > 459)
            {
                TRACE_ERROR(CABAC, "Error, ctxIdx value is too high, please extend cabac_context_init_I table!");
                return FAILURE;
            }
#endif // ENABLE_DEBUG
            else
            {
                decodedSE[binIdx] = DecodeDecision(dc, ctxIdx);
            }
        }

#if ENABLE_DEBUG
/*
        // Print bin string
        ////////////////////////////////////////////////////////////////////////
        {
            TRACE_2(CABAC, "binIdx (%i)  |  bin string ( ", binIdx);
            int i = 0;
            while (i <= binIdx)
            {
                printf("%i ", decodedSE[i]);
                i++;
            }
            printf(")\n");
        }
*/
#endif // ENABLE_DEBUG

        // Compare bin string with binarization
        ////////////////////////////////////////////////////////////////////////
        {
            int i = 0;
            match = 0;

            while (i < bin->bintable_y)
            {
                if (memcmp(decodedSE, (((uint8_t *)(bin->bintable)) + (i*(bin->bintable_x))), binIdx+1) == 0)
                {
                    //TRACE_3(CABAC, "binIdx (%i)  |  (MATCH binarization scheme [%i])", binIdx, i);
                    bin->SyntaxElementValue = i;
                    match++;
                }

                i++;
            }
        }

        // Check result of comparison
        ////////////////////////////////////////////////////////////////////////
        if (match == 1)
        {
            TRACE_3(CABAC, "> 1 MATCH, we can stop now");
            retcode = SUCCESS;
        }
        else if (match == 0)
        {
            TRACE_ERROR(CABAC, "> 0 MATCH, that should not happend");
            retcode = FAILURE;
        }
    }
/*
    // FIXME Re-init decoding engine? see Figure 9-1
    In case the request for a value of a syntax element is processed for the syntax element mb_type and the decoded value of
    mb_type is equal to I_PCM, the decoding engine is initialised after the decoding of any pcm_alignment_zero_bit and all
    pcm_sample_luma and pcm_sample_chroma data as specified in clause 9.3.1.2.
*/
    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Derivation process for ctxIdx.
 * \param *dc The current DecodingContext.
 * \param seType The type of syntax element we want to decode.
 * \param blkType The type of coefficient block we are decoding.
 * \param blkIdx The id of the coefficient block inside its macroblock.
 * \param decodedSE[] docme.
 * \param binIdx docme.
 * \param maxBinIdxCtx docme.
 * \param ctxIdxOffset docme.
 * \return ctxIdx docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1 Derivation process for ctxIdx.
 */
static int getCtxIdx(DecodingContext_t *dc, SyntaxElementType_e seType, BlockType_e blkType,
                     const int blkIdx, const uint8_t decodedSE[32],
                     int binIdx, const int maxBinIdxCtx, const int ctxIdxOffset)
{
    TRACE_2(CABAC, BLD_GREEN " getCtxIdx()" CLR_RESET);

    int ctxIdx = -1, ctxIdxInc = -1;
    bool ctxIdxOffset_intable_flag = false;

    if (binIdx > maxBinIdxCtx)
    {
        binIdx = maxBinIdxCtx;
        TRACE_3(CABAC, " binIdx > maxBinIdxCtx (%i < %i)", binIdx, maxBinIdxCtx);
    }

    int i = -1;
    while (i < 21)
    {
        i++;

        if (ctxIdxOffset == ctxIdxOffset_table[i])
        {
            ctxIdxOffset_intable_flag = true;
            i = 22;
        }
    }

    if (ctxIdxOffset_intable_flag)
    {
/*
        If the ctxIdxOffset is listed in Table 9-39, the ctxIdx for a binIdx is the
        sum of ctxIdxOffset and ctxIdxInc, which is found in Table 9-39.
*/
        ctxIdxInc = deriv_ctxIdxInc(dc, decodedSE, binIdx, ctxIdxOffset);

        if (ctxIdxInc == -1)
        {
            TRACE_ERROR(CABAC, "Warning, computation of ctxIdxInc failed!");
            ctxIdx = -1;
        }
        else
        {
            ctxIdx = ctxIdxOffset + ctxIdxInc;
        }
    }
    else
    {
/*
        Otherwise (ctxIdxOffset is not listed in Table 9-39), the ctxIdx is specified
        to be the sum of the following terms:
        ctxIdxOffset and ctxIdxBlockCatOffset(ctxBlockCat) as specified in Table 9-40
        and ctxIdxInc(ctxBlockCat).
        Subclause 9.3.3.1.1.9 specifies the assignment of ctxIdxInc(ctxBlockCat) for coded_block_flag, and
        subclause 9.3.3.1.3 assign_ctxIdxInc_se() specifies the assignment of ctxIdxInc(ctxBlockCat) for
        significant_coeff_flag, last_significant_coeff_flag, and coeff_abs_level_minus1.
*/
        if (seType == SE_coded_block_flag)
        {
            ctxIdxInc = deriv_ctxIdxInc_codedblockflag(dc, blkType, blkIdx);

            if (ctxIdxInc == -1)
            {
                TRACE_ERROR(CABAC, "Warning, computation of ctxIdxInc failed!");
                ctxIdx = -1;
            }
            else
            {
                ctxIdx = ctxIdxOffset + ctxIdxBlkTypeOffset_values[0][blkType] + ctxIdxInc;
            }
        }
        else if (seType == SE_significant_coeff_flag ||
                 seType == SE_last_significant_coeff_flag ||
                 seType == SE_coeff_abs_level_minus1)
        {
            ctxIdx = ctxIdxOffset + ctxIdxBlkTypeOffset_values[seType - 12][blkType] + assign_ctxIdxInc_se(dc, seType, blkType, binIdx);
        }
        else
        {
            TRACE_ERROR(CABAC, "error: seType out of range inside getCtxIdx()");
            return -1;
        }
    }

    TRACE_2(CABAC, " ctxIdx = %i", ctxIdx);
    return ctxIdx;
}

/* ************************************************************************** */

/*!
 * \return ctxIdxInc docme.
 */
static int deriv_ctxIdxInc(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx, const int ctxIdxOffset)
{
    TRACE_2(CABAC, BLD_GREEN "  ctxIdxInc()" CLR_RESET);
    int ctxIdxInc = -1;

    switch (ctxIdxOffset)
    {
        case 0:
            if (binIdx == 0)
                ctxIdxInc = deriv_ctxIdxInc_mbtype(dc, ctxIdxOffset);
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=0 and binIdx!=0 should not happend"); }
        break;

        case 3:
            if (binIdx == 0)
                ctxIdxInc = deriv_ctxIdxInc_mbtype(dc, ctxIdxOffset);
            else if (binIdx == 1)
                ctxIdxInc = 273;
            else if (binIdx < 4)
                ctxIdxInc = binIdx + 1;
            else if (binIdx < 6)
                ctxIdxInc = assign_ctxIdxInc_priorvalues(decodedSE, binIdx, ctxIdxOffset);
            else
                ctxIdxInc = binIdx + 1;
        break;
/*
        case 11: 14: 17: 21: 24: 27: 32: 36: 40: 47: 54:
            // UNIMPLEMENTED
        break;
*/
        case 60:
            if (binIdx == 0)
                ctxIdxInc = deriv_ctxIdxInc_mbQPd(dc);
            else if (binIdx == 1)
                ctxIdxInc = 2;
            else
                ctxIdxInc = 3;
        break;

        case 64:
            if (binIdx == 0)
                ctxIdxInc = deriv_ctxIdxInc_intrachromapredmode(dc);
            else if (binIdx < 3)
                ctxIdxInc = 3;
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=64 and binIdx>2 should not happend"); }
        break;

        case 68:
            if (binIdx == 0)
                ctxIdxInc = 0;
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=68 and binIdx!=0 should not happend"); }
        break;

        case 69:
            if (binIdx == 0 || binIdx == 1 || binIdx == 2)
                ctxIdxInc = 0;
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=68 and binIdx!=0,1,2 should not happend"); }
        break;

        case 70:
            TRACE_ERROR(CABAC, ">>> UNIMPLEMENTED (deriv_ctxIdxInc with ctxIdxOffset=70 (SyntaxElement : mb_field_decoding_flag))");
        break;

        case 73:
            if (binIdx == 0 || binIdx == 1 || binIdx == 2 || binIdx == 3)
                ctxIdxInc = deriv_ctxIdxInc_cbp_luma(dc, decodedSE, binIdx);
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=73 and binIdx!=0,1,2,3 should not happend"); }
        break;

        case 77:
            if (binIdx == 0 || binIdx == 1)
                ctxIdxInc = deriv_ctxIdxInc_cbp_chroma(dc, decodedSE, binIdx);
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=77 and binIdx!=0,1 should not happend"); }
        break;

        case 276:
            if (binIdx == 0)
                ctxIdxInc = 0;
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=276 and binIdx!=0 should not happend"); }
        break;

        case 399:
            if (binIdx == 0)
                ctxIdxInc = deriv_ctxIdxInc_transformsize8x8flag(dc);
            else
            { TRACE_ERROR(CABAC, "deriv_ctxIdxInc with ctxIdxOffset=399 and binIdx!=0 should not happend"); }
        break;

        default:
            TRACE_ERROR(CABAC, "Unknown ctxIdxOffset = %i", ctxIdxOffset);
        break;
    }

    TRACE_2(CABAC, "  ctxIdxInc() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/* ************************************************************************** */
/*
9.3.3.1.1.1 Derivation process of ctxIdxInc for the syntax element mb_skip_flag.
9.3.3.1.1.2 Derivation process of ctxIdxInc for the syntax element mb_field_decoding_flag.
*/
/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element mb_type.
 * \param *dc The current DecodingContext.
 * \param ctxIdxOffset docme.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.3 Derivation process of ctxIdxInc for the syntax element mb_type.
 */
static int deriv_ctxIdxInc_mbtype(DecodingContext_t *dc, const int ctxIdxOffset)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_mbtype()" CLR_RESET);

    int mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
    int mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

    int condTermFlagA = 1;
    int condTermFlagB = 1;

    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
        if (ctxIdxOffset == 3 && dc->mb_array[mbAddrA]->mb_type == I_NxN)
            condTermFlagA = 0;
/*
        else if (ctxIdxOffset == 0 && dc->mb_array[mbAddrA]->mb_type == SI)
            condTermFlagA = 0;
        else if (ctxIdxOffset == 27 && (dc->mb_array[mbAddrA]->mb_type == B_Skip || dc->mb_array[mbAddrA]->mb_type == B_Direct_16x16))
            condTermFlagA = 0;
*/
    }
    else
    {
        // mbAddrA is not available
        condTermFlagA = 0;
    }

    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
        if (ctxIdxOffset == 3 && dc->mb_array[mbAddrB]->mb_type == I_NxN)
            condTermFlagB = 0;
/*
        else if (ctxIdxOffset == 0 && dc->mb_array[mbAddrB]->mb_type == SI)
            condTermFlagB = 0;
        else if (ctxIdxOffset == 27 && (dc->mb_array[mbAddrB]->mb_type == B_Skip || dc->mb_array[mbAddrB]->mb_type == B_Direct_16x16))
            condTermFlagB = 0;
*/
    }
    else
    {
        // mbAddrB is not available
        condTermFlagB = 0;
    }

    TRACE_3(CABAC, "  ctxIdxInc_mbtype() = %i", condTermFlagA + condTermFlagB);
    return (condTermFlagA + condTermFlagB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element coded_block_pattern_luma.
 * \param *dc The current DecodingContext.
 * \param decodedSE[] The syntax element.
 * \param binIdx docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.4 Derivation process of ctxIdxInc for the syntax element coded_block_pattern.
 */
static int deriv_ctxIdxInc_cbp_luma(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_cbp()" CLR_RESET);

    int mbAddrA = -1;
    int mbAddrB = -1;

    int condTermFlagA = 1;
    int condTermFlagB = 1;

    int ctxIdxInc = 0;

    int luma8x8BlkIdxA = -1;
    int luma8x8BlkIdxB = -1;

    deriv_8x8lumablocks(dc, binIdx, &mbAddrA, &luma8x8BlkIdxA, &mbAddrB, &luma8x8BlkIdxB);

    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
#if ENABLE_IPCM
        if (dc->mb_array[mbAddrA]->mb_type == I_PCM)
        {
            condTermFlagA = 0;
        }
        else
#endif // ENABLE_IPCM

        if ((mbAddrA != (int)(dc->CurrMbAddr)) &&
            (dc->mb_array[mbAddrA]->mb_type != P_Skip && dc->mb_array[mbAddrA]->mb_type != B_Skip) &&
            (((dc->mb_array[mbAddrA]->CodedBlockPatternLuma >> luma8x8BlkIdxA) & 1) != 0))
        {
            condTermFlagA = 0;
        }
        else if ((mbAddrA == (int)(dc->CurrMbAddr)) &&
                 (decodedSE[luma8x8BlkIdxA] != 0))
        {
            condTermFlagA = 0;
        }
    }
    else
    {
        // mbAddrA is not available
        condTermFlagA = 0;
    }

    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
#if ENABLE_IPCM
        if (dc->mb_array[mbAddrB]->mb_type == I_PCM)
        {
            condTermFlagB = 0;
        }
        else
#endif // ENABLE_IPCM
        if ((mbAddrB != (int)(dc->CurrMbAddr)) &&
            (dc->mb_array[mbAddrB]->mb_type != P_Skip && dc->mb_array[mbAddrB]->mb_type != B_Skip) &&
            (((dc->mb_array[mbAddrB]->CodedBlockPatternLuma >> luma8x8BlkIdxB) & 1) != 0))
        {
            condTermFlagB = 0;
        }
        else if ((mbAddrB == (int)(dc->CurrMbAddr)) &&
                 (decodedSE[luma8x8BlkIdxB] != 0))
        {
            condTermFlagB = 0;
        }
    }
    else
    {
        // mbAddrB is not available
        condTermFlagB = 0;
    }

    ctxIdxInc = (condTermFlagA + condTermFlagB*2);

    TRACE_3(CABAC, "  ctxIdxInc_cbp() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element coded_block_pattern_chroma.
 * \param *dc The current DecodingContext.
 * \param decodedSE[] The syntax element.
 * \param binIdx docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.4 Derivation process of ctxIdxInc for the syntax element coded_block_pattern.
 */
static int deriv_ctxIdxInc_cbp_chroma(DecodingContext_t *dc, const uint8_t decodedSE[32], const int binIdx)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_cbp()" CLR_RESET);

    int condTermFlagA = 1;
    int condTermFlagB = 1;

    int ctxIdxInc = 0;

    int mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
    int mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
        if (dc->mb_array[mbAddrA]->mb_type == P_Skip ||
            dc->mb_array[mbAddrA]->mb_type == B_Skip)
            condTermFlagA = 0;
        else if (binIdx == 0 && dc->mb_array[mbAddrA]->CodedBlockPatternChroma == 0)
            condTermFlagA = 0;
        else if (binIdx == 1 && dc->mb_array[mbAddrA]->CodedBlockPatternChroma != 2)
            condTermFlagA = 0;
    }
    else
    {
        // mbAddrA is not available
        condTermFlagA = 0;
    }

    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
        if (dc->mb_array[mbAddrB]->mb_type == P_Skip ||
            dc->mb_array[mbAddrB]->mb_type == B_Skip)
            condTermFlagB = 0;
        else if (binIdx == 0 && dc->mb_array[mbAddrB]->CodedBlockPatternChroma == 0)
            condTermFlagB = 0;
        else if (binIdx == 1 && dc->mb_array[mbAddrB]->CodedBlockPatternChroma != 2)
            condTermFlagB = 0;
    }
    else
    {
        // mbAddrB is not available
        condTermFlagB = 0;
    }

    ctxIdxInc = condTermFlagA + condTermFlagB*2 + ((binIdx == 1) ? 4 : 0);

    TRACE_3(CABAC, "  ctxIdxInc_cbp() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element mb_qp_delta.
 * \param *dc The current DecodingContext.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.5 Derivation process of ctxIdxInc for the syntax element mb_qp_delta.
 */
static int deriv_ctxIdxInc_mbQPd(DecodingContext_t *dc)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_mbQPd()" CLR_RESET);
    int ctxIdxInc = 1;
    int prevMbAddr = dc->CurrMbAddr - 1;

    if (prevMbAddr > -1 &&
        dc->mb_array[prevMbAddr] != NULL)
    {
        if (dc->mb_array[prevMbAddr]->mb_type == I_PCM ||
            dc->mb_array[prevMbAddr]->mb_type == P_Skip ||
            dc->mb_array[prevMbAddr]->mb_type == B_Skip)
            ctxIdxInc = 0;
        else if ((dc->mb_array[prevMbAddr]->MbPartPredMode[0] != Intra_16x16) &&
                 (dc->mb_array[prevMbAddr]->CodedBlockPatternLuma == 0 && dc->mb_array[prevMbAddr]->CodedBlockPatternChroma == 0))
            ctxIdxInc = 0;
        else if (dc->mb_array[prevMbAddr]->mb_qp_delta == 0)
            ctxIdxInc = 0;
    }
    else
    {
        // prevMbAddr is not available
        ctxIdxInc = 0;
    }

    TRACE_3(CABAC, "  ctxIdxInc_mbQPd() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/* ************************************************************************** */
/*
9.3.3.1.1.6 Derivation process of ctxIdxInc for the syntax elements ref_idx_l0 and ref_idx_l1.
9.3.3.1.1.7 Derivation process of ctxIdxInc for the syntax elements mvd_l0 and mvd_l1.
*/
/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element mb_type.
 * \param *dc The current DecodingContext.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.8 Derivation process of ctxIdxInc for the syntax element intra_chroma_pred_mode.
 */
static int deriv_ctxIdxInc_intrachromapredmode(DecodingContext_t *dc)
{
    TRACE_3(CABAC, BLD_GREEN "  assign_ctxIdxInc_priorvalues()" CLR_RESET);

    int mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
    int mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

    int condTermFlagA = 1;
    int condTermFlagB = 1;

    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
        if (dc->mb_array[mbAddrA]->MbPartPredMode[0] > 3)
            condTermFlagA = 0;
        else if (dc->mb_array[mbAddrA]->mb_type == I_PCM)
            condTermFlagA = 0;
        else if (dc->mb_array[mbAddrA]->IntraChromaPredMode == 0)
            condTermFlagA = 0;
    }
    else
    {
        // mbAddrA is not available
        condTermFlagA = 0;
    }

    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
        if (dc->mb_array[mbAddrB]->MbPartPredMode[0] > 3)
            condTermFlagB = 0;
        else if (dc->mb_array[mbAddrB]->mb_type == I_PCM)
            condTermFlagB = 0;
        else if (dc->mb_array[mbAddrB]->IntraChromaPredMode == 0)
            condTermFlagB = 0;
    }
    else
    {
        // mbAddrB is not available
        condTermFlagB = 0;
    }

    TRACE_3(CABAC, "  ctxIdxInc_intrachromapredmode() = %i", condTermFlagA + condTermFlagB);
    return (condTermFlagA + condTermFlagB);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element coded_block_flag.
 * \param *dc The current DecodingContext.
 * \param blkType The type of coefficient block we are decoding.
 * \param blkIdx The id of the coefficient block inside its macroblock.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.9 Derivation process of ctxIdxInc for the syntax element coded_block_flag.
 */
static int deriv_ctxIdxInc_codedblockflag(DecodingContext_t *dc, const int blkType, const int blkIdx)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_codedblockflag()" CLR_RESET);

    int iYCbCr = 0;
    int mbAddrA = -1;
    int mbAddrB = -1;

    // Compute transBlockN
    ////////////////////////////////////////////////////////////////////////////
    int transBlockA = -1;
    int transBlockB = -1;

    if (blkType == blk_LUMA_16x16_DC)
    {
        mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
        mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

        // A
        if ((mbAddrA > -1 && dc->mb_array[mbAddrA] != NULL) &&
            dc->mb_array[mbAddrA]->MbPartPredMode[0] == Intra_16x16)
        {
            transBlockA = 16;
            //FIXME the luma DC block of macroblock mbAddrN is assigned to transBlockN
            //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[0][16];
        }

        // B
        if ((mbAddrB > -1 && dc->mb_array[mbAddrB] != NULL) &&
            dc->mb_array[mbAddrB]->MbPartPredMode[0] == Intra_16x16)
        {
            transBlockB = 16;
            //FIXME the luma DC block of macroblock mbAddrN is assigned to transBlockN
            //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[0][16];
        }
    }
    else if (blkType == blk_LUMA_4x4 || blkType == blk_LUMA_16x16_AC)
    {
        int luma4x4BlkIdxA = -1;
        int luma4x4BlkIdxB = -1;
        deriv_4x4lumablocks(dc, blkIdx, &mbAddrA, &luma4x4BlkIdxA, &mbAddrB, &luma4x4BlkIdxB);

        // A
        if (mbAddrA > -1 &&
            dc->mb_array[mbAddrA] != NULL)
        {
            if ((dc->mb_array[mbAddrA]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != B_Skip) &&
                (((dc->mb_array[mbAddrA]->CodedBlockPatternLuma >> (luma4x4BlkIdxA >> 2)) & 1) != 0))
            {
                if (dc->mb_array[mbAddrA]->mb_type != I_PCM &&
                    dc->mb_array[mbAddrA]->transform_size_8x8_flag == false)
                {
                    transBlockA = luma4x4BlkIdxA;
                    //FIXME the 4x4 luma block with index luma4x4BlkIdxN of macroblock mbAddrN is assigned to transBlockN
                    //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[0][luma4x4BlkIdxA];
                }
                else if (dc->mb_array[mbAddrA]->transform_size_8x8_flag == true)
                {
                    transBlockA = luma4x4BlkIdxA >> 2;
                    //FIXME the 8x8 luma block with index (luma4x4BlkIdxN >> 2) of macroblock mbAddrN is assigned to transBlockN
                    //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[0][luma4x4BlkIdxA >> 2];
                }
            }
        }

        // B
        if (mbAddrB > -1 &&
            dc->mb_array[mbAddrB] != NULL)
        {
            if ((dc->mb_array[mbAddrB]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != B_Skip) &&
                (((dc->mb_array[mbAddrB]->CodedBlockPatternLuma >> (luma4x4BlkIdxB >> 2)) & 1) != 0))
            {
                if (dc->mb_array[mbAddrB]->mb_type != I_PCM &&
                    dc->mb_array[mbAddrB]->transform_size_8x8_flag == false)
                {
                    transBlockB = luma4x4BlkIdxB;
                    //FIXME the 4x4 luma block with index luma4x4BlkIdxN of macroblock mbAddrN is assigned to transBlockN
                    //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[0][luma4x4BlkIdxB];
                }
                else if (dc->mb_array[mbAddrB]->transform_size_8x8_flag == true)
                {
                    transBlockB = luma4x4BlkIdxB >> 2;
                    //FIXME the 8x8 luma block with index (luma4x4BlkIdxN >> 2) of macroblock mbAddrN is assigned to transBlockN
                    //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[0][luma4x4BlkIdxB >> 2];
                }
            }
        }
    }
    else if (blkType == blk_CHROMA_DC_Cb || blkType == blk_CHROMA_DC_Cr)
    {
        if (blkType == blk_CHROMA_DC_Cb)
            iYCbCr = 1;
        else if (blkType == blk_CHROMA_DC_Cr)
            iYCbCr = 2;

        mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
        mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

        // A
        if (mbAddrA > -1 &&
            dc->mb_array[mbAddrA] != NULL)
        {
            if ((dc->mb_array[mbAddrA]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != I_PCM) &&
                dc->mb_array[mbAddrA]->CodedBlockPatternChroma != 0)
            {
                transBlockA = 4;
                //FIXME the chroma DC block of chroma component iCbCr of macroblock mbAddrN is assigned to transBlockN
                //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[iYCbCr][4];
            }
        }

        // B
        if (mbAddrB > -1 &&
            dc->mb_array[mbAddrB] != NULL)
        {
            if ((dc->mb_array[mbAddrB]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != I_PCM) &&
                dc->mb_array[mbAddrB]->CodedBlockPatternChroma != 0)
            {
                transBlockB = 4;
                //FIXME the chroma DC block of chroma component iCbCr of macroblock mbAddrN is assigned to transBlockN
                //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[iYCbCr][4];
            }
        }
    }
    else if (blkType == blk_CHROMA_AC_Cb || blkType == blk_CHROMA_AC_Cr)
    {
        if (blkType == blk_CHROMA_AC_Cb)
            iYCbCr = 1;
        else if (blkType == blk_CHROMA_AC_Cr)
            iYCbCr = 2;

        int chroma4x4BlkIdxA = -1;
        int chroma4x4BlkIdxB = -1;
        deriv_4x4chromablocks(dc, blkIdx, &mbAddrA, &chroma4x4BlkIdxA, &mbAddrB, &chroma4x4BlkIdxB);

        // A
        if (mbAddrA > -1 &&
            dc->mb_array[mbAddrA] != NULL)
        {
            if ((dc->mb_array[mbAddrA]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != I_PCM) &&
                dc->mb_array[mbAddrA]->CodedBlockPatternChroma == 2)
            {
                transBlockA = chroma4x4BlkIdxA;
                //FIXME the 4x4 chroma block with chroma4x4BlkIdxN of the chroma component iCbCr of macroblock mbAddrN is assigned to transBlockN
                //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[iYCbCr][chroma4x4BlkIdxA];
            }
        }

        // B
        if (mbAddrB > -1 &&
            dc->mb_array[mbAddrB] != NULL)
        {
            if ((dc->mb_array[mbAddrB]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != I_PCM) &&
                dc->mb_array[mbAddrB]->CodedBlockPatternChroma == 2)
            {
                transBlockB = chroma4x4BlkIdxB;
                //FIXME the 4x4 chroma block with chroma4x4BlkIdxN of the chroma component iCbCr of macroblock mbAddrN is assigned to transBlockN
                //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[iYCbCr][chroma4x4BlkIdxB];
            }
        }
    }
    else if (blkType == blk_LUMA_8x8)
    {
        int luma8x8BlkIdxA = -1;
        int luma8x8BlkIdxB = -1;
        deriv_8x8lumablocks(dc, blkIdx, &mbAddrA, &luma8x8BlkIdxA, &mbAddrB, &luma8x8BlkIdxB);

        // A
        if (mbAddrA > -1 &&
            dc->mb_array[mbAddrA] != NULL)
        {
            if ((dc->mb_array[mbAddrA]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrA]->mb_type != I_PCM) &&
                (((dc->mb_array[mbAddrA]->CodedBlockPatternLuma >> luma8x8BlkIdxA) & 1) != 0) &&
                (dc->mb_array[mbAddrA]->transform_size_8x8_flag == true))
            {
                transBlockA = luma8x8BlkIdxA;
                //FIXME the 8x8 luma block with index luma8x8BlkIdxN of macroblock mbAddrN is assigned to transBlockN
                //transBlockA = dc->mb_array[mbAddrA]->coded_block_flag[0][luma8x8BlkIdxA];
            }
        }

        // B
        if (mbAddrB > -1 &&
            dc->mb_array[mbAddrB] != NULL)
        {
            if ((dc->mb_array[mbAddrB]->mb_type != P_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != B_Skip &&
                 dc->mb_array[mbAddrB]->mb_type != I_PCM) &&
                (((dc->mb_array[mbAddrB]->CodedBlockPatternLuma >> luma8x8BlkIdxB) & 1) != 0) &&
                (dc->mb_array[mbAddrB]->transform_size_8x8_flag == true))
            {
                transBlockB = luma8x8BlkIdxB;
                //FIXME the 8x8 luma block with index luma8x8BlkIdxN of macroblock mbAddrN is assigned to transBlockN
                //transBlockB = dc->mb_array[mbAddrB]->coded_block_flag[0][luma8x8BlkIdxB];
            }
        }
    }
    else //if (blkType > 7)
    {
        TRACE_ERROR(CABAC, ">>> UNSUPPORTED (blkType > 7)");
        TRACE_ERROR(CABAC, "blkType == %i is out of range", blkType);
        return -1;
    }


    // Compute condTermFlagN
    ////////////////////////////////////////////////////////////////////////////
    bool condTermFlagA = false;
    bool condTermFlagB = false;

    // A
    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
        if (transBlockA == -1 &&
            dc->mb_array[mbAddrA]->mb_type != I_PCM)
        {
            condTermFlagA = false;
        }/*
        else if (dc->mb_array[dc->CurrMbAddr]->mb_type <= I_PCM &&
                 dc->pps_array[dc->active_slice->pic_parameter_set_id]->constrained_intra_pred_flag == true &&
                 dc->mb_array[mbAddrA]->mb_type > I_PCM &&
                 dc->active_nalu->nal_unit_type > 1 && dc->nalu->nal_unit_type < 5)
        {
            condTermFlagA = false;
        }*/
        else if (dc->mb_array[mbAddrA]->mb_type == I_PCM)
            condTermFlagA = true;
        else
            condTermFlagA = dc->mb_array[mbAddrA]->coded_block_flag[iYCbCr][transBlockA];
    }
    else
    {
        // mbAddrA is not available
        if (dc->mb_array[dc->CurrMbAddr]->mb_type > I_PCM)
            condTermFlagA = false;
        else
            condTermFlagA = true;
    }

    // B
    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
        if (transBlockB == -1 &&
            dc->mb_array[mbAddrB]->mb_type != I_PCM)
        {
            condTermFlagB = false;
        }/*
        else if (dc->mb_array[dc->CurrMbAddr]->mb_type <= I_PCM &&
                 dc->pps_array[dc->active_slice->pic_parameter_set_id]->constrained_intra_pred_flag == true &&
                 dc->mb_array[mbAddrB]->mb_type > I_PCM &&
                 dc->active_nalu->nal_unit_type > 1 && dc->nalu->nal_unit_type < 5)
        {
            condTermFlagB = false;
        }*/
        else if (dc->mb_array[mbAddrB]->mb_type == I_PCM)
            condTermFlagB = true;
        else
            condTermFlagB = dc->mb_array[mbAddrB]->coded_block_flag[iYCbCr][transBlockB];
    }
    else
    {
        // mbAddrB is not available
        if (dc->mb_array[dc->CurrMbAddr]->mb_type > I_PCM)
            condTermFlagB = false;
        else
            condTermFlagB = true;
    }

    TRACE_3(CABAC, "(blkIdx %i) ctxIdxInc_codedblockflag() = %i   (A:%i + 2*B:%i)", blkIdx, condTermFlagA + condTermFlagB*2, condTermFlagA, condTermFlagB);
    return (condTermFlagA + condTermFlagB*2);
}

/* ************************************************************************** */

/*!
 * \brief Derivation process of ctxIdxInc for the syntax element transform_size_8x8_flag.
 * \param *dc The current DecodingContext.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.1.10 Derivation process of ctxIdxInc for the syntax element transform_size_8x8_flag.
 */
static int deriv_ctxIdxInc_transformsize8x8flag(DecodingContext_t *dc)
{
    TRACE_3(CABAC, BLD_GREEN "  deriv_ctxIdxInc_transformsize8x8flag()" CLR_RESET);

    int mbAddrA = dc->mb_array[dc->CurrMbAddr]->mbAddrA;
    int mbAddrB = dc->mb_array[dc->CurrMbAddr]->mbAddrB;

    int condTermFlagA = 1;
    int condTermFlagB = 1;

    if (mbAddrA > -1 &&
        dc->mb_array[mbAddrA] != NULL)
    {
        if (dc->mb_array[mbAddrA]->transform_size_8x8_flag == false)
            condTermFlagA = 0;
    }
    else
    {
        // mbAddrA is not available
        condTermFlagA = 0;
    }

    if (mbAddrB > -1 &&
        dc->mb_array[mbAddrB] != NULL)
    {
        if (dc->mb_array[mbAddrB]->transform_size_8x8_flag == false)
            condTermFlagB = 0;
    }
    else
    {
        // mbAddrB is not available
        condTermFlagB = 0;
    }

    TRACE_3(CABAC, "  ctxIdxInc_transformsize8x8flag() = %i", condTermFlagA + condTermFlagB);
    return (condTermFlagA + condTermFlagB);
}

/* ************************************************************************** */

/*!
 * \brief Assignment process of ctxIdxInc using prior decoded bin values.
 * \param decodedSE[] The bin string we are trying to decode.
 * \param binIdx docme.
 * \param ctxIdxOffset docme.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.2 Assignment process of ctxIdxInc using prior decoded bin values.
 */
static int assign_ctxIdxInc_priorvalues(const uint8_t decodedSE[32], const int binIdx, const int ctxIdxOffset)
{
    TRACE_3(CABAC, BLD_GREEN "  assign_ctxIdxInc_priorvalues()" CLR_RESET);
    int ctxIdxInc = -1;

    if (ctxIdxOffset == 3)
    {
        if (binIdx == 4)
        {
            ctxIdxInc = (decodedSE[3] != 0) ? 5: 6;
        }
        else if (binIdx == 5)
        {
            ctxIdxInc = (decodedSE[3] != 0) ? 6: 7;
        }
    }
    else if (ctxIdxOffset == 14 && binIdx ==2)
    {
        ctxIdxInc = (decodedSE[1] != 1) ? 2: 3;
    }
    else if (ctxIdxOffset == 17 && binIdx == 4)
    {
        ctxIdxInc = (decodedSE[3] != 0) ? 2: 3;
    }
    else if (ctxIdxOffset == 27 && binIdx == 2)
    {
        ctxIdxInc = (decodedSE[1] != 0) ? 4: 5;
    }
    else if (ctxIdxOffset == 32 && binIdx == 4)
    {
        ctxIdxInc = (decodedSE[3] != 0) ? 2: 3;
    }
    else if (ctxIdxOffset == 36 && binIdx == 2)
    {
        ctxIdxInc = (decodedSE[1] != 0) ? 2: 3;
    }
    else
    {
        TRACE_WARNING(CABAC, "  ctxIdxInc_priorvalues() = %i WTF ARE WE DOING HERE", ctxIdxInc);
    }

    TRACE_3(CABAC, "  ctxIdxInc_priorvalues() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/* ************************************************************************** */

/*!
 * \brief Assignment process of ctxIdxInc for some particular syntax elements.
 * \param *dc The current DecodingContext.
 * \param seType The type of syntax element we want to decode.
 * \param blkType The type of coefficient block we are decoding.
 * \param binIdx docme.
 * \return ctxIdxInc docme.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.1.3 Assignment process of ctxIdxInc for syntax elements significant_coeff_flag, last_significant_coeff_flag, and coeff_abs_level_minus1.
 */
static int assign_ctxIdxInc_se(DecodingContext_t *dc, const SyntaxElementType_e seType, const BlockType_e blkType, const int binIdx)
{
    TRACE_3(CABAC, BLD_GREEN "  assign_ctxIdxInc_se()" CLR_RESET);
    int ctxIdxInc = -1;

    if (seType == SE_significant_coeff_flag || seType == SE_last_significant_coeff_flag)
    {
        int levelListIdx = dc->mb_array[dc->CurrMbAddr]->levelListIdx;

        if (blkType == blk_CHROMA_DC_Cb || blkType == blk_CHROMA_DC_Cr)
        {
            sps_t *sps = dc->sps_array[dc->pps_array[dc->active_slice->pic_parameter_set_id]->seq_parameter_set_id];
            int NumC8x8 = 4 / (sps->SubWidthC * sps->SubHeightC);

            ctxIdxInc = std::min(levelListIdx / NumC8x8, 2);
        }
        else if (blkType == blk_LUMA_8x8)
        {
            if (dc->active_slice->mb_field_decoding_flag == false)
                ctxIdxInc = ctxIdxInc_8x8blk[levelListIdx][(seType == SE_significant_coeff_flag) ? 0 : 2];
            else
                ctxIdxInc = ctxIdxInc_8x8blk[levelListIdx][(seType == SE_significant_coeff_flag) ? 1 : 2];
        }
        else
        {
            ctxIdxInc = levelListIdx;
        }
    }
    else //if (seType == SE_coeff_abs_level_minus1)
    {
        TRACE_3(CABAC, "  Eq1 : %i", dc->mb_array[dc->CurrMbAddr]->numDecodAbsLevelEq1);
        TRACE_3(CABAC, "  Gt1 : %i", dc->mb_array[dc->CurrMbAddr]->numDecodAbsLevelGt1);

        int Eq1 = dc->mb_array[dc->CurrMbAddr]->numDecodAbsLevelEq1;
        int Gt1 = dc->mb_array[dc->CurrMbAddr]->numDecodAbsLevelGt1;

        if (binIdx == 0)
        {
            ctxIdxInc = ((Gt1 != 0) ? 0 : std::min(4, 1 + Eq1));
        }
        else
        {
            if (blkType == blk_CHROMA_DC_Cb || blkType == blk_CHROMA_DC_Cr)
            {
                ctxIdxInc = 5 + std::min(3, Gt1);
            }
            else
            {
                ctxIdxInc = 5 + std::min(4, Gt1);
            }
        }
    }

    TRACE_3(CABAC, "  ctxIdxInc_se() = %i", ctxIdxInc);
    return ctxIdxInc;
}

/* ************************************************************************** */

/*!
 * \brief Arithmetic decoding process.
 * \param *dc The current DecodingContext.
 * \param ctxIdx docme.
 * \param bypassFlag docme.
 * \return binVal The value of one bin decoded from the stream.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.2 Arithmetic decoding process.
 *
 * Inputs to this process are the bypassFlag, ctxIdx as derived in subclause 9.3.3.1,
 * and the state variables codIRange and codIOffset of the arithmetic decoding engine.
 * Output of this process is the value of the bin.
 *
 * Note: This function has been integrated into decodingProcessFlow() and should
 * not be used directly!
 */
static int decodeBin(DecodingContext_t *dc, const int ctxIdx, const bool bypassFlag)
{
    int binVal = -1;

    if (bypassFlag)
    {
        binVal = DecodeBypass(dc->active_slice->cc, dc->bitstr);
    }
    else
    {
        if (ctxIdx == 276)
        {
            binVal = DecodeTerminate(dc->active_slice->cc, dc->bitstr);
        }
        else
        {
            binVal = DecodeDecision(dc, ctxIdx);
        }
    }

    return binVal;
}

/* ************************************************************************** */

/*!
 * \brief Arithmetic decoding process for a binary decision.
 * \param *dc The current DecodingContext.
 * \param ctxIdx The context index.
 * \return binVal The value of one bin decoded from the stream.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.2.1 Arithmetic decoding process for a binary decision.
 *
 * Inputs to this process are ctxIdx, codIRange, and codIOffset.
 * Outputs of this process are the decoded value binVal, and the updated
 * variables codIRange and codIOffset.
 */
static uint8_t DecodeDecision(DecodingContext_t *dc, const int ctxIdx)
{
    uint8_t binVal = 0;

    // Shortcut
    CabacContext_t *cc = dc->active_slice->cc;

    // 1
    uint16_t qCodIRangeIdx = (cc->codIRange >> 6) & 3;
    uint16_t codIRangeLPS = rangeTabLPS[cc->pStateIdx[ctxIdx]][qCodIRangeIdx];

    // 2
    cc->codIRange -= codIRangeLPS;

    if (cc->codIOffset < cc->codIRange)
    {
        binVal = cc->valMPS[ctxIdx];

        // 9.3.3.2.1.1 State transition process
        cc->pStateIdx[ctxIdx] = transIdxMPS[cc->pStateIdx[ctxIdx]];
    }
    else
    {
        binVal = 1 - cc->valMPS[ctxIdx];

        cc->codIOffset -= cc->codIRange;
        cc->codIRange = codIRangeLPS;

        // 9.3.3.2.1.1 State transition process
        if (cc->pStateIdx[ctxIdx] == 0)
        {
            cc->valMPS[ctxIdx] = 1 - cc->valMPS[ctxIdx];
        }

        cc->pStateIdx[ctxIdx] = transIdxLPS[cc->pStateIdx[ctxIdx]];
    }

#if ENABLE_DEBUG
    int frame_debug_range[2] = {-1, -1}; // Range of (idr) frame(s) to debug/analyse
    int mb_debug_range[2] = {-1, -1}; // Range of macroblock(s) to debug/analyse

    if ((int)(dc->idrCounter) >= frame_debug_range[0] &&  (int)(dc->idrCounter) <= frame_debug_range[1])
    {
        if ((int)(dc->CurrMbAddr) >= mb_debug_range[0] && (int)(dc->CurrMbAddr) <= mb_debug_range[1])
        {
            // Print decoder status
            printf("[CABAC] DecodeDecision()\n");
            printf("[CABAC] ctxIdx     : %i\n", ctxIdx);
            printf("[CABAC] codIRange  : %i\n", dc->active_slice->cc->codIRange);
            printf("[CABAC] codIOffset : %i\n", dc->active_slice->cc->codIOffset);
        }
    }
#endif // ENABLE_DEBUG

    RenormD(cc, dc->bitstr);

#if ENABLE_DEBUG
    if ((int)(dc->idrCounter) >= frame_debug_range[0] && (int)(dc->idrCounter) <= frame_debug_range[1])
    {
        if ((int)(dc->CurrMbAddr) >= mb_debug_range[0] && (int)(dc->CurrMbAddr) <= mb_debug_range[1])
        {
            // Print decoder status
            printf("[CABAC] RenormD()\n");
            printf("[CABAC] ctxIdx     : %i\n", ctxIdx);
            printf("[CABAC] codIRange  : %i\n", dc->active_slice->cc->codIRange);
            printf("[CABAC] codIOffset : %i\n\n", dc->active_slice->cc->codIOffset);
        }
    }
#endif // ENABLE_DEBUG

    return binVal;
}

/* ************************************************************************** */

/*!
 * \brief Renormalization process in the arithmetic decoding engine.
 * \param *cc The CabacContext for the current slice.
 * \param *bitstr The current bitstream.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.2.2 Renormalization process in the arithmetic decoding engine.
 *
 * A process called renormalization keeps the finite precision from becoming a
 * limit on the total number of symbols that can be encoded.
 *
 * Inputs to this process are bits from slice data and the variables codIRange and codIOffset.
 * Outputs of this process are the updated variables codIRange and codIOffset.
 */
static void RenormD(CabacContext_t *cc, Bitstream_t *bitstr)
{
    while (cc->codIRange < 256)
    {
        cc->codIRange <<= 1;
        cc->codIOffset <<= 1;
        cc->codIOffset |= read_bit(bitstr);
    }

#if ENABLE_DEBUG
/*
    {
        // Print decoder status
        printf("[CABAC] RenormD");
        printf("[CABAC] codIRange : %i\n", cc->codIRange);
        printf("[CABAC] codIOffset : %i\n\n", cc->codIOffset);
    }
*/
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */

/*!
 * \brief Bypass decoding process for binary decisions.
 * \param *cc The CabacContext for the current slice.
 * \param *bitstr The current bitstream.
 * \return binVal The value of one bin decoded from the stream.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.2.3 Bypass decoding process for binary decisions.
 *
 * Inputs to this process are bits from slice data and the variables codIRange and codIOffset.
 * Outputs of this process are the updated variable codIOffset and the decoded value binVal.
 */
static uint8_t DecodeBypass(CabacContext_t *cc, Bitstream_t *bitstr)
{
    TRACE_2(CABAC, BLD_GREEN "  DecodeBypass()" CLR_RESET);

    uint8_t binVal = 0;

    cc->codIOffset <<= 1;
    cc->codIOffset |= read_bit(bitstr);

    if (cc->codIOffset >= cc->codIRange)
    {
        binVal = 1;
        cc->codIOffset -= cc->codIRange;
    }

    return binVal;
}

/* ************************************************************************** */

/*!
 * \brief Decoding process for binary decisions before termination.
 * \param *cc The CabacContext for the current slice.
 * \param *bitstr The current bitstream.
 * \return binVal The value of one bin decoded from the stream.
 *
 * From 'ITU-T H.264' recommendation:
 * 9.3.3.2.4 Decoding process for binary decisions before termination.
 *
 * This special decoding routine applies to decoding of end_of_slice_flag and of
 * the bin indicating the I_PCM mode corresponding to ctxIdx equal to 276.
 *
 * Inputs to this process are bits from slice data and the variables codIRange and codIOffset.
 * Outputs of this process are the updated variables codIRange and codIOffset,
 * and the decoded value binVal.
 */
static uint8_t DecodeTerminate(CabacContext_t *cc, Bitstream_t *bitstr)
{
    TRACE_2(CABAC, BLD_GREEN "  DecodeTerminate()" CLR_RESET);

    uint8_t binVal = 1;

    // codIRange never goes under 25x, no need to check for integer underflow
    cc->codIRange -= 2;

    if (cc->codIOffset < cc->codIRange)
    {
        binVal = 0;

        // Renormalization
        RenormD(cc, bitstr);
    }

    return binVal;
}

/* ************************************************************************** */
/* ************************************************************************** */
