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
 * \file      mkv_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_convert.h"
#include "mkv_struct.h"
#include "ebml.h"

#include "mkv_tracks.h"

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

void mkv_codec(char *codec_str, Codecs_e *codec, CodecProfiles_e *profile)
{
    if (!codec_str || !codec || !profile)
        return;

    *codec = CODEC_UNKNOWN;
    *profile = PROF_UNKNOWN;

    if (strncmp(codec_str, "A_", 2) == 0)
    {
        if (strncmp(codec_str, "A_AAC", 5) == 0)
        {
            *codec = CODEC_AAC;
/*
            A_AAC/MPEG2/MAIN
            A_AAC/MPEG2/LC
            A_AAC/MPEG2/LC/SBR
            A_AAC/MPEG2/SSR
            A_AAC/MPEG4/MAIN
            A_AAC/MPEG4/LC
            A_AAC/MPEG4/LC/SBR
            A_AAC/MPEG4/SSR
            A_AAC/MPEG4/LTP
*/
        }
        else if (strncmp(codec_str, "A_MPEG", 6) == 0)
        {
            if (strcmp(codec_str, "A_MPEG/L3") == 0)
            {
                *codec = CODEC_MPEG_L3;
            }
            else if (strcmp(codec_str, "A_MPEG/L2") == 0)
            {
                *codec = CODEC_MPEG_L2;
            }
            else if (strcmp(codec_str, "A_MPEG/L1") == 0)
            {
                *codec = CODEC_MPEG_L1;
            }
        }
        else if (strncmp(codec_str, "A_PCM", 5) == 0)
        {
            if (strcmp(codec_str, "A_PCM/INT/BIG") == 0)
            {
                *codec = CODEC_LPCM;
            }
            else if (strcmp(codec_str, "A_PCM/INT/LIT") == 0)
            {
                *codec = CODEC_LPCM;
            }
            else if (strcmp(codec_str, "A_PCM/FLOAT/IEEE") == 0)
            {
                *codec = CODEC_LPCM;
            }
        }
        else if (strcmp(codec_str, "A_MPC") == 0)
        {
            *codec = CODEC_MPC;
        }
        else if (strcmp(codec_str, "A_AC3") == 0)
        {
            //A_AC3/BSID9
            //A_AC3/BSID10
            *codec = CODEC_AC3;
        }
        else if (strcmp(codec_str, "A_AC4") == 0)
        {
            *codec = CODEC_AC4;
        }
        else if (strcmp(codec_str, "A_ALAC") == 0)
        {
            *codec = CODEC_ALAC;
        }
        else if (strcmp(codec_str, "A_DTS") == 0)
        {
            //A_DTS/EXPRESS
            //A_DTS/LOSSLESS
            *codec = CODEC_DTS;
        }
        else if (strcmp(codec_str, "A_VORBIS") == 0)
        {
            *codec = CODEC_VORBIS;
        }
        else if (strcmp(codec_str, "A_OPUS") == 0)
        {
            *codec = CODEC_OPUS;
        }
        else if (strcmp(codec_str, "A_FLAC") == 0)
        {
            *codec = CODEC_FLAC;
        }
        else if (strncmp(codec_str, "A_REAL", 6) == 0)
        {
            if (strcmp(codec_str, "A_REAL/14_4") == 0)
            {
                *codec = CODEC_RA_14;
            }
            else if (strcmp(codec_str, "A_REAL/28_8") == 0)
            {
                *codec = CODEC_RA_28;
            }
            else if (strcmp(codec_str, "A_REAL/SIPR") == 0)
            {
                *codec = CODEC_UNKNOWN; // Sipro Voice Codec
            }
            else if (strcmp(codec_str, "A_REAL/COOK") == 0)
            {
                *codec = CODEC_RA_cook;
            }
            else if (strcmp(codec_str, "A_REAL/RALF") == 0)
            {
                *codec = CODEC_RA_cook;
            }
            else if (strcmp(codec_str, "A_REAL/ATRC") == 0)
            {
                *codec = CODEC_ATRAC;
            }
        }
    }
    else if (strncmp(codec_str, "V_", 2) == 0)
    {
        // V_MS/VFW/FOURCC
        // V_UNCOMPRESSED
        // V_QUICKTIME

        if (strncmp(codec_str, "V_MPEG4/ISO", 11) == 0)
        {
            if (strcmp(codec_str, "V_MPEG4/ISO/AVC") == 0)
            {
                *codec = CODEC_H264;
            }
            else if (strcmp(codec_str, "V_MPEG4/ISO/SP") == 0)
            {
                *codec = CODEC_MPEG4_ASP;
                *profile = PROF_MPEG4_SP;
            }
            else if (strcmp(codec_str, "V_MPEG4/ISO/ASP") == 0)
            {
                *codec = CODEC_MPEG4_ASP;
                *profile = PROF_MPEG4_ASP;
            }
            else if (strcmp(codec_str, "V_MPEG4/ISO/AP") == 0)
            {
                *codec = CODEC_MPEG4_ASP;
                *profile = PROF_MPEG4_AP;
            }
        }
        else if (strcmp(codec_str, "V_MPEGH/ISO/HEVC") == 0)
        {
            *codec = CODEC_H265;
        }
        else if (strncmp(codec_str, "V_VP", 4) == 0)
        {
            if (strcmp(codec_str, "V_VP9") == 0)
            {
                *codec = CODEC_VP9;
            }
            else if (strcmp(codec_str, "V_VP8") == 0)
            {
                *codec = CODEC_VP8;
            }
            else if (strcmp(codec_str, "V_VP7") == 0)
            {
                *codec = CODEC_VP7;
            }
            else if (strcmp(codec_str, "V_VP6") == 0)
            {
                *codec = CODEC_VP6;
            }
        }
        else if (strcmp(codec_str, "V_MPEG2") == 0)
        {
            *codec = CODEC_MPEG2;
        }
        else if (strcmp(codec_str, "V_MPEG1") == 0)
        {
            *codec = CODEC_MPEG1;
        }
        else if (strcmp(codec_str, "V_MPEG4/MS/V3") == 0)
        {
            *codec = CODEC_MSMPEG4;
        }
        else if (strcmp(codec_str, "V_REAL/RV10") == 0)
        {
            *codec = CODEC_RV10;
        }
        else if (strcmp(codec_str, "V_REAL/RV20") == 0)
        {
            *codec = CODEC_RV20;
        }
        else if (strcmp(codec_str, "V_REAL/RV30") == 0)
        {
            *codec = CODEC_RV30;
        }
        else if (strcmp(codec_str, "V_REAL/RV40") == 0)
        {
            *codec = CODEC_RV40;
        }
        else if (strcmp(codec_str, "V_THEORA") == 0)
        {
            *codec = CODEC_VP4;
        }
        else if (strcmp(codec_str, "V_PRORES") == 0)
        {
            *codec = CODEC_PRORES_422;
        }
    }
    else if (strncmp(codec_str, "S_", 2) == 0)
    {
        if (strcmp(codec_str, "S_TEXT/UTF8") == 0)
        {
            *codec = CODEC_SRT;
        }
        else if (strcmp(codec_str, "S_TEXT/SSA") == 0)
        {
            *codec = CODEC_SSA;
        }
        else if (strcmp(codec_str, "S_TEXT/ASS") == 0)
        {
            *codec = CODEC_ASS;
        }
        else if (strcmp(codec_str, "S_TEXT/USF") == 0)
        {
            *codec = CODEC_USF;
        }
        else if (strcmp(codec_str, "S_TEXT/WEBVTT") == 0)
        {
            *codec = CODEC_WebVTT;
        }
        else if (strcmp(codec_str, "S_VOBSUB") == 0)
        {
            *codec = CODEC_VobSub;
        }
/*
        S_IMAGE/BMP
        S_KATE
*/
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

int mkv_convert(MediaFile_t *media, mkv_t *mkv)
{
    int status = SUCCESS;

    // File metadatas
    media->container_profile = mkv->profile;
    if (mkv->info.WritingApp)
    {
        media->creation_app = strdup(mkv->info.WritingApp);
    }

    media->duration = (uint64_t)(mkv->info.Duration / ((double)mkv->info.TimecodeScale / 1000000.0));
    //mkv->info.DateUTC

    // Tracks metadatas
    if (mkv->tracks_count == 0) // Check if we have extracted tracks
    {
        TRACE_WARNING(MKV, "No tracks extracted from MKV file!");
    }
    else // Convert tracks
    {
        for (int i = 0; i < mkv->tracks_count; i++)
        {
            if (mkv->tracks[i])
            {
                status = mkv_convert_track(media, mkv, mkv->tracks[i]);
            }
        }

        if (media->tracks_video_count == 0 && media->tracks_audio_count == 0)
        {
            TRACE_WARNING(MKV, "No tracks converted successfully!");
        }
    }

    mkv_clean(mkv);

    return status;
}

/* ************************************************************************** */

int mkv_convert_track(MediaFile_t *media, mkv_t *mkv, mkv_track_t *track)
{
    TRACE_INFO(MKV, BLD_GREEN "mkv_convert_track()" CLR_RESET);
    int retcode = SUCCESS;
    MediaStream_t *map = NULL;

    if (media == NULL || track == NULL)
    {
        TRACE_ERROR(MKV, "Cannot access audio or video tracks from the MP4 parser!");
        retcode = FAILURE;
    }

    // Select and init a bitstream map (A or V)
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS)
    {
        int sample_count = vector_count(&track->sample_vector);
        if (sample_count <= 0)
            sample_count = 1;

        if (track->audio)
        {
            retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_audio[media->tracks_audio_count];
                media->tracks_audio_count++;
            }
        }
        else if (track->video)
        {
            retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_video[media->tracks_video_count];
                media->tracks_video_count++;
            }
        }
        else
        {
            //char fcc[5];
            //TRACE_1(MKV, "Not sure we can build bitstream_map for other track types! (track #%u handlerType: %s)",
            //        track->id, getFccString_le(track->handlerType, fcc));

            retcode = init_bitstream_map(&media->tracks_others[media->tracks_others_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_others[media->tracks_others_count];
                media->tracks_others_count++;
            }
        }
    }

    // Build bitstream map
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS && map)
    {
        // Track parameters

        mkv_codec(track->CodecID, &map->stream_codec, &map->stream_codec_profile);

        if (track->Name && strnlen(track->Name, 256) > 0)
        {
                map->track_title = (char *)malloc(sizeof(track->Name));
                strcpy(map->track_title, track->Name);
        }

        if (track->Language && strlen(track->Language) > 0)
        {
           map->track_languagecode = (char *)malloc(4);
           strncpy(map->track_languagecode, track->Language, 3);
           map->track_languagecode[3] = '\0';
        }

        map->track_id = track->TrackNumber;

        if (track->DefaultDuration > 0)
            map->frame_duration = (double)(track->DefaultDuration) / 1000000.0;

        if (track->TrackType == MKV_TRACK_VIDEO)
        {
            map->stream_type = stream_VIDEO;
            map->width = track->video->PixelWidth;
            map->height = track->video->PixelHeight;
            map->visible_width = track->video->DisplayWidth;
            map->visible_height = track->video->DisplayHeight;

            map->framerate = 1000000000.0 / track->DefaultDuration;
        }
        else if (track->TrackType == MKV_TRACK_AUDIO)
        {
            map->stream_type = stream_AUDIO;
            map->sampling_rate = track->audio->SamplingFrequency;
            map->channel_count = track->audio->Channels;
            map->bit_per_sample = track->audio->BitDepth;
        }
        else if (track->TrackType == MKV_TRACK_SUBTITLES)
        {
            map->stream_type = stream_TEXT;
        }
        else
        {
            map->stream_type = stream_UNKNOWN;
        }

        // Track samples

        map->sample_count = vector_count(&track->sample_vector);
        //TRACE_1(MKV, "sample_count: %i", sample_count);

        for (unsigned sid = 0; sid < map->sample_count; sid++)
        {
            mkv_sample_t *s = vector_get(&track->sample_vector, sid);

            if (track->TrackType == MKV_TRACK_VIDEO)
                map->sample_type[sid] = sample_VIDEO;
            else if (track->TrackType == MKV_TRACK_AUDIO)
                map->sample_type[sid] = sample_AUDIO;
            else if (track->TrackType == MKV_TRACK_SUBTITLES)
                map->sample_type[sid] = sample_TEXT;

            map->sample_offset[sid] = s->offset;
            map->sample_size[sid] = s->size;
            map->sample_pts[sid] = s->timecode * 1000;
        }
    }

    return retcode;
}

