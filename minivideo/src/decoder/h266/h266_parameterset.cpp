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
 * \file      h266_parameterset.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "h266_parameterset.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cinttypes>

/* ************************************************************************** */

int h266_decodeVPS(Bitstream_t *bitstr, h266_vps_t *vps)
{
    TRACE_WARNING(PARAM, "h266_decodeVPS() is not implemented yet");
    return FAILURE;
}

void h266_mapVPS(h266_vps_t *vps, int64_t offset, int64_t size, FILE *xml)
{
    TRACE_WARNING(PARAM, "h266_mapVPS() is not implemented yet");
}

void h266_freeVPS(h266_vps_t **vps_ptr)
{
    if (*vps_ptr != NULL)
    {
        free(*vps_ptr);
        *vps_ptr = NULL;

        TRACE_1(PARAM, ">> VPS freed");
    }
}

/* ************************************************************************** */

int h266_decodeSPS(Bitstream_t *bitstr, h266_sps_t *sps)
{
    TRACE_WARNING(PARAM, "h266_decodeSPS() is not implemented yet");
    return FAILURE;
}

void h266_mapSPS(h266_sps_t *sps, int64_t offset, int64_t size, FILE *xml)
{
    TRACE_WARNING(PARAM, "h266_mapSPS() is not implemented yet");
}

void h266_freeSPS(h266_sps_t **sps_ptr)
{
    if (*sps_ptr != NULL)
    {
        free(*sps_ptr);
        *sps_ptr = NULL;

        TRACE_1(PARAM, ">> SPS freed");
    }
}

/* ************************************************************************** */

int h266_decodePPS(Bitstream_t *bitstr, h266_pps_t *pps, h266_sps_t **sps_array)
{
    TRACE_WARNING(PARAM, "h266_decodePPS() is not implemented yet");
    return FAILURE;
}

void h266_mapPPS(h266_pps_t *pps, int64_t offset, int64_t size, FILE *xml)
{
    TRACE_WARNING(PARAM, "h266_mapPPS() is not implemented yet");
}

void h266_freePPS(h266_pps_t **pps_ptr)
{
    if (*pps_ptr != NULL)
    {
        free(*pps_ptr);
        *pps_ptr = NULL;

        TRACE_1(PARAM, ">> PPS freed");
    }
}

/* ************************************************************************** */
