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
 * \file      h264_transform.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_TRANSFORM_H
#define H264_TRANSFORM_H

// minivideo headers
#include "h264_decodingcontext.h"
#include "h264_macroblock.h"

/* ************************************************************************** */

void computeLevelScale4x4(DecodingContext_t *dc, sps_t *sps);
void computeLevelScale8x8(DecodingContext_t *dc, sps_t *sps);

void transform4x4_luma(DecodingContext_t *dc, Macroblock_t *mb, int luma4x4BlkIdx);
void transform8x8_luma(DecodingContext_t *dc, Macroblock_t *mb, int luma8x8BlkIdx);
void transform16x16_luma(DecodingContext_t *dc, Macroblock_t *mb);
void transform2x2_chroma(DecodingContext_t *dc, Macroblock_t *mb);
void transform4x4_chroma(DecodingContext_t *dc, Macroblock_t *mb);
void transform4x4_chroma_cat3(DecodingContext_t *dc, Macroblock_t *mb);

void inverse_scan_4x4(int LumaLevel[16], int c[4][4]);
void inverse_scan_8x8(int LumaLevel[64], int c[8][8]);

/* ************************************************************************** */
#endif // H264_TRANSFORM_H
