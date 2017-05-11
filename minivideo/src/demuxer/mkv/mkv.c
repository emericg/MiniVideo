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

static int mkv_parse_info(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml);
    if (mkv->xml) fprintf(mkv->xml, "  <title>Info</title>\n");

    while (mkv->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (element->offset_end))
    {
        // Parse sub element
        EbmlElement_t element_sub;
        retcode = parse_ebml_element(bitstr, &element_sub);

        // Then parse sub element content
        if (mkv->run == true && retcode == SUCCESS)
        {
            switch (element_sub.eid)
            {
            case eid_SegmentUID: {
                char *SegmentUID = read_ebml_data_binary(bitstr);
                TRACE_1(MKV, "* SegmentUID   = '%s'", SegmentUID);
            } break;
            case eid_SegmentFilename: {
                char *SegmentFilename = read_ebml_data_binary(bitstr);
                TRACE_1(MKV, "* Segment Filename   = %lu", SegmentFilename);
            } break;

            default:
                retcode = ebml_parse_unknown(bitstr, element, mkv->xml);
                break;
            }
        }

        jumpy_mkv(bitstr, NULL, element);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_seekhead_seek(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead_seek()" CLR_RESET);
    int retcode = SUCCESS;

    uint8_t *SeekID = NULL;
    uint64_t SeekPosition = 0;

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
            case eid_SeekId:
                SeekID = read_ebml_data_string(bitstr, element_sub.size);
                break;
            case eid_SeekPosition:
                SeekPosition = read_bits_64(bitstr, element_sub.size*8);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        jumpy_mkv(bitstr, element, &element_sub);
    }

#if ENABLE_DEBUG
    print_ebml_element(element);
    TRACE_1(MKV, "SeekID        = '%s'", SeekID);
    TRACE_1(MKV, "SeekPosition    = %llu", SeekPosition);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        write_ebml_element(element, mkv->xml);
        fprintf(mkv->xml, "  <title>Seek</title>\n");
        fprintf(mkv->xml, "  <SeekID>%s</SeekID>\n", SeekID);
        fprintf(mkv->xml, "  <SeekPosition>%lu</SeekPosition>\n", SeekPosition);
        fprintf(mkv->xml, "  </atom>\n");
    }

    free(SeekID);

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_seekhead(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    write_ebml_element(element, mkv->xml);
    if (mkv->xml) fprintf(mkv->xml, "  <title>SeekHead</title>\n");

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
            case eid_Seek:
                retcode = mkv_parse_seekhead_seek(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_segment(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_segment()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml);
    if (mkv->xml) fprintf(mkv->xml, "  <title>Segment</title>\n");

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
            case eid_SeekHead:
                retcode = mkv_parse_seekhead(bitstr, &element_sub, mkv);
                break;
            case eid_Info:
                retcode = mkv_parse_info(bitstr, &element_sub, mkv);
                break;
            case eid_Cluster:
                TRACE_INFO(MKV, "element_Cluster");
                retcode = FAILURE;
                break;
            case eid_Tracks:
                TRACE_INFO(MKV, "element_Tracks");
                retcode = FAILURE;
                break;
            case eid_Cues:
                TRACE_INFO(MKV, "element_Cues");
                retcode = FAILURE;
                break;
            case eid_Attachments:
                TRACE_INFO(MKV, "element_Attachments");
                retcode = FAILURE;
                break;
            case eid_Chapters:
                TRACE_INFO(MKV, "element_Chapters");
                retcode = FAILURE;
                break;
            case eid_Tags:
                TRACE_INFO(MKV, "element_Tags");
                retcode = FAILURE;
                break;
            case eid_void:
                TRACE_INFO(MKV, "element_Void");
                retcode = FAILURE;
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        //jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    return retcode;
}

/* ************************************************************************** */

int ebml_parse_header(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "ebml_parse_header()" CLR_RESET);
    int retcode = SUCCESS;

    uint64_t EBMLVersion = 1;
    uint64_t EBMLReadVersion = 1;
    uint64_t EBMLMaxIDLength = 4;
    uint64_t EBMLMaxSizeLength = 8;
    uint8_t *DocType = NULL;
    uint64_t DocTypeVersion = 1;
    uint64_t DocTypeReadVersion = 1;

    while (mkv->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < element->offset_end)
    {
        // Parse sub element
        EbmlElement_t element_sub;
        retcode = parse_ebml_element(bitstr, &element_sub);

        // Then parse sub element content
        if (mkv->run == true && retcode == SUCCESS)
        {
            switch (element_sub.eid)
            {
                case eid_EBMLVersion:
                    EBMLVersion = read_bits_64(bitstr, element_sub.size*8);
                    break;
                case eid_EBMLReadVersion:
                    EBMLReadVersion = read_bits_64(bitstr, element_sub.size*8);
                    break;
                case eid_EBMLMaxIDLength:
                    EBMLMaxIDLength = read_bits_64(bitstr, element_sub.size*8);
                    break;
                case eid_EBMLMaxSizeLength:
                    EBMLMaxSizeLength = read_bits_64(bitstr, element_sub.size*8);
                    break;
                case eid_DocType:
                    DocType = read_ebml_data_string(bitstr, element_sub.size);
                    break;
                case eid_DocTypeVersion:
                    DocTypeVersion = read_bits_64(bitstr, element_sub.size*8);
                    break;
                case eid_DocTypeReadVersion:
                    DocTypeReadVersion = read_bits_64(bitstr, element_sub.size*8);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, element, mkv->xml);
                    break;
            }
        }

        jumpy_mkv(bitstr, NULL, &element_sub);
    }

