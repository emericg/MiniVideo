/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      fourcc.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "fourcc.h"
#include "minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

char *getFccString_le(const uint32_t fcc_in, char *fcc_out)
{
    if (fcc_out)
    {
        fcc_out[0] = (fcc_in >> 24) & 0xFF;
        fcc_out[1] = (fcc_in >> 16) & 0xFF;
        fcc_out[2] = (fcc_in >>  8) & 0xFF;
        fcc_out[3] = (fcc_in >>  0) & 0xFF;
        fcc_out[4] = '\0';

        return fcc_out;
    }

    return "0000";
}

char *getFccString_be(const uint32_t fcc_in, char *fcc_out)
{
    if (fcc_out)
    {
        fcc_out[0] = (fcc_in >>  0) & 0xFF;
        fcc_out[1] = (fcc_in >>  8) & 0xFF;
        fcc_out[2] = (fcc_in >> 16) & 0xFF;
        fcc_out[3] = (fcc_in >> 24) & 0xFF;
        fcc_out[4] = '\0';

        return fcc_out;
    }

    return "0000";
}

/* ************************************************************************** */

AVCodec_e getCodecFromFourCC(const uint32_t fcc)
{
    AVCodec_e codec = CODEC_UNKNOWN;

    switch (fcc)
    {
    case fcc_MPG1:
    case fcc_mpg1:
        codec = CODEC_MPEG1;
        break;
    case fcc_MPEG:
    case fcc_mpeg:
    case fcc_MPG2:
    case fcc_mpg2:
        codec = CODEC_MPEG2;
        break;

    case fcc_xvid:
    case fcc_XVID:
    case fcc_FMP4:
    case fcc_MP4V:
    case fcc_divx:
    case fcc_DIVX:
    case fcc_DX50:
    case fcc_DX60:
    case fcc_DIV1:
    case fcc_DIV2:
    case fcc_DIV4:
    case fcc_DIV5:
        codec = CODEC_MPEG4_ASP;
        break;

    case fcc_AVC1:
    case fcc_avc1:
    case fcc_AVCC:
    case fcc_avcc:
    case fcc_H264:
    case fcc_h264:
    case fcc_X264:
    case fcc_x264:
        codec = CODEC_H264;
        break;

    case fcc_HVC1:
    case fcc_hvc1:
    case fcc_HEVC:
    case fcc_hevc:
    case fcc_HEV1:
    case fcc_hev1:
    case fcc_H265:
    case fcc_h265:
    case fcc_X265:
    case fcc_x265:
        codec = CODEC_H265;
        break;

    case fcc_MPG4:
    case fcc_MP41:
    case fcc_MP42:
    case fcc_MP43:
    case fcc_DIV3:
    case fcc_AP41:
    case fcc_COL1:
        codec = CODEC_MSMPEG4;
        break;
    case fcc_MSS1:
    case fcc_MSS2:
    case fcc_MSA1:
        codec = CODEC_WMSCR;
        break;
    case fcc_WMV1:
    case fcc_WMV7:
        codec = CODEC_WMV7;
        break;
    case fcc_WMV2:
    case fcc_WMV8:
        codec = CODEC_WMV8;
        break;
    case fcc_WMVA:
        codec = CODEC_WMV9;
        break;

    case fcc_D263:
    case fcc_H263:
    case fcc_L263:
    case fcc_M263:
    case fcc_S263:
    case fcc_T263:
    case fcc_U263:
    case fcc_X263:
        codec = CODEC_H263;
        break;

    case fcc_WMV3:
    case fcc_WVC1:
        codec = CODEC_VC1;
        break;

    case fcc_BBCD:
        codec = CODEC_VC2;
        break;

    case fcc_AVdn:
        codec = CODEC_VC3;
        break;

    case fcc_CFHD:
    case fcc_cfhd:
        codec = CODEC_VC5;
        break;

    case fcc_ap4h:
        codec = CODEC_PRORES_4444;
        break;
    case fcc_ap4x:
        codec = CODEC_PRORES_4444_XQ;
        break;
    case fcc_apch:
        codec = CODEC_PRORES_422_HQ;
        break;
    case fcc_apcn:
        codec = CODEC_PRORES_442;
        break;
    case fcc_apco:
        codec = CODEC_PRORES_442_PROXY;
        break;
    case fcc_apcs:
        codec = CODEC_PRORES_442_LT;
        break;

    case fcc_sowt:
    case fcc_tows:
    case fcc_raw:
    case fcc_in24:
    case fcc_in32:
    case fcc_fl32:
    case fcc_fl64:
        codec = CODEC_LPCM;
        break;

    case fcc_alaw:
    case fcc_ulaw:
        codec = CODEC_LogPCM;
        break;

    default:
        codec = CODEC_UNKNOWN;
        break;
    }

    return codec;
}

/* ************************************************************************** */
