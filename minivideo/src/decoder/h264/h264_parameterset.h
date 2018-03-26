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

int decodeSPS_legacy(DecodingContext_t *dc);
int decodeSPS(Bitstream_t *bitstr, sps_t *sps);
void freeSPS(sps_t **sps_ptr);

int decodePPS(Bitstream_t *bitstr, pps_t *pps, sps_t **sps_array);
void freePPS(pps_t **pps_ptr);

int decodeSEI(Bitstream_t *bitstr, sei_t *sei);
void freeSEI(sei_t **sei_ptr);

int decodeAUD(Bitstream_t *bitstr, aud_t *aud);

void mapSPS(sps_t *sps, int64_t offset, int64_t size, FILE *xml);
void mapPPS(pps_t *pps, sps_t **sps, int64_t offset, int64_t size, FILE *xml);
void mapSEI(sei_t *sei, int64_t offset, int64_t size, FILE *xml);

void printSPS(sps_t *sps);
void printPPS(pps_t *pps, sps_t **sps_array);
void printSEI(sei_t *sei);

/* ************************************************************************** */
#endif // H264_PARAMETER_SET_H
