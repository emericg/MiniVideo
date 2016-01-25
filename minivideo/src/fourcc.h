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
 * \file      fourcc.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef FOURCC_H
#define FOURCC_H

// minivideo headers
#include "avcodecs.h"
#include "typedef.h"

/* ************************************************************************** */

/*!
 * Good ressources about FourCCs:
 * http://www.fourcc.org/codecs.php
 * http://wiki.multimedia.cx/index.php?title=Category:Video_FourCCs
 *
 * Upper and lower cases are present, because we never know what will hit us
 * with those AVI containers...
 * These FourCC are set in big endian in this enum, so you may want to flip them
 * if you are reading them from Windows specific stuff (ex: WVF structures).
 */
typedef enum fourcc_list_e
{
    // Video codecs
    fcc_AVC1 = 0x41564331, //!< H.264 / MPEG-4 part 10 "AVC"
    fcc_avc1 = 0x61766331,
    fcc_AVCC = 0x41564343,
    fcc_avcc = 0x61766363,
    fcc_H264 = 0x48323634,
    fcc_h264 = 0x68323634,
    fcc_X264 = 0x58323634,
    fcc_x264 = 0x78323634,

    fcc_HVC1 = 0x48764331, //!< H.265 / MPEG-H part 2 "HEVC"
    fcc_hvc1 = 0x68766331,
    fcc_HEVC = 0x48455643,
    fcc_hevc = 0x68657663,
    fcc_H265 = 0x48323635,
    fcc_h265 = 0x68323635,
    fcc_X265 = 0x58323635,
    fcc_x265 = 0x78323635,

    fcc_VP30 = 0x76503330, //!< VPx family
    fcc_VP40 = 0x56503430,
    fcc_VP50 = 0x56503530,
    fcc_VP60 = 0x56503630,
    fcc_VP70 = 0x56503730,
    fcc_VP80 = 0x56503830,
    fcc_VP90 = 0x56503930,
    fcc_VP10 = 0x56503130,

    fcc_CFHD = 0x43464844, //!< CineForm / VC-5
    fcc_cfhd = 0x63666864,

    fcc_ap4h = 0x61703468, //!< Apple ProRes 4444
    fcc_ap4x = 0x61703478, //!< Apple ProRes 4444 (XQ)
    fcc_apch = 0x61706368, //!< Apple ProRes 422 (HQ)
    fcc_apcn = 0x6170636e, //!< Apple ProRes 422
    fcc_apco = 0x6170636f, //!< Apple ProRes 422 (Proxy)
    fcc_apcs = 0x61706373, //!< Apple ProRes 422 (LT)

    fcc_MPEG = 0x4D504547, //!< MPEG-1/2
    fcc_mpeg = 0x6D706567,
    fcc_MPG1 = 0x4D504731,
    fcc_mpg1 = 0x6D706731,
    fcc_MPG2 = 0x4D504732,
    fcc_mpg2 = 0x6D706732,

    fcc_xvid = 0x78766964, //!< MPEG-4 part 2 "ASP
    fcc_XVID = 0x58564944,
    fcc_FMP4 = 0x464D5034,
    fcc_MP4V = 0x6D703476,
    fcc_divx = 0x64697678, //!< DivX 4 -> 6
    fcc_DIVX = 0x44495658,
    fcc_DX50 = 0x44583530, //!< DivX 5

    fcc_DIV1 = 0x44495631, //!< Old MPEG-4 based codecs
    fcc_DIV2 = 0x44495632,
    fcc_DIV4 = 0x44495634,
    fcc_DIV5 = 0x44495635,

    fcc_MPG4 = 0x4D504734, //!< Microsoft MPEG-4
    fcc_MP41 = 0x4D503431, //!< Microsoft MPEG-4 (version 1)
    fcc_MP42 = 0x4D503432, //!< Microsoft MPEG-4 (version 2)
    fcc_MP43 = 0x4D503433, //!< Microsoft MPEG-4 (version 3)
    fcc_DIV3 = 0x44495633,
    fcc_WMV7 = 0x574D5637,
    fcc_WMV8 = 0x574D5638,
    fcc_AP41 = 0x41503431,
    fcc_COL1 = 0x434F4C31,

    fcc_WMV1 = 0x574D5631, //!< Windows Media Video codecs
    fcc_WMV2 = 0x574D5632,
    fcc_WMV3 = 0x574D5633,
    fcc_WVC1 = 0x57564331,

    fcc_dvsd = 0x64767364, //!< DV
    fcc_DVSD = 0x44565344,

    fcc_AI55 = 0x41,           //!< AVC Intra  50 / 1080 interlace
    fcc_AI5q = 0x41,           //!< AVC Intra  50 /  720
    fcc_AI15 = 0x41,           //!< AVC Intra 100 / 1080 interlace
    fcc_AI1q = 0x41,           //!< AVC Intra 100 /  720
    fcc_AI12 = 0x41,           //!< AVC Intra 100 / 1080

    // Uncompressed videos
    fcc_VYUV = 0x56595556, //!< Uncompressed YUV types
    fcc_YUY2 = 0x59555932,

    // Audio
    fcc_MP4A = 0x4D503441, //!< AAC
    fcc_mp4A = 0x6D703461,

    fcc_AC3  = 0x41432D33, //!< Ac-3
    fcc_ac3  = 0x61632D33  //!< ac-3

} fourcc_list_e;

/* ************************************************************************** */

AVCodec_e getCodecFromFourCC(uint32_t fcc);

char *getFccString_le(const int fcc_in, char *fcc_out);

char *getFccString_be(const int fcc_in, char *fcc_out);


/* ************************************************************************** */
#endif // FOURCC_H
