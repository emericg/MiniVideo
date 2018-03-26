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
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */
/* ************************************************************************** */

//static int parse_scriptcommand(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf);

/* ************************************************************************** */

//static int parse_marker(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf);

/* ************************************************************************** */

static int parse_bitratemutualexclusion(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_bitratemutualexclusion()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Bitrate Mutual Exclusion");

    asf->asfh.bme = (AsfBitrateMutualExclusionObject_t*)calloc(1, sizeof(AsfBitrateMutualExclusionObject_t));
    if (asf->asfh.bme)
    {
        read_asf_guid(bitstr, asf->asfh.bme->ExclusionType, asf->xml, "ExclusionType");
        asf->asfh.bme->StreamNumbersCount = read_asf_int16(bitstr, asf->xml, "StreamNumbersCount");
        for (int i = 0; i < asf->asfh.bme->StreamNumbersCount; i++)
            asf->asfh.bme->StreamNumbers[i] = read_asf_int16(bitstr, asf->xml, "StreamNumbers");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_errorcorrection(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_errorcorrection()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Error Correction");

    asf->asfh.ec = (AsfErrorCorrectionObject_t*)calloc(1, sizeof(AsfErrorCorrectionObject_t));
    if (asf->asfh.ec)
    {
        read_asf_guid(bitstr, asf->asfh.ec->ErrorCorrectionType, asf->xml, "ErrorCorrectionType");
        asf->asfh.ec->ErrorCorrectionDataLength = read_asf_int32(bitstr, asf->xml, "ErrorCorrectionDataLength");
        asf->asfh.ec->ErrorCorrectionData = read_asf_binary(bitstr, asf->asfh.ec->ErrorCorrectionDataLength, asf->xml, "ErrorCorrectionData");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}
/* ************************************************************************** */

static int parse_padding(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_padding()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Padding");

    asf->asfh.pad = (AsfPaddingObject_t*)calloc(1, sizeof(AsfPaddingObject_t));
    if (asf->asfh.pad)
    {
        asf->asfh.pad->PaddingDataLength = obj->size - 24; // 24 = sizeof(obj->guid) + sizeof(obj->size)
        asf->asfh.pad->PaddingData = read_asf_binary(bitstr, asf->asfh.pad->PaddingDataLength, asf->xml, "PaddingData");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_digitalsignature(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_digitalsignature()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Digital Signature");

    asf->asfh.ds = (AsfDigitalSignatureObject_t*)calloc(1, sizeof(AsfDigitalSignatureObject_t));
    if (asf->asfh.ds)
    {
        asf->asfh.ds->SignatureType = read_asf_int32(bitstr, asf->xml, "SignatureType");
        asf->asfh.ds->SignatureDataLength = read_asf_int32(bitstr, asf->xml, "SignatureDataLength");
        asf->asfh.ds->SignatureData = read_asf_binary(bitstr, asf->asfh.ds->SignatureDataLength, asf->xml, "SignatureData");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_extendedcontentencryption(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_extendedcontentencryption()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Extended Content Encryption");

    asf->asfh.ece = (AsfExtendedContentEncryptionObject_t*)calloc(1, sizeof(AsfExtendedContentEncryptionObject_t));
    if (asf->asfh.ece)
    {
        asf->asfh.ece->DataSize = read_asf_int32(bitstr, asf->xml, "DataSize");
        asf->asfh.ece->Data = read_asf_binary(bitstr, asf->asfh.ece->DataSize, asf->xml, "Data");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_contentencryption(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_contentencryption()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Content Encryption");

    asf->asfh.ce = (AsfContentEncryptionObject_t*)calloc(1, sizeof(AsfContentEncryptionObject_t));
    if (asf->asfh.ce)
    {
        asf->asfh.ce->SecretDataLength = read_asf_int32(bitstr, asf->xml, "SecretDataLength");
        asf->asfh.ce->SecretData = read_asf_binary(bitstr, asf->asfh.ce->SecretDataLength, asf->xml, "SecretData");
        asf->asfh.ce->ProtectionTypeLength = read_asf_int32(bitstr, asf->xml, "ProtectionTypeLength");
        asf->asfh.ce->ProtectionType = read_asf_string_utf16(bitstr, (asf->asfh.ce->ProtectionTypeLength/2), asf->xml, "ProtectionType");
        asf->asfh.ce->KeyIDLength = read_asf_int32(bitstr, asf->xml, "KeyIDLength");
        asf->asfh.ce->KeyID = read_asf_string_ascii(bitstr, asf->asfh.ce->KeyIDLength, asf->xml, "KeyID");
        asf->asfh.ce->LicenseURLLength = read_asf_int32(bitstr, asf->xml, "LicenseURLLength");
        asf->asfh.ce->LicenseURL = read_asf_string_utf16(bitstr, (asf->asfh.ce->LicenseURLLength/2), asf->xml, "LicenseURL");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_contentbranding(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_contentbranding()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Content Branding");

    asf->asfh.cb = (AsfContentBrandingObject_t*)calloc(1, sizeof(AsfContentBrandingObject_t));
    if (asf->asfh.cb)
    {
        asf->asfh.cb->BannerImageType = read_asf_int32(bitstr, asf->xml, "BannerImageType");
        asf->asfh.cb->BannerImageDataSize = read_asf_int32(bitstr, asf->xml, "BannerImageDataSize");
        asf->asfh.cb->BannerImageData = read_asf_binary(bitstr, asf->asfh.cb->BannerImageDataSize, asf->xml, "BannerImageData");
        asf->asfh.cb->BannerImageURLLength = read_asf_int32(bitstr, asf->xml, "BannerImageURLLength");
        asf->asfh.cb->BannerImageURL = read_asf_string_utf16(bitstr, (asf->asfh.cb->BannerImageURLLength / 2), asf->xml, "BannerImageURL");
        asf->asfh.cb->CopyrightURLLength = read_asf_int32(bitstr, asf->xml, "CopyrightURLLength");
        asf->asfh.cb->CopyrightURL = read_asf_string_utf16(bitstr, (asf->asfh.cb->CopyrightURLLength / 2), asf->xml, "CopyrightURL");
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_bitraterecord(Bitstream_t *bitstr, AsfBitrateRecord_t *br, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_bitraterecord()" CLR_RESET);
    int retcode = SUCCESS;

    if (br)
    {
        br->StreamNumber = read_asf_int(bitstr, 7, asf->xml, "StreamNumber");
        br->Reserved = read_asf_int(bitstr, 9, asf->xml, "Reserved");
        br->AverageBitrate = read_asf_int32(bitstr, asf->xml, "AverageBitrate");
    }

    return retcode;
}

static int parse_streambitrateproperties(Bitstream_t *bitstr, AsfObject_t *obj, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_streambitrateproperties()" CLR_RESET);
    int retcode = SUCCESS;

    print_asf_object(obj);
    write_asf_object(obj, asf->xml, "Stream Bitrate Properties");

    asf->asfh.sbp = (AsfStreamBitratePropertiesObject_t*)calloc(1, sizeof(AsfStreamBitratePropertiesObject_t));
    if (asf->asfh.sbp)
    {
        asf->asfh.sbp->BitrateRecordsCount = read_asf_int16(bitstr, asf->xml, "BitrateRecordsCount");
        if (asf->asfh.sbp->BitrateRecordsCount > 0)
        {
            asf->asfh.sbp->BitrateRecords = (AsfBitrateRecord_t**)malloc(asf->asfh.sbp->BitrateRecordsCount * sizeof(AsfBitrateRecord_t *));
            if (asf->asfh.sbp->BitrateRecords)
            {
                for (int i = 0; i < asf->asfh.sbp->BitrateRecordsCount; i++)
                {
                    xmlSpacer(asf->xml, "Bitrate Record", i);

                    asf->asfh.sbp->BitrateRecords[i] = (AsfBitrateRecord_t*)calloc(1, sizeof(AsfBitrateRecord_t));
                    parse_bitraterecord(bitstr, asf->asfh.sbp->BitrateRecords[i], asf);
                }
            }
        }
    }

    fprintf(asf->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int parse_contentdescriptor(Bitstream_t *bitstr, AsfContentDescriptor_t *cd, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "parse_contentdescriptor()" CLR_RESET);
    int retcode = SUCCESS;

    if (cd)
    {
        cd->DescriptorNameLength = read_asf_int16(bitstr, asf->xml, "DescriptorNameLength");
        cd->DescriptorName = read_asf_string_utf16(bitstr, (cd->DescriptorNameLength / 2), asf->xml, "DescriptorName");

        cd->DescriptorValueDataType = read_asf_int16(bitstr, asf->xml, "DescriptorValueDataType");
        cd->DescriptorValueLength = read_asf_int16(bitstr, asf->xml, "DescriptorValueLength");
        if (cd->DescriptorValueLength > 0)
        {
            if (cd->DescriptorValueDataType == 0) // string
                cd->DescriptorValue_data = (uint8_t *)read_asf_string_utf16(bitstr, (cd->DescriptorValueLength / 2), asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 1) // binary
                cd->DescriptorValue_data = read_asf_binary(bitstr, (cd->DescriptorValueLength / 2), asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 2 || cd->DescriptorValueDataType == 3) // bool / int32
                cd->DescriptorValue_numerical = read_asf_int32(bitstr, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 4) // int64
                cd->DescriptorValue_numerical = read_asf_int64(bitstr, asf->xml, "DescriptorValue");
            else if (cd->DescriptorValueDataType == 5) // int16
                cd->DescriptorValue_numerical = read_asf_int16(bitstr, asf->xml, "DescriptorValue");
            else
                TRACE_ERROR(ASF, "CONTENT DESCRIPTOR TYPE ERROR (%02X)", cd->DescriptorValueDataType);
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

    asf->asfh.ecd = (AsfExtendedContentDescriptionObject_t*)calloc(1, sizeof(AsfExtendedContentDescriptionObject_t));
    if (asf->asfh.ecd)
    {
        asf->asfh.ecd->ContentDescriptorsCount = read_asf_int16(bitstr, asf->xml, "ContentDescriptorsCount");
        if (asf->asfh.ecd->ContentDescriptorsCount > 0)
        {
            asf->asfh.ecd->ContentDescriptors = (AsfContentDescriptor_t**)malloc(asf->asfh.ecd->ContentDescriptorsCount * sizeof(AsfContentDescriptor_t *));
            if (asf->asfh.ecd->ContentDescriptors)
            {
                for (int i = 0; i < asf->asfh.ecd->ContentDescriptorsCount; i++)
                {
                    xmlSpacer(asf->xml, "Content Descriptor", i);

                    asf->asfh.ecd->ContentDescriptors[i] = (AsfContentDescriptor_t*)calloc(1, sizeof(AsfContentDescriptor_t));
                    parse_contentdescriptor(bitstr, asf->asfh.ecd->ContentDescriptors[i], asf);
                }
            }
        }
    }

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

    asf->asfh.cd = (AsfContentDescriptionObject_t*)calloc(1, sizeof(AsfContentDescriptionObject_t));
    if (asf->asfh.cd)
    {
        asf->asfh.cd->TitleLength = read_asf_int16(bitstr, asf->xml, "TitleLength");
        asf->asfh.cd->AuthorLength = read_asf_int16(bitstr, asf->xml, "AuthorLength");
        asf->asfh.cd->CopyrightLength = read_asf_int16(bitstr, asf->xml, "CopyrightLength");
        asf->asfh.cd->DescriptionLength = read_asf_int16(bitstr, asf->xml, "DescriptionLength");
        asf->asfh.cd->RatingLength = read_asf_int16(bitstr, asf->xml, "RatingLength");

        asf->asfh.cd->Title = read_asf_string_utf16(bitstr, (asf->asfh.cd->TitleLength / 2), asf->xml, "Title");
        asf->asfh.cd->Author = read_asf_string_utf16(bitstr, (asf->asfh.cd->AuthorLength / 2), asf->xml, "Author");
        asf->asfh.cd->Copyright = read_asf_string_utf16(bitstr, (asf->asfh.cd->CopyrightLength / 2), asf->xml, "Copyright");
        asf->asfh.cd->Description = read_asf_string_utf16(bitstr, (asf->asfh.cd->DescriptionLength / 2), asf->xml, "Description");
        asf->asfh.cd->Rating = read_asf_string_utf16(bitstr, (asf->asfh.cd->RatingLength / 2), asf->xml, "Rating");
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
        c->CodecName = read_asf_string_utf16(bitstr, c->CodecNameLength, asf->xml, "CodecName");

        c->CodecDescriptionLength = read_asf_int16(bitstr, asf->xml, "CodecDescriptionLength");
        c->CodecDescription = read_asf_string_utf16(bitstr, c->CodecDescriptionLength, asf->xml, "CodecDescription");

        c->CodecInformationLength = read_asf_int16(bitstr, asf->xml, "CodecInformationLength");
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

    asf->asfh.cl = (AsfCodecListObject_t*)calloc(1, sizeof(AsfCodecListObject_t));
    if (asf->asfh.cl)
    {
        read_asf_guid(bitstr, asf->asfh.cl->Reserved, asf->xml, "Reserved");

        asf->asfh.cl->CodecEntriesCount = read_asf_int32(bitstr, asf->xml, "CodecEntriesCount");
        if (asf->asfh.cl->CodecEntriesCount > 0)
        {
            asf->asfh.cl->CodecEntries = (AsfCodecEntry_t**)malloc(asf->asfh.cl->CodecEntriesCount * sizeof(AsfCodecEntry_t *));
            if (asf->asfh.cl->CodecEntries)
            {
                for (int i = 0; i < asf->asfh.cl->CodecEntriesCount; i++)
                {
                    xmlSpacer(asf->xml, "Codec Entry", i);

                    asf->asfh.cl->CodecEntries[i] = (AsfCodecEntry_t*)calloc(1, sizeof(AsfCodecEntry_t));
                    parse_codecentry(bitstr, asf, asf->asfh.cl->CodecEntries[i]);
                }
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
    write_asf_object(obj, asf->xml, "Stream Properties", "track");

    asf->tracks_count++;

    read_asf_guid(bitstr, asf->asfh.sp[tid].StreamType, asf->xml, "StreamType");
    read_asf_guid(bitstr, asf->asfh.sp[tid].ErrorCorrectionType, asf->xml, "ErrorCorrectionType");

    asf->asfh.sp[tid].TimeOffset = read_asf_int64(bitstr, asf->xml, "TimeOffset");
    asf->asfh.sp[tid].TypeSpecificDataLength = read_asf_int32(bitstr, asf->xml, "TypeSpecificDataLength");
    asf->asfh.sp[tid].ErrorCorrectionDataLength = read_asf_int32(bitstr, asf->xml, "ErrorCorrectionDataLength");

    asf->asfh.sp[tid].StreamNumber = read_asf_int(bitstr, 7, asf->xml, "StreamNumber");
    asf->asfh.sp[tid].Reserved = read_asf_int(bitstr, 8, asf->xml, "Reserved");
    asf->asfh.sp[tid].EncryptedContentFlag = read_asf_int(bitstr, 1, asf->xml, "EncryptedContentFlag");

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

    asf->asfh.fp.FileSize = read_asf_int64(bitstr, asf->xml, "FileSize");
    asf->asfh.fp.CreationDate = read_asf_int64(bitstr, asf->xml, "CreationDate");
    asf->asfh.fp.DataPacketsCount = read_asf_int64(bitstr, asf->xml, "DataPacketsCount");
    asf->asfh.fp.PlayDuration = read_asf_int64(bitstr, asf->xml, "PlayDuration");
    asf->asfh.fp.SendDuration = read_asf_int64(bitstr, asf->xml, "SendDuration");
    asf->asfh.fp.Preroll = read_asf_int64(bitstr, asf->xml, "Preroll");

    asf->asfh.fp.BroadcastFlag = read_asf_int(bitstr, 1, asf->xml, "BroadcastFlag");
    asf->asfh.fp.SeekableFlag = read_asf_int(bitstr, 1, asf->xml, "SeekableFlag");
    asf->asfh.fp.Reserved = read_asf_int(bitstr, 30, asf->xml, "Reserved");

    asf->asfh.fp.MinimumDataPacketSize = read_asf_int(bitstr, 1, asf->xml, "MinimumDataPacketSize");
    asf->asfh.fp.MaximumDataPacketSize = read_asf_int(bitstr, 1, asf->xml, "MaximumDataPacketSize");
    asf->asfh.fp.MaximumBitrate = read_asf_int(bitstr, 1, asf->xml, "MaximumBitrate");

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
    asf->asfh.ex.ReservedField2 = read_asf_int16(bitstr, asf->xml, "ReservedField2");

    asf->asfh.ex.HeaderExtensionDataSize = read_asf_int32(bitstr, asf->xml, "HeaderExtensionDataSize");
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

    asf->asfh.NumberOfHeaderObjects = read_asf_int32(bitstr, asf->xml, "NumberOfHeaderObjects");
    asf->asfh.Reserved1 = read_asf_int8(bitstr, asf->xml, "Reserved1");
    asf->asfh.Reserved2 = read_asf_int8(bitstr, asf->xml, "Reserved2");

    while (asf->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (header->offset_end - 24))
    {
        // Read ASF object
        AsfObject_t asf_object;
        retcode = parse_asf_object(bitstr, &asf_object);

        if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_File_Properties_Object], 16) == 0)
        {
            retcode = parse_fileproperties(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Stream_Properties_Object], 16) == 0)
        {
            retcode = parse_streamproperties(bitstr, &asf_object, asf, asf->tracks_count);
            asf->tracks_count++;
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Header_Extension_Object], 16) == 0)
        {
            retcode = parse_header_extension(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Codec_List_Object], 16) == 0)
        {
            retcode = parse_codeclist(bitstr, &asf_object, asf);
        }
/*
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Script_Command_Object], 16) == 0)
        {
            retcode = parse_scriptcommand(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDS[ASF_Marker_Object], 16) == 0)
        {
            retcode = parse_marker(bitstr, &asf_object, asf);
        }
*/
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Bitrate_Mutual_Exclusion_Object], 16) == 0)
        {
            retcode = parse_bitratemutualexclusion(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Error_Correction_Object], 16) == 0)
        {
            retcode = parse_errorcorrection(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Content_Description_Object], 16) == 0)
        {
            retcode = parse_contentdescription(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Extended_Content_Description_Object], 16) == 0)
        {
            retcode = parse_extendedcontentdescription(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Content_Branding_Object], 16) == 0)
        {
            retcode = parse_contentbranding(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Stream_Bitrate_Properties_Object], 16) == 0)
        {
            retcode = parse_streambitrateproperties(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Content_Encryption_Object], 16) == 0)
        {
            retcode = parse_contentencryption(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Extended_Content_Encryption_Object], 16) == 0)
        {
            retcode = parse_extendedcontentencryption(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Digital_Signature_Object], 16) == 0)
        {
            retcode = parse_digitalsignature(bitstr, &asf_object, asf);
        }
        else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Padding_Object], 16) == 0)
        {
            retcode = parse_padding(bitstr, &asf_object, asf);
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
        memset(&asf, 0, sizeof(asf_t));

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

            if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Header_Object], 16) == 0)
            {
                retcode = parse_header(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Data_Object], 16) == 0)
            {
                retcode = parse_data(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Simple_Index_Object], 16) == 0)
            {
                retcode = parse_simple_index(bitstr, &asf_object, &asf);
            }
            else if (memcmp(asf_object.guid, ASF_object_GUIDs[ASF_Index_Object], 16) == 0)
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
        retcode = asf_convert(bitstr, media, &asf),

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
