/*!
 * COPYRIGHT (C) 2015 Emeric Grange - All Rights Reserved
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
 * \file      wave.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

// minivideo headers
#include "wave.h"
#include "wave_struct.h"
#include "../riff/riff.h"
#include "../riff/riff_struct.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse fmt chunk.
 */
static int parse_fmt(Bitstream_t *bitstr, RiffChunk_t *fmt_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_fmt()\n" CLR_RESET);
    int retcode = SUCCESS;

    if (fmt_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid fmt_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        if (fmt_header->dwSize >= 16)
        {
            wave->fmt.wFormatTag = endian_flip_16(read_bits(bitstr, 16));
            wave->fmt.nChannels = endian_flip_16(read_bits(bitstr, 16));
            wave->fmt.nSamplesPerSec = endian_flip_32(read_bits(bitstr, 32));
            wave->fmt.nAvgBytesPerSec = endian_flip_32(read_bits(bitstr, 32));
            wave->fmt.nBlockAlign = endian_flip_16(read_bits(bitstr, 16));
            wave->fmt.wBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
        }

        if (fmt_header->dwSize >= 18)
        {
            wave->fmt.cbSize = endian_flip_16(read_bits(bitstr, 16));

            // TODO always check remaining size in chunk

            if (WAVE_FORMAT_PCM)
            {
                wave->fmt.cbSize = endian_flip_16(read_bits(bitstr, 16));

                wave->fmt.wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                int i = 0;
                for (i = 0; i < 16; i++)
                    wave->fmt.SubFormat[i] = read_bits(bitstr, 8);
            }
            else if (WAVE_FORMAT_MP1)
            {
                wave->fmt.fwHeadLayer = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwHeadBitrate = endian_flip_32(read_bits(bitstr, 32));
                wave->fmt.fwHeadMode = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.fwHeadModeExt = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.wHeadEmphasis = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.fwHeadFlag = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwPTSLow = endian_flip_32(read_bits(bitstr, 32));
                wave->fmt.dwPTSHigh = endian_flip_32(read_bits(bitstr, 32));
            }
            else if (WAVE_FORMAT_MP3)
            {
                wave->fmt.wID = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.fdwFlags = endian_flip_32(read_bits(bitstr, 32));
                wave->fmt.nBlockSize = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.nFramesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.nCodecDelay = endian_flip_16(read_bits(bitstr, 16));
            }
            else if (WAVE_FORMAT_EXTENSIBLE)
            {
                wave->fmt.wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.wSamplesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.wReserved = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                int i = 0;
                for (i = 0; i < 16; i++)
                    wave->fmt.SubFormat[i] = read_bits(bitstr, 8);
            }
            else
            {
                TRACE_WARNING(WAV, "Invalid fmt chunk extension size...\n");
            }
        }

#if ENABLE_DEBUG
        // Print header
        print_chunk_header(fmt_header);

        // Print content
        TRACE_1(WAV, "> wFormatTag      : %u\n", wave->fmt.wFormatTag);
        TRACE_1(WAV, "> nChannels       : %u\n", wave->fmt.nChannels);
        TRACE_1(WAV, "> nSamplesPerSec  : %u\n", wave->fmt.nSamplesPerSec);
        TRACE_1(WAV, "> nAvgBytesPerSec : %u\n", wave->fmt.nAvgBytesPerSec);
        TRACE_1(WAV, "> nBlockAlign     : %u\n", wave->fmt.nBlockAlign);
        TRACE_1(WAV, "> wBitsPerSample  : %u\n", wave->fmt.wBitsPerSample);

        // Extension
        if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 18)
        {
            TRACE_1(WAV, "> cbSize             : %u\n", wave->fmt.cbSize);

            TRACE_1(WAV, "> wValidBitsPerSample: %u\n", wave->fmt.wValidBitsPerSample);
            TRACE_1(WAV, "> dwChannelMask      : %u\n", wave->fmt.dwChannelMask);

            TRACE_1(WAV, "> SubFormat : {%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
                    wave->fmt.SubFormat[0], wave->fmt.SubFormat[1], wave->fmt.SubFormat[2], wave->fmt.SubFormat[3],
                    wave->fmt.SubFormat[4], wave->fmt.SubFormat[5],
                    wave->fmt.SubFormat[6], wave->fmt.SubFormat[7],
                    wave->fmt.SubFormat[8], wave->fmt.SubFormat[9],
                    wave->fmt.SubFormat[10], wave->fmt.SubFormat[11], wave->fmt.SubFormat[12], wave->fmt.SubFormat[13], wave->fmt.SubFormat[14], wave->fmt.SubFormat[15]);
        }

#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse fact chunk.
 */
static int parse_fact(Bitstream_t *bitstr, RiffChunk_t *fact_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_fact()\n" CLR_RESET);
    int retcode = SUCCESS;

    if (fact_header == NULL || fact_header->dwSize < 4)
    {
        TRACE_ERROR(WAV, "Invalid fact_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        wave->fact.dwSampleLength = endian_flip_32(read_bits(bitstr, 32));

#if ENABLE_DEBUG
        // Print header
        print_chunk_header(fact_header);

        // Print content
        TRACE_1(WAV, "> dwSampleLength     : %u\n", wave->fact.dwSampleLength);
#endif
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse cue chunk.
 */
static int parse_cue(Bitstream_t *bitstr, RiffChunk_t *data_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_cue()\n" CLR_RESET);
    int retcode = SUCCESS;

    if (data_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid data_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
#if ENABLE_DEBUG
        // Print header
        print_chunk_header(data_header);

        // Print content
#endif
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse data chunk.
 */
static int parse_data(Bitstream_t *bitstr, RiffChunk_t *data_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_data()\n" CLR_RESET);
    int retcode = SUCCESS;

    if (data_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid data_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        wave->data.datasOffset = bitstream_get_absolute_byte_offset(bitstr);
        wave->data.datasSize = data_header->dwSize;

#if ENABLE_DEBUG
        // Print header
        print_chunk_header(data_header);

        // Print content
        TRACE_1(WAV, "> datasOffset     : %u\n", wave->data.datasOffset);
        TRACE_1(WAV, "> datasSize       : %u\n", wave->data.datasSize);
#endif
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int wave_indexer_initmap(MediaFile_t *video, wave_t *wave)
{
    // Init a bitstreamMap_t for each wave track
    int retcode = init_bitstream_map(&video->tracks_audio[0], 1);

    if (retcode == SUCCESS)
    {
        BitstreamMap_t *track = video->tracks_audio[video->tracks_audio_count];
        video->tracks_audio_count++;

        track->stream_type  = stream_AUDIO;
        track->stream_level = stream_level_ES;

        if (wave->fmt.wFormatTag == WAVE_FORMAT_PCM ||
            wave->fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            track->stream_codec = CODEC_LPCM;

            if (wave->fact.dwSampleLength)
            {
                track->stream_size = wave->fact.dwSampleLength * (wave->fmt.wBitsPerSample*8) * wave->fmt.nChannels;

                if (wave->fmt.nSamplesPerSec)
                    track->duration = wave->fact.dwSampleLength * (1000.0 / (double)(wave->fmt.nSamplesPerSec));
            }
            else
            {
                track->stream_size = wave->data.datasSize; // may not be necessary

                if (wave->fmt.wBitsPerSample)
                    track->duration = wave->fmt.nSamplesPerSec * (wave->fmt.wBitsPerSample/8) * wave->fmt.nChannels;
            }

            track->bitrate = wave->fmt.nSamplesPerSec * (wave->fmt.wBitsPerSample*8) * wave->fmt.nChannels;
            track->bitrate_mode = BITRATE_CBR;

            TRACE_1(WAV, "track->bitrate %u \n", track->bitrate);

            // PCM only
            track->pcm_sample_format = 0;
            track->pcm_sample_size = 0;
            track->pcm_sample_endianness = 0;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP1)
        {
            track->stream_codec = CODEC_MPEG_L1;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP3)
        {
            track->stream_codec = CODEC_MPEG_L3;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_AAC)
        {
            track->stream_codec = CODEC_AAC;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_AC3)
        {
            track->stream_codec = CODEC_AC3;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_DTS ||
                 wave->fmt.wFormatTag == WAVE_FORMAT_DTS_MS)
        {
            track->stream_codec = CODEC_DTS;
        }
        else if (wave->fmt.wFormatTag == WAVE_FORMAT_WMA1 ||
                 wave->fmt.wFormatTag == WAVE_FORMAT_WMA2 ||
                 wave->fmt.wFormatTag == WAVE_FORMAT_WMAL ||
                 wave->fmt.wFormatTag == WAVE_FORMAT_WMAP ||
                 wave->fmt.wFormatTag == WAVE_FORMAT_WMAS)
        {
            track->stream_codec = CODEC_WMA;
        }

        track->channel_count = wave->fmt.nChannels;
        track->sampling_rate = wave->fmt.nSamplesPerSec;
        track->bit_per_sample = wave->fmt.wBitsPerSample;

        // SAMPLES
        track->sample_alignment = true;
        track->sample_count = track->sample_count_idr = 1;

        track->sample_type[0] = 1;
        track->sample_size[0] = wave->data.datasSize;
        track->sample_offset[0] = wave->data.datasOffset;
        track->sample_pts[0] = 0;
        track->sample_dts[0] = 0;
    }

    return retcode;
}

/* ************************************************************************** */

static int wave_indexer(Bitstream_t *bitstr, MediaFile_t *video, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "wave_indexer()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Convert index into a bitstream map
    retcode = wave_indexer_initmap(video, wave);

    if (retcode == SUCCESS)
    {
        if (video->tracks_audio[0])
        {
            video->duration = video->tracks_audio[0]->duration;
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int wave_fileParse(MediaFile_t *video)
{
    TRACE_INFO(WAV, BLD_GREEN "wave_fileParse()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(video, NULL);

    if (bitstr != NULL)
    {
        // Init a wave structure
        wave_t wave;
        memset(&wave, 0, sizeof(wave));

        // A convenient way to stop the parser
        bool superrun = true;

        // Read RIFF header
        RiffList_t RIFF_header;
        retcode = parse_list_header(bitstr, &RIFF_header);
        print_list_header(&RIFF_header);

        if (RIFF_header.dwList == fcc_RIFF &&
            RIFF_header.dwFourCC == fcc_WAVE)
        {
            // Loop on 2st level chunks
            while (superrun == true &&
                   retcode == SUCCESS &&
                   bitstream_get_absolute_byte_offset(bitstr) < (video->file_size - 8))
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_fmt_:
                    retcode = parse_fmt(bitstr, &chunk_header, &wave);
                    break;
                case fcc_fact:
                    retcode = parse_fact(bitstr, &chunk_header, &wave);
                    break;
                case fcc_data:
                    retcode = parse_data(bitstr, &chunk_header, &wave);
                    break;
                case fcc_cue_:
                    retcode = parse_cue(bitstr, &chunk_header, &wave);
                default:
                    TRACE_WARNING(WAV, BLD_GREEN "Unknown chunk type (%s)\n" CLR_RESET,
                                  getFccString_le(chunk_header.dwFourCC, fcc));
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, &RIFF_header, &chunk_header);
                    break;
                }

                int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);
                if (current_pos != chunk_header.offset_end)
                {
                    TRACE_WARNING(WAV, "WAVE parser is not where its supposed to be (%lli instead of %lli)\n",
                                  current_pos, chunk_header.offset_end);

                    int64_t jump = 0;

                    // Jump to the next list / chunk
                    if (current_pos < chunk_header.offset_end)
                    {
                        jump = chunk_header.offset_end - current_pos;

                        if (jump < (INT_MAX/8))
                            skip_bits(bitstr, (unsigned int)(jump*8));
                        else
                            bitstream_goto_offset(bitstr, chunk_header.offset_end);
                    }
                    else
                    {
                        jump = current_pos - chunk_header.offset_end;

                        if (jump > (INT_MIN/8))
                            rewind_bits(bitstr, (unsigned int)(jump*8));
                        else
                            bitstream_goto_offset(bitstr, chunk_header.offset_end);
                    }
                }
            }
        }

        // Go for the indexation
        retcode = wave_indexer(bitstr, video, &wave),

        // Free bitstream
        free_bitstream(&bitstr);
    }
    else
    {
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
