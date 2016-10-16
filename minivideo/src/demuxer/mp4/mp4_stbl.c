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
 * \file      mp4_stbl.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "mp4_stbl.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../fourcc.h"
#include "../../typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

/*!
 * \brief Sample Table Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.1 Sample Table Box.
 *
 * Parse the sample table box, container for the time/space map.
 * This box does not contain informations, only other boxes.
 */
int parse_stbl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stbl()" CLR_RESET);
    int retcode = SUCCESS;

    // Print stbl box header
    print_box_header(box_header);
    write_box_header(box_header, mp4->xml);
    if (mp4->xml) fprintf(mp4->xml, "  <title>Sample Table</title>\n");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_STSD:
                    retcode = parse_stsd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STTS:
                    retcode = parse_stts(bitstr, &box_subheader, track);
                    break;
                case BOX_CTTS:
                    retcode = parse_ctts(bitstr, &box_subheader, track);
                    break;
                case BOX_STSS:
                    retcode = parse_stss(bitstr, &box_subheader, track);
                    break;
                case BOX_STSC:
                    retcode = parse_stsc(bitstr, &box_subheader, track);
                    break;
                case BOX_STSZ:
                    retcode = parse_stsz(bitstr, &box_subheader, track);
                    break;
                case BOX_STZ2:
                    retcode = parse_stsz(bitstr, &box_subheader, track);
                    break;
                case BOX_STCO:
                    retcode = parse_stco(bitstr, &box_subheader, track);
                    break;
                case BOX_CO64:
                    retcode = parse_stco(bitstr, &box_subheader, track);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                    break;
            }

            jumpy_mp4(bitstr, box_header, &box_subheader);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </atom>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Description Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box.
 *
 * The SampleDescriptionBox contains information about codec types and some
 * initialization parameters needed to start decoding.
 * If an AVC box (AVCDecoderConfigurationRecord) is present, it also contains the
 * diferents SPS and PPS of the video.
 */
