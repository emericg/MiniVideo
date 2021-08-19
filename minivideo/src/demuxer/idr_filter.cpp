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
 * \date      2011
 */

// minivideo headers
#include "idr_filter.h"
#include "../import.h"
#include "../bitstream_map.h"
#include "../minivideo_typedef.h"
#include "../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* ************************************************************************** */

int idr_filtering(MediaStream_t **stream_ptr,
                  unsigned picture_number, const int picture_extraction_mode)
{
    TRACE_INFO(FILTR, BLD_GREEN "idr_filtering()" CLR_RESET);
    int available_pictures = 0;

    unsigned temporary_totalsamples_idr = 0;
    unsigned temporary_sample_id[999] = {0};

    if (stream_ptr == nullptr || *stream_ptr == nullptr)
    {
        TRACE_ERROR(FILTR, "Invalid MediaStream_t structure!");
    }
    else
    {
        MediaStream_t *map = *stream_ptr;

        // Check if the MediaStream_t is containing video data
        if (map->stream_type != stream_VIDEO)
        {
            TRACE_WARNING(FILTR, "This is not an video MediaStream_t!");
        }

        // Check if we have enough IDR samples inside the video
        if (map->frame_count_idr == 0)
        {
            TRACE_WARNING(FILTR, "No IDR samples inside the stream, 0 pictures will be extracted!", map->frame_count_idr);
            picture_number = 0;
        }
        else if (map->frame_count_idr < picture_number)
        {
            TRACE_WARNING(FILTR, "Not enough IDR samples inside the stream, only %i pictures will be extracted!", map->frame_count_idr);
            picture_number = map->frame_count_idr;
        }

        if (picture_extraction_mode == PICTURE_UNFILTERED)
        {
            // Init bitstream_map_filtered
            MediaStream_t *map_filtered = nullptr;
            int retcode = init_bitstream_map(&map_filtered, map->parameter_count, picture_number);

            // Write bitstream_map_filtered
            if (retcode)
            {
                map_filtered->stream_type = map->stream_type;
                map_filtered->stream_codec = map->stream_codec;

                map_filtered->parameter_count = map->parameter_count;
                map_filtered->sample_count = 0;
                map_filtered->frame_count_idr = 0;

                // Copy SPS and PPS
                for (unsigned i = 0; i < map->parameter_count; i++)
                {
                    map_filtered->parameter_type[i] = map->parameter_type[i];
                    map_filtered->parameter_offset[i] = map->parameter_offset[i];
                    map_filtered->parameter_size[i] = map->parameter_size[i];
                }

                // Then, keep only the IDR
                for (unsigned i = 0, j = 0; i < map->sample_count && j < picture_number; i++)
                {
                    if (map->sample_type[i] == sample_VIDEO_SYNC)
                    {
                        map_filtered->sample_type[j] = map->sample_type[i];
                        map_filtered->sample_size[j] = map->sample_size[i];
                        map_filtered->sample_offset[j] = map->sample_offset[i];
                        map_filtered->sample_pts[j] = map->sample_pts[i];
                        map_filtered->sample_dts[j] = map->sample_dts[i];

                        j++;
                        map_filtered->sample_count++;
                        map_filtered->frame_count_idr++;
                    }
                }

                free(map->sample_type);
                free(map->sample_pts);
                free(map->sample_offset);
                free(map->sample_size);

                map->sample_type = map_filtered->sample_type;
                map->sample_pts = map_filtered->sample_pts;
                map->sample_offset = map_filtered->sample_offset;
                map->sample_size = map_filtered->sample_size;

                available_pictures = picture_number;
            }
        }
        else
        {
            // First cut (remove small frames)
            ////////////////////////////////////////////////////////////////////

            // Compute average samples size
            unsigned payload = 0;
            for (unsigned i = 0; i < map->sample_count; i++)
            {
                if (map->sample_type[i] == sample_VIDEO_SYNC)
                {
                    payload += map->sample_size[i];
                }
            }

            // Used to filter the frames that are below the threshold (33% of the average frame size)
            unsigned frame_sizethreshold = static_cast<unsigned>(((double)payload / (double)map->frame_count_idr) / 1.66);

            for (unsigned i = 0; i < map->sample_count; i++)
            {
                if (map->sample_type[i] == sample_VIDEO_SYNC)
                {
                    if (map->sample_size[i] > frame_sizethreshold)
                    {
                        TRACE_1(FILTR, "IDR %i (size: %i / threshold: %i)", i, map->sample_size[i], frame_sizethreshold);

                        temporary_sample_id[temporary_totalsamples_idr] = i;
                        temporary_totalsamples_idr++;
                    }
                    else
                    {
                        TRACE_1(FILTR, "IDR %i (size: %i / threshold: %i) > REMOVED", i, map->sample_size[i], frame_sizethreshold);
                    }
                }
            }

            TRACE_1(FILTR, "We have a total of %i IDR after the first cut", temporary_totalsamples_idr);
            if (picture_number > temporary_totalsamples_idr)
                picture_number = temporary_totalsamples_idr;

            // Jump between two frames in PICTURE_DISTRIBUTED mode
            int frame_jump = 0;
            if (picture_number > 1)
                frame_jump = std::ceil(temporary_totalsamples_idr / (picture_number-1));
            TRACE_1(FILTR, "frame_jump is %i", frame_jump);

            // Init bitstream_map_filtered
            MediaStream_t *map_filtered = nullptr;
            int retcode = init_bitstream_map(&map_filtered, map->parameter_count, temporary_totalsamples_idr);

            // Write bitstream_map_filtered
            if (retcode)
            {
                map_filtered->stream_type = map->stream_type;
                map_filtered->stream_codec = map->stream_codec;

                map_filtered->sample_count = temporary_totalsamples_idr;
                map_filtered->frame_count_idr = temporary_totalsamples_idr;

                // Copy SPS and PPS
                for (unsigned i = 0; i < map->parameter_count; i++)
                {
                    map_filtered->parameter_type[i] = map->parameter_type[i];
                    map_filtered->parameter_offset[i] = map->parameter_offset[i];
                    map_filtered->parameter_size[i] = map->parameter_size[i];
                }

                // Second cut (frame distribution)
                ////////////////////////////////////////////////////////////////

                for (unsigned i = 0; i < picture_number; i++)
                {
                    if (picture_extraction_mode == PICTURE_ORDERED)
                    {
                        map_filtered->sample_type[i] = map->sample_type[temporary_sample_id[i]];
                        map_filtered->sample_pts[i] = map->sample_pts[temporary_sample_id[i]];

                        map_filtered->sample_offset[i] = map->sample_offset[temporary_sample_id[i]];
                        map_filtered->sample_size[i] = map->sample_size[temporary_sample_id[i]];
                    }
                    else if (picture_extraction_mode == PICTURE_DISTRIBUTED)
                    {
                        map_filtered->sample_type[i] = map->sample_type[temporary_sample_id[i*frame_jump]];
                        map_filtered->sample_pts[i] = map->sample_pts[temporary_sample_id[i*frame_jump]];

                        map_filtered->sample_offset[i] = map->sample_offset[temporary_sample_id[i*frame_jump]];
                        map_filtered->sample_size[i] = map->sample_size[temporary_sample_id[i*frame_jump]];
                    }
                }

                // Recap
                //print_bitstream_map(map);
                //print_bitstream_map(map_filtered);

                // Erase bitstream_map and replace it with bitstream_map_filtered
                //free_bitstream_map(bitstream_map_pointer);
                //*bitstream_map_pointer = map_filtered;

                free(map->sample_type);
                free(map->sample_pts);
                free(map->sample_offset);
                free(map->sample_size);

                map->sample_type = map_filtered->sample_type;
                map->sample_pts = map_filtered->sample_pts;
                map->sample_offset = map_filtered->sample_offset;
                map->sample_size = map_filtered->sample_size;

                // Exit
                available_pictures = picture_number;
            }
        }
    }

    return available_pictures;
}

/* ************************************************************************** */
