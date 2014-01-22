/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \file      avcodecs.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef AV_CODECS_H
#define AV_CODECS_H

/* ************************************************************************** */

//! Audio and Video compression standards (codecs)
typedef enum AVCodec_e
{
    CODEC_UNKNOWN     = 0,

    // Audio codecs
    CODEC_MP1         = 1,
    CODEC_MP2         = 2,
    CODEC_MP3         = 3, //!< MPEG 1/2 Audio Layer III
    CODEC_AAC         = 4, //!< Advanced Audio Coding
    CODEC_AC3         = 5, //!< Dolby Digital
    CODEC_DTS         = 6,
    CODEC_PCM         = 7, //!< Pulse-code modulation, not a codec
    CODEC_WAV         = 8, //!< Waveform Audio File Format
    CODEC_WMA         = 9, //!< Windows Media Audio
    CODEC_VORBIS      = 10,
    CODEC_FLAC        = 11,

    // Video codecs
    CODEC_H264        = 12, //!< MPEG-4 Part 10 "AVC"
    CODEC_XVID        = 13, //!< MPEG-4 Part 2 "ASP"
    CODEC_H262        = 14, //!< MPEG-2 Part 2
    CODEC_VP8         = 15,
    CODEC_VC1         = 16, //!< Windows Media Video
    CODEC_DV          = 17,

    // Subtitles codecs
    CODEC_SRT         = 18, //!< SubRip (.srt)
    CODEC_ASS         = 19, //!< SubStation Alpha (.ass .ssa)
    CODEC_SUB         = 20  //!< VobSub (.sub .idx)

} AVCodec_e;

//! Container file formats
typedef enum ContainerFormat_e
{
    CONTAINER_UNKNOWN = 0,

    CONTAINER_ES      = 1, //!< Plain ES format, not a container at all...
    CONTAINER_AVI     = 2,
    CONTAINER_MP4     = 3, //!< Support mp4, mov, ... extensions
    CONTAINER_MKV     = 4, //!< Support mkv and webm extensions
    CONTAINER_PS      = 5, //!< Support mpg, vob, ... extensions
    CONTAINER_TS      = 6,

} ContainerFormat_e;

//! Picture file formats
typedef enum PictureFormat_e
{
    PICTURE_UNKNOWN   = 0,

    PICTURE_JPG       = 1,
    PICTURE_PNG       = 2,
    PICTURE_BMP       = 3,
    PICTURE_TGA       = 4,
    PICTURE_YUV420    = 5,
    PICTURE_YUV444    = 6

} PictureFormat_e;

/* ************************************************************************** */
#endif /* AV_CODECS_H */
