/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
 * \file      mkv.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "mkv.h"
#include "mkv_struct.h"
#include "ebml.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>

/* ************************************************************************** */

static int mkv_parse_info(Bitstream_t *bitstr)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info()" CLR_RESET);
    int retcode = FAILURE;
    uint64_t elementSize = read_ebml_size(bitstr);
    uint64_t elementOffset = bitstream_get_absolute_byte_offset(bitstr);

    TRACE_1(MKV, "* info size   = %llu", elementSize);
    TRACE_1(MKV, "* info offset = %llu", elementOffset);

    while ((uint64_t)bitstream_get_absolute_byte_offset(bitstr) < (elementOffset + elementSize))
    {
        switch (read_ebml_eid(bitstr))
        {
            default:
                retcode = FAILURE;
                break;
        }
    }

    // TODO finish this
    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_seekhead_seek(Bitstream_t *bitstr)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead_seek()" CLR_RESET);
    int retcode = FAILURE;
    int64_t elementSize = (int64_t)read_ebml_size(bitstr);
    int64_t elementOffset = bitstream_get_absolute_byte_offset(bitstr);
    uint64_t subelementSize = 0;
    uint64_t subelementOffset = 0;
    uint64_t subelementValue = 0;

    TRACE_1(MKV, "  seekhead_seek size   = %llu", elementSize);
    TRACE_1(MKV, "  seekhead_seek offset = %llu", elementOffset);

    while (bitstream_get_absolute_byte_offset(bitstr) < (elementOffset + elementSize))
    {
        switch (read_ebml_eid(bitstr))
        {
            case element_SeekId:
                subelementSize = read_ebml_size(bitstr);
                subelementOffset = bitstream_get_absolute_byte_offset(bitstr);
                subelementValue = read_bits_64(bitstr, subelementSize*8);
                TRACE_1(MKV, "    seek_id size   = %llu", subelementSize);
                TRACE_1(MKV, "    seek_id offset = %llu", elementOffset);
                TRACE_1(MKV, "    seek_id value  = %llu", subelementValue);
                break;
            case element_SeekPosition:
                subelementSize = read_ebml_size(bitstr);
                subelementOffset = bitstream_get_absolute_byte_offset(bitstr);
                subelementValue = read_ebml_data_uint(bitstr);
                TRACE_1(MKV, "    seek_position size   = %llu", subelementSize);
                TRACE_1(MKV, "    seek_position offset = %llu", subelementOffset);
                TRACE_1(MKV, "    seek_position value  = %llu", subelementValue);
                break;
            default:
                retcode = FAILURE;
                break;
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_seekhead(Bitstream_t *bitstr)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead()" CLR_RESET);
    int retcode = FAILURE;
    int64_t elementSize = (int64_t)read_ebml_size(bitstr);
    int64_t elementOffset = bitstream_get_absolute_byte_offset(bitstr);

    TRACE_1(MKV, "* seekhead size   = %llu", elementSize);
    TRACE_1(MKV, "* seekhead offset = %llu", elementOffset);

    while (bitstream_get_absolute_byte_offset(bitstr) < (elementOffset + elementSize))
    {
        switch (read_ebml_eid(bitstr))
        {
            case element_Seek:
                retcode = mkv_parse_seekhead_seek(bitstr);
                break;
            default:
                retcode = FAILURE;
                break;
        }
    }

    // TODO finish this
    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_segment(Bitstream_t *bitstr)
{
    TRACE_INFO(MKV, BLD_GREEN "ebml_parse_segment()" CLR_RESET);
    int retcode = FAILURE;
    int64_t segmentSize = 0;
    int64_t segmentOffset = 0;

    if (read_bits(bitstr, 32) == element_Segment)
    {
        segmentSize = (int64_t)read_ebml_size(bitstr);
        segmentOffset = bitstream_get_absolute_byte_offset(bitstr);

        TRACE_1(MKV, "* segment size   = %llu", segmentSize);
        TRACE_1(MKV, "* segment offset = %llu", segmentOffset);

        while (bitstream_get_absolute_byte_offset(bitstr) < (segmentOffset + segmentSize))
        {
            switch (read_ebml_eid(bitstr))
            {
                case element_SeekHead:
                    mkv_parse_seekhead(bitstr);
                    retcode = SUCCESS;
                    break;
                case element_Info:
                    mkv_parse_info(bitstr);
                    retcode = SUCCESS;
                    break;
                case element_Cluster:
                    TRACE_1(MKV, "element_Cluster");
                    retcode = SUCCESS;
                    break;
                case element_Tracks:
                    TRACE_1(MKV, "element_Tracks");
                    retcode = SUCCESS;
                    break;
                case element_Cues:
                    TRACE_1(MKV, "element_Cues");
                    retcode = SUCCESS;
                    break;
                case element_Attachments:
                    TRACE_1(MKV, "element_Attachments");
                    retcode = SUCCESS;
                    break;
                case element_Chapters:
                    TRACE_1(MKV, "element_Chapters");
                    retcode = SUCCESS;
                    break;
                case element_Tags:
                    TRACE_1(MKV, "element_Tags");
                    retcode = SUCCESS;
                    break;
                default:
                    retcode = FAILURE;
                    break;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int mkv_fileParse(MediaFile_t *media)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_fileParse()" CLR_RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Parse the EBML header
        retcode = ebml_parse_header(bitstr);

        // Parste the segment content
        if (retcode == SUCCESS)
            retcode = mkv_parse_segment(bitstr);

        // Free bitstream
        free_bitstream(&bitstr);
    }
    else
    {
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
