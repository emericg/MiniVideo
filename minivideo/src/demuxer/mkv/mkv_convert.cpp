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
 * \file      mkv_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv_convert.h"
#include "mkv_codec.h"
#include "mkv_struct.h"
#include "ebml.h"

#include "mkv_tracks.h"

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

int mkv_convert(MediaFile_t *media, mkv_t *mkv)
{
    int status = SUCCESS;

    // File metadata
    media->container_profile = mkv->profile;
    if (mkv->info.WritingApp)
    {
        media->creation_app = strdup(mkv->info.WritingApp);
    }
    if (mkv->info.MuxingApp)
    {
        media->creation_lib = strdup(mkv->info.MuxingApp);
    }

    media->duration = (uint64_t)(mkv->info.Duration / ((double)mkv->info.TimecodeScale / 1000000.0));
    media->creation_time = EbmlTimeToUnixSeconds(mkv->info.DateUTC);

    // Tracks metadata
    if (mkv->tracks_count == 0) // Check if we have extracted tracks
    {
        TRACE_WARNING(MKV, "No tracks extracted from MKV file!");
    }
    else // Convert tracks
    {
        for (unsigned i = 0; i < mkv->tracks_count; i++)
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
        TRACE_ERROR(MKV, "Cannot access audio or video tracks from the MKV parser!");
        retcode = FAILURE;
    }

    // Select and init a bitstream map (A or V)
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS)
    {
        int sample_count = track->sample_vector.size();
        if (sample_count <= 0)
            sample_count = 1;

        if (track->TrackType == MKV_TRACK_AUDIO || track->audio)
        {
            retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], 0, sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_audio[media->tracks_audio_count];
                media->tracks_audio_count++;
            }
        }
        else if (track->TrackType == MKV_TRACK_VIDEO || track->video)
        {
            retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], track->sps_count + track->pps_count, sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_video[media->tracks_video_count];
                media->tracks_video_count++;
            }
        }
        else if (track->TrackType == MKV_TRACK_SUBTITLES)
        {
            retcode = init_bitstream_map(&media->tracks_subt[media->tracks_subtitles_count], 0, sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_subt[media->tracks_subtitles_count];
                media->tracks_subtitles_count++;
            }
        }
        else
        {
            //char fcc[5];
            //TRACE_1(MKV, "Not sure we can build bitstream_map for other track types! (track #%u handlerType: %s)",
            //        track->id, getFccString_le(track->handlerType, fcc));

            retcode = init_bitstream_map(&media->tracks_others[media->tracks_others_count], 0, sample_count);
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
            map->track_title = (char *)malloc(strnlen(track->Name, 256) + 1);
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
        {
            map->frame_duration = (double)(track->DefaultDuration) / 1000000.0;
            map->stream_duration_ms = map->frame_duration * track->sample_vector.size();
        }
        else
        {
            map->stream_duration_ms = media->duration;
        }

        if (track->TrackType == MKV_TRACK_VIDEO && track->video)
        {
            map->stream_type = stream_VIDEO;
            map->stream_packetized = true;

            map->width = track->video->PixelWidth;
            map->height = track->video->PixelHeight;
            map->visible_width = track->video->DisplayWidth;
            map->visible_height = track->video->DisplayHeight;
            map->color_depth = 8; // default
            if (track->video->Colour)
            {
                if (track->video->Colour->BitsPerChannel != 0)
                    map->color_depth = track->video->Colour->BitsPerChannel;
                map->color_range = track->video->Colour->Range;
            }
            map->color_depth *= 3;
            map->framerate = 1000000000.0 / track->DefaultDuration;

            if (track->video->StereoMode)
            {
                if (track->video->StereoMode == 1)
                    map->stereo_mode = STEREO_SIDEBYSIDE_LEFT;
                else if (track->video->StereoMode == 2)
                    map->stereo_mode = STEREO_TOPBOTTOM_RIGHT;
                else if (track->video->StereoMode == 3)
                    map->stereo_mode = STEREO_TOPBOTTOM_LEFT;
                else if (track->video->StereoMode == 4)
                    map->stereo_mode = STEREO_CHECKBOARD_RIGHT;
                else if (track->video->StereoMode == 5)
                    map->stereo_mode = STEREO_CHECKBOARD_LEFT;
                else if (track->video->StereoMode == 6)
                    map->stereo_mode = STEREO_ROWINTERLEAVED_RIGHT;
                else if (track->video->StereoMode == 7)
                    map->stereo_mode = STEREO_ROWINTERLEAVED_LEFT;
                else if (track->video->StereoMode == 8)
                    map->stereo_mode = STEREO_COLUMNINTERLEAVED_RIGHT;
                else if (track->video->StereoMode == 9)
                    map->stereo_mode = STEREO_COLUMNINTERLEAVED_LEFT;
                else if (track->video->StereoMode == 10)
                    map->stereo_mode = STEREO_ANAGLYPH_CR;
                else if (track->video->StereoMode == 11)
                    map->stereo_mode = STEREO_SIDEBYSIDE_RIGHT;
                else if (track->video->StereoMode == 12)
                    map->stereo_mode = STEREO_ANAGLYPH_GM;
            }

            if (track->video->Projection)
            {
                if (track->video->Projection->ProjectionType == 1)
                    map->video_projection = PROJECTION_EQUIRECTANGULAR;
                else if (track->video->Projection->ProjectionType == 2)
                    map->video_projection = PROJECTION_CUBEMAP_A;
                else if (track->video->Projection->ProjectionType == 3)
                    map->video_projection = PROJECTION_MESH;
            }

            // Codec specific metadata
            if (map->stream_codec == CODEC_H264 || map->stream_codec == CODEC_H265)
            {
                // Set SPS
                for (unsigned i = 0; i < track->sps_count; i++)
                {
                    map->parameter_type[i] = sample_VIDEO_PARAM;
                    map->parameter_offset[i] = track->sps_sample_offset[i];
                    map->parameter_size[i] = track->sps_sample_size[i];
                    map->parameter_count++;
                }
                // Set PPS
                for (unsigned i = 0; i < track->pps_count; i++)
                {
                    map->parameter_type[i + track->sps_count] = sample_VIDEO_PARAM;
                    map->parameter_offset[i + track->sps_count] = track->pps_sample_offset[i];
                    map->parameter_size[i + track->sps_count] = track->pps_sample_size[i];
                    map->parameter_count++;
                }
            }
        }
        else if (track->TrackType == MKV_TRACK_AUDIO && track->audio)
        {
            map->stream_type = stream_AUDIO;
            map->sampling_rate = track->audio->SamplingFrequency;
            map->channel_count = track->audio->Channels;
            if (track->audio->BitDepth)
                map->bit_per_sample = track->audio->BitDepth;
            else
                map->bit_per_sample = 16; // default
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
        map->sample_count = track->sample_vector.size();
        //TRACE_1(MKV, "sample_count: %i", sample_count);

        for (unsigned sid = 0; sid < map->sample_count; sid++)
        {
            mkv_sample_t *s = track->sample_vector.at(sid);

            if (track->TrackType == MKV_TRACK_VIDEO)
            {
                if (s->idr)
                {
                    map->sample_type[sid] = sample_VIDEO_SYNC;
                    map->frame_count_idr++;
                }
                else
                    map->sample_type[sid] = sample_VIDEO;
            }
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
        delete [] mkv->ebml.DocType;

        // mkv_info_t
        delete [] mkv->info.SegmentUID;
        delete [] mkv->info.SegmentFilename;
        delete [] mkv->info.PrevUID;
        delete [] mkv->info.PrevFilename;
        delete [] mkv->info.NextUID;
        delete [] mkv->info.NextFilename;
        delete [] mkv->info.SegmentFamily;
        for (int i = 0; i < mkv->info.chapter_count; i++)
        {
            // mkv_info_chapter_t // TODO
            //delete  mkv->info.chapter[i].ChapterTranslateID;
        }
        delete [] mkv->info.Title;
        delete [] mkv->info.MuxingApp;
        delete [] mkv->info.WritingApp;

        // mkv_tracks_t
        for (unsigned stream_id = 0; stream_id < mkv->tracks_count; stream_id++)
        {
            if (mkv->tracks[stream_id])
            {
                delete [] mkv->tracks[stream_id]->Name;
                delete [] mkv->tracks[stream_id]->Language;
                delete [] mkv->tracks[stream_id]->CodecID;
                delete [] mkv->tracks[stream_id]->CodecPrivate;
                delete [] mkv->tracks[stream_id]->CodecName;

                if (mkv->tracks[stream_id]->video)
                {
                    if (mkv->tracks[stream_id]->video->Projection)
                    {
                        delete [] mkv->tracks[stream_id]->video->Projection->ProjectionPrivate;
                        delete mkv->tracks[stream_id]->video->Projection;
                    }
                    if (mkv->tracks[stream_id]->video->Colour)
                    {
                        delete [] mkv->tracks[stream_id]->video->Colour->MasteringMetadata;
                        delete mkv->tracks[stream_id]->video->Colour;
                    }
                    delete [] mkv->tracks[stream_id]->video->ColourSpace;
                    delete mkv->tracks[stream_id]->video;
                }
                if (mkv->tracks[stream_id]->audio)
                {
                    delete mkv->tracks[stream_id]->audio;
                }
                if (mkv->tracks[stream_id]->translate)
                {
                    delete mkv->tracks[stream_id]->translate;
                }
                if (mkv->tracks[stream_id]->operation)
                {
                    delete mkv->tracks[stream_id]->operation;
                }
                if (mkv->tracks[stream_id]->encodings)
                {
                    delete mkv->tracks[stream_id]->encodings->encoding;
                    delete mkv->tracks[stream_id]->encodings;
                }

                // SPS
                for (unsigned sps_id = 0; sps_id < mkv->tracks[stream_id]->sps_count && sps_id < MAX_SPS; sps_id++)
                    freeSPS(&mkv->tracks[stream_id]->sps_array[sps_id]);
                delete *mkv->tracks[stream_id]->sps_array;
                delete [] mkv->tracks[stream_id]->sps_sample_offset;
                delete [] mkv->tracks[stream_id]->sps_sample_size;

                // PPS
                for (unsigned pps_id = 0; pps_id < mkv->tracks[stream_id]->pps_count && pps_id < MAX_PPS; pps_id++)
                    freePPS(&mkv->tracks[stream_id]->pps_array[pps_id]);
                delete *mkv->tracks[stream_id]->pps_array;
                delete [] mkv->tracks[stream_id]->pps_sample_offset;
                delete [] mkv->tracks[stream_id]->pps_sample_size;

                // Samples
                for (unsigned sample_id = 0; sample_id < mkv->tracks[stream_id]->sample_vector.size(); sample_id++)
                {
                    delete mkv->tracks[stream_id]->sample_vector.at(sample_id);
                }
                mkv->tracks[stream_id]->sample_vector.clear();

                delete mkv->tracks[stream_id];
            }
        }
    }
}

/* ************************************************************************** */
