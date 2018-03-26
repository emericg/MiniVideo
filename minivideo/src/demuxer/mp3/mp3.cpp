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
 * \file      mp3.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

// minivideo headers
#include "mp3.h"
#include "mp3_struct.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

static int mp3_indexer_track(MediaFile_t *media, mp3_t *mp3)
{
    TRACE_1(MP3, BLD_GREEN "mp3_indexer_track()" CLR_RESET);
    int retcode = FAILURE;

    // Write track metadata (samples have been written already)
    if (media && media->tracks_audio[0])
    {
        MediaStream_t *track = media->tracks_audio[0];
        track->stream_type = stream_AUDIO;

        if (mp3->mpeg_layer == 1)
            track->stream_codec = CODEC_MPEG_L1;
        else if (mp3->mpeg_layer == 2)
            track->stream_codec = CODEC_MPEG_L2;
        else
            track->stream_codec = CODEC_MPEG_L3;

        if (mp3->audio_channels == 0)
        {
            track->channel_count = 2;
            //track->channel_mode = CHAN_STEREO;
        }
        else if (mp3->audio_channels == 1)
        {
            track->channel_count = 2;
            //track->channel_mode = CHAN_STEREO_JOINT;
        }
        else if (mp3->audio_channels == 2)
        {
            track->channel_count = 2;
            //track->channel_mode = CHAN_DUAL;
        }
        else if (mp3->audio_channels == 3)
        {
            track->channel_count = 1;
            //track->channel_mode = CHAN_MONO;
        }

        track->sampling_rate = mp3->audio_samplingrate;
        track->bit_per_sample = 16;

        // SAMPLES
        track->stream_packetized = false;
        track->frame_count = mp3->sample_count;
        track->stream_size = mp3->sample_size_total;

        retcode = SUCCESS;
    }

    return retcode;
}

/* ************************************************************************** */

