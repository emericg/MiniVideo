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
 * \file      minivideo_containers.cpp
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
    if (long_description)
    {
        switch (container)
        {
        case CONTAINER_AVI:
            return "AVI (Audio Video Interleave) [.avi, ...]";
            break;
        case CONTAINER_ASF:
            return "ASF (Advanced Systems Format) [.asf, .wma, .wmv]";
            break;
        case CONTAINER_MKV:
            return "MKV (Matroska) [.mkv, .mka, .webm]";
            break;
        case CONTAINER_MP4:
            return "MP4 (ISO Base Media format) [.mov, .mp4, .3gp, .f4v, ...]";
            break;
        case CONTAINER_MPEG_PS:
            return "MPEG 'Program Stream' [.mpg, .vob, ...]";
            break;
        case CONTAINER_MPEG_TS:
            return "MPEG 'Transport Stream' [.ts, .mts, .m2ts, ...]";
            break;
        case CONTAINER_MPEG_MT:
            return "MPEG 'Media Transport'";
            break;
        case CONTAINER_MXF:
            return "MXF Material eXchange Format [.mxf]";
            break;
        case CONTAINER_FLV:
            return "SWF Small Web Format [.swf, .flv]";
            break;
        case CONTAINER_OGG:
            return "OGG [.ogg, .ogv, ...]";
            break;
        case CONTAINER_RM:
            return "RealMedia [.rm, .rmvb]";
            break;
        case CONTAINER_R3D:
            return "Redcode RAW [.r3d]";
            break;

        case CONTAINER_FLAC:
            return "FLAC (Free Lossless Audio Codec) [.flac]";
            break;
        case CONTAINER_WAVE:
            return "WAVE (Waveform Audio File Format) [.wav]";
            break;
        case CONTAINER_CAF:
            return "CAF (Core Audio Format) [.caf]";
            break;

        case CONTAINER_ES:
            return "Undefined 'Elementary Stream'";
            break;
        case CONTAINER_ES_AAC:
            return "AAC 'Elementary Stream'";
            break;
        case CONTAINER_ES_AC3:
            return "AC3 'Elementary Stream'";
            break;
        case CONTAINER_ES_MP3:
            return "MP3 'Elementary Stream'";
            break;

        default:
        case CONTAINER_UNKNOWN:
            return "UNKNOWN";
            break;
        }
    }
    else // short description
    {
        switch (container)
        {
            case CONTAINER_AVI:
                return "AVI";
                break;
            case CONTAINER_ASF:
                return "ASF";
                break;
            case CONTAINER_MKV:
                return "MKV";
                break;
            case CONTAINER_MP4:
                return "MP4";
                break;
            case CONTAINER_MPEG_PS:
                return "MPEG-PS";
                break;
            case CONTAINER_MPEG_TS:
                return "MPEG-TS";
                break;
            case CONTAINER_MPEG_MT:
                return "MPEG-MT";
                break;
            case CONTAINER_MXF:
                return "MXF";
                break;
            case CONTAINER_FLV:
                return "FLV";
                break;
            case CONTAINER_OGG:
                return "OGG";
                break;
            case CONTAINER_RM:
                return "RM";
                break;
            case CONTAINER_R3D:
                return "R3D";
                break;

            case CONTAINER_FLAC:
                return "FLAC";
                break;
            case CONTAINER_WAVE:
                return "WAVE";
                break;
            case CONTAINER_AIFF:
                return "AIFF";
                break;
            case CONTAINER_CAF:
                return "CAF";
                break;

            case CONTAINER_ES:
                return "ES";
                break;
            case CONTAINER_ES_AAC:
                return "AAC ES";
                break;
            case CONTAINER_ES_AC3:
                return "AC3 ES";
                break;
            case CONTAINER_ES_MP3:
                return "MP3 ES";
                break;

            default:
            case CONTAINER_UNKNOWN:
                return "UNKNOWN";
                break;
        }
    }
}

/* ************************************************************************** */

