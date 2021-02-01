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
 * \file      mp4_meta.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2020
 */

// minivideo headers
#include "mp4_meta.h"
#include "mp4_picture.h"
#include "mp4_stbl.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>

/* ************************************************************************** */

int parse_chpl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_chpl()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Chapters");

    skip_bits(bitstr, 32); // ?

    mp4->chapters_count = read_mp4_uint8(bitstr, mp4->xml, "entry_count");
    mp4->chapters = (Chapter_t *)calloc(mp4->chapters_count, sizeof(Chapter_t));

    char tagname[24]= {0};
    for (unsigned i = 0; i < mp4->chapters_count; i++)
    {
        xmlSpacer(mp4->xml, "chapter", i);

        snprintf(tagname, 24, "timestamp_%u", i);
        mp4->chapters[i].pts = read_mp4_uint64(bitstr, mp4->xml, tagname) / 10000;

        snprintf(tagname, 24, "title_size_%u", i);
        int sz = read_mp4_uint8(bitstr, mp4->xml, tagname);

        if (sz > 0)
        {
            snprintf(tagname, 24, "title_%u", i);
            mp4->chapters[i].name = read_mp4_string(bitstr, sz, mp4->xml, tagname);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_mdta(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdta()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "item key");

    // Read box content
    char *item_name = read_mp4_string(bitstr, box_header->size, mp4->xml, "item_name");
    free(item_name);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief Apple item keys box.
 */
int parse_keys(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_keys()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Apple item keys");

    uint32_t entry_count = read_mp4_uint32(bitstr, mp4->xml, "entry_count");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_MDTA:
                    retcode = parse_mdta(bitstr, &box_subheader, mp4);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                    break;
            }

            jumpy_mp4(bitstr, box_header, &box_subheader);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_data(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_data()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "item data");

    // Read box content
    uint32_t reserved = read_mp4_uint32(bitstr, mp4->xml, "reserved");

    char *annotation = read_mp4_string(bitstr, box_header->size, mp4->xml, "annotation");
    free(annotation);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief Apple item list box.
 */
int parse_ilst(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ilst()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    //box_header->version = (uint8_t)read_bits(bitstr, 8);
    //box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Apple item list");

    //uint32_t reserved = read_mp4_uint32(bitstr, mp4->xml, "reserved");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        xmlSpacer(mp4->xml, "annotation");
        uint32_t type = read_mp4_uint32(bitstr, mp4->xml, "type");
        uint32_t id = read_mp4_uint32(bitstr, mp4->xml, "id");

        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_DATA:
                    retcode = parse_data(bitstr, &box_subheader, mp4);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                    break;
            }

            jumpy_mp4(bitstr, box_header, &box_subheader);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
