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

    // Audio codecs
    CODEC_LPCM        =  1, //!< Linear pulse-code modulation (not a codec)
    CODEC_DPCM        =  2, //!< Differential pulse-code modulation (not a codec)
    CODEC_ADPCM       =  3, //!< Adaptative differential pulse-code modulation (not a codec)

    CODEC_APE         =  8, //!< Monkey's Audio
    CODEC_FLAC        =  9, //!< Free Lossless Audio Codec
    CODEC_ALAC        = 10, //!< Apple Lossless Audio Codec

    CODEC_MPEG_L1     = 16, //!< MPEG 1/2 Audio Layer I
    CODEC_MPEG_L2     = 17, //!< MPEG 1/2 Audio Layer II
    CODEC_MPEG_L3     = 18, //!< MPEG 1/2 Audio Layer III
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
    CODEC_MPEG4       = 65, //!< MPEG-4 Part 2 "ASP", XVID is a popular implementation
    CODEC_MSMPEG4     = 66, //!< MPEG-4 Part 2 "ASP" implementation from Microsoft (note: 3 different versions exist), NOT compatible with regular MPEG-4 ASP. Used in divx <= 3.
    CODEC_H263        = 67, //!< H.263 and variants
    CODEC_H264        = 68, //!< H.264 or MPEG-4 Part 10 "AVC"
    CODEC_H265        = 69, //!< H.265 or MPEG-H Part 2 "HEVC" or ISO/IEC 23008-2

    CODEC_WMV1        = 70, //!< Windows Media Video 7
    CODEC_WMV2        = 71, //!< Windows Media Video 8
    CODEC_WMV3        = 72, //!< WMV3 implements the VC-1 or WMV9 Simple and Main Profiles
    CODEC_WMVA        = 73, //!< WMVA is the original implementation of the VC-1 or WMV9 Advanced Profile (considered deprecated)
    CODEC_WVC1        = 74, //!< WVC1 implements a more recent and fully compliant version of the VC-1 or WMV9 Advanced Profile

    CODEC_VP4         = 75, //!< Xiph Ogg Theora
    CODEC_VP5         = 76,
    CODEC_VP6         = 77,
    CODEC_VP7         = 78,
    CODEC_VP8         = 79,
    CODEC_VP9         = 80,
    CODEC_VP10        = 81,

    CODEC_VC1         = 82, //!< VC-1 or Windows Media Video 9
    CODEC_VC2         = 83, //!< VC-2 or Dirac is an open and royalty-free video compression format developed by BBC Research
    CODEC_VC3         = 84, //!< VC-3, DNxHD is a popular implementation
    CODEC_VC5         = 85, //!< VC-5 or CineForm

    CODEC_DAALA       = 86, //!< Xiph Daala

    // Subtitles codecs
    CODEC_SRT         = 128, //!< SubRip (.srt)
    CODEC_SSA         = 129, //!< "SubStation Alpha" (.ssa)
    CODEC_ASS         = 130, //!< "Advanced SubStation Alpha" (.ass)

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

//! Sample type
typedef enum SampleType_e
{
    sample_AUDIO = 0,

    sample_VIDEO,
    sample_VIDEO_IDR,
    sample_VIDEO_PARAM,

    sample_TEXT_FILE

} SampleType_e;

//! Bitrate mode
typedef enum BitrateMode_e
{
    BITRATE_UNKNOWN = 0,

    BITRATE_CBR     = 1, //!< Constant Bitrate
    BITRATE_VBR     = 2, //!< Variable Bitrate
    BITRATE_ABR     = 3, //!< Average Bitrate
    BITRATE_CVBR    = 4  //!< Constrained Variable Bitrate

} BitrateMode_e;

/* ************************************************************************** */

