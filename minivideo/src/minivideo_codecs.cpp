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
 * \date      2014
 */

#include "minivideo_codecs.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wswitch"
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wswitch"
#endif

/* ************************************************************************** */

CodecProfiles_e getH264CodecProfile(const unsigned profil_idc,
                                    const bool constraint_set0_flag, const bool constraint_set1_flag,
                                    const bool constraint_set2_flag, const bool constraint_set3_flag,
                                    const bool constraint_set4_flag, const bool constraint_set5_flag)
{
    switch (profil_idc)
    {
        // Annex A Profiles
        case 66:
            if (constraint_set1_flag)
                return PROF_H264_CBP;
            else
                return PROF_H264_BP;
        case 77:
            return PROF_H264_MP;
        case 88:
            return PROF_H264_XP;
        case 100:
            if (constraint_set4_flag && constraint_set5_flag)
                return PROF_H264_CHiP;
            else if (constraint_set4_flag)
                return PROF_H264_PHiP;
            else
                return PROF_H264_HiP;
        case 110:
            if (constraint_set4_flag)
                return PROF_H264_PHi10P;
            else if (constraint_set3_flag)
                return PROF_H264_Hi10IntraP;
            else
                return PROF_H264_Hi10P;
        case 122:
            if (constraint_set3_flag)
                return PROF_H264_Hi422IntraP;
            else
                return PROF_H264_Hi422P;
        case 244:
            if (constraint_set5_flag)
                return PROF_H264_Hi444IntraP;
            else
                return PROF_H264_Hi444P;
        case 44:
            return PROF_H264_M444IntraP;

        // Annex G Profiles
        case 83:
            if (constraint_set5_flag)
                return PROF_H264_ScCBP;
            else
                return PROF_H264_ScBP;
        case 86:
            if (constraint_set5_flag)
                return PROF_H264_ScCHiP;
            else if (constraint_set3_flag)
                return PROF_H264_ScHiIntraP;
            else
                return PROF_H264_ScHiP;

        // Annex H Profiles
        case 118:
            return PROF_H264_MvHiP;
        case 128:
            return PROF_H264_StHiP;
        case 134:
            return PROF_H264_MfcHiP;

        // Annex I Profiles
        case 138:
            return PROF_H264_MvDHiP;
        case 135:
            return PROF_H264_MfcDHiP;
        // Annex J Profiles
        case 139:
            return PROF_H264_EMvDHiP;

        default:
            return PROF_H264_unknown;
    }
}

CodecProfiles_e getH265CodecProfile(const unsigned profil_idc,
                                    const bool general_one_picture_only_constraint_flag,
                                    const bool general_max_8bit_constraint_flag,
                                    const int chroma_format_idc)
{
    switch (profil_idc)
    {
        // Annex A Profiles
        case 1:
            return PROF_H265_Main;
        case 2:
            if (general_one_picture_only_constraint_flag)
                return PROF_H265_Main10_still;
            else
                return PROF_H265_Main10;
        case 3:
            return PROF_H265_Main_still;

        case 4: // TODO // everything else Main and Monochrome
            return PROF_H265_unknown;

        // Annex G Profiles
        case 6:
            return PROF_H265_Multiview_Main;

        // Annex H Profiles
        case 7:
            if (general_max_8bit_constraint_flag == false)
                return PROF_H265_Scalable_Main10;
            else
                return PROF_H265_Scalable_Main;

        // Annex I Profiles
        case 8:
            return PROF_H265_3D_Main;

        case 10:
            if (chroma_format_idc == 0)
                return PROF_H265_Scalable_Monochrome; // TODO // 12/16 bits
            else
                return PROF_H265_Scalable_Main444;

        case 5: // TODO High Throughput
        case 11: // TODO High Throughput

        default:
            return PROF_H265_unknown;
    }
}

CodecProfiles_e getH266CodecProfile(const unsigned profil_idc)
{
    switch (profil_idc)
    {
        default:
            return PROF_H266_unknown;
    }
}

/* ************************************************************************** */

