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
 * \file      pes_packetizer.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// minivideo headers
#include "../minitraces.h"
#include "../typedef.h"

#include "pes_packetizer.h"

/* ************************************************************************** */

/*!
 * \brief Form a PES packet header for each sample from a track.
 * \param f_src The input file, where to read samples.
 * \param f_dst The output file, where we write PES header + data.
 * \param bitstream_map Informations about the track.
 * \return SUCCESS if so.
 */
int pes_packetizer(FILE *f_src, FILE *f_dst, BitstreamMap_t *bitstream_map)
{
    TRACE_INFO(MUXER, GREEN "> pes_packetizer()\n" RESET);
    int retcode = SUCCESS;
    int i = 0;

    // 3000 = ~ 33,333 * 90 (30 fps)
    // 3600 = ~ 40,000 * 90 (25 fps)
    // 3750 = ~ 41,666 * 90 (24 fps)
    uint64_t pts_tick = 3750;
    uint64_t pts = 1;

    if (bitstream_map->frame_rate > 0)
    {
        pts_tick = (uint64_t)((1000.0 / bitstream_map->frame_rate) * 90);
        TRACE_1(MUXER, "frame rate : %f\n", bitstream_map->frame_rate);
        TRACE_1(MUXER, "pts_tick   : %llu\n", pts_tick);
    }
    else
    {
        TRACE_WARNING(MUXER, "Unknown frame rate (%f). Forcing 24 fps.\n", bitstream_map->frame_rate);
    }

    for (i = 0; i < bitstream_map->sample_count; i++)
    {
        size_t size   = (size_t)bitstream_map->sample_size[i];
        size_t offset = (size_t)bitstream_map->sample_offset[i];

        TRACE_2(MUXER, " > Sample id\t: %i\n", i);
        TRACE_2(MUXER, " | sample type\t: %i\n", bitstream_map->sample_type[i]);
        TRACE_2(MUXER, " | sample size\t: %i\n", size);
        TRACE_2(MUXER, " | sample offset\t: %i\n", offset);

        // Generate a fixed length PES header
        ////////////////////////////////////////////////////////////////////////

        uint8_t pes_header[19] = { 0x00, 0x00, 0x01, 0xE0, 0, 0, 0x80, 0x80, 0, 0x21, 0x00, 0x01, 0x00, 0x01, 0x21, 0x00, 0x01, 0x00, 0x01 };
        int pes_header_size = 9;

        bool pts_enable = true;
        bool dts_enable = true;

        {
            if (bitstream_map->sample_type[i] == sample_VIDEO_PARAM)
            {
                pts_enable = false;
                dts_enable = false;
            }

            // [9-13] PTS
            if (pts_enable == true)
            {
                pes_header_size += 5;

                if (bitstream_map->sample_pts[i] >= 0)
                {
                    pes_header[9] += (uint8_t)(bitstream_map->sample_pts[i] >> 30) & 0x0E;
                    pes_header[10] = (uint8_t)(bitstream_map->sample_pts[i] >> 22);
                    pes_header[11] += (uint8_t)(bitstream_map->sample_pts[i] >> 14) & 0xFE;
                    pes_header[12] = (uint8_t)(bitstream_map->sample_pts[i] >> 7);
                    pes_header[13] += (uint8_t)(bitstream_map->sample_pts[i] << 1) & 0xFE;
                }
                else
                {
                    pts += pts_tick;

                    pes_header[9] += (uint8_t)(pts >> 30) & 0x0E;
                    pes_header[10] = (uint8_t)(pts >> 22);
                    pes_header[11] += (uint8_t)(pts >> 14) & 0xFE;
                    pes_header[12] = (uint8_t)(pts >> 7);
                    pes_header[13] += (uint8_t)(pts << 1) & 0xFE;
                }
            }

            // [14-18] DTS
            if (dts_enable == true)
            {
                if (bitstream_map->sample_dts[i] >= 0)
                {
                    pes_header_size += 5;

                    pes_header[14] += (uint8_t)(bitstream_map->sample_dts[i] >> 30) & 0x0E;
                    pes_header[15] = (uint8_t)(bitstream_map->sample_dts[i] >> 22);
                    pes_header[16] += (uint8_t)(bitstream_map->sample_dts[i] >> 14) & 0xFE;
                    pes_header[17] = (uint8_t)(bitstream_map->sample_dts[i] >> 7);
                    pes_header[18] += (uint8_t)(bitstream_map->sample_dts[i] << 1) & 0xFE;
                }
                else
                {
                    dts_enable = false;
                }
            }

            // [4-5] pes packet length
            uint16_t packetlength = (uint16_t)size + 3 + (pes_header_size - 9);
            if (bitstream_map->stream_codec == CODEC_H264)
                packetlength += 4; // because of the additionnal 4 bytes start code

            pes_header[4] = (uint8_t)(packetlength >> 8);
            pes_header[5] = (uint8_t)(packetlength & 0x00FF);

            // [6] fixed header flags
            pes_header[6] = 0x80;

            // [7] header flags
            if (pts_enable)
            {
                if (dts_enable)
                    pes_header[7] = 0xC0;
                else
                    pes_header[7] = 0x80;
            }
            else
                pes_header[7] = 0x00;

            // header extension data length (+5 if PTS, +5 if DTS)
            pes_header[8] = (uint8_t)(pes_header_size - 9);
        }
/*
#if ENABLE_DEBUG
        {
            // Print PES header
            TRACE_1(DMUXER, " > header : 0x");
            int j = 0;
            for (j = 0; j < 14; j++)
            {
                printf("%02X ", pes_header[j]);
            }
            printf("\n");
        }
#endif
*/
        // Write packet header + data
        ////////////////////////////////////////////////////////////////////////

        if (fseek(f_src, offset, SEEK_SET) != 0)
        {
            TRACE_ERROR(MUXER, "Unable to seek through the input file!\n");
            retcode = FAILURE;
        }
        else
        {
            uint8_t *pes_data = (uint8_t *)malloc(size);

            if (pes_data == NULL)
            {
                TRACE_ERROR(MUXER, "Unable to allocate pes_buffer!\n");
                retcode = FAILURE;
            }
            else
            {
                size_t read  = fread(pes_data, sizeof(uint8_t), size, f_src);
                size_t write = 0;

                if (read != size)
                {
                    TRACE_ERROR(MUXER, "read != size (%i / %i)\n", read, size);
                    retcode = FAILURE;
                }
                else
                {
                    // PES header
                    write = fwrite(pes_header, sizeof(uint8_t), pes_header_size, f_dst);

                    // Add 'Annex B' start code
                    if (bitstream_map->stream_codec == CODEC_H264)
                    {
                        uint8_t startcode[4] = { 0x00, 0x00, 0x00, 0x01 };
                        write = fwrite(startcode, sizeof(uint8_t), 4, f_dst);
                    }

                    // ES content
                    write = fwrite(pes_data, sizeof(uint8_t), size, f_dst);

                    if (write != size)
                    {
                        TRACE_ERROR(MUXER, "write != size (%i / %i)\n", write, size);
                        retcode = FAILURE;
                    }
                }

                free(pes_data);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
