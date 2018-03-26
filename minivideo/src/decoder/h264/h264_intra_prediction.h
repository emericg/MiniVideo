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
 * \file      h264_intra_prediction.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef INTRA_PRED_H
#define INTRA_PRED_H

// minivideo headers
#include "h264_decodingcontext.h"

/* ************************************************************************** */

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..L, and from above left X, as follows:
//
//  X A B C D E F G H
//  I a b c d
//  J e f g h
//  K i j k l
//  L m n o p

/* ************************************************************************** */

//! Structure for 4x4 block intra prediction
typedef struct intrapred4x4_t
{
    int blkIdx;
    int BitDepthY;

    uint8_t ph[8 +1];
    uint8_t pv[4 +1];

    bool sample_left;
    bool sample_up_left;
    bool sample_up;
    bool sample_up_right;
} intrapred4x4_t;

//! Structure for 8x8 block intra prediction
typedef struct intrapred8x8_t
{
    int blkIdx;
    int BitDepthY;

    uint8_t ph[16 +1];
    uint8_t pv[8 +1];

    bool sample_left;
    bool sample_up_left;
    bool sample_up;
    bool sample_up_right;
} intrapred8x8_t;

//! Structure for 16x16 block intra prediction
typedef struct intrapred16x16_t
{
    int blkIdx;
    int BitDepthY;

    uint8_t phv;
    uint8_t ph[16];
    uint8_t pv[16];

    bool sample_left;
    bool sample_up;
} intrapred16x16_t;

//! Structure for chroma block intra prediction
typedef struct intrapredChroma_t
{
    int blkIdx;

    int BitDepthC;
    int MbWidthC;
    int MbHeightC;
    int ChromaArrayType;

    uint8_t ph[12 +1];
    uint8_t pv[12 +1];

    bool sample_left;
    bool sample_up;
} intrapredChroma_t;

/* ************************************************************************** */

int intra_prediction_process(DecodingContext_t *dc, Macroblock_t *mb);

/* ************************************************************************** */
#endif // H264_INTRA_PREDICTION_H
