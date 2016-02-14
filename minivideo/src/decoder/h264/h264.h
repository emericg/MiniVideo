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
 * \brief Decode H.264 bitstream.
 * \param *input_video A pointer to a MediaFile_t structure, containing every informations available about the current video file.
 * \param *output_directory The directory where to save exported pictures.
 * \param picture_format The picture file format.
 * \param picture_quality The quality we want for exported picture [1;100].
 * \param picture_number The number of thumbnail(s) we want to extract.
 * \param picture_extractionmode The method of distribution for thumbnails extraction.
 * \return 1 if succeed, 0 otherwise.
 *
 * This decoder is based on the 'ITU-T H.264' recommendation:
 * 'Advanced Video Coding for generic audiovisual services'
 * It also correspond to 'ISO/IEC 14496-10' international standard, part 10:
 * 'Advanced Video Coding'.
 *
 * You can download the H.264 specification for free on the ITU website:
 * http://www.itu.int/rec/T-REC-H.264
 *
 * The very first step to H.264 bitstream decoding. Initialize DecodingContext,
 * then start the decoding process, which loop on NAL Units found in the bitstream.
 * Each NAL Unit is processed following it's content type.
 */
int h264_decode(MediaFile_t *input_video,
                const char *output_directory,
                const int picture_format,
                const int picture_quality,
                const int picture_number,
                const int picture_extractionmode);

DecodingContext_t *initDecodingContext(MediaFile_t *media);
void freeDecodingContext(DecodingContext_t **dc_ptr);
int checkDecodingContext(DecodingContext_t *dc);

/* ************************************************************************** */
#endif // H264_H
