/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      mp4_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "mp4.h"
#include "mp4_struct.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

/*!
 * \brief Convert a Mp4Track_t structure into a generic MediaStream_t.
 * \param *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \param *track: A pointer to the mp4 track structure we want to extract data from.
 * \param idr_only Set to true if we only want to extract IDR samples.
 *
 * - Use STSZ box content to get back all samples.
 * - Use STSS box content to get back IDR samples only.
 */
bool convertTrack(MediaFile_t *media, Mp4_t *mp4, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "convertTrack()" CLR_RESET);
    bool retcode = SUCCESS;
    MediaStream_t *map = NULL;

    if (media == NULL || track == NULL)
    {
        TRACE_ERROR(MP4, "Cannot access audio or video tracks from the MP4 parser!");
        retcode = FAILURE;
    }

    // Select and init a bitstream map (A or V)
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS)
    {
        int sample_count = track->stsz_sample_count;
        if (sample_count <= 0)
            sample_count = 1;

        if (track->handlerType == HANDLER_AUDIO)
        {
            retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_audio[media->tracks_audio_count];
                media->tracks_audio_count++;
            }
        }
        else if (track->handlerType == HANDLER_VIDEO)
        {
            retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], sample_count + track->sps_count + track->pps_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_video[media->tracks_video_count];
                media->tracks_video_count++;
            }
        }
        else if (track->handlerType == HANDLER_SUBT ||
                 track->handlerType == HANDLER_SBTL ||
                 track->handlerType == HANDLER_TEXT)
        {
            retcode = init_bitstream_map(&media->tracks_subt[media->tracks_subtitles_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_subt[media->tracks_subtitles_count];
                media->tracks_subtitles_count++;
            }
        }
        else if (track->handlerType == HANDLER_TMCD)
        {
            retcode = init_bitstream_map(&media->tracks_others[media->tracks_others_count], sample_count);
            if (retcode == SUCCESS)
            {
                map = media->tracks_others[media->tracks_others_count];
                media->tracks_others_count++;
            }
        }
        else
        {
            char fcc[5];
            TRACE_WARNING(MP4, "Not sure we can build bitstream_map for other track types! (track #%u handlerType: %s)", track->id, getFccString_le(track->handlerType, fcc));

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
        unsigned int i = 0, j = 0;

        map->stream_fcc = track->fcc;
        map->stream_codec = (Codecs_e)track->codec;
        map->stream_codec_profile = (CodecProfiles_e)track->codec_profile;

        if (strnlen(track->compressorname, 32) > 0)
        {
            map->stream_encoder = (char *)malloc(sizeof(track->compressorname));
            strncpy(map->stream_encoder, track->compressorname, sizeof(track->compressorname));
        }
        if (strnlen(track->name, 128) > 0)
        {
            map->track_title = (char *)malloc(sizeof(track->name));
            strncpy(map->track_title, track->name, sizeof(track->name));
        }

        map->track_languagecode = (char *)malloc(4);
        strncpy(map->track_languagecode, track->language, 3);
        map->track_languagecode[3] = '\0';

        if (track->timescale)
        {
            map->stream_duration_ms = ((double)track->duration / (double)track->timescale * 1000.0);
            map->creation_time = ((double)track->creation_time / (double)track->timescale * 1000.0);
            map->modification_time = ((double)track->modification_time / (double)track->timescale * 1000.0);
        }

        map->stream_packetized = true;
        map->sample_count = track->stsz_sample_count + track->sps_count + track->pps_count;

        map->track_id = track->id;

        if (track->handlerType == HANDLER_AUDIO)
        {
            map->stream_type = stream_AUDIO;
            map->sampling_rate = track->sample_rate_hz;
            map->channel_count = track->channel_count;
            map->bit_per_sample = track->sample_size_bits;
        }
        else if (track->handlerType == HANDLER_VIDEO)
        {
            map->stream_type = stream_VIDEO;
            map->width = track->width;
            map->height = track->height;
            map->color_depth = track->color_depth;
            map->color_matrix = track->color_matrix;
            map->color_range = track->color_range;

            if (track->par_h && track->par_v)
            {
                map->pixel_aspect_ratio_h = track->par_h;
                map->pixel_aspect_ratio_v = track->par_v;
            }
            else
            {
                map->pixel_aspect_ratio_h = 1;
                map->pixel_aspect_ratio_v = 1;
            }

            map->frame_count_idr = track->stss_entry_count;

            // Framerate
            {
                uint32_t scalefactor = 1;
                map->framerate_num = track->timescale * scalefactor;

                if (track->stsz_sample_count == 0)
                    map->framerate_base = track->mediatime * scalefactor; // used for "progressive download" files
                else
                    map->framerate_base = ((double)track->duration / (double)track->stsz_sample_count * (double)scalefactor);

                if (map->framerate_base > 0.0)
                    map->framerate = map->framerate_num / map->framerate_base;

                TRACE_1(MP4, "framerate_num: %f  / framerate_base: %f",
                        map->framerate_num, map->framerate_base);
            }

            // Codec specific metadata
            if (track->codec == CODEC_H264 || track->codec == CODEC_H265)
            {
                // Set SPS
                for (i = 0; i < track->sps_count; i++)
                {
                    map->sample_type[i] = sample_VIDEO_PARAM;
                    map->sample_offset[i] = track->sps_sample_offset[i];
                    map->sample_size[i] = track->sps_sample_size[i];
                    map->sample_pts[i] = -1;
                    map->sample_dts[i] = -1;
                }
                // Set PPS
                for (i = 0; i < track->pps_count; i++)
                {
                    map->sample_type[i + track->sps_count] = sample_VIDEO_PARAM;
                    map->sample_offset[i + track->sps_count] = track->pps_sample_offset[i];
                    map->sample_size[i + track->sps_count] = track->pps_sample_size[i];
                    map->sample_pts[i + track->sps_count] = -1;
                    map->sample_dts[i + track->sps_count] = -1;
                }
            }
        }
        else if (track->handlerType == HANDLER_SUBT ||
                 track->handlerType == HANDLER_SBTL ||
                 track->handlerType == HANDLER_TEXT)
        {
            map->stream_type = stream_TEXT;
        }
        else if (track->handlerType == HANDLER_TMCD)
        {
            map->stream_type = stream_TMCD;
        }
        else if (track->handlerType == HANDLER_META)
        {
            map->stream_type = stream_META;
        }
        else if (track->handlerType == HANDLER_HINT)
        {
            map->stream_type = stream_HINT;
        }
        else
        {
            map->stream_type = stream_UNKNOWN;
        }

        // Set samples details into the bitstream map
        ////////////////////////////////////////////////////////////////

        // Bitrate mode
        uint32_t sample_size_cbr = 0;
        if (!track->stsz_entry_size)
        {
            // Assume constant sample size
            map->bitrate_mode = BITRATE_CBR;
            sample_size_cbr = track->stsz_sample_size;
        }

        // Set sample type & size
        for (i = 0; i < track->stsz_sample_count; i++)
        {
            unsigned sid = i + track->sps_count + track->pps_count; // Sample id

            // Set sample type
            if (track->handlerType == HANDLER_VIDEO)
            {
                map->sample_type[sid] = sample_VIDEO;

                for (j = 0; j < track->stss_entry_count; j++)
                {
                    if (i == (track->stss_sample_number[j] - 1))
                        map->sample_type[sid] = sample_VIDEO_SYNC;
                }
            }
            else if (track->handlerType == HANDLER_AUDIO)
            {
                map->sample_type[sid] = sample_AUDIO;
            }
            else if (track->handlerType == HANDLER_SUBT ||
                     track->handlerType == HANDLER_SBTL ||
                     track->handlerType == HANDLER_TEXT)
            {
                map->sample_type[sid] = sample_TEXT;
            }
            else
            {
                map->sample_type[sid] = sample_OTHER;
            }

            // Set sample size
            if (track->stsz_entry_size)
            {
                map->sample_size[sid] = track->stsz_entry_size[i];
            }
            else // Assume constant sample size
            {
                map->sample_size[sid] = sample_size_cbr;
            }

            // Contribute to stream size
            map->stream_size += map->sample_size[sid];
        }

        // Set sample decoding and presentation timecodes
        uint32_t k = 0;
        j = 0;
        int32_t _samples_pts_to_dts_shift = 0; // FIXME // from cslg

        if (track->ctts_sample_count) //if (_samples_pts_array)
        {
            // Compute DTS
            for (i = 0, k = track->sps_count + track->pps_count; i < track->stts_entry_count; i++)
            {
                j = 0;

                if (k == track->sps_count + track->pps_count)
                {
                    // Decoding time = 0 for the first DTS sample
                    map->sample_dts[k] = 0;

                    k++;
                    j = 1;
                }

                for (; j < track->stts_sample_count[i]; j++, k++)
                {
                    int64_t dts = map->sample_dts[k - 1];
                    dts += (int64_t)track->stts_sample_delta[i];

                    map->sample_dts[k] = dts;
                }
            }

            // Then compute PTS
            for (i = 0, k = track->sps_count + track->pps_count; i < track->ctts_entry_count; i++)
            {
                for (j = 0; j < track->ctts_sample_count[i]; j++, k++)
                {
                    int64_t dts = map->sample_dts[k];
                    int64_t pts = dts + track->ctts_sample_offset[i] + _samples_pts_to_dts_shift;

                    map->sample_pts[k] = pts; // Assign pts
                }
            }
        }
        else
        {
            // Compute DTS, then copy results into PTS
            for (i = 0, k = track->sps_count + track->pps_count; i < track->stts_entry_count; i++)
            {
                j = 0;

                if (k == track->sps_count + track->pps_count)
                {
                    // Decoding time = 0 for the first DTS sample
                    map->sample_dts[k] = map->sample_pts[k] = 0;

                    k++;
                    j = 1;
                }

                for (; j < track->stts_sample_count[i]; j++, k++)
                {
                    int64_t dts = map->sample_dts[k - 1];
                    int64_t pts = dts + (int64_t)track->stts_sample_delta[i];

                    map->sample_dts[k] = map->sample_pts[k] = pts;
                }
            }
        }

        // Set sample offset
        uint32_t index = track->sps_count + track->pps_count;
        uint32_t chunkOffset = 0;

        for (i = 0; (i < track->stsc_entry_count) && (chunkOffset < track->stco_entry_count); i++)
        {
            uint32_t n = 0, l = 0;
            k = 0;

            if ((i + 1) == track->stsc_entry_count)
            {
                if ((track->stsc_entry_count > 1) && (chunkOffset == 0))
                {
                    n = 1;
                }
                else
                {
                    n = track->stco_entry_count - chunkOffset;
                }
            }
            else
            {
                n = track->stsc_first_chunk[i + 1] - track->stsc_first_chunk[i];
            }

            for (k = 0; k < n; k++)
            {
                for (l = 0; l < track->stsc_samples_per_chunk[i]; l++)
                {
                    // Adjust DTS and PTS unit: from timescale to ns
                    {
                        if (map->sample_dts[index])
                        {
                            map->sample_dts[index] *= 1000000LL;
                            map->sample_dts[index] /= track->timescale;
                        }

                        //if (map->sample_pts[index])
                        {
                            //if (map->sample_pts[index] > av_pts_adjustment)
                            //    map->sample_pts[index] -= av_pts_adjustment; // FIXME // from edit list;

                            map->sample_pts[index] *= 1000000LL;
                            map->sample_pts[index] /= track->timescale;
                        }

                        //TRACE_2(MP4, "#%u > DTS: %lli  /  PTS: %lli", index, map->sample_dts[index], map->sample_pts[index]);
                    }

                    // FIXME // sample description index is not taken into account
                    if (l == 0)
                    {
                        map->sample_offset[index++] = track->stco_chunk_offset[chunkOffset];
                    }
                    else
                    {
                        map->sample_offset[index] = map->sample_offset[index - 1] + (int64_t)(map->sample_size[index - 1]);
                        index++;
                    }
                }

                chunkOffset++; // Increase chunk offset
            }
        }

#if ENABLE_DEBUG
        TRACE_1(MP4, BLD_GREEN ">> track content recap:" CLR_RESET);
        if (map->stream_type == stream_VIDEO)
        {
            TRACE_1(MP4, "Video Stream");
        }
        else if (map->stream_type == stream_AUDIO)
        {
            TRACE_1(MP4, "Audio Stream");
        }

        TRACE_1(MP4, "sample_count     : %u", map->sample_count);
        TRACE_1(MP4, "sample_count_idr : %u", map->frame_count_idr);
/*
        for (i = 0; i < map->sample_count; i++)
        {
            TRACE_2(MP4, "[%u] sample type   > %u", i, map->sample_type[i]);
            TRACE_1(MP4, "[%u] sample size   > %u", i, map->sample_size[i]);
            TRACE_1(MP4, "[%u] sample offset > %lli", i, map->sample_offset[i]);
            TRACE_2(MP4, "[%u] sample pts    > %lli", i, map->sample_pts[i]);
            TRACE_2(MP4, "[%u] sample dts    > %lli", i, map->sample_dts[i]);
        }
*/
#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Free an Mp4Track_t structure.
 * \param **track_ptr A pointer to the Mp4Track_t structure we want to freed.
 */
void freeTrack(Mp4Track_t **track_ptr)
{
    if (*track_ptr != NULL)
    {
        // SPS
        free((*track_ptr)->sps_sample_offset);
        free((*track_ptr)->sps_sample_size);

        // PPS
        free((*track_ptr)->pps_sample_offset);
        free((*track_ptr)->pps_sample_size);

        // stss
        free((*track_ptr)->stss_sample_number);

        // stss
        free((*track_ptr)->stts_sample_count);
        free((*track_ptr)->stts_sample_delta);

        // ctts
        free((*track_ptr)->ctts_sample_count);
        free((*track_ptr)->ctts_sample_offset);

        // stsc
        free((*track_ptr)->stsc_first_chunk);
        free((*track_ptr)->stsc_samples_per_chunk);
        free((*track_ptr)->stsc_sample_description_index);

        // stsz / stz2
        free((*track_ptr)->stsz_entry_size);

        // stco / co64
        free((*track_ptr)->stco_chunk_offset);

        // sdtp
        free((*track_ptr)->sdtp_is_leading);
        free((*track_ptr)->sdtp_sample_depends_on);
        free((*track_ptr)->sdtp_sample_is_depended_on);
        free((*track_ptr)->sdtp_sample_has_redundancy);

        // track
        free(*track_ptr);
        *track_ptr = NULL;
    }
}

/* ************************************************************************** */
