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
 * \file      mkv_cluster.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_tracks.h"
#include "mkv_struct.h"
#include "ebml.h"

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

int mkv_parse_block(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                    MkvSampleType_e type, int64_t cluster_timecode)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_block()" CLR_RESET);
    int retcode = SUCCESS;

    uint32_t stn = static_cast<uint32_t>(read_ebml_size(bitstr) - 1);
    uint32_t stc = read_bits(bitstr, 16);

    if (mkv->tracks_count < stn && !mkv->tracks[stn])
    {
        TRACE_WARNING(MKV, "simpleblock with no associated track: %i ???", stn);
        return FAILURE;
    }

    //TRACE_1(MKV, "block track number: %u", stn);
    //TRACE_1(MKV, "block timecode: %u", stc + cluster.Timecode);

    bool idr = false;
    bool visible;
    uint8_t lacing;
    bool discardable = false;

    if (type == sampletype_SimpleBlock)
    {
        idr = read_bit(bitstr);
        skip_bits(bitstr, 3);
        visible = read_bit(bitstr);
        lacing = read_bits(bitstr, 2);
        discardable = read_bit(bitstr);
    }
    else // if (type == sampletype_Block)
    {
        skip_bits(bitstr, 4);
        visible = read_bit(bitstr);
        lacing = read_bits(bitstr, 2);
        skip_bits(bitstr, 1);
    }

    // Map block
    if (mkv->xml)
    {
        if (type == sampletype_SimpleBlock)
        {
            write_ebml_element(element, mkv->xml, "SimpleBlock");
            fprintf(mkv->xml, "  <track>%u</track>\n", stn);

            fprintf(mkv->xml, "  <idr>%u</idr>\n", idr);
            fprintf(mkv->xml, "  <visible>%u</visible>\n", visible);
            fprintf(mkv->xml, "  <lacing>%u</lacing>\n", lacing);
            fprintf(mkv->xml, "  <discardable>%u</discardable>\n", discardable);
        }
        else
        {
            write_ebml_element(element, mkv->xml, "Block");
            fprintf(mkv->xml, "  <track>%u</track>\n", stn);

            fprintf(mkv->xml, "  <visible>%u</visible>\n", visible);
            fprintf(mkv->xml, "  <lacing>%u</lacing>\n", lacing);
        }
    }

    if (!lacing)
    {
        mkv_sample_t *s = new mkv_sample_t;
        if (s)
        {
            s->idr = idr;
            s->visible = visible;
            s->discardable = discardable;
        }

        // Parse sample
        s->offset = bitstream_get_absolute_byte_offset(bitstr);
            int block_size_overhead = s->offset - element->offset_start - 2;
            TRACE_2(MKV, "> block size overhead > %i", block_size_overhead);
        s->size = element->size - block_size_overhead;
        s->timecode = stc + cluster_timecode;

        // Index sample
        mkv->tracks[stn]->sample_vector.push_back(s);

        // Map sample
        if (mkv->xml)
        {
            xmlSpacer(mkv->xml, "Block", 0);
            fprintf(mkv->xml, "  <offset>%" PRId64 "</offset>\n", s->offset);
            fprintf(mkv->xml, "  <size>%" PRId64 "</size>\n", s->size);
            fprintf(mkv->xml, "  <timecode>%" PRIu64 "</timecode>\n", s->timecode);
        }
    }
    else
    {
        // Parse sample(s)
        uint8_t frame_count_minus1 = read_bits(bitstr, 8);
        uint8_t frame_count = frame_count_minus1 + 1;

        uint32_t *frame_x_size = new uint32_t[frame_count];
        uint64_t *frame_x_offset = new uint64_t[frame_count];

        uint64_t frame_0_offset = bitstream_get_absolute_byte_offset(bitstr);

        int lacing_cumulated = 0;

        // Lace-coded size of each frame of the lace, except for the last one (multiple uint8).
        // This is not used with Fixed-size lacing as it is calculated automatically from (total size of lace) / (number of frames in lace).

        TRACE_1(MKV, "> LACING (%u) > frame_count: %i > block size: %i", lacing, frame_count, element->size);

        for (int i = 0; i < frame_count; i++)
        {
            if (i == 0)
                frame_x_offset[0] = frame_0_offset;
            else
                frame_x_offset[i] = frame_x_offset[i-1] + frame_x_size[i-1];

            if (lacing == MKV_LACING_XIPH)
            {
                if (i < frame_count_minus1)
                {
                    frame_x_size[i] = 0;

                    uint8_t next = 255;
                    while (next == 255 && lacing_cumulated < element->size)
                    {
                        next = read_bits(bitstr, 8);
                        frame_x_size[i] += next;
                        lacing_cumulated += next;
                    }
                }
                else // last frame
                {
                    frame_x_size[i] = element->size - lacing_cumulated;
                }

                TRACE_2(MKV, "frame xiph [%u] : %i", i, frame_x_size[i]);
            }
            else if (lacing == MKV_LACING_EBML)
            {
                if (i == 0)
                {
                    frame_x_size[i] = read_ebml_size(bitstr);
                    lacing_cumulated += frame_x_size[i];
                }
                else if (i < frame_count_minus1)
                {
                    frame_x_size[i] = frame_x_size[i-1] + read_ebml_lacing_size(bitstr);
                    lacing_cumulated += frame_x_size[i];
                }
                else // last frame
                {
                    frame_x_size[i] = element->size - lacing_cumulated;
                }

                TRACE_2(MKV, "frame ebml [%u] : %i", i, frame_x_size[i]);
            }
            else if (lacing == MKV_LACING_FIXED)
            {
                int block_size_overhead = frame_0_offset - element->offset_start - 2;
                frame_x_size[i] = (element->size - block_size_overhead) / (double)(frame_count);

                TRACE_2(MKV, "frame fixed [%u] : %i", i, frame_x_size[i]);
            }

            mkv_sample_t *s = new mkv_sample_t;
            if (s)
            {
                s->idr = idr;
                s->visible = visible;
                s->discardable = discardable;

                // Parse sample
                s->offset = frame_x_offset[i];
                s->size = frame_x_size[i];
                s->timecode = stc + cluster_timecode;
            }

            // Index sample
            mkv->tracks[stn]->sample_vector.push_back(s);

            // Map sample
            if (mkv->xml)
            {
                xmlSpacer(mkv->xml, "Block", i);
                fprintf(mkv->xml, "  <offset>%" PRId64 "</offset>\n", s->offset);
                fprintf(mkv->xml, "  <size>%" PRId64 "</size>\n", s->size);
                fprintf(mkv->xml, "  <timecode>%" PRIu64 "</timecode>\n", s->timecode);
            }
        }

        delete [] frame_x_size;
        delete [] frame_x_offset;
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int mkv_parse_blockgroup(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, int64_t cluster_timecode)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_blockgroup()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Block Group");

    while (mkv->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < element->offset_end)
    {
        // Parse sub element
        EbmlElement_t element_sub;
        retcode = parse_ebml_element(bitstr, &element_sub);

        // Then parse subbox content
        if (mkv->run == true && retcode == SUCCESS)
        {
            switch (element_sub.eid)
            {
            case eid_Block:
                mkv_parse_block(bitstr, &element_sub, mkv, sampletype_Block, cluster_timecode);
                break;

            case eid_BlockDuration:
                /*BlockDuration =*/ read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "BlockDuration");
                break;
            case eid_ReferencePriority:
                /*ReferencePriority =*/ read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ReferencePriority");
                break;
            case eid_ReferenceBlock:
                /*ReferenceBlock =*/ read_ebml_data_int(bitstr, &element_sub, mkv->xml, "ReferenceBlock");
                break;
            case eid_CodecState:
                /*CodecState =*/ read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "CodecState");
                break;
            case eid_DiscardPadding:
                /*DiscardPadding =*/ read_ebml_data_int(bitstr, &element_sub, mkv->xml, "DiscardPadding");
                break;

            // TODO BlockAdditions
            // TODO slices

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            retcode = jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int mkv_parse_cluster(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cluster()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cluster");

    mkv_cluster_t cluster;
    cluster.Timecode = 0;

    while (mkv->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < element->offset_end)
    {
        // Parse sub element
        EbmlElement_t element_sub;
        retcode = parse_ebml_element(bitstr, &element_sub);

        // Then parse subbox content
        if (mkv->run == true && retcode == SUCCESS)
        {
            switch (element_sub.eid)
            {
            case eid_TimeCode:
                cluster.Timecode = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "Timecode");
                break;
            case eid_Position:
                cluster.Position = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "Position");
                break;
            case eid_PrevSize:
                cluster.PrevSize = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PrevSize");
                break;
            case eid_SilentTracks:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;

            case eid_BlockGroup:
                retcode = mkv_parse_blockgroup(bitstr, &element_sub, mkv, cluster.Timecode);
                break;

            case eid_SimpleBlock:
                mkv_parse_block(bitstr, &element_sub, mkv, sampletype_SimpleBlock, cluster.Timecode);
                break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            retcode = jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
