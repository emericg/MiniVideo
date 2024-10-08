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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "bitstream_map.h"
#include "minivideo_codecs.h"
#include "minivideo_fourcc.h"
#include "minitraces.h"
#include "minivideo_typedef.h"
#include "decoder/h264/h264_parameterset.h"

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

    if (*stream_ptr != nullptr)
    {
        TRACE_ERROR(DEMUX, "<b> Unable to alloc a new bitstream_map: not null!");
        retcode = FAILURE;
    }
    else
    {
        *stream_ptr = (MediaStream_t*)calloc(1, sizeof(MediaStream_t));
        if (*stream_ptr == nullptr)
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

                if ((*stream_ptr)->parameter_type == nullptr ||
                    (*stream_ptr)->parameter_size == nullptr ||
                    (*stream_ptr)->parameter_offset == nullptr)
                {
                    TRACE_ERROR(DEMUX, "<b> Unable to alloc bitstream_map > sample_type / sample_size / sample_offset / sample_timecode!");

                    if ((*stream_ptr)->parameter_type != nullptr)
                        free((*stream_ptr)->parameter_type);
                    if ((*stream_ptr)->parameter_size != nullptr)
                        free((*stream_ptr)->parameter_size);
                    if ((*stream_ptr)->parameter_offset != nullptr)
                        free((*stream_ptr)->parameter_offset);

                    free(*stream_ptr);
                    *stream_ptr = nullptr;
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

                if ((*stream_ptr)->sample_type == nullptr ||
                    (*stream_ptr)->sample_size == nullptr ||
                    (*stream_ptr)->sample_offset == nullptr ||
                    (*stream_ptr)->sample_pts == nullptr ||
                    (*stream_ptr)->sample_dts == nullptr)
                {
                    TRACE_ERROR(DEMUX, "<b> Unable to alloc bitstream_map > sample_type / sample_size / sample_offset / sample_timecode!");

                    if ((*stream_ptr)->sample_type != nullptr)
                        free((*stream_ptr)->sample_type);
                    if ((*stream_ptr)->sample_size != nullptr)
                        free((*stream_ptr)->sample_size);
                    if ((*stream_ptr)->sample_offset != nullptr)
                        free((*stream_ptr)->sample_offset);
                    if ((*stream_ptr)->sample_pts != nullptr)
                        free((*stream_ptr)->sample_pts);
                    if ((*stream_ptr)->sample_dts != nullptr)
                        free((*stream_ptr)->sample_dts);

                    free(*stream_ptr);
                    *stream_ptr = nullptr;
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
    if ((*stream_ptr) != nullptr)
    {
        TRACE_INFO(DEMUX, "<b> " BLD_BLUE "free_bitstream_map()" CLR_RESET);

        // Strings
        free((*stream_ptr)->stream_encoder);
        (*stream_ptr)->stream_encoder = nullptr;
        free((*stream_ptr)->track_title);
        (*stream_ptr)->track_title = nullptr;
        free((*stream_ptr)->track_languagecode);
        (*stream_ptr)->track_languagecode = nullptr;
        free((*stream_ptr)->subtitles_name);
        (*stream_ptr)->subtitles_name = nullptr;

        // Codec privates
        if ((*stream_ptr)->avcC)
        {
            for (unsigned i = 0; i < (*stream_ptr)->avcC->sps_count && i < MAX_SPS; i++)
                freeSPS(&(*stream_ptr)->avcC->sps_array[i]);
            free(*(*stream_ptr)->avcC->sps_array);
            free((*stream_ptr)->avcC->sps_sample_offset);
            free((*stream_ptr)->avcC->sps_sample_size);
            for (unsigned i = 0; i < (*stream_ptr)->avcC->pps_count && i < MAX_PPS; i++)
                freePPS(&(*stream_ptr)->avcC->pps_array[i]);
            free(*(*stream_ptr)->avcC->pps_array);
            free((*stream_ptr)->avcC->pps_sample_offset);
            free((*stream_ptr)->avcC->pps_sample_size);

            free((*stream_ptr)->avcC);
        }
        free((*stream_ptr)->hvcC);
        free((*stream_ptr)->vvcC);
        free((*stream_ptr)->vpcC);
        free((*stream_ptr)->av1C);
        free((*stream_ptr)->dvcC);
        free((*stream_ptr)->mvcC);

        // "Parameter sets" arrays
        free((*stream_ptr)->parameter_type);
        (*stream_ptr)->parameter_type = nullptr;

        free((*stream_ptr)->parameter_size);
        (*stream_ptr)->parameter_size = nullptr;

        free((*stream_ptr)->parameter_offset);
        (*stream_ptr)->parameter_offset = nullptr;

        // Samples arrays
        free((*stream_ptr)->sample_type);
        (*stream_ptr)->sample_type = nullptr;

        free((*stream_ptr)->sample_size);
        (*stream_ptr)->sample_size = nullptr;

        free((*stream_ptr)->sample_offset);
        (*stream_ptr)->sample_offset = nullptr;

        free((*stream_ptr)->sample_pts);
        (*stream_ptr)->sample_pts = nullptr;

        free((*stream_ptr)->sample_dts);
        (*stream_ptr)->sample_dts = nullptr;

        // The MediaStream_t itself
        free(*stream_ptr);
        *stream_ptr = nullptr;

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

    if (stream == nullptr)
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

        if (track->stream_type == stream_AUDIO)
        {
            if (track->stream_codec == CODEC_LPCM ||
                track->stream_codec == CODEC_LogPCM ||
                track->stream_codec == CODEC_DPCM ||
                track->stream_codec == CODEC_ADPCM)
            {
                // PCM
                if (track->sampling_rate)
                {
                    track->frame_duration = (1.0 / static_cast<double>(track->sampling_rate));
                    track->sample_duration = (1000000.0 / static_cast<double>(track->sampling_rate));
                    track->sample_per_frames = 1;
                }
            }
            else
            {
                // Audio frame duration
                if (track->sample_dts && track->sample_count >= 2)
                {
                    track->frame_duration = static_cast<double>(track->sample_dts[1] - track->sample_dts[0]);
                    track->frame_duration /= 1000.0; // µs to ms
                }
                // Audio frame duration (backup method)
                if (track->frame_duration <= 0.0 && track->sample_count > 0)
                {
                    track->frame_duration = (track->stream_duration_ms) / track->sample_count;
                }

                // Audio sample duration
                if (track->sampling_rate)
                {
                    track->sample_duration = (1000000.0 / static_cast<double>(track->sampling_rate));
                }

                // Audio sample per frames
                if (track->frame_duration > 0.0 && track->sample_duration > 0.0)
                {
                    track->sample_per_frames = static_cast<unsigned>((track->sample_duration * 1000.0) / track->frame_duration);
                }
            }
        }

        if (track->stream_type == stream_VIDEO)
        {
            // Video frame duration
            if (track->frame_duration == 0 && track->framerate > 0.0)
            {
                track->frame_duration = 1000.0 / track->framerate;
            }

            // Video frame interval // FIXME this is not reliable whenever using B frames
            if (track->sample_dts && track->sample_count >= 2)
                frameinterval = track->sample_dts[1] - track->sample_dts[0];

            // Color space
            if (track->color_matrix == 0)
                track->color_space = CLR_RGB;
            else if (track->color_matrix == 8)
                track->color_space = CLR_YCgCo;
            else if (track->color_matrix == 14)
                track->color_space = CLR_ICtCp;
            else
                track->color_space = CLR_YCbCr;
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

            if (track->stream_packetized == true && track->stream_intracoded == false)
            {
                if (track->sample_type[j] == sample_VIDEO_SYNC ||
                    track->sample_pts[j] || track->sample_dts[j])
                {
                    track->frame_count++;
                }
            }
            else
            {
                track->frame_count++;
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
            track->bitrate_avg = (unsigned)round(((double)track->stream_size / (double)(track->stream_duration_ms)));
            track->bitrate_avg *= 1000; // ms to s
            track->bitrate_avg *= 8; // B to b
        }

        if (track->stream_intracoded == true)
        {
            track->frame_count_idr = track->sample_count;
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief PCM sample size hack
 *
 * PCM sample size can be recomputed if the information gathered from the
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
        TRACE_INFO(DEMUX, BLD_GREEN "computePCMSettings()" CLR_RESET);

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

    for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
    {
        if (media->tracks_subt[i])
        {
            if (media->tracks_subt[i]->stream_codec == CODEC_UNKNOWN)
            {
                media->tracks_subt[i]->stream_codec = getCodecFromFourCC(media->tracks_subt[i]->stream_fcc);
            }
        }
    }

    for (unsigned i = 0; i < media->tracks_others_count; i++)
    {
        if (media->tracks_others[i])
        {
            if (media->tracks_others[i]->stream_codec == CODEC_UNKNOWN)
            {
                media->tracks_others[i]->stream_codec = getCodecFromFourCC(media->tracks_others[i]->stream_fcc);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

bool computeCodecsSpecifics(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeCodecsSpecifics()" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        if (media->tracks_video[i])
        {
            // shortcuts
            MediaStream_t *track = media->tracks_video[i];
            codecprivate_avcC_t *avcC = media->tracks_video[i]->avcC;
            codecprivate_hvcC_t *hvcC = media->tracks_video[i]->hvcC;
            codecprivate_vvcC_t *vvcC = media->tracks_video[i]->vvcC;
            codecprivate_vpcC_t *vpcC = media->tracks_video[i]->vpcC;
            codecprivate_av1C_t *av1C = media->tracks_video[i]->av1C;

            if (track->stream_codec == CODEC_H264 && track->avcC)
            {
                if (avcC->sps_count > 0 && avcC->sps_array[0])
                {
                    // H.264 profile & level
                    track->stream_codec_profile = getH264CodecProfile(
                        avcC->sps_array[0]->profile_idc,
                        avcC->sps_array[0]->constraint_setX_flag[0],
                        avcC->sps_array[0]->constraint_setX_flag[1],
                        avcC->sps_array[0]->constraint_setX_flag[2],
                        avcC->sps_array[0]->constraint_setX_flag[3],
                        avcC->sps_array[0]->constraint_setX_flag[4],
                        avcC->sps_array[0]->constraint_setX_flag[5]);
                    track->video_level = static_cast<double>(avcC->AVCLevelIndication) / 10.0;

                    track->max_ref_frames = avcC->sps_array[0]->max_num_ref_frames;
                    track->color_depth = avcC->sps_array[0]->BitDepthY;

                    // Chroma
                    if (avcC->sps_array[0]->chroma_format_idc == 0)
                        track->chroma_subsampling = CHROMA_SS_400;
                    else if (avcC->sps_array[0]->chroma_format_idc == 1)
                        track->chroma_subsampling = CHROMA_SS_420;
                    else if (avcC->sps_array[0]->chroma_format_idc == 2)
                        track->chroma_subsampling = CHROMA_SS_422;
                    else if (avcC->sps_array[0]->chroma_format_idc == 3)
                        track->chroma_subsampling = CHROMA_SS_444;
                    else
                        track->chroma_subsampling = CHROMA_SS_420;

                    if (avcC->sps_array[0]->vui)
                    {
                        track->color_range = avcC->sps_array[0]->vui->video_full_range_flag;
                        track->color_primaries = avcC->sps_array[0]->vui->colour_primaries;
                        track->color_transfer = avcC->sps_array[0]->vui->transfer_characteristics;
                        track->color_matrix = avcC->sps_array[0]->vui->matrix_coefficients;

                        if (avcC->sps_array[0]->vui->chroma_loc_info_present_flag)
                        {
                            if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 0) track->chroma_location = CHROMA_LOC_LEFT;
                            else if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 1) track->chroma_location = CHROMA_LOC_CENTER;
                            else if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 2) track->chroma_location = CHROMA_LOC_TOPLEFT;
                            else if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 3) track->chroma_location = CHROMA_LOC_TOP;
                            else if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 4) track->chroma_location = CHROMA_LOC_BOTTOMLEFT;
                            else if (avcC->sps_array[0]->vui->chroma_sample_loc_type_top_field == 5) track->chroma_location = CHROMA_LOC_BOTTOM;
                        }
                    }

                    if (avcC->pps_count > 0 && avcC->pps_array[0])
                    {
                        track->h264_feature_cabac = avcC->pps_array[0]->entropy_coding_mode_flag;
                        track->h264_feature_8x8_blocks = avcC->pps_array[0]->transform_8x8_mode_flag;
                    }
                }
                else
                {
                    track->stream_codec_profile = getH264CodecProfile(avcC->AVCProfileIndication);
                    track->video_level = static_cast<double>(avcC->AVCLevelIndication) / 10.0;
                }
            }
            else if (track->stream_codec == CODEC_H265 && track->hvcC)
            {
                // Handle H.265 profile & level
                track->stream_codec_profile = getH265CodecProfile(hvcC->general_profile_idc);
                track->video_level = static_cast<double>(hvcC->general_level_idc) / 30.0;
                track->color_depth = hvcC->bitDepthLumaMinus8 + 8;
            }
            else if (track->stream_codec == CODEC_H266 && track->vvcC)
            {
                // Handle H.266 profile & level
                // TODO
            }
            else if (track->vpcC)
            {
                // Handle VPx profile & level
                if (track->stream_codec == CODEC_VP9)
                {
                    if (vpcC->profile == 0) track->stream_codec_profile = PROF_VP9_0;
                    else if (vpcC->profile == 1) track->stream_codec_profile = PROF_VP9_1;
                    else if (vpcC->profile == 2) track->stream_codec_profile = PROF_VP9_2;
                    else if (vpcC->profile == 3) track->stream_codec_profile = PROF_VP9_3;
                }
                else if (track->stream_codec == CODEC_VP8)
                {
                    if (vpcC->profile == 0) track->stream_codec_profile = PROF_VP8_0;
                    else if (vpcC->profile == 1) track->stream_codec_profile = PROF_VP8_1;
                }

                track->video_level = static_cast<double>(vpcC->level) / 10.0;

                track->color_depth = vpcC->bitDepth;
                track->color_range = vpcC->videoFullRangeFlag;

                if (vpcC->chromaSubsampling == 0 || vpcC->chromaSubsampling == 1)
                    track->chroma_subsampling = CHROMA_SS_420;
                else if (vpcC->chromaSubsampling == 2)
                    track->chroma_subsampling = CHROMA_SS_422;
                else if (vpcC->chromaSubsampling == 3)
                    track->chroma_subsampling = CHROMA_SS_444;

                if (vpcC->chromaSubsampling == 1)
                    track->chroma_location = CHROMA_LOC_TOPLEFT;
                else
                    track->chroma_location = CHROMA_LOC_LEFT;

                track->color_primaries = vpcC->colourPrimaries;
                track->color_transfer = vpcC->transferCharacteristics;
                track->color_matrix = vpcC->matrixCoefficients;
            }
            else if (track->av1C)
            {
                // Handle AV1 profile & level
                if (av1C->seq_profile == 0) track->stream_codec_profile = PROF_AV1_Main;
                else if (av1C->seq_profile == 1) track->stream_codec_profile = PROF_AV1_High;
                else if (av1C->seq_profile == 2) track->stream_codec_profile = PROF_AV1_Professional;

                if (av1C->seq_level_idx_0 == 0) track->video_level = 2.0;
                else if (av1C->seq_level_idx_0 == 1) track->video_level = 2.1;
                else if (av1C->seq_level_idx_0 == 2) track->video_level = 2.2;
                else if (av1C->seq_level_idx_0 == 3) track->video_level = 2.3;
                else if (av1C->seq_level_idx_0 == 4) track->video_level = 3.0;
                else if (av1C->seq_level_idx_0 == 5) track->video_level = 3.1;
                else if (av1C->seq_level_idx_0 == 6) track->video_level = 3.1;
                else if (av1C->seq_level_idx_0 == 7) track->video_level = 3.1;
                else if (av1C->seq_level_idx_0 == 8) track->video_level = 4.0;
                else if (av1C->seq_level_idx_0 == 9) track->video_level = 4.1;
                else if (av1C->seq_level_idx_0 == 10) track->video_level = 4.2;
                else if (av1C->seq_level_idx_0 == 11) track->video_level = 4.3;
                else if (av1C->seq_level_idx_0 == 12) track->video_level = 5.0;
                else if (av1C->seq_level_idx_0 == 13) track->video_level = 5.1;
                else if (av1C->seq_level_idx_0 == 14) track->video_level = 5.2;
                else if (av1C->seq_level_idx_0 == 15) track->video_level = 5.3;
                else if (av1C->seq_level_idx_0 == 16) track->video_level = 6.0;
                else if (av1C->seq_level_idx_0 == 17) track->video_level = 6.1;
                else if (av1C->seq_level_idx_0 == 18) track->video_level = 6.2;
                else if (av1C->seq_level_idx_0 == 19) track->video_level = 6.3;
                else if (av1C->seq_level_idx_0 == 20) track->video_level = 7.0;
                else if (av1C->seq_level_idx_0 == 21) track->video_level = 7.1;
                else if (av1C->seq_level_idx_0 == 22) track->video_level = 7.2;
                else if (av1C->seq_level_idx_0 == 23) track->video_level = 7.3;

                if (av1C->twelve_bit) track->color_depth = 12;
                else if (av1C->high_bitdepth) track->color_depth = 10;
                else track->color_depth = 8;

                if (av1C->chroma_subsampling_x == 0 && av1C->chroma_subsampling_y == 0 && av1C->monochrome == 0)
                    track->chroma_subsampling = CHROMA_SS_444;
                else if (av1C->chroma_subsampling_x == 1 && av1C->chroma_subsampling_y == 0 && av1C->monochrome == 0)
                    track->chroma_subsampling = CHROMA_SS_422;
                else if (av1C->chroma_subsampling_x == 1 && av1C->chroma_subsampling_y == 1 && av1C->monochrome == 0)
                    track->chroma_subsampling = CHROMA_SS_420;
                else if (av1C->chroma_subsampling_x == 1 && av1C->chroma_subsampling_y == 1 && av1C->monochrome == 1)
                    track->chroma_subsampling = CHROMA_SS_400;

                if (av1C->chroma_sample_position == 1)
                    track->chroma_location = CHROMA_LOC_LEFT;
                else if (av1C->chroma_sample_position == 2)
                    track->chroma_location = CHROMA_LOC_TOPLEFT;
            }

            else if (track->stream_codec == CODEC_PRORES_422_PROXY ||
                track->stream_codec == CODEC_PRORES_422_LT ||
                track->stream_codec == CODEC_PRORES_422 ||
                track->stream_codec == CODEC_PRORES_422_HQ)
            {
                track->chroma_subsampling = CHROMA_SS_422;
            }
            else if (track->stream_codec == CODEC_PRORES_4444 ||
                     track->stream_codec == CODEC_PRORES_4444_XQ)
            {
                track->chroma_subsampling = CHROMA_SS_444;
            }
            else if (track->stream_codec == CODEC_PRORES_RAW ||
                     track->stream_codec == CODEC_PRORES_RAW_HQ)
            {
                // RAW variants are not using YUV pixel subsampling
                track->chroma_subsampling = CHROMA_SS_UNKNOWN;
                track->color_depth = 16;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

bool computeHDR(MediaFile_t *media)
{
    TRACE_INFO(DEMUX, BLD_GREEN "computeHDR()" CLR_RESET);
    bool retcode = SUCCESS;

    for (unsigned i = 0; i < media->tracks_video_count; i++)
    {
        if (media->tracks_video[i])
        {
            if (media->tracks_video[i]->color_transfer == COLOR_TRC_BT2020_10 ||
                media->tracks_video[i]->color_transfer == COLOR_TRC_ARIB_STD_B67)
            {
                media->tracks_video[i]->hdr_mode = HLG;
            }
            if (media->tracks_video[i]->color_transfer == COLOR_TRC_SMPTE2084)
            {
                if (media->tracks_video[i]->dvcC)
                {
                    media->tracks_video[i]->hdr_mode = DolbyVision;
                }
                else
                {
                    media->tracks_video[i]->hdr_mode = HDR10;
                }
            }

            if (media->tracks_video[i]->dvcC) // hack
            {
                media->tracks_video[i]->hdr_mode = DolbyVision;
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
            // First pass on PAR
            if (t->pixel_aspect_ratio_h && t->pixel_aspect_ratio_v)
            {
                // (if set by the container)
                t->pixel_aspect_ratio = t->pixel_aspect_ratio_h / (double)t->pixel_aspect_ratio_v;
            }
            else
            {
                t->pixel_aspect_ratio_h = 1;
                t->pixel_aspect_ratio_v = 1;
                t->pixel_aspect_ratio = 1.0;
            }

            // First pass on VAR
            if (!t->video_aspect_ratio_h && t->video_aspect_ratio_v)
            {
                // (if set by the container)
                t->video_aspect_ratio = t->video_aspect_ratio_h / (double)t->video_aspect_ratio_v;
            }
            else if (t->width && t->height)
            {
                // (if computed from video resolution)
                t->video_aspect_ratio_h = t->width;
                t->video_aspect_ratio_v = t->height;
                t->video_aspect_ratio = t->video_aspect_ratio_h / (double)t->video_aspect_ratio_v;
            }

            // First pass on DAR // Handle PAR
            if (t->display_aspect_ratio_h && t->display_aspect_ratio_v)
            {
                // (if set by the container)
                t->display_aspect_ratio = t->display_aspect_ratio_h / (double)t->display_aspect_ratio_v;
            }
            else
            {
                //if (t->pixel_aspect_ratio != 1.0)
                if (t->pixel_aspect_ratio < 0.99 || t->pixel_aspect_ratio > 1.01)
                {
                    // (if computed from PAR)
                    if (t->pixel_aspect_ratio >= 1.0) {
                        t->display_aspect_ratio_h = t->video_aspect_ratio_h * t->pixel_aspect_ratio;
                        t->display_aspect_ratio_v = t->video_aspect_ratio_v;
                        t->width_display = t->width * t->pixel_aspect_ratio;
                        t->height_display = t->height;
                    } else {
                        t->display_aspect_ratio_h = t->video_aspect_ratio_h;
                        t->display_aspect_ratio_v = t->video_aspect_ratio_v * t->pixel_aspect_ratio;
                        t->width_display = t->width;
                        t->height_display = t->height * t->pixel_aspect_ratio;
                    }
                    t->display_aspect_ratio = t->display_aspect_ratio_h / (double)t->display_aspect_ratio_v;
                }
                else
                {
                    // (if same as VAR)
                    t->display_aspect_ratio_h = t->video_aspect_ratio_h;
                    t->display_aspect_ratio_v = t->video_aspect_ratio_v;
                    t->display_aspect_ratio = t->video_aspect_ratio;
                    t->width_display = t->width;
                    t->height_display = t->height;
                }
            }

            // Second pass on DAR // Handle rotation
            if (t->video_rotation == 1 || t->video_rotation == 3)
            {
                // def
                unsigned temp = t->width_display;
                t->width_display = t->height_display;
                t->height_display = temp;

                // ar
                temp = t->display_aspect_ratio_h;
                t->display_aspect_ratio_h = t->display_aspect_ratio_v;
                t->display_aspect_ratio_v = temp;
                t->display_aspect_ratio = (double)t->width_display / (double)t->height_display;
            }
        }
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

    for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
    {
        if (media->tracks_subt[i])
        {
            computeSamplesDatasTrack(media->tracks_subt[i]);
        }
    }

    for (unsigned i = 0; i < media->tracks_others_count; i++)
    {
        if (media->tracks_others[i])
        {
            computeSamplesDatasTrack(media->tracks_others[i]);
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