/* ************************************************************************** */

void mkv_clean(mkv_t *mkv)
{
    if (mkv)
    {
        // ebml_header_t
        free(mkv->ebml.DocType);

        // mkv_info_t
        free(mkv->info.SegmentUID);
        free(mkv->info.SegmentFilename);
        free(mkv->info.PrevUID);
        free(mkv->info.PrevFilename);
        free(mkv->info.NextUID);
        free(mkv->info.NextFilename);
        free(mkv->info.SegmentFamily);
        for (int i = 0; i < mkv->info.chapter_count; i++)
        {
            // mkv_info_chapter_t // TODO
            //free(mkv->info.chapter[i].ChapterTranslateID);
        }
        free(mkv->info.Title);
        free(mkv->info.MuxingApp);
        free(mkv->info.WritingApp);

        // mkv_tracks_t
        for (int i = 0; i < mkv->tracks_count; i++)
        {
            if (mkv->tracks[i])
            {
                free(mkv->tracks[i]->Name);
                free(mkv->tracks[i]->Language);
                free(mkv->tracks[i]->CodecID);
                free(mkv->tracks[i]->CodecPrivate);
                free(mkv->tracks[i]->CodecName);

                if (mkv->tracks[i]->video)
                {
                    if (mkv->tracks[i]->video->Colour)
                    {
                        free(mkv->tracks[i]->video->Colour->MasteringMetadata);
                        free(mkv->tracks[i]->video->Colour);
                    }
                    free(mkv->tracks[i]->video->ColourSpace);
                    free(mkv->tracks[i]->video);
                }
                if (mkv->tracks[i]->audio)
                {
                    free(mkv->tracks[i]->audio);
                }
                if (mkv->tracks[i]->translate)
                {
                    free(mkv->tracks[i]->translate);
                }
                if (mkv->tracks[i]->operation)
                {
                    free(mkv->tracks[i]->operation);
                }
                if (mkv->tracks[i]->encodings)
                {
                    free(mkv->tracks[i]->encodings);
                }

                int samplecount = vector_count(&mkv->tracks[i]->sample_vector);
                for (int j = 0; j < samplecount; j++)
                {
                    mkv_sample_t *s = vector_get(&mkv->tracks[i]->sample_vector, j);
                    free(s);
                }
                vector_free(&mkv->tracks[i]->sample_vector);

                free(mkv->tracks[i]);
            }
        }
    }
}

/* ************************************************************************** */
