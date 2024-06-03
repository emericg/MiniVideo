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
 * \file      h266_parameterset_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H266_PARAMETER_SET_STRUCT_H
#define H266_PARAMETER_SET_STRUCT_H

#include <cstdint>

/* ************************************************************************** */

#define H266_MAX_VPS     32
#define H266_MAX_SPS     32
#define H266_MAX_PPS     256

/* ************************************************************************** */

// DCI - Decoding Capability Information
// 7.3.2.1 Decoding capability information syntax.
// 7.4.3.1 Decoding capability information semantics.

// OPI - Operating Point Information
// 7.3.2.2 Operating point information syntax.
// 7.4.3.2 Operating point information semantics.

/*!
 * \struct h266_vps_t
 * \brief VPS - Video Parameter Set.
 *
 * From 'ITU-T H.266' recommendation:
 * - 7.3.2.3 Video parameter set syntax.
 * - 7.4.3.3 Video parameter set semantics.
 */
typedef struct h266_vps_t
{
    //

} h266_vps_t;

/*!
 * \struct h266_sps_t
 * \brief SPS - Sequence Parameter Set.
 *
 * From 'ITU-T H.266' recommendation:
 * - 7.3.2.4 Sequence parameter set syntax.
 * - 7.4.3.4 Sequence parameter set semantics.
 */
typedef struct h266_sps_t
{
    //

} h266_sps_t;

/*!
 * \struct h266_pps_t
 * \brief PPS - Picture Parameter Set.
 *
 * From 'ITU-T H.266' recommendation:
 * - 7.3.2.5 Picture parameter set syntax.
 * - 7.4.3.5 Picture parameter set semantics
 */
typedef struct h266_pps_t
{
    //

} h266_pps_t;

/* ************************************************************************** */
#endif // H266_PARAMETER_SET_STRUCT_H
