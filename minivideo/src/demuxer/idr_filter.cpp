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
 * \file      idr_filter.c
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
    int retcode = FAILURE;

    unsigned i = 0;
    unsigned temporary_totalsamples_idr = 0;
    unsigned temporary_sample_id[999] = {0};

    if (stream_ptr == NULL || *stream_ptr == NULL)
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
            TRACE_1(FILTR, "PICTURE_UNFILTERED is specified: no need to process MediaStream_t.");
            retcode = picture_number;
        }
        else
        {
            // Warning: this is not true anymore, must count that manually
            unsigned spspps = map->sample_count - map->frame_count_idr;
            unsigned payload = 0;

            // First cut (remove small frames)
            ////////////////////////////////////////////////////////////////////

            // Compute average samples size
            for (i = spspps; i < map->sample_count; i++)
            {
                payload += map->sample_size[i];
            }

            // Used to filter the frames that are below the threshold (33% of the average frame size)
            unsigned frame_sizethreshold = static_cast<unsigned>(((double)payload / (double)map->frame_count_idr) / 1.66);

            // If we have enough frames (let's say 48), filter the frames from the first and last 3%
            // Note: for a movie, cut the last 33% to avoid spoilers & credits?
            unsigned frame_borders = 0;
            if (map->frame_count_idr > 48)
            {
                frame_borders = std::ceil(map->frame_count_idr * 0.03);
                TRACE_1(FILTR, "frame_borders is %i", frame_borders);
            }

            for (i = frame_borders; i < (map->frame_count_idr - frame_borders); i++)
            {
                if (map->sample_size[i + spspps] > frame_sizethreshold)
                {
                    TRACE_1(FILTR, "IDR %i (size: %i / threshold: %i)", i, map->sample_size[i + spspps], frame_sizethreshold);

                    temporary_sample_id[temporary_totalsamples_idr] = i + spspps;
                    temporary_totalsamples_idr++;
                }
                else
                {
                    TRACE_1(FILTR, "IDR %i (size: %i / threshold: %i) > REMOVED", i, map->sample_size[i + spspps], frame_sizethreshold);
                }
            }

            TRACE_1(FILTR, "We have a total of %i IDR after the first cut", temporary_totalsamples_idr);
            if (picture_number > temporary_totalsamples_idr)
                picture_number = temporary_totalsamples_idr;

            // Jump between two frames in PICTURE_DISTRIBUTED mode
            int frame_jump = std::ceil(temporary_totalsamples_idr / (picture_number-1));
            TRACE_1(FILTR, "frame_jump is %i", frame_jump);

            // Init bitstream_map_filtered
            MediaStream_t *map_filtered = NULL;
            retcode = init_bitstream_map(&map_filtered, spspps, temporary_totalsamples_idr);

            // Write bitstream_map_filtered
            if (retcode)
            {
                map_filtered->stream_type = map->stream_type;
                map_filtered->stream_codec = map->stream_codec;

                map_filtered->sample_count = spspps + temporary_totalsamples_idr;
                map_filtered->frame_count_idr = temporary_totalsamples_idr;

                // Copy SPS and PPS
                for (i = 0; i < spspps; i++)
                {
                    map_filtered->sample_type[i] = map->sample_type[i];
                    map_filtered->sample_pts[i] = map->sample_pts[i];

                    map_filtered->sample_offset[i] = map->sample_offset[i];
                    map_filtered->sample_size[i] = map->sample_size[i];
                }

                // Second cut (frame distribution)
                ////////////////////////////////////////////////////////////////

                for (i = 0; i < picture_number; i++)
                {
                    if (picture_extraction_mode == PICTURE_ORDERED)
                    {
                        map_filtered->sample_type[spspps + i] = map->sample_type[temporary_sample_id[i]];
                        map_filtered->sample_pts[spspps + i] = map->sample_pts[temporary_sample_id[i]];

                        map_filtered->sample_offset[spspps + i] = map->sample_offset[temporary_sample_id[i]];
                        map_filtered->sample_size[spspps + i] = map->sample_size[temporary_sample_id[i]];
                    }
                    else if (picture_extraction_mode == PICTURE_DISTRIBUTED)
                    {
                        map_filtered->sample_type[spspps + i] = map->sample_type[temporary_sample_id[i*frame_jump]];
                        map_filtered->sample_pts[spspps + i] = map->sample_pts[temporary_sample_id[i*frame_jump]];

                        map_filtered->sample_offset[spspps + i] = map->sample_offset[temporary_sample_id[i*frame_jump]];
                        map_filtered->sample_size[spspps + i] = map->sample_size[temporary_sample_id[i*frame_jump]];
                    }
                }
/*
                // Recap
                print_bitstream_map(map);
                print_bitstream_map(map_filtered);
*/

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
                retcode = picture_number;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
