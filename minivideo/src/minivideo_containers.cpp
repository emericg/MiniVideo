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
 * \date      2018
 */

#include "minivideo_containers.h"
#include "minitraces.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wswitch"
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wswitch"
#endif

/* ************************************************************************** */

const char *getContainerString(const Containers_e container, const bool long_description)
{
    switch (container)
    {
        case CONTAINER_AVI:
            if (long_description)
                return "AVI (Audio Video Interleave)";
            else
                return "AVI";

        case CONTAINER_ASF:
            if (long_description)
                return "ASF (Advanced Systems Format)";
            else
                return "ASF";

        case CONTAINER_MKV:
            if (long_description)
                return "MKV (Matroska)";
            else
                return "MKV";

        case CONTAINER_MP4:
            if (long_description)
                return "MP4 (ISO Base Media format)";
            else
                return "MP4";

        case CONTAINER_MPEG_PS:
            if (long_description)
                return "MPEG 'Program Stream'";
            else
                return "MPEG-PS";
        case CONTAINER_MPEG_TS:
            if (long_description)
                return "MPEG 'Transport Stream'";
            else
                return "MPEG-TS";
        case CONTAINER_MPEG_MT:
            if (long_description)
                return "MPEG 'Media Transport'";
            else
                return "MPEG-MT";

        case CONTAINER_MXF:
            if (long_description)
                return "MXF Material eXchange Format";
            else
                return "MXF";

        case CONTAINER_FLV:
            if (long_description)
                return "Flash Video file format";
            else
                return "FLV";

        case CONTAINER_SWF:
            if (long_description)
                return "SWF (Small Web Format)";
            else
                return "SWF";

        case CONTAINER_OGG:
            return "OGG";

        case CONTAINER_RM:
            return "RealMedia";

        case CONTAINER_R3D:
            return "Redcode RAW";

        case CONTAINER_FLAC:
            if (long_description)
                return "FLAC (Free Lossless Audio Codec)";
            else
                return "FLAC";

        case CONTAINER_WAVE:
            if (long_description)
                return "WAVE (Waveform Audio File Format)";
            else
                return "WAVE";

        case CONTAINER_CAF:
            if (long_description)
                return "CAF (Core Audio Format)";
            else
                return "CAF";

        case CONTAINER_AU:
            if (long_description)
                return "AU (Au file format)";
            else
                return "AU";

        case CONTAINER_ES:
            return "Undefined 'Elementary Stream'";
        case CONTAINER_ES_AAC:
            return "AAC 'Elementary Stream'";
        case CONTAINER_ES_AC3:
            return "AC3 'Elementary Stream'";
        case CONTAINER_ES_MP3:
            return "MP3 'Elementary Stream'";

        case CONTAINER_JPEG:
            return "JPEG";
        case CONTAINER_JPEG_XR:
            return "JPEG XR";
        case CONTAINER_JPEG_2K:
            return "JPEG 2000";
        case CONTAINER_GIF:
            return "GIF";
        case CONTAINER_BMP:
            return "BMP";
        case CONTAINER_TGA:
            return "TGA";
        case CONTAINER_TIFF:
            return "TIFF";
        case CONTAINER_ICNS:
            return "ICNS";
        case CONTAINER_ICO:
            return "ICO";
        case CONTAINER_PNG:
            return "PNG";
        case CONTAINER_DNG:
            return "DNG";
        case CONTAINER_DPX:
            return "DPX";
        case CONTAINER_CIFF:
            return "CIFF";
        case CONTAINER_CR2:
            return "CR2";
        case CONTAINER_WEBP:
            return "WebP";
        case CONTAINER_BPG:
            return "BPG";
        case CONTAINER_FLIF:
            return "FLIF";

        default:
            return "";
    }
}

/* ************************************************************************** */

