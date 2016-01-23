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
 * \file      h264_decodingcontext.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_DECODING_CONTEXT_H
#define H264_DECODING_CONTEXT_H

// minivideo headers
#include "../../import.h"
#include "h264_nalu.h"
#include "h264_parameterset_struct.h"
#include "h264_slice_struct.h"
#include "h264_macroblock_struct.h"

/* ************************************************************************** */

//! H.264 decoding context
typedef struct DecodingContext_t
{
    // Input
    ////////////////////////////////////////////////////////////////////////////

    MediaFile_t *VideoFile;          //!< H.264 input video
    Bitstream_t *bitstr;             //!< Bitstream, linked to the input file

    // Output
    ////////////////////////////////////////////////////////////////////////////

    char output_directory[4096];     //!< Absolute path of the directory where to save the thumbnails. Linux has a pathname limit of 4,096 characters.
    int output_format;               //!< The picture file format
    int picture_quality;             //!< The quality we want for exported picture in range [1;100]
    int picture_number;              //!< The number of thumbnail(s) we want to extract
    int picture_extractionmode;      //!< The method of distribution for thumbnails extraction
    int picture_exported;            //!< Number of picture successfully exported

    // H.264 Decoding context
    ////////////////////////////////////////////////////////////////////////////

    bool decoderRunning;             //!< Set 'decoderRunning' to false to stop video decoding
    unsigned frameCounter;           //!< The number of frame decoded
    unsigned idrCounter;             //!< The number of idr frame decoded
    unsigned errorCounter;           //!< The number of decoding error so far

    // NAL
    nalu_t *active_nalu;             //!< Current NAL Unit

    // SPS
    unsigned int active_sps;         //!< ID of the last/current SPS. May be inaccurate!
    sps_t *sps_array[MAX_SPS];

    // PPS
    unsigned int active_pps;         //!< ID of the last/current PPS. May be inaccurate!
    pps_t *pps_array[MAX_PPS];

    // SEI
    sei_t *active_sei;               //!< Current SEI

    // Slice
    bool IdrPicFlag;                 //!< Current frame type (INTRA=1, INTER=0)
    unsigned int frame_num;          //!< Current frame number, set from slice
    slice_t *active_slice;           //!< Current Slice

    // Macroblocks
    unsigned int CurrMbAddr;         //!< Address of the current macroblock
    unsigned int PicSizeInMbs;       //!< Number of macroblocks per frame
    Macroblock_t **mb_array;         //!< Array containing all macroblocks of current frame

    // Some useful parameters
    unsigned int profile_idc;        //!< Current H.264 profile, set from SPS
    unsigned int ChromaArrayType;    //!< Current subsampling scheme, set from SPS
    bool entropy_coding_mode_flag;   //!< Current entropy coding method, set from PPS

    // Quantization parameters
    int normAdjust4x4[6][4][4];      //!< Used to build LevelScale4x4. Computed during decoder initialization.
    int normAdjust8x8[6][8][8];      //!< Used to build LevelScale8x8. Computed during decoder initialization.

} DecodingContext_t;

/* ************************************************************************** */
#endif // H264_DECODING_CONTEXT_H
