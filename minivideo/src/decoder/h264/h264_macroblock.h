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
 * \file      h264_macroblock.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_MACROBLOCK_H
#define H264_MACROBLOCK_H

// minivideo headers
#include "h264_decodingcontext.h"

/* ************************************************************************** */

int macroblock_layer(DecodingContext_t *dc, const int mbAddr);

void freeMbArray(DecodingContext_t *dc);
void freeMbArrayContent(DecodingContext_t *dc);

unsigned int NextMbAddress(DecodingContext_t *dc, const unsigned int CurrMbAddr);

/* ************************************************************************** */
#endif // H264_MACROBLOCK_H
