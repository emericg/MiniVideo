/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \file      bitstream_map.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "bitstream.h"
#include "avcodecs.h"
#include "fourcc.h"
#include "minitraces.h"

// C standard libraries
#include <stdlib.h>
#include <math.h>

/* ************************************************************************** */

/*!
 * \brief Initialize a bitstream_map structure with a fixed number of entries.
 * \param bitstream_map A pointer to the *bitstreamMap_t structure to initialize.
 * \param entries The number of sample to init into the bitstreamMap_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * Everything inside the bitstreamMap_t structure is set to 0, even the number
 * of entries (sample_count).
 */
int init_bitstream_map(BitstreamMap_t **bitstream_map, uint32_t entries)
{
    TRACE_INFO(DEMUX, "<b> " BLD_BLUE "init_bitstream_map()\n" CLR_RESET);
    int retcode = SUCCESS;

    if (*bitstream_map != NULL)
    {
        TRACE_ERROR(DEMUX, "<b> Unable to alloc a new bitstream_map: not null!\n");
        retcode = FAILURE;
    }
    else
    {
        if (entries == 0)
        {
            TRACE_ERROR(DEMUX, "<b> Unable to allocate a new bitstream_map: no entries to allocate!\n");
            retcode = FAILURE;
        }
        else
        {
            *bitstream_map = (BitstreamMap_t*)calloc(1, sizeof(BitstreamMap_t));

            if (*bitstream_map == NULL)
            {
                TRACE_ERROR(DEMUX, "<b> Unable to allocate a new bitstream_map!\n");
                retcode = FAILURE;
            }
            else
            {
                (*bitstream_map)->sample_type = (uint32_t*)calloc(entries, sizeof(uint32_t));
                (*bitstream_map)->sample_size = (uint32_t*)calloc(entries, sizeof(uint32_t));
                (*bitstream_map)->sample_offset = (int64_t*)calloc(entries, sizeof(int64_t));
                (*bitstream_map)->sample_pts = (int64_t*)calloc(entries, sizeof(int64_t));
                (*bitstream_map)->sample_dts = (int64_t*)calloc(entries, sizeof(int64_t));

                if ((*bitstream_map)->sample_type == NULL ||
                    (*bitstream_map)->sample_size == NULL ||
                    (*bitstream_map)->sample_offset == NULL ||
                    (*bitstream_map)->sample_pts == NULL ||
                    (*bitstream_map)->sample_dts == NULL)
                {
                    TRACE_ERROR(DEMUX, "<b> Unable to alloc bitstream_map > sample_type / sample_size / sample_offset / sample_timecode!\n");

                    if ((*bitstream_map)->sample_type != NULL)
                        free((*bitstream_map)->sample_type);
                    if ((*bitstream_map)->sample_size != NULL)
                        free((*bitstream_map)->sample_size);
                    if ((*bitstream_map)->sample_offset != NULL)
                        free((*bitstream_map)->sample_offset);
                    if ((*bitstream_map)->sample_pts != NULL)
                        free((*bitstream_map)->sample_pts);
                    if ((*bitstream_map)->sample_dts != NULL)
                        free((*bitstream_map)->sample_dts);

                    free(*bitstream_map);
                    *bitstream_map = NULL;
                    retcode = FAILURE;
                }
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Destroy a bitstream_map structure.
 * \param *bitstream_map A pointer to a *bitstreamMap_t structure.
 */
void free_bitstream_map(BitstreamMap_t **bitstream_map)
{
    if ((*bitstream_map) != NULL)
    {
        TRACE_INFO(DEMUX, "<b> " BLD_BLUE "free_bitstream_map()\n" CLR_RESET);

        // Textual metadatas
        free((*bitstream_map)->stream_encoder);
        (*bitstream_map)->stream_encoder = NULL;
        free((*bitstream_map)->track_title);
        (*bitstream_map)->track_title = NULL;
        free((*bitstream_map)->track_languagecode);
        (*bitstream_map)->track_languagecode = NULL;
        free((*bitstream_map)->subtitles_name);
        (*bitstream_map)->subtitles_name = NULL;

        // Sample tables
        free((*bitstream_map)->sample_type);
        (*bitstream_map)->sample_type = NULL;

        free((*bitstream_map)->sample_size);
        (*bitstream_map)->sample_size = NULL;

        free((*bitstream_map)->sample_offset);
        (*bitstream_map)->sample_offset = NULL;

        free((*bitstream_map)->sample_pts);
        (*bitstream_map)->sample_pts = NULL;

        free((*bitstream_map)->sample_dts);
        (*bitstream_map)->sample_dts = NULL;


        free(*bitstream_map);
        *bitstream_map = NULL;

        TRACE_1(DEMUX, "<b> Bitstream_map freed\n");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Print the content of a bitstreamMap_t structure.
 * \param bitstream_map docme.
 */
void print_bitstream_map(BitstreamMap_t *bitstream_map)
{
#if ENABLE_DEBUG

    if (bitstream_map == NULL)
    {
        TRACE_ERROR(DEMUX, "Invalid bitstream_map structure!\n");
    }
    else
    {
        TRACE_INFO(DEMUX, BLD_GREEN "print_bitstream_map()\n" CLR_RESET);

        if (bitstream_map->stream_type == stream_VIDEO &&
            bitstream_map->sample_count > 0)
        {
            TRACE_INFO(DEMUX, "Elementary stream type > VIDEO\n");
        }
        else if (bitstream_map->stream_type == stream_AUDIO &&
                 bitstream_map->sample_count > 0)
        {
            TRACE_INFO(DEMUX, "Elementary stream type > AUDIO\n");
        }
        else
        {
            TRACE_WARNING(DEMUX, "Unknown elementary stream type!\n");
        }

        TRACE_1(DEMUX, "Track codec:     '%s'\n", getCodecString(bitstream_map->stream_type, bitstream_map->stream_codec, true));

        TRACE_INFO(DEMUX, "> samples alignment: %i\n", bitstream_map->sample_alignment);
        TRACE_INFO(DEMUX, "> samples count    : %i\n", bitstream_map->sample_count);
        TRACE_INFO(DEMUX, "> IDR samples count: %i\n", bitstream_map->frame_count_idr);

        if (bitstream_map->sample_count > 0)
        {
            TRACE_1(DEMUX, "SAMPLES\n");
            for (unsigned  i = 0; i < bitstream_map->sample_count; i++)
            {
                TRACE_1(DEMUX, "> sample_type      : %i\n", bitstream_map->sample_type[i]);
                TRACE_1(DEMUX, "  | sample_offset  : %i\n", bitstream_map->sample_offset[i]);
                TRACE_1(DEMUX, "  | sample_size    : %i\n", bitstream_map->sample_size[i]);
                TRACE_1(DEMUX, "  | sample_timecode: %i\n", bitstream_map->sample_pts[i]);
            }
        }
    }

#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

static void computeSamplesDatasTrack(BitstreamMap_t *t)
{
    if (t)
    {
        uint64_t totalbytes = 0;
        bool cbr = true;
        int64_t frameinterval = 0;
        bool cfr = true;
        unsigned j = 0;

        if (t->sample_alignment)
        {
            t->frame_count = t->sample_count;
        }
        if (t->stream_intracoded)
        {
            t->frame_count_idr = t->frame_count;
        }

        // Video frame duration
        if (t->stream_type == stream_VIDEO && t->frame_duration == 0 && t->framerate != 0)
            t->frame_duration = 1000.0 / t->framerate;
/*
        // Video frame interval
        if (t->sample_pts[0] && t->sample_pts[1])
        {
            frameinterval = t->sample_pts[1] - t->sample_pts[0];
        }
        TRACE_ERROR(DEMUX, "pts1: %lli \n", t->sample_pts[0]);
        TRACE_ERROR(DEMUX, "pts2: %lli \n", t->sample_pts[1]);
        TRACE_ERROR(DEMUX, "pts2: %lli \n", t->sample_pts[2]);
        TRACE_ERROR(DEMUX, "pts2: %lli \n", t->sample_pts[3]);
        TRACE_ERROR(DEMUX, "pts2: %lli \n", t->sample_pts[55]);
        TRACE_ERROR(DEMUX, "frameinterval: %lli \n", frameinterval);
*/
        // Iterate on each sample
        for (j = 0; j < t->sample_count; j++)
        {
            totalbytes += t->sample_size[j];

            if (t->sample_size[0] != t->sample_size[j])
                cbr = false;

            if (t->sample_alignment == false)
            {
                if (t->sample_pts[j] || t->sample_dts[j])
                    t->frame_count++;
            }
        }

        // Set bitrate mode
        if (cbr == true)
        {
            t->bitrate_mode = BITRATE_CBR;
        }
        else
        {
            // TODO check if we have AVBR / CVBR ?
            t->bitrate_mode = BITRATE_VBR;
        }
/*
        // Set framerate mode
        if (cfr == true)
        {
            t->framerate_mode = FRAMERATE_CFR;
        }
        else
        {
            t->framerate_mode = FRAMERATE_VFR;
        }
*/
        // Set stream size
        if (t->stream_size == 0)
        {
            t->stream_size = totalbytes;
        }

        // Set stream duration
        if (t->duration_ms == 0)
        {
            t->duration_ms = (double)t->frame_count * t->frame_duration;
        }

        // Set gross bitrate value (in bps)
        if (t->bitrate == 0 && t->duration_ms != 0)
        {
            t->bitrate = (unsigned int)round(((double)t->stream_size / (double)(t->duration_ms)));
            t->bitrate *= 1000; // ms to s
            t->bitrate *= 8; // B to b
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief PCM sample size hack
 *
 * PCM sample size can be recomputed if the informations gathered from the
 * conteners are wrong (like the sample size). This will also trigger a new
 * bitrate computation.
 */
bool computePCMSettings(BitstreamMap_t *track)
{
    bool retcode = SUCCESS;
    uint32_t sample_size_cbr = track->channel_count * (track->bit_per_sample/8);

    // First, check if the hack is needed
    if (track->sample_count > 0 && track->sample_size[0] != sample_size_cbr)
    {
        TRACE_ERROR(DEMUX, BLD_GREEN "computePCMSettings()\n" CLR_RESET);

        track->sample_per_frames = 1;
        track->stream_size = track->sample_count * sample_size_cbr;
        track->bitrate = 0; // reset bitrate

        for (unsigned i = 0; i < track->sample_count; i++)
        {
            track->sample_size[i] = sample_size_cbr;
        }
    }

    return retcode;
}

bool computeCodecs(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeCodecs()\n" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        if (media->tracks_video[i] && media->tracks_video[i]->stream_codec == CODEC_UNKNOWN)
        {
             media->tracks_video[i]->stream_codec = getCodecFromFourCC(media->tracks_video[i]->stream_fcc);
        }
    }

    for (unsigned i = 0; i < media->tracks_audio_count; i++)
    {
        if (media->tracks_audio[i] && media->tracks_audio[i]->stream_codec == CODEC_UNKNOWN)
        {
            media->tracks_audio[i]->stream_codec = getCodecFromFourCC(media->tracks_audio[i]->stream_fcc);
        }

        // PCM hack
        if (media->tracks_audio[i]->stream_codec == CODEC_LPCM ||
            media->tracks_audio[i]->stream_codec == CODEC_LogPCM ||
            media->tracks_audio[i]->stream_codec == CODEC_DPCM ||
            media->tracks_audio[i]->stream_codec == CODEC_ADPCM)
        {
            computePCMSettings(media->tracks_audio[i]);
        }
    }

    return retcode;
}

/* ************************************************************************** */

bool computeAspectRatios(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeAspectRatios()\n" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        BitstreamMap_t *t = media->tracks_video[i];
        if (t)
        {
            // First pass on PAR (if set by the container)
             if (t->pixel_aspect_ratio_h && t->pixel_aspect_ratio_v)
             {
                 t->pixel_aspect_ratio = (double)t->pixel_aspect_ratio_h / (double)t->pixel_aspect_ratio_v;
             }
             else
             {
                 t->pixel_aspect_ratio = 1.0;
                 t->pixel_aspect_ratio_h = 1;
                 t->pixel_aspect_ratio_v = 1;
             }

             if (t->video_aspect_ratio_h && t->video_aspect_ratio_v)
             {
                 // First pass on PAR (if set by the container)
                 t->video_aspect_ratio = (double)t->video_aspect_ratio_h / (double)t->video_aspect_ratio_v;
             }
             else if (t->width && t->height)
             {
                 // First pass on PAR (if computer from video resolution)
                 t->video_aspect_ratio = (double)t->width / (double)t->height,
                 t->video_aspect_ratio_h = t->width;
                 t->video_aspect_ratio_v = t->height;
             }

             // Compute display aspect ratio
             if (t->display_aspect_ratio_h && t->display_aspect_ratio_v)
             {
                 t->display_aspect_ratio = (double)t->display_aspect_ratio_h / (double)t->display_aspect_ratio_v;
             }
             else
             {
                 if (t->pixel_aspect_ratio != 1.0)
                 {
                     t->display_aspect_ratio = t->video_aspect_ratio * t->pixel_aspect_ratio;
                 }
                 else
                 {
                     t->display_aspect_ratio = t->video_aspect_ratio;
                 }
             }

             // Second pass on PAR
             if (t->pixel_aspect_ratio == 1.0 &&
                 (t->video_aspect_ratio != t->display_aspect_ratio))
             {
                 //
             }
        }
    }

    for (unsigned i = 0; i < media->tracks_audio_count; i++)
    {
        //
    }

    return retcode;
}

/* ************************************************************************** */

bool computeSamplesDatas(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeSamplesDatas()\n" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        if (media->tracks_video[i])
        {
            computeSamplesDatasTrack(media->tracks_video[i]);
        }
    }

    for (unsigned i = 0; i < media->tracks_audio_count; i++)
    {
        if (media->tracks_audio[i])
        {
            computeSamplesDatasTrack(media->tracks_audio[i]);
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */
