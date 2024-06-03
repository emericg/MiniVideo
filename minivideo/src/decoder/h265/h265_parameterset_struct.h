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
 * \file      h265_parameterset_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H265_PARAMETER_SET_STRUCT_H
#define H265_PARAMETER_SET_STRUCT_H

#include <cstdint>

/* ************************************************************************** */

#define H265_MAX_VPS     32
#define H265_MAX_SPS     32
#define H265_MAX_PPS     256

/* ************************************************************************** */

/*!
 * \struct h265_vps_t
 * \brief VPS - Video Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.1 Video parameter set syntax.
 * - 7.4.3.1 Video parameter set semantics.
 */
typedef struct h265_vps_t
{
    //

} h265_vps_t;

/*!
 * \struct h265_sps_t
 * \brief SPS - Sequence Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.2 Sequence parameter set syntax.
 * - 7.4.3.2 Sequence parameter set semantics.
 */
typedef struct h265_sps_t
{
    //

} h265_sps_t;

/*!
 * \struct h265_pps_t
 * \brief PPS - Picture Parameter Set.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.3 Picture parameter set syntax.
 * - 7.4.3.3 Picture parameter set semantics.
 */
typedef struct h265_pps_t
{
    //

} h265_pps_t;

/*!
 * \struct h265_aud_t
 * \brief AUD - Access Unit Delimiter.
 *
 * From 'ITU-T H.265' recommendation:
 * - 7.3.2.5 Access unit delimiter syntax.
 * - 7.4.3.5 Access unit delimiter semantics.
 */
typedef struct h265_aud_t
{
    uint8_t pic_type;

} h265_aud_t;

/* ************************************************************************** */
#endif // H265_PARAMETER_SET_STRUCT_H
