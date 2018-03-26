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
 * \file      h264_macroblock_utils.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef H264_MACROBLOCK_UTILS_H
#define H264_MACROBLOCK_UTILS_H

// minivideo headers
#include "h264_decodingcontext.h"

/* ************************************************************************** */

void print_macroblock_layer(DecodingContext_t *dc, Macroblock_t *mb);
void print_macroblock_pixel_residual(Macroblock_t *mb);
void print_macroblock_pixel_predicted(Macroblock_t *mb);
void print_macroblock_pixel_final(Macroblock_t *mb);

/* ************************************************************************** */
#endif // H264_MACROBLOCK_UTILS_H
