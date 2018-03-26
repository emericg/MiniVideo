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

uint32_t read_ebmllike_value(Bitstream_t *bitstr)
{
    uint32_t leadingZeroBits = 0;
    uint32_t elementSize = 0;
    uint32_t elementValue = 0;

    while (read_bit(bitstr) == 0 && leadingZeroBits < 4)
        leadingZeroBits++;

    elementSize = (leadingZeroBits + 1) * 7;
    elementValue = read_bits(bitstr, elementSize);
/*
    TRACE_WARNING(MKV, "read_ebml()");
    TRACE_WARNING(MKV, "- leadingZeroBits = %u", leadingZeroBits);
    TRACE_WARNING(MKV, "- elementSize     = %u", elementSize);
    TRACE_WARNING(MKV, "- elementValue    = 0x%0X", elementValue);
*/
    return elementValue;
}

/* ************************************************************************** */

mkv_sample_t *mkv_parse_block(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                              MkvSampleType_e type, int64_t cluster_timecode)
{
    uint32_t stn = read_ebmllike_value(bitstr) - 1;
    int16_t stc = static_cast<int16_t>(read_bits(bitstr, 16));
    uint8_t lacing = 0;

    //TRACE_1(MKV, "block track number: %u", stn);
    //TRACE_1(MKV, "block timecode: %u", stc + cluster.Timecode);

    mkv_sample_t *s = new mkv_sample_t;
    if (s)
    {
        if (type == sampletype_SimpleBlock)
        {
            s->idr = read_bit(bitstr);
            skip_bits(bitstr, 3);
            s->visible = read_bit(bitstr);
            lacing = read_bits(bitstr, 2);
            s->discardable = read_bit(bitstr);
        }
        else // if (type == sampletype_Block)
        {
            skip_bits(bitstr, 4);
            s->visible = read_bit(bitstr);
            lacing = read_bits(bitstr, 2);
            skip_bits(bitstr, 1);
        }

        if (lacing)
        {
            TRACE_WARNING(MKV, "Lacing used (track: %i, mode: %i), but not really supported...", stn, lacing);

            uint8_t frame_count_minus1 = read_bits(bitstr, 8);

            // Lace-coded size of each frame of the lace, except for the last one (multiple uint8).
            // This is not used with Fixed-size lacing as it is calculated automatically from (total size of lace) / (number of frames in lace).
            if (lacing == MKV_LACING_XIPH || lacing == MKV_LACING_EBML)
            {
                for (int i = 0; i < frame_count_minus1; i++)
                {
                    uint8_t frame_x_size = read_bits(bitstr, 8);
                }
            }
        }

        s->offset = bitstream_get_absolute_byte_offset(bitstr);
        s->size = element->size;
        s->timecode = stc + cluster_timecode;

        // Map sample
        if (mkv->xml)
        {
            if (type == sampletype_SimpleBlock)
            {
                write_ebml_element(element, mkv->xml, "SimpleBlock");
                fprintf(mkv->xml, "  <track>%u</track>\n", stn);

                fprintf(mkv->xml, "  <idr>%u</idr>\n", s->idr);
                fprintf(mkv->xml, "  <visible>%u</visible>\n", s->visible);
                fprintf(mkv->xml, "  <lacing>%u</lacing>\n", lacing);
                fprintf(mkv->xml, "  <discardable>%u</discardable>\n", s->discardable);
            }
            else
            {
                write_ebml_element(element, mkv->xml, "Block");
                fprintf(mkv->xml, "  <track>%u</track>\n", stn);

                fprintf(mkv->xml, "  <visible>%u</visible>\n", s->visible);
                fprintf(mkv->xml, "  <lacing>%u</lacing>\n", lacing);
            }

            fprintf(mkv->xml, "  <offset>%" PRId64 "</offset>\n", s->offset);
            fprintf(mkv->xml, "  <size>%" PRId64 "</size>\n", s->size);
            fprintf(mkv->xml, "  <timecode>%" PRId64 "</timecode>\n", s->timecode);
            fprintf(mkv->xml, "  </a>\n");
        }

        // Index sample
        if (mkv->tracks_count >= stn && mkv->tracks[stn])
        {
            mkv->tracks[stn]->sample_vector.push_back(s);
        }
        else
        {
            delete s;
            TRACE_WARNING(MKV, "simpleblock with no associated track: %i ???", stn);
        }
    }

    return s;
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

            case eid_BlockGroup:
                retcode = mkv_parse_blockgroup(bitstr, &element_sub, mkv, cluster.Timecode);
                break;

            case eid_SimpleBlock:
                mkv_parse_block(bitstr, &element_sub, mkv, sampletype_SimpleBlock, cluster.Timecode);
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