int parse_stsd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int reserved[6] = {0};
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        reserved[i] = read_bits(bitstr, 8);
    }
    /*unsigned int data_reference_index =*/ read_bits(bitstr, 16);

    // Parse subbox header
    Mp4Box_t box_subheader;
    retcode = parse_box_header(bitstr, &box_subheader);

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    /*unsigned int entry_count =*/ read_bits(bitstr, 32);

    char fcc[5];
    track->fcc = box_subheader.boxtype; // save fourcc as backup

    // Then parse subbox content
    switch (track->handlerType)
    {
        case HANDLER_AUDIO:
        {
            // AudioSampleEntry
            // Box Types: ‘mp4a’

            if (box_subheader.boxtype == fcc_mp4a)
            {
                track->codec = CODEC_AAC;
                TRACE_1(MP4, "> Audio track is using AAC codec");
            }
            else if (box_subheader.boxtype == fcc_AC3 || box_subheader.boxtype == fcc_ac3)
            {
                track->codec = CODEC_AC3;
                TRACE_1(MP4, "> Audio track is using AC3 codec");
            }
            else if (box_subheader.boxtype == fcc_AC4 || box_subheader.boxtype == fcc_ac4)
            {
                track->codec = CODEC_AC4;
                TRACE_1(MP4, "> Audio track is using AC4 codec");
            }
            else if (box_subheader.boxtype == fcc_sowt)
            {
                track->codec = CODEC_LPCM;
                TRACE_1(MP4, "> Audio track is using PCM audio");
            }
            else
            {
                track->codec = CODEC_UNKNOWN;
                TRACE_WARNING(MP4, "> Unknown codec in audio track (%s)",
                              getFccString_le(box_subheader.boxtype, fcc));
            }

            /*const unsigned int reserved[0] =*/ read_bits(bitstr, 32);
            /*const unsigned int reserved[1] =*/ read_bits(bitstr, 32);

            track->channel_count = read_bits(bitstr, 16);
            track->sample_size_bits = read_bits(bitstr, 16);

            /*unsigned int pre_defined =*/ read_bits(bitstr, 16);
            /*const unsigned int(16) reserved =*/ read_bits(bitstr, 16);

            track->sample_rate_hz = read_bits(bitstr, 16);
        } break;

        case HANDLER_VIDEO:
        {
            // VisualSampleEntry
            // Box Types: 'avc1', 'm4ds', 'hev1', 'CFHD'

            if (box_subheader.boxtype == fcc_avc1)
            {
                track->codec = CODEC_H264;
                TRACE_1(MP4, "> Video track is using H.264 codec");
            }
            else if (box_subheader.boxtype == fcc_hvc1)
            {
                track->codec = CODEC_H265;
                TRACE_1(MP4, "> Video track is using H.265 codec");
            }
            else if (box_subheader.boxtype == fcc_mp4v)
            {
                track->codec = CODEC_MPEG4_ASP;
                TRACE_1(MP4, "> Video track is using XVID codec");
            }
            else if (box_subheader.boxtype == fcc_CFHD)
            {
                track->codec = CODEC_VC5;
                TRACE_1(MP4, "> Video track is using CineForm codec");
            }
            else
            {
                track->codec = CODEC_UNKNOWN;
                TRACE_WARNING(MP4, "> Unknown codec in video track (%s)",
                              getFccString_le(box_subheader.boxtype, fcc));
            }

            /*unsigned int pre_defined =*/ read_bits(bitstr, 16);
            /*const unsigned int reserved =*/ read_bits(bitstr, 16);

            unsigned int pre_defined[3] = {0};
            pre_defined[0] = read_bits(bitstr, 32);
            pre_defined[1] = read_bits(bitstr, 32);
            pre_defined[2] = read_bits(bitstr, 32);

            track->width = read_bits(bitstr, 16);
            track->height = read_bits(bitstr, 16);

            // 0x00480000; // 72 dpi
            unsigned int horizresolution = read_bits(bitstr, 32);
            unsigned int vertresolution = read_bits(bitstr, 32);

            /*const unsigned int reserved =*/ read_bits(bitstr, 32);

            unsigned int frame_count = read_bits(bitstr, 16);

            uint8_t compressorsize = (uint8_t)read_bits(bitstr, 8);
            for (i = 0; i < 31; i++)
            {
                track->compressorname[i] = (char)read_bits(bitstr, 8);
            }
            track->compressorname[compressorsize] = '\0';

            track->color_depth = read_bits(bitstr, 16);
            /*int pre_defined = */ read_bits(bitstr, 16);

#if ENABLE_DEBUG
            {
                // Print box header
                print_box_header(box_header);

                // Print VisualSampleEntry box header
                print_box_header(&box_subheader);

                // Print VisualSampleEntry box content
                TRACE_1(MP4, "> width  : %u", track->width);
                TRACE_1(MP4, "> height : %u", track->height);
                TRACE_1(MP4, "> horizresolution : 0x%X", horizresolution);
                TRACE_1(MP4, "> vertresolution  : 0x%X", vertresolution);
                TRACE_1(MP4, "> frame_count     : %u", frame_count);
                TRACE_1(MP4, "> compressor      : '%s'", track->compressorname);
                TRACE_1(MP4, "> color depth     : %u", track->color_depth);
            }
#endif // ENABLE_DEBUG

            while (retcode == SUCCESS &&
                   bitstream_get_absolute_byte_offset(bitstr) < box_subheader.offset_end)
            {
                // Parse subbox header
                Mp4Box_t box_subsubheader;
                retcode = parse_box_header(bitstr, &box_subsubheader);

                // Then parse subbox content
                ////////////////////////////////////////////////////////////////
                if (retcode == SUCCESS)
                {
                    switch (box_subsubheader.boxtype)
                    {
                        case BOX_AVCC:
                            retcode = parse_avcC(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_BTRT:
                            retcode = parse_btrt(bitstr, &box_subsubheader, track, mp4);
                            break;
                        case BOX_CLAP:
                            retcode = parse_clap(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_COLR:
                            retcode = parse_colr(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_FIEL:
                            retcode = parse_fiel(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_GAMA:
                            retcode = parse_gama(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_PASP:
                            retcode = parse_pasp(bitstr, &box_subsubheader, track);
                            break;
                        default:
                            retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                            break;
                    }

                    jumpy_mp4(bitstr, &box_subheader, &box_subsubheader);
                }
            }
        } break;

        case HANDLER_TEXT:
        break;

        case HANDLER_META:
        break;

        case HANDLER_TMCD:
        break;

        case HANDLER_HINT:
        break;

        default:
            TRACE_1(MP4, "Unknown track type, skipped...");
        break;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AVCConfigurationBox.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.1 Syntax, 5.2.4.1.2 Semantics).
 */
int parse_avcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_avcC()" CLR_RESET);
    int retcode = SUCCESS;

    // avcC box means H.264 codec
    track->codec = CODEC_H264;

    // Parse box content
    unsigned int i = 0;

    unsigned int configurationVersion = read_bits(bitstr, 8);
    unsigned int AVCProfileIndication = read_bits(bitstr, 8);
    unsigned int profile_compatibility = read_bits(bitstr, 8);
    unsigned int AVCLevelIndication = read_bits(bitstr, 8);
    /*int reserved =*/ read_bits(bitstr, 6);
    unsigned int lengthSizeMinusOne = read_bits(bitstr, 2);
    /*int reserved =*/ read_bits(bitstr, 3);

    // SPS
    track->sps_count = read_bits(bitstr, 5); // MAX_SPS = 32
    track->sps_sample_offset = (int64_t*)calloc(track->sps_count, sizeof(int64_t));
    track->sps_sample_size = (unsigned int*)calloc(track->sps_count, sizeof(unsigned int));
    for (i = 0; i < track->sps_count; i++)
    {
        track->sps_sample_size[i] = read_bits(bitstr, 16);
        track->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

        skip_bits(bitstr, track->sps_sample_size[i] * 8); // sequenceParameterSetNALUnit
    }

    // PPS
    track->pps_count = read_bits(bitstr, 8); // MAX_PPS = 256
    track->pps_sample_offset = (int64_t*)calloc(track->pps_count, sizeof(int64_t));
    track->pps_sample_size = (unsigned int*)calloc(track->pps_count, sizeof(unsigned int));
    for (i = 0; i < track->pps_count; i++)
    {
       track->pps_sample_size[i] = read_bits(bitstr, 16);
       track->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

       skip_bits(bitstr, track->pps_sample_size[i] * 8); // pictureParameterSetNALUnit
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> configurationVersion  : %u", configurationVersion);
    TRACE_1(MP4, "> AVCProfileIndication  : %u", AVCProfileIndication);
    TRACE_1(MP4, "> profile_compatibility : %u", profile_compatibility);
    TRACE_1(MP4, "> AVCLevelIndication    : %u", AVCLevelIndication);
    TRACE_1(MP4, "> lengthSizeMinusOne    : %u", lengthSizeMinusOne);

    TRACE_1(MP4, "> numOfSequenceParameterSets    = %u", track->sps_count);
    for (i = 0; i < track->sps_count; i++)
    {
        TRACE_1(MP4, "> sequenceParameterSetLength[%u] : %u", i, track->sps_sample_size[i]);
        TRACE_1(MP4, "> sequenceParameterSetOffset[%u] : %u", i, track->sps_sample_offset[i]);
    }

    TRACE_1(MP4, "> numOfPictureParameterSets     = %u", track->pps_count);
    for (i = 0; i < track->pps_count; i++)
    {
        TRACE_1(MP4, "> pictureParameterSetLength[%u]  : %u", i, track->pps_sample_size[i]);
        TRACE_1(MP4, "> pictureParameterSetOffset[%u]  : %u", i, track->pps_sample_offset[i]);
    }
#endif // ENABLE_DEBUG
    /*
    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Movie Header</title>\n");
        fprintf(mp4->xml, "  <creation_time>%lu</creation_time>\n", mp4->creation_time);
        fprintf(mp4->xml, "  <modification_time>%lu</modification_time>\n", mp4->modification_time);
        fprintf(mp4->xml, "  <timescale>%u</timescale>\n", mp4->timescale);
        fprintf(mp4->xml, "  <duration>%lu</duration>\n", mp4->duration);
        fprintf(mp4->xml, "  <rate>%u</rate>\n", rate);
        fprintf(mp4->xml, "  <volume>%u</volume>\n", volume);
        fprintf(mp4->xml, "  <matrix>[%i, %i, %i, %i, %i, %i, %i, %i, %i]</matrix>\n",
                matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
                matrix[5], matrix[6], matrix[7], matrix[8]);
        //for (i = 0; i < 9; i++)
        //    fprintf(mp4->xml, "  <matrix index=\"%i\">%i</matrix>\n", i, matrix[i]);
        fprintf(mp4->xml, "  </atom>\n");
    }
*/
    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief BitrateBox.
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
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Bitrate</title>\n");
        fprintf(mp4->xml, "  <bufferSizeDB>%lu</bufferSizeDB>\n", bufferSizeDB);
        fprintf(mp4->xml, "  <bitrate_max>%lu</bitrate_max>\n", track->bitrate_max);
        fprintf(mp4->xml, "  <bitrate_avg>%u</bitrate_avg>\n", track->bitrate_avg);
        fprintf(mp4->xml, "  </atom>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief CleanApertureBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_clap(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
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
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
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
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief ColourInformationBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_colr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_colr()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int colour_type = read_bits(bitstr, 32);
    unsigned int colour_primaries;
    unsigned int transfer_characteristics;
    unsigned int matrix_coefficients;

    if (colour_type == 'nclc' ||
        colour_type == 'nclx') // "on-screen colours"
    {
        // https://developer.apple.com/library/mac/technotes/tn2227/_index.html

        colour_primaries = read_bits(bitstr, 16);
        transfer_characteristics = read_bits(bitstr, 16);
        matrix_coefficients = read_bits(bitstr, 16);
        /*unsigned int reserved =*/ //read_bits(bitstr, 7);
        track->color_range = read_bits(bitstr, 16);

        if (matrix_coefficients == 1)
        {
            track->color_matrix = CM_bt709;
        }
        else if (matrix_coefficients == 6)
        {
            track->color_matrix = CM_bt601;
        }
        else if (matrix_coefficients == 7)
        {
            track->color_matrix = CM_SMPTE240M;
        }
    }
    else if (colour_type == 'rICC')
    {
        // ICC_profile; // restricted ICC profile
    }
    else if (colour_type == 'prof')
    {
        // ICC_profile; // unrestricted ICC profile
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        char fcc[5];
        TRACE_1(MP4, "> colour_type             : %u", getFccString_le(colour_type, fcc));
        if (colour_type == 'nclc' || colour_type == 'nclx')
        {
            TRACE_1(MP4, "> colour_primaries        : %u", colour_primaries);
            TRACE_1(MP4, "> transfer_characteristics: %u", transfer_characteristics);
            TRACE_1(MP4, "> matrix_coefficients     : %u", matrix_coefficients);
            TRACE_1(MP4, "> full_range_flag         : %u", track->color_range);
        }
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief FIEL box.
 */
int parse_fiel(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_fiel()" CLR_RESET);
    int retcode = SUCCESS;

    // TODO

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief GAMA box.
 */
int parse_gama(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gama()" CLR_RESET);
    int retcode = SUCCESS;

    // TODO

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief PixelAspectRatioBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_pasp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pasp()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    //unsigned int pasp_size = read_bits(bitstr, 32);
    //unsigned int pasp_type = read_bits(bitstr, 32);

    track->par_h = read_bits(bitstr, 32);
    track->par_v = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        //TRACE_1(MP4, "> pasp_size : %u", pasp_size);
        //TRACE_1(MP4, "> pasp_type : %u", pasp_type);
        TRACE_1(MP4, "> hSpacing  : %u", track->par_h);
        TRACE_1(MP4, "> vSpacing  : %u", track->par_v);
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Decoding Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.1.2 Decoding Time to Sample Box.
 *
 * This box contains a compact version of a table that allows indexing from
 * decoding time to sample number.
 */
int parse_stts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stts()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->stts_entry_count = read_bits(bitstr, 32);
    track->stts_sample_count = (unsigned int*)calloc(track->stts_entry_count, sizeof(unsigned int));
    track->stts_sample_delta = (unsigned int*)calloc(track->stts_entry_count, sizeof(unsigned int));

    uint32_t i = 0;

    if (track->stts_sample_count == NULL || track->stts_sample_delta == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->stts_entry_count; i++)
        {
            track->stts_sample_count[i] = read_bits(bitstr, 32);
            track->stts_sample_delta[i] = read_bits(bitstr, 32);
        }
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> entry_count   : %u", track->stts_entry_count);
#if TRACE_1
        TRACE_1(MP4, "> sample_number : [");
        for (i = 0; i < track->stts_entry_count; i++)
        {
            printf("(%u / %u),", track->stts_sample_count[i], track->stts_sample_delta[i]);
        }
        printf("]\n");
#endif // TRACE_1
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Composition Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.1.3 Composition Time to Sample Box.
 *
 * This box provides the offset between decoding time and composition time.
 */
int parse_ctts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ctts()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->ctts_entry_count = read_bits(bitstr, 32);
    track->ctts_sample_count = (uint32_t*)calloc(track->ctts_entry_count, sizeof(uint32_t));
    track->ctts_sample_offset = (int64_t*)calloc(track->ctts_entry_count, sizeof(int64_t));

    uint32_t i = 0;

    if (track->ctts_sample_count == NULL || track->ctts_sample_offset == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->ctts_entry_count; i++)
        {
            track->ctts_sample_count[i] = read_bits(bitstr, 32);

            if (box_header->version == 0)
                track->ctts_sample_offset[i] = (int64_t)read_bits(bitstr, 32); // read uint
            else if (box_header->version == 1)
                track->ctts_sample_offset[i] = (int64_t)read_bits(bitstr, 32); // read int
        }
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> entry_count   : %u", track->ctts_entry_count);
#if TRACE_1
        TRACE_1(MP4, "> sample_number : [");
        for (i = 0; i < track->ctts_entry_count; i++)
        {
            if (box_header->version == 0)
                printf("(%u / %u),", track->ctts_sample_count[i], track->ctts_sample_offset_u[i]);
            else
                printf("(%u / %i),", track->ctts_sample_count[i], track->ctts_sample_offset_i[i]);
        }
        printf("]\n");
#endif // TRACE_1
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sync Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.2 Sync Sample Box.
 *
 * This box provides a compact marking of the random access points within the stream.
 */
int parse_stss(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stss()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stss_entry_count = read_bits(bitstr, 32);
    track->stss_sample_number = (unsigned int*)calloc(track->stss_entry_count, sizeof(unsigned int));

    if (track->stss_sample_number == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->stss_entry_count; i++)
        {
            track->stss_sample_number[i] = read_bits(bitstr, 32);
        }

#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);

            // Print box content
            TRACE_1(MP4, "> entry_count   : %u", track->stss_entry_count);
#if TRACE_1
            TRACE_1(MP4, "> sample_number : [");
            for (i = 0; i < track->stss_entry_count; i++)
            {
                printf("%u, ", track->stss_sample_number[i]);
            }
            printf("]\n");
#endif // TRACE_1
        }
#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample To Chunk Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.4 Sample To Chunk Box.
 *
 * Samples within the media data are grouped into chunks. Chunks can be of different
 * sizes, and the samples within a chunk can have different sizes. This table can
 * be used to find the chunk that contains a sample, its position, and the associated
 * sample description.
 */
int parse_stsc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsc()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stsc_entry_count = read_bits(bitstr, 32);
    track->stsc_first_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
    track->stsc_samples_per_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
    track->stsc_sample_description_index = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));

    if (track->stsc_first_chunk == NULL ||
        track->stsc_samples_per_chunk == NULL ||
        track->stsc_sample_description_index == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc first_chunk, samples_per_chunk or sample_description_index tables!");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->stsc_entry_count; i++)
        {
            track->stsc_first_chunk[i] = read_bits(bitstr, 32);
            track->stsc_samples_per_chunk[i] = read_bits(bitstr, 32);
            track->stsc_sample_description_index[i] = read_bits(bitstr, 32);
        }

#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);
#if TRACE_1
            // Print box content
            TRACE_1(MP4, "> entry_count : %u", track->stsc_entry_count);

            TRACE_1(MP4, "> first_chunk : [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_first_chunk[i]);
            }
            printf("]\n");

            TRACE_1(MP4, "> samples_per_chunk : [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_samples_per_chunk[i]);
            }
            printf("]\n");

            TRACE_1(MP4, "> sample_description_index : [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_sample_description_index[i]);
            }
            printf("]\n");
#endif // TRACE_1
        }
#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Size Boxes - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.3 Sample Size Box
 *
 * This box contains the sample count and a table giving the size in bytes of each
 * sample. This allows the media data itself to be unframed. The total number of
 * samples in the media is always indicated in the sample count.
 *
 * This box has two variants: STSZ and STZ2.
 * - This variant has a fixed size 32-bit field for representing the sample
 *   sizes; it permits defining a constant size for all samples in a track.
 * - The STZ2 variant permits smaller size fields, to save space when the sizes
 *   are varying but small.
 */
int parse_stsz(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsz()" CLR_RESET);
    int retcode = SUCCESS;
    unsigned int i = 0;
    int field_size = 32;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    if (box_header->boxtype == BOX_STSZ)
    {
        track->stsz_sample_size = read_bits(bitstr, 32);
        track->stsz_sample_count = read_bits(bitstr, 32);
    }
    else //if (box_header->type == BOX_STZ2)
    {
        /*unsigned int reserved =*/ read_bits(bitstr, 24);
        field_size = read_bits(bitstr, 8);
        track->stsz_sample_count = read_bits(bitstr, 32);
    }

    if (track->stsz_sample_size == 0)
    {
        track->stsz_entry_size = (unsigned int*)calloc(track->stsz_sample_count, sizeof(unsigned int));

        if (track->stsz_entry_size == NULL)
        {
             TRACE_ERROR(MP4, "Unable to alloc entry_size table!");
             retcode = FAILURE;
        }
        else
        {
            for (i = 0; i < track->stsz_sample_count; i++)
            {
                track->stsz_entry_size[i] = read_bits(bitstr, field_size);
            }
        }
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> sample_count : %u", track->stsz_sample_count);
        TRACE_1(MP4, "> sample_size  : %u", track->stsz_sample_size);
/*
        if (track->stsz_sample_size == 0)
        {
            TRACE_1(MP4, "> entry_size : [");
            for (i = 0; i < track->stsz_sample_count; i++)
            {
                printf("%u, ", track->stsz_entry_size[i]);
            }
            printf("]\n");
        }
*/
    }
#endif // ENABLE_DEBUG

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Chunk Offset Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.5 Chunk Offset Box.
 *
 * The chunk offset table gives the index of each chunk into the containing file.
 * There are two variants, permitting the use of 32-bit (STCO variant) or 64-bit
 * offsets (CO64 variant).
 */
int parse_stco(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stco()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stco_entry_count = read_bits(bitstr, 32);
    track->stco_chunk_offset = (int64_t*)calloc(track->stco_entry_count, sizeof(int64_t));

    if (track->stco_chunk_offset == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc chunk_offset table!");
        retcode = FAILURE;
    }
    else
    {
        if (box_header->boxtype == BOX_CO64)
        {
            for (i = 0; i < track->stco_entry_count; i++)
            {
                track->stco_chunk_offset[i] = (int64_t)read_bits_64(bitstr, 64);
            }
        }
        else //if (box_header->type == BOX_STCO)
        {
            for (i = 0; i < track->stco_entry_count; i++)
            {
                track->stco_chunk_offset[i] = (int64_t)read_bits(bitstr, 32);
            }
        }
#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);

            // Print box content
            TRACE_1(MP4, "> entry_count  : %u", track->stco_entry_count);
/*
            TRACE_1(MP4, "> chunk_offset : [");
            for (i = 0; i < track->stco_entry_count; i++)
            {
                printf("%lli, ", track->stco_chunk_offset[i]);
            }
            printf("]\n");
*/
        }
#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */
