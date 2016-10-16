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
#include "../../typedef.h"
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
    element_EBML = 0x1A45DFA3,            //!< * (level 0) EBML file header
    element_EBMLVersion = 0x4286,         //!<
    element_EBMLReadVersion = 0x42F7,     //!<
    element_EBMLMaxIDLength = 0x42F2,     //!<
    element_EBMLMaxSizeLength = 0x42F3,   //!<
    element_DocType = 0x4282,             //!<
    element_DocTypeVersion = 0x4287,
    element_DocTypeReadVersion = 0x4285,

    element_Segment = 0x18538067,         //!< * (level 0) This element contains all other top-level (level 1) elements.

    element_SeekHead = 0x114D9B74,        //!< (level 1) Meta Seek Information
    element_Seek = 0x4DBB,                //!<
    element_SeekId = 0x53AB,              //!<
    element_SeekPosition = 0x53AC,        //!<

    element_Info = 0x1549A966,            //!< * (level 1) Segment Information

    element_Cluster = 0x1F43B675,         //!< (level 1) Cluster
        element_TimeCode = 0xE7,          //!<
        element_BlockGroup = 0xA0,        //!<
        element_Block = 0xA1,             //!<

    element_Tracks = 0x1654AE6B,          //!< (level 1) Track
    element_TrackEntry = 0xAE,            //!<
        element_TrackNumber = 0xD7,       //!<
        element_TrackUID = 0x73C5,        //!<
        element_TrackType = 0x83,         //!<
        element_Name = 0x536E,            //!<
        element_CodecID = 0x86,           //!<
        element_CodecPrivate = 0x63A2,    //!<
        element_CodecName = 0x258688,     //!<

    element_Cues = 0x1C53BB6B,            //!< (level 1) Cueing Data
    element_CuePoint = 0xBB,              //!<
    element_CueTime = 0xB3,               //!<
    element_CueTrackPositions = 0xB7,     //!<
    element_CueTrack = 0xF7,              //!<
    element_CueClusterPosition = 0xF1,    //!<
    element_CueBlockNumber = 0x5378,      //!<

    element_Attachments = 0x1941A469,     //!< (level 1) Attachment

    element_Chapters = 0x1043A770,        //!< (level 1) Chapter

    element_Tags = 0x1254C367             //!< (level 1) Tagging

    //element_ = 0x, //!<

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

/*
    CodecID_H262[7] = {'V', '_', 'M', 'P', 'E', 'G', '2'};
    CodecID_H264[15] = {'V', '_', 'M', 'P', 'E', 'G', '4', '/', 'I', 'S', 'O', '/', 'A', 'V', 'C'};
    CodecID_XVID[15] = {'V', '_', 'M', 'P', 'E', 'G', '4', '/', 'I', 'S', 'O', '/', 'A', 'S', 'P'};
*/

//! Structure for MKV video infos
typedef struct mkv_t
{
    bool run;                   //!< A convenient way to stop the parser from any sublevel

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} mkv_t;

/* ************************************************************************** */
#endif // PARSER_MKV_STRUCT_H
