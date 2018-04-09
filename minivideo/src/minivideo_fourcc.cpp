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
 * \file      minivideo_fourcc.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "minivideo_fourcc.h"
#include "minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* ************************************************************************** */

char *getFccString_le(const uint32_t fcc_in, char fcc_out[5])
{
    if (fcc_out)
    {
        fcc_out[0] = static_cast<char>((fcc_in >> 24) & 0xFF);
        fcc_out[1] = static_cast<char>((fcc_in >> 16) & 0xFF);
        fcc_out[2] = static_cast<char>((fcc_in >>  8) & 0xFF);
        fcc_out[3] = static_cast<char>((fcc_in >>  0) & 0xFF);
        fcc_out[4] = '\0';

        return fcc_out;
    }

    return NULL;
}

char *getFccString_be(const uint32_t fcc_in, char fcc_out[5])
{
    if (fcc_out)
    {
        fcc_out[0] = static_cast<char>((fcc_in >>  0) & 0xFF);
        fcc_out[1] = static_cast<char>((fcc_in >>  8) & 0xFF);
        fcc_out[2] = static_cast<char>((fcc_in >> 16) & 0xFF);
        fcc_out[3] = static_cast<char>((fcc_in >> 24) & 0xFF);
        fcc_out[4] = '\0';

        return fcc_out;
    }

    return NULL;
}

/* ************************************************************************** */

