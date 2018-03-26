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
 * \file      mp4.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "mp4.h"
#include "mp4_struct.h"
#include "mp4_box.h"
#include "mp4_stbl.h"
#include "mp4_spatial.h"
#include "mp4_gopro.h"
#include "mp4_convert.h"

#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>
#include <cinttypes>

/* ************************************************************************** */

/*!
 * \brief File Type Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 4.3 File Type Box.
 */
static int parse_ftyp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ftyp()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Read brand identifier
    unsigned int major_brand = read_bits(bitstr, 32);

    if (major_brand == fourcc_be("qt  "))
        mp4->profile = PROF_ISOBMF_MOV;
    else if (major_brand == fourcc_be("3gp4"))
        mp4->profile = PROF_ISOBMF_3GP;

    // Read informative integer for the minor version of the major brand
    unsigned int minor_version = read_bits(bitstr, 32);

    // Read a list of brands, until the end of the box
    unsigned int compatible_brands[16] = {0};

    unsigned int i = 0;
    unsigned int nb_compatible_brands = static_cast<unsigned int>((box_header->size - 16) / 4);

    if (nb_compatible_brands > 16)
        nb_compatible_brands = 16;

    for (i = 0; i < nb_compatible_brands; i++)
    {
        compatible_brands[i] = read_bits(bitstr, 32);

        if (mp4->profile == 0)
        {
            if (compatible_brands[i] == fourcc_be("mp41") ||
                compatible_brands[i] == fourcc_be("mp42"))
            {
                mp4->profile = PROF_ISOBMF_MP4;
            }
            else if (compatible_brands[i] == fourcc_be("3gp4"))
            {
                mp4->profile = PROF_ISOBMF_3GP;
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> major_brand   : 0x%08X", major_brand);
    TRACE_1(MP4, "> minor_version : %i", minor_version);
    for (i = 0; i < nb_compatible_brands; i++)
    {
        TRACE_1(MP4, "> compatible_brands[%i] : '%s' (0x%X)",
                i, getFccString_le(compatible_brands[i], fcc), compatible_brands[i]);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        if (box_header->boxtype == BOX_STYP)
            write_box_header(box_header, mp4->xml, "Segment Type");
        else
            write_box_header(box_header, mp4->xml, "File Type");

        fprintf(mp4->xml, "  <major_brand>%s</major_brand>\n", getFccString_le(major_brand, fcc));
        fprintf(mp4->xml, "  <minor_version>%u</minor_version>\n", minor_version);
        for (i = 0; i < nb_compatible_brands; i++)
        {
            fprintf(mp4->xml, "  <compatible_brands index=\"%u\">%s</compatible_brands>\n",
                    i, getFccString_le(compatible_brands[i], fcc));
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Progressive Download Information Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.3.1 Progressive Download Information Box.
 */
static int parse_pdin(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pdin()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read rate/initial_delay
    int tbr = box_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "pdin contains %i pairs of values", tbr);
#endif
    // xmlMapper
    write_box_header(box_header, mp4->xml, "Progressive Download Information");

    for (int i = 0; i < tbr; i++) // to end of box
    {
        unsigned int rate = read_bits(bitstr, 32);
        unsigned int initial_delay = read_bits(bitstr, 32);

#if ENABLE_DEBUG
        TRACE_1(MP4, "[i] > rate          : %u", i, rate);
        TRACE_1(MP4, "    > initial_delay : %u", initial_delay);

#endif
        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <rate index=\"%i\">%u</rate>\n", i, rate);
            fprintf(mp4->xml, "  <rate index=\"%i\">%u</rate>\n", i, initial_delay);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Handler Reference Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.3 Handler Reference Box.
 *
 * This box within a Media Box declares the process by which the media-data in the
 * track is presented, and thus, the nature of the media in a track.
 * With MOV files, a secondary 'hdlr' box can be found inside the 'minf' box.
 */
static int parse_hdlr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hdlr()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];
    char name[128] = {0};
    char *pname = name;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    unsigned int pre_defined = read_bits(bitstr, 32);
    unsigned int handlerType = read_bits(bitstr, 32);

    // If we have a track structure, this hdlr describe the track
    if (track && track->handlerType == 0)
    {
        track->handlerType = handlerType;
        pname = track->name;
    }

    unsigned int reserved[3];
    reserved[0] = read_bits(bitstr, 32);
    reserved[1] = read_bits(bitstr, 32);
    reserved[2] = read_bits(bitstr, 32);

    int bytes_left = box_header->size - 32;
    if (bytes_left > 0)
    {
        // check if the bytes_left is also coded in the first byte (MOV style)
        // and make sure we store no more than 128 characters
        int namesize = next_bits(bitstr, 8);
        if (bytes_left == namesize + 1)
        {
            skip_bits(bitstr, 8);
            bytes_left = namesize;
        }
        if (bytes_left > 128) bytes_left = 128;

        for (int i = 0; i < bytes_left; i++)
        {
            pname[i] = read_bits(bitstr, 8);
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> pre_defined  : %u", pre_defined);
    TRACE_1(MP4, "> handler_type : 0x%X (%s)", handlerType,
            getFccString_le(handlerType, fcc));
    TRACE_1(MP4, "> name         : '%s'", pname);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Handler Reference");
        fprintf(mp4->xml, "  <pre_defined>%u</pre_defined>\n", pre_defined);
        fprintf(mp4->xml, "  <handler_type>%s</handler_type>\n",
                getFccString_le(handlerType, fcc));
        fprintf(mp4->xml, "  <name>%s</name>\n", pname);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Apple item list box.
 */
static int parse_ilst(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ilst()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Apple item list");

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

/*!
 * \brief Meta Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.11.1 The Meta box
 *
 * A meta box contains descriptive or annotative metadata.
 */
static int parse_meta(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_meta()" CLR_RESET);
    int retcode = SUCCESS;

    if (mp4->profile != PROF_ISOBMF_MOV)
    {
        // Read FullBox attributs
        // Well the spec says its a fullbox, but apparently not...
        box_header->version = (uint8_t)read_bits(bitstr, 8);
        box_header->flags = read_bits(bitstr, 24);
    }

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Meta");

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
                case BOX_HDLR:
                    retcode = parse_hdlr(bitstr, &box_subheader, NULL, mp4);
                    break;
                case BOX_ILST:
                    retcode = parse_ilst(bitstr, &box_subheader, NULL, mp4);
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

/*!
 * \brief User Data Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.10.1 User Data Box
 */
static int parse_udta(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_udta()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "User Data");

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
                case BOX_META:
                    retcode = parse_meta(bitstr, &box_subheader, mp4);
                    break;

                case BOX_FIRM:
                    retcode = parse_firm(bitstr, &box_subheader, mp4);
                    break;
                case BOX_LENS:
                    retcode = parse_lens(bitstr, &box_subheader, mp4);
                    break;
                case BOX_CAME:
                    retcode = parse_came(bitstr, &box_subheader, mp4);
                    break;
                case BOX_SETT:
                    retcode = parse_sett(bitstr, &box_subheader, mp4);
                    break;
                case BOX_AMBA:
                    retcode = parse_amba(bitstr, &box_subheader, mp4);
                    break;
                case BOX_MUID:
                    retcode = parse_muid(bitstr, &box_subheader, mp4);
                    break;
                case BOX_HMMT:
                    retcode = parse_hmmt(bitstr, &box_subheader, mp4);
                    break;
                case BOX_BCID:
                    retcode = parse_bcid(bitstr, &box_subheader, mp4);
                    break;
                case BOX_GURI:
                    retcode = parse_guri(bitstr, &box_subheader, mp4);
                    break;
                case BOX_GUSI:
                    retcode = parse_gusi(bitstr, &box_subheader, mp4);
                    break;
                case BOX_GUMI:
                    retcode = parse_gumi(bitstr, &box_subheader, mp4);
                    break;
                case BOX_GPMF:
                    retcode = parse_gpmf(bitstr, &box_subheader, mp4);
                    break;

                case BOX_FREE:
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
 * \brief Media Header Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.2 Media Header Box.
 *
 * The media header box declares overall information that is media-independent,
 * and relevant to characteristics of the media in a track.
 */
static int parse_mdhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    if (box_header->version == 1)
    {
        track->creation_time = read_bits_64(bitstr, 64);
        track->modification_time = read_bits_64(bitstr, 64);
        track->timescale = read_bits(bitstr, 32);
        track->duration = read_bits_64(bitstr, 64);
    }
    else // if (version == 0)
    {
        track->creation_time = read_bits(bitstr, 32);
        track->modification_time = read_bits(bitstr, 32);
        track->timescale = read_bits(bitstr, 32);
        track->duration = read_bits(bitstr, 32);
    }

    /*unsigned int pad =*/ read_bit(bitstr);

    // ISO-639-2/T language code
    // Each character is packed as the difference between its ASCII value and 0x60
    track->language[0] = (uint8_t)read_bits(bitstr, 5) + 96;
    track->language[1] = (uint8_t)read_bits(bitstr, 5) + 96;
    track->language[2] = (uint8_t)read_bits(bitstr, 5) + 96;

    /*unsigned int pre_defined =*/ read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> creation_time     : %llu", track->creation_time);
    TRACE_1(MP4, "> modification_time : %llu", track->modification_time);
    TRACE_1(MP4, "> timescale   : %u", track->timescale);
    TRACE_1(MP4, "> duration    : %llu", track->duration);
    TRACE_1(MP4, "> language[3] : '%c%c%c'",
            track->language[0], track->language[1], track->language[2]);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Media Header");
        fprintf(mp4->xml, "  <creation_time>%" PRId64 "</creation_time>\n", track->creation_time);
        fprintf(mp4->xml, "  <modification_time>%" PRId64 "</modification_time>\n", track->modification_time);
        fprintf(mp4->xml, "  <timescale>%u</timescale>\n", track->timescale);
        fprintf(mp4->xml, "  <duration>%" PRId64 "</duration>\n", track->duration);
        fprintf(mp4->xml, "  <language>%c%c%c</language>\n",
                track->language[0], track->language[1], track->language[2]);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Video Media Header Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.5.2 Video Media Header Box.
 */
static int parse_vmhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_vmhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    uint16_t graphicsmode = read_bits(bitstr, 16);
    uint16_t opcolor[3] = {0};
    for (int i = 0; i < 3; i++)
    {
        opcolor[i] = read_bits(bitstr, 16);
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> graphicsmode    : %u", graphicsmode);
    TRACE_1(MP4, "> opcolor         : [%u, %u, %u]", opcolor[0], opcolor[1], opcolor[2]);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Video Media Header");
        fprintf(mp4->xml, "  <graphicsmode>%u</graphicsmode>\n", graphicsmode);
        fprintf(mp4->xml, "  <opcolor>[%u, %u, %u]</opcolor>\n", opcolor[0], opcolor[1], opcolor[2]);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sound Media Header Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.5.3 Sound Media Header Box.
 */
static int parse_smhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_smhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    uint16_t balance = read_bits(bitstr, 16);
    uint16_t reserved = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> balance    : %u", balance);
    TRACE_1(MP4, "> reserved   : %u", reserved);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Sound Media Header");
        fprintf(mp4->xml, "  <balance>%u</balance>\n", balance);
        fprintf(mp4->xml, "  <reserved>%u</reserved>\n", reserved);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Hint Media Header Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.5.4 Sound Media Header Box.
 */
static int parse_hmhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hmhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    uint16_t maxPDUsize = read_bits(bitstr, 16);
    uint16_t avgPDUsize = read_bits(bitstr, 16);
    uint32_t maxbitrate = read_bits(bitstr, 32);
    uint32_t avgbitrate = read_bits(bitstr, 32);
    uint32_t reserved = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> maxPDUsize  : %u", maxPDUsize);
    TRACE_1(MP4, "> avgPDUsize  : %u", avgPDUsize);
    TRACE_1(MP4, "> maxbitrate  : %u", maxbitrate);
    TRACE_1(MP4, "> avgbitrate  : %u", avgbitrate);
    TRACE_1(MP4, "> reserved    : %u", reserved);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Hint Media Header");
        fprintf(mp4->xml, "  <maxPDUsize>%u</maxPDUsize>\n", maxPDUsize);
        fprintf(mp4->xml, "  <avgPDUsize>%u</avgPDUsize>\n", avgPDUsize);
        fprintf(mp4->xml, "  <maxbitrate>%u</maxbitrate>\n", maxbitrate);
        fprintf(mp4->xml, "  <avgbitrate>%u</avgbitrate>\n", avgbitrate);
        fprintf(mp4->xml, "  <reserved>%u</reserved>\n", reserved);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Data Reference Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.2 Data Reference Box.
 *
 * The data reference object contains a table of data references (normally URLs)
 * that declare the location(s) of the media data used within the presentation.
 * This box does not contain informations, only other boxes.
 */
static int parse_dref(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dref()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Data Reference");

    uint32_t entry_count = read_bits(bitstr, 32);
    if (mp4->xml) fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", entry_count);

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
                case BOX_ALIS:
                case BOX_URL:
                case BOX_URN:
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

/*!
 * \brief Data Information Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.1 Data Information Box.
 *
 * The data information box contains objects that declare the location of the
 * media information in a track.
 * This box does not contain informations, only other boxes.
 */
static int parse_dinf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dinf()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Data Information");

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
                case BOX_DREF:
                    retcode = parse_dref(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Media Information Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.4 Media Information Box.
 *
 * This box contains all the objects that declare characteristic information of
 * the media in the track.
 * This box does not contain informations, only other boxes.
 */
static int parse_minf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_minf()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Media Information");

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
                case BOX_VMHD:
                    retcode = parse_vmhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_SMHD:
                    retcode = parse_smhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_HMHD:
                    retcode = parse_hmhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_HDLR:
                    retcode = parse_hdlr(bitstr, &box_subheader, NULL, mp4);
                    break;
                case BOX_DINF:
                    retcode = parse_dinf(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STBL:
                    retcode = parse_stbl(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Media Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.4.1 Media Box.
 *
 * The media declaration container contains all the objects that declare information
 * about the media data within a track.
 * This box does not contain informations, only other boxes.
 */
static int parse_mdia(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdia()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Media");

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
                case BOX_MDHD:
                    retcode = parse_mdhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_HDLR:
                    retcode = parse_hdlr(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_MINF:
                    retcode = parse_minf(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Edit List Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.6 Edit List Box.
 */
static int parse_elst(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_elst()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

#if ENABLE_DEBUG
    print_box_header(box_header);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Edit List");
    }

    // Read box content
    if (box_header->version == 1)
    {
        // TODO ?
        TRACE_WARNING(MP4, "parse_elst() version 1 not supported");
    }
    else // if (version == 0)
    {
        uint32_t entry_count = read_bits(bitstr, 32);

        TRACE_1(MP4, "> entry_count: %u", entry_count);
        if (mp4->xml) fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", entry_count);

        for (uint32_t i = 0; i < entry_count; i++)
        {
            uint32_t segmentDuration = read_bits(bitstr, 32);
            track->mediatime = read_bits(bitstr, 32);
            uint32_t mediaRate = read_bits(bitstr, 32);

            if (mp4->xml)
            {
                fprintf(mp4->xml, "  <entry index=\"%u\">segmentDuration: %u / mediaTime: %u / mediaRate: %u</entry>\n",
                        i, segmentDuration, track->mediatime, mediaRate);
            }

            // we only need one "mediaTime", used to compute framerate of "progressive download" files
            //break;
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Edit Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.5 Edit Box.
 */
static int parse_edts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_edst()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Edit");

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
                case BOX_ELST:
                    retcode = parse_elst(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Track Header Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.3.2 Track Header Box.
 *
 * This box specifies the characteristics of a single track.
 * Exactly one Track Header Box is contained in a track.
 */
static int parse_tkhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_tkhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    if (box_header->version == 1)
    {
        track->creation_time = read_bits_64(bitstr, 64);
        track->modification_time = read_bits_64(bitstr, 64);
        track->id = read_bits(bitstr, 32);
        /*const unsigned int reserved =*/ read_bits(bitstr, 32);
        track->duration = read_bits_64(bitstr, 64);
    }
    else // if (box_header->version == 0)
    {
        track->creation_time = read_bits(bitstr, 32);
        track->modification_time = read_bits(bitstr, 32);
        track->id = read_bits(bitstr, 32);
        /*const unsigned int reserved =*/ read_bits(bitstr, 32);
        track->duration = read_bits(bitstr, 32);
    }

    unsigned int reserved[2] = {0};
    reserved[0] = read_bits(bitstr, 32);
    reserved[1] = read_bits(bitstr, 32);

    int layer = read_bits(bitstr, 16);
    int alternate_group = read_bits(bitstr, 16);
    int volume = read_bits(bitstr, 16);
    /*const unsigned int reserved =*/ read_bits(bitstr, 16);

    int i = 0, matrix[9] = {0};
    for (i = 0; i < 9; i++)
    {
        matrix[i] = read_bits(bitstr, 32);
    }

    unsigned int width = read_bits(bitstr, 32);
    unsigned int height = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> creation_time     : %llu", track->creation_time);
    TRACE_1(MP4, "> modification_time : %llu", track->modification_time);
    TRACE_1(MP4, "> track_ID          : %u", track->id);
    TRACE_1(MP4, "> duration          : %llu", track->duration);
    TRACE_1(MP4, "> layer             : %i", layer);
    TRACE_1(MP4, "> alternate_group   : %i", alternate_group);
    TRACE_1(MP4, "> volume            : %i", volume);
    TRACE_1(MP4, "> matrix : [%i, %i, %i, %i, %i, %i, %i, %i, %i]",
            matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
            matrix[5], matrix[6], matrix[7], matrix[8]);
    TRACE_1(MP4, "> width  : %u", width);
    TRACE_1(MP4, "> height : %u", height);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Track Header");
        fprintf(mp4->xml, "  <creation_time>%" PRId64 "</creation_time>\n", track->creation_time);
        fprintf(mp4->xml, "  <modification_time>%" PRId64 "</modification_time>\n", track->modification_time);
        fprintf(mp4->xml, "  <track_ID>%u</track_ID>\n", track->id);
        fprintf(mp4->xml, "  <duration>%" PRId64 "</duration>\n", track->duration);
        fprintf(mp4->xml, "  <layer>%i</layer>\n", layer);
        fprintf(mp4->xml, "  <alternate_group>%i</alternate_group>\n", alternate_group);
        fprintf(mp4->xml, "  <volume>%i</volume>\n", volume);
        fprintf(mp4->xml, "  <matrix>[%i, %i, %i, %i, %i, %i, %i, %i, %i]</matrix>\n",
                matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
                matrix[5], matrix[6], matrix[7], matrix[8]);
        //for (i = 0; i < 9; i++)
        //    fprintf(mp4->xml, "  <matrix index=\"%i\">%i</matrix>\n", i, matrix[i]);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the container for individual track or stream.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.3.3 Track Reference Box.
 *
 * This box provides a reference from the containing track to another track in the
 * presentation.
 * Exactly one Track Reference Box can be contained within the Track Box.
 *
 * This box does not contain informations, only other boxes.
 */
static int parse_trak(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_trak()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Track Reference", "track");

    // Init a track structure
    unsigned int track_id = mp4->tracks_count;
    mp4->tracks[track_id] = (Mp4Track_t*)calloc(1, sizeof(Mp4Track_t));

    if (mp4->tracks[track_id] == NULL)
    {
        TRACE_ERROR(MP4, "Unable to allocate a new mp4 track!");
        retcode = FAILURE;
    }
    else
    {
        mp4->tracks[track_id]->id = track_id;
        mp4->tracks_count++;
    }

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
                case BOX_TKHD:
                    retcode = parse_tkhd(bitstr, &box_subheader, mp4->tracks[track_id], mp4);
                    break;
                case BOX_EDTS:
                    retcode = parse_edts(bitstr, &box_subheader, mp4->tracks[track_id], mp4);
                    break;
                case BOX_MDIA:
                    retcode = parse_mdia(bitstr, &box_subheader, mp4->tracks[track_id], mp4);
                    break;

                case BOX_UDTA:
                    retcode = parse_udta(bitstr, &box_subheader, mp4);
                    break;

                case BOX_UUID:
                    if (memcmp(box_subheader.usertype, SphericalVideoUUID, 16))
                        mp4->tracks[track_id]->projection = PROJECTION_EQUIRECTANGULAR;
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
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
 * \brief Parse the object descriptor box.
 *
 * From 'ISO/IEC 14496-14' specification:
 * 5.1 object descriptor Box.
 */
static int parse_iods(Bitstream_t *bitstr, Mp4Box_t *box_header, FILE *xml)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_iods()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // TODO

    // Print trak box header
    print_box_header(box_header);

    // xmlMapper
    if (xml)
    {
        write_box_header(box_header, xml, "object descriptor");
        fprintf(xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the Movie Header Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.2.2 Movie Header Box.
 *
 * This box defines overall information which is media-independent, and relevant
 * to the entire presentation considered as a whole.
 */
static int parse_mvhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mvhd()" CLR_RESET);
    int retcode = SUCCESS;
    int i = 0;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    if (box_header->version == 1)
    {
        mp4->creation_time = read_bits_64(bitstr, 64);
        mp4->modification_time = read_bits_64(bitstr, 64);
        mp4->timescale = read_bits(bitstr, 32);
        mp4->duration = read_bits_64(bitstr, 64);
    }
    else // if (version == 0)
    {
        mp4->creation_time = read_bits(bitstr, 32);
        mp4->modification_time = read_bits(bitstr, 32);
        mp4->timescale = read_bits(bitstr, 32);
        mp4->duration = read_bits(bitstr, 32);
    }

    uint32_t rate = read_bits(bitstr, 32);
    uint32_t volume = read_bits(bitstr, 16);
    /*unsigned int reserved =*/ read_bits(bitstr, 16);
    /*unsigned int reserved =*/ read_bits(bitstr, 32);
    /*unsigned int reserved =*/ read_bits(bitstr, 32);

    // Provides a transformation matrix for the video;
    int32_t matrix[9];
    for (i = 0; i < 9; i++)
    {
        matrix[i] = read_bits(bitstr, 32);
    }

    // ?
    int32_t predefined[6];
    for (i = 0; i < 6; i++)
    {
        predefined[i] = read_bits(bitstr, 32);
    }

    uint32_t next_track_ID = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> creation_time     : %llu", mp4->creation_time);
    TRACE_1(MP4, "> modification_time : %llu", mp4->modification_time);
    TRACE_1(MP4, "> timescale  : %u", mp4->timescale);
    TRACE_1(MP4, "> duration   : %llu", mp4->duration);
    TRACE_1(MP4, "> rate       : %u", rate);
    TRACE_1(MP4, "> volume     : %u", volume);
    TRACE_1(MP4, "> matrix     : [%i, %i, %i, %i, %i, %i, %i, %i, %i]",
            matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
            matrix[5], matrix[6], matrix[7], matrix[8]);
    TRACE_1(MP4, "> next track ID     : %u", next_track_ID);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Movie Header");
        fprintf(mp4->xml, "  <creation_time>%" PRId64 "</creation_time>\n", mp4->creation_time);
        fprintf(mp4->xml, "  <modification_time>%" PRId64 "</modification_time>\n", mp4->modification_time);
        fprintf(mp4->xml, "  <timescale>%u</timescale>\n", mp4->timescale);
        fprintf(mp4->xml, "  <duration>%" PRId64 "</duration>\n", mp4->duration);
        fprintf(mp4->xml, "  <rate>%u</rate>\n", rate);
        fprintf(mp4->xml, "  <volume>%u</volume>\n", volume);
        fprintf(mp4->xml, "  <matrix>[%i, %i, %i, %i, %i, %i, %i, %i, %i]</matrix>\n",
                matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
                matrix[5], matrix[6], matrix[7], matrix[8]);
        //for (i = 0; i < 9; i++)
        //    fprintf(mp4->xml, "  <matrix index=\"%i\">%i</matrix>\n", i, matrix[i]);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the container for metadata.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.2.1 Movie Box.
 */
static int parse_moov(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_moov()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Movie Box");

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
                case BOX_MVHD:
                    retcode = parse_mvhd(bitstr, &box_subheader, mp4);
                    break;
                case BOX_IODS:
                    retcode = parse_iods(bitstr, &box_subheader, mp4->xml);
                    break;
                case BOX_TRAK:
                    retcode = parse_trak(bitstr, &box_subheader, mp4);
                    break;
                case BOX_META:
                    retcode = parse_meta(bitstr, &box_subheader, mp4);
                    break;
                case BOX_UDTA:
                    retcode = parse_udta(bitstr, &box_subheader, mp4);
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
 * \brief Parse the container for individual (fragmented) track or stream.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.8.6 Track Fragment Box.
 */
static int parse_traf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_traf()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Track Fragment");

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
                case BOX_TFHD:
                case BOX_TRUN:
                case BOX_TFDT:
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

/*!
 * \brief Parse the Movie (fragmented) Header Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.8.5 Movie Fragment Header Box.
 */
static int parse_mfhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mfhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    uint32_t sequence_number = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> sequence_number     : %u", sequence_number);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Movie Fragment Header");
        fprintf(mp4->xml, "  <sequence_number>%u</sequence_number>\n", sequence_number);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the container for (fragmented) metadata.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.8.4 Movie Fragment Box.
 */
static int parse_moof(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_moof()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Movie Fragment");

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
                case BOX_MFHD:
                    retcode = parse_mfhd(bitstr, &box_subheader, mp4);
                    break;
                case BOX_TRAF:
                    retcode = parse_traf(bitstr, &box_subheader, mp4);
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
 * \brief Parse the Media Data Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.1.1 Media Data Box.
 *
 * This box contains the media data. In video tracks, this box would contain
 * video frames.
 * The parser doesn't really care for this box as long as we have already
 * indexed the A/V samples.
 */
static int parse_mdat(Bitstream_t *bitstr, Mp4Box_t *box_header, FILE *xml)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdat()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);

    // xmlMapper
    if (xml)
    {
        write_box_header(box_header, xml, "Media Data");
        fprintf(xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int mp4_fileParse(MediaFile_t *media)
{
    TRACE_INFO(MP4, BLD_GREEN "mp4_fileParse()" CLR_RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an MP4 structure
        Mp4_t mp4;
        memset(&mp4, 0, sizeof(Mp4_t));

        // A convenient way to stop the parser
        mp4.run = true;

        // xmlMapper
        xmlMapperOpen(media, &mp4.xml);

        // Loop on 1st level boxes
        while (mp4.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - 8))
        {
            // Read box header
            Mp4Box_t box_header;
            retcode = parse_box_header(bitstr, &box_header);

            // Then parse box content
            if (mp4.run == true &&
                retcode == SUCCESS &&
                bitstream_get_absolute_byte_offset(bitstr) < (box_header.offset_end - 8))
            {
                switch (box_header.boxtype)
                {
                    case BOX_FTYP:
                        retcode = parse_ftyp(bitstr, &box_header, &mp4);
                        break;
                    case BOX_STYP:
                        retcode = parse_ftyp(bitstr, &box_header, &mp4);
                        break;
                    case BOX_PDIN:
                        retcode = parse_pdin(bitstr, &box_header, &mp4);
                        break;
                    case BOX_UDTA:
                        retcode = parse_udta(bitstr, &box_header, &mp4);
                        break;
                    case BOX_SIDX:
                        retcode = parse_unknown_box(bitstr, &box_header, mp4.xml);
                        break;
                    case BOX_MOOV:
                        retcode = parse_moov(bitstr, &box_header, &mp4);
                        break;
                    case BOX_MOOF:
                        retcode = parse_moof(bitstr, &box_header, &mp4);
                        break;
                    case BOX_MDAT:
                        retcode = parse_mdat(bitstr, &box_header, mp4.xml);
                        break;
                    case BOX_FREE:
                        retcode = parse_unknown_box(bitstr, &box_header, mp4.xml);
                        break;
                    case BOX_UUID:
                        retcode = parse_unknown_box(bitstr, &box_header, mp4.xml);
                        break;
                    default:
                        retcode = parse_unknown_box(bitstr, &box_header, mp4.xml);
                        break;
                }

                jumpy_mp4(bitstr, NULL, &box_header);
            }
        }

        // xmlMapper
        if (xmlMapperFinalize(mp4.xml) == SUCCESS)
            media->container_mapper_fd = mp4.xml;

        // Convert internal MP4 representation into a MediaFile_t
        mp4_convert(media, &mp4);

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
