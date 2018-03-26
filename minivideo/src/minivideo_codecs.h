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
 * \file      minivideo_codecs.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef MINIVIDEO_CODECS_H
#define MINIVIDEO_CODECS_H

#include "minivideo_avutils.h"
#include "minivideo_export.h"

/* ************************************************************************** */

//! Audio and Video compression standards (codecs)
typedef enum Codecs_e
{
    CODEC_UNKNOWN       = 0,

    // Audio codecs ////////////////////////////////////////////////////////////

    CODEC_MPEG_L1       =  1,   //!< MP1, or MPEG 1/2 Audio Layer I
    CODEC_MPEG_L2       =  2,   //!< MP2, or MPEG 1/2 Audio Layer II
    CODEC_MPEG_L3       =  3,   //!< MP3, or MPEG 1/2 Audio Layer III
    CODEC_AAC           =  4,   //!< Advanced Audio Coding, or MPEG-2 Part 7 and MPEG-4 Part 3
    CODEC_MPEG4_ALS     =  7,   //!<
    CODEC_MPEG4_CELP    =  8,   //!<
    CODEC_MPEG4_DST     =  9,   //!<
    CODEC_MPEG4_HVXC    = 10,   //!<
    CODEC_MPEG4_SLS     = 11,   //!<
    CODEC_MPEGH_3D_AUDIO= 12,   //!<

    CODEC_SPEEX         = 24,   //!< Xiph Speex
    CODEC_VORBIS        = 25,   //!< Xiph Ogg Vorbis
    CODEC_OPUS          = 26,   //!< Xiph Opus

    CODEC_AC2           = 32,   //!< Dolby Labs AC-2
    CODEC_AC3           = 33,   //!< Dolby Digital AC-3, or A/52A
    CODEC_EAC3          = 34,   //!< Dolby Digital Plus, or Enhanced AC-3, or A/52B
    CODEC_AC4           = 35,   //!< Dolby AC-4
    //Dolby Digital EX
    //Dolby Digital Live
    //Dolby TrueHD

    CODEC_DTS           = 42,   //!< "Dedicated To Sound"
    CODEC_DTS_HD        = 43,
    CODEC_DTS_X         = 44,

    CODEC_WMA           = 49,   //!< Windows Media Audio
    CODEC_MPC           = 50,   //!< Musepack
    CODEC_GSM           = 51,   //!< GSM
    CODEC_ATRAC         = 52,   //!< Sony ATRAC
    CODEC_ATRAC3plus    = 53,   //!< Sony ATRAC 3 plus
    CODEC_AMR           = 54,   //!< AMR, or Adaptive Multi Rate

    // Uncommon audio codecs ///////////////////////////////////////////////////

    CODEC_RA_14         = 64,   //!< Real Audio 1 / 14.4
    CODEC_RA_28         = 65,   //!< Real Audio 2 / 28.8
    CODEC_RA_cook       = 66,   //!< Real Audio G2 / Cook
    CODEC_RA_ralf       = 67,   //!< Real Audio Lossless Format
    CODEC_IAC2          = 68,   //!< Indeo Audio Codec

    // Lossless audio codecs ///////////////////////////////////////////////////

    CODEC_APE           = 128,   //!< Monkey's Audio
    CODEC_FLAC          = 129,   //!< Free Lossless Audio Codec
    CODEC_ALAC          = 130,   //!< Apple Lossless Audio Codec

    // Video codecs ////////////////////////////////////////////////////////////

    CODEC_H261          = 256,  //!< H.261
    CODEC_MPEG1         = 257,  //!< MPEG-1 Part 2 (ISO/IEC 11172-2)
    CODEC_MPEG2         = 258,  //!< MPEG-2 Part 2 (ISO/IEC 13818-2) or H.262
    CODEC_MPEG4_ASP     = 259,  //!< MPEG-4 Part 2 "ASP" (note:  XVID is a popular implementation)
    CODEC_MSMPEG4       = 260,  //!< MPEG-4 Part 2 "ASP" implementation from Microsoft (note: 3 versions exists / NOT compatible with regular MPEG-4 ASP / Used in divx <= 3)
    CODEC_H263          = 261,  //!< H.263 (and its numerous variants)
    CODEC_H264          = 262,  //!< H.264 or MPEG-4 Part 10 "AVC" (ISO/IEC 14496-10)
    CODEC_H265          = 263,  //!< H.265 or MPEG-H Part 2 "HEVC" (ISO/IEC 23008-2)

    CODEC_WMV7          = 264,  //!< Windows Media Video 7
    CODEC_WMV8          = 265,  //!< Windows Media Video 8
    CODEC_WMV9          = 266,  //!< Windows Media Video 9
    CODEC_WMSCR         = 267,  //!< Windows Media Screen (7-9)
    CODEC_WMP           = 268,  //!< Windows Media Picture

    CODEC_RV10          = 269,  //!< RealVideo
    CODEC_RV20          = 270,  //!< RealVideo G2
    CODEC_RV30          = 271,  //!< RealVideo 3
    CODEC_RV40          = 272,  //!< RealVideo 4

    CODEC_VC1           = 273,  //!< VC-1 or Windows Media Video 9
    CODEC_VC2           = 274,  //!< VC-2 or Dirac is an open and royalty-free video compression format developed by BBC Research
    CODEC_VC3           = 275,  //!< VC-3, Avid DNxHD is a popular implementation
    CODEC_VC5           = 276,  //!< VC-5 or CineForm

    CODEC_PRORES_422_PROXY  = 277,  //!< Apple ProRes 422 (Proxy)
    CODEC_PRORES_422_LT     = 278,  //!< Apple ProRes 422 (LT)
    CODEC_PRORES_422        = 279,  //!< Apple ProRes 422
    CODEC_PRORES_422_HQ     = 280,  //!< Apple ProRes 422 (HQ)
    CODEC_PRORES_4444       = 281,  //!< Apple ProRes 4444
    CODEC_PRORES_4444_XQ    = 282,  //!< Apple ProRes 4444 (XQ)

    CODEC_REDCode       = 283,  //!< REDCode

    CODEC_VP3           = 284,  //!< On2 VP3 Video
    CODEC_VP4           = 285,  //!< On2 VP4 Video / Xiph Ogg Theora
    CODEC_VP5           = 286,  //!< On2 VP5 Video
    CODEC_VP6           = 287,  //!< On2 VP6 Video
    CODEC_VP7           = 288,  //!< On2 VP7 Video
    CODEC_VP8           = 289,  //!< Google VP8
    CODEC_VP9           = 290,  //!< Google VP9

    CODEC_DAALA         = 291,  //!< Xiph Daala
    CODEC_THOR          = 292,  //!< Cisco Thor
    CODEC_AV1           = 293,  //!< AOMedia Video 1

    // Uncommon video codecs ///////////////////////////////////////////////////

    CODEC_CINEPAK       = 300,  //!< Cinepak
    CODEC_SNOW          = 301,  //!< Snow
    CODEC_SPARK         = 302,  //!< Sorenson Spark
    CODEC_SVQ1          = 303,  //!< Sorenson Video 1
    CODEC_SVQ3          = 304,  //!< Sorenson Video 3
    CODEC_INDEO2        = 305,  //!< Intel Indeo Video 2
    CODEC_INDEO3        = 306,  //!< Intel Indeo Video 3
    CODEC_INDEO4        = 307,  //!< Intel Indeo Video 4
    CODEC_INDEO5        = 308,  //!< Intel Indeo Video 5
    CODEC_CanopusHQ     = 309,  //!< Canopus HQ
    CODEC_CanopusHQA    = 310,  //!< Canopus HQ Alpha
    CODEC_CanopusHQX    = 311,  //!< Canopus HQX
    CODEC_BINK          = 312,  //!< Bink Video!
    CODEC_BINK2         = 313,  //!< Bink2
    CODEC_icod          = 314,  //!< Apple Intermediate Codec
    CODEC_rpza          = 315,  //!< Apple Video / "road pizza"
    CODEC_QtAnimation   = 316,  //!< Apple QuickTime Animation
    CODEC_QtGraphics    = 317,  //!< Apple QuickTime Graphics

    // Images codecs ///////////////////////////////////////////////////////////

    CODEC_JPEG          = 350,  //!< JPEG
    CODEC_MJPEG         = 351,  //!< Motion JPEG
    CODEC_MJPEG2K       = 352,  //!< Motion JPEG 2000
    CODEC_CorePNG       = 399,  //!< CorePNG

    // Lossless video codecs ///////////////////////////////////////////////////

    CODEC_FFV1          = 400,  //!< FFV1
    CODEC_CanopusLL     = 401,  //!< Canopus Lossless

    // Subtitles codecs ////////////////////////////////////////////////////////

    CODEC_SRT           = 512,  //!< SubRip (.srt)
    CODEC_SSA           = 513,  //!< SubStation Alpha (.ssa)
    CODEC_ASS           = 514,  //!< Advanced SubStation Alpha (.ass)
    CODEC_USF           = 515,  //!< Universal Subtitle Format (.usf)
    CODEC_VobSub        = 516,  //!< VobSub (.sub/.idx)
    CODEC_MicroDVD      = 517,  //!< MicroDVD (.sub)
    CODEC_SAMI          = 518,  //!< Synchronized Accessible Media Interchange (.smi, .sami)
    CODEC_MPEG4_TTXT    = 519,  //!< MPEG-4 Timed Text (.ttxt)
    CODEC_TTML          = 520,  //!< Timed Text Markup Language (.ttml, .dfxp)
    CODEC_WebVTT        = 521,  //!< Web Video Text Tracks (.vtt)

    // Uncompressed audio //////////////////////////////////////////////////////

    CODEC_LPCM          = 1024, //!< Linear pulse-code modulation
    CODEC_LogPCM        = 1025, //!< Logarithmic pulse-code modulation
    CODEC_DPCM          = 1026, //!< Differential pulse-code modulation
    CODEC_ADPCM         = 1027, //!< Adaptative differential pulse-code modulation
    CODEC_PDM           = 1028, //!< Pulse-density modulation

    // Uncompressed video //////////////////////////////////////////////////////

} Codecs_e;

