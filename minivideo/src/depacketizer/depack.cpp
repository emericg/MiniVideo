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
#include "depack.h"
#include "depack_struct.h"
#include "h264/depack_h264.h"
#include "h265/depack_h265.h"
#include "h266/depack_h266.h"

#include "../bitstream.h"
#include "../minitraces.h"
#include "../demuxer/xml_mapper.h"

/* ************************************************************************** */

unsigned depack_sample(MediaFile_t *media,
                       MediaStream_t *track, unsigned sample_index,
                       std::vector <es_sample_t> &essample_list, FILE *xml)
{
    TRACE_1(DEPAK, "depack_sample(dispatcher)");
    Bitstream_t *bitstr = nullptr;

    // Check if the stream is indeed packetized
    if (track->stream_packetized == true)
    {
        // Start a reader
        bitstr = init_bitstream0(media,
                                 track->sample_offset[sample_index],
                                 track->sample_size[sample_index]);
    }

    unsigned samplefound = 0;

    if (bitstr)
    {
        samplefound = depack_loaded_sample(bitstr, media, track, sample_index,
                                           essample_list, xml);
        free_bitstream(&bitstr);
    }

    return samplefound;
}

unsigned depack_loaded_sample(Bitstream_t *bitstr, MediaFile_t *media,
                              MediaStream_t *track, unsigned sample_index,
                              std::vector <es_sample_t> &essample_list, FILE *xml)
{
    TRACE_1(DEPAK, "depack_loaded_sample(dispatcher)");
    unsigned samplefound = 0;

    // Check if the stream is indeed packetized
    if (track->stream_packetized == false)
    {
        es_sample_t sample;
        sample.offset = track->sample_offset[sample_index];
        sample.size = track->sample_size[sample_index];
        sample.type_cstr = nullptr;
        essample_list.push_back(sample);
    }
    else
    {
        if (xml)
        {
            fprintf(xml, "<?xml version=\"1.0\" encoding=\"ASCII\" standalone=\"yes\"?>\n");
            fprintf(xml, "<file xmlMapper=\"%d.%d\" minivideo=\"%d.%d-%d\">\n",
                    xmlMapper_VERSION_MAJOR, xmlMapper_VERSION_MINOR,
                    minivideo_VERSION_MAJOR, minivideo_VERSION_MINOR, minivideo_VERSION_PATCH);
            fprintf(xml, "<samples>\n");
        }

        // Select a depacketizer and depack sample
        if (track->stream_codec == CODEC_H264)
        {
            samplefound = depack_h264_sample(bitstr, track, sample_index, essample_list, xml);
        }
        else if (track->stream_codec == CODEC_H265)
        {
            samplefound = depack_h265_sample(bitstr, track, sample_index, essample_list, xml);
        }
        else if (track->stream_codec == CODEC_H266)
        {
            samplefound = depack_h266_sample(bitstr, track, sample_index, essample_list, xml);
        }

        if (xml)
        {
            fprintf(xml, "</samples>\n");
            fprintf(xml, "</file>\n");
            rewind(xml);
        }
    }

    return samplefound;
}

/* ************************************************************************** */
