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
 * \file      h264_spatial.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_SPATIAL_H
#define H264_SPATIAL_H

// minivideo headers
#include "h264_decodingcontext.h"

/* ************************************************************************** */
// 6.4.[1-6] Inverse scanning processes

int InverseRasterScan(const int a,
                      const int b,
                      const int c,
                      const int d,
                      const int e);

int InverseRasterScan_x(const int a,
                        const int b,
                        const int c,
                        const int d);

int InverseRasterScan_y(const int a,
                        const int b,
                        const int c,
                        const int d);

void InverseMacroblockScan(const int mbAddr,
                           const bool MbaffFrameFlag,
                           const int PicWidthInSamplesL,
                           int *x,
                           int *y);

void InverseMacroblockPartitionScan(const int mbAddr,
                                    const int mbPartIdx,
                                    int *x,
                                    int *y);

void InverseSubMacroblockPartitionScan(const int mbAddr,
                                       const int mbPartIdx,
                                       const int subMbPartIdx,
                                       int *x,
                                       int *y);

void InverseLuma4x4BlkScan(const int luma4x4BlkIdx,
                           int *x,
                           int *y);

void InverseLuma8x8BlkScan(const int luma8x8BlkIdx,
                           int *x,
                           int *y);

void InverseChroma4x4BlkScan(const int chroma4x4BlkIdx,
                             int *x,
                             int *y);

/* ************************************************************************** */
// 6.4.[7-11] Derivation processes for neighbours

bool deriv_macroblock_availability(DecodingContext_t *dc, const int mbAddr);
void deriv_macroblockneighbours_availability(DecodingContext_t *dc, const int mbAddr);
void deriv_macroblockneighbours(DecodingContext_t *dc, const int mbAddr);

void deriv_8x8lumablocks(DecodingContext_t *dc,
                         const int luma8x8BlkIdx,
                         int *mbAddrA, int *luma8x8BlkIdxA,
                         int *mbAddrB, int *luma8x8BlkIdxB);

void deriv_8x8chromablocks_cat3(DecodingContext_t *dc,
                                const int chroma8x8BlkIdx,
                                int *mbAddrA, int *chroma8x8BlkIdxA,
                                int *mbAddrB, int *chroma8x8BlkIdxB);

void deriv_4x4lumablocks(DecodingContext_t *dc,
                         const int luma4x4BlkIdx,
                         int *mbAddrA, int *luma4x4BlkIdxA,
                         int *mbAddrB, int *luma4x4BlkIdxB);

void deriv_4x4chromablocks(DecodingContext_t *dc,
                           const int chroma4x4BlkIdx,
                           int *mbAddrA, int *chroma4x4BlkIdxA,
                           int *mbAddrB, int *chroma4x4BlkIdxB);

void deriv_4x4chromablocks_cat3(DecodingContext_t *dc,
                                const int chroma4x4BlkIdx,
                                int *mbAddrA, int *chroma4x4BlkIdxA,
                                int *mbAddrB, int *chroma4x4BlkIdxB);

void deriv_neighbouringlocations(DecodingContext_t *dc,
                                 const bool lumaBlock,
                                 const int xN, const int yN,
                                 int *mbAddrN,
                                 int *xW, int *yW);

int deriv_4x4lumablock_indices(const int xP, const int yP);
int deriv_4x4chromablock_indices(const int xP, const int yP);
int deriv_8x8lumablock_indices(const int xP, const int yP);

/* ************************************************************************** */
#endif // H264_SPATIAL_H
