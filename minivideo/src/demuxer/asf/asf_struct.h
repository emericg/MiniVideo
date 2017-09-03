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
 * \file      asf_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_ASF_STRUCT_H
#define PARSER_ASF_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include "../../minivideo_codecs.h"
#include <stdio.h>

/* ************************************************************************** */

//! ASF object structure
typedef struct AsfObject_t
{
    int64_t offset_start;   //!< Absolute position of the first byte of this box
    int64_t offset_end;     //!< Absolute position of the last byte of this box

    // Object parameters
    uint8_t guid[16];
    int64_t size;          //!< Box size in bytes, including all its fields and contained boxes

} AsfObject_t;

/* ************************************************************************** */
/* ************************************************************************** */

static const uint8_t ASF_object_GUIDS[64][16] =
{
    // 10.1 Top-level ASF object GUIDS
                           //
    {0x75, 0xB2, 0x26, 0x30, 0x66, 0x8E, 0x11, 0xCF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C},
    {0x75, 0xB2, 0x26, 0x36, 0x66, 0x8E, 0x11, 0xCF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C},
    {0x33, 0x00, 0x08, 0x90, 0xE5, 0xB1, 0x11, 0xCF, 0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB},
    {0xD6, 0xE2, 0x29, 0xD3, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},
    {0xFE, 0xB1, 0x03, 0xF8, 0x12, 0xAD, 0x4C, 0x64, 0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C},
    {0x3C, 0xB7, 0x3F, 0xD0, 0x0C, 0x4A, 0x48, 0x03, 0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C},

    // 10.2 Header Object GUIDs
    {0x8C, 0xAB, 0xDC, 0xA1, 0xA9, 0x47, 0x11, 0xCF, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65},
    {0xB7, 0xDC, 0x07, 0x91, 0xA9, 0xB7, 0x11, 0xCF, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65},
    {0x5F, 0xBF, 0x03, 0xB5, 0xA9, 0x2E, 0x11, 0xCF, 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65},

    {0x86, 0xD1, 0x52, 0x40, 0x31, 0x1D, 0x11, 0xD0, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6},
    {0x1E, 0xFB, 0x1A, 0x30, 0x0B, 0x62, 0x11, 0xD0, 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6},
    {0xF4, 0x87, 0xCD, 0x01, 0xA9, 0x51, 0x11, 0xCF, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65},
    {0xD6, 0xE2, 0x29, 0xDC, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},
    {0x75, 0xB2, 0x26, 0x35, 0x66, 0x8E, 0x11, 0xCF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C},

    {0x75, 0xB2, 0x26, 0x33, 0x66, 0x8E, 0x11, 0xCF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C},
    {0xD2, 0xD0, 0xA4, 0x40, 0xE3, 0x07, 0x11, 0xD2, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50},
    {0x22, 0x11, 0xB3, 0xFA, 0xBD, 0x23, 0x11, 0xD2, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E},
    {0x7B, 0xF8, 0x75, 0xCE, 0x46, 0x8D, 0x11, 0xD1, 0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2},
    {0x22, 0x11, 0xB3, 0xFB, 0xBD, 0x23, 0x11, 0xD2, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E},
    {0x29, 0x8A, 0xE6, 0x14, 0x26, 0x22, 0x4C, 0x17, 0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C},
    {0x22, 0x11, 0xB3, 0xFC, 0xBD, 0x23, 0x11, 0xD2, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E},
    {0x18, 0x06, 0xD4, 0x74, 0xCA, 0xDF, 0x45, 0x09, 0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8},

};

typedef enum ASF_object_Names_e
{
    // 10.1 Top-level ASF object GUIDS
    ASF_Header_Object,
    ASF_Data_Object,
    ASF_Simple_Index_Object,
    ASF_Index_Object,
    ASF_Media_Object_Index_Object,
    ASF_Timecode_Index_Object,

    // 10.2 Header Object GUIDs
    ASF_File_Properties_Object,
    ASF_Stream_Properties_Object,
    ASF_Header_Extension_Object,

    ASF_Codec_List_Object,
    ASF_Script_Command_Object,
    ASF_Marker_Object,
    ASF_Bitrate_Mutual_Exclusion_Object,
    ASF_Error_Correction_Object,

    ASF_Content_Description_Object,
    ASF_Extended_Content_Description_Object,
    ASF_Content_Branding_Object,
    ASF_Stream_Bitrate_Properties_Object,
    ASF_Content_Encryption_Object,
    ASF_Extended_Content_Encryption_Object,
    ASF_Digital_Signature_Object,
    ASF_Padding_Object,

} ASF_object_GUIDS_e;

