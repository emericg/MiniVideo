/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_chaptertrack(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                  std::vector <uint64_t> *ChapterTrackNumbers)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chaptertrack()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Chapter Track");

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
            case eid_ChapterTrackNumber:
                ChapterTrackNumbers->push_back(read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterTrackNumber"));
                break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_chapterdisplay(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                    mkv_chapter_atom_display_t *display)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapterdisplay()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Chapter Display");

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
            case eid_ChapString:
                display->ChapString = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapString");
                break;
            case eid_ChapLanguage:
                display->ChapLanguage = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapLanguage");
                break;
            case eid_ChapLanguageIETF:
                display->ChapLanguageIETF = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapLanguage");
                break;
            case eid_ChapCountry:
                display->ChapCountry = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapCountry");
                break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_chapterprocess(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                    mkv_chapter_atom_process_t *process)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapterprocess()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Chapter Process");

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
            case eid_ChapProcessCodecID:
                process->ChapProcessCodecID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapProcessCodecID");
                break;
            case eid_ChapProcessPrivate:
                process->ChapProcessCodecID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapProcessPrivate");
                break;
            //case eid_ChapProcessCommand:
            //    // TODO
            //    break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_chapters_atom(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                   mkv_chapter_atom_t *atom)
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
                atom->ChapterUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterUID");
                break;
            case eid_ChapterStringUID:
                atom->ChapterStringUID = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "ChapterStringUID");
                break;
            case eid_ChapterTimeStart:
                atom->ChapterTimeStart = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterTimeStart");
                break;
            case eid_ChapterTimeEnd:
                atom->ChapterTimeEnd = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterTimeEnd");
                break;
            case eid_ChapterFlagHidden:
                atom->ChapterFlagHidden = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterFlagHidden");
                break;
            case eid_ChapterFlagEnabled:
                atom->ChapterFlagEnabled = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterFlagEnabled");
                break;
            case eid_ChapterSegmentUID:
                atom->ChapterSegmentUID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ChapterSegmentUID");
                break;
            case eid_ChapterSegmentEditionUID:
                atom->ChapterSegmentEditionUID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ChapterSegmentEditionUID");
                break;
            case eid_ChapterPhysicalEquiv:
                atom->ChapterPhysicalEquiv = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChapterPhysicalEquiv");
                break;

            case eid_ChapterTrack:
                retcode = mkv_parse_chaptertrack(bitstr, &element_sub, mkv, &atom->ChapterTrackNumbers);
                break;
            case eid_ChapterDisplay: {
                mkv_chapter_atom_display_t *dd = new mkv_chapter_atom_display_t;
                atom->ChapterDisplays.push_back(dd);
                retcode = mkv_parse_chapterdisplay(bitstr, &element_sub, mkv, dd);
                } break;
            case eid_ChapProcess: {
                mkv_chapter_atom_process_t *pp = new mkv_chapter_atom_process_t;
                atom->ChapterProcesses.push_back(pp);
                retcode = mkv_parse_chapterprocess(bitstr, &element_sub, mkv, pp);
                } break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_chapters_entry(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                    mkv_chapter_editionentry_t *entry)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_chapters_entry()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Edition Entry");

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
                entry->EditionUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionUID");
                break;
            case eid_EditionFlagHidden:
                entry->EditionFlagHidden = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagHidden");
                break;
            case eid_EditionFlagDefault:
                entry->EditionFlagDefault = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagDefault");
                break;
            case eid_EditionFlagOrdered:
                entry->EditionFlagOrdered = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "EditionFlagOrdered");
                break;

            case eid_ChapterAtom: {
                mkv_chapter_atom_t *atom = new mkv_chapter_atom_t;
                entry->atoms.push_back(atom);
                mkv_parse_chapters_atom(bitstr, &element_sub, mkv, atom);
                } break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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
                {
                    mkv_chapter_editionentry_t *ee = new mkv_chapter_editionentry_t;
                    mkv->chapters.ChapterEditionEntry.push_back(ee);
                    retcode = mkv_parse_chapters_entry(bitstr, &element_sub, mkv, ee);
                } break;

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_simpletag(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                               mkv_tag_simpletag_t *simpletag)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_simpletag()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "SimpleTag");

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
            case eid_TagName:
                simpletag->TagName = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "TagName");
                break;
            case eid_TagLanguage:
                simpletag->TagLanguage = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "TagLanguage");
                break;
            case eid_TagLanguageIETF:
                simpletag->TagLanguageIETF = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "TagLanguageIETF");
                break;
            case eid_TagDefault:
                simpletag->TagDefault = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TagDefault");
                break;
            case eid_TagString:
                simpletag->TagString = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "TagString");
                break;
            case eid_TagBinary:
                simpletag->TagBinary = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "TagBinary");
                break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_target(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                            mkv_tag_target_t *tagtarget)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_target()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Targets");

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
            case eid_TargetTypeValue:
                tagtarget->TargetTypeValue = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TargetTypeValue");
                break;
            case eid_TargetType:
                tagtarget->TargetType = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "TargetType");
                break;
            case eid_TagTrackUID:
                tagtarget->TagTrackUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TagTrackUID");
                break;
            case eid_TagEditionUID:
                tagtarget->TagEditionUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TagEditionUID");
                break;
            case eid_TagChapterUID:
                tagtarget->TagChapterUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TagChapterUID");
                break;
            case eid_TagAttachmentUID:
                tagtarget->TagAttachmentUID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TagAttachmentUID");
                break;

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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
                case eid_Targets:
                    retcode = mkv_parse_target(bitstr, &element_sub, mkv, &tag.target);
                    break;
                case eid_SimpleTag: {
                    mkv_tag_simpletag_t *st = new mkv_tag_simpletag_t;
                    tag.simpletags.push_back(st);
                    retcode = mkv_parse_simpletag(bitstr, &element_sub, mkv, st);
                    } break;

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                    break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    delete [] tag.target.TargetType;
    for (unsigned i = 0; i < tag.simpletags.size(); i++)
    {
        delete [] tag.simpletags.at(i)->TagName;
        delete [] tag.simpletags.at(i)->TagBinary;
        delete [] tag.simpletags.at(i)->TagString;
        delete [] tag.simpletags.at(i)->TagLanguage;
        delete [] tag.simpletags.at(i)->TagLanguageIETF;
        delete tag.simpletags.at(i);
    }
    tag.simpletags.clear();

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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

static int mkv_parse_cues_cuepoint_pos_ref(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv,
                                           std::vector<uint64_t> *cueposref)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_cues_cuepoint_pos_ref()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Cue Reference");

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
                    cueposref->push_back(read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueRefTime"));
                    break;

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

    mkv_cuetrackpos_t cuepos;

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
                    cuepos.CueTrack = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueTrack");
                    break;
                case eid_CueClusterPosition:
                    cuepos.CueClusterPosition = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueClusterPosition");
                    break;
                case eid_CueRelativePosition:
                    cuepos.CueRelativePosition = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueRelativePosition");
                    break;
                case eid_CueDuration:
                    cuepos.CueDuration = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueDuration");
                    break;
                case eid_CueBlockNumber:
                    cuepos.CueBlockNumber = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueBlockNumber");
                    break;
                case eid_CueCodecState:
                    cuepos.CueCodecState = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CueCodecState");
                    break;

                case eid_CueReference:
                    retcode = mkv_parse_cues_cuepoint_pos_ref(bitstr, &element_sub, mkv, &cuepos.CueRefTimes);
                    break;

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }
        }

        retcode = jumpy_mkv(bitstr, element, &element_sub);
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    delete [] chap.ChapterTranslateID;

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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

    uint8_t *SeekID = nullptr;
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

            case eid_void:
                retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                break;
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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
            case eid_crc32:
                retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element_sub, mkv->xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element_sub, mkv->xml);
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
    Bitstream_t *bitstr = init_bitstream(media, nullptr);

    if (bitstr != nullptr)
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

                case eid_void:
                    retcode = ebml_parse_void(bitstr, &element, mkv.xml);
                    break;
                case eid_crc32:
                    retcode = ebml_parse_crc32(bitstr, &element, mkv.xml);
                    break;

                default:
                    retcode = ebml_parse_unknown(bitstr, &element, mkv.xml);
                    break;
                }

                retcode = jumpy_mkv(bitstr, nullptr, &element);
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
