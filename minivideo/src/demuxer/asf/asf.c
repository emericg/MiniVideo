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

static int parse_contentdescriptor(Bitstream_t *bitstr, AsfContentDescriptor_t *cd, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_contentdescriptor()" CLR_RESET);
    int retcode = SUCCESS;

    if (cd)
    {
        cd->DescriptorNameLength = read_asf_int16(bitstr, asf->xml, "DescriptorNameLength");
        if (cd->DescriptorNameLength > 0)
            cd->DescriptorName = read_asf_string(bitstr, cd->DescriptorNameLength, asf->xml, "DescriptorName");

        cd->DescriptorValueDataType = read_asf_int16(bitstr, asf->xml, "DescriptorValueDataType");
        cd->DescriptorValueLength = read_asf_int16(bitstr, asf->xml, "DescriptorValueLength");

        if (cd->DescriptorValueLength > 0)
        {
            if (cd->DescriptorValueDataType == 0) // string
                cd->DescriptorValue = read_asf_string(bitstr, cd->DescriptorValueLength, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 1) // binary
                cd->DescriptorValue = read_asf_binary(bitstr, cd->DescriptorValueLength, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 2 || cd->DescriptorValueDataType == 3) // bool / int32
                cd->DescriptorValueInt = read_asf_int32(bitstr, 32, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 4) // int64
                cd->DescriptorValueInt = read_asf_int64(bitstr, 64, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 5) // int16
                cd->DescriptorValueInt = read_asf_int16(bitstr, asf->xml, "DescriptorValue");
        }
    }

    return retcode;
}

