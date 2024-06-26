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
 * \file      h266_nalu.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

#ifndef H266_NALU_H
#define H266_NALU_H
/* ************************************************************************** */

// minivideo headers
#include "../../bitstream.h"

/* ************************************************************************** */

/*!
 * \brief H.266 NAL Unit - Network Abstraction Layer Unit.
 *
 * From 'ITU-T H.266' recommendation:
 * - 7.3.1 NAL unit syntax.
 * - 7.4.2 NAL unit semantics.
 *
 * A NAL (Network Abstraction Layer) is specified to format that data and provide
 * header information in a manner appropriate for conveyance on a variety of
 * communication channels or storage media.
 *
 * All coded video data are organized into NAL units, each of which is effectively
 * a packet that contains an integer number of bytes. The first byte of each NAL
 * unit is a header byte that contains an indication of the type of data in the
 * NAL unit, and the remaining bytes contain payload data of the type indicated
 * by the header.
 *
 * The NAL unit structure definition specifies a generic format for use in both
 * packet-oriented and bitstream-oriented transport systems, and a series of NAL
 * units generated by an encoder is referred to as a NAL unit stream.
 */
typedef struct h266_nalu_t
{
    // Nal Unit offset
    int64_t nal_offset;             //!< Absolute offset of the NAL Unit header (in byte)

    // Nal Unit header
    uint8_t nuh_layer_id;      //!<
    uint8_t nal_unit_type;     //!< Define NAL Unit content type (see h266_nalu_type_e enum)
    uint8_t nuh_temporal_id_plus1; //!<

} h266_nalu_t;

//! H.266 NAL Unit content type
typedef enum h266_nalu_type_e
{
    NALU_TYPE_TRAIL_NUT     = 0,    //!< Coded slice of a trailing picture or subpicture
    NALU_TYPE_STSA_NUT      = 1,    //!< Coded slice of an STSA picture or subpicture
    NALU_TYPE_RADL_NUT      = 2,    //!< Coded slice of a RADL picture or subpicture
    NALU_TYPE_RASL_NUT      = 3,    //!< Coded slice of a RASL picture or subpicture

    NALU_TYPE_RSV_VCL_4     = 4,    //!< Reserved non-IRAP VCL NAL unit types
    NALU_TYPE_RSV_VCL_5     = 5,
    NALU_TYPE_RSV_VCL_6     = 6,

    NALU_TYPE_IDR_W_RADL    = 7,    //!< Coded slice of an IDR picture or subpicture
    NALU_TYPE_IDR_N_LP      = 8,
    NALU_TYPE_CRA_NUT       = 9,    //!< Coded slice of a CRA picture or subpicture
    NALU_TYPE_GDR_NUT       = 10,   //!< Coded slice of a GDR picture or subpicture

    NALU_TYPE_RSV_IRAP_11   = 11,   //!< Reserved IRAP VCL NAL unit type

    NALU_TYPE_OPI_NUT       = 12,   //!< Operating Point Information
    NALU_TYPE_DCI_NUT       = 13,   //!< Decoding Capability Information
    NALU_TYPE_VPS_NUT       = 14,   //!< Video Parameter Set
    NALU_TYPE_SPS_NUT       = 15,   //!< Sequence Parameter Set
    NALU_TYPE_PPS_NUT       = 16,   //!< Picture Parameter Set
    NALU_TYPE_PREFIX_APS_NUT= 17,   //!< Adaptation Parameter Set
    NALU_TYPE_SUFFIX_APS_NUT= 18,
    NALU_TYPE_PH_NUT        = 19,   //!< Picture Header
    NALU_TYPE_AUD_NUT       = 20,   //!< Access Unit Delimiter
    NALU_TYPE_EOS_NUT       = 21,   //!< End of Sequence
    NALU_TYPE_EOB_NUT       = 22,   //!< End of Bitstream
    NALU_TYPE_PREFIX_SEI_NUT= 23,   //!< Supplemental Enhancement Information
    NALU_TYPE_SUFFIX_SEI_NUT= 24,
    NALU_TYPE_FD_NUT        = 25,   //!< Filler Data

} h266_nalu_type_e;

/* ************************************************************************** */

h266_nalu_t *h266_nalu_init(void);

void h266_nalu_reset(h266_nalu_t *nalu);
int h266_nalu_get_next_startcode(Bitstream_t *bitstr, h266_nalu_t *nalu);
int h266_nalu_parse_header(Bitstream_t *bitstr, h266_nalu_t *nalu);

int h266_nalu_clean_sample(Bitstream_t *bitstr);

const char *h266_nalu_get_string_type0(h266_nalu_t *nalu);
const char *h266_nalu_get_string_type1(unsigned type);

/* ************************************************************************** */
#endif // H266_NALU_H
