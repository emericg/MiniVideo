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
 * \file      mkv_codec.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_codec.h"
#include "mkv_struct.h"
#include "ebml.h"

#include "../../decoder/h264/h264_parameterset.h"

#include "../xml_mapper.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

/* ************************************************************************** */

void mkv_codec_from_string(char *codec_str, Codecs_e *codec, CodecProfiles_e *profile)
{
    if (!codec_str || !codec || !profile)
        return;

    *codec = CODEC_UNKNOWN;
    *profile = (CodecProfiles_e)PROF_UNKNOWN;

    if (strncmp(codec_str, "A_", 2) == 0) //////////////////////////////////////
    {
        if (strncmp(codec_str, "A_AAC", 5) == 0)
        {
            *codec = CODEC_AAC;

            if (strncmp(codec_str, "A_AAC/MPEG2", 11) == 0)
            {
                if (strcmp(codec_str, "A_AAC/MPEG2/MAIN") == 0)
                {
                    *profile = PROF_AAC_Main;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG2/LC") == 0)
                {
                    *profile = PROF_AAC_LC;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG2/LC/SBR") == 0)
                {
                    *profile = PROF_AAC_HE;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG2/SSR") == 0)
                {
                    *profile = PROF_AAC_SSR;
                }
            }
            else if (strncmp(codec_str, "A_AAC/MPEG4", 11) == 0)
            {
                if (strcmp(codec_str, "A_AAC/MPEG4/MAIN") == 0)
                {
                    *profile = PROF_AAC_Main;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG4/LC") == 0)
                {
                    *profile = PROF_AAC_LC;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG4/LC/SBR") == 0)
                {
                    *profile = PROF_AAC_HE;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG4/SSR") == 0)
                {
                    *profile = PROF_AAC_SSR;
                }
                else if (strcmp(codec_str, "A_AAC/MPEG4/LTP") == 0)
                {
                    *profile = PROF_AAC_LTP;
                }
            }
        }
        else if (strncmp(codec_str, "A_MPEG", 6) == 0)
        {
            if (strcmp(codec_str, "A_MPEG/L3") == 0)
            {
                *codec = CODEC_MPEG_L3;
            }
            else if (strcmp(codec_str, "A_MPEG/L2") == 0)
            {
                *codec = CODEC_MPEG_L2;
            }
            else if (strcmp(codec_str, "A_MPEG/L1") == 0)
            {
                *codec = CODEC_MPEG_L1;
            }
        }
        else if (strncmp(codec_str, "A_PCM", 5) == 0)
        {
            if (strcmp(codec_str, "A_PCM/INT/BIG") == 0)
            {
                *codec = CODEC_LPCM;
            }
            else if (strcmp(codec_str, "A_PCM/INT/LIT") == 0)
            {
                *codec = CODEC_LPCM;
            }
            else if (strcmp(codec_str, "A_PCM/FLOAT/IEEE") == 0)
            {
                *codec = CODEC_LPCM;
            }
        }
        else if (strcmp(codec_str, "A_MPC") == 0)
        {
            *codec = CODEC_MPC;
        }
        else if (strncmp(codec_str, "A_AC3", 5) == 0)
        {
            *codec = CODEC_AC3;

            //A_AC3/BSID9
            //A_AC3/BSID10
        }
        else if (strcmp(codec_str, "A_EAC3") == 0)
        {
            *codec = CODEC_EAC3;
        }
        else if (strcmp(codec_str, "A_AC4") == 0)
        {
            *codec = CODEC_AC4;
        }
        else if (strcmp(codec_str, "A_TRUEHD") == 0)
        {
            *codec = CODEC_DolbyTrueHD;
        }
        else if (strcmp(codec_str, "A_ALAC") == 0)
        {
            *codec = CODEC_ALAC;
        }
        else if (strncmp(codec_str, "A_DTS", 5) == 0)
        {
            *codec = CODEC_DTS;

            //A_DTS/EXPRESS
            //A_DTS/LOSSLESS
        }
        else if (strcmp(codec_str, "A_VORBIS") == 0)
        {
            *codec = CODEC_VORBIS;
        }
        else if (strcmp(codec_str, "A_OPUS") == 0)
        {
            *codec = CODEC_OPUS;
        }
        else if (strcmp(codec_str, "A_FLAC") == 0)
        {
            *codec = CODEC_FLAC;
        }
        else if (strncmp(codec_str, "A_REAL", 6) == 0)
        {
            if (strcmp(codec_str, "A_REAL/14_4") == 0)
            {
                *codec = CODEC_RA_14;
            }
            else if (strcmp(codec_str, "A_REAL/28_8") == 0)
            {
                *codec = CODEC_RA_28;
            }
            else if (strcmp(codec_str, "A_REAL/SIPR") == 0)
            {
                *codec = CODEC_UNKNOWN; // Sipro Voice Codec
            }
            else if (strcmp(codec_str, "A_REAL/COOK") == 0)
            {
                *codec = CODEC_RA_cook;
            }
            else if (strcmp(codec_str, "A_REAL/RALF") == 0)
            {
                *codec = CODEC_RA_ralf;
            }
            else if (strcmp(codec_str, "A_REAL/ATRC") == 0)
            {
                *codec = CODEC_ATRAC3plus;
            }
        }
        else if (strcmp(codec_str, "A_WAVPACK4") == 0)
        {
            *codec = CODEC_WAVPACK;
        }
        else if (strcmp(codec_str, "A_ATRAC/AT1") == 0)
        {
            *codec = CODEC_ATRAC;
        }

        // Not handled:
        // A_MS/ACM
        // A_QUICKTIME
        // A_QUICKTIME/QDMC
        // A_QUICKTIME/QDM2
        // A_TTA1
    }
    else if (strncmp(codec_str, "V_", 2) == 0) /////////////////////////////////
    {
        if (strncmp(codec_str, "V_MPEG", 6) == 0)
        {
            if (strncmp(codec_str, "V_MPEG4/ISO", 11) == 0)
            {
                if (strcmp(codec_str, "V_MPEG4/ISO/SP") == 0)
                {
                    *codec = CODEC_MPEG4_ASP;
                    *profile = PROF_MPEG4_SP;
                }
                else if (strcmp(codec_str, "V_MPEG4/ISO/ASP") == 0)
                {
                    *codec = CODEC_MPEG4_ASP;
                    *profile = PROF_MPEG4_ASP;
                }
                else if (strcmp(codec_str, "V_MPEG4/ISO/AP") == 0)
                {
                    *codec = CODEC_MPEG4_ASP;
                    *profile = PROF_MPEG4_AP;
                }
                else if (strcmp(codec_str, "V_MPEG4/ISO/AVC") == 0)
                {
                    *codec = CODEC_H264;
                }
            }
            else if (strcmp(codec_str, "V_MPEGH/ISO/HEVC") == 0)
            {
                *codec = CODEC_H265;
            }
            else if (strcmp(codec_str, "V_MPEGI/ISO/VVC") == 0)
            {
                *codec = CODEC_H266;
            }
            else if (strcmp(codec_str, "V_MPEG2") == 0)
            {
                *codec = CODEC_H262;
            }
            else if (strcmp(codec_str, "V_MPEG1") == 0)
            {
                *codec = CODEC_MPEG1;
            }
            else if (strcmp(codec_str, "V_MPEG4/MS/V3") == 0)
            {
                *codec = CODEC_MSMPEG4;
            }
        }
        else if (strncmp(codec_str, "V_VP", 4) == 0)
        {
            if (strcmp(codec_str, "V_VP9") == 0)
            {
                *codec = CODEC_VP9;
            }
            else if (strcmp(codec_str, "V_VP8") == 0)
            {
                *codec = CODEC_VP8;
            }
            else if (strcmp(codec_str, "V_VP7") == 0)
            {
                *codec = CODEC_VP7;
            }
            else if (strcmp(codec_str, "V_VP6") == 0)
            {
                *codec = CODEC_VP6;
            }
        }
        else if (strcmp(codec_str, "V_AV1") == 0)
        {
            *codec = CODEC_AV1;
        }
        else if (strcmp(codec_str, "V_AVS1") == 0)
        {
            *codec = CODEC_AVS1;
        }
        else if (strcmp(codec_str, "V_AVS2") == 0)
        {
            *codec = CODEC_AVS2;
        }
        else if (strcmp(codec_str, "V_AVS3") == 0)
        {
            *codec = CODEC_AVS3;
        }
        else if (strncmp(codec_str, "V_REAL", 6) == 0)
        {
            if (strcmp(codec_str, "V_REAL/RV10") == 0)
            {
                *codec = CODEC_RV10;
            }
            else if (strcmp(codec_str, "V_REAL/RV20") == 0)
            {
                *codec = CODEC_RV20;
            }
            else if (strcmp(codec_str, "V_REAL/RV30") == 0)
            {
                *codec = CODEC_RV30;
            }
            else if (strcmp(codec_str, "V_REAL/RV40") == 0)
            {
                *codec = CODEC_RV40;
            }
        }
        else if (strcmp(codec_str, "V_THEORA") == 0)
        {
            *codec = CODEC_VP4;
        }
        else if (strcmp(codec_str, "V_PRORES") == 0)
        {
            *codec = CODEC_PRORES_422; // Actually we don't know about what codec version is used
        }
        else if (strcmp(codec_str, "V_FFV1") == 0)
        {
            *codec = CODEC_FFV1;
        }

        // Not handled:
        // V_MS/VFW/FOURCC
        // V_UNCOMPRESSED
        // V_QUICKTIME
    }
    else if (strncmp(codec_str, "S_", 2) == 0) /////////////////////////////////
    {
        if (strcmp(codec_str, "S_TEXT/UTF8") == 0)
        {
            *codec = CODEC_SRT;
        }
        else if (strcmp(codec_str, "S_TEXT/SSA") == 0)
        {
            *codec = CODEC_SSA;
        }
        else if (strcmp(codec_str, "S_TEXT/ASS") == 0)
        {
            *codec = CODEC_ASS;
        }
        else if (strcmp(codec_str, "S_TEXT/USF") == 0)
        {
            *codec = CODEC_USF;
        }
        else if (strcmp(codec_str, "S_TEXT/WEBVTT") == 0)
        {
            *codec = CODEC_WebVTT;
        }
        else if (strcmp(codec_str, "S_VOBSUB") == 0)
        {
            *codec = CODEC_VobSub;
        }
        else if (strcmp(codec_str, "S_DVBSUB") == 0)
        {
            *codec = CODEC_DvbSub;
        }
        else if (strcmp(codec_str, "S_ARIBSUB") == 0)
        {
            *codec = CODEC_AriSub;
        }
        else if (strcmp(codec_str, "S_KATE") == 0)
        {
            *codec = CODEC_Kate;
        }
        else if (strcmp(codec_str, "S_HDMV/PGS") == 0)
        {
            *codec = CODEC_PGS;
        }
        else if (strcmp(codec_str, "S_HDMV/TEXTST") == 0)
        {
            *codec = CODEC_TextST;
        }

        // Not handled:
        // S_IMAGE/BMP
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_codec_private(Bitstream_t *bitstr, EbmlElement_t *element, /*mkv_track_t *track,*/ mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_codec_private()" CLR_RESET);
    int retcode = SUCCESS;

#if ENABLE_DEBUG
    TRACE_1(MKV, "> Unknown codec private data");
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"CodecPrivate\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                element->offset_start, element->size);

        xmlSpacer(mkv->xml, "Unknown codec private data", -1);

        fprintf(mkv->xml, "  </a>");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AVC Configuration.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.2 Syntax, 5.2.4.1.2 Semantics).
 */
int parse_h264_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_h264_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->avcC)
    {
        TRACE_WARNING(MKV, "parse_h264_private() we aleady have an codecprivate_avcC_t structure!");
        return FAILURE;
    }
    track->avcC = (codecprivate_avcC_t *)calloc(1, sizeof(codecprivate_avcC_t));
    if (!track->avcC)
    {
        TRACE_ERROR(MKV, "parse_h264_private() codecprivate_avcC_t allocation error!");
        return FAILURE;
    }

    // Parse box content
    track->avcC->configurationVersion = read_bits(bitstr, 8);
    track->avcC->AVCProfileIndication = read_bits(bitstr, 8);
    track->avcC->profile_compatibility = read_bits(bitstr, 8);
    track->avcC->AVCLevelIndication = read_bits(bitstr, 8);
    /*int reserved =*/ read_bits(bitstr, 6);
    track->avcC->lengthSizeMinusOne = read_bits(bitstr, 2);
    /*int reserved =*/ read_bits(bitstr, 3);

    // SPS
    track->avcC->sps_count = read_bits(bitstr, 5);
    track->avcC->sps_sample_offset = (int64_t *)calloc(track->avcC->sps_count, sizeof(int64_t));
    track->avcC->sps_sample_size = (int32_t *)calloc(track->avcC->sps_count, sizeof(int32_t));
    //avcC->sps_sample_offset = new int64_t [track->avcC->sps_count];
    //avcC->sps_sample_size = new int32_t[track->avcC->sps_count];

    for (unsigned i = 0; i < track->avcC->sps_count && i < MAX_SPS; i++) // MAX_SPS = 32
    {
        track->avcC->sps_sample_size[i] = read_bits(bitstr, 16);
        track->avcC->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

        track->avcC->sps_array[i] = (h264_sps_t *)calloc(1, sizeof(h264_sps_t));
        //track->avcC->sps_array[i] = new sps_t;

        skip_bits(bitstr, 8); // skip NAL header
        decodeSPS(bitstr, track->avcC->sps_array[i]);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]))
        {
            TRACE_WARNING(MKV, "SPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]));

            skip_bits(bitstr, ((track->avcC->sps_sample_offset[i] + track->avcC->sps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // PPS
    track->avcC->pps_count = read_bits(bitstr, 8);
    track->avcC->pps_sample_offset = (int64_t *)calloc(track->avcC->pps_count, sizeof(int64_t));
    track->avcC->pps_sample_size = (int32_t *)calloc(track->avcC->pps_count, sizeof(int32_t));
    //avcC->pps_sample_offset = new int64_t[track->avcC->pps_count];
    //avcC->pps_sample_size = new int32_t[track->avcC->pps_count];

    for (unsigned i = 0; i < track->avcC->pps_count && i < MAX_PPS; i++) // MAX_PPS = 256
    {
       track->avcC->pps_sample_size[i] = read_bits(bitstr, 16);
       track->avcC->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

       track->avcC->pps_array[i] = (h264_pps_t *)calloc(1, sizeof(h264_pps_t));
       //track->avcC->pps_array[i] = new h264_pps_t;

       skip_bits(bitstr, 8); // skip NAL header
       decodePPS(bitstr, track->avcC->pps_array[i], track->avcC->sps_array);
       bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]))
        {
            TRACE_WARNING(MKV, "PPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]));
            skip_bits(bitstr, ((track->avcC->pps_sample_offset[i] + track->avcC->pps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

#if ENABLE_DEBUG
    TRACE_1(MKV, "> configurationVersion  : %u", track->avcC->configurationVersion);
    TRACE_1(MKV, "> AVCProfileIndication  : %u", track->avcC->AVCProfileIndication);
    TRACE_1(MKV, "> profile_compatibility : %u", track->avcC->profile_compatibility);
    TRACE_1(MKV, "> AVCLevelIndication    : %u", track->avcC->AVCLevelIndication);
    TRACE_1(MKV, "> lengthSizeMinusOne    : %u", track->avcC->lengthSizeMinusOne);

    TRACE_1(MKV, "> numOfSequenceParameterSets    = %u", track->avcC->sps_count);
    for (unsigned i = 0; i < track->avcC->sps_count; i++)
    {
        TRACE_1(MKV, "> sequenceParameterSetLength[%u] : %u", i, track->avcC->sps_sample_size[i]);
        TRACE_1(MKV, "> sequenceParameterSetOffset[%u] : %li", i, track->avcC->sps_sample_offset[i]);
    }

    TRACE_1(MKV, "> numOfPictureParameterSets     = %u", track->avcC->pps_count);
    for (unsigned i = 0; i < track->avcC->pps_count; i++)
    {
        TRACE_1(MKV, "> pictureParameterSetLength[%u]  : %u", i, track->avcC->pps_sample_size[i]);
        TRACE_1(MKV, "> pictureParameterSetOffset[%u]  : %li", i, track->avcC->pps_sample_offset[i]);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"AVC Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->CodecPrivate_offset, track->CodecPrivate_size);

        xmlSpacer(mkv->xml, "AVC Configuration", -1);

        fprintf(mkv->xml, "  <configurationVersion>%u</configurationVersion>\n", track->avcC->configurationVersion);
        fprintf(mkv->xml, "  <AVCProfileIndication>%u</AVCProfileIndication>\n", track->avcC->AVCProfileIndication);
        fprintf(mkv->xml, "  <profile_compatibility>%u</profile_compatibility>\n", track->avcC->profile_compatibility);
        fprintf(mkv->xml, "  <AVCLevelIndication>%u</AVCLevelIndication>\n", track->avcC->AVCLevelIndication);
        fprintf(mkv->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", track->avcC->lengthSizeMinusOne);

        fprintf(mkv->xml, "  <numOfSequenceParameterSets>%u</numOfSequenceParameterSets>\n", track->avcC->sps_count);
        fprintf(mkv->xml, "  <numOfPictureParameterSets>%u</numOfPictureParameterSets>\n", track->avcC->pps_count);

        for (unsigned i = 0; i < track->avcC->sps_count; i++)
        {
            xmlSpacer(mkv->xml, "SequenceParameterSet infos", i);
            fprintf(mkv->xml, "  <sequenceParameterSetLength index=\"%u\">%u</sequenceParameterSetLength>\n", i, track->avcC->sps_sample_size[i]);
            fprintf(mkv->xml, "  <sequenceParameterSetOffset index=\"%u\">%" PRId64 "</sequenceParameterSetOffset>\n", i, track->avcC->sps_sample_offset[i]);
        }

        for (unsigned i = 0; i < track->avcC->pps_count; i++)
        {
            xmlSpacer(mkv->xml, "PictureParameterSet", i);
            fprintf(mkv->xml, "  <pictureParameterSetLength index=\"%u\">%u</pictureParameterSetLength>\n", i, track->avcC->pps_sample_size[i]);
            fprintf(mkv->xml, "  <pictureParameterSetOffset index=\"%u\">%" PRId64 "</pictureParameterSetOffset>\n", i, track->avcC->pps_sample_offset[i]);
        }

        fprintf(mkv->xml, "  </a>\n");

        for (unsigned i = 0; i < track->avcC->sps_count; i++)
        {
            printSPS(track->avcC->sps_array[i]);

            mapSPS(track->avcC->sps_array[i],
                   track->avcC->sps_sample_offset[i],
                   track->avcC->sps_sample_size[i],
                   mkv->xml);
        }
        for (unsigned i = 0; i < track->avcC->pps_count; i++)
        {
            printPPS(track->avcC->pps_array[i], track->avcC->sps_array);

            mapPPS(track->avcC->pps_array[i],
                   track->avcC->sps_array,
                   track->avcC->pps_sample_offset[i],
                   track->avcC->pps_sample_size[i],
                   mkv->xml);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief HEVC Configuration.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 8.3.3 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC23008-2 video content.
 * Contain HEVCDecoderConfigurationRecord data structure (8.3.3.1.2 Syntax, 8.3.3.1.3 Semantics).
 */
int parse_h265_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_h265_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->hvcC)
    {
        TRACE_WARNING(MKV, "parse_h265_private() we aleady have an codecprivate_hvcC_t structure!");
        return FAILURE;
    }
    track->hvcC = (codecprivate_hvcC_t *)calloc(1, sizeof(codecprivate_hvcC_t));
    if (!track->hvcC)
    {
        TRACE_ERROR(MKV, "parse_h265_private() codecprivate_hvcC_t allocation error!");
        return FAILURE;
    }

    // Parse box content
    track->hvcC->configurationVersion = read_bits(bitstr, 8);
    track->hvcC->general_profile_space = read_bits(bitstr, 2);
    track->hvcC->general_tier_flag = read_bit(bitstr);
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
    track->hvcC->temporalIdNested = read_bit(bitstr);
    track->hvcC->lengthSizeMinusOne = read_bit(bitstr);

    track->hvcC->numOfArrays = read_bit(bitstr);
    for (unsigned i = 0; i < track->hvcC->numOfArrays; i++)
    {
        // TODO // NAL unit table
    }

#if ENABLE_DEBUG
    TRACE_1(MKV, "> configurationVersion    : %u", track->hvcC->configurationVersion);
    TRACE_1(MKV, "> general_profile_space   : %u", track->hvcC->general_profile_space);
    TRACE_1(MKV, "> general_tier_flag       : %u", track->hvcC->general_tier_flag);
    TRACE_1(MKV, "> general_profile_idc     : %u", track->hvcC->general_profile_idc);
    TRACE_1(MKV, "> general_profile_compatibility_flags : %u", track->hvcC->general_profile_compatibility_flags);
    TRACE_1(MKV, "> general_constraint_indicator_flags  : %lu", track->hvcC->general_constraint_indicator_flags);
    TRACE_1(MKV, "> general_level_idc       : %u", track->hvcC->general_level_idc);

    TRACE_1(MKV, "> min_spatial_segmentation_idc: %u", track->hvcC->min_spatial_segmentation_idc);
    TRACE_1(MKV, "> parallelismType         : %u", track->hvcC->parallelismType);
    TRACE_1(MKV, "> chromaFormat            : %u", track->hvcC->chromaFormat);
    TRACE_1(MKV, "> bitDepthLumaMinus8      : %u", track->hvcC->bitDepthLumaMinus8);
    TRACE_1(MKV, "> bitDepthChromaMinus8    : %u", track->hvcC->bitDepthChromaMinus8);

    TRACE_1(MKV, "> avgFrameRate            : %u", track->hvcC->avgFrameRate);
    TRACE_1(MKV, "> constantFrameRate       : %u", track->hvcC->constantFrameRate);
    TRACE_1(MKV, "> numTemporalLayers       : %u", track->hvcC->numTemporalLayers);
    TRACE_1(MKV, "> temporalIdNested        : %u", track->hvcC->temporalIdNested);
    TRACE_1(MKV, "> lengthSizeMinusOne      : %u", track->hvcC->lengthSizeMinusOne);
    TRACE_1(MKV, "> numOfArrays             : %u", track->hvcC->numOfArrays);

    for (unsigned j = 0; j < track->hvcC->numOfArrays; j++)
    {
        // TODO // NAL unit table
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"HEVC Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->CodecPrivate_offset, track->CodecPrivate_size);

        xmlSpacer(mkv->xml, "HEVC Configuration", -1);

        fprintf(mkv->xml, "  <configurationVersion>%u</configurationVersion>\n", track->hvcC->configurationVersion);
        fprintf(mkv->xml, "  <general_profile_space>%u</general_profile_space>\n", track->hvcC->general_profile_space);
        fprintf(mkv->xml, "  <general_tier_flag>%u</general_tier_flag>\n", track->hvcC->general_tier_flag);
        fprintf(mkv->xml, "  <general_profile_idc>%u</general_profile_idc>\n", track->hvcC->general_profile_idc);
        fprintf(mkv->xml, "  <general_profile_compatibility_flags>%u</general_profile_compatibility_flags>\n", track->hvcC->general_profile_compatibility_flags);
        fprintf(mkv->xml, "  <general_constraint_indicator_flags>%" PRIu64 "</general_constraint_indicator_flags>\n", track->hvcC->general_constraint_indicator_flags);
        fprintf(mkv->xml, "  <general_level_idc>%u</general_level_idc>\n", track->hvcC->general_level_idc);

        fprintf(mkv->xml, "  <min_spatial_segmentation_idc>%u</min_spatial_segmentation_idc>\n", track->hvcC->min_spatial_segmentation_idc);
        fprintf(mkv->xml, "  <parallelismType>%u</parallelismType>\n", track->hvcC->parallelismType);
        fprintf(mkv->xml, "  <chromaFormat>%u</chromaFormat>\n", track->hvcC->chromaFormat);
        fprintf(mkv->xml, "  <bitDepthLumaMinus8>%u</bitDepthLumaMinus8>\n", track->hvcC->bitDepthLumaMinus8);
        fprintf(mkv->xml, "  <bitDepthChromaMinus8>%u</bitDepthChromaMinus8>\n", track->hvcC->bitDepthChromaMinus8);

        fprintf(mkv->xml, "  <avgFrameRate>%u</avgFrameRate>\n", track->hvcC->avgFrameRate);
        fprintf(mkv->xml, "  <constantFrameRate>%u</constantFrameRate>\n", track->hvcC->constantFrameRate);
        fprintf(mkv->xml, "  <numTemporalLayers>%u</numTemporalLayers>\n", track->hvcC->numTemporalLayers);
        fprintf(mkv->xml, "  <temporalIdNested>%u</temporalIdNested>\n", track->hvcC->temporalIdNested);
        fprintf(mkv->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", track->hvcC->lengthSizeMinusOne);

        fprintf(mkv->xml, "  <numOfArrays>%u</numOfArrays>\n", track->hvcC->numOfArrays);

        fprintf(mkv->xml, "  </a>\n");

        for (unsigned j = 0; j < track->hvcC->numOfArrays; j++)
        {
            // TODO // NAL unit table
        }
    }

    return retcode;
}

/* ************************************************************************** */

int parse_h266_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_h266_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->vvcC)
    {
        TRACE_WARNING(MKV, "parse_h266_private() we aleady have an codecprivate_vvcC_t structure!");
        return FAILURE;
    }
    track->vvcC = (codecprivate_vvcC_t *)calloc(1, sizeof(codecprivate_vvcC_t));
    if (!track->vvcC)
    {
        TRACE_ERROR(MKV, "parse_h266_private() codecprivate_vvcC_t allocation error!");
        return FAILURE;
    }

    // Parse box content
    // TODO

#if ENABLE_DEBUG
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        xmlSpacer(mkv->xml, "VVC Configuration", -1);
        fprintf(mkv->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief VPx Configuration.
 */
int parse_vpx_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_vpx_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->vpcC)
    {
        TRACE_WARNING(MKV, "parse_vpx_private() we aleady have an codecprivate_vpcC_t structure!");
        return FAILURE;
    }
    track->vpcC = (codecprivate_vpcC_t *)calloc(1, sizeof(codecprivate_vpcC_t));
    if (!track->vpcC)
    {
        TRACE_ERROR(MKV, "parse_vpx_private() codecprivate_vpcC_t allocation error!");
        return FAILURE;
    }

    track->vpcC->profile = read_bits(bitstr, 8);
    track->vpcC->level = read_bits(bitstr, 8);
    track->vpcC->bitDepth = read_bits(bitstr, 4);
    track->vpcC->chromaSubsampling = read_bits(bitstr, 3);
    track->vpcC->videoFullRangeFlag = read_bit(bitstr);
    track->vpcC->colourPrimaries = read_bits(bitstr, 8);
    track->vpcC->transferCharacteristics = read_bits(bitstr, 8);
    track->vpcC->matrixCoefficients = read_bits(bitstr, 8);

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
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"VPx Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->CodecPrivate_offset, track->CodecPrivate_size);

        fprintf(mkv->xml, "  <profile>%u</profile>\n", track->vpcC->profile);
        fprintf(mkv->xml, "  <level>%u</level>\n", track->vpcC->level);
        fprintf(mkv->xml, "  <bitDepth>%u</bitDepth>\n", track->vpcC->bitDepth);
        fprintf(mkv->xml, "  <chromaSubsampling>%u</chromaSubsampling>\n", track->vpcC->chromaSubsampling);
        fprintf(mkv->xml, "  <videoFullRangeFlag>%u</videoFullRangeFlag>\n", track->vpcC->videoFullRangeFlag);
        fprintf(mkv->xml, "  <colourPrimaries>%u</colourPrimaries>\n", track->vpcC->colourPrimaries);
        fprintf(mkv->xml, "  <transferCharacteristics>%u</transferCharacteristics>\n", track->vpcC->transferCharacteristics);
        fprintf(mkv->xml, "  <matrixCoefficients>%u</matrixCoefficients>\n", track->vpcC->matrixCoefficients);

        fprintf(mkv->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AV1 Configuration.
 */
int parse_av1_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_av1_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->av1C)
    {
        TRACE_WARNING(MKV, "parse_av1_private() we aleady have an codecprivate_av1C_t structure!");
        return FAILURE;
    }
    track->av1C = (codecprivate_av1C_t *)calloc(1, sizeof(codecprivate_av1C_t));
    if (!track->av1C)
    {
        TRACE_ERROR(MKV, "parse_av1_private() codecprivate_av1C_t allocation error!");
        return FAILURE;
    }

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
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"AV1 Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->CodecPrivate_offset, track->CodecPrivate_size);

        fprintf(mkv->xml, "  <version>%u</version>\n", track->av1C->version);
        fprintf(mkv->xml, "  <seq_profile>%u</seq_profile>\n", track->av1C->seq_profile);
        fprintf(mkv->xml, "  <seq_level_idx_0>%u</seq_level_idx_0>\n", track->av1C->seq_level_idx_0);
        fprintf(mkv->xml, "  <seq_tier_0>%u</seq_tier_0>\n", track->av1C->seq_tier_0);
        fprintf(mkv->xml, "  <high_bitdepth>%u</high_bitdepth>\n", track->av1C->high_bitdepth);
        fprintf(mkv->xml, "  <twelve_bit>%i</twelve_bit>\n", track->av1C->twelve_bit);
        fprintf(mkv->xml, "  <monochrome>%u</monochrome>\n", track->av1C->monochrome);
        fprintf(mkv->xml, "  <chroma_subsampling_x>%u</chroma_subsampling_x>\n", track->av1C->chroma_subsampling_x);
        fprintf(mkv->xml, "  <chroma_subsampling_y>%u</chroma_subsampling_y>\n", track->av1C->chroma_subsampling_y);
        fprintf(mkv->xml, "  <chroma_sample_position>%u</chroma_sample_position>\n", track->av1C->chroma_sample_position);
        fprintf(mkv->xml, "  <initial_presentation_delay_present>%u</initial_presentation_delay_present>\n", track->av1C->initial_presentation_delay_present);
        if (track->av1C->initial_presentation_delay_present)
        {
            fprintf(mkv->xml, "  <initial_presentation_delay_minus_one>%u</initial_presentation_delay_minus_one>\n",
                    track->av1C->initial_presentation_delay_minus_one);
        }
        fprintf(mkv->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Dolby Vision Configuration.
 */
int parse_dolbyvision_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_dolbyvision_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->dvcC)
    {
        TRACE_WARNING(MKV, "parse_dolbyvision_private() we aleady have an codecprivate_dvcC_t structure!");
        return FAILURE;
    }
    track->dvcC = (codecprivate_dvcC_t *)calloc(1, sizeof(codecprivate_dvcC_t));
    if (!track->dvcC)
    {
        TRACE_ERROR(MKV, "parse_dolbyvision_private() codecprivate_dvcC_t allocation error!");
        return FAILURE;
    }

    // Parse box content
    track->dvcC->dv_version_major = read_bits(bitstr, 8);
    track->dvcC->dv_version_minor = read_bits(bitstr, 8);
    track->dvcC->dv_profile = read_bits(bitstr, 7);
    track->dvcC->dv_level = read_bits(bitstr, 6);
    track->dvcC->rpu_present_flag = read_bit(bitstr);
    track->dvcC->el_present_flag = read_bit(bitstr);
    track->dvcC->bl_present_flag = read_bit(bitstr);
    if (track->dvcC->bl_present_flag)
    {
        track->dvcC->dv_bl_signal_compatibility_id = read_bits(bitstr, 4);
    }

#if ENABLE_DEBUG
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"Dolby Vision Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->BlockAdditionExtra_offset, track->BlockAdditionExtra_size);

        fprintf(mkv->xml, "  <dv_version_major>%u</dv_version_major>\n", track->dvcC->dv_version_major);
        fprintf(mkv->xml, "  <dv_version_minor>%u</dv_version_minor>\n", track->dvcC->dv_version_minor);
        fprintf(mkv->xml, "  <dv_profile>%u</dv_profile>\n", track->dvcC->dv_profile);
        fprintf(mkv->xml, "  <dv_level>%u</dv_level>\n", track->dvcC->dv_level);
        fprintf(mkv->xml, "  <rpu_present_flag>%u</rpu_present_flag>\n", track->dvcC->rpu_present_flag);
        fprintf(mkv->xml, "  <el_present_flag>%u</el_present_flag>\n", track->dvcC->el_present_flag);
        fprintf(mkv->xml, "  <bl_present_flag>%u</bl_present_flag>\n", track->dvcC->bl_present_flag);
        if (track->dvcC->bl_present_flag)
        {
            fprintf(mkv->xml, "  <dv_bl_signal_compatibility_id>%u</dv_bl_signal_compatibility_id>\n", track->dvcC->dv_bl_signal_compatibility_id);
        }

        fprintf(mkv->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief MultiView 3D Configuration.
 */
int parse_mvc_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_mvc_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Init
    if (track->mvcC)
    {
        TRACE_WARNING(MKV, "parse_mvc_private() we aleady have an codecprivate_mvcC_t structure!");
        return FAILURE;
    }
    track->mvcC = (codecprivate_mvcC_t *)calloc(1, sizeof(codecprivate_mvcC_t));
    if (!track->mvcC)
    {
        TRACE_ERROR(MKV, "parse_mvc_private() codecprivate_mvcC_t allocation error!");
        return FAILURE;
    }

    // TODO

#if ENABLE_DEBUG
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        fprintf(mkv->xml, "  <a tt=\"MVC 3D Configuration\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                track->BlockAdditionExtra_offset, track->BlockAdditionExtra_size);

        fprintf(mkv->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */
