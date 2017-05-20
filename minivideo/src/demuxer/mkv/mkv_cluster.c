/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
/* ************************************************************************** */

int mkv_parse_blockgroup(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
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
                write_ebml_element(&element_sub, mkv->xml, "Block");
                if (mkv->xml) fprintf(mkv->xml, "  </a>\n");
                break;
            case eid_BlockDuration:
                /*BlockDuration =*/ read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "BlockDuration");
                break;
            case eid_ReferencePriority:
                /*ReferencePriority =*/ read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "ReferencePriority");
                break;
            case eid_ReferenceBlock:
                /*ReferenceBlock =*/ read_ebml_data_int2(bitstr, &element_sub, mkv->xml, "ReferenceBlock");
                break;
            case eid_CodecState:
                /*CodecState =*/ read_ebml_data_binary2(bitstr, &element_sub, mkv->xml, "CodecState");
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
                cluster.Timecode = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "Timecode");
                break;

            case eid_SimpleBlock:
            {
                write_ebml_element(&element_sub, mkv->xml, "SimpleBlock");
                if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

                uint32_t stn = read_ebmllike_value(bitstr) - 1;
                int16_t stc = (int)read_bits(bitstr, 16);
                uint32_t flags = read_bits(bitstr, 8);

                //TRACE_1(MKV, "simpleblock track number: %u", stn);
                //TRACE_1(MKV, "simpleblock timecode: %u", stc + cluster.Timecode);
                //TRACE_1(MKV, "simpleblock flags: %u", flags);

                if ((unsigned)mkv->tracks_count >= stn && mkv->tracks[stn])
                {
                    mkv_sample_t *s = malloc(sizeof(mkv_sample_t));
                    if (s)
                    {
                        s->offset = bitstream_get_absolute_byte_offset(bitstr);
                        s->size = element_sub.size;
                        s->timecode = stc + cluster.Timecode;
                        vector_add(&(mkv->tracks[stn]->sample_vector), s);
                    }
                }
                else
                {
                    TRACE_WARNING(MKV, "simpleblock with no associated track: %i", stn);
                }
            } break;

            case eid_BlockGroup:
                retcode = mkv_parse_blockgroup(bitstr, &element_sub, mkv);
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
