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
 * \file      minivideo_twocc.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef MINIVIDEO_TWOCC_H
#define MINIVIDEO_TWOCC_H

// minivideo headers
#include "minivideo_codecs.h"
#include "minivideo_export.h"

// C standard libraries
#include <cstdint>

/* ************************************************************************** */

/*!
 * \brief Find a codec from a TwoCC value.
 * \param tcc[in]: A TwoCC value.
 * \return A Codecs_e value.
 */
minivideo_EXPORT Codecs_e getCodecFromTwoCC(const uint16_t tcc);

/*!
 * \brief Get a printable TwoCC description (from a TwoCC value).
 * \param tcc[in]: A TwoCC value.
 * \return A NULL terminated C string.
 */
minivideo_EXPORT const char *getTccString(const uint16_t tcc);

/* ************************************************************************** */

/*!
 * Good ressources about TwoCCs:
 * - http://wiki.multimedia.cx/index.php?title=Twocc
 * - http://www.videolan.org/developers/vlc/doc/doxygen/html/vlc__codecs_8h_source.html (search for WAVE_FORMAT_)
 */
typedef enum twocc_list_e
{
    // Audio codecs
    WAVE_FORMAT_MS_WAVE     = 0x0000,
    WAVE_FORMAT_MS_PCM      = 0x0001,
    WAVE_FORMAT_MS_ADPCM    = 0x0002,
    WAVE_FORMAT_MS_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_VSELP       = 0x0004,
    WAVE_FORMAT_IBM_CVSD    = 0x0005,
    WAVE_FORMAT_MS_ALAW     = 0x0006, //!< Microsoft CCITT G.711 (A-Law and u-Law)
    WAVE_FORMAT_MS_MULAW    = 0x0007, //!< Microsoft CCITT G.711 (A-Law and u-Law)
    WAVE_FORMAT_MS_DTS      = 0x0008,
    WAVE_FORMAT_MS_DRM      = 0x0009,
    WAVE_FORMAT_WMAS        = 0x000A, //!< WMA 9 Speech
    WAVE_FORMAT_MS_WMRTV    = 0x000B, //!< Windows Media RT Voice

    WAVE_FORMAT_AC2         = 0x0030, //!< AC-2
    WAVE_FORMAT_MP1         = 0x0050,
    WAVE_FORMAT_MP3         = 0x0055,

    WAVE_FORMAT_AC3         = 0x2000, //!< AC-3 or A52
    WAVE_FORMAT_DTS         = 0x2001,
    WAVE_FORMAT_RA_14       = 0x2002, //!< RealAudio 1 / 14.4
    WAVE_FORMAT_RA_28       = 0x2003, //!< RealAudio 2 / 28.8
    WAVE_FORMAT_RA_cook     = 0x2004, //!< RealAudio G2 / Cook
    WAVE_FORMAT_RA3         = 0x2005, //!< RealAudio 3
    WAVE_FORMAT_RA9         = 0x2006, //!< RealAudio 9 AAC (RAAC)
    WAVE_FORMAT_RA10p       = 0x2007, //!< RealAudio 10 AAC+ (RACP)

    WAVE_FORMAT_AAC         = 0x00FF, //!< AAC (also: 0x0180, 0x4143, 0x706D, 0xA106)
    WAVE_FORMAT_WMA1        = 0x0160, //!< WMA version 1
    WAVE_FORMAT_WMA2        = 0x0161, //!< WMA (v2) 7, 8, 9 Series
    WAVE_FORMAT_WMAP        = 0x0162, //!< WMA 9 Professional
    WAVE_FORMAT_WMAL        = 0x0163, //!< WMA 9 Lossless

    WAVE_FORMAT_IAC2        = 0x0402, //!< Indeo Audio Codec

    WAVE_FORMAT_VORBIS1     = 0x674F, //!< Ogg Vorbis mode 1
    WAVE_FORMAT_VORBIS2     = 0x6750, //!< Ogg Vorbis mode 2
    WAVE_FORMAT_VORBIS3     = 0x6751, //!< Ogg Vorbis mode 3
    WAVE_FORMAT_VORBIS1p    = 0x676F, //!< Ogg Vorbis mode 1+
    WAVE_FORMAT_VORBIS2p    = 0x6770, //!< Ogg Vorbis mode 2+
    WAVE_FORMAT_VORBIS3p    = 0x6772, //!< Ogg Vorbis mode 3+

    WAVE_FORMAT_EXTENSIBLE  = 0xFFFE

} twocc_list_e;

/* ************************************************************************** */
#endif // MINIVIDEO_TWOCC_H