/* ************************************************************************** */
/* ************************************************************************** */

typedef struct AsfFilePropertiesObject_t
{
    uint8_t FileID[16];

    int64_t FileSize;
    int64_t CreationDate;
    int64_t DataPacketsCount;
    int64_t PlayDuration;
    int64_t SendDuration;
    int64_t Preroll;

    bool BroadcastFlag;
    bool SeekableFlag;
    int32_t Reserved;

    int32_t MinimumDataPacketSize;
    int32_t MaximumDataPacketSize;
    int32_t MaximumBitrate;

} AsfFilePropertiesObject_t;

typedef struct AsfStreamPropertiesObject_t
{
    uint8_t StreamType[16];
    uint8_t ErrorCorrectionType[16];

    int64_t TimeOffset;

    int32_t TypeSpecificDataLength;
    int32_t ErrorCorrectionDataLength;

    uint8_t StreamNumber;
    uint8_t Reserved;
    bool EncryptedContentFlag;

    int32_t Reserved2;

    uint8_t *TypeSpecificData;
    uint8_t *ErrorCorrectionData;

} AsfStreamPropertiesObject_t;

typedef struct AsfBitrateRecord_t
{
    int32_t StreamNumber;
    int32_t Reserved;
    int32_t AverageBitrate;

} AsfBitrateRecord_t;

typedef struct AsfStreamBitratePropertiesObject_t
{
    int16_t BitrateRecordsCount;
    AsfBitrateRecord_t **BitrateRecords;

} AsfStreamBitratePropertiesObject_t;

typedef struct AsfHeaderExtensionObject_t
{
    uint8_t ReservedField1[16];
    int16_t ReservedField2;
    int32_t HeaderExtensionDataSize;
    uint8_t *HeaderExtensionData;

    //AsfExtendedStreamPropertiesObject_t
    //AsfAdvancedMutualExclusionObject_t
    //AsfGroupMutualExclusionObject_t
    //AsfStreamPrioritizationObject_t
    //AsfBandwidthSharingObject_t
    //AsfLanguageListObject_t
    //AsfMetadataObject_t
    //AsfMetadataLibraryObject_t
    //AsfIndexParametersObject_t
    //AsfMediaObjectIndexParametersObject_t
    //AsfTimecodeIndexParametersObject_t
    //AsfCompatibilityObject_t
    //AsfAdvancedContentEncryptionObject_t

} AsfHeaderExtensionObject_t;

typedef enum AsfCodecType
{
    AsfCodecVideo   = 0x0001,
    AsfCodecAudio   = 0x0002,
    AsfCodecUnknown = 0xFFFF

} AsfCodecType;

typedef struct AsfCodecEntry_t
{
    int16_t Type;
    int16_t CodecNameLength;
    uint8_t *CodecName;
    int16_t CodecDescriptionLength;
    uint8_t *CodecDescription;
    int16_t CodecInformationLength;
    uint8_t *CodecInformation;

} AsfCodecEntry_t;

typedef struct AsfCodecListObject_t
{
    uint8_t Reserved[16];
    int32_t CodecEntriesCount;
    AsfCodecEntry_t **CodecEntries;

} AsfCodecListObject_t;

typedef struct AsfCommandTypes_t
{
    int16_t CommandTypeNameLength;
    uint8_t *CommandTypeName;

} AsfCommandTypes_t;

typedef struct AsfCommands_t
{
    int32_t PresentationTime;
    int16_t TypeIndex;
    int16_t CommandNameLength;
    uint8_t *CommandName;

} AsfCommands_t;

typedef struct AsfScriptCommandObject_t
{
    uint8_t Reserved[16];
    int16_t CommandsCount;
    int16_t CommandTypesCount;
    AsfCommandTypes_t *CommandTypes;
    AsfCommands_t *Commands;

} AsfScriptCommandObject_t;

typedef struct AsfContentDescriptor_t
{
    int16_t DescriptorNameLength;
    char *DescriptorName;
    int16_t DescriptorValueDataType;
    int16_t DescriptorValueLength;

    uint8_t *DescriptorValue;
    uint64_t DescriptorValueInt;

} AsfContentDescriptor_t;

