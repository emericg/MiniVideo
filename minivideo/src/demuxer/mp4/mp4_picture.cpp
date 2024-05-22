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
 * \file      mp4_picture.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2020
 */

// minivideo headers
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

static int parse_infe(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_infe()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Information entry");

    // Read box content
    uint16_t item_id = read_mp4_uint16(bitstr, mp4->xml, "item_id");
    uint16_t reserved = read_mp4_uint16(bitstr, mp4->xml, "reserved");
    uint32_t item_type = read_mp4_fcc(bitstr, mp4->xml, "item_type");
    char *item_name = read_mp4_string(bitstr, box_header->size, mp4->xml, "item_name");
    free(item_name);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

int parse_iinf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iinf()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Information entries");

    // Read box content
    track->pict_entry_count = read_mp4_uint16(bitstr, mp4->xml, "entry_count");

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
                case BOX_INFE:
                    retcode = parse_infe(bitstr, &box_subheader, track, mp4);
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
/* ************************************************************************** */

int parse_iprp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iprp()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item Property");

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
                case BOX_IPCO:
                    retcode = parse_ipco(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_IPMA:
                    retcode = parse_ipma(bitstr, &box_subheader, track, mp4);
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

static int parse_ispe(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ispe()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Image spacial Extent");

    unsigned width = read_mp4_uint32(bitstr, mp4->xml, "width");
    unsigned height = read_mp4_uint32(bitstr, mp4->xml, "height");

    if (track)
    {
        track->width = width;
        track->height = width;
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

static int parse_irot(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_irot()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Image rotation");

    unsigned rotation = read_mp4_uint8(bitstr, mp4->xml, "rotation");

    if (track)
    {
        track->rotation = rotation * 90.0;
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

static int parse_iscl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iscl()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Image scaling");

    uint16_t target_width_numerator = read_mp4_uint16(bitstr, mp4->xml, "target_width_numerator");
    uint16_t target_width_denominator = read_mp4_uint16(bitstr, mp4->xml, "target_width_denominator");
    uint16_t target_height_numerator = read_mp4_uint16(bitstr, mp4->xml, "target_height_numerator");
    uint16_t target_height_denominator = read_mp4_uint16(bitstr, mp4->xml, "target_height_denominator");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

static int parse_pixi(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pixi()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Pixel information");

    unsigned numChannels = read_mp4_uint8(bitstr, mp4->xml, "numChannels");
    for (unsigned i = 0;
         (i < numChannels) && (bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end);
         ++i)
    {
        xmlSpacer(mp4->xml, "Channel", i);
        unsigned bitsPerChannel = read_mp4_uint8(bitstr, mp4->xml, "bitsPerChannel");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

static int parse_rloc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_rloc()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Image Relative Location");

    unsigned horizontal_offset = read_mp4_uint32(bitstr, mp4->xml, "horizontal_offset");
    unsigned vertical_offset = read_mp4_uint32(bitstr, mp4->xml, "vertical_offset");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

int parse_ipco(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ipco()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item Property Container");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        //case BOX_COLL:
        //    retcode = parse_coll(bitstr, &box_subsubheader, track, mp4);
         //   break;
        // Then parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_AVCC: {
                    track->codec = CODEC_H264;
                    retcode = parse_avcC(bitstr, &box_subheader, track, mp4);
                } break;
                case BOX_HVCC: {
                    track->codec = CODEC_H265;
                    retcode = parse_hvcC(bitstr, &box_subheader, track, mp4);
                } break;
                case BOX_VVCC: {
                    track->codec = CODEC_H266;
                    retcode = parse_vvcC(bitstr, &box_subheader, track, mp4);
                } break;
                case BOX_AV1C: {
                    track->codec = CODEC_AV1;
                    retcode = parse_av1C(bitstr, &box_subheader, track, mp4);
                } break;
                case BOX_JPGC: {
                    track->codec = CODEC_JPEG;
                    retcode = parse_jpgC(bitstr, &box_subheader, track, mp4);
                } break;

                // imir // ?
                // rloc // Relative Location
                // auxC // Image properties for auxiliary images

                case fourcc_be("clap"):
                    retcode = parse_clap(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("pasp"):
                    retcode = parse_pasp(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("colr"):
                    retcode = parse_colr(bitstr, &box_subheader, track, mp4);
                    break;

                case fourcc_be("ispe"):
                    retcode = parse_ispe(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("pixi"):
                    retcode = parse_pixi(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("irot"):
                    retcode = parse_irot(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("iscl"):
                    retcode = parse_iscl(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("rloc"):
                    retcode = parse_rloc(bitstr, &box_subheader, track, mp4);
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

int parse_ipma(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ipma()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item Property Association");

    // Read box content
    unsigned entry_count = read_mp4_uint32(bitstr, mp4->xml, "entry_count");

    for (unsigned i = 0; (i < entry_count); i++)
    {
        if (bitstream_get_absolute_byte_offset(bitstr) >= box_header->offset_end) break;

        xmlSpacer(mp4->xml, "Item", i);
        uint16_t item_id = read_mp4_uint16(bitstr, mp4->xml, "item_id");
        uint16_t item_size = read_mp4_uint8(bitstr, mp4->xml, "item_size");
        uint8_t *item_data = read_mp4_data(bitstr, item_size, mp4->xml, "item_data");
        free(item_data);
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_dimg(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dimg()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "dimg");

    // Read box content
    uint16_t from_item_id = read_mp4_uint16(bitstr, mp4->xml, "from_item_id");
    uint16_t entry_count = read_mp4_uint16(bitstr, mp4->xml, "entry_count");

    for (int i = 0; i < entry_count; i++)
    {
        uint16_t to_item_id = read_mp4_uint16(bitstr, mp4->xml, "to_item_id");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_thmb(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_thmb()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "thmb");

    // Read box content
    uint16_t from_item_id = read_mp4_uint16(bitstr, mp4->xml, "from_item_id");
    uint16_t entry_count = read_mp4_uint16(bitstr, mp4->xml, "entry_count");

    for (int i = 0;
         (i < entry_count) && (bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end);
         i++)
    {
        uint16_t to_item_id = read_mp4_uint16(bitstr, mp4->xml, "to_item_id");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_cdsc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_cdsc()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "cdsc");

    // Read box content
    uint16_t from_item_id = read_mp4_uint16(bitstr, mp4->xml, "from_item_id");
    uint16_t entry_count = read_mp4_uint16(bitstr, mp4->xml, "entry_count");

    for (int i = 0;
         (i < entry_count) && (bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end);
         i++)
    {
        uint16_t to_item_id = read_mp4_uint16(bitstr, mp4->xml, "to_item_id");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_iref(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iref()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item reference");

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
                case fourcc_be("dimg"):
                    retcode = parse_dimg(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("thmb"):
                    retcode = parse_thmb(bitstr, &box_subheader, track, mp4);
                    break;
                case fourcc_be("cdsc"):
                    retcode = parse_cdsc(bitstr, &box_subheader, track, mp4);
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
/* ************************************************************************** */

/*!
 * \brief Item data.
 */
int parse_idat(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_idat()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes?
    //box_header->version = (uint8_t)read_bits(bitstr, 8);
    //box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item reference");

    // Read box content
    uint32_t reserved = read_mp4_uint32(bitstr, mp4->xml, "reserved");
    uint16_t width = read_mp4_uint16(bitstr, mp4->xml, "width");
    uint16_t height = read_mp4_uint16(bitstr, mp4->xml, "height");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_iloc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iloc()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Item location");

    // Read box content
    uint8_t offset_size = read_mp4_uint(bitstr, 4, mp4->xml, "offset_size");
    uint8_t length_size = read_mp4_uint(bitstr, 4, mp4->xml, "length_size");
    uint8_t base_offset_size = read_mp4_uint(bitstr, 4, mp4->xml, "base_offset_size");

    uint8_t index_size = 0;
    if (box_header->version == 1 || box_header->version == 2)
    {
        index_size = read_mp4_uint(bitstr, 4, mp4->xml, "index_size");
    }

    uint32_t entry_count = 0;
    if (box_header->version < 2)
    {
        entry_count = read_mp4_uint16(bitstr, mp4->xml, "entry_count");
    }
    else if (box_header->version == 2)
    {
        entry_count = read_mp4_uint32(bitstr, mp4->xml, "entry_count");
    }

    for (unsigned i = 0;
         (i < entry_count) && (bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 4));
         i++)
    {
        uint32_t item_id = 0;
        if (box_header->version < 2)
        {
            item_id = read_mp4_uint16(bitstr, mp4->xml, "item_id");
        }
        else if (box_header->version == 2)
        {
            item_id = read_mp4_uint32(bitstr, mp4->xml, "item_id");
        }

        if (box_header->version == 1 || box_header->version == 2)
        {
            uint16_t reserved = read_mp4_uint(bitstr, 12, mp4->xml, "reserved");
            uint16_t construction_method = read_mp4_uint(bitstr, 4, mp4->xml, "construction_method");
        }

        uint16_t data_reference_index = read_mp4_uint16(bitstr, mp4->xml, "data_reference_index");
        uint16_t base_offset = read_mp4_uint(bitstr, 8*base_offset_size, mp4->xml, "base_offset");

        unsigned extent_count = read_mp4_uint16(bitstr, mp4->xml, "extent_count");
        for (unsigned j = 0; j < extent_count; j++)
        {
            if ((box_header->version == 1 || box_header->version == 2) && (index_size > 0))
            {
                uint16_t extent_index = read_mp4_uint(bitstr, 8*index_size, mp4->xml, "extent_index");
            }

            uint16_t extent_offset = read_mp4_uint(bitstr, 8*offset_size, mp4->xml, "extent_offset");
            uint16_t extent_length = read_mp4_uint(bitstr, 8*length_size, mp4->xml, "extent_length");
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Primary item reference.
 */
int parse_pitm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pitm()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    uint16_t mainref = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> main item reference : %u", mainref);
#endif

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Primary Item reference");
        fprintf(mp4->xml, "  <main_item_reference>%u</main_item_reference>\n", mainref);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
