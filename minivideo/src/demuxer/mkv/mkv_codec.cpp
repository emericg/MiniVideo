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

#include "mkv_tracks.h"

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

    if (strncmp(codec_str, "A_", 2) == 0)
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
                    //*profile = PROF_AAC_HQ;
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
        else if (strcmp(codec_str, "A_AC3") == 0)
        {
            //A_AC3/BSID9
            //A_AC3/BSID10
            *codec = CODEC_AC3;
        }
        else if (strcmp(codec_str, "A_EAC3") == 0)
        {
            *codec = CODEC_EAC3;
        }
        else if (strcmp(codec_str, "A_AC4") == 0)
        {
            *codec = CODEC_AC4;
        }
        else if (strcmp(codec_str, "A_ALAC") == 0)
        {
            *codec = CODEC_ALAC;
        }
        else if (strcmp(codec_str, "A_DTS") == 0)
        {
            //A_DTS/EXPRESS
            //A_DTS/LOSSLESS
            *codec = CODEC_DTS;
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
                *codec = CODEC_RA_cook;
            }
            else if (strcmp(codec_str, "A_REAL/ATRC") == 0)
            {
                *codec = CODEC_ATRAC;
            }
        }
    }
    else if (strncmp(codec_str, "V_", 2) == 0)
    {
        // V_MS/VFW/FOURCC
        // V_UNCOMPRESSED
        // V_QUICKTIME

        if (strncmp(codec_str, "V_MPEG4/ISO", 11) == 0)
        {
            if (strcmp(codec_str, "V_MPEG4/ISO/AVC") == 0)
            {
                *codec = CODEC_H264;
            }
            else if (strcmp(codec_str, "V_MPEG4/ISO/SP") == 0)
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
        }
        else if (strcmp(codec_str, "V_MPEGH/ISO/HEVC") == 0)
        {
            *codec = CODEC_H265;
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
            *codec = CODEC_PRORES_422;
        }
    }
    else if (strncmp(codec_str, "S_", 2) == 0)
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
/*
        S_IMAGE/BMP
        S_KATE
*/
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief AVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.2 Syntax, 5.2.4.1.2 Semantics).
 */
