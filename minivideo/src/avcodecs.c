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

const char *getContainerString(ContainerFormat_e container, int long_description)
{
    if (long_description == 1)
    {
        switch (container)
        {
        case CONTAINER_AVI:
            return "AVI 'Audio Video Interleave' (.avi, ...)";
            break;
        case CONTAINER_ASF:
            return "ASF 'Advanced Systems Format' (.asf, .wma, .wmv, ...)";
            break;
        case CONTAINER_MKV:
            return "Matroska (.mkv, .mka, .webm)";
            break;
        case CONTAINER_MP4:
            return "ISOM 'ISO Base Media' format (.mov, .mp4, .3gp, .f4v, ...)";
            break;
        case CONTAINER_MXF:
            return "MXF 'Material eXchange Format' (.mxf)";
            break;
        case CONTAINER_FLV:
            return "SWF 'Small Web Format' (.flv)";
            break;
        case CONTAINER_OGG:
            return "OGG (.ogg, .ogv, ...)";
            break;
        case CONTAINER_RM:
            return "RealMedia (.rm, .rmvb)";
            break;
        case CONTAINER_MPEG_PS:
            return "MPEG 'Program Stream' (.mpg, .vob, ...)";
            break;
        case CONTAINER_MPEG_TS:
            return "MPEG 'Transport Stream' (.ts, .mts, .m2ts, ...)";
            break;

        case CONTAINER_FLAC:
            return "FLAC 'Free Lossless Audio Codec' (.flac)";
            break;
        case CONTAINER_WAVE:
            return "WAVE 'Waveform Audio File Format' (.wav)";
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
            case CONTAINER_MP4:
                return "MP4";
                break;
            case CONTAINER_MKV:
                return "MKV";
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
            case CONTAINER_MPEG_PS:
                return "MPEG-PS";
                break;
            case CONTAINER_MPEG_TS:
                return "MPEG-TS";
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

const char *getCodecString(StreamType_e type, AVCodec_e codec)
{
    if (type == stream_AUDIO)
    {
        switch (codec)
        {
            case CODEC_LPCM:
                return "LPCM";
                break;
            case CODEC_DPCM:
                return "DPCM";
                break;
            case CODEC_ADPCM:
                return "ADPCM";
                break;

            case CODEC_APE:
                return "Monkey's Audio";
                break;
            case CODEC_FLAC:
                return "Free Lossless Audio Codec (FLAC)";
                break;

            case CODEC_MPEG_L1:
                return "MPEG-1/2 Layer 1";
                break;
            case CODEC_MPEG_L2:
                return "MPEG-1/2 Layer 2";
                break;
            case CODEC_MPEG_L3:
                return "MPEG-1/2 Layer 3 (MP3)";
                break;
            case CODEC_AAC:
                return "AAC";
                break;
            case CODEC_AAC_HE:
                return "HE-AAC";
                break;
            case CODEC_AC3:
                return "AC3 (DolbyDigital)";
                break;
            case CODEC_EAC3:
                return "EAC3 (DolbyDigital+)";
                break;
            case CODEC_DTS:
                return "DTS";
                break;
            case CODEC_DTS_HD:
                return "DTS-HD";
                break;
            case CODEC_WMA:
                return "WMA";
                break;
            case CODEC_MPC:
                return "Musepack (MPC)";
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

            default:
            case CODEC_UNKNOWN:
                return "UNKNOWN";
                break;
        }
    }
    else if (type == stream_VIDEO)
    {
        switch (codec)
        {
            case CODEC_MPEG12:
                return "MPEG-1/2 videos";
                break;
            case CODEC_MPEG4:
                return "MPEG-4 ASP (XVID)";
                break;
            case CODEC_MSMPEG4:
                return "MS MPEG-4";
                break;
            case CODEC_H263:
                return "H.263";
                break;
            case CODEC_H264:
                return "H.264 (MPEG-4 AVC)";
                break;
            case CODEC_H265:
                return "H.265 (MPEG-H HEVC)";
                break;

            case CODEC_WMV1:
                return "WMV1";
                break;
            case CODEC_WMV2:
                return "WMV2";
                break;
            case CODEC_WMV3:
                return "WMV3";
                break;
            case CODEC_WMVA:
                return "WMVA";
                break;
            case CODEC_WVC1:
                return "WVC1";
                break;

            case CODEC_VP4:
                return "VP4 (Ogg Theora)";
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

            case CODEC_VC1:
                return "VC-1 (Windows Media Video)";
                break;
            case CODEC_VC2:
                return "VC-2 (Dirac)";
                break;
            case CODEC_VC3:
                return "VC-3 (DNxHD)";
                break;
            case CODEC_VC5:
                return "VC-5 (CineForm)";
                break;

            case CODEC_DAALA:
                return "Daala";
                break;

        default:
        case CODEC_UNKNOWN:
            return "UNKNOWN";
            break;

        }
    }
    else if (type == stream_TEXT)
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

            default:
            case CODEC_UNKNOWN:
                return "UNKNOWN";
                break;
        }
    }

    return "UNKNOWN";
}

/* ************************************************************************** */

const char *getPictureString(PictureFormat_e picture)
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
