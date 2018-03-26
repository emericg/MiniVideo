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
 * \file      mkv_tracks.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_tracks.h"
#include "mkv_codec.h"
#include "mkv_struct.h"
#include "ebml.h"

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
                TrackTranslateEditionUID = read_ebml_data_uint_UID(bitstr, &element_sub, mkv->xml, "TrackTranslateEditionUID");
                break;
            case eid_TrackTranslateCodec:
                TrackTranslateCodec = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TrackTranslateCodec");
                break;
            case eid_TrackTranslateTrackID:
                TrackTranslateTrackID = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "TrackTranslateTrackID");
                break;

            default:
                retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                break;
            }

            retcode = jumpy_mkv(bitstr, element, &element_sub);
        }
    }

    if (mkv->xml) fprintf(mkv->xml, "  </a>\n");

    free(TrackTranslateTrackID);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int mkv_parse_tracks_entry_video_colour_mastering(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video_colour_mastering()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Mastering Metadata");

    track->video->Colour->MasteringMetadata = new mkv_track_video_colour_mastering_t;
    if (track->video->Colour->MasteringMetadata == NULL)
        retcode = FAILURE;
    mkv_track_video_colour_mastering_t *mastering = track->video->Colour->MasteringMetadata;

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
            case eid_PrimaryRChromaticityX:
                mastering->PrimaryRChromaticityX = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryRChromaticityX");
                break;
            case eid_PrimaryRChromaticityY:
                mastering->PrimaryRChromaticityY = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryRChromaticityY");
                break;
            case eid_PrimaryGChromaticityX:
                mastering->PrimaryGChromaticityX = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryGChromaticityX");
                break;
            case eid_PrimaryGChromaticityY:
                mastering->PrimaryGChromaticityY = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryGChromaticityY");
                break;
            case eid_PrimaryBChromaticityX:
                mastering->PrimaryBChromaticityX = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryBChromaticityX");
                break;
            case eid_PrimaryBChromaticityY:
                mastering->PrimaryBChromaticityY = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "PrimaryBChromaticityY");
                break;
            case eid_WhitePointChromaticityX:
                mastering->WhitePointChromaticityX = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "WhitePointChromaticityX");
                break;
            case eid_WhitePointChromaticityY:
                mastering->WhitePointChromaticityY = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "WhitePointChromaticityY");
                break;
            case eid_LuminanceMax:
                mastering->LuminanceMax = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "LuminanceMax");
                break;
            case eid_LuminanceMin:
                mastering->LuminanceMin = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "LuminanceMin");
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

