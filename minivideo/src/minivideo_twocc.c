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
 * \file      minivideo_twocc.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "minivideo_twocc.h"
#include "minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

Codecs_e getCodecFromTwoCC(const uint16_t tcc)
{
    Codecs_e codec = CODEC_UNKNOWN;

    switch (tcc)
    {
    case WAVE_FORMAT_MS_PCM:
    case WAVE_FORMAT_IEEE_FLOAT:
        codec = CODEC_LPCM;
        break;
    case WAVE_FORMAT_MS_ADPCM:
        codec = CODEC_ADPCM;
        break;
    case WAVE_FORMAT_ALAW:
    case WAVE_FORMAT_MULAW:
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