const char *getContainerProfileString(const ContainerProfiles_e profile, const bool long_description)
{
    switch (profile)
    {
        case PROF_AVI:
            return "AVI v1";
        case PROF_AVI_OpenDML:
            return "AVI v2 \"OpenDML\"";
        case PROF_AVI_DMF:
            return "AVI DMF \"DivX Media Format\"";

        case PROF_MPEG_PS_1:
            return "MPEG-PS v1";
        case PROF_MPEG_PS_2:
            return "MPEG-PS v2";

        case PROF_ISOBMF_MOV:
            return "QuickTime File Format";
        case PROF_ISOBMF_MP4:
            return "ISO BMF MP4";
        case PROF_ISOBMF_3GP:
            return "ISO BMF 3GP";
        case PROF_ISOBMF_MJP:
            return "ISO BMF Motion Jpeg 2000";
        case PROF_ISOBMF_HEIF:
            return "High Efficiency Image File Format";
        case PROF_ISOBMF_AVIF:
            return "AV1 Image File Format";

        case PROF_MKV_MATROSKA:
            return "Matroska";
        case PROF_MKV_WEBM:
            return "WebM";

        case PROF_WAVE_AMB:
            return "WAVE ambisonic";
        case PROF_WAVE_RF64:
            return "RF64";
        case PROF_WAVE_BWFv1:
            return "Broadcast Wave Format v1";
        case PROF_WAVE_BWFv2:
            return "Broadcast Wave Format v2";
        case PROF_WAVE_BWF64:
            return "Broadcast Wave Format 64";

        default:
            return "";
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Detect the container used by a multimedia file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \return container: A ContainerFormat_e value.
 *
 * This function check the first 16 bytes of a media file in order to find
 * evidence of a known file container format.
 */
Containers_e getContainerUsingStartcodes(uint8_t buffer[16])
{
    TRACE_2(IO, "getContainerUsingStartcodes()");

    Containers_e container = CONTAINER_UNKNOWN;

    // Read the first bytes of the file
    if (buffer)
    {
        // Parse the file to find evidence of a container format
        if (buffer[0] == 0x47)
        {
            TRACE_1(IO, "* File type      : TS (MPEG 'Transport Stream') container detected");
            container = CONTAINER_MPEG_TS;
        }
        else if (buffer[0] == 0x1A && buffer[1] == 0x45 && buffer[2] == 0xDF && buffer[3] == 0xA3)
        {
            TRACE_1(IO, "* File type      : EBML file detected, possibly MKV or WebM container");
            container = CONTAINER_MKV;
        }
        else if (buffer[0] == 0x52 && buffer[1] == 0x49 && buffer[2] == 0x46 && buffer[3] == 0x46)
        {
            // RIFF header:
            // 52 49 46 46 xx xx xx xx // R I F F (size)
            // then:
            // 41 56 49 20 4C 49 53 54 // A V I   L I S T
            // 57 41 56 45 66 6D 74 20 // W A V E f m t

            if (buffer[8] == 0x41 && buffer[9] == 0x56 && buffer[10] == 0x49 && buffer[11] == 0x20)
            {
                TRACE_1(IO, "* File type      : AVI container detected");
                container = CONTAINER_AVI;
            }
            else if (buffer[8] == 0x57 && buffer[9] == 0x41 && buffer[10] == 0x56 && buffer[11] == 0x45)
            {
                TRACE_1(IO, "* File type      : WAVE container detected");
                container = CONTAINER_WAVE;
            }
            else
            {
                TRACE_WARNING(IO, "* File type      : Unknown RIFF container detected");
            }
        }
        else if (buffer[0] == 0x00 && buffer[1] == 0x00)
        {
            if (buffer[2] == 0x01)
            {
                if (buffer[3] == 0xBA)
                {
                    TRACE_1(IO, "* File type      : PS (MPEG 'Program Stream') container detected");
                    container = CONTAINER_MPEG_PS;
                }
                else if (buffer[3] == 0xB3)
                {
                    TRACE_1(IO, "* File type      : MPEG-1/2 / H.262 Elementary Stream detected (raw video datas)");
                    container = CONTAINER_ES;
                    //media->codec_video = CODEC_MPEG12;
                }
                else if (buffer[3] == 0x67)
                {
                    TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (raw video datas)");
                    container = CONTAINER_ES;
                    //media->codec_video = CODEC_H264;
                }
            }
            else if (buffer[2] == 0x00 && buffer[3] == 0x01)
            {
                if (buffer[4] == 0xBA)
                {
                    TRACE_1(IO, "* File type      : MPEG-PS (MPEG 'Program Stream') container detected");
                    container = CONTAINER_MPEG_PS;
                }
                else if (buffer[4] == 0xB3)
                {
                    TRACE_1(IO, "* File type      : MPEG-1/2 / H.262 Elementary Stream detected (raw video datas)");
                    container = CONTAINER_ES;
                    //media->codec_video = CODEC_MPEG12;
                }
                else if (buffer[4] == 0x67)
                {
                    TRACE_1(IO, "* File type      : H.264 'Annex B' Elementary Stream detected (raw video datas)");
                    container = CONTAINER_ES;
                    //media->codec_video = CODEC_H264;
                }
            }

            if (buffer[4] == 0x66 && buffer[5] == 0x74 && buffer[6] == 0x79 && buffer[7] == 0x70)
            {
                // MP4: 00 00 xx xx 66 74 79 70 // (size) f t y p

                TRACE_1(IO, "* File type      : ISO BMF (MOV,MP4, ...) container detected");
                container = CONTAINER_MP4;
            }
            else if (buffer[4] == 0x73 && buffer[5] == 0x74 && buffer[6] == 0x79 && buffer[7] == 0x70)
            {
                // MP4: 00 00 xx xx 73 74 79 70 // (size) s t y p

                TRACE_1(IO, "* File type      : ISO BMF (MOV,MP4, ...) container detected");
                container = CONTAINER_MP4;
            }
        }
        else if (buffer[0] == 0x30 && buffer[1] == 0x26 && buffer[2] == 0xB2 && buffer[3] == 0x75)
        {
            TRACE_1(IO, "* File type      : ASF container detected");
            container = CONTAINER_ASF;
        }
        else if (buffer[0] == 0x4F && buffer[1] == 0x67 && buffer[2] == 0x67 && buffer[3] == 0x53)
        {
            TRACE_1(IO, "* File type      : OGG container detected");
            container = CONTAINER_OGG;
        }
        else if (buffer[0] == 0x66 && buffer[1] == 0x61 && buffer[2] == 0x4C && buffer[3] == 0x43)
        {
            TRACE_1(IO, "* File type      : FLAC container detected");
            container = CONTAINER_FLAC;
        }
        else if (buffer[0] == 0x06 && buffer[1] == 0x0E && buffer[2] == 0x2B && buffer[3] == 0x34)
        {
            TRACE_1(IO, "* File type      : MXF container detected");
            container = CONTAINER_MXF;
        }
        else if (buffer[0] == 0x46 && buffer[1] == 0x4C && buffer[2] == 0x56 && buffer[3] == 0x01)
        {
            TRACE_1(IO, "* File type      : FLV container detected");
            container = CONTAINER_FLV;
        }
        else if (buffer[0] == 0x63 && buffer[1] == 0x61 && buffer[2] == 0x66 && buffer[3] == 0x66)
        {
            TRACE_1(IO, "* File type      : CAF container detected");
            container = CONTAINER_CAF;
        }
        else if (buffer[0] == 0x2E && buffer[1] == 0x73 && buffer[2] == 0x6E && buffer[3] == 0x64)
        {
            // AU magic number:  2E 73 6E 64 // . s n d
            TRACE_1(IO, "* File type      : AU container detected");
            container = CONTAINER_AU;
        }
        else if (buffer[0] == 0xFF &&
                 (buffer[1] == 0xFE || buffer[1] == 0xFD || buffer[1] == 0xFB))
        {
            TRACE_1(IO, "* File type      : MP1/2/3 Elementary Stream detected");
            container = CONTAINER_ES_MP3;
        }
    }
    else
    {
        TRACE_ERROR(IO, "Container: no file header data provided...");
    }

    return container;
}

/*!
 * \brief Detect the container used by a multimedia file.
 * \param[in] *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \return container: A ContainerFormat_e value.
 *
 * This function check the file extension to guess the container. As this method
 * is *definitely* not reliable (file extensions are set by users, and users
 * make mistakes) this code is here mostly for fun. And to list extensions for
 * whoever is interested.
 */
Containers_e getContainerUsingExtension(const std::string &ext)
{
    TRACE_2(IO, "getContainerUsingExtension()");

    // Set container to unknown
    Containers_e container = CONTAINER_UNKNOWN;

    if (ext.size() > 0 && ext.size() < 8)
    {
        if (ext == "mov" || ext == "qt" || // QuickTime file format
            ext == "mp4" || // ISO Base Media file format
            ext == "m4v" || ext == "m4a" || ext == "m4p" || ext == "m4b" ||
            ext == "m4s" || ext == "mp4v" || ext == "mp4a" ||
            ext == "3gp" || ext == "3g2" || ext == "3gpp" || // ISO BMF / 3GP profile
            ext == "f4v" || ext == "f4p" || ext == "f4a" || ext == "f4b" || // ISO BMF / Flash Video
            ext == "mj2" ||  ext == "mjp2") // ISO BMF / Motion JPEG2000 profile
        {
            container = CONTAINER_MP4;
        }
        else if (ext == "webm" ||
                 ext == "mkv" || ext == "mka" || ext == "mks" || ext == "mk3d")
        {
            container = CONTAINER_MKV;
        }
        else if (ext == "avi" || ext == "divx")
        {
            container = CONTAINER_AVI;
        }
        else if (ext == "asf" || ext == "wma" || ext == "wmv")
        {
            container = CONTAINER_ASF;
        }
        else if (ext == "ps"  || ext == "vob" || ext == "evo" ||
                 ext == "m2p" || ext == "m2v" || ext == "mpg" || ext == "mpeg")
        {
            container = CONTAINER_MPEG_PS;
        }
        else if (ext == "ts"  || ext == "trp" ||
                 ext == "mts" || ext == "m2ts")
        {
            container = CONTAINER_MPEG_TS;
        }
        else if (ext == "mt"  || ext == "mmt")
        {
            container = CONTAINER_MPEG_MT;
        }
        else if (ext == "ogg" || ext == "ogv" || ext == "oga" ||
                 ext == "ogx" || ext == "ogm" || ext == "spx" ||
                 ext == "opus")
        {
            container = CONTAINER_OGG;
        }
        else if (ext == "mxf")
        {
            container = CONTAINER_MXF;
        }
        else if (ext == "flv" || ext == "f4v" || ext == "f4p" || ext == "f4a" || ext == "f4b")
        {
            container = CONTAINER_FLV;
        }
        else if (ext == "swf")
        {
            container = CONTAINER_SWF;
        }
        else if (ext == "rm" || ext == "rmvb")
        {
            container = CONTAINER_RM;
        }
        else if (ext == "r3d")
        {
            container = CONTAINER_R3D;
        }

        else if (ext == "flac")
        {
            container = CONTAINER_FLAC;
        }
        else if (ext == "wav" || ext == "wave" || ext == "amb")
        {
            container = CONTAINER_WAVE;
        }
        else if (ext == "aiff" || ext == "aif" || ext == "aifc")
        {
            container = CONTAINER_AIFF;
        }
        else if (ext == "caf")
        {
            container = CONTAINER_CAF;
        }
        else if (ext == "au" || ext == "snd")
        {
            container = CONTAINER_AU;
        }

        else if (ext == "264" || ext == "h264")
        {
            container = CONTAINER_ES;
            //codec = CODEC_H264;
        }
        else if (ext == "265" || ext == "h265")
        {
            container = CONTAINER_ES;
            //codec = CODEC_H265;
        }
        else if (ext == "aac")
        {
            container = CONTAINER_ES_AAC;
            //codec = CODEC_AAC;
        }
        else if (ext == "ac3")
        {
            container = CONTAINER_ES_AC3;
            //codec = CODEC_AC3;
        }
        else if (ext == "mp1" || ext == "mp2" || ext == "mp3")
        {
            container = CONTAINER_ES_MP3;
            //codec = CODEC_MP3;
        }

        else if (ext == "jpg" || ext == "jpeg" || ext == "jpe" ||
                 ext == "jif" || ext == "jfif" || ext == "jfi")
        {
            container = CONTAINER_JPEG;
            //codec = CODEC_JPEG;
        }
        else if (ext == "jxr" || ext == "hdp" || ext == "wdp")
        {
            container = CONTAINER_JPEG_XR;
            //codec = CODEC_JPEG_XR;
        }
        else if (ext == "jp2" || ext == "j2k" || ext == "jpf" ||
                 ext == "jpx" || ext == "jpm" || ext == "mj2")
        {
            container = CONTAINER_JPEG_2K;
            //codec = CODEC_JPEG_2K;
        }
        else if (ext == "gif")
        {
            container = CONTAINER_GIF;
            //codec = CODEC_GIF;
        }
        else if (ext == "bmp" || ext == "dib")
        {
            container = CONTAINER_BMP;
            //codec = CODEC_BMP;
        }
        else if (ext == "tga" || ext == "icb" || ext == "vda" || ext == "vst")
        {
            container = CONTAINER_TGA;
            //codec = CODEC_TGA;
        }
        else if (ext == "tif" || ext == "tiff")
        {
            container = CONTAINER_TIFF;
            //codec = CODEC_TIFF;
        }
        else if (ext == "ico")
        {
            container = CONTAINER_ICO;
        }
        else if (ext == "icns")
        {
            container = CONTAINER_ICNS;
        }
        else if (ext == "png")
        {
            container = CONTAINER_PNG;
            //codec = CODEC_PNG;
        }
        else if (ext == "dng")
        {
            container = CONTAINER_DNG;
            //codec = CODEC_DNG;
        }
        else if (ext == "dpx")
        {
            container = CONTAINER_DPX;
        }
        else if (ext == "crw")
        {
            container = CONTAINER_CIFF;
        }
        else if (ext == "cr2")
        {
            container = CONTAINER_CR2;
        }
        else if (ext == "webp")
        {
            container = CONTAINER_WEBP;
            //codec = CODEC_VP8;
        }
        else if (ext == "bpg")
        {
            container = CONTAINER_BPG;
            //codec = CODEC_H265;
        }
        else if (ext == "flif")
        {
            container = CONTAINER_FLIF;
            //codec = CODEC_FLIF16;
        }
        else if (ext == "heif" || ext == "heic")
        {
            container = CONTAINER_MP4;
            //codec = CODEC_H265;
        }
        else if (ext == "avif")
        {
            container = CONTAINER_MP4;
            //codec = CODEC_AV1;
        }

        TRACE_1(IO, "* File extension : '%s' detected", getContainerString(container));
    }
    else
    {
        TRACE_ERROR(IO, "Container: no extension provided...");
    }

    return container;
}

/* ************************************************************************** */
