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
 * \file      h265_parameterset.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H265_PARAMETER_SET_H
#define H265_PARAMETER_SET_H

// minivideo headers
#include "h265_parameterset_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int h265_decodeVPS(Bitstream_t *bitstr, h265_vps_t *vps);
void h265_printVPS(h265_vps_t *vps);
void h265_mapVPS(h265_vps_t *vps, int64_t offset, int64_t size, FILE *xml);

int h265_decodeSPS(Bitstream_t *bitstr, h265_sps_t *sps);
void h265_printSPS(h265_sps_t *sps);
void h265_mapSPS(h265_sps_t *sps, int64_t offset, int64_t size, FILE *xml);

int h265_decodePPS(Bitstream_t *bitstr, h265_pps_t *pps, h265_sps_t **sps_array);
void h265_printPPS(h265_pps_t *pps);
void h265_mapPPS(h265_pps_t *pps, int64_t offset, int64_t size, FILE *xml);

/* ************************************************************************** */
#endif // H265_PARAMETER_SET_H
