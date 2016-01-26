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

#include "typedef.h"
#include "avutils.h"

/* ************************************************************************** */

//! Container file formats
typedef enum ContainerFormat_e
{
    CONTAINER_UNKNOWN = 0,

    // General purpose containers
    CONTAINER_AVI     =  1, //!< AVI "Audio Video Interleave" (.avi, ...)
    CONTAINER_ASF     =  2, //!< ASF "Advanced Systems Format" (.asf, .wma, .wmv, ...)
    CONTAINER_MKV     =  3, //!< Matroska (.mkv, .mka, .webm)
    CONTAINER_MP4     =  4, //!< ISOM "ISO Base Media" format (.mov, .mp4, .3gp, .f4v, ...)
    CONTAINER_MPEG_PS =  5, //!< MPEG "Program Stream" (.mpg, .vob, ...)
    CONTAINER_MPEG_TS =  6, //!< MPEG "Transport Stream" (.ts, .mts, .m2ts, ...)
    CONTAINER_MXF     =  7, //!< MXF "Material eXchange Format" (.mxf)
    CONTAINER_FLV     =  8, //!< SWF "Small Web Format" (.flv)
    CONTAINER_OGG     =  9, //!< OGG (.ogg, .ogv, .oga, ...)
    CONTAINER_RM      = 10, //!< RealMedia (.rm, .rmvb)

    // Audio containers
    CONTAINER_FLAC    = 12, //!< FLAC "Free Lossless Audio Codec" (.flac)
    CONTAINER_WAVE    = 13, //!< WAVE "Waveform Audio File Format" (.wav)

    // ES formats (not containers!)
    CONTAINER_ES      = 16, //!< Undefined "Elementary Stream"
    CONTAINER_ES_AAC  = 17, //!< AAC "Elementary Stream"
    CONTAINER_ES_AC3  = 18, //!< AC3 "Elementary Stream"
    CONTAINER_ES_MP3  = 19, //!< MP3 "Elementary Stream"

} ContainerFormat_e;