typedef enum CodecProfiles_e
{
    CODEC_PROF_UNKNOWN  = 0,

    PROF_H262_SP        = 1,
    PROF_H262_MP,
    PROF_H262_SNR,
    PROF_H262_Spatial,
    PROF_H262_HP,
    PROF_H262_422,
    PROF_H262_MVP,

    PROF_MPEG4_SP,
    PROF_MPEG4_ASP,
    PROF_MPEG4_AP,
    PROF_MPEG4_SStP,

    PROF_VC1_SIMPLE,
    PROF_VC1_MAIN,
    PROF_VC1_ADVANCED,

    PROF_H264_CBP,
    PROF_H264_BP,
    PROF_H264_XP,
    PROF_H264_MP,
    PROF_H264_HiP,
    PROF_H264_PHiP,
    PROF_H264_CHiP,
    PROF_H264_Hi10P,
    PROF_H264_Hi422P,
    PROF_H264_Hi444PP,
    PROF_H264_Hi10It,
    PROF_H264_Hi422It,
    PROF_H264_Hi444It,
    PROF_H264_M444It,
    PROF_H264_ScBP,
    PROF_H264_ScCBP,
    PROF_H264_ScHiP,
    PROF_H264_ScCHiP,
    PROF_H264_ScHiItP,
    PROF_H264_StHiP,
    PROF_H264_MvHiP,
    PROF_H264_MvDHiP,
    PROF_H264_,

    PROF_H265_Main,
    PROF_H265_Main10,
    PROF_H265_MainStill,
    PROF_H265_Main12,
    PROF_H265_Main444,
    PROF_H265_,

    PROF_VP8_0,
    PROF_VP8_1,

    PROF_VP9_0,
    PROF_VP9_1,
    PROF_VP9_2,
    PROF_VP9_3,

    PROF_AAC_LC,
    PROF_AAC_Main,
    PROF_AAC_SSR,
    PROF_AAC_MainAudio,
    PROF_AAC_Scalable,
    PROF_AAC_HQ,
    PROF_AAC_LD,
    PROF_AAC_LDv2,
    PROF_AAC_Mobile,
    PROF_AAC_AAC,
    PROF_AAC_HE,
    PROF_AAC_HEv2,

} CodecProfiles_e;

//! Picture file formats
typedef enum Pictures_e
{
    PICTURE_UNKNOWN   = 0,

    PICTURE_JPG       = 1,
    PICTURE_WEBP      = 2,
    PICTURE_PNG       = 3,
    PICTURE_TGA       = 4,
    PICTURE_BMP       = 5,

    PICTURE_YUV444    = 16,     //!< 4:4:4 YCbCr planar
    PICTURE_YUV420    = 17      //!< 4:2:0 YCbCr planar

} Pictures_e;

/* ************************************************************************** */

minivideo_EXPORT const char *getCodecString(const StreamType_e type, const Codecs_e codec, const bool long_description = false);
minivideo_EXPORT const char *getCodecProfileString(const CodecProfiles_e profile, const bool long_description = false);

minivideo_EXPORT const char *getPictureString(const Pictures_e picture, const bool long_description = false);

/* ************************************************************************** */
#endif // MINIVIDEO_CODECS_H
