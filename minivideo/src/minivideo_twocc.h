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
 * \date      2016
 */

#ifndef MINIVIDEO_TWOCC_H
#define MINIVIDEO_TWOCC_H
/* ************************************************************************** */

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
    WAVE_FORMAT_MS_WAVE                 = 0x0000, //!< Unknown codec
    WAVE_FORMAT_MS_PCM                  = 0x0001, //!< Microsoft PCM
    WAVE_FORMAT_MS_ADPCM                = 0x0002,
    WAVE_FORMAT_MS_IEEE_FLOAT           = 0x0003,
    WAVE_FORMAT_VSELP                   = 0x0004,
    WAVE_FORMAT_IBM_CVSD                = 0x0005,
    WAVE_FORMAT_MS_ALAW                 = 0x0006, //!< Microsoft CCITT G.711 (A-Law and u-Law)
    WAVE_FORMAT_MS_MULAW                = 0x0007, //!< Microsoft CCITT G.711 (A-Law and u-Law)
    WAVE_FORMAT_MS_DTS                  = 0x0008,
    WAVE_FORMAT_MS_DRM                  = 0x0009,
    WAVE_FORMAT_WMAS                    = 0x000A, //!< WMA 9 Speech
    WAVE_FORMAT_MS_WMRTV                = 0x000B, //!< Windows Media RT Voice

    WAVE_FORMAT_OKI_ADPCM               = 0x0010,
    WAVE_FORMAT_DVI_ADPCM               = 0x0011, //!< Intel IMA ADPCM
    WAVE_FORMAT_MEDIASPACE_ADPCM        = 0x0012,
    WAVE_FORMAT_SIERRA_ADPCM            = 0x0013,
    WAVE_FORMAT_G723_ADPCM              = 0x0014,
    WAVE_FORMAT_DIGISTD                 = 0x0015,
    WAVE_FORMAT_DIGIFIX                 = 0x0016,
    WAVE_FORMAT_YAMAHA_ADPCM            = 0x0020,
    WAVE_FORMAT_SONARC                  = 0x0021,
    WAVE_FORMAT_DSPGROUP_TRUESPEECH     = 0x0022,
    WAVE_FORMAT_ECHOSC1                 = 0x0023,
    WAVE_FORMAT_AUDIOFILE_AF36          = 0x0024,
    WAVE_FORMAT_APTX                    = 0x0025,
    WAVE_FORMAT_AUDIOFILE_AF10          = 0x0026,
    WAVE_FORMAT_PROSODY_1612            = 0x0027,
    WAVE_FORMAT_LRC                     = 0x0028,

    WAVE_FORMAT_DOLBY_AC2               = 0x0030, //!< DOLBY AC-2
    WAVE_FORMAT_GSM610                  = 0x0031,
    WAVE_FORMAT_ANTEX_ADPCME            = 0x0033,
    WAVE_FORMAT_CONTROL_RES_VQLPC       = 0x0034,
    WAVE_FORMAT_DIGIREAL                = 0x0035,
    WAVE_FORMAT_DIGIADPCM               = 0x0036,
    WAVE_FORMAT_CONTROL_RES_CR10        = 0x0037,
    WAVE_FORMAT_NMS_VBXADPCM            = 0x0038,
    WAVE_FORMAT_CS_IMAADPCM             = 0x0039,
    WAVE_FORMAT_ECHOSC3                 = 0x003A,
    WAVE_FORMAT_ROCKWELL_ADPCM          = 0x003B,
    WAVE_FORMAT_ROCKWELL_DIGITALK       = 0x003C,
    WAVE_FORMAT_XEBEC                   = 0x003D,

    WAVE_FORMAT_G721_ADPCM              = 0x0040,
    WAVE_FORMAT_G728_CELP               = 0x0041,
    WAVE_FORMAT_MSG723                  = 0x0042,

    WAVE_FORMAT_MP1                     = 0x0050, //!< MPEG Layer 1
    WAVE_FORMAT_RT24                    = 0x0052,
    WAVE_FORMAT_PAC                     = 0x0053,

    WAVE_FORMAT_MP3                     = 0x0055, //!< MPEG Layer 3
    WAVE_FORMAT_LUCENT_G723             = 0x0059,
    WAVE_FORMAT_CIRRUS                  = 0x0060,
    WAVE_FORMAT_ESPCM                   = 0x0061,
    WAVE_FORMAT_VOXWARE                 = 0x0062,
    WAVE_FORMAT_CANOPUS_ATRAC           = 0x0063,
    WAVE_FORMAT_G726_ADPCM              = 0x0064,
    WAVE_FORMAT_G722_ADPCM              = 0x0065,
    WAVE_FORMAT_DSAT_DISPLAY            = 0x0067,
    WAVE_FORMAT_VOXWARE_BYTE_ALIGNED    = 0x0069,
    WAVE_FORMAT_VOXWARE_AC8             = 0x0070,
    WAVE_FORMAT_VOXWARE_AC10            = 0x0071,
    WAVE_FORMAT_VOXWARE_AC16            = 0x0072,
    WAVE_FORMAT_VOXWARE_AC20            = 0x0073,
    WAVE_FORMAT_VOXWARE_RT24            = 0x0074,
    WAVE_FORMAT_VOXWARE_RT29            = 0x0075,
    WAVE_FORMAT_VOXWARE_RT29HW          = 0x0076,
    WAVE_FORMAT_VOXWARE_VR12            = 0x0077,
    WAVE_FORMAT_VOXWARE_VR18            = 0x0078,
    WAVE_FORMAT_VOXWARE_TQ40            = 0x0079,
    WAVE_FORMAT_SOFTSOUND               = 0x0080,
    WAVE_FORMAT_VOXWARE_TQ60            = 0x0081,
    WAVE_FORMAT_MSRT24                  = 0x0082,
    WAVE_FORMAT_G729A                   = 0x0083,
    WAVE_FORMAT_MVI_MVI2                = 0x0084,
    WAVE_FORMAT_DF_G726                 = 0x0085,
    WAVE_FORMAT_DF_GSM610               = 0x0086,
    WAVE_FORMAT_ISIAUDIO                = 0x0088,
    WAVE_FORMAT_ONLIVE                  = 0x0089,
    WAVE_FORMAT_SBC24                   = 0x0091,
    WAVE_FORMAT_DOLBY_AC3_SPDIF         = 0x0092,
    WAVE_FORMAT_MEDIASONIC_G723         = 0x0093,
    WAVE_FORMAT_PROSODY_8KBPS           = 0x0094,
    WAVE_FORMAT_ZYXEL_ADPCM             = 0x0097,
    WAVE_FORMAT_PHILIPS_LPCBB           = 0x0098,
    WAVE_FORMAT_PACKED                  = 0x0099,
    WAVE_FORMAT_MALDEN_PHONYTALK        = 0x00A0,

    WAVE_FORMAT_AAC                     = 0x00FF, //!< AAC (also: 0x0180, 0x4143, 0x706D, 0xA106)

    WAVE_FORMAT_RHETOREX_ADPCM          = 0x0100,
    WAVE_FORMAT_IRAT                    = 0x0101,
    WAVE_FORMAT_VIVO_G723               = 0x0111,
    WAVE_FORMAT_VIVO_SIREN              = 0x0112,
    WAVE_FORMAT_DIGITAL_G723            = 0x0123,
    WAVE_FORMAT_SANYO_LD_ADPCM          = 0x0125,
    WAVE_FORMAT_SIPROLAB_ACEPLNET       = 0x0130,
    WAVE_FORMAT_SIPROLAB_ACELP4800      = 0x0131,
    WAVE_FORMAT_SIPROLAB_ACELP8V3       = 0x0132,
    WAVE_FORMAT_SIPROLAB_G729           = 0x0133,
    WAVE_FORMAT_SIPROLAB_G729A          = 0x0134,
    WAVE_FORMAT_SIPROLAB_KELVIN         = 0x0135,
    WAVE_FORMAT_G726ADPCM               = 0x0140,
    WAVE_FORMAT_QUALCOMM_PUREVOICE      = 0x0150,
    WAVE_FORMAT_QUALCOMM_HALFRATE       = 0x0151,
    WAVE_FORMAT_TUBGSM                  = 0x0155,

    WAVE_FORMAT_WMA1                    = 0x0160, //!< WMA version 1
    WAVE_FORMAT_WMA2                    = 0x0161, //!< WMA (v2) 7, 8, 9 Series
    WAVE_FORMAT_WMAP                    = 0x0162, //!< WMA 9 Professional
    WAVE_FORMAT_WMAL                    = 0x0163, //!< WMA 9 Lossless

    WAVE_FORMAT_CREATIVE_ADPCM          = 0x0200,
    WAVE_FORMAT_CREATIVE_FASTSPEECH8    = 0x0202,
    WAVE_FORMAT_CREATIVE_FASTSPEECH10   = 0x0203,
    WAVE_FORMAT_UHER_ADPCM              = 0x0210,
    WAVE_FORMAT_QUARTERDECK             = 0x0220,
    WAVE_FORMAT_ILINK_VC                = 0x0230,
    WAVE_FORMAT_RAW_SPORT               = 0x0240,
    WAVE_FORMAT_IPI_HSX                 = 0x0250,
    WAVE_FORMAT_IPI_RPELP               = 0x0251,
    WAVE_FORMAT_CS2                     = 0x0260,
    WAVE_FORMAT_SONY_SCX                = 0x0270,
    WAVE_FORMAT_FM_TOWNS_SND            = 0x0300,
    WAVE_FORMAT_BTV_DIGITAL             = 0x0400,
    WAVE_FORMAT_IAC2                    = 0x0402, //!< Indeo Audio Codec
    WAVE_FORMAT_QDESIGN_MUSIC           = 0x0450,
    WAVE_FORMAT_VME_VMPCM               = 0x0680,
    WAVE_FORMAT_TPC                     = 0x0681,
    WAVE_FORMAT_OLIGSM                  = 0x1000,
    WAVE_FORMAT_OLIADPCM                = 0x1001,
    WAVE_FORMAT_OLICELP                 = 0x1002,
    WAVE_FORMAT_OLISBC                  = 0x1003,
    WAVE_FORMAT_OLIOPR                  = 0x1004,
    WAVE_FORMAT_LH_CODEC                = 0x1100,
    WAVE_FORMAT_NORRIS                  = 0x1400,
    WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS = 0x1500,

    WAVE_FORMAT_DOLBY_AC3               = 0x2000, //!< DOLBY AC-3 or A52
    WAVE_FORMAT_DTS                     = 0x2001,
    WAVE_FORMAT_RA_14                   = 0x2002, //!< RealAudio 1 / 14.4
    WAVE_FORMAT_RA_28                   = 0x2003, //!< RealAudio 2 / 28.8
    WAVE_FORMAT_RA_cook                 = 0x2004, //!< RealAudio G2 / Cook
    WAVE_FORMAT_RA3                     = 0x2005, //!< RealAudio 3
    WAVE_FORMAT_RA9                     = 0x2006, //!< RealAudio 9 AAC (RAAC)
    WAVE_FORMAT_RA10p                   = 0x2007, //!< RealAudio 10 AAC+ (RACP)

    WAVE_FORMAT_VORBIS1                 = 0x674F, //!< Ogg Vorbis mode 1
    WAVE_FORMAT_VORBIS2                 = 0x6750, //!< Ogg Vorbis mode 2
    WAVE_FORMAT_VORBIS3                 = 0x6751, //!< Ogg Vorbis mode 3
    WAVE_FORMAT_VORBIS1p                = 0x676F, //!< Ogg Vorbis mode 1+
    WAVE_FORMAT_VORBIS2p                = 0x6770, //!< Ogg Vorbis mode 2+
    WAVE_FORMAT_VORBIS3p                = 0x6772, //!< Ogg Vorbis mode 3+

    WAVE_FORMAT_EXTENSIBLE              = 0xFFFE

} twocc_list_e;

/* ************************************************************************** */
#endif // MINIVIDEO_TWOCC_H
