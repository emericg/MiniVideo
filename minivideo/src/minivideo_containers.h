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
 * \file      minivideo_containers.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2018
 */

#ifndef MINIVIDEO_CONTAINERS_H
#define MINIVIDEO_CONTAINERS_H

#include "minivideo_export.h"

#include <cstdint>
#include <string>

/* ************************************************************************** */

//! Container file formats
typedef enum Containers_e
{
    CONTAINER_UNKNOWN = 0,

    // General purpose containers //////////////////////////////////////////////

    CONTAINER_AVI       =  1,   //!< AVI "Audio Video Interleave" (.avi, ...)
    CONTAINER_ASF       =  2,   //!< ASF "Advanced Systems Format" (.asf, .wma, .wmv, ...)
    CONTAINER_MKV       =  3,   //!< Matroska (.mkv, .mka, .mks, .mk3d, .webm)
    CONTAINER_MP4       =  4,   //!< ISOM "ISO Base Media" format (.mov, .mp4, .3gp, .f4v, ...)
    CONTAINER_MPEG_PS   =  5,   //!< MPEG "Program Stream" (.mpg, .vob, ...)
    CONTAINER_MPEG_TS   =  6,   //!< MPEG "Transport Stream" (.ts, .mts, .m2ts, ...)
    CONTAINER_MPEG_MT   =  7,   //!< MPEG "Media Transport" (.mt, .mmt)
    CONTAINER_MXF       =  8,   //!< MXF "Material eXchange Format" (.mxf)
    CONTAINER_FLV       =  9,   //!< FLV "Flash Video" file format (.flv)
    CONTAINER_SWF       = 10,   //!< SWF "Small Web Format" (.swf)
    CONTAINER_OGG       = 11,   //!< OGG (.ogg, .ogv, .oga, .opus, ...)
    CONTAINER_RM        = 12,   //!< RealMedia (.rm, .rmvb)
    CONTAINER_R3D       = 13,   //!< REDCode RAW (.r3d)

    // Audio containers ////////////////////////////////////////////////////////

    CONTAINER_FLAC      = 64,   //!< FLAC "Free Lossless Audio Codec" (.flac)
    CONTAINER_WAVE      = 65,   //!< WAVE "Waveform Audio File Format" (.wav)
    CONTAINER_AIFF      = 66,   //!< AIFF "Audio Interchange File Format" (.aiff, .aif, .aifc)
    CONTAINER_CAF       = 67,   //!< CAF "Core Audio Format" (.caf)
    CONTAINER_AU        = 68,   //!< AU "Au file format" (.au, .snd)

    // ES formats (not containers!) ////////////////////////////////////////////

    CONTAINER_ES        = 128,  //!< Undefined "Elementary Stream"
    CONTAINER_ES_AAC    = 129,  //!< AAC "Elementary Stream"
    CONTAINER_ES_AC3    = 130,  //!< AC3 "Elementary Stream"
    CONTAINER_ES_MP3    = 131,  //!< MP3 "Elementary Stream"

    // Image containers ////////////////////////////////////////////////////////

    CONTAINER_JPEG      = 256,
    CONTAINER_JPEG_XR   = 257,
    CONTAINER_JPEG_2K   = 258,
    CONTAINER_GIF       = 259,
    CONTAINER_BMP       = 260,
    CONTAINER_TGA       = 261,
    CONTAINER_TIFF      = 262,
    CONTAINER_ICNS      = 263,
    CONTAINER_ICO       = 264,
    CONTAINER_PNG       = 265,
    CONTAINER_DNG       = 266,
    CONTAINER_DPX       = 267,
    CONTAINER_CIFF      = 268,
    CONTAINER_CR2       = 269,
    CONTAINER_WEBP      = 270,
    CONTAINER_BPG       = 271,
    CONTAINER_FLIF      = 272

} Containers_e;

typedef enum ContainerProfiles_e
{
    PROF_UNKNOWN        = 0,

    PROF_AVI            = 1,
    PROF_AVI_OpenDML    = 2,
    PROF_AVI_DMF        = 3,

    PROF_MPEG_PS_1      = 8,
    PROF_MPEG_PS_2      = 9,

    PROF_ISOBMF_MOV     = 16,
    PROF_ISOBMF_MP4     = 17,
    PROF_ISOBMF_3GP     = 18,
    PROF_ISOBMF_MJP     = 19,
    PROF_ISOBMF_HEIF    = 20,
    PROF_ISOBMF_AVIF    = 21,

    PROF_MKV_MATROSKA   = 32,
    PROF_MKV_WEBM       = 33,

    PROF_WAVE_AMB       = 128,
    PROF_WAVE_RF64      = 129,
    PROF_WAVE_BWFv1     = 130,
    PROF_WAVE_BWFv2     = 131,
    PROF_WAVE_BWF64     = 132

} ContainerProfiles_e;

/* ************************************************************************** */

minivideo_EXPORT const char *getContainerString(const Containers_e container, const bool long_description = false);
minivideo_EXPORT const char *getContainerProfileString(const ContainerProfiles_e profile, const bool long_description = false);

Containers_e getContainerUsingStartcodes(uint8_t buffer[16]);
Containers_e getContainerUsingExtension(const std::string &ext);

/* ************************************************************************** */
#endif // MINIVIDEO_CONTAINERS_H
