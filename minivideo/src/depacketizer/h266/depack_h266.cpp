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
 * \date      2024
 */

// minivideo headers
#include "depack_h266.h"
#include "../../decoder/h266/h266_nalu.h"
#include "../../decoder/h266/h266_parameterset.h"
#include "../../bitstream.h"
#include "../../minitraces.h"

/* ************************************************************************** */

unsigned depack_h266_sample(Bitstream_t *bitstr,
                            MediaStream_t *track, unsigned sample_index,
                            std::vector <es_sample_t> &samples, FILE *xml)
{
    unsigned samplefound = 0;

    if (bitstr && track)
    {
        // Load sample from the parser
        int64_t samplesoffset = track->sample_offset[sample_index];
        uint32_t samplesize = track->sample_size[sample_index];

        //TRACE_1(DEPAK, "> " BLD_BLUE "READING CONTAINER SAMPLE %u (offset: %lli / size: %lli)",
        //        sampleindex, samplesoffset, samplesize);

        while (true)
        {
            //
            uint32_t current_nalu_size = read_bits(bitstr, 32);
            int64_t current_nalu_offset = bitstream_get_absolute_byte_offset(bitstr);

            //
            if (current_nalu_size <= samplesize)
            {
                es_sample_t sample;
                sample.offset = current_nalu_offset;
                sample.size = current_nalu_size;

                h266_nalu_t n;
                h266_nalu_parse_header(bitstr, &n);

                sample.type = n.nal_unit_type;
                sample.type_cstr = h266_nalu_get_string_type1(n.nal_unit_type);

                if (sample.type == NALU_TYPE_VPS_NUT)
                {
                    //h266_vps_t *vps = (h266_vps_t*)calloc(1, sizeof(h266_vps_t));
                    //h266_decodeVPS(bitstr, vps);
                    //h266_mapVPS(vps, sample.offset, sample.size, xml);
                    //h266_freeVPS(&vps);
                }
                if (sample.type == NALU_TYPE_SPS_NUT)
                {
                    //h266_sps_t *sps = (h266_sps_t*)calloc(1, sizeof(h266_sps_t));
                    //h266_decodeSPS(bitstr, sps);
                    //h266_mapSPS(sps, sample.offset, sample.size, xml);
                    //h266_freeSPS(&sps);
                }
                if (sample.type == NALU_TYPE_PPS_NUT)
                {
                    //h266_pps_t *pps = (h266_pps_t*)calloc(1, sizeof(h266_pps_t));
                    //h266_decodePPS(bitstr, pps, nullptr);
                    //h266_mapPPS(pps, nullptr, sample.offset, sample.size, xml);
                    //h266_freePPS(&pps);
                }
                if (sample.type == NALU_TYPE_PREFIX_SEI_NUT ||
                    sample.type == NALU_TYPE_SUFFIX_SEI_NUT)
                {
                    //h266_sei_t *sei = (h266_sei_t*)calloc(1, sizeof(h266_sei_t));
                    //h266_decodeSEI(bitstr, sei);
                    //h266_mapSEI(sei, sample.offset, sample.size, xml);
                    //h266_freeSEI(&sei);
                }

                samplefound++;
                samples.push_back(sample);

                TRACE_INFO(DEPAK, "> SAMPLE %i (offset: %lli / size: %u)",
                           samplefound, current_nalu_offset, current_nalu_size);

                // Next jump should not be to the last byte of our buffer
                if (current_nalu_offset + current_nalu_size < samplesoffset + samplesize)
                {
                    //skip_bits(bitstr, current_nalu_size*8); // FIXME
                    bitstream_goto_offset(bitstr, current_nalu_offset + current_nalu_size); // FIXME
                }
                else
                {
                    break;
                }
            }
            else
            {
                TRACE_WARNING(DEPAK, "DEPACK > SAMPLE %u (offset: %lli / size: %u) doesn't fit in its buffer (%u)",
                              samplefound + 1, current_nalu_offset, current_nalu_size, samplesize);
                break;
            }
        }
    }

    return samplefound;
}

/* ************************************************************************** */
