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
 * \file      h264_nalu.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_NALU_H
#define H264_NALU_H

// minivideo headers
#include "../../bitstream.h"

/* ************************************************************************** */

/*!
 * \brief H.264 NAL Unit - Network Abstraction Layer Unit.
 *
 * From 'ITU-T H.264' recommendation:
 * - 7.3.1 NAL unit syntax.
 * - 7.4.1 NAL unit semantics.
 * - G.7.3.1.1 NAL unit header SVC extension syntax.
 * - H.7.3.1.1 NAL unit header MVC extension syntax.
 * - J.7.3.1.1 NAL unit header 3D-AVC extension syntax.
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
typedef struct h264_nalu_t
{
    // Nal Unit offset
    int64_t nal_offset;             //!< Absolute offset of the NAL Unit header (in byte)

    // Nal Unit header
    uint8_t nal_ref_idc;       //!< Define NAL Unit priority (see NALUnit_RefIdc enum)
    uint8_t nal_unit_type;     //!< Define NAL Unit content type (see NALUnit_Type enum)

    // Nal Unit header (SVC extension)
    uint8_t idr_flag;
    uint8_t priority_id;
    uint8_t no_inter_layer_pred_flag;
    uint8_t dependency_id;
    uint8_t quality_id;
    uint8_t temporal_id;
    uint8_t use_ref_base_pic_flag;
    uint8_t discardable_flag;
    uint8_t output_flag;

    // Nal Unit header (MVC extension)
    uint8_t non_idr_flag;
    //uint8_t priority_id; // re-used
    uint16_t view_id;
    //uint8_t temporal_id; // re-used
    uint8_t anchor_pic_flag;
    uint8_t inter_view_flag;

    // Nal Unit header (3D AVC extension)
    uint8_t view_idx;
    uint8_t depth_flag;
    //uint8_t non_idr_flag; // re-used
    //uint8_t temporal_id; // re-used
    //uint8_t anchor_pic_flag; // re-used
    //uint8_t inter_view_flag; // re-used

} h264_nalu_t;

//! H.264 NAL Unit content type
typedef enum h264_nalu_type_e
{
    NALU_TYPE_UNKNOWN      = 0,
    NALU_TYPE_SLICE        = 1,     //!< Contain a coded slice
    NALU_TYPE_DPA          = 2,     //!< Contain a data partition, type A
    NALU_TYPE_DPB          = 3,     //!< Contain a data partition, type B
    NALU_TYPE_DPC          = 4,     //!< Contain a data partition, type C
    NALU_TYPE_IDR          = 5,     //!< Contain an IDR
    NALU_TYPE_SEI          = 6,     //!< Contain Supplemental Enhancement Information
    NALU_TYPE_SPS          = 7,     //!< Contain a Sequence Parameter Set
    NALU_TYPE_PPS          = 8,     //!< Contain a Picture Parameter Set
    NALU_TYPE_AUD          = 9,     //!< Contain/Indicate a Access Unit Delimiter
    NALU_TYPE_END_SEQUENCE = 10,    //!< Indicate end of sequence
    NALU_TYPE_END_STREAM   = 11,    //!< Indicate end of stream
    NALU_TYPE_FILL         = 12,    //!< Contain filler data
    NALU_TYPE_SPS_EXT      = 13,    //!< Sequence Parameter Set extension
    NALU_TYPE_PREFIX_NAL   = 14,    //!< Prefix NAL unit
    NALU_TYPE_SPS_SUBSET   = 15,    //!< Subset Sequence Parameter Set
    NALU_TYPE_DPS          = 16,    //!< Depth Parameter Set
    NALU_TYPE_SLICE_AUX    = 19,    //!< Coded slice of an auxiliary coded picture without partitioning
    NALU_TYPE_SLICE_EXT    = 20,    //!< Coded slice extension
    NALU_TYPE_SLICE_EXT_3D = 21,    //!< Coded slice extension for depth view / 3D-AVC texture view

} h264_nalu_type_e;

//! H.264 NAL Unit priority
typedef enum h264_nalu_priority_e
{
    NALU_PRIORITY_HIGHEST    = 3,   //!< High priority, mandatory to start decoding a sequence
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_LOW        = 1,
    NALU_PRIORITY_DISPOSABLE = 0    //!< Disposable NAL Unit, can be discarded without consequences

} h264_nalu_priority_e;

/* ************************************************************************** */

h264_nalu_t *h264_nalu_init(void);

void h264_nalu_reset(h264_nalu_t *nalu);
int h264_nalu_get_next_startcode(Bitstream_t *bitstr, h264_nalu_t *nalu);
int h264_nalu_parse_header(Bitstream_t *bitstr, h264_nalu_t *nalu);

int h264_nalu_clean_sample(Bitstream_t *bitstr);

const char *h264_nalu_get_string_type0(h264_nalu_t *nalu);
const char *h264_nalu_get_string_type1(unsigned type);

/* ************************************************************************** */
#endif // H264_NALU_H
