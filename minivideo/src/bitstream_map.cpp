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
 * \file      bitstream_map.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "bitstream_map.h"
#include "minivideo_codecs.h"
#include "minivideo_fourcc.h"
#include "minitraces.h"
#include "minivideo_typedef.h"

// C standard libraries
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Initialize a MediaStream_t structure with a fixed number of entries.
 * \param stream_ptr: The address of the pointer to the MediaStream_t structure to initialize.
 * \param parameter_count: The number of sample to init into the MediaStream_t structure.
 * \param sample_count: The number of sample to init into the MediaStream_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * Everything inside the MediaStream_t structure is set to 0, even the number
 * of entries (sample_count).
 */
int init_bitstream_map(MediaStream_t **stream_ptr, uint32_t parameter_count, uint32_t sample_count)
{
    TRACE_INFO(DEMUX, "<b> " BLD_BLUE "init_bitstream_map()" CLR_RESET);
    int retcode = SUCCESS;

    if (*stream_ptr != NULL)
    {
        TRACE_ERROR(DEMUX, "<b> Unable to alloc a new bitstream_map: not null!");
        retcode = FAILURE;
    }
    else
    {
        *stream_ptr = (MediaStream_t*)calloc(1, sizeof(MediaStream_t));
        if (*stream_ptr == NULL)
        {
            TRACE_ERROR(DEMUX, "<b> Unable to allocate a new bitstream_map!");
            retcode = FAILURE;
        }
        else
        {
            if (retcode == SUCCESS && parameter_count > 0)
            {
                (*stream_ptr)->parameter_type = (uint32_t*)calloc(parameter_count, sizeof(uint32_t));
                (*stream_ptr)->parameter_size = (uint32_t*)calloc(parameter_count, sizeof(uint32_t));
                (*stream_ptr)->parameter_offset = (int64_t*)calloc(parameter_count, sizeof(int64_t));

                if ((*stream_ptr)->parameter_type == NULL ||
                    (*stream_ptr)->parameter_size == NULL ||
                    (*stream_ptr)->parameter_offset == NULL)
                {
                    TRACE_ERROR(DEMUX, "<b> Unable to alloc bitstream_map > sample_type / sample_size / sample_offset / sample_timecode!");

                    if ((*stream_ptr)->parameter_type != NULL)
                        free((*stream_ptr)->parameter_type);
                    if ((*stream_ptr)->parameter_size != NULL)
                        free((*stream_ptr)->parameter_size);
                    if ((*stream_ptr)->parameter_offset != NULL)
                        free((*stream_ptr)->parameter_offset);

                    free(*stream_ptr);
                    *stream_ptr = NULL;
                    retcode = FAILURE;
                }
            }

            if (retcode == SUCCESS && sample_count > 0)
            {
                (*stream_ptr)->sample_type = (uint32_t*)calloc(sample_count, sizeof(uint32_t));
                (*stream_ptr)->sample_size = (uint32_t*)calloc(sample_count, sizeof(uint32_t));
                (*stream_ptr)->sample_offset = (int64_t*)calloc(sample_count, sizeof(int64_t));
                (*stream_ptr)->sample_pts = (int64_t*)calloc(sample_count, sizeof(int64_t));
                (*stream_ptr)->sample_dts = (int64_t*)calloc(sample_count, sizeof(int64_t));

                if ((*stream_ptr)->sample_type == NULL ||
                    (*stream_ptr)->sample_size == NULL ||
                    (*stream_ptr)->sample_offset == NULL ||
                    (*stream_ptr)->sample_pts == NULL ||
                    (*stream_ptr)->sample_dts == NULL)
                {
                    TRACE_ERROR(DEMUX, "<b> Unable to alloc bitstream_map > sample_type / sample_size / sample_offset / sample_timecode!");

                    if ((*stream_ptr)->sample_type != NULL)
                        free((*stream_ptr)->sample_type);
                    if ((*stream_ptr)->sample_size != NULL)
                        free((*stream_ptr)->sample_size);
                    if ((*stream_ptr)->sample_offset != NULL)
                        free((*stream_ptr)->sample_offset);
                    if ((*stream_ptr)->sample_pts != NULL)
                        free((*stream_ptr)->sample_pts);
                    if ((*stream_ptr)->sample_dts != NULL)
                        free((*stream_ptr)->sample_dts);

                    free(*stream_ptr);
                    *stream_ptr = NULL;
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
 * \param stream_ptr: The address of the pointer to the MediaStream_t structure to initialize.
 */
void free_bitstream_map(MediaStream_t **stream_ptr)
{
    if ((*stream_ptr) != NULL)
    {
        TRACE_INFO(DEMUX, "<b> " BLD_BLUE "free_bitstream_map()" CLR_RESET);

        // Strings
        free((*stream_ptr)->stream_encoder);
        (*stream_ptr)->stream_encoder = NULL;
        free((*stream_ptr)->track_title);
        (*stream_ptr)->track_title = NULL;
        free((*stream_ptr)->track_languagecode);
        (*stream_ptr)->track_languagecode = NULL;
        free((*stream_ptr)->subtitles_name);
        (*stream_ptr)->subtitles_name = NULL;

        // "Parameter sets" arrays
        free((*stream_ptr)->parameter_type);
        (*stream_ptr)->parameter_type = NULL;

        free((*stream_ptr)->parameter_size);
        (*stream_ptr)->parameter_size = NULL;

        free((*stream_ptr)->parameter_offset);
        (*stream_ptr)->parameter_offset = NULL;

        // Samples arrays
        free((*stream_ptr)->sample_type);
        (*stream_ptr)->sample_type = NULL;

        free((*stream_ptr)->sample_size);
        (*stream_ptr)->sample_size = NULL;

        free((*stream_ptr)->sample_offset);
        (*stream_ptr)->sample_offset = NULL;

        free((*stream_ptr)->sample_pts);
        (*stream_ptr)->sample_pts = NULL;

        free((*stream_ptr)->sample_dts);
        (*stream_ptr)->sample_dts = NULL;

        // The MediaStream_t itself
        free(*stream_ptr);
        *stream_ptr = NULL;

        TRACE_1(DEMUX, "<b> MediaStream_t freed");
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Print the content of a MediaStream_t structure.
 * \param *stream: A pointer to a MediaStream_t structure.
 */
void print_bitstream_map(MediaStream_t *stream)
{
#if ENABLE_DEBUG

    if (stream == NULL)
    {
        TRACE_ERROR(DEMUX, "Invalid bitstream_map structure!");
    }
    else
    {
        TRACE_INFO(DEMUX, BLD_GREEN "print_bitstream_map()" CLR_RESET);

        if (stream->stream_type == stream_VIDEO &&
            stream->sample_count > 0)
        {
            TRACE_INFO(DEMUX, "Elementary stream type > VIDEO");
        }
        else if (stream->stream_type == stream_AUDIO &&
                 stream->sample_count > 0)
        {
            TRACE_INFO(DEMUX, "Elementary stream type > AUDIO");
        }
        else
        {
            TRACE_WARNING(DEMUX, "Unknown elementary stream type!");
        }

        TRACE_1(DEMUX, "Track codec:     '%s'", getCodecString(stream->stream_type, stream->stream_codec, true));

        TRACE_INFO(DEMUX, "> stream packetized  : %i", stream->stream_packetized);
        TRACE_INFO(DEMUX, "> samples count      : %i", stream->sample_count);
        TRACE_INFO(DEMUX, "> frames count       : %i", stream->frame_count);
        TRACE_INFO(DEMUX, "> IDR frames count   : %i", stream->frame_count_idr);

        if (stream->sample_count > 0)
        {
            TRACE_1(DEMUX, "SAMPLES");
            for (unsigned  i = 0; i < stream->sample_count; i++)
            {
                TRACE_1(DEMUX, "> sample_type      : %i", stream->sample_type[i]);
                TRACE_1(DEMUX, "  | sample_offset  : %i", stream->sample_offset[i]);
                TRACE_1(DEMUX, "  | sample_size    : %i", stream->sample_size[i]);
                TRACE_1(DEMUX, "  | sample_timecode: %i", stream->sample_pts[i]);
            }
        }
    }

#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

static void computeSamplesDatasTrack(MediaStream_t *track)
{
    if (track)
    {
        uint64_t totalbytes = 0;
        bool cbr = true;
        int64_t frameinterval = 0;
        bool cfr = true;
        unsigned j = 0;

        if (track->stream_packetized == false)
        {
            track->frame_count = track->sample_count;
        }
        if (track->stream_intracoded)
        {
            track->frame_count_idr = track->frame_count;
        }

        // Audio frame duration
        if (track->stream_type == stream_AUDIO)
        {
            if (track->stream_codec == CODEC_LPCM ||
                track->stream_codec == CODEC_LogPCM ||
                track->stream_codec == CODEC_DPCM ||
                track->stream_codec == CODEC_ADPCM)
            {
                if (track->sampling_rate)
                {
                    track->frame_duration = (1.0 / static_cast<double>(track->sampling_rate));
                }
            }
            else
            {
                if (track->sample_dts && track->sample_count >= 2)
                {
                    track->frame_duration = static_cast<double>(track->sample_dts[1] - track->sample_dts[0]);
                    track->frame_duration /= 1000; // µs to  ms
                }
            }
        }
        // Video frame duration
        if (track->stream_type == stream_VIDEO && track->frame_duration == 0 && track->framerate != 0)
        {
            track->frame_duration = 1000.0 / track->framerate;
        }

        // Video frame interval
        if (track->stream_type == stream_VIDEO)
        {
            // FIXME this is not reliable whenever using B frames
            if (track->sample_dts && track->sample_count >= 2)
                frameinterval = track->sample_dts[1] - track->sample_dts[0];
        }

        unsigned samplesizerefid = 10;
        if (track->sample_count <= samplesizerefid)
        {
            if (track->sample_count > 1)
                samplesizerefid = track->sample_count - 1;
            else
                samplesizerefid = 0;
        }

        // Iterate on each sample
        for (j = 0; j < track->sample_count; j++)
        {
            totalbytes += track->sample_size[j];

            if (samplesizerefid)
                if (track->sample_size[j] > (track->sample_size[samplesizerefid] + 1) || track->sample_size[j] < (track->sample_size[samplesizerefid] - 1))
                    cbr = false; // TODO find a reference // TODO not use TAGS

            if (track->stream_packetized == true)
            {
                if (track->sample_type[j] == sample_VIDEO_SYNC ||
                    track->sample_pts[j] || track->sample_dts[j])
                {
                    track->frame_count++;
                }
            }
/*
            if (j > 0)
            {
                // FIXME this is not reliable whenever using B frames
                if ((t->sample_dts[j] - t->sample_dts[j - 1]) != frameinterval)
                {
                    cfr = false;
                    TRACE_ERROR(DEMUX, "dts F: %lli != %lli (j: %u)", (t->sample_dts[j] - t->sample_dts[j - 1]), frameinterval, j);
                    TRACE_ERROR(DEMUX, "dts F: %lli !=  %lli ", (t->sample_pts[j] - t->sample_pts[j - 1]), frameinterval);
                }
            }
*/
        }

        // Set bitrate mode
        if (track->bitrate_mode == BITRATE_UNKNOWN)
        {
            if (cbr == true)
            {
                track->bitrate_mode = BITRATE_CBR;
            }
            else
            {
                // TODO check if we have AVBR / CVBR ?
                track->bitrate_mode = BITRATE_VBR;
            }
        }
/*
        // Set framerate mode
        if (t->framerate_mode == FRAMERATE_UNKNOWN)
        {
            if (cfr == true)
            {
                t->framerate_mode = FRAMERATE_CFR;
            }
            else
            {
                t->framerate_mode = FRAMERATE_VFR;
            }
        }
*/
        // Set stream size
        if (track->stream_size == 0)
        {
            track->stream_size = totalbytes;
        }

        // Set stream duration
        if (track->stream_duration_ms == 0)
        {
            track->stream_duration_ms = (double)track->frame_count * track->frame_duration;
        }

        // Set gross bitrate value (in bps)
        if (track->bitrate_avg == 0 && track->stream_duration_ms != 0)
        {
            track->bitrate_avg = (unsigned int)round(((double)track->stream_size / (double)(track->stream_duration_ms)));
            track->bitrate_avg *= 1000; // ms to s
            track->bitrate_avg *= 8; // B to b
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief PCM sample size hack
 *
 * PCM sample size can be recomputed if the informations gathered from the
 * containers seems wrong (like the sample size). This will also trigger a new
 * bitrate computation.
 */
bool computePCMSettings(MediaStream_t *track)
{
    bool retcode = SUCCESS;
    uint32_t sample_size_cbr = track->channel_count * (track->bit_per_sample / 8);

    // First, check if the hack is needed
    if (track->sample_count > 0 && track->sample_size[0] != sample_size_cbr)
    {
        TRACE_ERROR(DEMUX, BLD_GREEN "computePCMSettings()" CLR_RESET);

        track->sample_per_frames = 1;
        track->stream_size = track->sample_count * sample_size_cbr;
        track->bitrate_avg = 0; // reset bitrate

        for (unsigned i = 0; i < track->sample_count; i++)
        {
            track->sample_size[i] = sample_size_cbr;
        }
    }

    return retcode;
}

bool computeCodecs(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeCodecs()" CLR_RESET);
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
        if (media->tracks_audio[i])
        {

            if (media->tracks_audio[i]->stream_codec == CODEC_UNKNOWN)
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
    }

    return retcode;
}

/* ************************************************************************** */

bool computeAspectRatios(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeAspectRatios()" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        MediaStream_t *t = media->tracks_video[i];
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
    TRACE_INFO(DEMUX, BLD_GREEN "computeSamplesDatas()" CLR_RESET);
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

uint64_t computeStreamMemory(MediaStream_t *stream)
{
    uint64_t mem = 0;

    if (stream)
    {
        mem += sizeof(*stream);

        if (stream->stream_encoder) mem += strlen(stream->stream_encoder);
        if (stream->track_title) mem += strlen(stream->track_title);
        if (stream->track_languagecode) mem += strlen(stream->track_languagecode);
        if (stream->subtitles_name) mem += strlen(stream->subtitles_name);

        mem += stream->sample_count * (4 + 4 + 8 + 8 + 8);
    }
    TRACE_1(DEMUX, "track(x): %u B\n", mem);

    return mem;
}

bool computeMediaMemory(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeMediaMemory()" CLR_RESET);
    bool retcode = SUCCESS;

    uint64_t mem = 0;

    mem += sizeof(*media);

    if (media->creation_app)
    {
        mem += strlen(media->creation_app);
    }
    if (media->creation_lib)
    {
        mem += strlen(media->creation_lib);
    }

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        mem += computeStreamMemory(media->tracks_video[i]);
    }

    for (unsigned i = 0; i < media->tracks_audio_count; i++)
    {
        mem += computeStreamMemory(media->tracks_audio[i]);
    }

    for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
    {
        mem += computeStreamMemory(media->tracks_subt[i]);
    }

    for (unsigned i = 0; i < media->tracks_others_count; i++)
    {
        mem += computeStreamMemory(media->tracks_others[i]);
    }

    media->parsingMemory = mem;
    TRACE_INFO(DEMUX, "media parsing memory: %u B\n", mem);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */
