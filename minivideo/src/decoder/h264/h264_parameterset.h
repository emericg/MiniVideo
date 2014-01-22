/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      h264_parameterset.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_PARAMETER_SET_H
#define H264_PARAMETER_SET_H

// minivideo headers
#include "h264_decodingcontext.h"
#include "h264_parameterset_struct.h"

/* ************************************************************************** */

int decodeSPS(DecodingContext_t *dc);
void freeSPS(sps_t **sps_ptr);

int decodePPS(DecodingContext_t *dc);
void freePPS(pps_t **pps_ptr);

int decodeSEI(DecodingContext_t *dc);
void freeSEI(sei_t **sei_ptr);

void printSPS(DecodingContext_t *dc);
void printPPS(DecodingContext_t *dc);
void printSEI(DecodingContext_t *dc);
void printVUI(vui_t *vui);
void printHRD(hrd_t *hrd);

/* ************************************************************************** */
#endif /* H264_PARAMETER_SET_H */