const char *getCodecString(const StreamType_e type, const Codecs_e codec, const bool long_description)
{
    if (type == stream_AUDIO || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_LPCM:
                if (long_description)
                    return "Linear PCM (Linear Pulse Code Modulation)";
                else
                    return "PCM";
            case CODEC_LogPCM:
                if (long_description)
                    return "Logarithmic PCM (Logarithmic Pulse Code Modulation)";
                else
                    return "Log PCM";
            case CODEC_DPCM:
                if (long_description)
                    return "DPCM (Differential Pulse Code Modulation)";
                else
                    return "DPCM";
            case CODEC_ADPCM:
                if (long_description)
                    return "ADPCM (Adaptative Differential Pulse Code Modulation)";
                else
                    return "ADPCM";
            case CODEC_PDM:
                if (long_description)
                    return "PDM (Pulse Density Modulation)";
                else
                    return "PDM";

            case CODEC_MPEG_L1:
                if (long_description)
                    return "MP1 (MPEG-1/2 Layer 1)";
                else
                    return "MP1";
            case CODEC_MPEG_L2:
                if (long_description)
                    return "MP2 (MPEG-1/2 Layer 2)";
                else
                    return "MP2";
            case CODEC_MPEG_L3:
                if (long_description)
                    return "MP3 (MPEG-1/2 Layer 3)";
                else
                    return "MP3";

            case CODEC_AAC:
                if (long_description)
                    return "AAC (Advance Audio Coding)";
                else
                    return "AAC";

            case CODEC_MPEG4_HVXC:
                if (long_description)
                    return "MPEG-4 HVXC (Harmonic Vector eXcitation Coding)";
                else
                    return "MPEG-4 HVXC";
            case CODEC_MPEG4_CELP:
                if (long_description)
                    return "MPEG-4 CELP (Code Excited Linear Prediction)";
                else
                    return "MPEG-4 CELP";
            case CODEC_MPEG4_TwinVQ:
                return "MPEG-4 TwinVQ";
            case CODEC_MPEG4_HILN:
                if (long_description)
                    return "MPEG-4 HILN (Harmonic and Individual Line plus Noise)";
                else
                    return "MPEG-4 HILN";
            case CODEC_MPEG4_DST:
                if (long_description)
                    return "MPEG-4 DST (Direct Stream Transfer)";
                else
                    return "MPEG-4 DST";
            case CODEC_MPEG4_ALS:
                if (long_description)
                    return "MPEG-4 ALS (Audio Lossless Coding)";
                else
                    return "MPEG-4 ALS";
            case CODEC_MPEG4_SLS:
                if (long_description)
                    return "MPEG-4 SLS (Scalable Lossless Coding)";
                else
                    return "MPEG-4 SLS";
            case CODEC_MPEGH_3D_AUDIO:
                return "MPEG-H 3D AUDIO";

            case CODEC_SPEEX:
                return "Speex";
            case CODEC_VORBIS:
                return "Vorbis";
            case CODEC_OPUS:
                return "Opus";

            case CODEC_AC2:
                if (long_description)
                    return "AC-2 (Dolby Labs)";
                else
                    return "AC-2";
            case CODEC_AC3:
                if (long_description)
                    return "AC-3 (Dolby Digital)";
                else
                    return "AC-3";
            case CODEC_EAC3:
                if (long_description)
                    return "EAC-3 (Dolby Digital Plus)";
                else
                    return "EAC-3";
            case CODEC_AC4:
                if (long_description)
                    return "AC-4 (Dolby AC-4)";
                else
                    return "AC-4";
            case CODEC_DolbyTrueHD:
                return "Dolby TrueHD";
            case CODEC_DolbyE:
                return "Dolby E";

            case CODEC_DTS:
                return "DTS";
            case CODEC_DTS_HD:
                return "DTS-HD";
            case CODEC_DTS_X:
                return "DTS-X";

            case CODEC_WMA:
                if (long_description)
                    return "WMA (Windows Media Audio)";
                else
                    return "WMA";
            case CODEC_MPC:
                if (long_description)
                    return "MPC (Musepack)";
                else
                    return "MPC";
            case CODEC_GSM:
                return "GSM";

            case CODEC_ATRAC:
                if (long_description)
                    return "Sony ATRAC";
                else
                    return "ATRAC";
            case CODEC_ATRAC3plus:
                if (long_description)
                    return "Sony ATRAC 3 plus";
                else
                    return "ATRAC3+";

            case CODEC_AMR:
                if (long_description)
                    return "AMR (Adaptive Multi Rate)";
                else
                    return "AMR";

            case CODEC_RA_14:
                return "RealAudio 1 / 14.4";
            case CODEC_RA_28:
                return "RealAudio 2 / 28.8";
            case CODEC_RA_cook:
                return "RealAudio G2 / Cook";
            case CODEC_RA_ralf:
                return "RealAudio Lossless Format";
            case CODEC_IAC2:
                return "Indeo Audio Codec";

            case CODEC_APE:
                if (long_description)
                    return "APE (Monkey's Audio)";
                else
                    return "APE";
            case CODEC_FLAC:
                if (long_description)
                    return "FLAC (Free Lossless Audio Codec)";
                else
                    return "FLAC";
            case CODEC_ALAC:
                if (long_description)
                    return "ALAC (Apple Lossless Audio Codec)";
                else
                    return "ALAC";
            case CODEC_WAVPACK:
                return "WavPack";
        }
    }

    if (type == stream_VIDEO || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_MPEG1:
                return "MPEG-1";
            case CODEC_MPEG4_ASP:
                return "MPEG-4 part 2 'ASP'";
            case CODEC_MPEG4_IVC:
                return "MPEG-4 part 33 'IVC'";
            case CODEC_MPEG5_EVC:
                return "MPEG-5 part 1 'EVC'";
            case CODEC_MPEG5_LCEVC:
                return "MPEG-5 part 2 'LCEVC'";

            case CODEC_H120:
                return "H.120";
            case CODEC_H261:
                return "H.261";
            case CODEC_H262:
                return "MPEG-2";
            case CODEC_H263:
                return "H.263";
            case CODEC_H264:
                if (long_description)
                    return "H.264 (MPEG-4 part 10 'AVC')";
                else
                    return "H.264";
            case CODEC_H265:
                if (long_description)
                    return "H.265 (MPEG-H part 2 'HEVC')";
                else
                    return "H.265";
            case CODEC_H266:
                if (long_description)
                    return "H.266 (MPEG-I part 3 'VVC')";
                else
                    return "H.266";

            case CODEC_VP3:
                if (long_description)
                    return "VP3 (Ogg Theora)";
                else
                    return "VP3";
            case CODEC_VP4:
                return "VP4";
            case CODEC_VP5:
                return "VP5";
            case CODEC_VP6:
                return "VP6";
            case CODEC_VP7:
                return "VP7";
            case CODEC_VP8:
                return "VP8";
            case CODEC_VP9:
                return "VP9";

            case CODEC_DAALA:
                return "Daala";
            case CODEC_THOR:
                return "Thor";

            case CODEC_AV1:
                if (long_description)
                    return "AV1 (AOMedia Video 1)";
                else
                    return "AV1";

            case CODEC_AVS1:
                return "AVS1";
            case CODEC_AVS2:
                return "AVS2";
            case CODEC_AVS3:
                return "AVS3";

            case CODEC_VC1:
                if (long_description)
                    return "VC-1 (Windows Media Video)";
                else
                    return "VC-1";
            case CODEC_VC2:
                if (long_description)
                    return "VC-2 (Dirac)";
                else
                    return "VC-2";
            case CODEC_VC3:
                if (long_description)
                    return "VC-3 (DNxHD)";
                else
                    return "VC-3";
            case CODEC_VC5:
                if (long_description)
                    return "VC-5 (CineForm variant)";
                else
                    return "VC-5";
            case CODEC_VC6:
                if (long_description)
                    return "VC-6 (?)";
                else
                    return "VC-6";

            case CODEC_MSMPEG4:
                return "Microsoft MPEG-4";
            case CODEC_WMV7:
                if (long_description)
                    return "Windows Media Video 7";
                else
                    return "WMV7";
            case CODEC_WMV8:
                if (long_description)
                    return "Windows Media Video 8";
                else
                    return "WMV8";
            case CODEC_WMV9:
                if (long_description)
                    return "Windows Media Video 9";
                else
                    return "WMV9";
            case CODEC_WMSCR:
                if (long_description)
                    return "Windows Media Screen 7-9";
                else
                    return "WMS 7-9";
            case CODEC_WMP:
                if (long_description)
                    return "Windows Media Picture";
                else
                    return "WMP";

            case CODEC_RV10:
                return "RealVideo";
            case CODEC_RV20:
                return "RealVideo G2";
            case CODEC_RV30:
                return "RealVideo 3";
            case CODEC_RV40:
                return "RealVideo 4";

            case CODEC_INDEO2:
                if (long_description)
                    return "Indeo 2 (Intel Indeo Video 2)";
                else
                    return "Indeo 2";
            case CODEC_INDEO3:
                if (long_description)
                    return "Indeo 3 (Intel Indeo Video 3)";
                else
                    return "Indeo 3";
            case CODEC_INDEO4:
                if (long_description)
                    return "Indeo 4 (Intel Indeo Video 4)";
                else
                    return "Indeo 4";
            case CODEC_INDEO5:
                if (long_description)
                    return "Indeo 5 (Intel Indeo Video 5)";
                else
                    return "Indeo 5";
            case CODEC_SPARK:
                return "Sorenson Spark";
            case CODEC_SVQ1:
                if (long_description)
                    return "SVQ1 (Sorenson Video 1)";
                else
                    return "SVQ1";
            case CODEC_SVQ3:
                if (long_description)
                    return "SVQ3 (Sorenson Video 3)";
                else
                    return "SVQ3";
            case CODEC_CanopusHQ:
                return "Canopus HQ";
            case CODEC_CanopusHQA:
                return "Canopus HQ Alpha";
            case CODEC_CanopusHQX:
                return "Canopus HQX";
            case CODEC_BINK:
                return "Bink Video!";
            case CODEC_BINK2:
                return "Bink2 Video!";
            case CODEC_CINEPAK:
                return "Cinepak";
            case CODEC_DIRAC:
                return "Dirac";
            case CODEC_SNOW:
                return "Snow";
            case CODEC_icod:
                return "Apple Intermediate Codec";
            case CODEC_rpza:
                return "Apple Video";
            case CODEC_QtAnimation:
                return "Apple QuickTime Animation";
            case CODEC_QtGraphics:
                return "Apple QuickTime Graphics";

            case CODEC_CINEFORM:
                return "CineForm";
            case CODEC_REDCode:
                return "REDCode";
            case CODEC_DNxHD:
                return "DNxHD";

            case CODEC_PRORES_422_PROXY:
                return "Apple ProRes 422 (Proxy)";
            case CODEC_PRORES_422_LT:
                return "Apple ProRes 422 (LT)";
            case CODEC_PRORES_422:
                return "Apple ProRes 422";
            case CODEC_PRORES_422_HQ:
                return "Apple ProRes 422 (HQ)";
            case CODEC_PRORES_4444:
                return "Apple ProRes 4444";
            case CODEC_PRORES_4444_XQ:
                return "Apple ProRes 4444 (XQ)";
            case CODEC_PRORES_RAW:
                return "Apple ProRes RAW";
            case CODEC_PRORES_RAW_HQ:
                return "Apple ProRes RAW (HQ)";

            case CODEC_DV_SONY:
                return "Sony DV";
            case CODEC_DV_CANOPUS:
                return "Canopus DV";
            case CODEC_DV_APPLE:
                return "Apple DV";
            case CODEC_DV_PANASONIC:
                return "Panasonic DV";

            case CODEC_FFV1:
                return "FFV1";
            case CODEC_CanopusLL:
                return "Canopus Lossless";

            case CODEC_PNG:
                if (long_description)
                    return "PNG (image)";
                else
                    return "PNG";
            case CODEC_CorePNG:
                return "CorePNG";
            case CODEC_BMP:
                if (long_description)
                    return "Windows Bitmap (image)";
                else
                    return "BMP";
            case CODEC_TGA:
                if (long_description)
                    return "Truevision Targa (image)";
                else
                    return "TGA";
            case CODEC_TIFF:
                if (long_description)
                    return "Tagged Image File Format (image)";
                else
                    return "TIFF";

            case CODEC_JPEG:
                if (long_description)
                    return "JPEG (image)";
                else
                    return "JPEG";
            case CODEC_MJPEG:
                return "Motion JPEG";
            case CODEC_JPEG_2K:
                if (long_description)
                    return "JPEG 2000 (image)";
                else
                    return "JPEG 2000";
            case CODEC_MJPEG_2K:
                return "Motion JPEG 2000";
            case CODEC_JPEG_XR:
                return "JPEG XR";
            case CODEC_JPEG_XS:
                return "JPEG XS";
            case CODEC_JPEG_XT:
                return "JPEG XT";
            case CODEC_JPEG_XL:
                return "JPEG XL";

            case CODEC_GIF:
                if (long_description)
                    return "Graphic Interchange Format (image)";
                else
                    return "GIF";
            case CODEC_WEBP:
                if (long_description)
                    return "WebP (image)";
                else
                    return "WebP";
            case CODEC_HEIF:
                if (long_description)
                    return "HEIF (High Efficiency Image Format)";
                else
                    return "HEIF";
            case CODEC_AVIF:
                if (long_description)
                    return "AVIF (AV1 Image Format)";
                else
                    return "AVIF";
        }
    }

    if (type == stream_TEXT || type == stream_UNKNOWN)
    {
        switch (codec)
        {
            case CODEC_SRT:
                return "SubRip";
            case CODEC_MicroDVD:
                return "MicroDVD";
            case CODEC_SSA:
                return "SubStation Alpha";
            case CODEC_ASS:
                return "Advanced SubStation Alpha";
            case CODEC_USF:
                return "Universal Subtitle Format";
            case CODEC_SSF:
                return "Structured Subtitle Format";
            case CODEC_SAMI:
                if (long_description)
                    return "Synchronized Accessible Media Interchange (SAMI)";
                else
                    return "SAMI";
            case CODEC_CMML:
                if (long_description)
                    return "Continuous Media Markup Language (CMML)";
                else
                    return "CMML";
            case CODEC_SMIL:
                if (long_description)
                    return "Synchronized Multimedia Integration Language (SMIL)";
                else
                    return "SMIL";
            case CODEC_STL:
                if (long_description)
                    return "Spruce Subtitle File (STL)";
                else
                    return "STL";
            case CODEC_TTML:
                if (long_description)
                    return "Timed Text Markup Language (TTML)";
                else
                    return "TTML";
            case CODEC_MPEG4_TTXT:
                return "MPEG-4 Timed Text";
            case CODEC_WebVTT:
                if (long_description)
                    return "Web Video Text Tracks (WebVTT)";
                else
                    return "WebVTT";
            case CODEC_Kate:
                if (long_description)
                    return "Karaoke and Text Encapsulation (Kate)";
                else
                    return "Kate";
            case CODEC_LRC:
                return "Song Lyrics (LRC)";

            case CODEC_Telext:
                return "Telext";
            case CODEC_DvbSub:
                return "DvbSub";
            case CODEC_VobSub:
                return "VobSub";
            case CODEC_AriSub:
                return "AriSub";
            case CODEC_PGS:
                return "Presentation Graphics Subtitles";
            case CODEC_TextST:
                return "TextST";
            case CODEC_CineCanvas:
                return "CineCanvas";
            case CODEC_PAC:
                return "Presentation Audio/Video Coding";
            case CODEC_XDS:
                return "Extended Data Services";
        }
    }

    return "";
}