const char *getContainerProfileString(const ContainerProfiles_e profile, const bool long_description)
{
    switch (profile)
    {
        case PROF_AVI_OpenDML:
            return "AVI v2 \"OpenDML\"";
            break;

        case PROF_MPEG_PS_1:
            return "MPEG-PS v1";
            break;
        case PROF_MPEG_PS_2:
            return "MPEG-PS v2";
            break;

        case PROF_ISOBMF_MOV:
            return "QuickTime File Format";
            break;
        case PROF_ISOBMF_MP4:
            return "ISO BMF MP4";
            break;
        case PROF_ISOBMF_3GP:
            return "ISO BMF 3GP";
            break;
        case PROF_ISOBMF_MJP:
            return "ISO BMF Motion Jpeg 2000";
            break;

        case PROF_WAVE_AMB:
            return "WAVE ambisonic";
            break;
        case PROF_WAVE_RF64:
            return "RF64";
            break;
        case PROF_WAVE_BWFv1:
            return "Broadcast Wave Format v1";
            break;
        case PROF_WAVE_BWFv2:
            return "Broadcast Wave Format v2";
            break;
        case PROF_WAVE_BWF64:
            return "Broadcast Wave Format 64";
            break;

        case PROF_MKV_MATROSKA:
            return "Matroska";
            break;
        case PROF_MKV_WEBM:
            return "WebM";
            break;

        case PROF_UNKNOWN:
        default:
            return "UNKNOWN";
            break;
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
            TRACE_1(IO, "* File type      : AU container detected");
            container = CONTAINER_AU;
        }
        else if (buffer[0] == 0xFF &&
                 (buffer[1] == 0xFE || buffer[1] == 0xFD ||buffer[1] == 0xFB))
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
Containers_e getContainerUsingExtension(std::string ext)
{
    TRACE_2(IO, "getContainerUsingExtension()");

    // Set container to unknown
    Containers_e container = CONTAINER_UNKNOWN;

    if (ext.size() > 0 && ext.size() < 16)
    {
        if (ext == "avi" || ext == "divx")
        {
            TRACE_1(IO, "* File extension  : AVI container detected");
            container = CONTAINER_AVI;
        }
        else if (ext == "webm" ||
                 ext == "mkv" || ext == "mka" || ext == "mks" || ext == "mk3d")
        {
            TRACE_1(IO, "* File extension  : MKV container detected");
            container = CONTAINER_MKV;
        }
        else if (ext == "mov" || ext == "qt" || // QuickTime file format
                 ext == "mp4" || // ISO Base Media file format
                 ext == "m4v" || ext == "m4a" || ext == "m4p" || ext == "m4b" ||
                 ext == "m4s" || ext == "mp4v" || ext == "mp4a" ||
                 ext == "3gp" || ext == "3g2" || ext == "3gpp" || // ISO BMF / 3GP profile
                 ext == "f4v" || // ISO BMF / Flash Video
                 ext == "mj2" ||  ext == "mjp2") // ISO BMF / Motion JPEG2000 profile
        {
            TRACE_1(IO, "* File extension  : ISO BMF (MOV,MP4, ...) container detected");
            container = CONTAINER_MP4;
        }
        else if (ext == "ps"  || ext == "vob" || ext == "evo" ||
                 ext == "m2p" || ext == "m2v" || ext == "mpg" || ext == "mpeg")
        {
            TRACE_1(IO, "* File extension  : PS (MPEG 'Program Stream') container detected");
            container = CONTAINER_MPEG_PS;
        }
        else if (ext == "ts"  || ext == "trp" ||
                 ext == "mts" || ext == "m2ts")
        {
            TRACE_1(IO, "* File extension  : TS (MPEG 'Transport Stream') container detected");
            container = CONTAINER_MPEG_TS;
        }
        else if (ext == "asf" ||
                 ext == "wma" || ext == "wmv")
        {
            TRACE_1(IO, "* File extension  : ASF container detected");
            container = CONTAINER_ASF;
        }
        else if (ext == "ogg" || ext == "ogv" || ext == "oga" ||
                 ext == "ogx" || ext == "ogm" || ext == "spx" ||
                 ext == "opus")
        {
            TRACE_1(IO, "* File extension  : OGG container detected");
            container = CONTAINER_OGG;
        }
        else if (ext == "mxf")
        {
            TRACE_1(IO, "* File extension  : MXF container detected");
            container = CONTAINER_MXF;
        }
        else if (ext == "flac")
        {
            TRACE_1(IO, "* File extension  : FLAC container detected");
            container = CONTAINER_FLAC;
        }
        else if (ext == "wav" || ext == "wave" ||
                 ext == "amb")
        {
            TRACE_1(IO, "* File extension  : WAVE container detected");
            container = CONTAINER_WAVE;
        }
        else if (ext == "aiff" || ext == "aif" || ext == "aifc")
        {
            TRACE_1(IO, "* File extension  : AIFF container detected");
            container = CONTAINER_AIFF;
        }
        else if (ext == "caf")
        {
            TRACE_1(IO, "* File extension  : CAF container detected");
            container = CONTAINER_CAF;
        }
        else if (ext == "au" || ext == "snd")
        {
            TRACE_1(IO, "* File extension  : AU container detected");
            container = CONTAINER_AU;
        }
        else if (ext == "flv" ||
                 ext == "f4v" || ext == "f4p" || ext == "f4a" || ext == "f4b")
        {
            TRACE_1(IO, "* File extension  : FLV container detected");
            container = CONTAINER_FLV;
        }
        else if (ext == "rm" || ext == "rmvb")
        {
            TRACE_1(IO, "* File extension  : RealMedia container detected");
            container = CONTAINER_RM;
        }
        else if (ext == "264" || ext == "h264")
        {
            TRACE_1(IO, "* File extension  : H.264 ES detected");
            container = CONTAINER_ES;
            //codec = CODEC_H264;
        }
        else if (ext == "265" || ext == "h265")
        {
            TRACE_1(IO, "* File extension  : H.265 ES detected");
            container = CONTAINER_ES;
            //codec = CODEC_H265;
        }
        else if (ext == "aac")
        {
            TRACE_1(IO, "* File extension  : AAC ES detected");
            container = CONTAINER_ES_AAC;
            //codec = CODEC_AAC;
        }
        else if (ext == "ac3")
        {
            TRACE_1(IO, "* File extension  : AC3 ES detected");
            container = CONTAINER_ES_AC3;
            //codec = CODEC_AC3;
        }
        else if (ext == "mp1" || ext == "mp2" || ext == "mp3")
        {
            TRACE_1(IO, "* File extension  : MP3 ES detected");
            container = CONTAINER_ES_MP3;
            //codec = CODEC_MP3;
        }
    }
    else
    {
        TRACE_ERROR(IO, "Container: no extension provided...");
    }

    return container;
}

/* ************************************************************************** */