//! Audio speakers from WAVEFORMATEXTENSIBLE 'dwChannelMask' field
typedef enum AudioSpeakers_e
{
    SPEAKER_FRONT_LEFT              = 0x1,
    SPEAKER_FRONT_RIGHT             = 0x2,
    SPEAKER_FRONT_CENTER            = 0x4,
    SPEAKER_LOW_FREQUENCY           = 0x8,
    SPEAKER_BACK_LEFT               = 0x10,
    SPEAKER_BACK_RIGHT              = 0x20,
    SPEAKER_FRONT_LEFT_OF_CENTER    = 0x40,
    SPEAKER_FRONT_RIGHT_OF_CENTER   = 0x80,
    SPEAKER_BACK_CENTER             = 0x100,
    SPEAKER_SIDE_LEFT               = 0x200,
    SPEAKER_SIDE_RIGHT              = 0x400,
    SPEAKER_TOP_CENTER              = 0x800,
    SPEAKER_TOP_FRONT_LEFT          = 0x1000,
    SPEAKER_TOP_FRONT_CENTER        = 0x2000,
    SPEAKER_TOP_FRONT_RIGHT         = 0x4000,
    SPEAKER_TOP_BACK_LEFT           = 0x8000,
    SPEAKER_TOP_BACK_CENTER         = 0x10000,
    SPEAKER_TOP_BACK_RIGHT          = 0x20000

} AudioSpeakers_e;

//! MPEG audio channel modes
typedef enum MPEGChannelMode_e
{
    CHAN_MONO           = 0,
    CHAN_STEREO         = 1,
    CHAN_STEREO_JOINT   = 2,
    CHAN_DUAL           = 3

} MPEGChannelMode_e;

//! Audio channel modes
typedef enum ChannelMode_e
{
    CHANS_UNKNOWN       = 0,

    CHANS_MONO          = 1,
    CHANS_STEREO        = 2,
    CHANS_30            = 3,
    CHANS_QUAD          = 4,
    CHANS_50            = 5,
    CHANS_60            = 6,
    CHANS_70            = 7,

    CHANS_SURROUND_21   = 16,
    CHANS_SURROUND_31   = 17,
    CHANS_SURROUND_41   = 18,
    CHANS_SURROUND_51   = 19,
    CHANS_SURROUND_71   = 20,
    CHANS_SURROUND_91   = 21,
    CHANS_SURROUND_111  = 22,

    CHANS_SURROUND_102  = 32,
    CHANS_SURROUND_222  = 33

} ChannelMode_e;

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
typedef enum SubSampling_e
{
    SS_UNKNOWN = 0,     //!< Unknown subsampling

    SS_400     = 1,     //!< Greyscale
    SS_420     = 2,     //!< 4:2:0 subsampling
    SS_422     = 3,     //!< 4:2:2 subsampling
    SS_444     = 4      //!< 4:4:4 subsampling

} SubSampling_e;

//! Frame rate (picture per second)
typedef enum FrameRate_e
{
    FR_UNKNOWN = 0,   //!< Unknown frame rate

    FR_24p,           //!< 24 frames
    FR_24p_NTSC,      //!< (24 frames * 1000) / 1001 = 23.976 frames/s
    FR_25p,
    FR_30p,
    FR_30p_NTSC,      //!< (30 frames * 1000) / 1001 = 29.970 frames/s
    FR_48p,
    FR_48p_NTSC,      //!< (48 frames * 1000) / 1001 = 47.952 frames/s
    FR_50p,
    FR_50i,           //!< 50 interlaced fields (25 frames)
    FR_60p,
    FR_60p_NTSC,      //!< (60 frames * 1000) / 1001 = 59.970 frames/s
    FR_60i,           //!< 60 interlaced fields (30 frames)
    FR_60i_NTSC,      //!< 60 interlaced fields * 1000/1001 = 59.940 interlaced fields (29.970 frames)
    FR_72p,
    FR_120p,
    FR_240p

} FrameRate_e;

/* ************************************************************************** */

const char *getContainerString(ContainerFormat_e container, int long_description);
const char *getCodecString(StreamType_e type, AVCodec_e codec);
const char *getPictureString(PictureFormat_e picture);

/* ************************************************************************** */
#endif // AV_CODECS_H
