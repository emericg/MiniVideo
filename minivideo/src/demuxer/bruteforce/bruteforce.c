/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
 * \file      bruteforce.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>

// minivideo headers
#include "../../minitraces.h"
#include "../../typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "bruteforce.h"

#define INITIAL_SEARCH_WINDOW  4194304 // 4MiB
#define GUESS_BUFFERSIZE        262144 // 256 KiB

/* ************************************************************************** */

/*!
 * \brief Parse a file, the brute-force way.
 * \param *video A pointer to a VideoFile_t structure.
 * \param video_codec docme.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * \todo Handle idr_only == false.
 * \todo Handle INITIAL_SEARCH_WINDOW.
 *
 * This parser is just brutally searching for 3 bytes start codes, for instance
 * 0x000001 for mpeg videos and 0x9D012A for VP8 videos.
 *
 * It will only successfuly parse "ES" files containing only one audio or video
 * track.
 */
int bruteforce_fileParse(VideoFile_t *video, AVCodec_e video_codec)
{
    TRACE_INFO(DEMUX, GREEN "bruteforce_fileParse()\n" RESET);

    // Search parameters
    int64_t search_offset = 0;
    uint32_t current_byte = 0;
    uint32_t next_byte = 0;

    // Init bitstream and bitstream_map
    Bitstream_t *bitstr = init_bitstream(video, NULL);
    int retcode = init_bitstream_map(&video->tracks_video[0], 999999);

    if (bitstr != NULL && video->tracks_video[0] != NULL)
    {
        video->tracks_video[0]->stream_type = stream_VIDEO;
        video->tracks_video[0]->stream_level = stream_level_ES;
        video->tracks_video[0]->stream_codec = video_codec;
        video->tracks_video[0]->sample_alignment = true;

        // Force alignment, because start_code_prefix are always byte-aligned
        if (bitstream_force_alignment(bitstr) == true)
        {
            int current_startcode_size = 0;

            // Search for start_code_prefix in the bitstream
            while ((search_offset < (video->file_size - 32))/* &&
                   (search_offset > INITIAL_SEARCH_WINDOW && temporary_totalsamples == 0)*/)
            {
                current_byte = read_byte_aligned(bitstr);
                search_offset++;

                if (current_byte == 0x00)
                {
                    current_startcode_size++;
                }
                else
                {
                    // We have a full 0x000001 start code
                    if (current_byte == 0x01 && current_startcode_size > 2)
                    {
                        // Determine the type of the sample detected
                        next_byte = next_byte_aligned(bitstr);
                        if (next_byte == 0x65 || next_byte == 0x67 || next_byte == 0x68)
                        {
                            // Write results into the bitstream_map
                            video->tracks_video[0]->sample_offset[video->tracks_video[0]->sample_count] = bitstream_get_absolute_byte_offset(bitstr);
                            video->tracks_video[0]->sample_size[video->tracks_video[0]->sample_count] = GUESS_BUFFERSIZE;
                            video->tracks_video[0]->sample_pts[video->tracks_video[0]->sample_count] = -1;

                            // Set size of the previous frame, now that we know where it stops
                            if (video->tracks_video[0]->sample_count > 0)
                                video->tracks_video[0]->sample_size[video->tracks_video[0]->sample_count - 1] = bitstream_get_absolute_byte_offset(bitstr) - video->tracks_video[0]->sample_offset[video->tracks_video[0]->sample_count - 1];

                            if (next_byte == 0x65)
                            {
                                video->tracks_video[0]->sample_type[video->tracks_video[0]->sample_count] = sample_VIDEO_IDR;

                                video->tracks_video[0]->sample_count++;
                                video->tracks_video[0]->sample_count_idr++;
                                TRACE_1(DEMUX, "* IDR nal unit found at byte offset %i\n", bitstream_get_absolute_byte_offset(bitstr));
                            }
                            else
                            {
                                video->tracks_video[0]->sample_type[video->tracks_video[0]->sample_count] = sample_VIDEO_PARAM;

                                video->tracks_video[0]->sample_count++;
                                TRACE_1(DEMUX, "* SPS or PPS nal unit found at byte offset %i\n", bitstream_get_absolute_byte_offset(bitstr));
                            }
                        }
                    }

                    // Reset search
                    current_startcode_size = 0;
                }
            }

            if (video->tracks_video[0]->sample_count == 0)
            {
                TRACE_ERROR(DEMUX, "* No NAL Unit have been found in this bitstream!\n");
                retcode = FAILURE;
            }
            else
            {
                // Set the size of the last sample
                video->tracks_video[0]->sample_size[video->tracks_video[0]->sample_count - 1] = video->file_size - video->tracks_video[0]->sample_offset[video->tracks_video[0]->sample_count - 1];

#if ENABLE_DEBUG
                TRACE_INFO(DEMUX, "bitstream_map->totalsamples = %i\n", video->tracks_video[0]->sample_count);
                int i = 0;
                for (i = 0; i < video->tracks_video[0]->sample_count; i++)
                {
                    TRACE_1(DEMUX, "bitstream_map->sample_offset[%i] = %i\n", i, video->tracks_video[0]->sample_offset[i]);
                    TRACE_1(DEMUX, "bitstream_map->sample_size[%i] = %i\n", i, video->tracks_video[0]->sample_size[i]);
                }
#endif /* ENABLE_DEBUG */
            }
        }

        // Free bitstream
        free_bitstream(&bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
