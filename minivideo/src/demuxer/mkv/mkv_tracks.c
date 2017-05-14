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
 * \file      mkv_tracks.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_tracks.h"
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
/* ************************************************************************** */

static int mkv_parse_tracks_entry_translate(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_translate()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Translate");

    uint64_t TrackTranslateEditionUID = 0;
    uint64_t TrackTranslateCodec = 0;
    uint8_t *TrackTranslateTrackID = NULL;

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
            case eid_TrackTranslateEditionUID:
                TrackTranslateEditionUID = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackTranslateEditionUID = %llu", TrackTranslateEditionUID);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackTranslateEditionUID>%lu</TrackTranslateEditionUID>\n", TrackTranslateEditionUID);
                break;
            case eid_TrackTranslateCodec:
                TrackTranslateCodec = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackTranslateCodec      = %llu", TrackTranslateCodec);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackTranslateCodec>%lu</TrackTranslateCodec>\n", TrackTranslateCodec);
                break;
            case eid_TrackTranslateTrackID:
                TrackTranslateTrackID = read_ebml_data_binary(bitstr, element_sub.size);
                TRACE_1(MKV, "TrackTranslateTrackID      = '%s'", TrackTranslateTrackID);
                fprintf(mkv->xml, "  <TrackTranslateTrackID>%s</TrackTranslateTrackID>\n", TrackTranslateTrackID);
                break;
            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    free(TrackTranslateTrackID);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_tracks_entry_video_colour_mastering(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video_colour_mastering()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Mastering Metadata");

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

static int mkv_parse_tracks_entry_video_colour(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video_colour()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Colour");

    uint64_t MatrixCoefficients = 0;
    uint64_t BitsPerChannel = 0;
    uint64_t ChromaSubsamplingHorz = 0;
    uint64_t ChromaSubsamplingVert = 0;
    uint64_t CbSubsamplingHorz = 0;
    uint64_t CbSubsamplingVert = 0;
    uint64_t ChromaSitingHorz = 0;
    uint64_t ChromaSitingVert = 0;
    uint64_t Range = 0;
    uint64_t TransferCharacteristics = 0;
    uint64_t Primaries = 0;
    uint64_t MaxCLL = 0;
    uint64_t MaxFALL = 0;

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
            case eid_MatrixCoefficients:
                MatrixCoefficients = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* MatrixCoefficients = %llu", MatrixCoefficients);
                if (mkv->xml) fprintf(mkv->xml, "  <MatrixCoefficients>%lu</MatrixCoefficients>\n", MatrixCoefficients);
                break;

            case eid_MasteringMetadata:
                retcode = mkv_parse_tracks_entry_video_colour_mastering(bitstr, element, mkv);
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

static int mkv_parse_tracks_entry_video(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Video");

    track->video = calloc(1, sizeof(mkv_track_video_t));
    if (track->video == NULL)
        retcode = FAILURE;

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
            case eid_FlagInterlaced:
                track->video->FlagInterlaced = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FlagInterlaced");
                break;
            case eid_FieldOrder:
                track->video->FieldOrder = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FieldOrder");
                break;
            case eid_StereoMode:
                track->video->StereoMode = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "StereoMode");
                break;
            case eid_AlphaMode:
                track->video->AlphaMode = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "AlphaMode");
                break;
            case eid_PixelWidth:
                track->video->PixelWidth = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelWidth");
                break;
            case eid_PixelHeight:
                track->video->PixelHeight = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelHeight");
                break;
            case eid_PixelCropBottom:
                track->video->PixelCropBottom = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelCropBottom");
                break;
            case eid_PixelCropTop:
                track->video->PixelCropTop = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelCropTop");
                break;
            case eid_PixelCropLeft:
                track->video->PixelCropLeft = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelCropLeft");
                break;
            case eid_PixelCropRight:
                track->video->PixelCropRight = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "PixelCropRight");
                break;
            case eid_DisplayWidth:
                track->video->DisplayWidth = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "DisplayWidth");
                break;
            case eid_DisplayHeight:
                track->video->DisplayHeight = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "DisplayHeight");
                break;
            case eid_DisplayUnit:
                track->video->DisplayUnit = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "DisplayUnit");
                break;
            case eid_AspectRatioType:
                track->video->AspectRatioType = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "AspectRatioType");
                break;
            case eid_ColourSpace:
                track->video->ColourSpace = read_ebml_data_binary2(bitstr, &element_sub, mkv->xml, "ColourSpace");
                if (track->video->ColourSpace) track->video->ColourSpace_size = element_sub.size;
                break;

            case eid_Colour:
                retcode = mkv_parse_tracks_entry_video_colour(bitstr, &element_sub, mkv, track);
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