#if ENABLE_DEBUG
    print_ebml_element(element);
    TRACE_1(MKV, "EBMLVersion        = %llu", EBMLVersion);
    TRACE_1(MKV, "EBMLReadVersion    = %llu", EBMLReadVersion);
    TRACE_1(MKV, "EBMLMaxIDLength    = %llu", EBMLMaxIDLength);
    TRACE_1(MKV, "EBMLMaxSizeLength  = %llu", EBMLMaxSizeLength);
    TRACE_1(MKV, "DocType            = '%s'", DocType);
    TRACE_1(MKV, "DocTypeVersion     = %llu", DocTypeVersion);
    TRACE_1(MKV, "DocTypeReadVersion = %llu", DocTypeReadVersion);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        write_ebml_element(element, mkv->xml);
        fprintf(mkv->xml, "  <title>EBML Header</title>\n");
        fprintf(mkv->xml, "  <EBMLVersion>%llu</EBMLVersion>\n", EBMLVersion);
        fprintf(mkv->xml, "  <EBMLReadVersion>%llu</EBMLReadVersion>\n", EBMLReadVersion);
        fprintf(mkv->xml, "  <EBMLMaxIDLength>%llu</EBMLMaxIDLength>\n", EBMLMaxIDLength);
        fprintf(mkv->xml, "  <EBMLMaxSizeLength>%llu</EBMLMaxSizeLength>\n", EBMLMaxSizeLength);
        fprintf(mkv->xml, "  <DocType>%s</DocType>\n", DocType);
        fprintf(mkv->xml, "  <DocTypeVersion>%llu</DocTypeVersion>\n", DocTypeVersion);
        fprintf(mkv->xml, "  <DocTypeReadVersion>%llu</DocTypeReadVersion>\n", DocTypeReadVersion);
        fprintf(mkv->xml, "  </atom>\n");
    }

    free(DocType);

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
        // Init an MKV structure
        mkv_t mkv;
        memset(&mkv, 0, sizeof(mkv_t));

        // A convenient way to stop the parser
        mkv.run = true;

        // xmlMapper
        xmlMapperOpen(media, &mkv.xml);

        // Loop on 1st level boxes
        while (mkv.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < media->file_size)
        {
            // Read element
            EbmlElement_t element;
            retcode = parse_ebml_element(bitstr, &element);

            // Then parse element content
            if (mkv.run == true &&
                retcode == SUCCESS &&
                bitstream_get_absolute_byte_offset(bitstr) < element.offset_end)
            {
                switch (element.eid)
                {
                case eid_EBML:
                    retcode = ebml_parse_header(bitstr, &element, &mkv);
                    break;
                case eid_Segment:
                    retcode = mkv_parse_segment(bitstr, &element, &mkv);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element, mkv.xml);
                    break;
                }

                jumpy_mkv(bitstr, NULL, &element);
            }
        }

        // xmlMapper
        if (xmlMapperFinalize(mkv.xml) == SUCCESS)
            media->container_mapper_fd = mkv.xml;

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