/* ************************************************************************** */

const char *getCodecProfileString(const CodecProfiles_e profile, const bool long_description)
{
    switch (profile)
    {
        case PROF_VC1_SIMPLE:
            return "Simple Profile";
        case PROF_VC1_MAIN:
            return "Main Profile";
        case PROF_VC1_ADVANCED:
            return "Advanced Profile";

        case PROF_MPEG4_SP:
            return "Simple Profile";
        case PROF_MPEG4_ASP:
            return "Advanced Simple Profile";
        case PROF_MPEG4_AP:
            return "Advanced Profile";
        case PROF_MPEG4_SStP:
            return "Simple Studio Profile";

        case PROF_H262_SP:
            return "Simple Profile";
        case PROF_H262_MP:
            return "Main Profile";
        case PROF_H262_SNR:
            return "SNR Scalable Profile";
        case PROF_H262_Spatial:
            return "Spatially Scalable Profile";
        case PROF_H262_HP:
            return "High Profile";
        case PROF_H262_422:
            return "4:2:2 Profile";
        case PROF_H262_MVP:
            return "Multiview Profile";

        case PROF_H264_BP:
            return "Baseline Profile";
        case PROF_H264_CBP:
            return "Constrained Baseline Profile";
        case PROF_H264_XP:
            return "Extended Profile";
        case PROF_H264_MP:
            return "Main Profile";
        case PROF_H264_HiP:
            return "High Profile";
        case PROF_H264_PHiP:
            return "Progressive High Profile";
        case PROF_H264_CHiP:
            return "Constrained High Profile";
        case PROF_H264_Hi10P:
            return "High 10 Profile";
        case PROF_H264_PHi10P:
            return "Progressive High 10 Profile";
        case PROF_H264_Hi10IntraP:
            return "High 10 Intra Profile";
        case PROF_H264_Hi422P:
            return "High 4:2:2 Profile";
        case PROF_H264_Hi422IntraP:
            return "High 4:2:2 Intra Profile";
        case PROF_H264_Hi444P:
            return "High 4:4:4 Profile";
        case PROF_H264_Hi444PP:
            return "High 4:4:4 Predictive Profile";
        case PROF_H264_Hi444IntraP:
            return "High 4:4:4 Intra Profile";
        case PROF_H264_M444IntraP:
            return "CAVLC 4:4:4 Intra Profile";
        case PROF_H264_ScBP:
            return "Scalable Baseline Profile";
        case PROF_H264_ScCBP:
            return "Scalable Constrained Baseline Profile";
        case PROF_H264_ScHiP:
            return "Scalable High Profile";
        case PROF_H264_ScCHiP:
            return "Scalable Constrained High Profile";
        case PROF_H264_ScHiIntraP:
            return "Scalable High Intra Profile";
        case PROF_H264_StHiP:
            return "Stereo High Profile";
        case PROF_H264_MvHiP:
            return "Multiview High Profile";
        case PROF_H264_MvDHiP:
            return "Multiview Depth High Profile";
        case PROF_H264_EMvDHiP:
            return "Enhanced Multiview Depth High Profile";
        case PROF_H264_MfcHiP:
            return "MFC High Profile";
        case PROF_H264_MfcDHiP:
            return "MFC Depth High Profile";
        case PROF_H264_unknown:
            return "Unknown H.264 profile...";

        case PROF_H265_Main:
            return "Main Profile";
        case PROF_H265_Main_still:
            return "Main Still Picture Profile";
        case PROF_H265_Main10:
            return "Main 10 Profile";
        case PROF_H265_Main10_still:
            return "Main 10 Still Picture Profile";
        case PROF_H265_Main10_intra:
            return "Main 10 Intra Profile";
        case PROF_H265_Main12:
            return "Main 12 Profile";
        case PROF_H265_Main12_intra:
            return "Main 12 Intra Profile";
        case PROF_H265_Monochrome:
            return "Monochrome Profile";
        case PROF_H265_Monochrome10:
            return "Monochrome 10 Profile";
        case PROF_H265_Monochrome12:
            return "Monochrome 12 Profile";
        case PROF_H265_Monochrome12_intra:
            return "Monochrome 12 Intra Profile";
        case PROF_H265_Monochrome16:
            return "Monochrome 16 Profile";
        case PROF_H265_Monochrome16_intra:
            return "Monochrome 16 Intra Profile";
        case PROF_H265_Main422_10:
            return "Main 4:2:2 10 Profile";
        case PROF_H265_Main422_10_intra:
            return "Main 4:2:2 10 Intra Profile";
        case PROF_H265_Main422_12:
            return "Main 4:2:2 12 Profile";
        case PROF_H265_Main422_12_intra:
            return "Main 4:2:2 12 Intra Profile";
        case PROF_H265_Main444:
            return "Main 4:4:4 Profile";
        case PROF_H265_Main444_still:
            return "Main 4:4:4 Still Picture Profile";
        case PROF_H265_Main444_10:
            return "Main 4:4:4 10 Profile";
        case PROF_H265_Main444_10_intra:
            return "Main 4:4:4 10 Intra Profile";
        case PROF_H265_Main444_12:
            return "Main 4:4:4 12 Profile";
        case PROF_H265_Main444_12_intra:
            return "Main 4:4:4 12 Intra Profile";
        case PROF_H265_Main444_16_still:
            return "Main 4:4:4 16 Still Picture Profile";
        case PROF_H265_Main444_16_intra:
            return "Main 4:4:4 16 Intra Profile";
        case PROF_H265_HighThroughput_444:
            return "High Throughput 4:4:4 Profile";
        case PROF_H265_HighThroughput_444_10:
            return "High Throughput 4:4:4 10 Profile";
        case PROF_H265_HighThroughput_444_14:
            return "High Throughput 4:4:4 14 Picture Profile";
        case PROF_H265_HighThroughput_444_16_intra:
            return "High Throughput 4:4:4 16 Intra Profile";
        case PROF_H265_ScreenExtended_Main:
            return "Screen Extended Main Profile";
        case PROF_H265_ScreenExtended_Main10:
            return "Screen Extended Main 10 Profile";
        case PROF_H265_ScreenExtended_Main_444:
            return "Screen Extended Main 4:4:4 Profile";
        case PROF_H265_ScreenExtended_Main_444_10:
            return "Screen Extended Main 4:4:4 10 Profile";
        case PROF_H265_ScreenExtended_HighThroughput_444:
            return "Screen Extended Main High Throughput 4:4:4 Profile";
        case PROF_H265_ScreenExtended_HighThroughput_444_10:
            return "Screen Extended High Throughput Main 4:4:4 10 Profile";
        case PROF_H265_ScreenExtended_HighThroughput_444_14:
            return "Screen Extended High Throughput Main 4:4:4 14 Profile";
        case PROF_H265_Multiview_Main:
            return "MultiView Main Profile";
        case PROF_H265_3D_Main:
            return "3D Main Profile";
        case PROF_H265_Scalable_Main:
            return "Scalable Main Profile";
        case PROF_H265_Scalable_Main10:
            return "Scalable Main 10 Profile";
        case PROF_H265_Scalable_Main444:
            return "Scalable Main 4:4:4 Profile";
        case PROF_H265_Scalable_Monochrome:
            return "Scalable Main Profile";
        case PROF_H265_Scalable_Monochrome_12:
            return "Scalable Monochrome 12 Profile";
        case PROF_H265_Scalable_Monochrome_16:
            return "Scalable Monochrome 16 Profile";
        case PROF_H265_unknown:
            return "Unknown H.265 profile...";

        case PROF_H266_Main10:
            return "Main 10 Profile";
        case PROF_H266_Main10_still:
            return "Main 10 Still Picture Profile";
        case PROF_H266_Main10_444:
            return "Main 10 4:4:4 Profile";
        case PROF_H266_Main10_444_still:
            return "Main 10 4:4:4 Still Picture Profile";
        case PROF_H266_Main10_multilayer:
            return "Main 10 Multilayer Profile";
        case PROF_H266_Main10_444_multilayer:
            return "Main 10 4:4:4 Multilayer Profile";
        case PROF_H266_Main12:
            return "Main 12 Profile";
        case PROF_H266_Main12_intra:
            return "Main 12 Intra Profile";
        case PROF_H266_Main12_still:
            return "Main 12 Still Picture Profile";
        case PROF_H266_Main12_444:
            return "Main 12 4:4:4 Profile";
        case PROF_H266_Main12_444_intra:
            return "Main 12 4:4:4 Intr   Profile";
        case PROF_H266_Main12_444_still:
            return "Main 12 4:4:4 Still Picture Profile";
        case PROF_H266_Main16_444:
            return "Main 16 4:4:4 Profile";
        case PROF_H266_Main16_444_intra:
            return "Main 16 4:4:4 Intra Profile";
        case PROF_H266_Main16_444_still:
            return "Main 16 4:4:4 Still Picture Profile";
        case PROF_H266_unknown:
            return "Unknown H.266 profile...";

        case PROF_VP8_0:
            return "Profile 0";
        case PROF_VP8_1:
            return "Profile 1";

        case PROF_VP9_0:
            return "Profile 0";
        case PROF_VP9_1:
            return "Profile 1";
        case PROF_VP9_2:
            return "Profile 2";
        case PROF_VP9_3:
            return "Profile 3";

        case PROF_AV1_Main:
            return "Main";
        case PROF_AV1_High:
            return "High";
        case PROF_AV1_Professional:
            return "Professional";

        case PROF_AAC_LC:
            return "Low Complexity Profile";
        case PROF_AAC_Main:
            return "Main Profile";
        case PROF_AAC_SSR:
            return "Scalable Sample Rate Profile";
        case PROF_AAC_MainAudio:
            return "Main Audio Profile";
        case PROF_AAC_Scalable:
            return "Scalable Audio Profile";
        case PROF_AAC_HQ:
            return "High Quality Audio Profile";
        case PROF_AAC_LD:
            return "Low Delay Audio Profile";
        case PROF_AAC_LDv2:
            return "Low Delay AAC v2 Profile";
        case PROF_AAC_Mobile:
            return "Mobile Audio Internetworking Profile";
        case PROF_AAC_AAC:
            return "AAC Profile";
        case PROF_AAC_HE:
            return "High Efficiency AAC Profile";
        case PROF_AAC_HEv2:
            return "High Efficiency AAC Profile v2";
        case PROF_AAC_LTP:
            return "Long Term Prediction Profile";

        default:
            return "";
    }
}