int parse_h264_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_h264_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned configurationVersion = read_bits(bitstr, 8);
    unsigned AVCProfileIndication = read_bits(bitstr, 8);
    unsigned profile_compatibility = read_bits(bitstr, 8);
    unsigned AVCLevelIndication = read_bits(bitstr, 8);
    /*int reserved =*/ read_bits(bitstr, 6);
    unsigned lengthSizeMinusOne = read_bits(bitstr, 2);
    /*int reserved =*/ read_bits(bitstr, 3);

    // SPS
    track->sps_count = read_bits(bitstr, 5);
    track->sps_sample_offset = new int64_t [track->sps_count];
    track->sps_sample_size = new unsigned int[track->sps_count];

    for (unsigned i = 0; i < track->sps_count && i < MAX_SPS; i++) // MAX_SPS = 32
    {
        track->sps_sample_size[i] = read_bits(bitstr, 16);
        track->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

        track->sps_array[i] = (sps_t *)calloc(1, sizeof(sps_t));
        //track->sps_array[i] = new sps_t;

        skip_bits(bitstr, 8); // skip NAL header
        decodeSPS(bitstr, track->sps_array[i]);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->sps_sample_offset[i] + track->sps_sample_size[i]))
        {
            TRACE_WARNING(MKV, "SPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->sps_sample_offset[i] + track->sps_sample_size[i]));

            skip_bits(bitstr, ((track->sps_sample_offset[i] + track->sps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // PPS
    track->pps_count = read_bits(bitstr, 8);
    track->pps_sample_offset = new int64_t[track->pps_count];
    track->pps_sample_size = new unsigned int[track->pps_count];

    for (unsigned i = 0; i < track->pps_count && i < MAX_PPS; i++) // MAX_PPS = 256
    {
       track->pps_sample_size[i] = read_bits(bitstr, 16);
       track->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

       track->pps_array[i] = (pps_t *)calloc(1, sizeof(pps_t));
       //track->pps_array[i] = new pps_t;

       skip_bits(bitstr, 8); // skip NAL header
       decodePPS(bitstr, track->pps_array[i], track->sps_array);
       bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

       if (bitstream_get_absolute_byte_offset(bitstr) != (track->pps_sample_offset[i] + track->pps_sample_size[i]))
       {
            TRACE_WARNING(MKV, "PPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->pps_sample_offset[i] + track->pps_sample_size[i]));
            skip_bits(bitstr, ((track->pps_sample_offset[i] + track->pps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // Handle H.264 profile & level
    if (track->sps_count > 0 && track->sps_array[0])
    {
        track->codec_profile = getH264CodecProfile(track->sps_array[0]->profile_idc,
                                                   track->sps_array[0]->constraint_setX_flag[0],
                                                   track->sps_array[0]->constraint_setX_flag[1],
                                                   track->sps_array[0]->constraint_setX_flag[2],
                                                   track->sps_array[0]->constraint_setX_flag[3],
                                                   track->sps_array[0]->constraint_setX_flag[4],
                                                   track->sps_array[0]->constraint_setX_flag[5]);
        track->codec_level = static_cast<double>(AVCLevelIndication) / 10.0;

        track->max_ref_frames = track->sps_array[0]->max_num_ref_frames;
        track->color_depth = track->sps_array[0]->BitDepthY;

        if (track->sps_array[0]->vui)
        {
            track->color_range = track->sps_array[0]->vui->video_full_range_flag;
            track->color_primaries = track->sps_array[0]->vui->colour_primaries;
            track->color_transfer = track->sps_array[0]->vui->transfer_characteristics;
            track->color_matrix = track->sps_array[0]->vui->matrix_coefficients;
        }

        if (track->pps_count && track->pps_array[0])
        {
            track->use_cabac = track->pps_array[0]->entropy_coding_mode_flag;
            track->use_8x8_blocks = track->pps_array[0]->transform_8x8_mode_flag;
        }
    }
    else
    {
        track->codec_profile = getH264CodecProfile(AVCProfileIndication);
        track->codec_level = static_cast<double>(AVCLevelIndication) / 10.0;
    }

#if ENABLE_DEBUG
    TRACE_1(MKV, "> configurationVersion  : %u", configurationVersion);
    TRACE_1(MKV, "> AVCProfileIndication  : %u", AVCProfileIndication);
    TRACE_1(MKV, "> profile_compatibility : %u", profile_compatibility);
    TRACE_1(MKV, "> AVCLevelIndication    : %u", AVCLevelIndication);
    TRACE_1(MKV, "> lengthSizeMinusOne    : %u", lengthSizeMinusOne);

    TRACE_1(MKV, "> numOfSequenceParameterSets    = %u", track->sps_count);
    for (unsigned i = 0; i < track->sps_count; i++)
    {
        TRACE_1(MKV, "> sequenceParameterSetLength[%u] : %u", i, track->sps_sample_size[i]);
        TRACE_1(MKV, "> sequenceParameterSetOffset[%u] : %li", i, track->sps_sample_offset[i]);
    }

    TRACE_1(MKV, "> numOfPictureParameterSets     = %u", track->pps_count);
    for (unsigned i = 0; i < track->pps_count; i++)
    {
        TRACE_1(MKV, "> pictureParameterSetLength[%u]  : %u", i, track->pps_sample_size[i]);
        TRACE_1(MKV, "> pictureParameterSetOffset[%u]  : %li", i, track->pps_sample_offset[i]);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        xmlSpacer(mkv->xml, "AVC Configuration:", -1);
        fprintf(mkv->xml, "  <configurationVersion>%u</configurationVersion>\n", configurationVersion);
        fprintf(mkv->xml, "  <AVCProfileIndication>%u</AVCProfileIndication>\n", AVCProfileIndication);
        fprintf(mkv->xml, "  <profile_compatibility>%u</profile_compatibility>\n", profile_compatibility);
        fprintf(mkv->xml, "  <AVCLevelIndication>%u</AVCLevelIndication>\n", AVCLevelIndication);
        fprintf(mkv->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", lengthSizeMinusOne);

        fprintf(mkv->xml, "  <numOfSequenceParameterSets>%u</numOfSequenceParameterSets>\n", track->sps_count);
        for (unsigned i = 0; i < track->sps_count; i++)
        {
            xmlSpacer(mkv->xml, "SequenceParameterSet infos", i);
            fprintf(mkv->xml, "  <sequenceParameterSetLength index=\"%u\">%u</sequenceParameterSetLength>\n", i, track->sps_sample_size[i]);
            fprintf(mkv->xml, "  <sequenceParameterSetOffset index=\"%u\">%" PRId64 "</sequenceParameterSetOffset>\n", i, track->sps_sample_offset[i]);
        }

        fprintf(mkv->xml, "  <numOfPictureParameterSets>%u</numOfPictureParameterSets>\n", track->pps_count);
        for (unsigned i = 0; i < track->pps_count; i++)
        {
            xmlSpacer(mkv->xml, "PictureParameterSet", i);
            fprintf(mkv->xml, "  <pictureParameterSetLength index=\"%u\">%u</pictureParameterSetLength>\n", i, track->pps_sample_size[i]);
            fprintf(mkv->xml, "  <pictureParameterSetOffset index=\"%u\">%" PRId64 "</pictureParameterSetOffset>\n", i, track->pps_sample_offset[i]);
        }

        for (unsigned i = 0; i < track->sps_count; i++)
        {
            printPPS(track->pps_array[i], track->sps_array);

            mapSPS(track->sps_array[i],
                   track->sps_sample_offset[i],
                   track->sps_sample_size[i],
                   mkv->xml);
        }
        for (unsigned i = 0; i < track->pps_count; i++)
        {
            printPPS(track->pps_array[i], track->sps_array);

            mapPPS(track->pps_array[i],
                   track->sps_array,
                   track->pps_sample_offset[i],
                   track->pps_sample_size[i],
                   mkv->xml);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief HEVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 8.3.3 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 23008-2 video content.
 * Contain HEVCDecoderConfigurationRecord data structure (8.3.3.1.2 Syntax, 8.3.3.1.3 Semantics).
 */
int parse_h265_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "parse_h265_private()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned configurationVersion = read_bits(bitstr, 8);
    unsigned general_profile_space = read_bits(bitstr, 2);
    bool general_tier_flag = read_bits(bitstr, 1);
    unsigned general_profile_idc = read_bits(bitstr, 5);
    uint32_t general_profile_compatibility_flags = read_bits(bitstr, 32);
    uint64_t general_constraint_indicator_flags = read_bits_64(bitstr, 48);

    unsigned general_level_idc = read_bits(bitstr, 8);
    skip_bits(bitstr, 4); // reserved
    unsigned min_spatial_segmentation_idc = read_bits(bitstr, 12);
    skip_bits(bitstr, 6); // reserved
    unsigned parallelismType = read_bits(bitstr, 2);
    skip_bits(bitstr, 6); // reserved
    unsigned chromaFormat = read_bits(bitstr, 2);
    skip_bits(bitstr, 5); // reserved
    unsigned bitDepthLumaMinus8 = read_bits(bitstr, 3);
    skip_bits(bitstr, 5); // reserved
    unsigned bitDepthChromaMinus8 = read_bits(bitstr, 3);

    unsigned avgFrameRate = read_bits(bitstr, 16);
    unsigned constantFrameRate = read_bits(bitstr, 2);
    unsigned numTemporalLayers = read_bits(bitstr, 3);
    bool temporalIdNested = read_bits(bitstr, 1);
    unsigned lengthSizeMinusOne = read_bits(bitstr, 1);

    unsigned numOfArrays = read_bits(bitstr, 1);
    for (unsigned i = 0; i < numOfArrays; i++)
    {
        // TODO // NAL unit table
    }

    // Handle H.265 profile & level
    track->codec_profile = getH265CodecProfile(general_profile_idc);
    track->codec_level = static_cast<double>(general_level_idc) / 30.0;
    track->color_depth = bitDepthLumaMinus8 + 8;

#if ENABLE_DEBUG
    TRACE_1(MKV, "> configurationVersion    : %u", configurationVersion);
    TRACE_1(MKV, "> general_profile_space   : %u", general_profile_space);
    TRACE_1(MKV, "> general_tier_flag       : %u", general_tier_flag);
    TRACE_1(MKV, "> general_profile_idc     : %u", general_profile_idc);
    TRACE_1(MKV, "> general_profile_compatibility_flags : %u", general_profile_compatibility_flags);
    TRACE_1(MKV, "> general_constraint_indicator_flags  : %lu", general_constraint_indicator_flags);
    TRACE_1(MKV, "> general_level_idc       : %u", general_level_idc);

    TRACE_1(MKV, "> min_spatial_segmentation_idc: %u", min_spatial_segmentation_idc);
    TRACE_1(MKV, "> parallelismType         : %u", parallelismType);
    TRACE_1(MKV, "> chromaFormat            : %u", chromaFormat);
    TRACE_1(MKV, "> bitDepthLumaMinus8      : %u", bitDepthLumaMinus8);
    TRACE_1(MKV, "> bitDepthChromaMinus8    : %u", bitDepthChromaMinus8);

    TRACE_1(MKV, "> avgFrameRate            : %u", avgFrameRate);
    TRACE_1(MKV, "> constantFrameRate       : %u", constantFrameRate);
    TRACE_1(MKV, "> numTemporalLayers       : %u", numTemporalLayers);
    TRACE_1(MKV, "> temporalIdNested        : %u", temporalIdNested);
    TRACE_1(MKV, "> lengthSizeMinusOne      : %u", lengthSizeMinusOne);
    TRACE_1(MKV, "> numOfArrays             : %u", numOfArrays);

    for (unsigned j = 0; j < numOfArrays; j++)
    {
        // TODO // NAL unit table
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        xmlSpacer(mkv->xml, "HEVC Configuration:", -1);
        fprintf(mkv->xml, "  <configurationVersion>%u</configurationVersion>\n", configurationVersion);
        fprintf(mkv->xml, "  <general_profile_space>%u</general_profile_space>\n", general_profile_space);
        fprintf(mkv->xml, "  <general_tier_flag>%u</general_tier_flag>\n", general_tier_flag);
        fprintf(mkv->xml, "  <general_profile_idc>%u</general_profile_idc>\n", general_profile_idc);
        fprintf(mkv->xml, "  <general_profile_compatibility_flags>%u</general_profile_compatibility_flags>\n", general_profile_compatibility_flags);
        fprintf(mkv->xml, "  <general_constraint_indicator_flags>%" PRIu64 "</general_constraint_indicator_flags>\n", general_constraint_indicator_flags);
        fprintf(mkv->xml, "  <general_level_idc>%u</general_level_idc>\n", general_level_idc);

        fprintf(mkv->xml, "  <min_spatial_segmentation_idc>%u</min_spatial_segmentation_idc>\n", min_spatial_segmentation_idc);
        fprintf(mkv->xml, "  <parallelismType>%u</parallelismType>\n", parallelismType);
        fprintf(mkv->xml, "  <chromaFormat>%u</chromaFormat>\n", chromaFormat);
        fprintf(mkv->xml, "  <bitDepthLumaMinus8>%u</bitDepthLumaMinus8>\n", bitDepthLumaMinus8);
        fprintf(mkv->xml, "  <bitDepthChromaMinus8>%u</bitDepthChromaMinus8>\n", bitDepthChromaMinus8);

        fprintf(mkv->xml, "  <avgFrameRate>%u</avgFrameRate>\n", avgFrameRate);
        fprintf(mkv->xml, "  <constantFrameRate>%u</constantFrameRate>\n", constantFrameRate);
        fprintf(mkv->xml, "  <numTemporalLayers>%u</numTemporalLayers>\n", numTemporalLayers);
        fprintf(mkv->xml, "  <temporalIdNested>%u</temporalIdNested>\n", temporalIdNested);
        fprintf(mkv->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", lengthSizeMinusOne);

        fprintf(mkv->xml, "  <numOfArrays>%u</numOfArrays>\n", numOfArrays);
        for (unsigned j = 0; j < numOfArrays; j++)
        {
            // TODO // NAL unit table
        }
    }

    return retcode;
}

/* ************************************************************************** */
