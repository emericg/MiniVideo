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
 * \date      2017
 */

// minivideo headers
#include "depack_h264.h"
#include "../../decoder/h264/h264_nalu.h"
#include "../../decoder/h264/h264_parameterset.h"
#include "../../bitstream.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"

/* ************************************************************************** */

unsigned depack_h264_sample_legacy(Bitstream_t *bitstr, MediaStream_t *track,
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

            h264_nalu_t n;
            h264_nalu_parse_header(bitstr, &n);
            essample_list[samplefound].type_cstr = h264_nalu_get_string_type0(&n);
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
        {
            samplefoundsize += essample_list[i].size;
        }

        if (samplefoundsize != samplesize)
        {
            TRACE_ERROR(DEPAK, "DEPACK > That's weird, size mismatch: %lli vs %lli",
                        samplefoundsize, samplesize);
        }
    }
#endif // ENABLE_DEBUG

    return samplefound;
}

/* ************************************************************************** */

unsigned depack_h264_sample(Bitstream_t *bitstr,
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

                h264_nalu_t n;
                h264_nalu_parse_header(bitstr, &n);

                sample.type = n.nal_unit_type;
                sample.type_cstr = h264_nalu_get_string_type1(n.nal_unit_type);

                if (sample.type == NALU_TYPE_AUD)
                {
                    //int decodeAUD(Bitstream_t *bitstr, h264_aud_t *aud)
                    //void mapAUD(h264_aud_t *aud, std::vector<std::pair<std::string, std::string>> *vector)

                    h264_aud_t *aud = (h264_aud_t*)calloc(1, sizeof(h264_aud_t));
                    decodeAUD(bitstr, aud);
                    mapAUD(aud, sample.offset, sample.size, xml);

                    //sample.content = calloc(1, sizeof(h264_aud_t));
                    //decodeAUD(bitstr, (h264_aud_t *)sample.content);
                    //mapAUD((h264_aud_t *)sample.content, xml);
                }
                if (sample.type == NALU_TYPE_SPS)
                {
                    h264_sps_t *sps = (h264_sps_t*)calloc(1, sizeof(h264_sps_t));
                    decodeSPS(bitstr, sps);
                    mapSPS(sps, sample.offset, sample.size, xml);
                    freeSPS(&sps);
                }
                if (sample.type == NALU_TYPE_PPS)
                {
                    h264_pps_t *pps = (h264_pps_t*)calloc(1, sizeof(h264_pps_t));
                    decodePPS(bitstr, pps, nullptr);
                    mapPPS(pps, nullptr, sample.offset, sample.size, xml);
                    freePPS(&pps);
                }
                if (sample.type == NALU_TYPE_SEI)
                {
                    h264_sei_t *sei = (h264_sei_t*)calloc(1, sizeof(h264_sei_t));
                    decodeSEI(bitstr, sei);
                    mapSEI(sei, sample.offset, sample.size, xml);
                    freeSEI(&sei);
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
