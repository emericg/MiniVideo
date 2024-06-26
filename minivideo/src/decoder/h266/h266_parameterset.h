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
 * \file      h266_parameterset.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H266_PARAMETER_SET_H
#define H266_PARAMETER_SET_H
/* ************************************************************************** */

// minivideo headers
#include "h266_parameterset_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int h266_decodeVPS(Bitstream_t *bitstr, h266_vps_t *vps);
void h266_mapVPS(h266_vps_t *vps, int64_t offset, int64_t size, FILE *xml);
void h266_freeVPS(h266_vps_t **vps_ptr);

int h266_decodeSPS(Bitstream_t *bitstr, h266_sps_t *sps);
void h266_mapSPS(h266_sps_t *sps, int64_t offset, int64_t size, FILE *xml);
void h266_freeSPS(h266_sps_t **sps_ptr);

int h266_decodePPS(Bitstream_t *bitstr, h266_pps_t *pps, h266_sps_t **sps_array);
void h266_mapPPS(h266_pps_t *pps, int64_t offset, int64_t size, FILE *xml);
void h266_freePPS(h266_pps_t **pps_ptr);

/* ************************************************************************** */
#endif // H266_PARAMETER_SET_H
