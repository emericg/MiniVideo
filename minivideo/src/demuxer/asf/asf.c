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
 * \file      asf.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "asf.h"
#include "asf_object.h"
#include "asf_convert.h"
#include "asf_struct.h"
#include "../xml_mapper.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_twocc.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ************************************************************************** */

static int parse_header(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_header()" CLR_RESET);
    int retcode = SUCCESS;

    asf->asfh.NumberOfHeaderObjects = endian_flip_32(read_bits(bitstr, 32));
    asf->asfh.Reserved1 = read_bits(bitstr, 8);
    asf->asfh.Reserved2 = read_bits(bitstr, 8);

#if ENABLE_DEBUG
    print_asf_object(header);
    TRACE_1(ASF, "> NumberOfHeaderObjects: %u", asf->asfh.NumberOfHeaderObjects);
    TRACE_1(ASF, "> Reserved1            : %u", asf->asfh.Reserved1);
    TRACE_1(ASF, "> Reserved2            : %u", asf->asfh.Reserved1);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (asf->xml)
    {
        write_asf_object(header, asf->xml, "Header Object");
        fprintf(asf->xml, "  <NumberOfHeaderObjects>%lu</NumberOfHeaderObjects>\n", asf->asfh.NumberOfHeaderObjects);
        fprintf(asf->xml, "  <Reserved1>%u</Reserved1>\n", asf->asfh.Reserved1);
        fprintf(asf->xml, "  <Reserved2>%u</Reserved2>\n", asf->asfh.Reserved2);
    }

    while (asf->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (header->offset_end - 24))
    {
        // Read ASF object
        AsfObject_t asf_object;
        retcode = parse_asf_object(bitstr, &asf_object);

        if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_File_Properties_Object], 16) == 0)
        {
            TRACE_INFO(ASF, "ASF_File_Properties_Object");
            retcode = parse_unknown_object(bitstr, &asf_object, asf->xml);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Stream_Properties_Object], 16) == 0)
        {
            TRACE_INFO(ASF, "ASF_Stream_Properties_Object");
            retcode = parse_unknown_object(bitstr, &asf_object, asf->xml);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Header_Extension_Object], 16) == 0)
        {
            TRACE_INFO(ASF, "ASF_Header_Extension_Object");
            retcode = parse_unknown_object(bitstr, &asf_object, asf->xml);
        }
        else
        {
            retcode = parse_unknown_object(bitstr, &asf_object, asf->xml);
        }

        retcode = jumpy_asf(bitstr, header, &asf_object);
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int parse_data(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_data()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(header);
    write_asf_object(header, asf->xml, "Data Object");

    //

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int parse_index(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_index()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(header);
    write_asf_object(header, asf->xml, "Index Object");

    //

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_simple_index(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_simple_index()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(header);
    write_asf_object(header, asf->xml, "Simple Index Object");

    //

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int asf_fileParse(MediaFile_t *media)
{
    TRACE_INFO(ASF, BLD_GREEN "asf_fileParse()" CLR_RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an ASF structure
        asf_t asf;
        memset(&asf, 0, sizeof(asf));

        // A convenient way to stop the parser
        asf.run = true;

        // xmlMapper
        xmlMapperOpen(media, &asf.xml);

        // Loop on 1st level list
        while (asf.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - 24))
        {
            // Read ASF object
            AsfObject_t asf_object;
            retcode = parse_asf_object(bitstr, &asf_object);

            if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Header_Object], 16) == 0)
            {
                TRACE_INFO(ASF, "ASF_Header_Object");
                retcode = parse_header(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Data_Object], 16) == 0)
            {
                TRACE_INFO(ASF, "ASF_Data_Object");
                retcode = parse_data(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Simple_Index_Object], 16) == 0)
            {
                TRACE_INFO(ASF, "ASF_Simple_Index_Object");
                retcode = parse_simple_index(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Index_Object], 16) == 0)
            {
                TRACE_INFO(ASF, "ASF_Index_Object");
                retcode = parse_index(bitstr, &asf_object, &asf);
            }
            else
            {
                retcode = parse_unknown_object(bitstr, &asf_object, asf.xml);
            }

            retcode = jumpy_asf(bitstr, NULL, &asf_object);
        }

        // xmlMapper
        if (xmlMapperFinalize(asf.xml) == SUCCESS)
            media->container_mapper_fd = asf.xml;

        // Go for the indexation
        retcode = asf_indexer(bitstr, media, &asf),
        media->container_profile = asf.profile;

        // Free asf_t structure content
        asf_clean(&asf);

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
