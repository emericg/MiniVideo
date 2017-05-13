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

static int mkv_parse_track_entry_translate(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_translate()" CLR_RESET);
    int retcode = SUCCESS;

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
                TrackTranslateTrackID = read_ebml_data_string(bitstr, element_sub.size);
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

static int mkv_parse_track_entry_video_colour_mastering(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_video_colour_mastering()" CLR_RESET);
    int retcode = SUCCESS;

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

static int mkv_parse_track_entry_video_colour(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_video_colour()" CLR_RESET);
    int retcode = SUCCESS;

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
                retcode = mkv_parse_track_entry_video_colour_mastering(bitstr, element, mkv->xml);
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

static int mkv_parse_track_entry_video(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_video()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml, "Video");

    uint64_t FlagInterlaced = 0;
    uint64_t FieldOrder = 0;
    uint64_t StereoMode = 0;
    uint64_t AlphaMode = 0;
    uint64_t PixelWidth = 0;
    uint64_t PixelHeight = 0;
    uint64_t PixelCropBottom = 0;
    uint64_t PixelCropTop = 0;
    uint64_t PixelCropLeft = 0;
    uint64_t PixelCropRight = 0;
    uint64_t DisplayWidth = 0;
    uint64_t DisplayHeight = 0;
    uint64_t DisplayUnit = 0;
    uint64_t AspectRatioType = 0;
    uint8_t *ColourSpace = NULL;

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
                FlagInterlaced = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FlagInterlaced = %llu", FlagInterlaced);
                if (mkv->xml) fprintf(mkv->xml, "  <FlagInterlaced>%lu</FlagInterlaced>\n", FlagInterlaced);
                break;
            case eid_FieldOrder:
                FieldOrder = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FieldOrder      = %llu", FieldOrder);
                if (mkv->xml) fprintf(mkv->xml, "  <FieldOrder>%lu</FieldOrder>\n", FieldOrder);
                break;
            case eid_StereoMode:
                StereoMode = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* StereoMode = %llu", StereoMode);
                if (mkv->xml) fprintf(mkv->xml, "  <StereoMode>%lu</StereoMode>\n", StereoMode);
                break;
            case eid_AlphaMode:
                AlphaMode = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* AlphaMode      = %llu", AlphaMode);
                if (mkv->xml) fprintf(mkv->xml, "  <AlphaMode>%lu</AlphaMode>\n", AlphaMode);
                break;
            case eid_PixelWidth:
                PixelWidth = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelWidth = %llu", PixelWidth);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelWidth>%lu</PixelWidth>\n", PixelWidth);
                break;
            case eid_PixelHeight:
                PixelHeight = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelHeight      = %llu", PixelHeight);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelHeight>%lu</PixelHeight>\n", PixelHeight);
                break;
            case eid_PixelCropBottom:
                PixelCropBottom = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelCropBottom = %llu", PixelCropBottom);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelCropBottom>%lu</PixelCropBottom>\n", PixelCropBottom);
                break;
            case eid_PixelCropTop:
                PixelCropTop = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelCropTop      = %llu", PixelCropTop);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelCropTop>%lu</PixelCropTop>\n", PixelCropTop);
                break;
            case eid_PixelCropLeft:
                PixelCropLeft = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelCropLeft = %llu", PixelCropLeft);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelCropLeft>%lu</PixelCropLeft>\n", PixelCropLeft);
                break;
            case eid_PixelCropRight:
                PixelCropRight = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* PixelCropRight      = %llu", PixelCropRight);
                if (mkv->xml) fprintf(mkv->xml, "  <PixelCropRight>%lu</PixelCropRight>\n", PixelCropRight);
                break;
            case eid_DisplayWidth:
                DisplayWidth = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* DisplayWidth      = %llu", DisplayWidth);
                if (mkv->xml) fprintf(mkv->xml, "  <DisplayWidth>%lu</DisplayWidth>\n", DisplayWidth);
                break;
            case eid_DisplayHeight:
                DisplayHeight = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* DisplayHeight      = %llu", DisplayHeight);
                if (mkv->xml) fprintf(mkv->xml, "  <DisplayHeight>%lu</DisplayHeight>\n", DisplayHeight);
                break;
            case eid_DisplayUnit:
                DisplayUnit = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* DisplayUnit      = %llu", DisplayUnit);
                if (mkv->xml) fprintf(mkv->xml, "  <DisplayUnit>%lu</DisplayUnit>\n", DisplayUnit);
                break;
            case eid_AspectRatioType:
                AspectRatioType = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* AspectRatioType      = %llu", AspectRatioType);
                if (mkv->xml) fprintf(mkv->xml, "  <AspectRatioType>%lu</AspectRatioType>\n", AspectRatioType);
                break;
            case eid_ColourSpace:
                ColourSpace = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "ColourSpace      = '%s'", ColourSpace);
                fprintf(mkv->xml, "  <ColourSpace>%s</ColourSpace>\n", ColourSpace);
                break;

            case eid_Colour:
                retcode = mkv_parse_track_entry_video_colour(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    free(ColourSpace);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_track_entry_audio(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_audio()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml, "Audio");

    double SamplingFrequency = 0;
    double OutputSamplingFrequency = 0;
    uint64_t Channels = 0;
    uint64_t BitDepth = 0;

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
                SamplingFrequency = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* SamplingFrequency = %f", SamplingFrequency);
                if (mkv->xml) fprintf(mkv->xml, "  <SamplingFrequency>%f</SamplingFrequency>\n", SamplingFrequency);
                break;
            case eid_OutputSamplingFrequency:
                OutputSamplingFrequency = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* OutputSamplingFrequency = %f", OutputSamplingFrequency);
                if (mkv->xml) fprintf(mkv->xml, "  <OutputSamplingFrequency>%f</OutputSamplingFrequency>\n", OutputSamplingFrequency);
                break;
            case eid_Channels:
                Channels = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* Channels = %llu", Channels);
                if (mkv->xml) fprintf(mkv->xml, "  <Channels>%lu</Channels>\n", Channels);
                break;
            case eid_BitDepth:
                BitDepth = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* BitDepth = %llu", BitDepth);
                if (mkv->xml) fprintf(mkv->xml, "  <BitDepth>%lu</BitDepth>\n", BitDepth);
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

static int mkv_parse_track_entry_operation(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_operation()" CLR_RESET);
    int retcode = SUCCESS;

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

static int mkv_parse_track_entry_contentencoding(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry_contentencoding()" CLR_RESET);
    int retcode = SUCCESS;

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

static int mkv_parse_track_entry(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track_entry()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml, "Track Entry");

    uint64_t TrackNumber = 0;
    uint64_t TrackUID = 0;
    uint64_t TrackType = 0;
    uint64_t FlagEnabled = 0;
    uint64_t FlagDefault = 0;
    uint64_t FlagForced = 0;
    uint64_t FlagLacing = 0;
    uint64_t MinCache = 0;
    uint64_t MaxCache = 0;
    uint64_t DefaultDuration = 0;
    uint64_t DefaultDecodedFieldDuration = 0;
    uint64_t MaxBlockAdditionID = 0;
    uint8_t *Name = NULL;
    uint8_t *Language = NULL;
    uint8_t *CodecID = NULL;
    uint8_t *CodecPrivate = NULL;
    uint8_t *CodecName = NULL;
    uint64_t AttachmentLink = 0;
    uint64_t CodecDecodeAll = 0;
    uint64_t TrackOverlay = 0;
    uint64_t CodecDelay = 0;
    uint64_t SeekPreRoll = 0;

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
                TrackNumber = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackNumber   = %llu", TrackNumber);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackNumber>%lu</TrackNumber>\n", TrackNumber);
                break;
            case eid_TrackUID:
                TrackUID = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackUID   = %llu", TrackUID);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackUID>%lu</TrackUID>\n", TrackUID);
                break;
            case eid_TrackType:
                TrackType = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackType   = %llu", TrackType);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackType>%lu</TrackType>\n", TrackType);
                break;
            case eid_FlagEnabled:
                FlagEnabled = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FlagEnabled   = %llu", FlagEnabled);
                if (mkv->xml) fprintf(mkv->xml, "  <FlagEnabled>%lu</FlagEnabled>\n", FlagEnabled);
                break;
            case eid_FlagDefault:
                FlagDefault = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FlagDefault   = %llu", FlagDefault);
                if (mkv->xml) fprintf(mkv->xml, "  <FlagDefault>%lu</FlagDefault>\n", FlagDefault);
                break;
            case eid_FlagForced:
                FlagForced = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FlagForced   = %llu", FlagForced);
                if (mkv->xml) fprintf(mkv->xml, "  <FlagForced>%lu</FlagForced>\n", FlagForced);
                break;
            case eid_FlagLacing:
                FlagLacing = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* FlagLacing   = %llu", FlagLacing);
                if (mkv->xml) fprintf(mkv->xml, "  <FlagLacing>%lu</FlagLacing>\n", FlagLacing);
                break;
            case eid_MinCache:
                MinCache = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* MinCache   = %llu", MinCache);
                if (mkv->xml) fprintf(mkv->xml, "  <MinCache>%lu</MinCache>\n", MinCache);
                break;
            case eid_MaxCache:
                MaxCache = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* MaxCache   = %llu", MaxCache);
                if (mkv->xml) fprintf(mkv->xml, "  <MaxCache>%lu</MaxCache>\n", MaxCache);
                break;
            case eid_DefaultDuration:
                DefaultDuration = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* DefaultDuration   = %llu", DefaultDuration);
                if (mkv->xml) fprintf(mkv->xml, "  <DefaultDuration>%lu</DefaultDuration>\n", DefaultDuration);
                break;
            case eid_DefaultDecodedFieldDuration:
                DefaultDecodedFieldDuration = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* DefaultDecodedFieldDuration   = %llu", DefaultDecodedFieldDuration);
                if (mkv->xml) fprintf(mkv->xml, "  <DefaultDecodedFieldDuration>%lu</DefaultDecodedFieldDuration>\n", DefaultDecodedFieldDuration);
                break;
            case eid_MaxBlockAdditionID:
                MaxBlockAdditionID = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* MaxBlockAdditionID   = %llu", MaxBlockAdditionID);
                if (mkv->xml) fprintf(mkv->xml, "  <MaxBlockAdditionID>%lu</MaxBlockAdditionID>\n", MaxBlockAdditionID);
                break;
            case eid_Name:
                Name = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "Name        = '%s'", Name);
                fprintf(mkv->xml, "  <Name>%s</Name>\n", Name);
                break;
            case eid_Language:
                Language = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "Language        = '%s'", Language);
                fprintf(mkv->xml, "  <Language>%s</Language>\n", Language);
                break;
            case eid_CodecID:
                CodecID = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "CodecID        = '%s'", CodecID);
                fprintf(mkv->xml, "  <CodecID>%s</CodecID>\n", CodecID);
                break;
            case eid_CodecPrivate:
                CodecPrivate = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "CodecPrivate        = '%s'", CodecPrivate);
                fprintf(mkv->xml, "  <CodecPrivate>%s</CodecPrivate>\n", CodecPrivate);
                break;
            case eid_CodecName:
                CodecName = read_ebml_data_string(bitstr, element_sub.size);
                TRACE_1(MKV, "CodecName        = '%s'", CodecName);
                fprintf(mkv->xml, "  <CodecName>%s</CodecName>\n", CodecName);
                break;
            case eid_CodecDecodeAll:
                CodecDecodeAll = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* CodecDecodeAll   = %llu", CodecDecodeAll);
                if (mkv->xml) fprintf(mkv->xml, "  <CodecDecodeAll>%lu</CodecDecodeAll>\n", CodecDecodeAll);
                break;
            case eid_TrackOverlay:
                TrackOverlay = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* TrackOverlay   = %llu", TrackOverlay);
                if (mkv->xml) fprintf(mkv->xml, "  <TrackOverlay>%lu</TrackOverlay>\n", TrackOverlay);
                break;
            case eid_AttachmentLink:
                AttachmentLink = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* AttachmentLink   = %llu", AttachmentLink);
                if (mkv->xml) fprintf(mkv->xml, "  <AttachmentLink>%lu</AttachmentLink>\n", AttachmentLink);
                break;
            case eid_CodecDelay:
                CodecDelay = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* CodecDelay   = %llu", CodecDelay);
                if (mkv->xml) fprintf(mkv->xml, "  <CodecDelay>%lu</CodecDelay>\n", CodecDelay);
                break;
            case eid_SeekPreRoll:
                SeekPreRoll = read_bits_64(bitstr, element_sub.size*8);
                TRACE_1(MKV, "* SeekPreRoll   = %llu", SeekPreRoll);
                if (mkv->xml) fprintf(mkv->xml, "  <SeekPreRoll>%lu</SeekPreRoll>\n", SeekPreRoll);
                break;

            case eid_TrackTranslate:
                retcode = mkv_parse_track_entry_translate(bitstr, &element_sub, mkv);
                break;
            case eid_Video:
                retcode = mkv_parse_track_entry_video(bitstr, &element_sub, mkv);
                break;
            case eid_Audio:
                retcode = mkv_parse_track_entry_audio(bitstr, &element_sub, mkv);
                break;
            case eid_TrackOperation:
                retcode = mkv_parse_track_entry_operation(bitstr, &element_sub, mkv);
                break;
            case eid_ContentEncodings:
                retcode = mkv_parse_track_entry_contentencoding(bitstr, &element_sub, mkv);
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </atom>\n");

    free(Name);
    free(Language);
    free(CodecID);
    free(CodecPrivate);
    free(CodecName);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int mkv_parse_track(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_track()" CLR_RESET);
    int retcode = SUCCESS;

    write_ebml_element(element, mkv->xml, "Track");

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
                retcode = mkv_parse_track_entry(bitstr, &element_sub, mkv);
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
