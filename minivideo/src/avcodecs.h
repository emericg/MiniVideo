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

//! Stream type
typedef enum StreamType_e
{
    stream_UNKNOWN = 0,
    stream_AUDIO   = 1,
    stream_VIDEO   = 2,
    stream_TEXT    = 3

} StreamType_e;

//! Stream level
typedef enum StreamLevel_e
{
    stream_level_UNKNOWN = 0,
    stream_level_PES     = 1,
    stream_level_ES      = 2

} StreamLevel_e;

//! Container file formats
typedef enum ContainerFormat_e
{
    CONTAINER_UNKNOWN = 0,

    CONTAINER_ES      = 1,  //!< Plain ES format (not a container)
    CONTAINER_ASF     = 2,  //!< "Advanced Systems Format" (.asf, .wma, .wmv)
    CONTAINER_AVI     = 3,  //!< "Audio Video Interleave" (.avi)
    CONTAINER_FLAC    = 4,  //!< FLAC (.flac)
    CONTAINER_FLV     = 5,  //!< SWF "Small Web Format" (.flv)
    CONTAINER_OGG     = 6,  //!< OGG (.ogg, .ogv, ...)
    CONTAINER_MKV     = 7,  //!< Matroska (.mkv, .webm)
    CONTAINER_MP3     = 8,  //!< MP3 ES format (not a container)
    CONTAINER_MP4     = 9,  //!< ISOM "ISO Base Media" format (.mov, .mp4, .3gp, .f4v, ...)
    CONTAINER_MPEG_PS = 10, //!< MPEG Program Stream (.mpg, .vob, ...)
    CONTAINER_MPEG_TS = 11  //!< MPEG Transport Stream (.ts, .mts, .m2ts, ...)

} ContainerFormat_e;

//! Audio and Video compression standards (codecs)
typedef enum AVCodec_e
{
    CODEC_UNKNOWN     = 0,

    // Audio codecs
    CODEC_PCM         = 1,  //!< Pulse-code modulation (not a codec) (PCM, LPCM, DPCM, ADPCM ?)
    CODEC_MPEG_L1     = 2,  //!< MPEG 1/2 Audio Layer I
    CODEC_MPEG_L2     = 3,  //!< MPEG 1/2 Audio Layer II
    CODEC_MPEG_L3     = 4,  //!< MPEG 1/2 Audio Layer III
    CODEC_AAC         = 5,  //!< Advanced Audio Coding
    CODEC_AAC_HE      = 6,  //!< High Efficiency Advanced Audio Coding
    CODEC_AC3         = 7,  //!< AC3, or Dobly Digital, or A/52A */
    CODEC_EAC3        = 8,  //!< Enhanced AC3, or Dobly Digital Plus, or A/52B */
    CODEC_DTS         = 9,
    CODEC_DTS_HD      = 10,
    CODEC_FLAC        = 11,
    CODEC_OPUS        = 12,
    CODEC_VORBIS      = 13, //!< Ogg Vorbis
    CODEC_WMA         = 14, //!< Windows Media Audio

    // Video codecs
    CODEC_MPEG12      = 32, //!< MPEG-1/2 videos
    CODEC_MPEG4       = 33, //!< MPEG-4 Part 2 "ASP" or XVID
    CODEC_MSMPEG4     = 34, //!< MPEG-4 Part 2 "ASP" implementation from Microsoft, NOT compatible with regular MPEG-4 ASP. Used in divx <= 3.
    CODEC_H263        = 35, //!< H.263, sometimes found inside "mobile" 3GP files
    CODEC_H264        = 36, //!< H.264 or MPEG-4 Part 10 "AVC"
    CODEC_VC1         = 37, //!< VC1 or Windows Media Video
    CODEC_VC2         = 38, //!< VC2 or Dirac
    CODEC_VP4         = 39,
    CODEC_VP6         = 40,
    CODEC_VP8         = 41,
    CODEC_VP9         = 42,

    // Subtitles codecs
    CODEC_SRT         = 64, //!< SubRip (.srt)
    CODEC_SSA         = 65, //!< "SubStation Alpha" (.ssa)
    CODEC_ASS         = 66, //!< "Advanced SubStation Alpha" (.ass)

} AVCodec_e;

//! Picture file formats
typedef enum PictureFormat_e
{
    PICTURE_UNKNOWN   = 0,

    PICTURE_BMP       = 1,
    PICTURE_JPG       = 2,
    PICTURE_PNG       = 3,
    PICTURE_TGA       = 4,
    PICTURE_YUV444    = 5,  //!< Planar YCbCr file without subsampling
    PICTURE_YUV420    = 6   //!< Planar YCbCr file with 4:2:0 subsampling

} PictureFormat_e;

/* ************************************************************************** */

const char *getContainerString(ContainerFormat_e container);
const char *getCodecString(StreamType_e type, AVCodec_e codec);
const char *getPictureString(PictureFormat_e picture);

/* ************************************************************************** */
#endif /* AV_CODECS_H */
