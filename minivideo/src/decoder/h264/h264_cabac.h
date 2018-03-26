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
 * \file      h264_cabac.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef H264_CABAC_H
#define H264_CABAC_H

// minivideo headers
#include "../../minivideo_typedef.h"

#include "h264_decodingcontext.h"

/* ************************************************************************** */

//! SyntaxElement type
typedef enum SyntaxElementType_e
{
    SE_mb_type = 0,
    SE_mb_skip_flag,
    SE_sub_mb_type,
    SE_mvd_lx0,
    SE_mvd_lx1, // 4
    SE_ref_idx_lx,
    SE_mb_qp_delta,
    SE_intra_chroma_pred_mode,
    SE_prev_intraxxx_pred_mode_flag, // 8
    SE_rem_intraxxx_pred_mode,
    SE_mb_field_decoding_flag,
    SE_coded_block_pattern,
    SE_coded_block_flag, // 12
    SE_significant_coeff_flag,
    SE_last_significant_coeff_flag,
    SE_coeff_abs_level_minus1,
    SE_coeff_sign_flag, // 16
    SE_transform_size_8x8_flag,
    SE_end_of_slice_flag
} SyntaxElementType_e;

//! Binarization informations needed to decode a syntax element with CABAC
typedef struct binarization_t
{
    int SyntaxElementValue;

    int maxBinIdxCtx;
    int ctxIdxOffset;         //!< The context index offset, specifies the lower value of the range of ctxIdx.
    bool bypassFlag;          //!< Indicate if the DecodeBypass() process must be used.

    uint8_t **bintable;       //!< The binarization table we are gonna use to run decoded bins against.
    int bintable_x;           //!< Width of our binarization table.
    int bintable_y;           //!< Height of our binarization table.
} binarization_t;

/* ************************************************************************** */

void residual_block_cabac(DecodingContext_t *dc,
                          int *coeffLevel,
                          const int startIdx, const int endIdx,
                          const int maxNumCoeff,
                          const int blkType, const int blxIdx);

/* ************************************************************************** */

int read_ae(DecodingContext_t *dc,
            SyntaxElementType_e seType);

int read_ae_blk(DecodingContext_t *dc,
                SyntaxElementType_e seType,
                BlockType_e blkType,
                const int blkIdx);

/* ************************************************************************** */

int initCabacContextVariables(DecodingContext_t *dc);
int initCabacDecodingEngine(DecodingContext_t *dc);

/* ************************************************************************** */
#endif // H264_CABAC_H