static int parse_extendedcontentdescription(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_extendedcontentdescription()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Extended Content Description");

    asf->asfh.ecd = malloc(sizeof(AsfExtendedContentDescriptionObject_t));
    if (asf->asfh.ecd)
    {
        asf->asfh.ecd->ContentDescriptorsCount = read_asf_int16(bitstr, asf->xml, "ContentDescriptorsCount");

        asf->asfh.ecd->ContentDescriptors = malloc(asf->asfh.ecd->ContentDescriptorsCount * sizeof(AsfContentDescriptor_t *));
        if (asf->asfh.ecd->ContentDescriptors)
        {
            for (int i = 0; i < asf->asfh.ecd->ContentDescriptorsCount; i++)
            {
                char SpaceTitle[32];
                sprintf(SpaceTitle, "Content Descriptor #%i", i);
                if (asf->xml) fprintf(asf->xml, "  <spacer>%s</spacer>\n",  SpaceTitle);

                asf->asfh.ecd->ContentDescriptors[i] = malloc(sizeof(AsfCodecEntry_t));
                parse_contentdescriptor(bitstr, asf->asfh.ecd->ContentDescriptors[i], asf);
            }
        }    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_contentdescription(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_contentdescription()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Content Description");

    asf->asfh.cd = malloc(sizeof(AsfContentDescriptionObject_t));
    if (asf->asfh.cd)
    {
        asf->asfh.cd->TitleLength = read_asf_int32(bitstr, 16, asf->xml, "TitleLength");
        asf->asfh.cd->AuthorLength = read_asf_int32(bitstr, 16, asf->xml, "AuthorLength");
        asf->asfh.cd->CopyrightLength = read_asf_int32(bitstr, 16, asf->xml, "CopyrightLength");
        asf->asfh.cd->DescriptionLength = read_asf_int32(bitstr, 16, asf->xml, "DescriptionLength");
        asf->asfh.cd->RatingLength = read_asf_int32(bitstr, 16, asf->xml, "RatingLength");

        if (asf->asfh.cd->TitleLength > 0)
            asf->asfh.cd->Title = read_asf_string(bitstr, asf->asfh.cd->TitleLength, asf->xml, "Title");
        if (asf->asfh.cd->AuthorLength > 0)
            asf->asfh.cd->Author = read_asf_string(bitstr, asf->asfh.cd->AuthorLength, asf->xml, "Author");
        if (asf->asfh.cd->CopyrightLength > 0)
            asf->asfh.cd->Copyright = read_asf_string(bitstr, asf->asfh.cd->CopyrightLength, asf->xml, "Copyright");
        if (asf->asfh.cd->DescriptionLength > 0)
            asf->asfh.cd->Description = read_asf_string(bitstr, asf->asfh.cd->DescriptionLength, asf->xml, "Description");
        if (asf->asfh.cd->RatingLength > 0)
            asf->asfh.cd->Rating = read_asf_string(bitstr, asf->asfh.cd->RatingLength, asf->xml, "Rating");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_codecentry(Bitstream_t *bitstr, asf_t *asf, AsfCodecEntry_t *c)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_codecentry()" CLR_RESET);
    int retcode = SUCCESS;

    if (c)
    {
        {
            c->Type = (int16_t)endian_flip_16(read_bits(bitstr, 16));

            char TypeString[16];
            if (c->Type == AsfCodecVideo)
                strcpy(TypeString, "Video");
            else if (c->Type == AsfCodecAudio)
                strcpy(TypeString, "Audio");
            else
                strcpy(TypeString, "Unknown");

            TRACE_1(ASF, "* %s  = %i (%s)", "Type", c->Type, TypeString);
            if (asf->xml) fprintf(asf->xml, "  <%s>%i (%s)</%s>\n", "Type", c->Type, TypeString, "Type");
        }

        c->CodecNameLength = read_asf_int16(bitstr, asf->xml, "CodecNameLength");
        if (c->CodecNameLength > 0)
            c->CodecName = read_asf_string(bitstr, c->CodecNameLength, asf->xml, "CodecName");

        c->CodecDescriptionLength = read_asf_int16(bitstr, asf->xml, "CodecDescriptionLength");
        if (c->CodecDescriptionLength > 0)
            c->CodecDescription = read_asf_string(bitstr, c->CodecDescriptionLength, asf->xml, "CodecDescription");

        c->CodecInformationLength = read_asf_int16(bitstr, asf->xml, "CodecInformationLength");
        if (c->CodecInformationLength > 0)
            c->CodecInformation = read_asf_binary(bitstr, c->CodecInformationLength, asf->xml, "CodecInformation");
    }

    return retcode;
}

static int parse_codeclist(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_codeclist()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Codec List");

    asf->asfh.cl = malloc(sizeof(AsfCodecListObject_t));
    if (asf->asfh.cl)
    {
        read_asf_guid(bitstr, asf->asfh.cl->Reserved, asf->xml, "Reserved");

        asf->asfh.cl->CodecEntriesCount = read_asf_int32(bitstr, 32, asf->xml, "CodecEntriesCount");

        asf->asfh.cl->CodecEntries = malloc(asf->asfh.cl->CodecEntriesCount * sizeof(AsfCodecEntry_t *));
        if (asf->asfh.cl->CodecEntries)
        {
            for (int i = 0; i < asf->asfh.cl->CodecEntriesCount; i++)
            {
                char SpaceTitle[32];
                sprintf(SpaceTitle, "Codec Entry #%i", i);
                if (asf->xml) fprintf(asf->xml, "  <spacer>%s</spacer>\n",  SpaceTitle);

                asf->asfh.cl->CodecEntries[i] = malloc(sizeof(AsfCodecEntry_t));
                parse_codecentry(bitstr, asf, asf->asfh.cl->CodecEntries[i]);
            }
        }
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_streamproperties(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf, int tid)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_streamproperties()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Stream Properties");

    read_asf_guid(bitstr, asf->asfh.sp[tid].StreamType, asf->xml, "StreamType");
    read_asf_guid(bitstr, asf->asfh.sp[tid].ErrorCorrectionType, asf->xml, "ErrorCorrectionType");

    asf->asfh.sp[tid].TimeOffset = read_asf_int64(bitstr, 64, asf->xml, "TimeOffset");
    asf->asfh.sp[tid].TypeSpecificDataLength = read_asf_int32(bitstr, 32, asf->xml, "TypeSpecificDataLength");
    asf->asfh.sp[tid].ErrorCorrectionDataLength = read_asf_int32(bitstr, 32, asf->xml, "ErrorCorrectionDataLength");

    asf->asfh.sp[tid].StreamNumber = read_asf_int32(bitstr, 7, asf->xml, "StreamNumber");
    asf->asfh.sp[tid].Reserved = read_asf_int32(bitstr, 8, asf->xml, "Reserved");
    asf->asfh.sp[tid].EncryptedContentFlag = read_asf_int32(bitstr, 1, asf->xml, "EncryptedContentFlag");

    asf->asfh.sp[tid].TypeSpecificData = read_asf_binary(bitstr, asf->asfh.sp[tid].TypeSpecificDataLength,
                                                         asf->xml, "TypeSpecificData");
    asf->asfh.sp[tid].ErrorCorrectionData = read_asf_binary(bitstr, asf->asfh.sp[tid].ErrorCorrectionDataLength,
                                                            asf->xml, "ErrorCorrectionData");

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_fileproperties(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_fileproperties()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "File Properties");

    read_asf_guid(bitstr, asf->asfh.fp.FileID, asf->xml, "FileID");

    asf->asfh.fp.FileSize = read_asf_int64(bitstr, 64, asf->xml, "FileSize");
    asf->asfh.fp.CreationDate = read_asf_int64(bitstr, 64, asf->xml, "CreationDate");
    asf->asfh.fp.DataPacketsCount = read_asf_int64(bitstr, 64, asf->xml, "DataPacketsCount");
    asf->asfh.fp.PlayDuration = read_asf_int64(bitstr, 64, asf->xml, "PlayDuration");
    asf->asfh.fp.SendDuration = read_asf_int64(bitstr, 64, asf->xml, "SendDuration");
    asf->asfh.fp.Preroll = read_asf_int64(bitstr, 64, asf->xml, "Preroll");

    asf->asfh.fp.BroadcastFlag = read_asf_int32(bitstr, 1, asf->xml, "BroadcastFlag");
    asf->asfh.fp.SeekableFlag = read_asf_int32(bitstr, 1, asf->xml, "SeekableFlag");
    asf->asfh.fp.Reserved = read_asf_int32(bitstr, 30, asf->xml, "Reserved");

    asf->asfh.fp.MinimumDataPacketSize = read_asf_int32(bitstr, 1, asf->xml, "MinimumDataPacketSize");
    asf->asfh.fp.MaximumDataPacketSize = read_asf_int32(bitstr, 1, asf->xml, "MaximumDataPacketSize");
    asf->asfh.fp.MaximumBitrate = read_asf_int32(bitstr, 1, asf->xml, "MaximumBitrate");

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_header_extension(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_header_extension()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(header);
    write_asf_object(header, asf->xml, "Header Extension Object");

    read_asf_guid(bitstr, asf->asfh.ex.ReservedField1, asf->xml, "ReservedField1");
    asf->asfh.ex.ReservedField2 = read_asf_int32(bitstr, 16, asf->xml, "ReservedField2");

    asf->asfh.ex.HeaderExtensionDataSize = read_asf_int32(bitstr, 32, asf->xml, "HeaderExtensionDataSize");
    asf->asfh.ex.HeaderExtensionData = read_asf_binary(bitstr, asf->asfh.ex.HeaderExtensionDataSize, asf->xml, "HeaderExtensionData");

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_header(Bitstream_t *bitstr, AsfObject_t *header, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_header()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(header);
    write_asf_object(header, asf->xml, "Header Object");

    asf->asfh.NumberOfHeaderObjects = read_asf_int32(bitstr, 32, asf->xml, "NumberOfHeaderObjects");
    asf->asfh.Reserved1 = read_asf_int32(bitstr, 8, asf->xml, "Reserved1");
    asf->asfh.Reserved2 = read_asf_int32(bitstr, 8, asf->xml, "Reserved2");

    while (asf->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (header->offset_end - 24))
    {
        // Read ASF object
        AsfObject_t asf_object;
        retcode = parse_asf_object(bitstr, &asf_object);

        if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_File_Properties_Object], 16) == 0)
        {
            retcode = parse_fileproperties(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Stream_Properties_Object], 16) == 0)
        {
            retcode = parse_streamproperties(bitstr, &asf_object, asf, asf->tracks_count);
            asf->tracks_count++;
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Header_Extension_Object], 16) == 0)
        {
            retcode = parse_header_extension(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Codec_List_Object], 16) == 0)
        {
            retcode = parse_codeclist(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Content_Description_Object], 16) == 0)
        {
            retcode = parse_contentdescription(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Extended_Content_Description_Object], 16) == 0)
        {
            retcode = parse_extendedcontentdescription(bitstr, &asf_object, asf);
        }//
        //ASF_Content_Branding_Object
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
                retcode = parse_header(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Data_Object], 16) == 0)
            {
                retcode = parse_data(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Simple_Index_Object], 16) == 0)
            {
                retcode = parse_simple_index(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Index_Object], 16) == 0)
            {
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