/* ************************************************************************** */

const char *getScanModeString(const ScanType_e mode)
{
    switch (mode)
    {
        case SCAN_PROGRESSIVE:
            return "Progressive";
        case SCAN_INTERLACED:
            return "Interlaced scan";

        default:
            return "";
    }
}
/* ************************************************************************** */

const char *getStereoModeString(const StereoMode_e mode)
{
    switch (mode)
    {
        case MONOSCOPIC:
            return "Monoscopic";

        case STEREO_ANAGLYPH_CR:
            return "Anaglyph (cyan/red)";
        case STEREO_ANAGLYPH_GM:
            return "Anaglyph (green/magenta)";

        case STEREO_SIDEBYSIDE_LEFT:
            return "Side by side (left eye is first)";
        case STEREO_SIDEBYSIDE_RIGHT:
            return "Side by side (right eye is first)";
        case STEREO_TOPBOTTOM_LEFT:
            return "Top bottom (left eye is first)";
        case STEREO_TOPBOTTOM_RIGHT:
            return "Top bottom (right eye is first)";
        case STEREO_CHECKBOARD_LEFT:
            return "Checkerboard (left eye is first)";
        case STEREO_CHECKBOARD_RIGHT:
            return "Checkerboard (right eye is first)";
        case STEREO_ROWINTERLEAVED_LEFT:
            return "Row interleaved (left eye is first)";
        case STEREO_ROWINTERLEAVED_RIGHT:
            return "Row interleaved (right eye is first)";
        case STEREO_COLUMNINTERLEAVED_LEFT:
            return "Column interleaved (left eye is first)";
        case STEREO_COLUMNINTERLEAVED_RIGHT:
            return "Column interleaved (right eye is first)";

        default:
            return "";
    }
}

