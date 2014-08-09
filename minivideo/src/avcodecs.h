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

/* ************************************************************************** */

//! Sample type
typedef enum SampleType_e
{
    sample_AUDIO = 0,

    sample_VIDEO,
    sample_VIDEO_IDR,
    sample_VIDEO_PARAM,

    sample_TEXT_FILE

} SampleType_e;

/* ************************************************************************** */

//! Container file formats
typedef enum ContainerFormat_e
{
    CONTAINER_UNKNOWN = 0,

    CONTAINER_ES      = 1,  //!< Plain "Elementary Stream" format (not a container!)
    CONTAINER_ASF     = 2,  //!< ASF "Advanced Systems Format" (.asf, .wma, .wmv)
    CONTAINER_AVI     = 3,  //!< AVI "Audio Video Interleave" (.avi)
    CONTAINER_FLAC    = 4,  //!< FLAC (.flac)
    CONTAINER_FLV     = 5,  //!< SWF "Small Web Format" (.flv)
    CONTAINER_OGG     = 6,  //!< OGG (.ogg, .ogv, ...)
    CONTAINER_MKV     = 7,  //!< Matroska (.mkv, .webm)
    CONTAINER_MP3     = 8,  //!< MP3 "Elementary Stream" format (not a container!)
    CONTAINER_MP4     = 9,  //!< ISOM "ISO Base Media" format (.mov, .mp4, .3gp, .f4v, ...)
    CONTAINER_MPEG_PS = 10, //!< MPEG "Program Stream" (.mpg, .vob, ...)
    CONTAINER_MPEG_TS = 11  //!< MPEG "Transport Stream" (.ts, .mts, .m2ts, ...)

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
    CODEC_AC3         = 7,  //!< AC3, or Dobly Digital, or A/52A
    CODEC_EAC3        = 8,  //!< Enhanced AC3, or Dobly Digital Plus, or A/52B
    CODEC_DTS         = 9,
    CODEC_DTS_HD      = 10,
    CODEC_FLAC        = 11,
    CODEC_OPUS        = 12,
    CODEC_VORBIS      = 13, //!< Ogg Vorbis
    CODEC_WMA         = 14, //!< Windows Media Audio

    // Video codecs
    CODEC_MPEG12      = 32, //!< MPEG-1/2 videos
    CODEC_MPEG4       = 33, //!< MPEG-4 Part 2 "ASP" or XVID
    CODEC_MSMPEG4     = 34, //!< MPEG-4 Part 2 "ASP" implementation from Microsoft (note: 3 different versions exist), NOT compatible with regular MPEG-4 ASP. Used in divx <= 3.
    CODEC_H263        = 35, //!< H.263, sometimes found inside "mobile" 3GP files
    CODEC_H264        = 36, //!< H.264 or MPEG-4 Part 10 "AVC"
    CODEC_VP4         = 37,
    CODEC_VP6         = 38,
    CODEC_VP8         = 39,
    CODEC_VP9         = 40,
    CODEC_VC1         = 41, //!< VC-1 or Windows Media Video 9
    CODEC_VC2         = 42, //!< VC-2 or Dirac is an open and royalty-free video compression format developed by BBC Research
    CODEC_WMV1        = 42, //!< Windows Media Video 7
    CODEC_WMV2        = 42, //!< Windows Media Video 8
    CODEC_WMV3        = 43, //!< WMV3 implements the VC-1 or WMV9 Simple and Main Profiles
    CODEC_WMVA        = 44, //!< WMVA is the original implementation of the VC-1 or WMV9 Advanced Profile (considered deprecated)
    CODEC_WVC1        = 45, //!< WVC1 implements a more recent and fully compliant version of the VC-1 or WMV9 Advanced Profile

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

//! Picture extraction repartition mode
typedef enum PictureRepartition_e
{
    PICTURE_UNFILTERED  = 0,
    PICTURE_ORDERED     = 1,
    PICTURE_DISTRIBUTED = 2

} PictureRepartition_e;

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

const char *getContainerString(ContainerFormat_e container, int long_description);
const char *getCodecString(StreamType_e type, AVCodec_e codec);
const char *getPictureString(PictureFormat_e picture);

/* ************************************************************************** */
#endif /* AV_CODECS_H */
