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
#include "minivideo_export.h"

/* ************************************************************************** */

/*!
 * Good ressources about FourCCs:
 * http://www.fourcc.org/codecs.php
 * http://wiki.multimedia.cx/index.php?title=Category:Video_FourCCs
 * https://support.microsoft.com/en-us/kb/281188
 *
 * Both upper and lower cases FourCC are present sometimes, because we encountered
 * in the wild...
 * These FourCC are set in big endian in this enum, so you may want to flip them
 * if you are reading them from Windows specific stuff (ex: WVF structures).
 */
typedef enum fourcc_list_e
{
    // Video codecs ////////////////////////////////////////////////////////////

    fcc_MPEG = 0x4D504547, //!< MPEG-1/2
    fcc_mpeg = 0x6D706567,
    fcc_MPG1 = 0x4D504731,
    fcc_mpg1 = 0x6D706731,
    fcc_MPG2 = 0x4D504732,
    fcc_mpg2 = 0x6D706732,
    fcc_DVR  = 0x44565220,
    fcc_PIM1 = 0x50494D31,
    fcc_PIM2 = 0x50494D32,

    fcc_H261 = 0x48323631, //!< H.261

    fcc_DIV1 = 0x44495631, //!< Old MPEG-4 based codecs
    fcc_DIV2 = 0x44495632,
    fcc_DIV4 = 0x44495634,
    fcc_DIV5 = 0x44495635,

    fcc_div3 = 0x64697633, //!< DivX 3
    fcc_mpg3 = 0x6D706733, //!< DivX 3
    fcc_divx = 0x64697678, //!< DivX 4 -> 6
    fcc_DIVX = 0x44495658,
    fcc_DX50 = 0x44583530, //!< DivX 5
    fcc_DX60 = 0x44583630, //!< DivX 6

    fcc_MP41 = 0x4D503431, //!< Microsoft MPEG-4 (version 1)
    fcc_MPG4 = 0x4D504734,
    fcc_mpg4 = 0x6D706734,
    fcc_MP42 = 0x4D503432, //!< Microsoft MPEG-4 (version 2)
    fcc_MP43 = 0x4D503433, //!< Microsoft MPEG-4 (version 3)
    fcc_DIV3 = 0x44495633,
    fcc_WMV1 = 0x574D5631, //!< Windows Media Video 7
    fcc_WMV7 = 0x574D5637,
    fcc_WMV2 = 0x574D5632, //!< Windows Media Video 8
    fcc_WMV8 = 0x574D5638,
    fcc_AP41 = 0x41503431,
    fcc_COL1 = 0x434F4C31,
    fcc_MP4S = 0x4D503453, //!< Microsoft ISO MPEG-4 version 1 (MPEG-4 Simple Profile compliant)
    fcc_M4S2 = 0x4D345332, //!< Microsoft ISO MPEG-4 version 1.1 (MPEG-4 Advance Simple Profile compliant)
    fcc_MSS1 = 0x4D535331, //!< Windows Media Screen 7
    fcc_MSS2 = 0x4D535332, //!< Windows Media Screen 9
    fcc_MSA1 = 0x4D534131, //!< Windows Media Screen
    fcc_WMV3 = 0x574D5633, //!< Windows Media Video 9 (VC-1 compliant)
    fcc_WMVA = 0x574D5641, //!< Windows Media Video 9 Advanced Profile
    fcc_WVC1 = 0x57564331, //!< Windows Media Video 9 Advanced Profile (VC-1 compliant)

    fcc_xvid = 0x78766964, //!< MPEG-4 part 2 "ASP"
    fcc_XVID = 0x58564944,
    fcc_FMP4 = 0x464D5034,
    fcc_MP4V = 0x4D503456,
    fcc_mp4v = 0x6D703476,

    fcc_D263 = 0x44323633, //!< H.263
    fcc_H263 = 0x48323633,
    fcc_L263 = 0x4C323633,
    fcc_M263 = 0x4D323633,
    fcc_S263 = 0x53323633,
    fcc_T263 = 0x54323633,
    fcc_U263 = 0x55323633,
    fcc_X263 = 0x58323633,

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
    fcc_HEV1 = 0x48455631,
    fcc_hev1 = 0x68657631,
    fcc_H265 = 0x48323635,
    fcc_h265 = 0x68323635,
    fcc_X265 = 0x58323635,
    fcc_x265 = 0x78323635,

    fcc_VP30 = 0x76503330,
    fcc_VP31 = 0x76503331, //!< Ogg Theora
    fcc_VP32 = 0x76503332,
    fcc_VP40 = 0x56503430,
    fcc_VP50 = 0x56503530,
    fcc_VP60 = 0x56503630,
    fcc_VP61 = 0x56503631,
    fcc_VP62 = 0x56503632,
    fcc_VP6F = 0x56503646,
    fcc_VP70 = 0x56503730, //!< VP7 (General Profile)
    fcc_VP71 = 0x56503731, //!< VP7 (Error Resilient Profile)
    fcc_VP72 = 0x56503732,
    fcc_VP80 = 0x56503830,
    fcc_VP90 = 0x56503930,
    fcc_VP10 = 0x56503130,

    fcc_BBCD = 0x42424344, //!< Dirac / VC-2

    fcc_AVdn = 0x4156646E, //!< DNxHD / VC-3

    fcc_CFHD = 0x43464844, //!< CineForm / VC-5
    fcc_cfhd = 0x63666864,

    fcc_apco = 0x6170636f, //!< Apple ProRes 422 (Proxy)
    fcc_apcs = 0x61706373, //!< Apple ProRes 422 (LT)
    fcc_apcn = 0x6170636e, //!< Apple ProRes 422
    fcc_apch = 0x61706368, //!< Apple ProRes 422 (HQ)
    fcc_ap4h = 0x61703468, //!< Apple ProRes 4444
    fcc_ap4x = 0x61703478, //!< Apple ProRes 4444 (XQ)

    fcc_REDC = 0x52454443, //!< REDCode

    fcc_RV10 = 0x52563130, //!< RealVideo
    fcc_RV13 = 0x52563133,
    fcc_rv20 = 0x72763230, //!< RealVideo G2
    fcc_rv30 = 0x72763330, //!< RealVideo 3
    fcc_rv40 = 0x72763430, //!< RealVideo 4

    // Uncommon video codecs ///////////////////////////////////////////////////

    fcc_cvid = 0x63766964, //!< Cinepak
    fcc_CVID = 0x43564944,
    fcc_SNOW = 0x534E4F57, //!< Snow
    fcc_snow = 0x736E6F77, //!< Snow
    fcc_svq1 = 0x73767131, //!< Sorenson Video
    fcc_SVQ1 = 0x53565131,
    fcc_svqi = 0x73767169,
    fcc_SVQ3 = 0x53565133,
    fcc_RT21 = 0x52543231, //!< Intel Indeo Video 2 or Real-Time Video 2
    fcc_IV31 = 0x49563331, //!< Intel Indeo Video 3
    fcc_iV31 = 0x69563331,
    fcc_iv31 = 0x69763331,
    fcc_IV32 = 0x49563332,
    fcc_iV32 = 0x69563332,
    fcc_iv32 = 0x69763332,
    fcc_iv41 = 0x69763431,
    fcc_iV41 = 0x69563431,
    fcc_IV41 = 0x49563431,
    fcc_IV42 = 0x49563432,
    fcc_iV42 = 0x69563432,
    fcc_IV50 = 0x49563530,
    fcc_iV50 = 0x69563530,
    fcc_CHQX = 0x43485158, //!< Canopus HQ
    fcc_icod = 0x69636F64, //!< Apple Intermediate Codec
    fcc_rpza = 0x72707A61, //!< Apple Video
    fcc_azpr = 0x617A7072,

    fcc_dvsd = 0x64767364, //!< DV
    fcc_DVSD = 0x44565344,
    fcc_CDVC = 0x43564443, //!< Canopus DV
    fcc_CDVH = 0x43555648,
    fcc_CUV5 = 0x43555635,
    fcc_dv5n = 0x6476356E, //!< Apple DVCPRO50 - NTSC
    fcc_dv5p = 0x64763570, //!< Apple DVCPRO50 - PAL
    fcc_dvc  = 0x64766320, //!< Apple DV/DVCPRO - NTSC
    fcc_dvcp = 0x64766370, //!< Apple DV - PAL
    fcc_dvpp = 0x64767070, //!< Apple DVCPRO - PAL

    fcc_AI55 = 0x41493535, //!< AVC Intra  50 / 1080 interlaced
    fcc_AI5q = 0x41493571, //!< AVC Intra  50 /  720
    fcc_AI15 = 0x41493135, //!< AVC Intra 100 / 1080 interlaced
    fcc_AI1q = 0x41493171, //!< AVC Intra 100 /  720
    fcc_AI12 = 0x41493132, //!< AVC Intra 100 / 1080

    fcc_BIKf = 0x42494B66, //!< Bink Video!
    fcc_BIKg = 0x42494B67,
    fcc_BIKh = 0x42494B68,
    fcc_BIKi = 0x42494B69,

    // Lossless video codecs ///////////////////////////////////////////////////

    fcc_FFV1 = 0x46465631, //!< FFV1
    fcc_PNG1 = 0x504E4731, //!< CorePNG
    fcc_CLLC = 0x434C4C43, //!< Canopus LossLess

    // Uncompressed video //////////////////////////////////////////////////////

    fcc_VYUV = 0x56595556, //!< Uncompressed YUV types
    fcc_YUY2 = 0x59555932,

    // Audio codecs ////////////////////////////////////////////////////////////

    fcc_MP4A = 0x4D503441, //!< AAC
    fcc_mp4a = 0x6D703461,

    fcc_AC3  = 0x41432D33, //!< Ac-3
    fcc_ac3  = 0x61632D33, //!< ac-3
    fcc_AC4  = 0x41432D34, //!< Ac-4
    fcc_ac4  = 0x61632D34, //!< ac-4

    fcc_agsm = 0x6167736D, //!< GSM

    // Uncommon audio codecs ///////////////////////////////////////////////////

    fcc_28_8 = 0x32385F38, //!< RealAudio 28.8
    fcc_cook = 0x636F6F6B, //!< RealAudio cook
    fcc_atrc = 0x61747263, //!< Used for Sony ATRAC in RealMedia
    fcc_dnet = 0x646E6574, //!< Used for A52/AC3 in RealMedia (the byte order of the data is reversed from standard AC3)
    fcc_raac = 0x72616163, //!< Used for AAC-LC in RealMedia
    fcc_racp = 0x72616370, //!< Used for HE-AAC in RealMedia

    // Lossless audio codecs ///////////////////////////////////////////////////

    fcc_alac = 0x616C6163, //!< Apple Lossless Audio Coding

    // Uncompressed audio //////////////////////////////////////////////////////

    fcc_alaw = 0x616C6177, //!< A-law logarithmic PCM
    fcc_ulaw = 0x756C6177, //!< mu-law logarithmic PCM

    fcc_fl32 = 0x666C3332, //!< 32-bit floating point linear PCM (little endian)
    fcc_f32l = 0x6633326C, //!< 32-bit floating point linear PCM (little endian)
    fcc_fl64 = 0x666C3634, //!< 64-bit floating point linear PCM (little endian)
    fcc_f64l = 0x6636346C, //!< 64-bit floating point linear PCM (little endian)

    fcc_raw  = 0x72617720, //!< 16-bit unsigned linear PCM (little endian)
    fcc_araw = 0x61726177, //!< 16-bit unsigned linear PCM (little endian)
    fcc_lpcm = 0x6C70636D, //!< ??
    fcc_sowt = 0x736F7774, //!< 16-bit signed linear PCM (little endian)
    fcc_tows = 0x74647773, //!< 16-bit signed linear PCM (big endian)
    fcc_in24 = 0x696E3234, //!< 24-bit signed linear PCM (big endian)
    fcc_in32 = 0x696E3332, //!< 32-bit signed linear PCM (big endian)
    fcc_s8   = 0x73382020, //!< 8-bit signed linear PCM
    fcc_s16l = 0x7331366C, //!< 16-bit signed linear PCM (little endian)
    fcc_s16b = 0x73313662, //!< 16-bit signed linear PCM (big endian)
    fcc_s24l = 0x7332346C, //!< 24-bit signed linear PCM (little endian)
    fcc_s24b = 0x73323462, //!< 24-bit signed linear PCM (big endian)
    fcc_s32l = 0x7333326C, //!< 32-bit signed linear PCM (little endian)
    fcc_s32b = 0x73333262, //!< 32-bit signed linear PCM (big endian)
    fcc_u8   = 0x75382020, //!< 8-bit unsigned linear PCM
    fcc_u16l = 0x7531366C, //!< 16-bit unsigned linear PCM (little endian)
    fcc_u16b = 0x75313662, //!< 16-bit unsigned linear PCM (big endian)
    fcc_u24l = 0x7532346C, //!< 24-bit unsigned linear PCM (little endian)
    fcc_u24b = 0x75323462, //!< 24-bit unsigned linear PCM (big endian)
    fcc_u32l = 0x7533326C, //!< 32-bit unsigned linear PCM (little endian)
    fcc_u32b = 0x75333262, //!< 32-bit unsigned linear PCM (big endian)

    fcc_ima4 = 0x696D6134, //!< Apple IMA ADPCM

} fourcc_list_e;

/* ************************************************************************** */

minivideo_EXPORT AVCodec_e getCodecFromFourCC(const uint32_t fcc);

char *getFccString_le(const uint32_t fcc_in, char *fcc_out);

char *getFccString_be(const uint32_t fcc_in, char *fcc_out);

/* ************************************************************************** */
#endif // FOURCC_H