/* ************************************************************************** */

const char *getHdrModeString(const HdrMode_e mode)
{
    switch (mode)
    {
        case SDR:
            return "SDR";

        case HLG:
            return "HLG";
        case HDR10:
            return "HDR10";
        case HDR10plus:
            return "HDR10+";
        case DolbyVision:
            return "Dolby Vision";

        default:
            return "";
    }
}

/* ************************************************************************** */

const char *getChromaSubsamplingString(const ChromaSubSampling_e subsampling)
{
    switch (subsampling)
    {
        case CHROMA_SS_311:
            return "3:1:1";
        case CHROMA_SS_400:
            return "4:0:0";
        case CHROMA_SS_410:
            return "4:1:0";
        case CHROMA_SS_411:
            return "4:1:1";
        case CHROMA_SS_420:
            return "4:2:0";
        case CHROMA_SS_421:
            return "4:2:1";
        case CHROMA_SS_422:
            return "4:2:2";
        case CHROMA_SS_440:
            return "4:4:0";
        case CHROMA_SS_441:
            return "4:4:1";
        case CHROMA_SS_442:
            return "4:4:2";
        case CHROMA_SS_444:
            return "4:4:4";

        default:
            return "";
    }
}

const char *getChromaLocationString(const ChromaLocation_e location)
{
    switch (location)
    {
        case CHROMA_LOC_LEFT:
            return "Left";
        case CHROMA_LOC_CENTER:
            return "Center";
        case CHROMA_LOC_TOPLEFT:
            return "Top left";
        case CHROMA_LOC_TOP:
            return "Top";
        case CHROMA_LOC_BOTTOMLEFT:
            return "Bottom left";
        case CHROMA_LOC_BOTTOM:
            return "Bottom";

        default:
            return "";
    }
}