/*!
 * \brief Google spatial media support (video v2)
 * \param bitstr
 * \param element
 * \param mkv
 * \param track
 * \return
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
static int mkv_parse_tracks_entry_video_projection(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video_projection()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Projection");

    track->video->Projection = new mkv_track_video_projection_t;
    if (track->video->Projection == NULL)
        retcode = FAILURE;
    mkv_track_video_projection_t *projection = track->video->Projection;

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
            case eid_spatial_ProjectionType:
                projection->ProjectionType = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ProjectionType");
                break;
            case eid_spatial_ProjectionPrivate:
                projection->ProjectionPrivate = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ProjectionPrivate");
                if (projection->ProjectionPrivate) projection->ProjectionPrivate_size = element_sub.size;
                break;
            case eid_spatial_ProjectionPoseYaw:
                projection->ProjectionPoseYaw = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ProjectionPoseYaw");
                break;
            case eid_spatial_ProjectionPosePitch:
                projection->ProjectionPosePitch = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ProjectionPosePitch");
                break;
            case eid_spatial_ProjectionPoseRoll:
                projection->ProjectionPoseRoll = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ProjectionPoseRoll");
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

static int mkv_parse_tracks_entry_video_colour(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video_colour()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Colour");

    track->video->Colour = new mkv_track_video_colour_t;
    if (track->video->Colour == NULL)
        retcode = FAILURE;
    mkv_track_video_colour_t *colour = track->video->Colour;

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
                colour->MatrixCoefficients = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MatrixCoefficients");
                break;
            case eid_BitsPerChannel:
                colour->BitsPerChannel = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "BitsPerChannel");
                break;
            case eid_ChromaSubsamplingHorz:
                colour->ChromaSubsamplingHorz = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChromaSubsamplingHorz");
                break;
            case eid_ChromaSubsamplingVert:
                colour->ChromaSubsamplingVert = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChromaSubsamplingVert");
                break;
            case eid_CbSubsamplingHorz:
                colour->CbSubsamplingHorz = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CbSubsamplingHorz");
                break;
            case eid_CbSubsamplingVert:
                colour->CbSubsamplingVert = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CbSubsamplingVert");
                break;
            case eid_ChromaSitingHorz:
                colour->ChromaSitingHorz = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChromaSitingHorz");
                break;
            case eid_ChromaSitingVert:
                colour->ChromaSitingVert = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ChromaSitingVert");
                break;
            case eid_Range:
                colour->Range = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "Range");
                break;
            case eid_TransferCharacteristics:
                colour->TransferCharacteristics = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TransferCharacteristics");
                break;
            case eid_Primaries:
                colour->Primaries = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "Primaries");
                break;
            case eid_MaxCLL:
                colour->MaxCLL = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MaxCLL");
                break;
            case eid_MaxFALL:
                colour->MaxFALL = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MaxFALL");
                break;

            case eid_MasteringMetadata:
                retcode = mkv_parse_tracks_entry_video_colour_mastering(bitstr, element, mkv, track);
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

static int mkv_parse_tracks_entry_video(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_video()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Video");

    track->video = new mkv_track_video_t;
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
                track->video->FlagInterlaced = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FlagInterlaced");
                break;
            case eid_FieldOrder:
                track->video->FieldOrder = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FieldOrder");
                break;
            case eid_StereoMode:
                track->video->StereoMode = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "StereoMode");
                break;
            case eid_AlphaMode:
                track->video->AlphaMode = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "AlphaMode");
                break;
            case eid_PixelWidth:
                track->video->PixelWidth = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelWidth");
                break;
            case eid_PixelHeight:
                track->video->PixelHeight = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelHeight");
                break;
            case eid_PixelCropBottom:
                track->video->PixelCropBottom = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelCropBottom");
                break;
            case eid_PixelCropTop:
                track->video->PixelCropTop = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelCropTop");
                break;
            case eid_PixelCropLeft:
                track->video->PixelCropLeft = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelCropLeft");
                break;
            case eid_PixelCropRight:
                track->video->PixelCropRight = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "PixelCropRight");
                break;
            case eid_DisplayWidth:
                track->video->DisplayWidth = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DisplayWidth");
                break;
            case eid_DisplayHeight:
                track->video->DisplayHeight = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DisplayHeight");
                break;
            case eid_DisplayUnit:
                track->video->DisplayUnit = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DisplayUnit");
                break;
            case eid_AspectRatioType:
                track->video->AspectRatioType = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "AspectRatioType");
                break;
            case eid_ColourSpace:
                track->video->ColourSpace = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "ColourSpace");
                if (track->video->ColourSpace) track->video->ColourSpace_size = element_sub.size;
                break;

            case eid_spatial_Projection:
                retcode = mkv_parse_tracks_entry_video_projection(bitstr, &element_sub, mkv, track);
                break;

            case eid_Colour:
                retcode = mkv_parse_tracks_entry_video_colour(bitstr, &element_sub, mkv, track);
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

static int mkv_parse_tracks_entry_audio(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_audio()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Audio");

    track->audio = new mkv_track_audio_t;
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
                track->audio->SamplingFrequency = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "SamplingFrequency");
                break;
            case eid_OutputSamplingFrequency:
                track->audio->OutputSamplingFrequency = read_ebml_data_float(bitstr, &element_sub, mkv->xml, "OutputSamplingFrequency");
                break;
            case eid_Channels:
                track->audio->Channels = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "Channels");
                break;
            case eid_BitDepth:
                track->audio->BitDepth = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "BitDepth");
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

static int mkv_parse_tracks_entry_operation(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_operation()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Entry Operation");

    track->operation = new mkv_track_operation_t;
    if (track->operation == NULL)
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

static int mkv_parse_tracks_entry_contentencoding(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_contentencoding()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Content Encoding");

    track->encodings->encoding = new mkv_track_encoding_t;
    if (track->encodings->encoding == NULL)
        retcode = FAILURE;
    mkv_track_encoding_t *encoding = track->encodings->encoding;

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
            case eid_ContentEncodingOrder:
                encoding->ContentEncodingOrder = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ContentEncodingOrder");
                break;
            case eid_ContentEncodingScope:
                encoding->ContentEncodingScope = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ContentEncodingScope");
                break;
            case eid_ContentEncodingType:
                encoding->ContentEncodingType = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "ContentEncodingType");
                break;

            case eid_ContentCompression:
            {
                while (mkv->run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < element_sub.offset_end)
                {
                    // Parse sub element
                    EbmlElement_t element_subsub;
                    retcode = parse_ebml_element(bitstr, &element_subsub);

                    // Then parse subbox content
                    if (mkv->run == true && retcode == SUCCESS)
                    {
                        switch (element_subsub.eid)
                        {
                        case eid_ContentCompAlgo:
                            encoding->ContentCompAlgo = read_ebml_data_uint(bitstr, &element_subsub, mkv->xml, "ContentCompAlgo");
                            break;
                        case eid_ContentCompSettings:
                            encoding->ContentCompSettings = read_ebml_data_binary(bitstr, &element_subsub, mkv->xml, "ContentCompSettings");
                            if (encoding->ContentCompSettings) encoding->ContentCompSettings_size = element_subsub.size;
                            break;

                        default:
                            retcode = ebml_parse_unknown(bitstr, &element_subsub, mkv->xml);
                            break;
                        }

                        retcode = jumpy_mkv(bitstr, &element_sub, &element_subsub);
                    }
                }
            } break;

            case eid_ContentEncryption:
            {
                while (mkv->run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < element_sub.offset_end)
                {
                    // Parse sub element
                    EbmlElement_t element_subsub;
                    retcode = parse_ebml_element(bitstr, &element_subsub);

                    // Then parse subbox content
                    if (mkv->run == true && retcode == SUCCESS)
                    {
                        switch (element_subsub.eid)
                        {
                        case eid_ContentEncAlgo:
                            encoding->ContentEncAlgo = read_ebml_data_uint(bitstr, &element_subsub, mkv->xml, "ContentEncAlgo");
                            break;
                        case eid_ContentEncKeyID:
                            encoding->ContentEncKeyID = read_ebml_data_binary(bitstr, &element_subsub, mkv->xml, "ContentEncKeyID");
                            if (encoding->ContentEncKeyID) encoding->ContentEncKeyID_size = element_subsub.size;
                            break;
                        case eid_ContentSignature:
                            encoding->ContentSignature = read_ebml_data_binary(bitstr, &element_subsub, mkv->xml, "ContentSignature");
                            if (encoding->ContentSignature) encoding->ContentSignature_size = element_subsub.size;
                            break;
                        case eid_ContentSigKeyID:
                            encoding->ContentSigKeyID = read_ebml_data_binary(bitstr, &element_subsub, mkv->xml, "ContentSigKeyID");
                            if (encoding->ContentSigKeyID) encoding->ContentSigKeyID_size = element_subsub.size;
                            break;
                        case eid_ContentSigAlgo:
                            encoding->ContentSigAlgo = read_ebml_data_uint(bitstr, &element_subsub, mkv->xml, "ContentSigAlgo");
                            break;
                        case eid_ContentSigHashAlgo:
                            encoding->ContentSigHashAlgo = read_ebml_data_uint(bitstr, &element_subsub, mkv->xml, "ContentSigHashAlgo");
                            break;

                        default:
                            retcode = ebml_parse_unknown(bitstr, &element_subsub, mkv->xml);
                            break;
                        }

                        retcode = jumpy_mkv(bitstr, &element_sub, &element_subsub);
                    }
                }
            } break;

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

static int mkv_parse_tracks_entry_contentencodings(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry_contentencodings()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Content Encodings");

    track->encodings = new mkv_track_encodings_t;
    if (track->encodings == NULL)
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
            case eid_ContentEncoding:
                retcode = mkv_parse_tracks_entry_contentencoding(bitstr, &element_sub, mkv, track);
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

static int mkv_parse_tracks_entry(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv, mkv_track_t *mkv_track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_parse_tracks_entry()" CLR_RESET);
    int retcode = SUCCESS;

    print_ebml_element(element);
    write_ebml_element(element, mkv->xml, "Track Entry", "track");

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
                mkv_track->TrackNumber = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TrackNumber");
                break;
            case eid_TrackUID:
                mkv_track->TrackUID = read_ebml_data_uint_UID(bitstr, &element_sub, mkv->xml, "TrackUID");
                break;
            case eid_TrackType:
                mkv_track->TrackType = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TrackType");
                break;
            case eid_FlagEnabled:
                mkv_track->FlagEnabled = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FlagEnabled");
                break;
            case eid_FlagDefault:
                mkv_track->FlagDefault = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FlagDefault");
                break;
            case eid_FlagForced:
                mkv_track->FlagForced = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FlagForced");
                break;
            case eid_FlagLacing:
                mkv_track->FlagLacing = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "FlagLacing");
                break;
            case eid_MinCache:
                mkv_track->MinCache = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MinCache");
                break;
            case eid_MaxCache:
                mkv_track->MaxCache = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MaxCache");
                break;
            case eid_DefaultDuration:
                mkv_track->DefaultDuration = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DefaultDuration");
                break;
            case eid_DefaultDecodedFieldDuration:
                mkv_track->DefaultDecodedFieldDuration = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "DefaultDecodedFieldDuration");
                break;
            case eid_TrackTimecodeScale:
                mkv_track->TrackTimecodeScale = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TrackTimecodeScale");
                break;
            case eid_MaxBlockAdditionID:
                mkv_track->MaxBlockAdditionID = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "MaxBlockAdditionID");
                break;
            case eid_Name:
                mkv_track->Name = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "Name");
                break;
            case eid_Language:
                mkv_track->Language = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "Language");
                break;
            case eid_CodecID:
                mkv_track->CodecID = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "CodecID");
                break;
            case eid_CodecPrivate:
                mkv_track->CodecPrivate = read_ebml_data_binary(bitstr, &element_sub, mkv->xml, "CodecPrivate");
                if (mkv_track->CodecPrivate)
                {
                    mkv_track->CodecPrivate_offset = bitstream_get_absolute_byte_offset(bitstr) - element_sub.size;
                    mkv_track->CodecPrivate_size = element_sub.size;

                    if (mkv_track->CodecPrivate_offset && mkv_track->CodecPrivate_size)
                    {
                        int64_t saveOffset = bitstream_get_absolute_byte_offset(bitstr);

                        bitstream_goto_offset(bitstr, mkv_track->CodecPrivate_offset);

                        if (mkv_track->CodecID && strcmp(mkv_track->CodecID, "V_MPEG4/ISO/AVC") == 0)
                        {
                            parse_h264_private(bitstr, mkv_track, mkv);
                        }

                        bitstream_goto_offset(bitstr, saveOffset);
                    }
                }
                break;
            case eid_CodecName:
                mkv_track->CodecName = read_ebml_data_string(bitstr, &element_sub, mkv->xml, "CodecName");
                break;
            case eid_CodecDecodeAll:
                mkv_track->CodecDecodeAll = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CodecDecodeAll");
                break;
            case eid_TrackOverlay:
                mkv_track->TrackOverlay = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "TrackOverlay");
                break;
            case eid_AttachmentLink:
                mkv_track->AttachmentLink = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "AttachmentLink");
                break;
            case eid_CodecDelay:
                mkv_track->CodecDelay = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "CodecDelay");
                break;
            case eid_SeekPreRoll:
                mkv_track->SeekPreRoll = read_ebml_data_uint(bitstr, &element_sub, mkv->xml, "SeekPreRoll");
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
                retcode = mkv_parse_tracks_entry_contentencodings(bitstr, &element_sub, mkv, mkv_track);
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
            {
                mkv->tracks[mkv->tracks_count] = new mkv_track_t;
                if (mkv->tracks[mkv->tracks_count])
                    retcode = mkv_parse_tracks_entry(bitstr, &element_sub, mkv, mkv->tracks[mkv->tracks_count]);
                else
                    retcode = ebml_parse_unknown(bitstr, &element_sub, mkv->xml);
                mkv->tracks_count++;
            } break;

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
