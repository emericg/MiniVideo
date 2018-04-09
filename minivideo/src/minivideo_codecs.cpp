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
 * \file      minivideo_codecs.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#include "minivideo_codecs.h"
#include "minitraces.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wswitch"
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wswitch"
#endif

/* ************************************************************************** */

const char *getCodecString(const StreamType_e type, const Codecs_e codec, const bool long_description)
{
    if (type == stream_AUDIO || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_LPCM:
                if (long_description)
                    return "Linear PCM (Linear Pulse Code Modulation)";
                else
                    return "PCM";
                break;
            case CODEC_LogPCM:
                if (long_description)
                    return "Logarithmic PCM (Logarithmic Pulse Code Modulation)";
                else
                    return "Log PCM";
                break;
            case CODEC_DPCM:
                if (long_description)
                    return "DPCM (Differential Pulse Code Modulation)";
                else
                    return "DPCM";
                break;
            case CODEC_ADPCM:
                if (long_description)
                    return "ADPCM (Adaptative Differential Pulse Code Modulation)";
                else
                    return "ADPCM";
                break;
            case CODEC_PDM:
                if (long_description)
                    return "PDM (Pulse Density Modulation)";
                else
                    return "PDM";
                break;

            case CODEC_MPEG_L1:
                if (long_description)
                    return "MP1 (MPEG-1/2 Layer 1)";
                else
                    return "MP1";
                break;
            case CODEC_MPEG_L2:
                if (long_description)
                    return "MP2 (MPEG-1/2 Layer 2)";
                else
                    return "MP2";
                break;
            case CODEC_MPEG_L3:
                if (long_description)
                    return "MP3 (MPEG-1/2 Layer 3)";
                else
                    return "MP3";
                break;
            case CODEC_AAC:
                if (long_description)
                    return "AAC (Advance Audio Coding)";
                else
                    return "AAC";
                break;
            case CODEC_MPEG4_ALS:
                return "MPEG-4 ALS";
                break;
            case CODEC_MPEG4_CELP:
                return "MPEG-4 CELP";
                break;
            case CODEC_MPEG4_DST:
                return "MPEG-4 DST";
                break;
            case CODEC_MPEG4_HVXC:
                return "MPEG-4 HVXC";
                break;
            case CODEC_MPEG4_SLS:
                return "MPEG-4 SLS";
                break;
            case CODEC_MPEGH_3D_AUDIO:
                return "MPEG-H 3D AUDIO";
                break;

            case CODEC_SPEEX:
                return "Speex";
                break;
            case CODEC_VORBIS:
                return "Ogg Vorbis";
                break;
            case CODEC_OPUS:
                return "OPUS";
                break;

            case CODEC_AC2:
                if (long_description)
                    return "AC-2 (Dolby Labs AC-2)";
                else
                    return "AC-2";
                break;
            case CODEC_AC3:
                if (long_description)
                    return "AC-3 (Dolby Digital AC-3)";
                else
                    return "AC-3";
                break;
            case CODEC_EAC3:
                if (long_description)
                    return "EAC-3 (Dolby Digital AC-3+)";
                else
                    return "EAC-3";
                break;
            case CODEC_AC4:
                if (long_description)
                    return "AC-4 (Dolby AC-4)";
                else
                    return "AC-4";
                break;
            case CODEC_DTS:
                return "DTS";
                break;
            case CODEC_DTS_HD:
                return "DTS-HD";
                break;
            case CODEC_DTS_X:
                return "DTS-X";
                break;

            case CODEC_WMA:
                if (long_description)
                    return "WMA (Windows Media Audio)";
                else
                    return "WMA";
                break;
            case CODEC_MPC:
                if (long_description)
                    return "MPC (Musepack)";
                else
                    return "MPC";
                break;
            case CODEC_GSM:
                return "GSM";
                break;

            case CODEC_ATRAC:
                if (long_description)
                    return "Sony ATRAC";
                else
                    return "ATRAC";
                break;
            case CODEC_ATRAC3plus:
                if (long_description)
                    return "Sony ATRAC 3 plus";
                else
                    return "ATRAC3plus";
                break;

            case CODEC_AMR:
                if (long_description)
                    return "AMR (Adaptive Multi Rate)";
                else
                    return "AMR";
                break;

            case CODEC_RA_14:
                return "RealAudio 1 / 14.4";
                break;
            case CODEC_RA_28:
                return "RealAudio 2 / 28.8";
                break;
            case CODEC_RA_cook:
                return "RealAudio G2 / Cook";
                break;
            case CODEC_RA_ralf:
                return "RealAudio Lossless Format";
                break;
            case CODEC_IAC2:
                return "Indeo Audio Codec";
                break;

            case CODEC_APE:
                if (long_description)
                    return "APE (Monkey's Audio)";
                else
                    return "APE";
                break;
            case CODEC_FLAC:
                if (long_description)
                    return "FLAC (Free Lossless Audio Codec)";
                else
                    return "FLAC";
                break;
            case CODEC_ALAC:
                if (long_description)
                    return "ALAC (Apple Lossless Audio Codec)";
                else
                    return "ALAC";
                break;
        }
    }

    if (type == stream_VIDEO || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_MPEG1:
                return "MPEG-1";
                break;
            case CODEC_H261:
                return "H.261";
                break;
            case CODEC_MPEG2:
                return "MPEG-2";
                break;
            case CODEC_MPEG4_ASP:
                return "MPEG-4 part 2 'ASP'";
                break;
            case CODEC_MSMPEG4:
                return "Microsoft MPEG-4";
                break;
            case CODEC_H263:
                return "H.263";
                break;
            case CODEC_H264:
                if (long_description)
                    return "H.264 (MPEG-4 part 10 'AVC')";
                else
                    return "H.264";
                break;
            case CODEC_H265:
                if (long_description)
                    return "H.265 (MPEG-H part 2 'HEVC')";
                else
                    return "H.265";
                break;

            case CODEC_WMV7:
                if (long_description)
                    return "Windows Media Video 7";
                else
                    return "WMV7";
                break;
            case CODEC_WMV8:
                if (long_description)
                    return "Windows Media Video 8";
                else
                    return "WMV8";
                break;
            case CODEC_WMV9:
                if (long_description)
                    return "Windows Media Video 9";
                else
                    return "WMV9";
                break;
            case CODEC_WMSCR:
                if (long_description)
                    return "Windows Media Screen 7-9";
                else
                    return "WMS 7-9";
                break;
            case CODEC_WMP:
                if (long_description)
                    return "Windows Media Picture";
                else
                    return "WMP";
                break;

            case CODEC_RV10:
                return "RealVideo";
                break;
            case CODEC_RV20:
                return "RealVideo G2";
                break;
            case CODEC_RV30:
                return "RealVideo 3";
                break;
            case CODEC_RV40:
                return "RealVideo 4";
                break;

            case CODEC_VC1:
                if (long_description)
                    return "VC-1 (Windows Media Video)";
                else
                    return "VC-1";
                break;
            case CODEC_VC2:
                if (long_description)
                    return "VC-2 (Dirac)";
                else
                    return "VC-2";
                break;
            case CODEC_VC3:
                if (long_description)
                    return "VC-3 (DNxHD)";
                else
                    return "VC-3";
                break;
            case CODEC_VC5:
                if (long_description)
                    return "VC-5 (CineForm)";
                else
                    return "VC-5";
                break;

            case CODEC_PRORES_422_PROXY:
                return "Apple ProRes 422 (Proxy)";
                break;
            case CODEC_PRORES_422_LT:
                return "Apple ProRes 422 (LT)";
                break;
            case CODEC_PRORES_422:
                return "Apple ProRes 422";
                break;
            case CODEC_PRORES_422_HQ:
                return "Apple ProRes 422 (HQ)";
                break;
            case CODEC_PRORES_4444:
                return "Apple ProRes 4444";
                break;
            case CODEC_PRORES_4444_XQ:
                return "Apple ProRes 4444 (XQ)";
                break;

            case CODEC_REDCode:
                return "REDCode";
                break;

            case CODEC_VP3:
                if (long_description)
                    return "VP3 (Ogg Theora)";
                else
                    return "VP3";
                break;
            case CODEC_VP4:
                return "VP4";
                break;
            case CODEC_VP5:
                return "VP5";
                break;
            case CODEC_VP6:
                return "VP6";
                break;
            case CODEC_VP7:
                return "VP7";
                break;
            case CODEC_VP8:
                return "VP8";
                break;
            case CODEC_VP9:
                return "VP9";
                break;

            case CODEC_DAALA:
                return "Daala";
                break;
            case CODEC_THOR:
                return "Thor";
                break;
            case CODEC_AV1:
                if (long_description)
                    return "AV1 (AOMedia Video 1)";
                else
                    return "AV1";
                break;

            case CODEC_CINEPAK:
                return "Cinepak";
                break;
            case CODEC_SNOW:
                return "SNOW";
                break;
            case CODEC_SPARK:
                return "Sorenson Spark";
                break;
            case CODEC_SVQ1:
                if (long_description)
                    return "SVQ1 (Sorenson Video 1)";
                else
                    return "SVQ1";
                break;
            case CODEC_SVQ3:
                if (long_description)
                    return "SVQ3 (Sorenson Video 3)";
                else
                    return "SVQ3";
                break;
            case CODEC_INDEO2:
                if (long_description)
                    return "Indeo 2 (Intel Indeo Video 2)";
                else
                    return "Indeo 2";
                break;
            case CODEC_INDEO3:
                if (long_description)
                    return "Indeo 3 (Intel Indeo Video 3)";
                else
                    return "Indeo 3";
                break;
            case CODEC_INDEO4:
                if (long_description)
                    return "Indeo 4 (Intel Indeo Video 4)";
                else
                    return "Indeo 4";
                break;
            case CODEC_INDEO5:
                if (long_description)
                    return "Indeo 5 (Intel Indeo Video 5)";
                else
                    return "Indeo 5";
                break;
            case CODEC_CanopusHQ:
                return "Canopus HQ";
                break;
            case CODEC_CanopusHQA:
                return "Canopus HQ Alpha";
                break;
            case CODEC_CanopusHQX:
                return "Canopus HQX";
                break;
            case CODEC_BINK:
                return "Bink Video!";
                break;
            case CODEC_BINK2:
                return "Bink2 Video!";
                break;
            case CODEC_icod:
                return "Apple Intermediate Codec";
                break;
            case CODEC_rpza:
                return "Apple Video";
                break;
            case CODEC_QtAnimation:
                return "Apple QuickTime Animation";
                break;
            case CODEC_QtGraphics:
                return "Apple QuickTime Graphics";
                break;

            case CODEC_CorePNG:
                if (long_description)
                    return "CorePNG (image)";
                else
                    return "PNG";
                break;
            case CODEC_JPEG:
                if (long_description)
                    return "JPEG (image)";
                else
                    return "JPEG";
                break;
            case CODEC_MJPEG:
                if (long_description)
                    return "Motion JPEG (image)";
                else
                    return "Motion JPEG";
                break;
            case CODEC_MJPEG2K:
                if (long_description)
                    return "Motion JPEG 2000 (image)";
                else
                    return "Motion JPEG 2000";
                break;

            case CODEC_FFV1:
                return "FFV1";
                break;
            case CODEC_CanopusLL:
                return "Canopus Lossless";
                break;
        }
    }

    if (type == stream_TEXT || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_SRT:
                return "SubRip";
                break;
            case CODEC_SSA:
                return "SubStation Alpha";
                break;
            case CODEC_ASS:
                return "Advanced SubStation Alpha";
                break;
            case CODEC_USF:
                return "Universal Subtitle Format";
                break;
            case CODEC_VobSub:
                return "VobSub";
                break;
            case CODEC_MicroDVD:
                return "MicroDVD";
                break;
            case CODEC_SAMI:
                return "Synchronized Accessible Media Interchange";
                break;
            case CODEC_MPEG4_TTXT:
                return "MPEG-4 Timed Text";
                break;
            case CODEC_TTML:
                return "Timed Text Markup Language";
                break;
            case CODEC_WebVTT:
                return "Web Video Text Tracks";
                break;
        }
    }

    return "Unknown";
}

