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
 * \file      depack_h264.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "depack_h264.h"
#include "../../decoder/h264/h264_nalu.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minivideo_fourcc.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstring>

/* ************************************************************************** */

unsigned depack_h264_sample(Bitstream_t *bitstr, MediaStream_t *track,
                            unsigned sample_index, es_sample_t *essample_list)
{
    int status = SUCCESS;
    unsigned samplefound = 0;

    // Load sample from the parser
    uint32_t sampleindex = sample_index;
    int64_t samplesoffset = track->sample_offset[sampleindex];
    uint32_t samplesize = track->sample_size[sampleindex];

    TRACE_1(DEPAK, "> " BLD_BLUE "READING CONTAINER SAMPLE %u (offset: %lli / size: %lli)",
            sampleindex, samplesoffset, samplesize);

    while (status == SUCCESS && samplefound < 16)
    {
        //
        uint32_t current_nalu_size = read_bits(bitstr, 32);
        int64_t current_nalu_offset = bitstream_get_absolute_byte_offset(bitstr);

        //
        if (current_nalu_size <= samplesize)
        {
            essample_list[samplefound].offset = current_nalu_offset;
            essample_list[samplefound].size = current_nalu_size;

            nalu_t n;
            nalu_parse_header(bitstr, &n);
            essample_list[samplefound].type_str = (char *)nalu_get_string_type0(&n);
            samplefound++;

            TRACE_1(DEPAK, "> SAMPLE %i (offset: %lli / size: %u)",
                    samplefound, current_nalu_offset, current_nalu_size);

            // Next jump should not be to the last byte of our buffer
            if (current_nalu_offset + current_nalu_size < samplesoffset + samplesize)
            {
                //skip_bits(bitstr, current_nalu_size*8); // FIXME
                bitstream_goto_offset(bitstr, current_nalu_offset + current_nalu_size); // FIXME
            }
            else
                break;
        }
        else
        {
            TRACE_WARNING(DEPAK, "DEPACK > SAMPLE %u (offset: %lli / size: %u) doesn't fit in its buffer (%u)",
                          samplefound + 1, current_nalu_offset, current_nalu_size, samplesize);
            status = FAILURE;
        }
    }

#if ENABLE_DEBUG
    // Sanity check
    if (samplefound > 0)
    {
        int64_t samplefoundsize = samplefound * 4; // star code size

        for (unsigned i = 0; i < samplefound; i++)
            samplefoundsize += essample_list[i].size;

        if (samplefoundsize != samplesize)
            TRACE_ERROR(DEPAK, "DEPACK > That's weird, size mismatch: %lli vs %lli",
                        samplefoundsize, samplesize);
    }
#endif // ENABLE_DEBUG

    return samplefound;
}

/* ************************************************************************** */
