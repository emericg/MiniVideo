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
 * \file      asf_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_ASF_STRUCT_H
#define PARSER_ASF_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include "../../minivideo_codecs.h"
#include "../../minivideo_containers.h"

#include <cstdio>

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

static const uint8_t ASF_object_GUIDs[64][16] =
{
    // 10.1 Top-level ASF object GUIDs
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

    // 10.3 Header Extension Object GUIDs
    {0x14, 0xE6, 0xA5, 0xCB, 0xC6, 0x72, 0x43, 0x32, 0x83, 0x99, 0xA9, 0x69, 0x52, 0x06, 0x5B, 0x5A},
    {0xA0, 0x86, 0x49, 0xCF, 0x47, 0x75, 0x46, 0x70, 0x8A, 0x16, 0x6E, 0x35, 0x35, 0x75, 0x66, 0xCD},
    {0xD1, 0x46, 0x5A, 0x40, 0x5A, 0x79, 0x43, 0x38, 0xB7, 0x1B, 0xE3, 0x6B, 0x8F, 0xD6, 0xC2, 0x49},
    {0xD4, 0xFE, 0xD1, 0x5B, 0x88, 0xD3, 0x45, 0x4F, 0x81, 0xF0, 0xED, 0x5C, 0x45, 0x99, 0x9E, 0x24},
    {0xA6, 0x96, 0x09, 0xE6, 0x51, 0x7B, 0x11, 0xD2, 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9},
    {0x7C, 0x43, 0x46, 0xA9, 0xEF, 0xE0, 0x4B, 0xFC, 0xB2, 0x29, 0x39, 0x3E, 0xDE, 0x41, 0x5C, 0x85},
    {0xC5, 0xF8, 0xCB, 0xEA, 0x5B, 0xAF, 0x48, 0x77, 0x84, 0x67, 0xAA, 0x8C, 0x44, 0xFA, 0x4C, 0xCA},
    {0x44, 0x23, 0x1C, 0x94, 0x94, 0x98, 0x49, 0xD1, 0xA1, 0x41, 0x1D, 0x13, 0x4E, 0x45, 0x70, 0x54},
    {0xD6, 0xE2, 0x29, 0xDF, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},
    {0x6B, 0x20, 0x3B, 0xAD, 0x3F, 0x11, 0x48, 0xE4, 0xAC, 0xA8, 0xD7, 0x61, 0x3D, 0xE2, 0xCF, 0xA7},
    {0xF5, 0x5E, 0x49, 0x6D, 0x97, 0x97, 0x4B, 0x5D, 0x8C, 0x8B, 0x60, 0x4D, 0xFE, 0x9B, 0xFB, 0x24},
    {0x26, 0xF1, 0x8B, 0x5D, 0x45, 0x84, 0x47, 0xEC, 0x9F, 0x5F, 0x0E, 0x65, 0x1F, 0x04, 0x52, 0xC9},
    {0x43, 0x05, 0x85, 0x33, 0x69, 0x81, 0x49, 0xE6, 0x9B, 0x74, 0xAD, 0x12, 0xCB, 0x86, 0xD5, 0x8C},

    // 10.4 Stream Properties Object Stream Type GUIDs
    {0xF8, 0x69, 0x9E, 0x40, 0x5B, 0x4D, 0x11, 0xCF, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B},
    {0xBC, 0x19, 0xEF, 0xC0, 0x5B, 0x4D, 0x11, 0xCF, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B},
    {0x59, 0xDA, 0xCF, 0xC0, 0x59, 0xE6, 0x11, 0xD0, 0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6},
    {0xB6, 0x1B, 0xE1, 0x00, 0x5B, 0x4E, 0x11, 0xCF, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B},
    {0x35, 0x90, 0x7D, 0xE0, 0xE4, 0x15, 0x11, 0xCF, 0xA9, 0x17, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B},
    {0x91, 0xBD, 0x22, 0x2C, 0xF2, 0x1C, 0x49, 0x7A, 0x8B, 0x6D, 0x5A, 0xA8, 0x6B, 0xFC, 0x01, 0x85},
    {0x3A, 0xFB, 0x65, 0xE2, 0x47, 0xEF, 0x40, 0xF2, 0xAC, 0x2C, 0x70, 0xA9, 0x0D, 0x71, 0xD3, 0x43},

    // 10.4.1 Web stream Type-Specific Data GUIDs
    {0x77, 0x62, 0x57, 0xD4, 0xC6, 0x27, 0x41, 0xCB, 0x8F, 0x81, 0x7A, 0xC7, 0xFF, 0x1C, 0x40, 0xCC},
    {0xDA, 0x1E, 0x6B, 0x13, 0x83, 0x59, 0x40, 0x50, 0xB3, 0x98, 0x38, 0x8E, 0x96, 0x5B, 0xF0, 0x0C},

    // 10.5 Stream Properties Object Error Correction Type GUIDs
    {0x20, 0xFB, 0x57, 0x00, 0x5B, 0x55, 0x11, 0xCF, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B},
    {0xBF, 0xC3, 0xCD, 0x50, 0x61, 0x8F, 0x11, 0xCF, 0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20},

    // 10.6 Header Extension Object GUIDs
    {0xAB, 0xD3, 0xD2, 0x11, 0xA9, 0xBA, 0x11, 0xcf, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65},

    // 10.7 Advanced Content Encryption Object System ID GUIDs
    {0x7A, 0x07, 0x9B, 0xB6, 0xDA, 0xA4, 0x4e, 0x12, 0xA5, 0xCA, 0x91, 0xD3, 0x8D, 0xC1, 0x1A, 0x8D},

    // 10.8 Codec List Object GUIDs
    {0x86, 0xD1, 0x52, 0x41, 0x31, 0x1D, 0x11, 0xD0, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6},

    // 10.9 Script Command Object GUIDs
    {0x4B, 0x1A, 0xCB, 0xE3, 0x10, 0x0B, 0x11, 0xD0, 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6},

    // 10.10 Marker Object GUIDs
    {0x4C, 0xFE, 0xDB, 0x20, 0x75, 0xF6, 0x11, 0xCF, 0x9C, 0x0F, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB},

    // 10.11 Mutual Exclusion Object Exclusion Type GUIDs
    {0xD6, 0xE2, 0x2A, 0x00, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},
    {0xD6, 0xE2, 0x2A, 0x01, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},
    {0xD6, 0xE2, 0x2A, 0x02, 0x35, 0xDA, 0x11, 0xD1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE},

    // 10.12 Bandwidth Sharing Object GUIDs
    {0xAF, 0x60, 0x60, 0xAA, 0x51, 0x97, 0x11, 0xD2, 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9},
    {0xAF, 0x60, 0x60, 0xAB, 0x51, 0x97, 0x11, 0xD2, 0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9},

    // 10.13 Standard Payload Extension System GUIDs
    {0x39, 0x95, 0x95, 0xEC, 0x86, 0x67, 0x4E, 0x2D, 0x8F, 0xDB, 0x98, 0x81, 0x4C, 0xE7, 0x6C, 0x1E},
    {0xE1, 0x65, 0xEC, 0x0E, 0x19, 0xED, 0x45, 0xD7, 0xB4, 0xA7, 0x25, 0xCB, 0xD1, 0xE2, 0x8E, 0x9B},
    {0xD5, 0x90, 0xDC, 0x20, 0x07, 0xBC, 0x43, 0x6C, 0x9C, 0xF7, 0xF3, 0xBB, 0xFB, 0xF1, 0xA4, 0xDC},
    {0x1B, 0x1E, 0xE5, 0x54, 0xF9, 0xEA, 0x4B, 0xC8, 0x82, 0x1A, 0x37, 0x6B, 0x74, 0xE4, 0xC4, 0xB8},
    {0xC6, 0xBD, 0x94, 0x50, 0x86, 0x7F, 0x49, 0x07, 0x83, 0xA3, 0xC7, 0x79, 0x21, 0xB7, 0x33, 0xAD},
    {0x66, 0x98, 0xB8, 0x4E, 0x0A, 0xFA, 0x43, 0x30, 0xAE, 0xB2, 0x1C, 0x0A, 0x98, 0xD7, 0xA4, 0x4D},
    {0x00, 0xE1, 0xAF, 0x06, 0x7B, 0xEC, 0x11, 0xD1, 0xA5, 0x82, 0x00, 0xC0, 0x4F, 0xC2, 0x9C, 0xFB},

};

