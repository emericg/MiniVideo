/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef MINIVIDEO_CODECS_H
#define MINIVIDEO_CODECS_H
/* ************************************************************************** */

#include "minivideo_avutils.h"
#include "minivideo_export.h"

/* ************************************************************************** */

//! Audio and Video compression standards (codecs)
typedef enum Codecs_e
{
    CODEC_UNKNOWN       = 0,

    // Audio codecs ////////////////////////////////////////////////////////////

    CODEC_MPEG_L1       =  1,   //!< MP1, or MPEG 1/2 Audio Layer I (ISO/IEC 11172-3 and ISO/IEC 13818-3)
    CODEC_MPEG_L2       =  2,   //!< MP2, or MPEG 1/2 Audio Layer II
    CODEC_MPEG_L3       =  3,   //!< MP3, or MPEG 1/2/2.5 Audio Layer III

    CODEC_AAC           =  4,   //!< Advanced Audio Coding (MPEG-2 Part 7 and MPEG-4 Part 3, Subpart 4) (ISO/IEC 13818-7 and ISO/IEC 14496-3)
    CODEC_MPEG4_HVXC    =  5,   //!< MPEG-4 Part 3, Subpart 2
    CODEC_MPEG4_CELP    =  6,   //!< MPEG-4 Part 3, Subpart 3
    CODEC_MPEG4_TwinVQ  =  7,   //!< MPEG-4 Part 3, Subpart 4
    CODEC_MPEG4_HILN    =  8,   //!< MPEG-4 Part 3, Subpart 7
    CODEC_MPEG4_DST     =  9,   //!< MPEG-4 Part 3, Subpart 10
    CODEC_MPEG4_ALS     = 10,   //!< MPEG-4 Part 3, Subpart 11
    CODEC_MPEG4_SLS     = 11,   //!< MPEG-4 Part 3, Subpart 12
    CODEC_MPEGH_3D_AUDIO= 12,   //!< ISO/IEC 23008-3 (MPEG-H Part 3)

    CODEC_SPEEX         = 24,   //!< Xiph Speex
    CODEC_VORBIS        = 25,   //!< Xiph Vorbis
    CODEC_OPUS          = 26,   //!< Xiph Opus

    CODEC_AC2           = 32,   //!< Dolby Labs, AC-2
    CODEC_AC3           = 33,   //!< Dolby Digital, AC-3, or A/52A
    CODEC_EAC3          = 34,   //!< Dolby Digital Plus, or Enhanced AC-3, or AC-3+, or A/52B
    CODEC_AC4           = 35,   //!< Dolby AC-4
    CODEC_DolbyTrueHD   = 36,   //!< Dolby TrueHD
    //Dolby Digital EX          // ?
    //Dolby Digital Surround EX
    //Dolby Digital Live

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

    CODEC_APE           = 100,  //!< Monkey's Audio
    CODEC_ALAC          = 101,  //!< Apple Lossless Audio Codec
    CODEC_FLAC          = 102,  //!< Free Lossless Audio Codec
    CODEC_WAVPACK       = 103,  //!< WavPack

    // Video codecs ////////////////////////////////////////////////////////////

    CODEC_MPEG1         = 256,  //!< MPEG-1 Part 2 (ISO/IEC 11172-2)
    CODEC_MPEG4_ASP     = 257,  //!< MPEG-4 Part 2 "ASP" (note: XVID is a popular implementation)
    CODEC_MPEG4_IVC     = 258,  //!< MPEG-4 Part 33 "IVC"
    CODEC_MPEG5_EVC     = 259,  //!< MPEG-5 Part 1 "EVC"
    CODEC_MPEG5_LCEVC   = 260,  //!< MPEG-5 Part 2 "LCEVC"

    CODEC_H120          = 120,  //!< H.120, the first video codec
    CODEC_H261          = 261,  //!< H.261
    CODEC_H262          = 262,  //!< MPEG-2 Part 2 (ISO/IEC 13818-2) or H.262
    CODEC_H263          = 263,  //!< H.263 (and its numerous variants)
    CODEC_H264          = 264,  //!< MPEG-4 Part 10 "AVC" (ISO/IEC 14496-10) or H.264
    CODEC_H265          = 265,  //!< MPEG-H Part 2 "HEVC" (ISO/IEC 23008-2) or H.265
    CODEC_H266          = 266,  //!< MPEG-I Part 3 "VVC" (ISO/IEC 23090-3) or H.266

    CODEC_VP3           = 273,  //!< On2 VP3 Video
    CODEC_VP4           = 274,  //!< On2 VP4 Video / Xiph Ogg Theora
    CODEC_VP5           = 275,  //!< On2 VP5 Video
    CODEC_VP6           = 276,  //!< On2 VP6 Video
    CODEC_VP7           = 277,  //!< On2 VP7 Video
    CODEC_VP8           = 278,  //!< Google VP8
    CODEC_VP9           = 279,  //!< Google VP9

    CODEC_DAALA         = 280,  //!< Xiph Daala
    CODEC_THOR          = 281,  //!< Cisco Thor
    CODEC_AV1           = 282,  //!< AOM (Alliance for Open Media) Video 1

    CODEC_AVS1          = 291,  //!<
    CODEC_AVS2          = 292,  //!<
    CODEC_AVS3          = 293,  //!<

    CODEC_VC1           = 301,  //!< VC-1 (subset of Windows Media Video 9)
    CODEC_VC2           = 302,  //!< VC-2 (subset of Dirac)
    CODEC_VC3           = 303,  //!< VC-3 (Avid DNxHD)
    CODEC_VC5           = 305,  //!< VC-5 (CineForm variant)
    CODEC_VC6           = 306,  //!< VC-6

    CODEC_MSMPEG4       = 310,  //!< MPEG-4 Part 2 "ASP" from Microsoft (NOT compatible with MPEG-4 ASP / 3 version exists / Used in divx <= 3)
    CODEC_WMV7          = 311,  //!< Windows Media Video 7
    CODEC_WMV8          = 312,  //!< Windows Media Video 8
    CODEC_WMV9          = 313,  //!< Windows Media Video 9
    CODEC_WMSCR         = 314,  //!< Windows Media Screen (7-9)
    CODEC_WMP           = 315,  //!< Windows Media Picture

    CODEC_RV10          = 320,  //!< RealVideo
    CODEC_RV20          = 321,  //!< RealVideo G2
    CODEC_RV30          = 322,  //!< RealVideo 3
    CODEC_RV40          = 323,  //!< RealVideo 4

    CODEC_INDEO2        = 330,  //!< Intel Indeo Video 2
    CODEC_INDEO3        = 331,  //!< Intel Indeo Video 3
    CODEC_INDEO4        = 332,  //!< Intel Indeo Video 4
    CODEC_INDEO5        = 333,  //!< Intel Indeo Video 5
    CODEC_SPARK         = 334,  //!< Sorenson Spark
    CODEC_SVQ1          = 335,  //!< Sorenson Video 1
    CODEC_SVQ3          = 336,  //!< Sorenson Video 3
    CODEC_CanopusHQ     = 337,  //!< Canopus HQ
    CODEC_CanopusHQA    = 338,  //!< Canopus HQ Alpha
    CODEC_CanopusHQX    = 339,  //!< Canopus HQX
    CODEC_BINK          = 340,  //!< Bink Video!
    CODEC_BINK2         = 341,  //!< Bink2
    CODEC_CINEPAK       = 342,  //!< Cinepak
    CODEC_DIRAC         = 343,  //!< Dirac
    CODEC_SNOW          = 344,  //!< FFmpeg Snow
    CODEC_icod          = 345,  //!< Apple Intermediate Codec
    CODEC_rpza          = 356,  //!< Apple Video "road pizza"
    CODEC_QtAnimation   = 347,  //!< Apple QuickTime Animation
    CODEC_QtGraphics    = 348,  //!< Apple QuickTime Graphics

    // Lossy video codecs //////////////////////////////////////////////////////

    CODEC_CINEFORM          = 360,  //!< CineForm
    CODEC_REDCode           = 361,  //!< REDCode
    CODEC_DNxHD             = 362,

    CODEC_PRORES_422_PROXY  = 370,  //!< Apple ProRes 422 (Proxy)
    CODEC_PRORES_422_LT     = 371,  //!< Apple ProRes 422 (LT)
    CODEC_PRORES_422        = 372,  //!< Apple ProRes 422
    CODEC_PRORES_422_HQ     = 373,  //!< Apple ProRes 422 (HQ)
    CODEC_PRORES_4444       = 374,  //!< Apple ProRes 4444
    CODEC_PRORES_4444_XQ    = 375,  //!< Apple ProRes 4444 (XQ)
    CODEC_PRORES_RAW        = 376,  //!< Apple ProRes RAW
    CODEC_PRORES_RAW_HQ     = 377,  //!< Apple ProRes RAW (HQ)

    CODEC_DV_SONY           = 380,  //!< Sony DV
    CODEC_DV_CANOPUS        = 381,  //!< Canopus DV
    CODEC_DV_APPLE          = 382,  //!< Apple DV
    CODEC_DV_PANASONIC      = 383,  //!< Panasonic DV

    // Lossless video codecs ///////////////////////////////////////////////////

    CODEC_FFV1              = 390,  //!< FFmpeg FFV1
    CODEC_CanopusLL         = 391,  //!< Canopus Lossless

    // Images codecs ///////////////////////////////////////////////////////////

    CODEC_PNG           = 400,  //!< PNG
    CODEC_CorePNG       = 401,  //!< Video PNG
    CODEC_BMP           = 402,  //!< Windows Bitmap
    CODEC_TGA           = 403,  //!< Truevision Graphics Adapter
    CODEC_TIFF          = 404,  //!< Tagged Image File Format

    CODEC_JPEG          = 410,  //!< JPEG
    CODEC_MJPEG         = 411,  //!< Motion JPEG
    CODEC_JPEG_2K       = 412,  //!< JPEG 2000
    CODEC_MJPEG_2K      = 413,  //!< Motion JPEG 2000
    CODEC_JPEG_XR       = 414,  //!< JPEG XR
    CODEC_JPEG_XS       = 415,  //!< JPEG XS
    CODEC_JPEG_XT       = 416,  //!< JPEG XT
    CODEC_JPEG_XL       = 417,  //!< JPEG XL

    CODEC_GIF           = 420,  //!< Graphic Interchange Format
    CODEC_WEBP          = 421,  //!< WebP
    CODEC_HEIF          = 422,  //!< High Efficiency Image Format
    CODEC_AVIF          = 423,  //!< AV1 Image Format

    // Subtitles codecs ////////////////////////////////////////////////////////

    CODEC_SRT           = 512,  //!< SubRip (.srt)
    CODEC_MicroDVD      = 513,  //!< MicroDVD (.sub)
    CODEC_SSA           = 514,  //!< SubStation Alpha (.ssa)
    CODEC_ASS           = 515,  //!< Advanced SubStation Alpha (.ass)
    CODEC_USF           = 516,  //!< Universal Subtitle Format (.usf)
    CODEC_SSF           = 517,  //!< Structured Subtitle Format  (.ssf)
    CODEC_SAMI          = 518,  //!< Synchronized Accessible Media Interchange (.smi, .sami)
    CODEC_CMML          = 519,  //!< Continuous Media Markup Language
    CODEC_SMIL          = 520,  //!< Synchronized Multimedia Integration Language (.smil)
    CODEC_STL           = 521,  //!< Spruce Subtitle File (.stl)
    CODEC_TTML          = 522,  //!< Timed Text Markup Language (variants: DFXP, SMPTE-TT, EBU-TT, IMSC) (.ttml, .dfxp)
    CODEC_MPEG4_TTXT    = 523,  //!< MPEG-4 Part 17 "Timed Text" (.ttxt)
    CODEC_WebVTT        = 524,  //!< Web Video Text Tracks (.vtt)
    CODEC_Kate          = 525,  //!< Karaoke And Text Encapsulation
    CODEC_LRC           = 526,  //!< "Song Lyrics"

    CODEC_Telext        = 540,  //!< Teletext subtitles
    CODEC_DvbSub        = 541,  //!< Digital Video Broadcasting subtitles (same format than Teletext?)
    CODEC_VobSub        = 542,  //!< VobSub (.sub/.idx)
    CODEC_XSub          = 543,  //!< DivX embedded sutitles
    CODEC_AriSub        = 544,  //!< ARIB STD-B24 subtitles
    CODEC_PGS           = 545,  //!< Presentation Graphics Subtitles (.sup)
    CODEC_TextST        = 546,  //!< TextST subtitles (same format than PGS?)
    CODEC_CineCanvas    = 547,  //!< CineCanvas
    CODEC_PAC           = 548,  //!< Presentation Audio/Video Coding
    CODEC_XDS           = 549,  //!< Extended Data Services

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

    PROF_VC1_SIMPLE,
    PROF_VC1_MAIN,
    PROF_VC1_ADVANCED,

    PROF_MPEG4_SP,
    PROF_MPEG4_ASP,
    PROF_MPEG4_AP,
    PROF_MPEG4_SStP,

    PROF_H262_SP,
    PROF_H262_MP,
    PROF_H262_SNR,
    PROF_H262_Spatial,
    PROF_H262_HP,
    PROF_H262_422,
    PROF_H262_MVP,

    PROF_H264_BP,
    PROF_H264_CBP,
    PROF_H264_XP,
    PROF_H264_MP,
    PROF_H264_HiP,
    PROF_H264_PHiP,
    PROF_H264_CHiP,
    PROF_H264_Hi10P,
    PROF_H264_PHi10P,
    PROF_H264_Hi10IntraP,
    PROF_H264_Hi422P,
    PROF_H264_Hi422IntraP,
    PROF_H264_Hi444P,
    PROF_H264_Hi444PP,
    PROF_H264_Hi444IntraP,
    PROF_H264_M444IntraP,
    PROF_H264_ScBP,
    PROF_H264_ScCBP,
    PROF_H264_ScHiP,
    PROF_H264_ScCHiP,
    PROF_H264_ScHiIntraP,
    PROF_H264_StHiP,
    PROF_H264_MvHiP,
    PROF_H264_MvDHiP,
    PROF_H264_EMvDHiP,
    PROF_H264_MfcHiP,
    PROF_H264_MfcDHiP,
    PROF_H264_unknown, // because they keep adding and adding them...

    PROF_H265_Main,
    PROF_H265_Main_still,
    PROF_H265_Main_intra,
    PROF_H265_Main10,
    PROF_H265_Main10_still,
    PROF_H265_Main10_intra,
    PROF_H265_Main12,
    PROF_H265_Main12_intra,
    PROF_H265_Monochrome,
    PROF_H265_Monochrome10,
    PROF_H265_Monochrome12,
    PROF_H265_Monochrome12_intra,
    PROF_H265_Monochrome16,
    PROF_H265_Monochrome16_intra,
    PROF_H265_Main422_10,
    PROF_H265_Main422_10_intra,
    PROF_H265_Main422_12,
    PROF_H265_Main422_12_intra,
    PROF_H265_Main444,
    PROF_H265_Main444_still,
    PROF_H265_Main444_intra,
    PROF_H265_Main444_10,
    PROF_H265_Main444_10_intra,
    PROF_H265_Main444_12,
    PROF_H265_Main444_12_intra,
    PROF_H265_Main444_16_still,
    PROF_H265_Main444_16_intra,
    PROF_H265_HighThroughput_444,
    PROF_H265_HighThroughput_444_10,
    PROF_H265_HighThroughput_444_14,
    PROF_H265_HighThroughput_444_16_intra,
    PROF_H265_ScreenExtended_Main,
    PROF_H265_ScreenExtended_Main10,
    PROF_H265_ScreenExtended_Main_444,
    PROF_H265_ScreenExtended_Main_444_10,
    PROF_H265_ScreenExtended_HighThroughput_444,
    PROF_H265_ScreenExtended_HighThroughput_444_10,
    PROF_H265_ScreenExtended_HighThroughput_444_14,
    PROF_H265_Multiview_Main,   // Annex G Multiview
    PROF_H265_3D_Main,          // Annex I 3D
    PROF_H265_Scalable_Main,    // Annex H Scalable
    PROF_H265_Scalable_Main10,
    PROF_H265_Scalable_Main444,
    PROF_H265_Scalable_Monochrome,
    PROF_H265_Scalable_Monochrome_12,
    PROF_H265_Scalable_Monochrome_16,
    PROF_H265_unknown, // because they keep adding and adding them...

    PROF_H266_Main10,
    PROF_H266_Main10_still,
    PROF_H266_Main10_444,
    PROF_H266_Main10_444_still,
    PROF_H266_Main10_multilayer,
    PROF_H266_Main10_444_multilayer,
    PROF_H266_Main12,
    PROF_H266_Main12_intra,
    PROF_H266_Main12_still,
    PROF_H266_Main12_444,
    PROF_H266_Main12_444_intra,
    PROF_H266_Main12_444_still,
    PROF_H266_Main16_444,
    PROF_H266_Main16_444_intra,
    PROF_H266_Main16_444_still,
    PROF_H266_unknown, // because they keep adding and adding them...

    PROF_VP8_0,
    PROF_VP8_1,

    PROF_VP9_0,
    PROF_VP9_1,
    PROF_VP9_2,
    PROF_VP9_3,

    PROF_AV1_Main,
    PROF_AV1_High,
    PROF_AV1_Professional,

    PROF_AAC_LC = 256,
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
    PROF_AAC_LTP,
    PROF_AAC_unknown, // because they keep adding and adding them...

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

CodecProfiles_e getH264CodecProfile(const unsigned profil_idc,
                                    const bool constraint_set1_flag = false,
                                    const bool constraint_set2_flag = false,
                                    const bool constraint_set3_flag = false,
                                    const bool constraint_set4_flag = false,
                                    const bool constraint_set5_flag = false,
                                    const bool constraint_set6_flag = false);
CodecProfiles_e getH265CodecProfile(const unsigned profil_idc,
                                    const bool general_one_picture_only_constraint_flag = false,
                                    const bool general_max_8bit_constraint_flag = false,
                                    const int chroma_format_idc = 1);

minivideo_EXPORT const char *getCodecString(const StreamType_e type, const Codecs_e codec, const bool long_description = false);
minivideo_EXPORT const char *getCodecProfileString(const CodecProfiles_e profile, const bool long_description = false);

minivideo_EXPORT const char *getPictureString(const Pictures_e picture, const bool long_description = false);

/* ************************************************************************** */

minivideo_EXPORT const char *getColorPrimariesString(const ColorPrimaries_e primaries);
minivideo_EXPORT const char *getColorTransferCharacteristicString(const ColorTransferCharacteristic_e transfert);
minivideo_EXPORT const char *getColorMatrixString(const ColorSpace_e space);

/* ************************************************************************** */
#endif // MINIVIDEO_CODECS_H
