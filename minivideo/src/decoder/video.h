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
 * \file      video.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef VIDEO_H
#define VIDEO_H

// minivideo headers
#include "../typedef.h"

/* ************************************************************************** */

//! The color space used by the video
typedef enum ColorSpace_e
{
    CS_UNKNOWN = 0,     //!< Unknown color space
    CS_YUV     = 1,     //!< YUV (YCbCr) color space
    CS_RGB     = 2      //!< RGB color space
} ColorSpace_e;

//! The subsampling format used by the video
typedef enum Subsampling_e
{
    SS_UNKNOWN = 0,     //!< Unknown subsampling
    SS_400     = 1,     //!< Greyscale
    SS_420     = 2,     //!< 4:2:0 subsampling
    SS_422     = 3,     //!< 4:2:2 subsampling
    SS_444     = 4      //!< 4:4:4 subsampling
} Subsampling_e;

//! Frame rate (picture per second)
typedef enum FrameRate_e
{
    FR_UNKNOWN = 0,   //!< Unknown frame rate
    FR_24p,           //!< 24 frames
    FR_24p_NTSC,      //!< 24 frames * 1000/1001 = 23.976 frames
    FR_25p,
    FR_30p,
    FR_30p_NTSC,      //!< 30 frames * 1000/1001 = 29.970 frames
    FR_48p,
    FR_50p,
    FR_50i,           //!< 50 interlaced fields (25 frames)
    FR_60p,
    FR_60i,           //!< 60 interlaced fields (30 frames)
    FR_60i_NTSC,      //!< 60 interlaced fields * 1000/1001 = 59.940 interlaced fields (29.970 frames)
    FR_72p
} FrameRate_e;

/* ************************************************************************** */
#endif /* VIDEO_H */