static int mkv_parse_tracks_entry_audio(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_audio()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Audio");

    track->audio = calloc(1, sizeof(mkv_track_audio_t));
    if (track->audio == NULL)
        retcode = FAILURE;

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
            case eid_SamplingFrequency:
                track->audio->SamplingFrequency = read_ebml_data_float2(bitstr, &element_sub, mkv->xml, "SamplingFrequency");
                break;
            case eid_OutputSamplingFrequency:
                track->audio->OutputSamplingFrequency = read_ebml_data_float2(bitstr, &element_sub, mkv->xml, "OutputSamplingFrequency");
                break;
            case eid_Channels:
                track->audio->Channels = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "Channels");
                break;
            case eid_BitDepth:
                track->audio->BitDepth = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "BitDepth");
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

static int mkv_parse_tracks_entry_operation(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_operation()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Entry Operation");

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

static int mkv_parse_tracks_entry_contentencoding(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_contentencoding()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Content Encoding");

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

static int mkv_parse_tracks_entry(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *mkv_track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Track Entry");

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
            case eid_TrackNumber:
                mkv_track->TrackNumber = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "TrackNumber");
                break;
            case eid_TrackUID:
                mkv_track->TrackUID = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "TrackUID");
                break;
            case eid_TrackType:
                mkv_track->TrackType = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "TrackType");
                break;
            case eid_FlagEnabled:
                mkv_track->FlagEnabled = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FlagEnabled");
                break;
            case eid_FlagDefault:
                mkv_track->FlagDefault = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FlagDefault");
                break;
            case eid_FlagForced:
                mkv_track->FlagForced = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FlagForced");
                break;
            case eid_FlagLacing:
                mkv_track->FlagLacing = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "FlagLacing");
                break;
            case eid_MinCache:
                mkv_track->MinCache = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "MinCache");
                break;
            case eid_MaxCache:
                mkv_track->MaxCache = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "MaxCache");
                break;
            case eid_DefaultDuration:
                mkv_track->DefaultDuration = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "DefaultDuration");
                break;
            case eid_DefaultDecodedFieldDuration:
                mkv_track->DefaultDecodedFieldDuration = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "DefaultDecodedFieldDuration");
                break;
            case eid_MaxBlockAdditionID:
                mkv_track->MaxBlockAdditionID = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "MaxBlockAdditionID");
                break;
            case eid_Name:
                mkv_track->Name = read_ebml_data_string2(bitstr, &element_sub, mkv->xml, "Name");
                break;
            case eid_Language:
                mkv_track->Language = read_ebml_data_string2(bitstr, &element_sub, mkv->xml, "Language");
                break;
            case eid_CodecID:
                mkv_track->CodecID = read_ebml_data_string2(bitstr, &element_sub, mkv->xml, "CodecID");
                break;
            case eid_CodecPrivate:
                mkv_track->CodecPrivate = read_ebml_data_binary2(bitstr, &element_sub, mkv->xml, "CodecPrivate");
                if (mkv_track->CodecPrivate) mkv_track->CodecPrivate_size = element_sub.size;
                break;
            case eid_CodecName:
                mkv_track->CodecName = read_ebml_data_string2(bitstr, &element_sub, mkv->xml, "CodecName");
                break;
            case eid_CodecDecodeAll:
                mkv_track->CodecDecodeAll = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "CodecDecodeAll");
                break;
            case eid_TrackOverlay:
                mkv_track->TrackOverlay = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "TrackOverlay");
                break;
            case eid_AttachmentLink:
                mkv_track->AttachmentLink = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "AttachmentLink");
                break;
            case eid_CodecDelay:
                mkv_track->CodecDelay = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "CodecDelay");
                break;
            case eid_SeekPreRoll:
                mkv_track->SeekPreRoll = read_ebml_data_uint2(bitstr, &element_sub, mkv->xml, "SeekPreRoll");
                break;

            case eid_TrackTranslate:
                retcode = mkv_parse_tracks_entry_translate(bitstr, &element_sub, mkv, mkv_track);
                break;
            case eid_Video:
                retcode = mkv_parse_tracks_entry_video(bitstr, &element_sub, mkv, mkv_track);
                break;
            case eid_Audio:
                retcode = mkv_parse_tracks_entry_audio(bitstr, &element_sub, mkv, mkv_track);
                break;
            case eid_TrackOperation:
                retcode = mkv_parse_tracks_entry_operation(bitstr, &element_sub, mkv, mkv_track);
                break;
            case eid_ContentEncodings:
                retcode = mkv_parse_tracks_entry_contentencoding(bitstr, &element_sub, mkv, mkv_track);
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

int mkv_parse_tracks(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Tracks");

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
            case eid_TrackEntry:
                mkv->tracks[mkv->tracks_count] = (mkv_track_t*)calloc(1, sizeof(mkv_track_t));
                if (mkv->tracks[mkv->tracks_count])
                    retcode = mkv_parse_tracks_entry(bitstr, &element_sub, mkv, mkv->tracks[mkv->tracks_count]);
                else
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                mkv->tracks_count++;
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