//! Audio and Video compression standards (codecs)
typedef enum AVCodec_e
{
    CODEC_UNKNOWN     = 0,

    // Raw audio
    CODEC_LPCM        =  1, //!< Linear pulse-code modulation
    CODEC_LogPCM      =  2, //!< Logarithmic pulse-code modulation
    CODEC_DPCM        =  3, //!< Differential pulse-code modulation
    CODEC_ADPCM       =  4, //!< Adaptative differential pulse-code modulation

    // Audio codecs
    CODEC_APE         =  8, //!< Monkey's Audio
    CODEC_FLAC        =  9, //!< Free Lossless Audio Codec
    CODEC_ALAC        = 10, //!< Apple Lossless Audio Codec

    CODEC_MPEG_L1     = 16, //!< MP1, or MPEG 1/2 Audio Layer I
    CODEC_MPEG_L2     = 17, //!< MP2, or MPEG 1/2 Audio Layer II
    CODEC_MPEG_L3     = 18, //!< MP3, or MPEG 1/2 Audio Layer III
    CODEC_AAC         = 19, //!< Advanced Audio Coding
    CODEC_AAC_HE      = 20, //!< High Efficiency Advanced Audio Coding

    CODEC_AC3         = 21, //!< AC3, or Dobly Digital, or A/52A
    CODEC_EAC3        = 22, //!< Enhanced AC3, or Dobly Digital Plus, or A/52B

    CODEC_DTS         = 23,
    CODEC_DTS_HD      = 24,

    CODEC_WMA         = 25, //!< Windows Media Audio
    CODEC_MPC         = 26, //!< Musepack
    CODEC_SPEEX       = 27, //!< Xiph Speex
    CODEC_VORBIS      = 28, //!< Xiph Ogg Vorbis
    CODEC_OPUS        = 29, //!< Xiph Opus

    // Video codecs
    CODEC_MPEG12      = 64, //!< MPEG-1/2 videos, also known as H.262
    CODEC_MPEG4_ASP   = 65, //!< MPEG-4 Part 2 "ASP", XVID is a popular implementation
    CODEC_MSMPEG4     = 66, //!< MPEG-4 Part 2 "ASP" implementation from Microsoft (note: 3 different versions exist), NOT compatible with regular MPEG-4 ASP. Used in divx <= 3.
    CODEC_H263        = 67, //!< H.263 and its numerous variants
    CODEC_H264        = 68, //!< H.264 or MPEG-4 Part 10 "AVC"
    CODEC_H265        = 69, //!< H.265 or MPEG-H Part 2 "HEVC" or ISO/IEC 23008-2

    CODEC_WMV1        = 70, //!< Windows Media Video 7
    CODEC_WMV2        = 71, //!< Windows Media Video 8
    CODEC_WMV3        = 72, //!< WMV3 implements the VC-1 or WMV9 Simple and Main Profiles
    CODEC_WMVA        = 73, //!< WMVA is the original implementation of the VC-1 or WMV9 Advanced Profile (considered deprecated)
    CODEC_WVC1        = 74, //!< WVC1 implements a more recent and fully compliant version of the VC-1 or WMV9 Advanced Profile

    CODEC_VP3         = 75, //!< On2 VP3 Video
    CODEC_VP4         = 76, //!< On2 VP4 Video / Xiph Ogg Theora
    CODEC_VP5         = 77, //!< On2 VP5 Video
    CODEC_VP6         = 78, //!< On2 VP6 Video
    CODEC_VP7         = 79, //!< On2 VP7 Video
    CODEC_VP8         = 80, //!< Google VP8
    CODEC_VP9         = 81, //!< Google VP9
    CODEC_VP10        = 82, //!< Google VP10

    CODEC_DAALA       = 83, //!< Xiph Daala

    CODEC_VC1         = 84, //!< VC-1 or Windows Media Video 9
    CODEC_VC2         = 85, //!< VC-2 or Dirac is an open and royalty-free video compression format developed by BBC Research
    CODEC_VC3         = 86, //!< VC-3, Avid DNxHD is a popular implementation
    CODEC_VC5         = 87, //!< VC-5 or CineForm

    CODEC_PRORES_4444       = 88, //!< Apple ProRes 4444
    CODEC_PRORES_4444_XQ    = 89, //!< Apple ProRes 4444 (XQ)
    CODEC_PRORES_422_HQ     = 90, //!< Apple ProRes 422 (HQ)
    CODEC_PRORES_442        = 91, //!< Apple ProRes 422
    CODEC_PRORES_442_PROXY  = 92, //!< Apple ProRes 422 (Proxy)
    CODEC_PRORES_442_LT     = 93, //!< Apple ProRes 422 (LT)

    // Deprecated video codecs
    CODEC_CINEPAK     = 190, //!< Cinepak
    CODEC_SVQ1        = 191, //!< Sorenson Video
    CODEC_SVQ3        = 192, //!< Sorenson Video 3
    CODEC_IV31        = 193, //!< Intel Indeo Video 3
    CODEC_IV41        = 194, //!< Intel Indeo Video 4
    CODEC_IV50        = 195, //!< Intel Indeo Video 5

    // Raw video

    // Subtitles codecs
    CODEC_SRT         = 200, //!< SubRip (.srt)
    CODEC_SSA         = 201, //!< "SubStation Alpha" (.ssa)
    CODEC_ASS         = 202, //!< "Advanced SubStation Alpha" (.ass)

} AVCodec_e;

//! Picture file formats
typedef enum PictureFormat_e
{
    PICTURE_UNKNOWN   = 0,

    PICTURE_BMP       = 1,
    PICTURE_JPG       = 2,
    PICTURE_PNG       = 3,
    PICTURE_WEBP      = 4,
    PICTURE_TGA       = 5,

    PICTURE_YUV444    = 16,  //!< Planar YCbCr file without subsampling
    PICTURE_YUV420    = 17   //!< Planar YCbCr file with 4:2:0 subsampling

} PictureFormat_e;

/* ************************************************************************** */

const char *getContainerString(ContainerFormat_e container, bool long_description);
const char *getCodecString(StreamType_e type, AVCodec_e codec, bool long_description);
const char *getPictureString(PictureFormat_e picture, bool long_description);

/* ************************************************************************** */
#endif // AV_CODECS_H
