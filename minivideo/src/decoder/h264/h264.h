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
 * \file      h264.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_H
#define H264_H

// minivideo headers
#include "h264_decodingcontext.h"

/* ************************************************************************** */

/*!
 * \brief Initialize an H.264 decoder.
 * \param *input_video: A pointer to a MediaFile_t structure, containing every informations available about the current video file.
 * \param tid: The video track ID to decode.
 * \return A pointer to a DecodingContext_t.
 *
 * This decoder is based on the 'ITU-T H.264' recommendation:
 * - 'Advanced Video Coding for generic audiovisual services'
 * It also correspond to 'ISO/IEC 14496-10' international standard, part 10:
 * - 'Advanced Video Coding'.
 *
 * You can download the H.264 specification for free on the ITU website:
 * http://www.itu.int/rec/T-REC-H.264
 *
 * The very first step to H.264 bitstream decoding. Initialize DecodingContext,
 * then start the decoding process, which loop on NAL Units found in the bitstream.
 * Each NAL Unit is processed following it's content type.
 */
DecodingContext_t *h264_init(MediaFile_t *input_video, unsigned tid);

/*!
 * \brief Decode H.264 bitstream.
 * \param *dc: A pointer to a valid DecodingContext_t structure.
 * \param sid: The sample ID to decode.
 * \return 1 if succeed, 0 otherwise.
 *
 * If a video frame is decoded, the result will be stored in intermediate format
 * in the Macroblock_t **mb_array, to be exploited by h264_export_surface() or
 * h264_export_file().
 */
int h264_decode(DecodingContext_t *dc, unsigned sid);

int h264_export_surface(DecodingContext_t *dc, OutputSurface_t *out);

int h264_export_file(DecodingContext_t *dc, OutputFile_t *out);

void h264_cleanup(DecodingContext_t *dc);

int checkDecodingContext(DecodingContext_t *dc);

/* ************************************************************************** */
#endif // H264_H
