/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
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
 * \file      avcodecs.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#include "avcodecs.h"

/* ************************************************************************** */

const char *getContainerString(ContainerFormat_e container, bool long_description)
{
    if (long_description)
    {
        switch (container)
        {
        case CONTAINER_AVI:
            return "AVI (Audio Video Interleave) [.avi, ...]";
            break;
        case CONTAINER_ASF:
            return "ASF (Advanced Systems Format) [.asf, .wma, .wmv, ...]";
            break;
        case CONTAINER_MKV:
            return "MKV (Matroska) [.mkv, .mka, .webm]";
            break;
        case CONTAINER_MP4:
            return "MP4 (ISO Base Media format) [.mov, .mp4, .3gp, .f4v, ...]";
            break;
        case CONTAINER_MPEG_PS:
            return "MPEG 'Program Stream' [.mpg, .vob, ...]";
            break;
        case CONTAINER_MPEG_TS:
            return "MPEG 'Transport Stream' [.ts, .mts, .m2ts, ...]";
            break;
        case CONTAINER_MPEG_MT:
            return "MPEG 'Media Transport'";
            break;
        case CONTAINER_MXF:
            return "MXF Material eXchange Format [.mxf]";
            break;
        case CONTAINER_FLV:
            return "SWF Small Web Format [.flv]";
            break;
        case CONTAINER_OGG:
            return "OGG [.ogg, .ogv, ...]";
            break;
        case CONTAINER_RM:
            return "RealMedia [.rm, .rmvb]";
            break;
        case CONTAINER_R3D:
            return "Redcode RAW [.r3d]";
            break;

        case CONTAINER_FLAC:
            return "FLAC (Free Lossless Audio Codec) [.flac]";
            break;
        case CONTAINER_WAVE:
            return "WAVE (Waveform Audio File Format) [.wav]";
            break;

        case CONTAINER_ES:
            return "Undefined 'Elementary Stream'";
            break;
        case CONTAINER_ES_AAC:
            return "AAC 'Elementary Stream'";
            break;
        case CONTAINER_ES_AC3:
            return "AC3 'Elementary Stream'";
            break;
        case CONTAINER_ES_MP3:
            return "MP3 'Elementary Stream'";
            break;

        default:
        case CONTAINER_UNKNOWN:
            return "UNKNOWN";
            break;
        }
    }
    else // short description
    {
        switch (container)
        {
            case CONTAINER_AVI:
                return "AVI";
                break;
            case CONTAINER_ASF:
                return "ASF";
                break;
            case CONTAINER_MKV:
                return "MKV";
                break;
            case CONTAINER_MP4:
                return "MP4";
                break;
            case CONTAINER_MPEG_PS:
                return "MPEG-PS";
                break;
            case CONTAINER_MPEG_TS:
                return "MPEG-TS";
                break;
            case CONTAINER_MPEG_MT:
                return "MPEG-MT";
                break;
            case CONTAINER_MXF:
                return "MXF";
                break;
            case CONTAINER_FLV:
                return "FLV";
                break;
            case CONTAINER_OGG:
                return "OGG";
                break;
            case CONTAINER_RM:
                return "RM";
                break;
            case CONTAINER_R3D:
                return "R3D";
                break;

            case CONTAINER_FLAC:
                return "FLAC";
                break;
            case CONTAINER_WAVE:
                return "WAVE";
                break;

            case CONTAINER_ES:
                return "ES";
                break;
            case CONTAINER_ES_AAC:
                return "AAC ES";
                break;
            case CONTAINER_ES_AC3:
                return "AC3 ES";
                break;
            case CONTAINER_ES_MP3:
                return "MP3 ES";
                break;

            default:
            case CONTAINER_UNKNOWN:
                return "UNKNOWN";
                break;
        }
    }
}

/* ************************************************************************** */

const char *getCodecString(StreamType_e type, AVCodec_e codec, bool long_description)
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
            case CODEC_AAC_HE:
                if (long_description)
                    return "HE-AAC ('High Efficiency' Advance Audio Coding)";
                else
                    return "HE-AAC";
                break;
            case CODEC_AAC_LD:
                if (long_description)
                    return "LD-AAC ('Low Delay' Advance Audio Coding)";
                else
                    return "LD-AAC";
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

            case CODEC_AC3:
                if (long_description)
                    return "AC-3 (Dolby Digital)";
                else
                    return "AC-3";
                break;
            case CODEC_EAC3:
                if (long_description)
                    return "EAC-3 (Dolby Digital+)";
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

            case CODEC_RA_28:
                return "RealAudio 28.8";
                break;
            case CODEC_RA_cook:
                return "RealAudio cook";
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
            case CODEC_VP10:
                return "VP10";
                break;

            case CODEC_DAALA:
                return "Daala";
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
            case CODEC_icod:
                return "Apple Intermediate Codec";
                break;
            case CODEC_rpza:
                return "Apple Video";
                break;

            case CODEC_FFV1:
                return "FFV1";
                break;
            case CODEC_CorePNG:
                return "CorePNG";
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
        }
    }

    return "Unknown";
}

/* ************************************************************************** */

const char *getPictureString(PictureFormat_e picture, bool long_description)
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
