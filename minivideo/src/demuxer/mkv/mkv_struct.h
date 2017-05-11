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
 * \file      mkv_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_MKV_STRUCT_H
#define PARSER_MKV_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include <stdio.h>

/* ************************************************************************** */

/*!
 * \enum EbmlElement_e
 * \brief Identifies the EBML element type.
 *
 * The element marked with an * are mandatory.
 */
typedef enum EbmlElement_e
{
    eid_EBML = 0x1A45DFA3,              //!< * (level 0) EBML file header
        eid_EBMLVersion = 0x4286,       //!<
        eid_EBMLReadVersion = 0x42F7,   //!<
        eid_EBMLMaxIDLength = 0x42F2,   //!<
        eid_EBMLMaxSizeLength = 0x42F3, //!<
        eid_DocType = 0x4282,           //!<
        eid_DocTypeVersion = 0x4287,    //!<
        eid_DocTypeReadVersion = 0x4285,//!<

    eid_Segment = 0x18538067,           //!< * (level 0) This element contains all other top-level (level 1) elements.

    eid_SeekHead = 0x114D9B74,          //!< (level 1) Meta Seek Information
        eid_Seek = 0x4DBB,              //!<
            eid_SeekId = 0x53AB,        //!<
            eid_SeekPosition = 0x53AC,  //!<

    eid_Info = 0x1549A966,              //!< * (level 1) Segment Information
        eid_SegmentUID = 0x73A4,        //!<
        eid_SegmentFilename = 0x7384,   //!<
        eid_PrevUID = 0x3CB923,         //!<
        eid_PrevFilename = 0x3C83AB,    //!<
        eid_NextUID = 0x3EB923,         //!<
        eid_NextFilename = 0x3E83BB,    //!<
        eid_SegmentFamily = 0x4444,     //!<
        eid_ChapterTranslate = 0x6924,  //!<
            eid_ChapterTranslateEditionUID = 0x69FC,//!<
            eid_ChapterTranslateCodec = 0x69BF,     //!<
            eid_ChapterTranslateID = 0x69A5,        //!<
        eid_TimecodeScale = 0x2AD7B1,   //!<
        eid_Duration = 0x4489,          //!<
        eid_DateUTC = 0x4461,           //!<
        eid_Title = 0x7BA9,             //!<
        eid_MuxingApp = 0x4D80,         //!<
        eid_WritingApp = 0x5741,        //!<

    eid_Cluster = 0x1F43B675,           //!< (level 1) Cluster
        eid_TimeCode = 0xE7,            //!<
        eid_SimpleBlock = 0xA3,         //!<
        eid_BlockGroup = 0xA0,          //!<
        eid_Block = 0xA1,               //!<

    eid_Tracks = 0x1654AE6B,            //!< (level 1) Track
    eid_TrackEntry = 0xAE,              //!<
        eid_TrackNumber = 0xD7,         //!<
        eid_TrackUID = 0x73C5,          //!<
        eid_TrackType = 0x83,           //!<
        eid_Name = 0x536E,              //!<
        eid_CodecID = 0x86,             //!<
        eid_CodecPrivate = 0x63A2,      //!<
        eid_CodecName = 0x258688,       //!<

    eid_Cues = 0x1C53BB6B,              //!< (level 1) Cueing Data
    eid_CuePoint = 0xBB,                //!<
    eid_CueTime = 0xB3,                 //!<
    eid_CueTrackPositions = 0xB7,       //!<
    eid_CueTrack = 0xF7,                //!<
    eid_CueClusterPosition = 0xF1,      //!<
    eid_CueBlockNumber = 0x5378,        //!<

    eid_Attachments = 0x1941A469,       //!< (level 1) Attachment

    eid_Chapters = 0x1043A770,          //!< (level 1) Chapter

    eid_Tags = 0x1254C367,              //!< (level 1) Tagging

    eid_void = 0xEC,                    //!< (global) Tagging
    eid_crc32 = 0xBF                    //!< (global) Tagging

    //eid_ = 0x, //!<

} EbmlElement_e;

/*!
 * \enum EbmlDocType_e
 * \brief Identifies the doctype of the current EBML file.
 */
typedef enum EbmlDocType_e
{
    doctype_unknown  = 0,
    doctype_matroska = 1,
    doctype_webm     = 2

} EbmlDocType_e;

//! Structure for MKV video infos
typedef struct mkv_t
{
    bool run;                   //!< A convenient way to stop the parser from any sublevel

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} mkv_t;

/* ************************************************************************** */
#endif // PARSER_MKV_STRUCT_H