typedef enum ASF_object_Names_e
{
    // 10.1 Top-level ASF object GUIDs
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

    // 10.3 Header Extension Object GUIDs
    ASF_Extended_Stream_Properties_Object,
    ASF_Advanced_Mutual_Exclusion_Object,
    ASF_Group_Mutual_Exclusion_Object,
    ASF_Stream_Prioritization_Object,
    ASF_Bandwidth_Sharing_Object,
    ASF_Language_List_Object,
    ASF_Metadata_Object,
    ASF_Metadata_Library_Object,
    ASF_Index_Parameters_Object,
    ASF_Media_Object_Index_Parameters_Object,
    ASF_Timecode_Index_Parameters_Object,
    ASF_Compatibility_Object,
    ASF_Advanced_Content_Encryption_Object,

    // 10.4 Stream Properties Object Stream Type GUIDs
    ASF_Audio_Media,
    ASF_Video_Media,
    ASF_Command_Media,
    ASF_JFIF_Media,
    ASF_Degradable_JPEG_Media,
    ASF_File_Transfer_Media,
    ASF_Binary_Media,

    // 10.4.1 Web stream Type-Specific Data GUIDs
    ASF_Web_Stream_Media_Subtype,
    ASF_Web_Stream_Format,

    // 10.5 Stream Properties Object Error Correction Type GUIDs
    ASF_No_Error_Correction,
    ASF_Audio_Spread,

    // 10.6 Header Extension Object GUIDs
    ASF_Reserved_1,

    // 10.7 Advanced Content Encryption Object System ID GUIDs
    ASF_Content_Encryption_System_Windows_Media_DRM_Network_Devices,

    // 10.8 Codec List Object GUIDs
    ASF_Reserved_2,

    // 10.9 Script Command Object GUIDs
    ASF_Reserved_3,

    // 10.10 Marker Object GUIDs
    ASF_Reserved_4,

    // 10.11 Mutual Exclusion Object Exclusion Type GUIDs
    ASF_Mutex_Language,
    ASF_Mutex_Bitrate,
    ASF_Mutex_Unknown,

    // 10.12 Bandwidth Sharing Object GUIDs
    ASF_Bandwidth_Sharing_Exclusive,
    ASF_Bandwidth_Sharing_Partial,

    // 10.13 Standard Payload Extension System GUIDs
    ASF_Payload_Extension_System_Timecode,
    ASF_Payload_Extension_System_File_Name,
    ASF_Payload_Extension_System_Content_Type,
    ASF_Payload_Extension_System_Pixel_Aspect_Ratio,
    ASF_Payload_Extension_System_Sample_Duration,
    ASF_Payload_Extension_System_Encryption_Sample_ID,
    ASF_Payload_Extension_System_Degradable_JPEG,

} ASF_object_Names_e;

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