Codecs_e getCodecFromFourCC(const uint32_t fcc)
{
    Codecs_e codec = CODEC_UNKNOWN;

    switch (fcc)
    {
    case fcc_mp1v:
    case fcc_MPG1:
    case fcc_mpg1:
    case fcc_PIM1:
        codec = CODEC_MPEG1;
        break;
    case fcc_mp2v:
    case fcc_MPEG:
    case fcc_mpeg:
    case fcc_MPG2:
    case fcc_mpg2:
    case fcc_PIM2:
    case fcc_DVR:
        codec = CODEC_MPEG2;
        break;

    case fcc_h261:
    case fcc_H261:
        codec = CODEC_H261;
        break;

    case fcc_xvid:
    case fcc_XVID:
    case fcc_FMP4:
    case fcc_mpg3:
    case fcc_div3:
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
    case fcc_MP4V:
    case fcc_mp4v:
        codec = CODEC_MPEG4_ASP;
        break;

    case fcc_AVC1:
    case fcc_avc1:
    case fcc_avc3:
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

    case fcc_mpg4:
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
    case fcc_h263:
    case fcc_H263:
    case fcc_L263:
    case fcc_M263:
    case fcc_s263:
    case fcc_S263:
    case fcc_T263:
    case fcc_U263:
    case fcc_x263:
    case fcc_X263:
        codec = CODEC_H263;
        break;

    case fcc_VP30:
    case fcc_VP31:
    case fcc_VP32:
        codec = CODEC_VP3;
        break;
    case fcc_VP40:
        codec = CODEC_VP4;
        break;
    case fcc_VP50:
        codec = CODEC_VP5;
        break;
    case fcc_VP60:
    case fcc_VP61:
    case fcc_VP62:
    case fcc_VP6F:
        codec = CODEC_VP6;
        break;
    case fcc_VP70:
    case fcc_VP71:
    case fcc_VP72:
        codec = CODEC_VP7;
        break;
    case fcc_VP80:
        codec = CODEC_VP8;
        break;
    case fcc_VP90:
        codec = CODEC_VP9;
        break;

    case fcc_av01:
        codec = CODEC_AV1;
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

    case fcc_apco:
        codec = CODEC_PRORES_422_PROXY;
        break;
    case fcc_apcs:
        codec = CODEC_PRORES_422_LT;
        break;
    case fcc_apcn:
        codec = CODEC_PRORES_422;
        break;
    case fcc_apch:
        codec = CODEC_PRORES_422_HQ;
        break;
    case fcc_ap4h:
        codec = CODEC_PRORES_4444;
        break;
    case fcc_ap4x:
        codec = CODEC_PRORES_4444_XQ;
        break;

    case fcc_REDC:
        codec = CODEC_REDCode;
        break;

    case fcc_RV10:
    case fcc_RV13:
        codec = CODEC_RV10;
        break;
    case fcc_rv20:
        codec = CODEC_RV20;
        break;
    case fcc_rv30:
        codec = CODEC_RV30;
        break;
    case fcc_rv40:
        codec = CODEC_RV40;
        break;

    case fcc_cvid:
    case fcc_CVID:
        codec = CODEC_CINEPAK;
        break;

    case fcc_SNOW:
    case fcc_snow:
        codec = CODEC_SNOW;
        break;

    case fcc_svq1:
    case fcc_SVQ1:
    case fcc_svqi:
        codec = CODEC_SVQ1;
        break;
    case fcc_SVQ3:
        codec = CODEC_SVQ3;
        break;

    case fcc_RT21:
        codec = CODEC_INDEO2;
        break;
    case fcc_IV31:
    case fcc_IV32:
    case fcc_iV31:
    case fcc_iV32:
    case fcc_iv31:
    case fcc_iv32:
        codec = CODEC_INDEO3;
        break;
    case fcc_iv41:
    case fcc_iV41:
    case fcc_IV41:
    case fcc_iV42:
    case fcc_IV42:
        codec = CODEC_INDEO4;
        break;
    case fcc_IV50:
    case fcc_iV50:
        codec = CODEC_INDEO5;
        break;

    case fcc_CHQX:
        codec = CODEC_CanopusHQ;
        break;

    case fcc_icod:
        codec = CODEC_icod;
        break;
    case fcc_rpza:
    case fcc_azpr:
        codec = CODEC_rpza;
        break;
    case fcc_rle:
        codec = CODEC_QtAnimation;
        break;
    case fcc_smc:
        codec = CODEC_QtGraphics;
        break;

    case fcc_BIKf:
    case fcc_BIKg:
    case fcc_BIKh:
    case fcc_BIKi:
        codec = CODEC_BINK;
        break;

    case fcc_FFV1:
        codec = CODEC_FFV1;
        break;
    case fcc_CLLC:
        codec = CODEC_CanopusLL;
        break;

////////////////////////////////////////////////////////////////////////////////

    case fcc_PNG1:
        codec = CODEC_CorePNG;
        break;
    case fcc_JPEG:
    case fcc_jpeg:
        codec = CODEC_JPEG;
        break;
    case fcc_MJPG:
    case fcc_mjpg:
    case fcc_dmb1:
        codec = CODEC_MJPEG;
        break;
    case fcc_MJ2:
    case fcc_MJP2:
        codec = CODEC_MJPEG2K;
        break;

////////////////////////////////////////////////////////////////////////////////

    case fcc_AAC:
    case fcc_AACP:
    case fcc_MP4A:
    case fcc_mp4a:
    case fcc_raac:
    case fcc_racp:
        codec = CODEC_AAC;
        break;

    case fcc_AC3:
    case fcc_ac3:
        codec = CODEC_AC3;
        break;

    case fcc_AC4:
    case fcc_ac4:
        codec = CODEC_AC4;
        break;

    case fcc_agsm:
        codec = CODEC_GSM;
        break;

    case fcc_samr:
        codec = CODEC_AMR;
        break;

    case fcc_alac:
        codec = CODEC_ALAC;
        break;

    case fcc_lpcJ:
        codec = CODEC_RA_14;
        break;
    case fcc_28_8:
        codec = CODEC_RA_28;
        break;
    case fcc_cook:
        codec = CODEC_RA_cook;
        break;
    case fcc_ralf:
        codec = CODEC_RA_ralf;
        break;

    case fcc_atrc:
        codec = CODEC_ATRAC;
        break;

    case fcc_raw:
    case fcc_araw:
    case fcc_lpcm:
    case fcc_sowt:
    case fcc_tows:
    case fcc_in24:
    case fcc_in32:
    case fcc_s8:
    case fcc_u8:
    case fcc_s16l:
    case fcc_s16b:
    case fcc_s24l:
    case fcc_s24b:
    case fcc_s32l:
    case fcc_s32b:
    case fcc_u16l:
    case fcc_u16b:
    case fcc_u24l:
    case fcc_u24b:
    case fcc_u32l:
    case fcc_u32b:
    case fcc_f32l:
    case fcc_f64l:
    case fcc_fl32:
    case fcc_fl64:
        codec = CODEC_LPCM;
        break;

    case fcc_alaw:
    case fcc_ulaw:
        codec = CODEC_LogPCM;
        break;

    case fcc_ima4:
        codec = CODEC_ADPCM;
        break;

////////////////////////////////////////////////////////////////////////////////

    default:
        codec = CODEC_UNKNOWN;
        break;
    }

    return codec;
}

/* ************************************************************************** */