/* ************************************************************************** */

const char *getPictureString(const Pictures_e picture, const bool long_description)
{
    switch (picture)
    {
        case PICTURE_BMP:
            return "BMP";
            break;
        case PICTURE_JPG:
            return "JPG";
            break;
        case PICTURE_PNG:
            return "PNG";
            break;
        case PICTURE_WEBP:
            return "WebP";
            break;
        case PICTURE_TGA:
            return "TGA";
            break;
        case PICTURE_YUV444:
            return "YCbCr 4:4:4";
            break;
        case PICTURE_YUV420:
            return "YCbCr 4:2:0";
            break;
        case PICTURE_UNKNOWN:
        default:
            return "UNKNOWN";
            break;
    }
}

/* ************************************************************************** */

const char *getCodecProfileString(const CodecProfiles_e profile, const bool long_description)
{
    switch (profile)
    {
        case PROF_H262_SP:
            return "Simple Profile";
            break;
        case PROF_H262_MP:
            return "Main Profile";
            break;
        case PROF_H262_SNR:
            return "SNR Scalable Profile";
            break;
        case PROF_H262_Spatial:
            return "Spatially Scalable Profile";
            break;
        case PROF_H262_HP:
            return "High Profile";
            break;
        case PROF_H262_422:
            return "4:2:2 Profile";
            break;
        case PROF_H262_MVP:
            return "Multiview Profile";
            break;

        case PROF_MPEG4_SP:
            return "Simple Profile";
            break;
        case PROF_MPEG4_ASP:
            return "Advanced Simple Profile";
            break;
        case PROF_MPEG4_AP:
            return "Advanced Profile";
            break;
        case PROF_MPEG4_SStP:
            return "Simple Studio Profile";
            break;

        case PROF_VC1_SIMPLE:
            return "Simple Profile";
            break;
        case PROF_VC1_MAIN:
            return "Main Profile";
            break;
        case PROF_VC1_ADVANCED:
            return "Advanced Profile";
            break;

        case PROF_H264_CBP:
            return "Constrained Baseline Profile";
            break;
        case PROF_H264_BP:
            return "Baseline Profile";
            break;
        case PROF_H264_XP:
            return "Extended Profile";
            break;
        case PROF_H264_MP:
            return "Main Profile";
            break;
        case PROF_H264_HiP:
            return "High Profile";
            break;
        case PROF_H264_PHiP:
            return "Progressive High Profile";
            break;
        case PROF_H264_CHiP:
            return "Constrained High Profile";
            break;
        case PROF_H264_Hi10P:
            return "High 10 Profile";
            break;
        case PROF_H264_Hi422P:
            return "High 4:2:2 Profile";
            break;
        case PROF_H264_Hi444PP:
            return "High 4:4:4 Predictive Profile";
            break;
        case PROF_H264_Hi10It:
            return "High 10 Intra Profile";
            break;
        case PROF_H264_Hi422It:
            return "High 4:2:2 Intra Profile";
            break;
        case PROF_H264_Hi444It:
            return "High 4:4:4 Intra Profile";
            break;
        case PROF_H264_M444It:
            return "CAVLC 4:4:4 Intra Profile";
            break;
        case PROF_H264_ScBP:
            return "Scalable Baseline Profile";
            break;
        case PROF_H264_ScCBP:
            return "Scalable Constrained Baseline Profile";
            break;
        case PROF_H264_ScHiP:
            return "Scalable High Profile";
            break;
        case PROF_H264_ScCHiP:
            return "Scalable Constrained High Profile";
            break;
        case PROF_H264_ScHiItP:
            return "Scalable High Intra Profile";
            break;
        case PROF_H264_StHiP:
            return "Stereo High Profile";
            break;
        case PROF_H264_MvHiP:
            return "Multiview High Profile";
            break;
        case PROF_H264_MvDHiP:
            return "Multiview Depth High Profile";
            break;
        case PROF_H264_:
            return "Unknown profile...";
            break;

        case PROF_H265_Main:
            return "Main Profile";
            break;
        case PROF_H265_Main10:
            return "Main 10 Profile";
            break;
        case PROF_H265_MainStill:
            return "Main Still Picture Profile";
            break;
        case PROF_H265_Main12:
            return "Main 12 Profile";
            break;
        case PROF_H265_Main444:
            return "Main 444 Profile";
            break;
        case PROF_H265_:
            return "Unknown profile...";
            break;

        case PROF_VP8_0:
            return "Profile 0";
            break;
        case PROF_VP8_1:
            return "Profile 1";
            break;

        case PROF_VP9_0:
            return "Profile 0";
            break;
        case PROF_VP9_1:
            return "Profile 1";
            break;
        case PROF_VP9_2:
            return "Profile 2";
            break;
        case PROF_VP9_3:
            return "Profile 3";
            break;

        case PROF_AAC_LC:
            return "Low Complexity Profile";
            break;
        case PROF_AAC_Main:
            return "Main Profile";
            break;
        case PROF_AAC_SSR:
            return "Scalable Sample Rate Profile";
            break;
        case PROF_AAC_MainAudio:
            return "Main Audio Profile";
            break;
        case PROF_AAC_Scalable:
            return "Scalable Audio Profile";
            break;
        case PROF_AAC_HQ:
            return "High Quality Audio Profile";
            break;
        case PROF_AAC_LD:
            return "Low Delay Audio Profile";
            break;
        case PROF_AAC_LDv2:
            return "Low Delay AAC v2 Profile";
            break;
        case PROF_AAC_Mobile:
            return "Mobile Audio Internetworking Profile";
            break;
        case PROF_AAC_AAC:
            return "AAC Profile";
            break;
        case PROF_AAC_HE:
            return "High Efficiency AAC Profile";
            break;
        case PROF_AAC_HEv2:
            return "High Efficiency AAC Profile v2";
            break;

        case CODEC_PROF_UNKNOWN:
        default:
            return "UNKNOWN";
            break;
    }
}

/* ************************************************************************** */