typedef struct AsfExtendedContentDescriptionObject_t
{
    int16_t ContentDescriptorsCount;
    AsfContentDescriptor_t **ContentDescriptors;

} AsfExtendedContentDescriptionObject_t;

typedef struct AsfContentDescriptionObject_t
{
    int16_t TitleLength;
    int16_t AuthorLength;
    int16_t CopyrightLength;
    int16_t DescriptionLength;
    int16_t RatingLength;

    uint8_t *Title;
    uint8_t *Author;
    uint8_t *Copyright;
    uint8_t *Varies;
    uint8_t *Description;
    uint8_t *Rating;

} AsfContentDescriptionObject_t;

typedef struct AsfMarkers
{
    int64_t Offset;
    int64_t PresentationTime;
    int16_t EntryLength;
    int32_t SendTime;
    int32_t Flags;
    int32_t MarkerDescriptionLength;
    uint8_t *MarkerDescription;

} AsfMarkers;

typedef struct AsfMarkerObject_t
{
    uint8_t Reserved[16];
    int32_t MarkersCount;
    int16_t Reserved2;
    int16_t NameLength;
    uint8_t *Name;
    AsfMarkers *Markers;

} AsfMarkerObject_t;

typedef struct AsfBitrateMutualExclusionObject_t
{
    uint8_t ExclusionType[16];
    int16_t StreamNumbersCount;
    int32_t *StreamNumbers;

} AsfBitrateMutualExclusionObject_t;

typedef struct AsfErrorCorrectionObject_t
{
    uint8_t ErrorCorrectionType[16];
    int32_t ErrorCorrectionDataLength;
    uint8_t *ErrorCorrectionData;

} AsfErrorCorrectionObject_t;

typedef struct AsfHeaderObject_t
{
    int32_t NumberOfHeaderObjects;
    uint8_t Reserved1;
    uint8_t Reserved2;

    AsfFilePropertiesObject_t fp;       //!< Contains global file attributes
    AsfStreamPropertiesObject_t sp[16]; //!< Defines a digital media stream and its characteristics
    AsfHeaderExtensionObject_t ex;      //!< Allows additional functionality to be added to an ASF file while maintaining backward compatibility

    AsfCodecListObject_t *cl;
    AsfScriptCommandObject_t *sc;        //!< Contains commands that can be executed on the playback timeline
    AsfMarkerObject_t *md;               //!< Provides named jump points within a file
    AsfBitrateMutualExclusionObject_t *bme;
    AsfErrorCorrectionObject_t *eco;
    AsfContentDescriptionObject_t *cd;   //!< Contains bibliographic information
    AsfExtendedContentDescriptionObject_t *ecd;
    AsfStreamBitratePropertiesObject_t *sbp;
    //AsfContentBrandingObject_t *cb;
    //AsfContentEncryptionObject_t *ce;
    //AsfExtendedContentEncryptionObject_t *ece;
    //AsfDigitalSignatureObject_t *ds;
    //AsfPaddingObject_t *pad;

} AsfHeaderObject_t;

/* ************************************************************************** */

/*!
 * 5.2 ASF data packet definition
 */
typedef struct AsfDataPacket_t
{
    // 5.2.1 Error correction data
    uint8_t ErrorCorrectionDataLength;
    uint8_t ErrorCorrectionLengthType;
    uint8_t OpaqueDataPresent;
    uint8_t ErrorCorrectionPresent;
    uint8_t *ErrorCorrectionData;

    // 5.2.2 Payload parsing information

    // 5.2.3 Payload data
    // 5.2.3.1 Single payload
    // 5.2.3.2 Single payload, compressed payload data
    // 5.2.3.3 Multiple payloads

} AsfDataPacket_t;

typedef struct AsfDataObject_t
{
    AsfDataPacket_t fp;

} AsfDataObject_t;

/* ************************************************************************** */

//! Structure for ASF video tracks
typedef struct AsfTrack_t
{
    unsigned int track_id;
    unsigned int track_indexed;

} AsfTrack_t;

//! Structure for ASF video infos
typedef struct asf_t
{
    bool run;                   //!< A convenient way to stop the parser from any sublevel

    ContainerProfiles_e profile;

    AsfHeaderObject_t asfh;
    AsfDataObject_t asfd;

    unsigned int tracks_count;
    AsfTrack_t *tracks[16];

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} asf_t;

/* ************************************************************************** */
#endif // PARSER_ASF_STRUCT_H