static int mp3_indexer(MediaFile_t *media, mp3_t *mp3)
{
    TRACE_INFO(MP3, BLD_GREEN "mp3_indexer()" CLR_RESET);
    int retcode = SUCCESS;

    if (media && media->tracks_audio[0])
    {
        // Average bitrate
        if (mp3->audio_vbr == true)
        {
            mp3->audio_bitrate_vbr = (uint32_t)((double)mp3->audio_bitrate_vbr / (double)mp3->sample_count);
            media->tracks_audio[0]->bitrate_mode = BITRATE_VBR;
        }
        else
        {
            mp3->audio_bitrate_vbr = mp3->audio_bitrate_cbr;
            media->tracks_audio[0]->bitrate_mode = BITRATE_CBR;
        }

        // Stream duration
        if (mp3->mpeg_layer == 1)
        {
            if (mp3->mpeg_version == 1) // MPEG v1 layer1
            {
                // duration: frame_count * 384 / sampling_rate
                mp3->media_duration_s = ((double)mp3->sample_count * (double)mp3->mpeg_sampleperframe) / (double)mp3->audio_samplingrate;
            }
            else // MPEG v2/2.5 layer 1
            {
                // duration: frame_count * 132 / sampling_rate
                mp3->media_duration_s = ((double)mp3->sample_count * 132.0) / (double)mp3->audio_samplingrate;
            }
        }
        else // MPEG v1/2/2.5 layer 2/3
        {
            // duration: frame_count * 1152 / sampling_rate
            mp3->media_duration_s = ((double)mp3->sample_count * (double)mp3->mpeg_sampleperframe) / (double)mp3->audio_samplingrate;
        }

        media->tracks_audio_count = 1;
        media->duration = mp3->media_duration_s * 1000.0;
        media->tracks_audio[0]->stream_duration_ms = mp3->media_duration_s * 1000.0;

        retcode = SUCCESS;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int64_t parse_frame_full(Bitstream_t *bitstr, uint32_t frame_header, mp3_t *mp3, MediaFile_t *media)
{
    // Frame infos
    int64_t frame_offset = bitstream_get_absolute_byte_offset(bitstr) - 4;
    int64_t next_frame_offset = 0;
    uint32_t frame_size = 0;
    int64_t frame_pts = 0;
    uint32_t frame_bitrate = 0;

    // Read MP3 frame header content
    uint32_t audio_version_id   = (frame_header & 0x00180000) >> 19;
    uint32_t layer_index        = (frame_header & 0x00060000) >> 17;
    uint32_t protection_bit     = (frame_header & 0x00010000) >> 16;
    uint32_t bitrate_index      = (frame_header & 0x0000F000) >> 12;
    uint32_t samplingrate_index = (frame_header & 0x00000C00) >> 10;
    uint32_t padding_bit        = (frame_header & 0x00000200) >> 9;
    uint32_t private_bit        = (frame_header & 0x00000100) >> 8;
    mp3->audio_channels         = (frame_header & 0x000000C0) >> 6;
    uint32_t mode_extension     = (frame_header & 0x00000030) >> 4;
    uint32_t copyright          = (frame_header & 0x00000080) >> 3;
    uint32_t original           = (frame_header & 0x00000040) >> 2;
    uint32_t emphasis           = (frame_header & 0x00000003);

    switch (audio_version_id)
    {
    case 0x00:
        mp3->mpeg_version = 3; // MPEG-2.5
        break;

    case 0x01:
    case 0x02:
        mp3->mpeg_version = 2; // MPEG-2
        break;

    case 0x03:
        mp3->mpeg_version = 1; // MPEG-1
        break;
    }

    switch (layer_index)
    {
    case 0x03:
        mp3->mpeg_layer = 1; // Layer 1
        break;

    case 0x02:
        mp3->mpeg_layer = 2; // Layer 2
        break;

    default:
    case 0x01:
    case 0x00: // reserved
        mp3->mpeg_layer = 3; // Layer 3
        break;
    }

    if (bitrate_index_table[mp3->mpeg_version - 1][mp3->mpeg_layer - 1][bitrate_index])
    {
        frame_bitrate = bitrate_index_table[mp3->mpeg_version - 1][mp3->mpeg_layer - 1][bitrate_index];

        mp3->audio_bitrate_vbr += frame_bitrate;

        if (mp3->sample_count == 0)
            mp3->audio_bitrate_cbr = frame_bitrate;
        else if (mp3->audio_bitrate_cbr != frame_bitrate)
            mp3->audio_vbr = true;
    }

    if (samplingrate_index_table[mp3->mpeg_version - 1][samplingrate_index])
    {
        mp3->audio_samplingrate = samplingrate_index_table[mp3->mpeg_version - 1][samplingrate_index];
    }

    if (sampleperframe_table[mp3->mpeg_version - 1][mp3->mpeg_layer - 1])
    {
        mp3->mpeg_sampleperframe = sampleperframe_table[mp3->mpeg_version - 1][mp3->mpeg_layer - 1];
    }

    if (frame_bitrate && mp3->audio_samplingrate)
    {
        // Audio frame duration in milliseconds
        if (mp3->mpeg_layer == 1)
        {
            if (mp3->mpeg_version == 1) // MPEG-1 layer 1
            {
                // tick: number of frames * 384 / sampling rate
                mp3->audio_pts_tick = ((double)mp3->mpeg_sampleperframe / (double)mp3->audio_samplingrate) * 1000.0;
            }
            else // MPEG-2/2.5 layer 1
            {
                // tick: number of frames * 132 / sampling rate
                mp3->audio_pts_tick = (132.0 / (double)mp3->audio_samplingrate * 1000.0);
            }
        }
        else // MPEG-1/2/2.5 layer 2/3
        {
            // tick: number of frames * 1152 / sampling rate
            mp3->audio_pts_tick = ((double)mp3->mpeg_sampleperframe / (double)mp3->audio_samplingrate) * 1000.0;
        }

        // Frame PTS (method 1)
        //if (mp3->sample_count > 0)
        //    frame_pts = media->tracks_audio[0]->sample_pts[mp3->sample_count - 1];
        //frame_pts += mp3->audio_pts_tick * 1000.0;

        // Frame PTS (method 2)
        frame_pts = (int64_t)(mp3->audio_pts_tick * mp3->sample_count * 1000.0);

        // Frame size
        frame_size = (uint32_t)((((double)(((double)mp3->mpeg_sampleperframe / 8.0) * (double)frame_bitrate) / (double)(mp3->audio_samplingrate)) * 1000.0) + padding_bit);
        next_frame_offset = frame_offset + frame_size;

        if (frame_size != 0 && next_frame_offset < bitstr->bitstream_size)
        {
            TRACE_2(MP3, "> Valid MPEG-%u Layer %u frame @ %lli, size: %u (bitrate: %u, samplingrate: %u)",
                    mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

            // Set MP3 frame into the bitstream_map
            int sid = media->tracks_audio[0]->sample_count;
            if (sid < 999999)
            {
                media->tracks_audio[0]->sample_type[sid] = sample_AUDIO;
                media->tracks_audio[0]->sample_size[sid] = frame_size;
                media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                media->tracks_audio[0]->sample_pts[sid] = frame_pts;
                media->tracks_audio[0]->sample_dts[sid] = 0;
                media->tracks_audio[0]->sample_count++;
            }

            // Update total track size
            mp3->sample_size_total += frame_size;
            mp3->sample_count++;

            // Update max sample size
            if (frame_size > mp3->sample_size_max)
            {
                mp3->sample_size_max = frame_size;
            }
        }
        else
        {
            TRACE_WARNING(MP3, "> Invalid MPEG-%u Layer %u frame: out of boundaries @ %lli + size: %u (bitrate: %u, samplingrate: %u)",
                          mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

            if ((frame_size > 0) && (frame_size <= 1152))
            {
                // If the MP3 frame size is <= 1152 but out of boundaries, there
                // is a chance that we are at the end of the file anyway, so let's
                // not throw everything away and just stop the parsing here.
                next_frame_offset = bitstr->bitstream_size;
                mp3->run = false;
            }
            else
            {
                next_frame_offset = -1;
            }
        }
    }
    else
    {
        TRACE_WARNING(MP3, "> Invalid MPEG-%u L%u frame: bad bitrate or samplingrate @ %lli + size: %u (bitrate: %u, samplingrate: %u)",
                      mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

        // Try "manual parsing", maybe we will find a new startcode closeby...
        next_frame_offset = frame_offset + 1;
    }

    return next_frame_offset;
}

/* ************************************************************************** */

int64_t parse_frame(Bitstream_t *bitstr, uint32_t frame_header, mp3_t *mp3, MediaFile_t *media)
{
    // Frame infos
    int64_t frame_offset = bitstream_get_absolute_byte_offset(bitstr) - 4;
    int64_t next_frame_offset = 0;
    uint32_t frame_size = 0;
    uint32_t frame_pts = (int64_t)(mp3->audio_pts_tick * mp3->sample_count * 1000.0); // (FAST version)

    // Read MP3 frame header content (FAST version)
    uint32_t bitrate_index = (frame_header & 0x0000F000) >> 12;
    uint32_t padding_bit = (frame_header & 0x00000200) >> 9;
    uint32_t frame_bitrate = bitrate_index_table[mp3->mpeg_version - 1][mp3->mpeg_layer - 1][bitrate_index];

    // Bitrate handling
    mp3->audio_bitrate_vbr += frame_bitrate;
    if (mp3->audio_bitrate_cbr != frame_bitrate)
        mp3->audio_vbr = true;

    if (frame_bitrate && mp3->audio_samplingrate)
    {
        frame_size = (uint32_t)((((double)(((double)mp3->mpeg_sampleperframe / 8.0) * (double)frame_bitrate) / (double)(mp3->audio_samplingrate)) * 1000.0) + padding_bit);
        next_frame_offset = frame_offset + frame_size;

        if (frame_size != 0 && next_frame_offset < bitstr->bitstream_size)
        {
            TRACE_2(MP3, "> Valid MPEG-%u Layer %u frame @ %lli, size: %u (bitrate: %u, samplingrate: %u)",
                    mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

            // Set MP3 frame into the bitstream_map
            int sid = media->tracks_audio[0]->sample_count;
            if (sid < 999999)
            {
                media->tracks_audio[0]->sample_type[sid] = sample_AUDIO;
                media->tracks_audio[0]->sample_size[sid] = frame_size;
                media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                media->tracks_audio[0]->sample_pts[sid] = frame_pts;
                media->tracks_audio[0]->sample_dts[sid] = 0;
                media->tracks_audio[0]->sample_count++;
            }

            // Update total track size
            mp3->sample_size_total += frame_size;
            mp3->sample_count++;

            // Update max sample size
            if (frame_size > mp3->sample_size_max)
            {
                mp3->sample_size_max = frame_size;
            }
        }
        else
        {
            TRACE_WARNING(MP3, "> Invalid MPEG-%u Layer %u frame: out of boundaries @ %lli + size: %u (bitrate: %u, samplingrate: %u)",
                          mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

            if ((frame_size > 0) && (frame_size <= 1152))
            {
                // If the MP3 frame size is <= 1152 but out of boundaries, there
                // is a chance that we are at the end of the file anyway, so let's
                // not throw everything away and just stop the parsing here.
                next_frame_offset = bitstr->bitstream_size;
                mp3->run = false;
            }
            else
            {
                next_frame_offset = -1;
            }
        }
    }
    else
    {
        TRACE_WARNING(MP3, "> Invalid MPEG-%u L%u frame: bad bitrate or samplingrate @ %lli + size: %u (bitrate: %u, samplingrate: %u)",
                      mp3->mpeg_version, mp3->mpeg_layer, frame_offset, frame_size, frame_bitrate, mp3->audio_samplingrate);

        // Try "manual parsing", maybe we will find a new startcode closeby...
        next_frame_offset = frame_offset + 1;
    }

    return next_frame_offset;
}

/* ************************************************************************** */

int mp3_fileParse(MediaFile_t *media)
{
    int retcode = SUCCESS;

    TRACE_INFO(MP3, BLD_GREEN "mp3_fileParse()" CLR_RESET);

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init a MediaStream_t to store samples
        retcode = init_bitstream_map(&media->tracks_audio[0], 0, 999999);

        // Init an MP3 structure
        mp3_t mp3;
        memset(&mp3, 0, sizeof(mp3_t));

        // A convenient way to stop the parser
        mp3.run = true;

        // stuff
        int64_t min_frame_size = 128;
        int64_t frame_offset = 0;
        uint32_t frame_header = 0;
        bool first_frame_parsed = false;

        // Loop on 1st level elements
        while (mp3.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - min_frame_size))
        {
            // Seek to the next frame offset // Assume the MP3 frames will not be bigger than 4Gib
            uint32_t jump_bits = (uint32_t)(frame_offset - bitstream_get_absolute_byte_offset(bitstr)) * 8;
            if (jump_bits > 0)
                skip_bits(bitstr, jump_bits);

            // Read the next frame header
            frame_header = read_bits(bitstr, 32);

            // note: check frame header against 11 bits long instead of 12, to be compatible with MPEG-1/2/2.5
            if ((frame_header & 0xFFE00000) == 0xFFE00000)
            {
                TRACE_1(MP3, "> MP3 frame @ %lli", frame_offset);

                if (first_frame_parsed == false)
                {
                    frame_offset = parse_frame_full(bitstr, frame_header, &mp3, media);

                    if (frame_offset > 0)
                        first_frame_parsed = true;
                    else
                        retcode = FAILURE;
                }
                else
                {
                    frame_offset = parse_frame(bitstr, frame_header, &mp3, media);
                }
            }
            else if ((frame_header & 0xFFFFFF00) == 0x54414700)
            {
                TRACE_INFO(MP3, "> ID3v1 tag @ %lli", frame_offset);
                mp3.run = false;

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = media->file_size - frame_offset;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }
            }
            else if ((frame_header & 0xFFFFFF00) == 0x49443300)
            {
                int id3tag_version = ((frame_header & 0x000000FF) << 8) + read_bits(bitstr, 8);
                /*int id3tag_flag =*/ read_bits(bitstr, 8);

                TRACE_INFO(MP3, "> ID3v2.%i @ %lli", id3tag_version, frame_offset);

                uint32_t id3tag_size = read_bits(bitstr, 8) & 0x0000007F;
                id3tag_size <<= 7;
                id3tag_size += read_bits(bitstr, 8) & 0x0000007F;
                id3tag_size <<= 7;
                id3tag_size += read_bits(bitstr, 8) & 0x0000007F;
                id3tag_size <<= 7;
                id3tag_size += read_bits(bitstr, 8) & 0x0000007F;
                id3tag_size += 10; // bytes already read

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = id3tag_size;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }

                // Simulate TAG parsing
                frame_offset += id3tag_size;
            }
            else if (frame_header == 0x4C595249)
            {
                TRACE_INFO(MP3, "> Lyrics3 tag @ %lli", frame_offset);
                frame_offset += 32; // just restart MP3 frame detection 32 bytes later...

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = 0;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }
            }
            else if (frame_header == 0x58696E67)
            {
                TRACE_INFO(MP3, "> XING tag @ %lli", frame_offset);
                frame_offset += 32; // just restart MP3 frame detection 32 bytes later...

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = 0;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }
            }
            else if (frame_header == 0x56425249)
            {
                TRACE_INFO(MP3, "> VBRI tag @ %lli", frame_offset);
                frame_offset += 32; // just restart MP3 frame detection 32 bytes later...

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = 0;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }
            }
            else if (frame_header == 0x41504554)
            {
                TRACE_WARNING(MP3, "> APE tag @ %lli", frame_offset);

                /*uint32_t apetag_header =*/ read_bits(bitstr, 32); // 0x41474558, second part of the tag header
                /*uint32_t apetag_version =*/ read_bits(bitstr, 32);
                uint32_t apetag_size = 8 + read_bits(bitstr, 32); //  APE header size (8 bytes) + tag content size

                // Add the TAG to the track
                int sid = media->tracks_audio[0]->sample_count;
                if (sid < 999999)
                {
                    media->tracks_audio[0]->sample_type[sid] = sample_AUDIO_TAG;
                    media->tracks_audio[0]->sample_size[sid] = apetag_size;
                    media->tracks_audio[0]->sample_offset[sid] = frame_offset;
                    media->tracks_audio[0]->sample_pts[sid] = 0;
                    media->tracks_audio[0]->sample_dts[sid] = 0;
                    media->tracks_audio[0]->sample_count++;
                }

                // Simulate TAG parsing
                frame_offset = frame_offset + apetag_size;
            }
            else
            {
                // Try to find a new startcode closeby...
                frame_offset += 1;
                TRACE_3(MP3, "Unknown frame header @ %lli (startcode: 0x%X)",
                        frame_offset, frame_header);
            }
        }

        if (retcode == SUCCESS)
        {
            retcode = mp3_indexer_track(media, &mp3);
            if (retcode == SUCCESS)
            {
                retcode = mp3_indexer(media, &mp3);
            }
        }
    }
    else
    {
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
