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

static int mkv_parse_track(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml);
    if (mkv->xml) fprintf(mkv->xml, "  <title>Track</title>\n");

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

static int mkv_parse_info_chapter(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info_chapter()" CLR_RESET);
    int retcode = SUCCESS;

    uint64_t ChapterTranslateEditionUID = 0;
    uint64_t ChapterTranslateCodec = 0;
    uint8_t *ChapterTranslateID = NULL;

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
            case eid_ChapterTranslateEditionUID:
                ChapterTranslateEditionUID = read_bits_64(bitstr, element_sub.size*8);
                break;
            case eid_ChapterTranslateCodec:
                ChapterTranslateCodec = read_bits_64(bitstr, element_sub.size*8);
                break;
            case eid_ChapterTranslateID:
                ChapterTranslateID = read_ebml_data_binary(bitstr, element_sub.size);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, element, mkv->xml);
                break;
            }
        }

        jumpy_mkv(bitstr, element, &element_sub);
    }

#if ENABLE_DEBUG
    print_ebml_element(element);
    TRACE_1(MKV, "ChapterTranslateEditionUID= %llu", ChapterTranslateEditionUID);
    TRACE_1(MKV, "ChapterTranslateCodec     = %llu", ChapterTranslateCodec);
    TRACE_1(MKV, "ChapterTranslateID        = '%s'", ChapterTranslateID);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mkv->xml)
    {
        write_ebml_element(element, mkv->xml);
        fprintf(mkv->xml, "  <title>ChapterTranslate</title>\n");
        fprintf(mkv->xml, "  <ChapterTranslateEditionUID>%lu</ChapterTranslateEditionUID>\n", ChapterTranslateEditionUID);
        fprintf(mkv->xml, "  <ChapterTranslateCodec>%lu</ChapterTranslateCodec>\n", ChapterTranslateCodec);
        fprintf(mkv->xml, "  <ChapterTranslateID>%s</ChapterTranslateID>\n", ChapterTranslateID);
        fprintf(mkv->xml, "  </atom>\n");
    }

    free(ChapterTranslateID);

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_info(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info()" CLR_RESET);
    int retcode = SUCCESS;

    uint8_t *SegmentUID = NULL;
    uint8_t *SegmentFilename = NULL;
    uint8_t *PrevUID = NULL;
    uint8_t *PrevFilename = NULL;
    uint8_t *NextUID = NULL;
    uint8_t *NextFilename = NULL;
    uint8_t *SegmentFamily = NULL;
    //
    uint64_t TimecodeScale = 0;
    double Duration = 0.0;
    uint64_t DateUTC = 0;
    uint8_t *Title = NULL;
    uint8_t *MuxingApp = NULL;
    uint8_t *WritingApp = NULL;

    write_ebml_element(element, mkv->xml);
    if (mkv->xml) fprintf(mkv->xml, "  <title>Info</title>\n");

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
            case eid_SegmentUID:
                SegmentUID = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* SegmentUID   = '%s'", SegmentUID);
                if (mkv->xml) fprintf(mkv->xml, "  <SegmentUID>%s</SegmentUID>\n", SegmentUID);
                break;
            case eid_SegmentFilename:
                SegmentFilename = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* Segment Filename   = '%s'", SegmentFilename);
                if (mkv->xml) fprintf(mkv->xml, "  <SegmentFilename>%s</SegmentFilename>\n", SegmentFilename);
                break;
            case eid_PrevUID:
                PrevUID = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* PrevUID   = '%s'", PrevUID);
                if (mkv->xml) fprintf(mkv->xml, "  <PrevUID>%s</PrevUID>\n", PrevUID);
                break;
            case eid_PrevFilename:
                PrevFilename = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* PrevFilename   = '%s'", PrevFilename);
                if (mkv->xml) fprintf(mkv->xml, "  <PrevFilename>%s</PrevFilename>\n", PrevFilename);
                break;
            case eid_NextUID:
                NextUID = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* NextUID   = '%s'", NextUID);
                if (mkv->xml) fprintf(mkv->xml, "  <NextUID>%s</NextUID>\n", NextUID);
                break;
            case eid_NextFilename:
                NextFilename = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* NextFilename   = '%s'", NextFilename);
                if (mkv->xml) fprintf(mkv->xml, "  <NextFilename>%s</NextFilename>\n", NextFilename);
                break;
            case eid_SegmentFamily:
                SegmentFamily = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* SegmentFamily   = '%s'", SegmentFamily);
                if (mkv->xml) fprintf(mkv->xml, "  <SegmentFamily>%s</SegmentFamily>\n", SegmentFamily);
                break;
            case eid_ChapterTranslate:
                retcode = mkv_parse_info_chapter(bitstr, &element_sub, mkv);
                break;
            case eid_TimecodeScale:
                TimecodeScale = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TimecodeScale   = '%llu'", TimecodeScale);
                if (mkv->xml) fprintf(mkv->xml, "  <TimecodeScale>%lu</TimecodeScale>\n", TimecodeScale);
                break;
            case eid_Duration:
                Duration = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* Duration   = '%f'", Duration);
                if (mkv->xml) fprintf(mkv->xml, "  <Duration>%f</Duration>\n", Duration);
                break;
            case eid_DateUTC:
                DateUTC = read_ebml_data_date(bitstr, element_sub.size);
                TRACE_1(MKV, "* DateUTC   = '%lli'", DateUTC);
                if (mkv->xml) fprintf(mkv->xml, "  <DateUTC>%li</DateUTC>\n", DateUTC);
                break;
            case eid_Title:
                Title = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* Title   = '%s'", Title);
                if (mkv->xml) fprintf(mkv->xml, "  <Title>%s</Title>\n", Title);
                break;
            case eid_MuxingApp:
                MuxingApp = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* MuxingApp   = '%s'", MuxingApp);
                if (mkv->xml) fprintf(mkv->xml, "  <MuxingApp>%s</MuxingApp>\n", MuxingApp);
                break;
            case eid_WritingApp:
                WritingApp = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "* WritingApp   = '%s'", WritingApp);
                if (mkv->xml) fprintf(mkv->xml, "  <WritingApp>%s</WritingApp>\n", WritingApp);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, element, mkv->xml);
                break;
            }
        }

        jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    free(SegmentUID);
    free(SegmentFilename);
    free(PrevUID);
    free(PrevFilename);
    free(NextUID);
    free(NextFilename);
    free(SegmentFamily);
    free(Title);
    free(MuxingApp);
    free(WritingApp);

    return retcode;
}

/* ************************************************************************** */
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
                retcode = mkv_parse_track(bitstr, &element_sub, mkv);
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
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        jumpy_mkv(bitstr, element, &element_sub);
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

        jumpy_mkv(bitstr, element, &element_sub);
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
        fprintf(mkv->xml, "  <EBMLVersion>%lu</EBMLVersion>\n", EBMLVersion);
        fprintf(mkv->xml, "  <EBMLReadVersion>%lu</EBMLReadVersion>\n", EBMLReadVersion);
        fprintf(mkv->xml, "  <EBMLMaxIDLength>%lu</EBMLMaxIDLength>\n", EBMLMaxIDLength);
        fprintf(mkv->xml, "  <EBMLMaxSizeLength>%lu</EBMLMaxSizeLength>\n", EBMLMaxSizeLength);
        fprintf(mkv->xml, "  <DocType>%s</DocType>\n", DocType);
        fprintf(mkv->xml, "  <DocTypeVersion>%lu</DocTypeVersion>\n", DocTypeVersion);
        fprintf(mkv->xml, "  <DocTypeReadVersion>%lu</DocTypeReadVersion>\n", DocTypeReadVersion);
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
