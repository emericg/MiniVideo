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
 * \file      mp4_stsd.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "mp4_stsd.h"
#include "mp4_spatial.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

#include "../../decoder/h264/h264_nalu.h"
#include "../../decoder/h264/h264_parameterset.h"
#include "../../decoder/h265/h265_nalu.h"
//#include "../../decoder/h265/h265_parameterset.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Sample Description Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box.
 *
 * The SampleDescriptionBox contains information about codec types and some
 * initialization parameters needed to start decoding.
 */
int parse_stsd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse stsd box content
    unsigned int entry_count = read_bits(bitstr, 32);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", entry_count);

    // Parse subbox header (> SampleEntry)
    for (unsigned int i = 0; i < entry_count; i++)
    {
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);
        track->fcc = box_subheader.boxtype; // save fourcc as backup

        // Parse subbox content (> SampleEntry)
        unsigned int reserved[6] = {0};
        for (int j = 0; j < 6; j++)
        {
            reserved[j] = read_bits(bitstr, 8);
        }
        unsigned int data_reference_index = read_bits(bitstr, 16);

        write_box_header(&box_subheader, mp4->xml);
        if (mp4->xml) fprintf(mp4->xml, "  <data_reference_index>%u</data_reference_index>\n", data_reference_index);

        // Parse subbox content (> SampleEntry extensions)
        switch (track->handlerType)
        {
        case MP4_HANDLER_AUDIO:
            parse_stsd_audio(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_VIDEO:
            parse_stsd_video(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_TMCD:
            parse_stsd_tmcd(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_TEXT:
            parse_stsd_text(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_META:
            parse_stsd_meta(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_HINT:
            parse_stsd_hint(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_SDSM:
            parse_stsd_sdsm(bitstr, &box_subheader, track, mp4);
            break;

        case MP4_HANDLER_ODSM:
            parse_stsd_odsm(bitstr, &box_subheader, track, mp4);
            break;

        default:
            TRACE_1(MP4, "Unknown track type, skipped...");
            break;
        }

        if (mp4->xml) fprintf(mp4->xml, "  </a>\n");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_stsd_audio(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_audio()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // AudioSampleEntry
    {
        if (box_header->boxtype == fcc_AC3 ||
            box_header->boxtype == fcc_ac3)
        {
            track->codec = CODEC_AC3;
            TRACE_1(MP4, "> Audio track is using AC3 codec");
        }
        if (box_header->boxtype == fcc_ec3)
        {
            track->codec = CODEC_EAC3;
            TRACE_1(MP4, "> Audio track is using Enhanced AC-3 codec");
        }
        else if (box_header->boxtype == fcc_AC4 ||
                 box_header->boxtype == fcc_ac4)
        {
            track->codec = CODEC_AC4;
            TRACE_1(MP4, "> Audio track is using AC4 codec");
        }
        else if (box_header->boxtype == fcc_sowt)
        {
            track->codec = CODEC_LPCM;
            TRACE_1(MP4, "> Audio track is using PCM audio");
        }
        else if (box_header->boxtype == fcc_samr)
        {
            track->codec = CODEC_AMR;
            TRACE_1(MP4, "> Audio track is using AMR audio");
        }

        unsigned int reserved0 = read_bits(bitstr, 32);
        unsigned int reserved1 = read_bits(bitstr, 32);

        track->channel_count = read_bits(bitstr, 16);
        track->sample_size_bits = read_bits(bitstr, 16);

        unsigned int pre_defined = read_bits(bitstr, 16);
        unsigned short reserved3 = read_bits(bitstr, 16);

        track->sample_rate_hz = read_bits(bitstr, 16);
        unsigned short reserved4 = read_bits(bitstr, 16);

#if ENABLE_DEBUG
        TRACE_1(MP4, "> channel_count   : %u", track->channel_count);
        TRACE_1(MP4, "> sample_size_bits: %u", track->sample_size_bits);
        TRACE_1(MP4, "> sample_rate_hz  : %u", track->sample_rate_hz);
#endif
        if (mp4->xml) // xmlMapper
        {
            fprintf(mp4->xml, "  <title>Audio Sample Entry</title>\n");
            xmlSpacer(mp4->xml, "Sound Sample Description (Version 0)", -1);
            fprintf(mp4->xml, "  <reserved0>%u</reserved0>\n", reserved0);
            fprintf(mp4->xml, "  <reserved1>%u</reserved1>\n", reserved1);
            fprintf(mp4->xml, "  <channel_count>%u</channel_count>\n", track->channel_count);
            fprintf(mp4->xml, "  <sample_size_bits>%u</sample_size_bits>\n", track->sample_size_bits);
            fprintf(mp4->xml, "  <pre_defined>%u</pre_defined>\n", pre_defined);
            fprintf(mp4->xml, "  <reserved3>%u</reserved3>\n", reserved3);
            fprintf(mp4->xml, "  <sample_rate_hz>%u</sample_rate_hz>\n", track->sample_rate_hz);
            fprintf(mp4->xml, "  <reserved4>%u</reserved4>\n", reserved4);
        }

        int atom_version = reserved0 >> 16;
        if (atom_version == 1)
        {
            unsigned samples_per_packet = read_bits(bitstr, 32);
            unsigned bytes_per_packet = read_bits(bitstr, 32);
            unsigned bytes_per_frame = read_bits(bitstr, 32);
            unsigned bytes_per_sample = read_bits(bitstr, 32);

#if ENABLE_DEBUG
            //
#endif
            if (mp4->xml) // xmlMapper
            {
                xmlSpacer(mp4->xml, "Sound Sample Description (Version 1)", -1);
                fprintf(mp4->xml, "  <samples_per_packet>%u</samples_per_packet>\n", samples_per_packet);
                fprintf(mp4->xml, "  <bytes_per_packet>%u</bytes_per_packet>\n", bytes_per_packet);
                fprintf(mp4->xml, "  <bytes_per_frame>%u</bytes_per_frame>\n", bytes_per_frame);
                fprintf(mp4->xml, "  <bytes_per_sample>%u</bytes_per_sample>\n", bytes_per_sample);
            }
        }
        else if (atom_version == 2)
        {
            unsigned version = read_bits(bitstr, 16);
            unsigned revision_level = read_bits(bitstr, 16);
            unsigned vendor = read_bits(bitstr, 32);
            unsigned always3 = read_bits(bitstr, 16);
            unsigned always16 = read_bits(bitstr, 16);
            unsigned alwaysMinus2 = read_bits(bitstr, 16);
            unsigned always0 = read_bits(bitstr, 16);
            unsigned always65536 = read_bits(bitstr, 32);
            unsigned sizeOfStructOnly = read_bits(bitstr, 32);
            uint64_t asr = read_bits_64(bitstr, 64);
            double audioSampleRate = *((double *)&asr);
            unsigned numAudioChannels = read_bits(bitstr, 32);
            unsigned always7F000000 = read_bits(bitstr, 32);
            unsigned constBitsPerChannel = read_bits(bitstr, 32);
            unsigned formatSpecificFlags = read_bits(bitstr, 32);
            unsigned constBytesPerAudioPacket = read_bits(bitstr, 32);
            unsigned constLPCMFramesPerAudioPacket = read_bits(bitstr, 32);

#if ENABLE_DEBUG
            //
#endif
            if (mp4->xml) // xmlMapper
            {
                xmlSpacer(mp4->xml, "Sound Sample Description (Version 2)", -1);
                fprintf(mp4->xml, "  <version>%u</version>\n", version);
                fprintf(mp4->xml, "  <revision_level>%u</revision_level>\n", revision_level);
                fprintf(mp4->xml, "  <vendor>%u</vendor>\n", vendor);
                fprintf(mp4->xml, "  <always3>%u</always3>\n", always3);
                fprintf(mp4->xml, "  <always16>%u</always16>\n", always16);
                fprintf(mp4->xml, "  <alwaysMinus2>%u</alwaysMinus2>\n", alwaysMinus2);
                fprintf(mp4->xml, "  <always0>%u</always0>\n", always0);
                fprintf(mp4->xml, "  <always65536>%u</always65536>\n", always65536);
                fprintf(mp4->xml, "  <sizeOfStructOnly>%u</sizeOfStructOnly>\n", sizeOfStructOnly);
                fprintf(mp4->xml, "  <audioSampleRate>%f</audioSampleRate>\n", audioSampleRate);
                fprintf(mp4->xml, "  <numAudioChannels>%u</numAudioChannels>\n", numAudioChannels);
                fprintf(mp4->xml, "  <always7F000000>%u</always7F000000>\n", always7F000000);
                fprintf(mp4->xml, "  <constBitsPerChannel>%u</constBitsPerChannel>\n", constBitsPerChannel);
                fprintf(mp4->xml, "  <formatSpecificFlags>%u</formatSpecificFlags>\n", formatSpecificFlags);
                fprintf(mp4->xml, "  <constBytesPerAudioPacket>%u</constBytesPerAudioPacket>\n", constBytesPerAudioPacket);
                fprintf(mp4->xml, "  <constLPCMFramesPerAudioPacket>%u</constLPCMFramesPerAudioPacket>\n", constLPCMFramesPerAudioPacket);
            }
        }
        else if (atom_version > 2)
        {
            TRACE_WARNING(MP4, "Unknown atom_version value: %i", atom_version);
        }
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            case BOX_WAVE:
                retcode = parse_wave(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_CHAN:
                retcode = parse_chan(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_SA3D:
                retcode = parse_sa3d(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_SAND:
                retcode = parse_sand(bitstr, &box_subsubheader, track, mp4);
                break;

            case BOX_DAMR:
                retcode = parse_damr(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_DAC3:
                retcode = parse_dac3(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_DEC3:
                retcode = parse_dec3(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Visual Sample Entry.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2.2 Syntax
 *
 * If an AVC box (AVCDecoderConfigurationRecord) is present, it also contains the
 * diferents SPS and PPS of the video.
 */
int parse_stsd_video(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_video()" CLR_RESET);
    int retcode = SUCCESS;

    // VisualSampleEntry
    {
        if (box_header->boxtype == fcc_avc1 || box_header->boxtype == fcc_avc3)
        {
            track->codec = CODEC_H264;
            TRACE_1(MP4, "> Video track is using H.264 codec");
        }
        else if (box_header->boxtype == fcc_hvc1 ||
                 box_header->boxtype == fcc_dvh1 || box_header->boxtype == fcc_dvhe)
        {
            track->codec = CODEC_H265;
            TRACE_1(MP4, "> Video track is using H.265 codec");
        }
        else if (box_header->boxtype == fcc_vvc1)
        {
            track->codec = CODEC_H266;
            TRACE_1(MP4, "> Video track is using H.266 codec");
        }
        else if (box_header->boxtype == fcc_vp08)
        {
            track->codec = CODEC_VP8;
            TRACE_1(MP4, "> Video track is using VP8 codec");
        }
        else if (box_header->boxtype == fcc_vp09)
        {
            track->codec = CODEC_VP9;
            TRACE_1(MP4, "> Video track is using VP9 codec");
        }
        else if (box_header->boxtype == fcc_CFHD)
        {
            track->codec = CODEC_CINEFORM;
            TRACE_1(MP4, "> Video track is using CineForm codec");
        }

        else if (box_header->boxtype == fcc_jpeg)
        {
            track->codec = CODEC_JPEG;
            TRACE_1(MP4, "> Video track is using JPEG picture(s)");
        }
        else if (box_header->boxtype == fcc_png)
        {
            track->codec = CODEC_PNG;
            TRACE_1(MP4, "> Video track is using PNG picture(s)");
        }
        else if (box_header->boxtype == fcc_tga)
        {
            track->codec = CODEC_TGA;
            TRACE_1(MP4, "> Video track is using TGA picture(s)");
        }
        else if (box_header->boxtype == fcc_gif)
        {
            track->codec = CODEC_GIF;
            TRACE_1(MP4, "> Video track is using GIF picture(s)");
        }

        skip_bits(bitstr, 16); // pre_defined
        skip_bits(bitstr, 16); // reserved

        unsigned int pre_defined[3];
        pre_defined[0] = read_bits(bitstr, 32);
        pre_defined[1] = read_bits(bitstr, 32);
        pre_defined[2] = read_bits(bitstr, 32);

        track->width = read_bits(bitstr, 16);
        track->height = read_bits(bitstr, 16);

        // 0x00480000; // 72 dpi
        unsigned int horizresolution = read_bits(bitstr, 32);
        unsigned int vertresolution = read_bits(bitstr, 32);

        skip_bits(bitstr, 32); // reserved

        unsigned int frame_count = read_bits(bitstr, 16);

        uint8_t compressorsize = (uint8_t)read_bits(bitstr, 8);
        for (int i = 0; i < 31; i++)
        {
            track->compressorname[i] = (char)read_bits(bitstr, 8);
        }
        track->compressorname[compressorsize] = '\0';

        unsigned int color_depth = read_bits(bitstr, 16);
        track->color_depth = color_depth / 3;
        skip_bits(bitstr, 16); // pre_defined

#if ENABLE_DEBUG
        TRACE_1(MP4, "> width  : %u", track->width);
        TRACE_1(MP4, "> height : %u", track->height);
        TRACE_1(MP4, "> horizresolution : 0x%X", horizresolution);
        TRACE_1(MP4, "> vertresolution  : 0x%X", vertresolution);
        TRACE_1(MP4, "> frame_count     : %u", frame_count);
        TRACE_1(MP4, "> compressor      : '%s'", track->compressorname);
        TRACE_1(MP4, "> color depth     : %u", color_depth);
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Visual Sample Entry</title>\n");
            fprintf(mp4->xml, "  <width>%u</width>\n", track->width);
            fprintf(mp4->xml, "  <height>%u</height>\n", track->height);
            fprintf(mp4->xml, "  <horizresolution>%u</horizresolution>\n", horizresolution);
            fprintf(mp4->xml, "  <vertresolution>%u</vertresolution>\n", vertresolution);
            fprintf(mp4->xml, "  <frame_count>%u</frame_count>\n", frame_count);
            fprintf(mp4->xml, "  <compressorname>%s</compressorname>\n", track->compressorname);
            fprintf(mp4->xml, "  <color_depth>%u</color_depth>\n", color_depth);
        }
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            case BOX_D263:
                retcode = parse_d263(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_AVCC:
                retcode = parse_avcC(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_HVCC:
                retcode = parse_hvcC(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_VVCC:
                retcode = parse_vvcC(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_VPCC:
                retcode = parse_vpcC(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_AV1C:
                retcode = parse_av1C(bitstr, &box_subsubheader, track, mp4);
                break;

            case BOX_DVCC:
                retcode = parse_dvcC(bitstr, &box_subsubheader, track, mp4);
                break;

            case BOX_BTRT:
                retcode = parse_btrt(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_CLAP:
                retcode = parse_clap(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_COLR:
                retcode = parse_colr(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_CHRM:
                retcode = parse_chrm(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_FIEL:
                retcode = parse_fiel(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_GAMA:
                retcode = parse_gama(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_PASP:
                retcode = parse_pasp(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_ST3D:
                retcode = parse_st3d(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_SV3D:
                retcode = parse_sv3d(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_COLL:
                retcode = parse_coll(bitstr, &box_subsubheader, track, mp4);
                break;
            case BOX_SMDM:
                retcode = parse_smdm(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Timecode Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Timecode Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-6983
 */
int parse_stsd_tmcd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_tmcd()" CLR_RESET);
    int retcode = SUCCESS;

    {
        skip_bits(bitstr, 32); // reserved
        unsigned int flags = read_bits(bitstr, 32);
        unsigned int time_scale = read_bits(bitstr, 32);
        unsigned int frame_duration = read_bits(bitstr, 32);
        track->number_of_frames = read_bits(bitstr, 8);
        skip_bits(bitstr, 8); // reserved

#if ENABLE_DEBUG
        TRACE_1(MP4, "> flags  : %u", flags);
        TRACE_1(MP4, "> time_scale : %u", time_scale);
        TRACE_1(MP4, "> frame_duration : %u", frame_duration);
        TRACE_1(MP4, "> number_of_frames  : %u", track->number_of_frames);
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Timecode Sample Description</title>\n");
            fprintf(mp4->xml, "  <flags>%u</flags>\n", flags);
            fprintf(mp4->xml, "  <time_scale>%u</time_scale>\n", time_scale);
            fprintf(mp4->xml, "  <frame_duration>%u</frame_duration>\n", frame_duration);
            fprintf(mp4->xml, "  <number_of_frames>%u</number_of_frames>\n", track->number_of_frames);
        }
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Text Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Text Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-69835
 */
int parse_stsd_text(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_text()" CLR_RESET);
    int retcode = SUCCESS;

    {
        unsigned int display_flags = read_bits(bitstr, 32);
        unsigned int text_justification = read_bits(bitstr, 32);
        unsigned int backgroundcolor_r = read_bits(bitstr, 16);
        unsigned int backgroundcolor_g = read_bits(bitstr, 16);
        unsigned int backgroundcolor_b = read_bits(bitstr, 16);
        uint64_t default_text_box = read_bits_64(bitstr, 64);
        /*uint64_t reserved =*/ read_bits_64(bitstr, 64);
        unsigned int font_number = read_bits(bitstr, 16);
        unsigned int font_face = read_bits(bitstr, 16);
        /*unsigned int reserved =*/ read_bits(bitstr, 8);
        /*unsigned int reserved =*/ read_bits(bitstr, 16);
        unsigned int foregroundcolor_r = read_bits(bitstr, 16);
        unsigned int foregroundcolor_g = read_bits(bitstr, 16);
        unsigned int foregroundcolor_b = read_bits(bitstr, 16);
        //char text_name[128]; // TODO

#if ENABLE_DEBUG
        TRACE_1(MP4, "> display_flags  : %u", display_flags);
        TRACE_1(MP4, "> text_justification  : %u", text_justification);
        TRACE_1(MP4, "> background_color  : 0x%X%X%X",
                backgroundcolor_r, backgroundcolor_g, backgroundcolor_b);
        TRACE_1(MP4, "> default_text_box  : %llu", default_text_box);
        TRACE_1(MP4, "> font_number  : %u", font_number);
        TRACE_1(MP4, "> font_face  : %u", font_face);
        TRACE_1(MP4, "> foreground_color  : 0x%X%X%X",
                foregroundcolor_r, foregroundcolor_g, foregroundcolor_b);
        //TRACE_1(MP4, "> text_name  : '%s'", text_name); // TODO
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Text Sample Description</title>\n");
            fprintf(mp4->xml, "  <display_flags>%u</display_flags>\n", display_flags);
            fprintf(mp4->xml, "  <text_justification>%u</text_justification>\n", text_justification);
            fprintf(mp4->xml, "  <background_color>0x%X%X%X</background_color>\n",
                    backgroundcolor_r, backgroundcolor_g, backgroundcolor_b);
            fprintf(mp4->xml, "  <default_text_box>%" PRIu64 "</default_text_box>\n", default_text_box);
            fprintf(mp4->xml, "  <font_number>%u</font_number>\n", font_number);
            fprintf(mp4->xml, "  <font_face>%u</font_face>\n", font_face);
            fprintf(mp4->xml, "  <foreground_color>0x%X%X%X</foreground_color>\n",
                    foregroundcolor_r, foregroundcolor_g, foregroundcolor_b);
            //fprintf(mp4->xml, "  <text_name>%s</text_name>\n", text_name); // TODO
        }
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Timed Metadata Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Timed Metadata Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-SW130
 */
int parse_stsd_meta(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_meta()" CLR_RESET);
    int retcode = SUCCESS;

    {
        /*unsigned int reserved =*/ read_bits(bitstr, 32);

        print_box_header(box_header);
        if (mp4->xml) fprintf(mp4->xml, "  <title>Timed Metadata Sample Description</title>\n");
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Hint
 */
int parse_stsd_hint(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_hint()" CLR_RESET);
    int retcode = SUCCESS;

    {
        /*unsigned int reserved =*/ read_bits(bitstr, 32);

        print_box_header(box_header);
        if (mp4->xml) fprintf(mp4->xml, "  <title>Hint</title>\n");
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief MPEG-4 Systems
 */
int parse_stsd_sdsm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_sdsm()" CLR_RESET);
    int retcode = SUCCESS;

    // Print box header
    print_box_header(box_header);
    //write_box_header(box_header, mp4->xml, "MPEG-4 Systems Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <title>MPEG-4 Systems Sample Description</title>\n");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief MPEG-4 Objects
 */
int parse_stsd_odsm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_odsm()" CLR_RESET);
    int retcode = SUCCESS;

    // Print box header
    print_box_header(box_header);
    //write_box_header(box_header, mp4->xml, "MPEG-4 Objects Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <title>MPEG-4 Objects Sample Description</title>\n");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief QuickTime Format Atom ('frma')
 * \param bitstr
 * \param box_header
 * \param track
 * \param mp4
 * \return
 *
 * https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-75770
 */
int parse_frma(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_frma()" CLR_RESET);
    int retcode = SUCCESS;

    char fcc[5];
    track->pcm_format = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> format  : %s", getFccString_le(track->pcm_format, fcc));
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Format");
        fprintf(mp4->xml, "  <format>%s</format>\n", getFccString_le(track->pcm_format, fcc));
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

int parse_enda(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_enda()" CLR_RESET);
    int retcode = SUCCESS;

    track->pcm_endianness = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> endian  : %u", track->pcm_endianness);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Format");
        fprintf(mp4->xml, "  <endian>%u</endian>\n", track->pcm_endianness);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

int parse_chan(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_chan()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned int mChannelBitmap = read_bits(bitstr, 16);
    unsigned int mChannelDescriptions = read_bits(bitstr, 16);
    unsigned int mChannelLayoutTag = read_bits(bitstr, 32);
    unsigned int mNumberChannelDescriptions = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> mChannelBitmap  : %u", mChannelBitmap);
    TRACE_1(MP4, "> mChannelDescriptions  : %u", mChannelDescriptions);
    TRACE_1(MP4, "> mChannelLayoutTag  : %u", mChannelLayoutTag);
    TRACE_1(MP4, "> mNumberChannelDescriptions  : %u", mNumberChannelDescriptions);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Audio Channel Layout");
        fprintf(mp4->xml, "  <mChannelBitmap>%u</mChannelBitmap>\n", mChannelBitmap);
        fprintf(mp4->xml, "  <mChannelDescriptions>%u</mChannelDescriptions>\n", mChannelDescriptions);
        fprintf(mp4->xml, "  <mChannelLayoutTag>%u</mChannelLayoutTag>\n", mChannelLayoutTag);
        fprintf(mp4->xml, "  <mNumberChannelDescriptions>%u</mNumberChannelDescriptions>\n", mNumberChannelDescriptions);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief siDecompressionParam Atom ('wave')
 * \param bitstr
 * \param box_header
 * \param track
 * \param mp4
 * \return
 */
int parse_wave(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_wave()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "siDecompressionParam");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_FRMA:
                    retcode = parse_frma(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_ENDA:
                    retcode = parse_enda(bitstr, &box_subheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subheader);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief AMR parameters Atom ('damr')
 *
 * 3GPP TS 26.244 specification
 */
int parse_damr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_damr()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AMR parameters");

    char *vendor = read_mp4_string(bitstr, 4, mp4->xml, "vendor");
    free(vendor);

    uint8_t decoder_version = read_mp4_uint8(bitstr, mp4->xml, "decoder_version");
    uint16_t mode_set = read_mp4_uint16(bitstr, mp4->xml, "mode_set"); // A bitmask indicating which AMR modes are supported.
    uint8_t mode_change_period = read_mp4_uint8(bitstr, mp4->xml, "mode_change_period"); // Indicates the number of frames between mode changes.
    uint8_t frames_per_sample = read_mp4_uint8(bitstr, mp4->xml, "frames_per_sample");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief AC3 parameters Atom ('dac3')
 */
int parse_dac3(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dac3()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AC3 parameters");

    uint8_t fscod = read_mp4_uint(bitstr, 2, mp4->xml, "fscod");
    uint8_t bsid = read_mp4_uint(bitstr, 5, mp4->xml, "bsid");
    uint8_t bsmod = read_mp4_uint(bitstr, 3, mp4->xml, "bsmod");
    uint8_t acmod = read_mp4_uint(bitstr, 3, mp4->xml, "acmod");
    uint8_t lfeon = read_mp4_uint(bitstr, 1, mp4->xml, "lfeon");
    uint16_t bit_rate_code = read_mp4_uint(bitstr, 5, mp4->xml, "bit_rate_code");
    uint8_t reserved = read_mp4_uint(bitstr, 5, mp4->xml, "reserved");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief Enhanced AC3 parameters Atom ('dec3')
 *
 * Annex F of ETSI TS 102 366 and Deriving the contents of the EC3Specific box section of this information set.
 */
int parse_dec3(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dec3()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "E-AC3 parameters");

    uint16_t data_rate = read_mp4_uint(bitstr, 13, mp4->xml, "data_rate");
    uint8_t num_ind_sub = read_mp4_uint(bitstr, 3, mp4->xml, "num_ind_sub");

    for (uint8_t i = 0; i < num_ind_sub + 1; i++)
    {
        xmlSpacer(mp4->xml, "Stream", i+1);
        uint8_t fscod = read_mp4_uint(bitstr, 2, mp4->xml, "fscod");
        uint8_t bsid = read_mp4_uint(bitstr, 5, mp4->xml, "bsid");
        uint8_t reserved1 = read_mp4_uint(bitstr, 1, mp4->xml, "reserved");
        uint8_t asvc = read_mp4_uint(bitstr, 1, mp4->xml, "asvc");
        uint8_t bsmod = read_mp4_uint(bitstr, 3, mp4->xml, "bsmod");
        uint8_t acmod = read_mp4_uint(bitstr, 3, mp4->xml, "acmod");
        uint8_t lfeon = read_mp4_uint(bitstr, 1, mp4->xml, "lfeon");
        uint8_t reserved2 = read_mp4_uint(bitstr, 3, mp4->xml, "reserved");
        uint8_t num_dep_sub = read_mp4_uint(bitstr, 4, mp4->xml, "num_dep_sub");
        if (num_dep_sub > 0)
        {
            uint16_t chan_loc = read_mp4_uint(bitstr, 9, mp4->xml, "chan_loc");
        }
        else
        {
            uint8_t reserved3 = read_mp4_uint(bitstr, 1, mp4->xml, "reserved");
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.1 Syntax, 5.2.4.1.2 Semantics).
 */
int parse_avcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_avcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->avcC)
    {
        TRACE_WARNING(MKV, "parse_avcC() we aleady have an codecprivate_avcC_t structure!");
        return FAILURE;
    }
    track->avcC = (codecprivate_avcC_t *)calloc(1, sizeof(codecprivate_avcC_t));
    if (!track->avcC)
    {
        TRACE_ERROR(MKV, "parse_avcC() codecprivate_avcC_t allocation error!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AVC Configuration");

    // Parse box content
    track->avcC->configurationVersion = read_bits(bitstr, 8);
    track->avcC->AVCProfileIndication = read_bits(bitstr, 8);
    track->avcC->profile_compatibility = read_bits(bitstr, 8);
    track->avcC->AVCLevelIndication = read_bits(bitstr, 8);
    skip_bits(bitstr, 6); // reserved
    track->avcC->lengthSizeMinusOne = read_bits(bitstr, 2);
    skip_bits(bitstr, 3); // reserved

    // SPS
    track->avcC->sps_count = read_bits(bitstr, 5);
    track->avcC->sps_sample_offset = (int64_t *)calloc(track->avcC->sps_count, sizeof(int64_t));
    track->avcC->sps_sample_size = (int32_t *)calloc(track->avcC->sps_count, sizeof(int32_t));

    for (unsigned i = 0; i < track->avcC->sps_count && i < MAX_SPS; i++) // MAX_SPS = 32
    {
        track->avcC->sps_sample_size[i] = read_bits(bitstr, 16);
        track->avcC->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);
        track->avcC->sps_array[i] = (h264_sps_t *)calloc(1, sizeof(h264_sps_t));

        skip_bits(bitstr, 8); // skip NAL header
        decodeSPS(bitstr, track->avcC->sps_array[i]);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]))
        {
            TRACE_WARNING(MP4, "SPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]));

            skip_bits(bitstr, ((track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // PPS
    track->avcC->pps_count = read_bits(bitstr, 8);
    track->avcC->pps_sample_offset = (int64_t *)calloc(track->avcC->pps_count, sizeof(int64_t));
    track->avcC->pps_sample_size = (int32_t *)calloc(track->avcC->pps_count, sizeof(int32_t));

    for (unsigned i = 0; i < track->avcC->pps_count && i < MAX_PPS; i++) // MAX_PPS = 256
    {
        track->avcC->pps_sample_size[i] = read_bits(bitstr, 16);
        track->avcC->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);
        track->avcC->pps_array[i] = (h264_pps_t *)calloc(1, sizeof(h264_pps_t));

        skip_bits(bitstr, 8); // skip NAL header
        decodePPS(bitstr, track->avcC->pps_array[i], track->avcC->sps_array);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]))
        {
            TRACE_WARNING(MP4, "PPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]));
            skip_bits(bitstr, ((track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

#if ENABLE_DEBUG
    TRACE_1(MP4, "> configurationVersion  : %u", track->avcC->configurationVersion);
    TRACE_1(MP4, "> AVCProfileIndication  : %u", track->avcC->AVCProfileIndication);
    TRACE_1(MP4, "> profile_compatibility : %u", track->avcC->profile_compatibility);
    TRACE_1(MP4, "> AVCLevelIndication    : %u", track->avcC->AVCLevelIndication);
    TRACE_1(MP4, "> lengthSizeMinusOne    : %u", track->avcC->lengthSizeMinusOne);

    TRACE_1(MP4, "> numOfSequenceParameterSets    = %u", track->avcC->sps_count);
    for (unsigned i = 0; i < track->avcC->sps_count; i++)
    {
        TRACE_1(MP4, "> sequenceParameterSetLength[%u] : %u", i, track->avcC->sps_sample_size[i]);
        TRACE_1(MP4, "> sequenceParameterSetOffset[%u] : %li", i, track->avcC->sps_sample_offset[i]);
    }

    TRACE_1(MP4, "> numOfPictureParameterSets     = %u", track->avcC->pps_count);
    for (unsigned i = 0; i < track->avcC->pps_count; i++)
    {
        TRACE_1(MP4, "> pictureParameterSetLength[%u]  : %u", i, track->avcC->pps_sample_size[i]);
        TRACE_1(MP4, "> pictureParameterSetOffset[%u]  : %li", i, track->avcC->pps_sample_offset[i]);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        fprintf(mp4->xml, "  <configurationVersion>%u</configurationVersion>\n", track->avcC->configurationVersion);
        fprintf(mp4->xml, "  <AVCProfileIndication>%u</AVCProfileIndication>\n", track->avcC->AVCProfileIndication);
        fprintf(mp4->xml, "  <profile_compatibility>%u</profile_compatibility>\n", track->avcC->profile_compatibility);
        fprintf(mp4->xml, "  <AVCLevelIndication>%u</AVCLevelIndication>\n", track->avcC->AVCLevelIndication);
        fprintf(mp4->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", track->avcC->lengthSizeMinusOne);

        fprintf(mp4->xml, "  <numOfSequenceParameterSets>%u</numOfSequenceParameterSets>\n", track->avcC->sps_count);
        for (unsigned i = 0; i < track->avcC->sps_count; i++)
        {
            xmlSpacer(mp4->xml, "SequenceParameterSet infos", i);
            fprintf(mp4->xml, "  <sequenceParameterSetLength index=\"%u\">%u</sequenceParameterSetLength>\n", i, track->avcC->sps_sample_size[i]);
            fprintf(mp4->xml, "  <sequenceParameterSetOffset index=\"%u\">%" PRId64 "</sequenceParameterSetOffset>\n", i, track->avcC->sps_sample_offset[i]);
        }

        fprintf(mp4->xml, "  <numOfPictureParameterSets>%u</numOfPictureParameterSets>\n", track->avcC->pps_count);
        for (unsigned i = 0; i < track->avcC->pps_count; i++)
        {
            xmlSpacer(mp4->xml, "PictureParameterSet", i);
            fprintf(mp4->xml, "  <pictureParameterSetLength index=\"%u\">%u</pictureParameterSetLength>\n", i, track->avcC->pps_sample_size[i]);
            fprintf(mp4->xml, "  <pictureParameterSetOffset index=\"%u\">%" PRId64 "</pictureParameterSetOffset>\n", i, track->avcC->pps_sample_offset[i]);
        }

        for (unsigned i = 0; i < track->avcC->sps_count; i++)
        {
            printPPS(track->avcC->pps_array[i], track->avcC->sps_array);

            mapSPS(track->avcC->sps_array[i],
                   track->avcC->sps_sample_offset[i],
                   track->avcC->sps_sample_size[i],
                   mp4->xml);
        }
        for (unsigned i = 0; i < track->avcC->pps_count; i++)
        {
            printPPS(track->avcC->pps_array[i], track->avcC->sps_array);

            mapPPS(track->avcC->pps_array[i],
                   track->avcC->sps_array,
                   track->avcC->pps_sample_offset[i],
                   track->avcC->pps_sample_size[i],
                   mp4->xml);
        }

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief HEVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15 (2015)' specification:
 * 8.3.3.1 Decoder configuration information.
 */
int parse_hvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hvcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->hvcC)
    {
        TRACE_WARNING(MKV, "parse_hvcC() we aleady have an codecprivate_hvcC_t structure!");
        return FAILURE;
    }
    track->hvcC = (codecprivate_hvcC_t *)calloc(1, sizeof(codecprivate_hvcC_t));
    if (!track->hvcC)
    {
        TRACE_ERROR(MKV, "parse_hvcC() codecprivate_hvcC_t allocation error!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "HEVC Configuration");

    // Parse box content
    track->hvcC->configurationVersion = read_bits(bitstr, 8);
    track->hvcC->general_profile_space = read_bits(bitstr, 2);
    track->hvcC->general_tier_flag = read_bits(bitstr, 1);
    track->hvcC->general_profile_idc = read_bits(bitstr, 5);
    track->hvcC->general_profile_compatibility_flags = read_bits(bitstr, 32);
    track->hvcC->general_constraint_indicator_flags = read_bits_64(bitstr, 48);
    track->hvcC->general_level_idc = read_bits(bitstr, 8);

    skip_bits(bitstr, 4); // reserved
    track->hvcC->min_spatial_segmentation_idc = read_bits(bitstr, 12);
    skip_bits(bitstr, 6); // reserved
    track->hvcC->parallelismType = read_bits(bitstr, 2);
    skip_bits(bitstr, 6); // reserved
    track->hvcC->chromaFormat = read_bits(bitstr, 2);
    skip_bits(bitstr, 5); // reserved
    track->hvcC->bitDepthLumaMinus8 = read_bits(bitstr, 3);
    skip_bits(bitstr, 5); // reserved
    track->hvcC->bitDepthChromaMinus8 = read_bits(bitstr, 3);
    track->hvcC->avgFrameRate = read_bits(bitstr, 16);
    track->hvcC->constantFrameRate = read_bits(bitstr, 2);
    track->hvcC->numTemporalLayers = read_bits(bitstr, 3);
    track->hvcC->temporalIdNested = read_bits(bitstr, 1);
    track->hvcC->lengthSizeMinusOne = read_bits(bitstr, 2);

    track->hvcC->numOfArrays = read_bits(bitstr, 8);
/*
    bool *array_completeness = NULL;
    uint8_t *NAL_unit_type = NULL;
    uint16_t *numNalus = NULL;
    uint16_t **nalUnitLength = NULL;
    uint8_t ***nalUnit = NULL;

    for (unsigned j = 0; j < track->hvcC->numOfArrays; j++)
    {
        array_completeness = (bool *)malloc(track->hvcC->numOfArrays);
        NAL_unit_type = (uint8_t *)malloc(track->hvcC->numOfArrays);
        numNalus = (uint16_t *)malloc(track->hvcC->numOfArrays);
        nalUnitLength = (uint16_t **)malloc(track->hvcC->numOfArrays);
        nalUnit = (uint8_t ***)malloc(track->hvcC->numOfArrays);
        if (array_completeness && NAL_unit_type && numNalus && nalUnitLength && nalUnit)
        {
            array_completeness[j] = read_bits(bitstr, 1);
            skip_bits(bitstr, 1); // reserved
            NAL_unit_type[j] = read_bits(bitstr, 6); // can be VPS, SPS, PPS, or SEI NAL unit
            numNalus[j] = read_bits(bitstr, 16);

            for (unsigned i = 0; i < numNalus[j]; i++)
            {
                nalUnitLength[j] = (uint16_t *)malloc(numNalus[j]);
                nalUnit[j] = (uint8_t **)malloc(numNalus[j]);
                //if (nalUnitLength[j][i] && nalUnit[j][i])
                {
                    nalUnitLength[j][i] = read_bits(bitstr, 16);

                    if (NAL_unit_type[j] == NALU_TYPE_VPS_NUT)
                    {
                        skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                    else if (NAL_unit_type[j] == NALU_TYPE_SPS_NUT)
                    {
                        skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                    else if (NAL_unit_type[j] == NALU_TYPE_PPS_NUT)
                    {
                        skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                    else if (NAL_unit_type[j] == NALU_TYPE_PREFIX_SEI_NUT)
                    {
                        skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                    else if (NAL_unit_type[j] == NALU_TYPE_SUFFIX_SEI_NUT)
                    {
                        skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                    else
                    {
                        //skip_bits(bitstr, nalUnitLength[j][i] * 8);
                    }
                }
                //else
                //{
                //    retcode = FAILURE;
                //}
            }
        }
        else
        {
            retcode = FAILURE;
        }
    }
*/
#if ENABLE_DEBUG
    TRACE_1(MP4, "> configurationVersion : %u", track->hvcC->configurationVersion);
    TRACE_1(MP4, "> general_profile_space: %u", track->hvcC->general_profile_space);
    TRACE_1(MP4, "> general_tier_flag    : %u", track->hvcC->general_tier_flag);
    TRACE_1(MP4, "> general_profile_idc  : %u", track->hvcC->general_profile_idc);
    TRACE_1(MP4, "> general_profile_compatibility_flags: %u", track->hvcC->general_profile_compatibility_flags);
    TRACE_1(MP4, "> general_constraint_indicator_flags : %lu", track->hvcC->general_constraint_indicator_flags);
    TRACE_1(MP4, "> general_level_idc    : %u", track->hvcC->general_level_idc);

    TRACE_1(MP4, "> min_spatial_segmentation_idc: %u", track->hvcC->min_spatial_segmentation_idc);
    TRACE_1(MP4, "> parallelismType      : %u", track->hvcC->parallelismType);
    TRACE_1(MP4, "> chromaFormat         : %u", track->hvcC->chromaFormat);
    TRACE_1(MP4, "> bitDepthLumaMinus8   : %u", track->hvcC->bitDepthLumaMinus8);
    TRACE_1(MP4, "> bitDepthChromaMinus8 : %u", track->hvcC->bitDepthChromaMinus8);

    TRACE_1(MP4, "> avgFrameRate         : %u", track->hvcC->avgFrameRate);
    TRACE_1(MP4, "> constantFrameRate    : %u", track->hvcC->constantFrameRate);
    TRACE_1(MP4, "> numTemporalLayers    : %u", track->hvcC->numTemporalLayers);
    TRACE_1(MP4, "> temporalIdNested     : %u", track->hvcC->temporalIdNested);
    TRACE_1(MP4, "> lengthSizeMinusOne   : %u", track->hvcC->lengthSizeMinusOne);

    TRACE_1(MP4, "> numOfArrays          : %u", track->hvcC->numOfArrays);
    for (unsigned j = 0; j < track->hvcC->numOfArrays; j++)
    {
        // TODO // NAL unit table
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        fprintf(mp4->xml, "  <configurationVersion>%u</configurationVersion>\n", track->hvcC->configurationVersion);
        fprintf(mp4->xml, "  <general_profile_space>%u</general_profile_space>\n", track->hvcC->general_profile_space);
        fprintf(mp4->xml, "  <general_tier_flag>%u</general_tier_flag>\n", track->hvcC->general_tier_flag);
        fprintf(mp4->xml, "  <general_profile_idc>%u</general_profile_idc>\n", track->hvcC->general_profile_idc);
        fprintf(mp4->xml, "  <general_profile_compatibility_flags>%u</general_profile_compatibility_flags>\n", track->hvcC->general_profile_compatibility_flags);
        fprintf(mp4->xml, "  <general_constraint_indicator_flags>%" PRId64 "</general_constraint_indicator_flags>\n", track->hvcC->general_constraint_indicator_flags);
        fprintf(mp4->xml, "  <general_level_idc>%u</general_level_idc>\n", track->hvcC->general_level_idc);

        fprintf(mp4->xml, "  <min_spatial_segmentation_idc>%u</min_spatial_segmentation_idc>\n", track->hvcC->min_spatial_segmentation_idc);
        fprintf(mp4->xml, "  <parallelismType>%u</parallelismType>\n", track->hvcC->parallelismType);
        fprintf(mp4->xml, "  <chromaFormat>%u</chromaFormat>\n", track->hvcC->chromaFormat);
        fprintf(mp4->xml, "  <bitDepthLumaMinus8>%u</bitDepthLumaMinus8>\n", track->hvcC->bitDepthLumaMinus8);
        fprintf(mp4->xml, "  <bitDepthChromaMinus8>%u</bitDepthChromaMinus8>\n", track->hvcC->bitDepthChromaMinus8);

        fprintf(mp4->xml, "  <avgFrameRate>%u</avgFrameRate>\n", track->hvcC->avgFrameRate);
        fprintf(mp4->xml, "  <constantFrameRate>%u</constantFrameRate>\n", track->hvcC->constantFrameRate);
        fprintf(mp4->xml, "  <numTemporalLayers>%u</numTemporalLayers>\n", track->hvcC->numTemporalLayers);
        fprintf(mp4->xml, "  <temporalIdNested>%u</temporalIdNested>\n", track->hvcC->temporalIdNested);
        fprintf(mp4->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", track->hvcC->lengthSizeMinusOne);
/*
        fprintf(mp4->xml, "  <numOfArrays>%u</numOfArrays>\n", track->hvcC->numOfArrays);
        for (unsigned j = 0; j < track->hvcC->numOfArrays; j++)
        {
            xmlSpacer(mp4->xml, "Array infos", j);
            fprintf(mp4->xml, "  <array_completeness>%u</array_completeness>\n", track->hvcC->array_completeness[j]);
            fprintf(mp4->xml, "  <NAL_unit_type>%u</NAL_unit_type>\n", track->hvcC->NAL_unit_type[j]);
            fprintf(mp4->xml, "  <numNalus>%u</numNalus>\n", track->hvcC->numNalus[j]);

            for (unsigned i = 0; i < track->hvcC->numNalus[j]; i++)
            {
                xmlSpacer(mp4->xml, "nal unit", i);
                //nalUnitLength[j] = (uint16_t *)malloc(numNalus[j]);
                //nalUnit[j] = (uint8_t **)malloc(numNalus[j]);
                //if (nalUnitLength[j] && nalUnit[j])
                {
                    fprintf(mp4->xml, "  <nalUnitLength>%u</nalUnitLength>\n", nalUnitLength[j][i]);
                }
            }
        }
*/
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Dolby Vision Configuration Box.
 */
int parse_dvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dvcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->dvcC)
    {
        TRACE_WARNING(MKV, "parse_dvcC() we aleady have an codecprivate_dvcC_t structure!");
        return FAILURE;
    }
    track->dvcC = (codecprivate_dvcC_t *)calloc(1, sizeof(codecprivate_dvcC_t));
    if (!track->dvcC)
    {
        TRACE_ERROR(MKV, "parse_dvcC() we aleady have an codecprivate_dvcC_t structure!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Dolby Vision Configuration");

    // Parse box content
    track->dvcC->dv_version_major = read_mp4_uint8(bitstr, mp4->xml, "dv_version_major");
    track->dvcC->dv_version_minor = read_mp4_uint8(bitstr, mp4->xml, "dv_version_minor");
    track->dvcC->dv_profile = read_mp4_uint(bitstr, 7, mp4->xml, "dv_profile");
    track->dvcC->dv_level = read_mp4_uint(bitstr, 6, mp4->xml, "dv_level");
    track->dvcC->rpu_present_flag = read_mp4_flag(bitstr, mp4->xml, "rpu_present_flag");
    track->dvcC->el_present_flag = read_mp4_flag(bitstr, mp4->xml, "el_present_flag");
    track->dvcC->bl_present_flag = read_mp4_flag(bitstr, mp4->xml, "bl_present_flag");
    if (track->dvcC->bl_present_flag)
    {
        track->dvcC->dv_bl_signal_compatibility_id = read_mp4_uint(bitstr, 4, mp4->xml, "dv_bl_signal_compatibility_id");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief VVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15 (2022)' specification:
 * 8.3.3.1 Decoder configuration information.
 */
int parse_vvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_vvcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Init
    if (track->vvcC)
    {
        TRACE_WARNING(MKV, "parse_vvcC() we aleady have an codecprivate_vvcC_t structure!");
        return FAILURE;
    }
    track->vvcC = (codecprivate_vvcC_t *)calloc(1, sizeof(codecprivate_vvcC_t));
    if (!track->vvcC)
    {
        TRACE_ERROR(MKV, "parse_vvcC() codecprivate_vvcC_t allocation error!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "VVC Configuration");

    // Parse box content
    // TODO

#if ENABLE_DEBUG
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        // TODO

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief VPx Configuration Box.
 *
 * From 'VP Codec ISO Media File Format Binding':
 * VP Codec Sample Entry Box
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_vpcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_vpcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Init
    if (track->vpcC)
    {
        TRACE_WARNING(MKV, "parse_vpcC() we aleady have an codecprivate_vpcC_t structure!");
        return FAILURE;
    }
    track->vpcC = (codecprivate_vpcC_t *)calloc(1, sizeof(codecprivate_vpcC_t));
    if (!track->vpcC)
    {
        TRACE_ERROR(MKV, "parse_vpcC() codecprivate_vpcC_t allocation error!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "VPx Configuration");

    // Parse box content
    track->vpcC->profile = read_bits(bitstr, 8);
    track->vpcC->level = read_bits(bitstr, 8);
    track->vpcC->bitDepth = read_bits(bitstr, 4);
    track->vpcC->chromaSubsampling = read_bits(bitstr, 3);
    track->vpcC->videoFullRangeFlag = read_bits(bitstr, 1);
    track->vpcC->colourPrimaries = read_bits(bitstr, 8);
    track->vpcC->transferCharacteristics = read_bits(bitstr, 8);
    track->vpcC->matrixCoefficients = read_bits(bitstr, 8);

    track->vpcC->codecIntializationDataSize = read_bits(bitstr, 16);
    if (track->vpcC->codecIntializationDataSize > 0)
    {
        // Note: must be 0 for VP8 and VP9
    }

#if ENABLE_DEBUG
    TRACE_1(MP4, "> profile : %u", track->vpcC->profile);
    TRACE_1(MP4, "> level: %u", track->vpcC->level);
    TRACE_1(MP4, "> bitDepth: %u", track->vpcC->bitDepth);
    TRACE_1(MP4, "> chromaSubsampling: %u", track->vpcC->chromaSubsampling);
    TRACE_1(MP4, "> videoFullRangeFlag: %u", track->vpcC->videoFullRangeFlag);
    TRACE_1(MP4, "> colourPrimaries: %u", track->vpcC->colourPrimaries);
    TRACE_1(MP4, "> transferCharacteristics: %u", track->vpcC->transferCharacteristics);
    TRACE_1(MP4, "> matrixCoefficients: %u", track->vpcC->matrixCoefficients);

    TRACE_1(MP4, "> codecIntializationDataSize: %u", track->vpcC->codecIntializationDataSize);
    if (track->vpcC->codecIntializationDataSize)
    {
        // TODO
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        fprintf(mp4->xml, "  <profile>%u</profile>\n", track->vpcC->profile);
        fprintf(mp4->xml, "  <level>%u</level>\n", track->vpcC->level);
        fprintf(mp4->xml, "  <bitDepth>%u</bitDepth>\n", track->vpcC->bitDepth);
        fprintf(mp4->xml, "  <chromaSubsampling>%u</chromaSubsampling>\n", track->vpcC->chromaSubsampling);
        fprintf(mp4->xml, "  <videoFullRangeFlag>%u</videoFullRangeFlag>\n", track->vpcC->videoFullRangeFlag);
        fprintf(mp4->xml, "  <colourPrimaries>%u</colourPrimaries>\n", track->vpcC->colourPrimaries);
        fprintf(mp4->xml, "  <transferCharacteristics>%u</transferCharacteristics>\n", track->vpcC->transferCharacteristics);
        fprintf(mp4->xml, "  <matrixCoefficients>%u</matrixCoefficients>\n", track->vpcC->matrixCoefficients);

        fprintf(mp4->xml, "  <codecIntializationDataSize>%u</codecIntializationDataSize>\n", track->vpcC->codecIntializationDataSize);
        if (track->vpcC->codecIntializationDataSize)
        {
            // TODO
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AV1 Configuration Box.
 *
 * From 'AV1 Codec ISO Media File Format Binding':
 * v1.2.0, 12 December 2019
 * 2.3. AV1 Codec Configuration Box
 * - https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox
 */
int parse_av1C(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_av1C()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->av1C)
    {
        TRACE_WARNING(MKV, "parse_av1C() we aleady have an codecprivate_av1C_t structure!");
        return FAILURE;
    }
    track->av1C = (codecprivate_av1C_t *)calloc(1, sizeof(codecprivate_av1C_t));
    if (!track->av1C)
    {
        TRACE_ERROR(MKV, "parse_av1C() codecprivate_av1C_t allocation error!");
        return FAILURE;
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AV1 Configuration");

    // Parse box content
    track->av1C->marker = read_bit(bitstr);
    track->av1C->version = read_bits(bitstr, 7);
    track->av1C->seq_profile = read_bits(bitstr, 3);
    track->av1C->seq_level_idx_0 = read_bits(bitstr, 5);
    track->av1C->seq_tier_0 = read_bit(bitstr);
    track->av1C->high_bitdepth = read_bit(bitstr);
    track->av1C->twelve_bit = read_bit(bitstr);
    track->av1C->monochrome = read_bit(bitstr);
    track->av1C->chroma_subsampling_x = read_bit(bitstr);
    track->av1C->chroma_subsampling_y = read_bit(bitstr);
    track->av1C->chroma_sample_position = read_bits(bitstr, 2);
    read_bits(bitstr, 3); // reserved

    track->av1C->initial_presentation_delay_present = read_bit(bitstr);
    if (track->av1C->initial_presentation_delay_present)
    {
        track->av1C->initial_presentation_delay_minus_one = read_bits(bitstr, 4);
    }
    else
    {
        read_bits(bitstr, 4); // reserved
    }

    // TODO // Parse OBUs
    //unsigned int (8)[] configOBUs = read_bits(bitstr, 8);

#if ENABLE_DEBUG
    TRACE_1(MP4, "> version : %u", track->av1C->version);
    TRACE_1(MP4, "> seq_profile: %u", track->av1C->seq_profile);
    TRACE_1(MP4, "> seq_level_idx_0: %u", track->av1C->seq_level_idx_0);
    TRACE_1(MP4, "> seq_tier_0: %u", track->av1C->seq_tier_0);
    TRACE_1(MP4, "> high_bitdepth: %u", track->av1C->high_bitdepth);
    TRACE_1(MP4, "> twelve_bit: %u", track->av1C->twelve_bit);
    TRACE_1(MP4, "> monochrome: %u", track->av1C->monochrome);
    TRACE_1(MP4, "> chroma_subsampling_x: %u", track->av1C->chroma_subsampling_x);
    TRACE_1(MP4, "> chroma_subsampling_y: %u", track->av1C->chroma_subsampling_y);
    TRACE_1(MP4, "> chroma_sample_position: %u", track->av1C->chroma_sample_position);
    TRACE_1(MP4, "> initial_presentation_delay_present: %u", track->av1C->initial_presentation_delay_present);
    if (track->av1C->initial_presentation_delay_present)
    {
        TRACE_1(MP4, "> initial_presentation_delay_minus_one: %u", track->av1C->initial_presentation_delay_minus_one);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        fprintf(mp4->xml, "  <version>%u</version>\n", track->av1C->version);
        fprintf(mp4->xml, "  <seq_profile>%u</seq_profile>\n", track->av1C->seq_profile);
        fprintf(mp4->xml, "  <seq_level_idx_0>%u</seq_level_idx_0>\n", track->av1C->seq_level_idx_0);
        fprintf(mp4->xml, "  <seq_tier_0>%u</seq_tier_0>\n", track->av1C->seq_tier_0);
        fprintf(mp4->xml, "  <high_bitdepth>%u</high_bitdepth>\n", track->av1C->high_bitdepth);
        fprintf(mp4->xml, "  <twelve_bit>%i</twelve_bit>\n", track->av1C->twelve_bit);
        fprintf(mp4->xml, "  <monochrome>%u</monochrome>\n", track->av1C->monochrome);
        fprintf(mp4->xml, "  <chroma_subsampling_x>%u</chroma_subsampling_x>\n", track->av1C->chroma_subsampling_x);
        fprintf(mp4->xml, "  <chroma_subsampling_y>%u</chroma_subsampling_y>\n", track->av1C->chroma_subsampling_y);
        fprintf(mp4->xml, "  <chroma_sample_position>%u</chroma_sample_position>\n", track->av1C->chroma_sample_position);
        fprintf(mp4->xml, "  <initial_presentation_delay_present>%u</initial_presentation_delay_present>\n", track->av1C->initial_presentation_delay_present);
        if (track->av1C->initial_presentation_delay_present)
        {
            fprintf(mp4->xml, "  <initial_presentation_delay_minus_one>%u</initial_presentation_delay_minus_one>\n", track->av1C->initial_presentation_delay_minus_one);
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief JPGC Configuration Box.
 *
 * From 'AV1 Codec ISO Media File Format Binding':
 * v1.2.0, 12 December 2019
 * 2.3. AV1 Codec Configuration Box
 * - https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox
 */
int parse_jpgC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_jpgC()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
#if ENABLE_DEBUG
    print_box_header(box_header);
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "JPGC Configuration");
        // TODO
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief H.263 Configuration Box ('d263')
 *
 * 3GPP TS 26.244 specification
 */
int parse_d263(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_d263()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "H.263 parameters");

    char *vendor = read_mp4_string(bitstr, 4, mp4->xml, "vendor");
    free(vendor);

    uint8_t decoder_version = read_mp4_uint8(bitstr, mp4->xml, "decoder_version");
    uint8_t level = read_mp4_uint8(bitstr, mp4->xml, "level");
    uint8_t profil = read_mp4_uint8(bitstr, mp4->xml, "profil");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Bitrate Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_btrt(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_btrt()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int bufferSizeDB = read_bits(bitstr, 32);
    track->bitrate_max = read_bits(bitstr, 32);
    track->bitrate_avg = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> bufferSizeDB : %u", bufferSizeDB);
    TRACE_1(MP4, "> maxBitrate   : %u", track->bitrate_max);
    TRACE_1(MP4, "> avgBitrate   : %u", track->bitrate_avg);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Bitrate");
        fprintf(mp4->xml, "  <bufferSizeDB>%u</bufferSizeDB>\n", bufferSizeDB);
        fprintf(mp4->xml, "  <bitrate_max>%u</bitrate_max>\n", track->bitrate_max);
        fprintf(mp4->xml, "  <bitrate_avg>%u</bitrate_avg>\n", track->bitrate_avg);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Clean Aperture Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_clap(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_clap()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    //unsigned int clap_size = read_bits(bitstr, 32);
    //unsigned int clap_type = read_bits(bitstr, 32);

    unsigned int cleanApertureWidthN = read_bits(bitstr, 32);
    unsigned int cleanApertureWidthD = read_bits(bitstr, 32);
    unsigned int cleanApertureHeightN = read_bits(bitstr, 32);
    unsigned int cleanApertureHeightD = read_bits(bitstr, 32);
    unsigned int horizOffN = read_bits(bitstr, 32);
    unsigned int horizOffD = read_bits(bitstr, 32);
    unsigned int vertOffN = read_bits(bitstr, 32);
    unsigned int vertOffD = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    //TRACE_1(MP4, "> clap_size   : %u", clap_size);
    //TRACE_1(MP4, "> clap_type   : %u", clap_type);
    TRACE_1(MP4, "> cleanApertureWidthN   : %u", cleanApertureWidthN);
    TRACE_1(MP4, "> cleanApertureWidthD   : %u", cleanApertureWidthD);
    TRACE_1(MP4, "> cleanApertureHeightN  : %u", cleanApertureHeightN);
    TRACE_1(MP4, "> cleanApertureHeightD  : %u", cleanApertureHeightD);
    TRACE_1(MP4, "> horizOffN  : %u", horizOffN);
    TRACE_1(MP4, "> horizOffD  : %u", horizOffD);
    TRACE_1(MP4, "> vertOffN   : %u", vertOffN);
    TRACE_1(MP4, "> vertOffD   : %u", vertOffD);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Clean Aperture");
        fprintf(mp4->xml, "  <cleanApertureWidthN>%u</cleanApertureWidthN>\n", cleanApertureWidthN);
        fprintf(mp4->xml, "  <cleanApertureWidthD>%u</cleanApertureWidthD>\n", cleanApertureWidthD);
        fprintf(mp4->xml, "  <cleanApertureHeightN>%u</cleanApertureHeightN>\n", cleanApertureHeightN);
        fprintf(mp4->xml, "  <cleanApertureHeightD>%u</cleanApertureHeightD>\n", cleanApertureHeightD);
        fprintf(mp4->xml, "  <horizOffN>%u</horizOffN>\n", horizOffN);
        fprintf(mp4->xml, "  <horizOffD>%u</horizOffD>\n", horizOffD);
        fprintf(mp4->xml, "  <vertOffN>%u</vertOffN>\n", vertOffN);
        fprintf(mp4->xml, "  <vertOffD>%u</vertOffD>\n", vertOffD);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Colour Information Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 *
 * - https://developer.apple.com/library/mac/technotes/tn2227/_index.html
 */
int parse_colr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_colr()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Parse box content
    unsigned int colour_type = read_bits(bitstr, 32);
    unsigned int colour_primaries = 0;
    unsigned int transfer_characteristics = 0;
    unsigned int matrix_coefficients = 0;
    bool colour_range = 0;

    if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
    {
        colour_primaries = read_bits(bitstr, 16);
        transfer_characteristics = read_bits(bitstr, 16);
        matrix_coefficients = read_bits(bitstr, 16);

        if (colour_type == fourcc_be("nclx"))
        {
            colour_range = read_bit(bitstr);
            /*unsigned int reserved =*/ read_bits(bitstr, 7);

            if (track->color_range) track->color_range = colour_range;
        }

        if (track->color_matrix == 0 && track->color_matrix && track->color_matrix)
        {
            track->color_primaries = colour_primaries;
            track->color_transfer = transfer_characteristics;
            track->color_matrix = colour_primaries;
        }
    }
    else if (colour_type == fourcc_be("rICC"))
    {
        // TODO // restricted ICC profile
    }
    else if (colour_type == fourcc_be("prof"))
    {
        // TODO // unrestricted ICC profile
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> colour_type             : %s", getFccString_le(colour_type, fcc));
    if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
    {
        TRACE_1(MP4, "> colour_primaries        : %u", colour_primaries);
        TRACE_1(MP4, "> transfer_characteristics: %u", transfer_characteristics);
        TRACE_1(MP4, "> matrix_coefficients     : %u", matrix_coefficients);
    }
    if (colour_type == fourcc_be("nclx"))
    {
        TRACE_1(MP4, "> full_range_flag         : %u", track->color_range);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Colour Information");
        fprintf(mp4->xml, "  <colour_type>%s</colour_type>\n", getFccString_le(colour_type, fcc));
        if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
        {
            fprintf(mp4->xml, "  <colour_primaries>%u</colour_primaries>\n", colour_primaries);
            fprintf(mp4->xml, "  <transfer_characteristics>%u</transfer_characteristics>\n", transfer_characteristics);
            fprintf(mp4->xml, "  <matrix_coefficients>%u</matrix_coefficients>\n", matrix_coefficients);
        }
        if (colour_type == fourcc_be("nclx"))
        {
            fprintf(mp4->xml, "  <full_range_flag>%u</full_range_flag>\n", track->color_range);
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Content Light Level Box.
 *
 * From 'VP Codec ISO Media File Format Binding' specification:
 * Carriage of HDR Metadata
 * Content Light Level Box
 *
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_coll(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_coll()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int maxCLL = read_bits(bitstr, 16);
    unsigned int maxFALL = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> maxCLL : %u", maxCLL);
    TRACE_1(MP4, "> maxFALL: %u", maxFALL);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Content Light Level");
        fprintf(mp4->xml, "  <maxCLL>%u</maxCLL>\n", maxCLL);
        fprintf(mp4->xml, "  <maxFALL>%u</maxFALL>\n", maxFALL);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}
/* ************************************************************************** */

/*!
 * \brief SMPTE-2086 Mastering Display Metadata Box.
 *
 * From 'VP Codec ISO Media File Format Binding' specification:
 * Carriage of HDR Metadata
 * SMPTE-2086 Mastering Display Metadata Box
 *
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_smdm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_smdm()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned primaryRChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryRChromaticity_y = read_bits(bitstr, 16);
    unsigned primaryGChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryGChromaticity_y = read_bits(bitstr, 16);
    unsigned primaryBChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryBChromaticity_y = read_bits(bitstr, 16);
    unsigned whitePointChromaticity_x = read_bits(bitstr, 16);
    unsigned whitePointChromaticity_y = read_bits(bitstr, 16);
    unsigned luminanceMax = read_bits(bitstr, 32);
    unsigned luminanceMin = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> primaryRChromaticity_x: %u", primaryRChromaticity_x);
    TRACE_1(MP4, "> primaryRChromaticity_y: %u", primaryRChromaticity_y);
    TRACE_1(MP4, "> primaryGChromaticity_x: %u", primaryGChromaticity_x);
    TRACE_1(MP4, "> primaryGChromaticity_y: %u", primaryGChromaticity_y);
    TRACE_1(MP4, "> primaryBChromaticity_x: %u", primaryBChromaticity_x);
    TRACE_1(MP4, "> primaryBChromaticity_y: %u", primaryBChromaticity_y);
    TRACE_1(MP4, "> whitePointChromaticity_x: %u", whitePointChromaticity_x);
    TRACE_1(MP4, "> whitePointChromaticity_y: %u", whitePointChromaticity_y);
    TRACE_1(MP4, "> luminanceMax: %u", luminanceMax);
    TRACE_1(MP4, "> luminanceMin: %u", luminanceMin);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "SMPTE-2086 Mastering Display Metadata");
        fprintf(mp4->xml, "  <primaryRChromaticity_x>%u</primaryRChromaticity_x>\n", primaryRChromaticity_x);
        fprintf(mp4->xml, "  <primaryRChromaticity_y>%u</primaryRChromaticity_y>\n", primaryRChromaticity_y);
        fprintf(mp4->xml, "  <primaryGChromaticity_x>%u</primaryGChromaticity_x>\n", primaryGChromaticity_x);
        fprintf(mp4->xml, "  <primaryGChromaticity_y>%u</primaryGChromaticity_y>\n", primaryGChromaticity_y);
        fprintf(mp4->xml, "  <primaryBChromaticity_x>%u</primaryBChromaticity_x>\n", primaryBChromaticity_x);
        fprintf(mp4->xml, "  <primaryBChromaticity_y>%u</primaryBChromaticity_y>\n", primaryBChromaticity_y);
        fprintf(mp4->xml, "  <whitePointChromaticity_x>%u</whitePointChromaticity_x>\n", whitePointChromaticity_x);
        fprintf(mp4->xml, "  <whitePointChromaticity_y>%u</whitePointChromaticity_y>\n", whitePointChromaticity_y);
        fprintf(mp4->xml, "  <luminanceMax>%u</luminanceMax>\n", luminanceMax);
        fprintf(mp4->xml, "  <luminanceMin>%u</luminanceMin>\n", luminanceMin);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Field information box.
 *
 * Quick Time File Format - Video Sample Description Extensions
 */
int parse_fiel(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_fiel()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Field information");

    uint8_t fieldCount = read_mp4_uint8(bitstr, mp4->xml, "fieldCount");
    uint8_t fieldOrder = read_mp4_uint8(bitstr, mp4->xml, "fieldOrder");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Gamma box.
 *
 * Quick Time File Format - Video Sample Description Extensions
 */
int parse_gama(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gama()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Gamma");

    read_mp4_f1616(bitstr, 1, mp4->xml, "GammaValue");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Chromaticity box.
 */
int parse_chrm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_chrm()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Chromaticity");

    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Red X");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Red Y");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Green X");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Green Y");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Blue X");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "Primary Blue Y");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "White Point X");
    //read_mp4_f1616(bitstr, 4, mp4->xml, "White Point Y");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Padding Bits box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.6 Padding Bits Box.
 */
int parse_padb(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_padb()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i;
    unsigned int sample_count = read_bits(bitstr, 32);

    for (i = 0; i < ((sample_count + 1)/2); i++)
    {
        /*const int reserved1 =*/ read_bit(bitstr);
        /*int pad1 =*/ read_bits(bitstr, 3);
        /*const int reserved2 =*/ read_bit(bitstr);
        /*int pad2 =*/ read_bits(bitstr, 3);
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> sample_count  : %u", sample_count);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Padding Bits");
        fprintf(mp4->xml, "  <sample_count>%u</sample_count>\n", sample_count);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Pixel Aspect Ratio Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_pasp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pasp()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    track->par_h = read_bits(bitstr, 32);
    track->par_v = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> hSpacing  : %u", track->par_h);
    TRACE_1(MP4, "> vSpacing  : %u", track->par_v);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Pixel Aspect Ratio");
        fprintf(mp4->xml, "  <hSpacing>%u</hSpacing>\n", track->par_h);
        fprintf(mp4->xml, "  <vSpacing>%u</vSpacing>\n", track->par_v);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */
