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
 * \file      mkv.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "mkv.h"
#include "mkv_struct.h"
#include "ebml.h"

#include "mkv_tracks.h"
#include "mkv_cluster.h"
#include "mkv_convert.h"

#include "../xml_mapper.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_attachments_attachedfile(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_attachments_attachedfile()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Attached File");

    mkv_attachedfile_t file;

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
            case eid_FileDescription:
                file.FileDescription = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "FileDescription");
                break;
            case eid_FileName:
                file.FileName = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "FileName");
                break;
            case eid_FileMimeType:
                file.FileMimeType = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "FileMimeType");
                break;
            case eid_FileData:
                file.FileData = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "FileData");
                if (file.FileData) file.FileData_size = element_sub.size;
                break;
            case eid_FileUID:
                file.FileUID = read_ebml_data_uint_UID(bitstr, &element_sub, mkv->xml, "FileUID");
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    delete [] file.FileDescription;
    delete [] file.FileName;
    delete [] file.FileMimeType;
    delete [] file.FileData;

    return retcode;
}
/* ************************************************************************** */

static int mkv_parse_attachments(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_attachments()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Attachments");

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
            case eid_AttachedFile:
                retcode = mkv_parse_attachments_attachedfile(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_chapters_atom(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapters_atom()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Chapter Atom");

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
            case eid_ChapterUID:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterUID");
                break;
            case eid_ChapterStringUID:
                read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapterStringUID");
                break;
            case eid_ChapterTimeStart:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterTimeStart");
                break;
            case eid_ChapterTimeEnd:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterTimeEnd");
                break;
            case eid_ChapterFlagHidden:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterFlagHidden");
                break;
            case eid_ChapterFlagEnabled:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterFlagEnabled");
                break;
            case eid_ChapterSegmentUID:
                read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ChapterSegmentUID");
                break;
            case eid_ChapterSegmentEditionUID:
                read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ChapterSegmentEditionUID");
                break;
            case eid_ChapterPhysicalEquiv:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterPhysicalEquiv");
                break;

            // TODO stuff still missing here

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_chapters_entry(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapters_entry()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Edition Entry");

    //mkv_editionentry_t ee;

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
            case eid_EditionUID:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionUID");
                break;
            case eid_EditionFlagHidden:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagHidden");
                break;
            case eid_EditionFlagDefault:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagDefault");
                break;
            case eid_EditionFlagOrdered:
                read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagOrdered");
                break;

            case eid_ChapterAtom:
                mkv_parse_chapters_atom(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

static int mkv_parse_chapters(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapters()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Chapters");

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
            case eid_EditionEntry:
                retcode = mkv_parse_chapters_entry(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_tag(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tag()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Tag");

    mkv_tag_t tag;

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
                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_tags(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tags()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Tags");

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
            case eid_Tag:
                retcode = mkv_parse_tag(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_cues_cuepoint_pos_ref(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cues_cuepoint_pos_ref()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cue Reference");

    uint64_t CueRefTime = 0;

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
                case eid_CueRefTime:
                    CueRefTime = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueTrack");
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}
/* ************************************************************************** */

static int mkv_parse_cues_cuepoint_pos(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cues_cuepoint_pos()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cue Track Positions");

    mkv_cuetrackpos_t pos;

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
                case eid_CueTrack:
                    pos.CueTrack = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueTrack");
                    break;
                case eid_CueClusterPosition:
                    pos.CueClusterPosition = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueClusterPosition");
                    break;
                case eid_CueRelativePosition:
                    pos.CueRelativePosition = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueRelativePosition");
                    break;
                case eid_CueDuration:
                    pos.CueDuration = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueDuration");
                    break;
                case eid_CueBlockNumber:
                    pos.CueBlockNumber = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueBlockNumber");
                    break;
                case eid_CueCodecState:
                    pos.CueCodecState = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueCodecState");
                    break;

                case eid_CueReference:
                    retcode = mkv_parse_cues_cuepoint_pos_ref(bitstr, &element_sub, mkv);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_cues_cuepoint(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cues_cuepoint()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cue Point");

    mkv_cuepoint_t cue;

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
                case eid_CueTime:
                    cue.CueTime = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueTime");
                    break;
                case eid_CueTrackPositions:
                    retcode = mkv_parse_cues_cuepoint_pos(bitstr, &element_sub, mkv);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_cues(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cues()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cues");

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
            case eid_CuePoint:
                retcode = mkv_parse_cues_cuepoint(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_info_chapter(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info_chapter()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Info Chapter");

    mkv_info_chapter_t chap;

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
                chap.ChapterTranslateEditionUID = read_ebml_data_uint_UID(bitstr, &element_sub, mkv->xml, "ChapterTranslateEditionUID");
                break;
            case eid_ChapterTranslateCodec:
                chap.ChapterTranslateCodec = read_ebml_data_uint_UID(bitstr, &element_sub, mkv->xml, "ChapterTranslateCodec");
                break;
            case eid_ChapterTranslateID:
                chap.ChapterTranslateID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ChapterTranslateID");
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_info(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_info()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Info");

    mkv->info.TimecodeScale = 1000000; // default value

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
                mkv->info.SegmentUID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "SegmentUID");
                break;
            case eid_SegmentFilename:
                mkv->info.SegmentFilename = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "SegmentFilename");
                break;
            case eid_PrevUID:
                mkv->info.PrevUID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "PrevUID");
                break;
            case eid_PrevFilename:
                mkv->info.PrevFilename = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "PrevFilename");
                break;
            case eid_NextUID:
                mkv->info.NextUID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "NextUID");
                break;
            case eid_NextFilename:
                mkv->info.NextFilename = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "NextFilename");
                break;
            case eid_SegmentFamily:
                mkv->info.SegmentFamily = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "SegmentFamily");
                break;
            case eid_ChapterTranslate:
                retcode = mkv_parse_info_chapter(bitstr, &element_sub, mkv);
                break;
            case eid_TimecodeScale:
                mkv->info.TimecodeScale = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TimecodeScale");
                break;
            case eid_Duration:
                mkv->info.Duration = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "Duration");
                break;
            case eid_DateUTC:
                mkv->info.DateUTC = read_ebml_data_date(bitstr, &element_sub, mkv->xml, "DateUTC");
                break;
            case eid_Title:
                mkv->info.Title = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "Title");
                break;
            case eid_MuxingApp:
                mkv->info.MuxingApp = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "MuxingApp");
                break;
            case eid_WritingApp:
                mkv->info.WritingApp = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "WritingApp");
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_seekhead_seek(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead_seek()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Seek");

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
                SeekID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "SeekID");
                delete [] SeekID;
                break;
            case eid_SeekPosition:
                SeekPosition = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "SeekPosition");
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

static int mkv_parse_seekhead(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_seekhead()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "SeekHead");

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

            retcode = jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_segment(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_segment()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Segment");

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
                retcode = mkv_parse_cluster(bitstr, &element_sub, mkv);
                break;
            case eid_Tracks:
                retcode = mkv_parse_tracks(bitstr, &element_sub, mkv);
                break;
            case eid_Cues:
                retcode = mkv_parse_cues(bitstr, &element_sub, mkv);
                break;
            case eid_Attachments:
                retcode = mkv_parse_attachments(bitstr, &element_sub, mkv);
                break;
            case eid_Chapters:
                retcode = mkv_parse_chapters(bitstr, &element_sub, mkv);
                break;
            case eid_Tags:
                retcode = mkv_parse_tags(bitstr, &element_sub, mkv);
                break;
            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

int ebml_parse_header(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "ebml_parse_header()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "EBML Header");

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
                    mkv->ebml.EBMLVersion = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EBMLVersion");
                    break;
                case eid_EBMLReadVersion:
                    mkv->ebml.EBMLReadVersion = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EBMLReadVersion");
                    break;
                case eid_EBMLMaxIDLength:
                    mkv->ebml.EBMLMaxIDLength = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EBMLMaxIDLength");
                    break;
                case eid_EBMLMaxSizeLength:
                    mkv->ebml.EBMLMaxSizeLength = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EBMLMaxSizeLength");
                    break;
                case eid_DocType:
                    mkv->ebml.DocType = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "DocType");
                    if (strncmp(mkv->ebml.DocType, "matroska", 8) == 0)
                        mkv->profile = PROF_MKV_MATROSKA;
                    else if (strncmp(mkv->ebml.DocType, "webm", 4) == 0)
                        mkv->profile = PROF_MKV_WEBM;
                    break;
                case eid_DocTypeVersion:
                    mkv->ebml.DocTypeVersion = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DocTypeVersion");
                    break;
                case eid_DocTypeReadVersion:
                    mkv->ebml.DocTypeReadVersion = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DocTypeReadVersion");
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

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

                retcode = jumpy_mkv(bitstr, NULL, &element);
            }
        }

        // xmlMapper
        if (xmlMapperFinalize(mkv.xml) == SUCCESS)
            media->container_mapper_fd = mkv.xml;

        // Convert internal MKV representation into a MediaFile_t
        mkv_convert(media, &mkv);

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