typedef struct AsfBitrateMutualExclusion_t
{
    int32_t StreamNumber;
    int32_t Reserved;
    int32_t AverageBitrate;

} AsfBitrateMutualExclusion_t;

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

    // TODO
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
    int16_t CodecNameLength; // in UTF16 char
    char *CodecName;
    int16_t CodecDescriptionLength; // in UTF16 char
    char *CodecDescription;
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
    char *CommandTypeName;

} AsfCommandTypes_t;

typedef struct AsfCommands_t
{
    int32_t PresentationTime;
    int16_t TypeIndex;
    int16_t CommandNameLength;
    char *CommandName;

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
    int16_t DescriptorNameLength; // in bytes
    char *DescriptorName;
    int16_t DescriptorValueDataType;
    int16_t DescriptorValueLength; // in bytes

    uint8_t *DescriptorValue_data;
    uint64_t DescriptorValue_numerical;

} AsfContentDescriptor_t;

typedef struct AsfExtendedContentDescriptionObject_t
{
    int16_t ContentDescriptorsCount;
    AsfContentDescriptor_t **ContentDescriptors;

} AsfExtendedContentDescriptionObject_t;

typedef struct AsfContentDescriptionObject_t
{
    // Lengths in bytes
    int16_t TitleLength;
    int16_t AuthorLength;
    int16_t CopyrightLength;
    int16_t DescriptionLength;
    int16_t RatingLength;

    char *Title;
    char *Author;
    char *Copyright;
    char *Varies;
    char *Description;
    char *Rating;

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
    int16_t StreamNumbers[128];

} AsfBitrateMutualExclusionObject_t;

