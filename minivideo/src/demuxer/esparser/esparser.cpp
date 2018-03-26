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
 * \file      esparser.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// minivideo headers
#include "esparser.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

#define INITIAL_SEARCH_WINDOW  4194304 // 4MiB
#define GUESS_BUFFERSIZE        262144 // 256 KiB

/* ************************************************************************** */

int es_fileParse(MediaFile_t *media, Codecs_e video_codec)
{
    TRACE_INFO(DEMUX, BLD_GREEN "es_fileParse()" CLR_RESET);

    // Search parameters
    int64_t search_offset = 0;
    uint32_t current_byte = 0;
    uint32_t next_byte = 0;

    // Init bitstream and bitstream_map
    Bitstream_t *bitstr = init_bitstream(media, NULL);
    int retcode = init_bitstream_map(&media->tracks_video[0], 0, 999999);
    media->tracks_video_count = 1;

    if (bitstr != NULL && media->tracks_video[0] != NULL)
    {
        media->tracks_video[0]->stream_type = stream_VIDEO;
        media->tracks_video[0]->stream_codec = video_codec;
        media->tracks_video[0]->stream_packetized = false;

        // Force alignment, because start_code_prefix are always byte-aligned
        if (bitstream_force_alignment(bitstr) == true)
        {
            int current_startcode_size = 0;

            // Search for start_code_prefix in the bitstream
            while ((search_offset < (media->file_size - 32))/* &&
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
                            media->tracks_video[0]->sample_offset[media->tracks_video[0]->sample_count] = bitstream_get_absolute_byte_offset(bitstr);
                            media->tracks_video[0]->sample_size[media->tracks_video[0]->sample_count] = GUESS_BUFFERSIZE;
                            media->tracks_video[0]->sample_pts[media->tracks_video[0]->sample_count] = -1;

                            // Set size of the previous frame, now that we know where it stops
                            if (media->tracks_video[0]->sample_count > 0)
                                media->tracks_video[0]->sample_size[media->tracks_video[0]->sample_count - 1] = bitstream_get_absolute_byte_offset(bitstr) - media->tracks_video[0]->sample_offset[media->tracks_video[0]->sample_count - 1];

                            if (next_byte == 0x65)
                            {
                                media->tracks_video[0]->sample_type[media->tracks_video[0]->sample_count] = sample_VIDEO_SYNC;

                                media->tracks_video[0]->sample_count++;
                                media->tracks_video[0]->frame_count_idr++;
                                TRACE_1(DEMUX, "* IDR nal unit found at byte offset %i", bitstream_get_absolute_byte_offset(bitstr));
                            }
                            else
                            {
                                media->tracks_video[0]->sample_type[media->tracks_video[0]->sample_count] = sample_VIDEO_PARAM;
                                media->tracks_video[0]->sample_count++;
                                TRACE_1(DEMUX, "* SPS or PPS nal unit found at byte offset %i", bitstream_get_absolute_byte_offset(bitstr));
                            }
                        }
                    }

                    // Reset search
                    current_startcode_size = 0;
                }
            }

            if (media->tracks_video[0]->sample_count == 0)
            {
                TRACE_ERROR(DEMUX, "* No NAL Unit have been found in this bitstream!");
                retcode = FAILURE;
            }
            else
            {
                // Set the size of the last sample
                media->tracks_video[0]->sample_size[media->tracks_video[0]->sample_count - 1] = media->file_size - media->tracks_video[0]->sample_offset[media->tracks_video[0]->sample_count - 1];

#if ENABLE_DEBUG
                TRACE_INFO(DEMUX, "bitstream_map->totalsamples = %i", media->tracks_video[0]->sample_count);
                for (unsigned i = 0; i < media->tracks_video[0]->sample_count; i++)
                {
                    TRACE_1(DEMUX, "bitstream_map->sample_offset[%i] = %i", i, media->tracks_video[0]->sample_offset[i]);
                    TRACE_1(DEMUX, "bitstream_map->sample_size[%i] = %i", i, media->tracks_video[0]->sample_size[i]);
                }
#endif // ENABLE_DEBUG
            }
        }

        // Free bitstream
        free_bitstream(&bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
