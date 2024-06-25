/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
#include "h264_parameterset_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int decodeSPS(Bitstream_t *bitstr, h264_sps_t *sps);
void printSPS(h264_sps_t *sps);
void mapSPS(h264_sps_t *sps, int64_t offset, int64_t size, FILE *xml);
void freeSPS(h264_sps_t **sps_ptr);

int decodePPS(Bitstream_t *bitstr, h264_pps_t *pps, h264_sps_t **sps_array);
void printPPS(h264_pps_t *pps, h264_sps_t **sps_array);
void mapPPS(h264_pps_t *pps, h264_sps_t **sps, int64_t offset, int64_t size, FILE *xml);
void freePPS(h264_pps_t **pps_ptr);

int decodeSEI(Bitstream_t *bitstr, h264_sei_t *sei);
void printSEI(h264_sei_t *sei);
void mapSEI(h264_sei_t *sei, int64_t offset, int64_t size, FILE *xml);
void freeSEI(h264_sei_t **sei_ptr);

int decodeAUD(Bitstream_t *bitstr, h264_aud_t *aud);
void mapAUD(h264_aud_t *aud, int64_t offset, int64_t size, FILE *xml);
void freeAUD(h264_aud_t  **aud_ptr);

/* ************************************************************************** */
#endif // H264_PARAMETER_SET_H
