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
 * \file      minivideo_twocc.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "minivideo_twocc.h"
#include "minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>

/* ************************************************************************** */

Codecs_e getCodecFromTwoCC(const uint16_t tcc)
{
    Codecs_e codec = CODEC_UNKNOWN;

    switch (tcc)
    {
    case WAVE_FORMAT_MS_PCM:
    case WAVE_FORMAT_MS_IEEE_FLOAT:
        codec = CODEC_LPCM;
        break;
    case WAVE_FORMAT_MS_ADPCM:
        codec = CODEC_ADPCM;
        break;
    case WAVE_FORMAT_MS_ALAW:
    case WAVE_FORMAT_MS_MULAW:
        codec = CODEC_LogPCM;
        break;
    case WAVE_FORMAT_MS_DTS:
        codec = CODEC_DTS;
        break;
    case WAVE_FORMAT_MP1:
        codec = CODEC_MPEG_L2;
        break;
    case WAVE_FORMAT_MP3:
        codec = CODEC_MPEG_L3;
        break;

    case WAVE_FORMAT_AC3:
        codec = CODEC_AC3;
        break;
    case WAVE_FORMAT_DTS:
        codec = CODEC_DTS;
        break;
    case WAVE_FORMAT_RA_14:
        codec = CODEC_RA_14;
        break;
    case WAVE_FORMAT_RA_28:
        codec = CODEC_RA_28;
        break;
    case WAVE_FORMAT_RA_cook:
        codec = CODEC_RA_cook;
     break;


    case WAVE_FORMAT_RA9:
    case WAVE_FORMAT_RA10p:
    case WAVE_FORMAT_AAC:
        codec = CODEC_AAC;
        break;

    case WAVE_FORMAT_WMAS:
    case WAVE_FORMAT_WMA1:
    case WAVE_FORMAT_WMA2:
    case WAVE_FORMAT_WMAP:
    case WAVE_FORMAT_WMAL:
         codec = CODEC_WMA;
         break;

    case WAVE_FORMAT_IAC2:
         codec = CODEC_IAC2;
         break;

    case WAVE_FORMAT_VORBIS1:
    case WAVE_FORMAT_VORBIS2:
    case WAVE_FORMAT_VORBIS3:
    case WAVE_FORMAT_VORBIS1p:
    case WAVE_FORMAT_VORBIS2p:
    case WAVE_FORMAT_VORBIS3p:
        codec = CODEC_VORBIS;
        break;

    default:
        codec = CODEC_UNKNOWN;
        break;
    }

    return codec;
}

/* ************************************************************************** */

const char *getTccString(const uint16_t tcc)
{
    switch (tcc)
    {
    case WAVE_FORMAT_MS_WAVE:
        return "WAVE_FORMAT_MS_WAVE";
        break;
    case WAVE_FORMAT_MS_PCM:
        return "WAVE_FORMAT_MS_PCM";
        break;
    case WAVE_FORMAT_MS_ADPCM:
        return "WAVE_FORMAT_MS_ADPCM";
        break;
    case WAVE_FORMAT_MS_IEEE_FLOAT:
        return "WAVE_FORMAT_MS_IEEE_FLOAT";
        break;
    case WAVE_FORMAT_VSELP:
        return "WAVE_FORMAT_VSELP";
        break;
    case WAVE_FORMAT_IBM_CVSD:
        return "WAVE_FORMAT_IBM_CVSD";
        break;
    case WAVE_FORMAT_MS_ALAW:
        return "WAVE_FORMAT_MS_ALAW";
        break;
    case WAVE_FORMAT_MS_MULAW:
        return "WAVE_FORMAT_MS_MULAW";
        break;
    case WAVE_FORMAT_MS_DTS:
        return "WAVE_FORMAT_MS_DTS";
        break;
    case WAVE_FORMAT_MS_DRM:
        return "WAVE_FORMAT_MS_DRM";
        break;
    case WAVE_FORMAT_WMAS:
        return "WAVE_FORMAT_WMAS";
        break;
    case WAVE_FORMAT_MS_WMRTV:
        return "WAVE_FORMAT_MS_WMRTV";
        break;

    case WAVE_FORMAT_AC2:
        return "WAVE_FORMAT_AC2";
        break;
    case WAVE_FORMAT_MP1:
        return "WAVE_FORMAT_MP1";
        break;
    case WAVE_FORMAT_MP3:
        return "WAVE_FORMAT_MP3";
        break;
    case WAVE_FORMAT_AC3:
        return "WAVE_FORMAT_AC3";
        break;
    case WAVE_FORMAT_DTS:
        return "WAVE_FORMAT_DTS";
        break;
    case WAVE_FORMAT_AAC:
        return "WAVE_FORMAT_AAC";
        break;

    case WAVE_FORMAT_WMA1:
        return "WAVE_FORMAT_WMA1";
        break;
    case WAVE_FORMAT_WMA2:
        return "WAVE_FORMAT_WMA2";
        break;
    case WAVE_FORMAT_WMAP:
        return "WAVE_FORMAT_WMAP";
        break;
    case WAVE_FORMAT_WMAL:
        return "WAVE_FORMAT_WMAL";
        break;

    case WAVE_FORMAT_IAC2:
        return "WAVE_FORMAT_IAC2";
        break;

    case WAVE_FORMAT_VORBIS1:
        return "WAVE_FORMAT_VORBIS1";
        break;
    case WAVE_FORMAT_VORBIS2:
        return "WAVE_FORMAT_VORBIS2";
        break;
    case WAVE_FORMAT_VORBIS3:
        return "WAVE_FORMAT_VORBIS3";
        break;
    case WAVE_FORMAT_VORBIS1p:
        return "WAVE_FORMAT_VORBIS1p";
        break;
    case WAVE_FORMAT_VORBIS2p:
        return "WAVE_FORMAT_VORBIS2p";
        break;
    case WAVE_FORMAT_VORBIS3p:
        return "WAVE_FORMAT_VORBIS3p";
        break;

    case WAVE_FORMAT_EXTENSIBLE:
        return "WAVE_FORMAT_EXTENSIBLE";
        break;

    default:
        return "UNKNOWN";
        break;
    }
}

/* ************************************************************************** */