/* ************************************************************************** */

const char *getColorPrimariesString(const ColorPrimaries_e primaries)
{
    switch (primaries)
    {
        case COLOR_PRI_BT709:
            return "Rec. 709";
        case COLOR_PRI_BT470M:
            return "BT470M";
        case COLOR_PRI_BT470BG:
            return "BT470BG";
        case COLOR_PRI_SMPTE170M:
            return "SMPTE 170M";
        case COLOR_PRI_SMPTE240M:
            return "SMPTE 240M";
        case COLOR_PRI_FILM:
            return "FILM";
        case COLOR_PRI_BT2020:
            return "Rec. 2020";
        case COLOR_PRI_SMPTE428:
            return "SMPTE 428";
        case COLOR_PRI_SMPTE431:
            return "SMPTE 431";
        case COLOR_PRI_SMPTE432:
            return "SMPTE 432";
        case COLOR_PRI_JEDEC_P22:
            return "JEDEC P22";

        default:
            return "";
    }
}

const char *getColorTransferCharacteristicString(const ColorTransferCharacteristic_e transfer)
{
    switch (transfer)
    {
        case COLOR_TRC_BT709:
            return "Rec. 709";
        case COLOR_TRC_GAMMA22:
            return "GAMMA 22";
        case COLOR_TRC_GAMMA28:
            return "GAMMA 28";
        case COLOR_TRC_SMPTE170M:
            return "SMPTE 170M";
        case COLOR_TRC_SMPTE240M:
            return "SMPTE 240M";
        case COLOR_TRC_LINEAR:
            return "LINEAR";
        case COLOR_TRC_LOG:
            return "LOG";
        case COLOR_TRC_LOG_SQRT:
            return "LOG SQRT";
        case COLOR_TRC_IEC61966_2_4:
            return "IEC 61966_2_4";
        case COLOR_TRC_BT1361_ECG:
            return "BT 1361_ECG";
        case COLOR_TRC_IEC61966_2_1:
            return "IEC 61966_2_1";
        case COLOR_TRC_BT2020_10:
            return "BT 2020_1";
        case COLOR_TRC_BT2020_12:
            return "BT 2020_12";
        case COLOR_TRC_SMPTE2084:
            return "SMPTE 2084";
        case COLOR_TRC_SMPTE428:
            return "SMPTE 428";
        case COLOR_TRC_ARIB_STD_B67:
            return "ARIB_STD_B6";

        default:
            return "";
    }
}

const char *getColorMatrixString(const ColorSpace_e space)
{
    switch (space)
    {
        case COLOR_SPC_RGB:
            return "RGB";
        case COLOR_SPC_BT709:
            return "Rec. 709";
        case COLOR_SPC_FCC:
            return "FCC";
        case COLOR_SPC_BT470BG:
            return "BT 470BG";
        case COLOR_SPC_SMPTE170M:
            return "SMPTE 170M";
        case COLOR_SPC_SMPTE240M:
            return "SMPTE 240M";
        case COLOR_SPC_YCGCO:
            return "YCgCo";
        case COLOR_SPC_BT2020_NCL:
            return "YCgCo";
        case COLOR_SPC_BT2020_CL:
            return "YCgCo";
        case COLOR_SPC_SMPTE2085:
            return "SMPTE 2085";
        case COLOR_SPC_CHROMA_DERIVED_NCL:
            return "CHROMA_DERIVED_NCL";
        case COLOR_SPC_CHROMA_DERIVED_CL:
            return "CHROMA_DERIVED_CL";
        case COLOR_SPC_ICTCP:
            return "ICtCp";

        default:
            return "";
    }
}

/* ************************************************************************** */