typedef struct AsfErrorCorrectionObject_t
{
    uint8_t ErrorCorrectionType[16];
    int32_t ErrorCorrectionDataLength;
    uint8_t *ErrorCorrectionData;

} AsfErrorCorrectionObject_t;

typedef struct AsfContentBrandingObject_t
{
    int32_t BannerImageType;
    int32_t BannerImageDataSize;
    uint8_t *BannerImageData;
    int32_t BannerImageURLLength; // in bytes
    char *BannerImageURL;
    int32_t CopyrightURLLength; // in bytes
    char *CopyrightURL;

} AsfContentBrandingObject_t;

typedef struct AsfContentEncryptionObject_t
{
    int32_t SecretDataLength;
    uint8_t *SecretData;
    int32_t ProtectionTypeLength; // in bytes
    char *ProtectionType;
    int32_t KeyIDLength; // in bytes
    char *KeyID;
    int32_t LicenseURLLength;
    char *LicenseURL; // in bytes

} AsfContentEncryptionObject_t;

typedef struct AsfExtendedContentEncryptionObject_t
{
    int32_t DataSize;
    uint8_t *Data;

} AsfExtendedContentEncryptionObject_t;

typedef struct AsfDigitalSignatureObject_t
{
    int32_t SignatureType;
    int32_t SignatureDataLength;
    uint8_t *SignatureData;

} AsfDigitalSignatureObject_t;

typedef struct AsfPaddingObject_t
{
    int32_t PaddingDataLength;
    uint8_t *PaddingData;

} AsfPaddingObject_t;

/* ************************************************************************** */
typedef struct AsfHeaderObject_t
{
    int32_t NumberOfHeaderObjects;
    int8_t Reserved1;
    int8_t Reserved2;

    AsfFilePropertiesObject_t fp;       //!< Contains global file attributes
    AsfStreamPropertiesObject_t sp[16]; //!< Defines a digital media stream and its characteristics
    AsfHeaderExtensionObject_t ex;      //!< Allows additional functionality to be added to an ASF file while maintaining backward compatibility

    AsfCodecListObject_t *cl;
    AsfScriptCommandObject_t *sc;        //!< Contains commands that can be executed on the playback timeline
    AsfMarkerObject_t *md;               //!< Provides named jump points within a file
    AsfBitrateMutualExclusionObject_t *bme;
    AsfErrorCorrectionObject_t *ec;
    AsfContentDescriptionObject_t *cd;   //!< Contains bibliographic information
    AsfExtendedContentDescriptionObject_t *ecd;
    AsfStreamBitratePropertiesObject_t *sbp;
    AsfContentBrandingObject_t *cb;
    AsfContentEncryptionObject_t *ce;
    AsfExtendedContentEncryptionObject_t *ece;
    AsfDigitalSignatureObject_t *ds;
    AsfPaddingObject_t *pad;

} AsfHeaderObject_t;

/* ************************************************************************** */
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
