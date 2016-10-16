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
#include "wave_convert.h"
#include "../riff/riff.h"
#include "../riff/riff_struct.h"
#include "../xml_mapper.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../twocc.h"
#include "../../fourcc.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <string.h>

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse fmt chunk.
 */
static int parse_fmt(Bitstream_t *bitstr, RiffChunk_t *fmt_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_fmt()" CLR_RESET);
    int retcode = SUCCESS;

    if (fmt_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid fmt_header structure!");
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
        else
        {
            TRACE_WARNING(WAV, "Invalid fmt chunk size...");
        }

        if (fmt_header->dwSize >= 18)
        {
            wave->fmt.cbSize = endian_flip_16(read_bits(bitstr, 16));

            if (wave->fmt.wFormatTag == WAVE_FORMAT_MS_PCM && fmt_header->dwSize >= 26)
            {
                wave->fmt.cbSize = endian_flip_16(read_bits(bitstr, 16));

                wave->fmt.wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                for (int i = 0; i < 16; i++)
                    wave->fmt.SubFormat[i] = read_bits(bitstr, 8);
            }
            else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP1 && fmt_header->dwSize >= 40)
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
            else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP3 && fmt_header->dwSize >= 30)
            {
                wave->fmt.wID = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.fdwFlags = endian_flip_32(read_bits(bitstr, 32));
                wave->fmt.nBlockSize = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.nFramesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.nCodecDelay = endian_flip_16(read_bits(bitstr, 16));
            }
            else if (wave->fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt_header->dwSize >= 44)
            {
                wave->fmt.wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.wSamplesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.wReserved = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                for (int i = 0; i < 16; i++)
                    wave->fmt.SubFormat[i] = read_bits(bitstr, 8);
            }
            else
            {
                TRACE_WARNING(WAV, "Invalid fmt chunk extension size...");
            }
        }

#if ENABLE_DEBUG
        print_chunk_header(fmt_header);
        if (fmt_header->dwSize >= 16)
        {
            TRACE_1(WAV, "> wFormatTag      : %u", wave->fmt.wFormatTag);
            TRACE_1(WAV, "> nChannels       : %u", wave->fmt.nChannels);
            TRACE_1(WAV, "> nSamplesPerSec  : %u", wave->fmt.nSamplesPerSec);
            TRACE_1(WAV, "> nAvgBytesPerSec : %u", wave->fmt.nAvgBytesPerSec);
            TRACE_1(WAV, "> nBlockAlign     : %u", wave->fmt.nBlockAlign);
            TRACE_1(WAV, "> wBitsPerSample  : %u", wave->fmt.wBitsPerSample);
        }
        if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 18) // extension
        {
            TRACE_1(WAV, "> cbSize             : %u", wave->fmt.cbSize);

            TRACE_1(WAV, "> wValidBitsPerSample: %u", wave->fmt.wValidBitsPerSample);
            TRACE_1(WAV, "> dwChannelMask      : %u", wave->fmt.dwChannelMask);

            TRACE_1(WAV, "> fwHeadLayer  : %u", wave->fmt.fwHeadLayer);
            TRACE_1(WAV, "> dwHeadBitrate: %u", wave->fmt.dwHeadBitrate);
            TRACE_1(WAV, "> fwHeadMode   : %u", wave->fmt.fwHeadMode);
            TRACE_1(WAV, "> fwHeadModeExt: %u", wave->fmt.fwHeadModeExt);
            TRACE_1(WAV, "> wHeadEmphasis: %u", wave->fmt.wHeadEmphasis);
            TRACE_1(WAV, "> fwHeadFlag   : %u", wave->fmt.fwHeadFlag);
            TRACE_1(WAV, "> dwPTSLow     : %u", wave->fmt.dwPTSLow);
            TRACE_1(WAV, "> dwPTSHigh    : %u", wave->fmt.dwPTSHigh);

            TRACE_1(WAV, "> wID            : %u", wave->fmt.wID);
            TRACE_1(WAV, "> fdwFlags       : %u", wave->fmt.fdwFlags);
            TRACE_1(WAV, "> nBlockSize     : %u", wave->fmt.nBlockSize);
            TRACE_1(WAV, "> nFramesPerBlock: %u", wave->fmt.nFramesPerBlock);
            TRACE_1(WAV, "> nCodecDelay    : %u", wave->fmt.nCodecDelay);

            TRACE_1(WAV, "> wValidBitsPerSample: %u", wave->fmt.wValidBitsPerSample);
            TRACE_1(WAV, "> wSamplesPerBlock   : %u", wave->fmt.wSamplesPerBlock);
            TRACE_1(WAV, "> wReserved          : %u", wave->fmt.wReserved);
            TRACE_1(WAV, "> dwChannelMask      : %u", wave->fmt.dwChannelMask);

            TRACE_1(WAV, "> SubFormat: {%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                    wave->fmt.SubFormat[0], wave->fmt.SubFormat[1], wave->fmt.SubFormat[2], wave->fmt.SubFormat[3],
                    wave->fmt.SubFormat[4], wave->fmt.SubFormat[5],
                    wave->fmt.SubFormat[6], wave->fmt.SubFormat[7],
                    wave->fmt.SubFormat[8], wave->fmt.SubFormat[9],
                    wave->fmt.SubFormat[10], wave->fmt.SubFormat[11], wave->fmt.SubFormat[12], wave->fmt.SubFormat[13], wave->fmt.SubFormat[14], wave->fmt.SubFormat[15]);
        }
#endif // ENABLE_DEBUG

        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(fmt_header, wave->xml);
            if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 16)
            {
                fprintf(wave->xml, "  <title>Format chunk</title>\n");
                fprintf(wave->xml, "  <wFormatTag>%u</wFormatTag>\n", wave->fmt.wFormatTag);
                fprintf(wave->xml, "  <nChannels>%u</nChannels>\n", wave->fmt.nChannels);
                fprintf(wave->xml, "  <nSamplesPerSec>%u</nSamplesPerSec>\n", wave->fmt.nSamplesPerSec);
                fprintf(wave->xml, "  <nAvgBytesPerSec>%u</nAvgBytesPerSec>\n", wave->fmt.nAvgBytesPerSec);
                fprintf(wave->xml, "  <nBlockAlign>%u</nBlockAlign>\n", wave->fmt.nBlockAlign);
                fprintf(wave->xml, "  <wBitsPerSample>%u</wBitsPerSample>\n", wave->fmt.wBitsPerSample);
            }
            if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 18) // extension
            {
                fprintf(wave->xml, "  <cbSize>%u</cbSize>\n", wave->fmt.cbSize);
                if (wave->fmt.wFormatTag == WAVE_FORMAT_MS_PCM && fmt_header->dwSize >= 26)
                {
                    fprintf(wave->xml, "  <wValidBitsPerSample>%u</wValidBitsPerSample>\n", wave->fmt.wValidBitsPerSample);
                    fprintf(wave->xml, "  <dwChannelMask>%u</dwChannelMask>\n", wave->fmt.dwChannelMask);
                    fprintf(wave->xml, "  <SubFormat>%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X</SubFormat>\n",
                            wave->fmt.SubFormat[0], wave->fmt.SubFormat[1], wave->fmt.SubFormat[2], wave->fmt.SubFormat[3],
                            wave->fmt.SubFormat[4], wave->fmt.SubFormat[5],
                            wave->fmt.SubFormat[6], wave->fmt.SubFormat[7],
                            wave->fmt.SubFormat[8], wave->fmt.SubFormat[9],
                            wave->fmt.SubFormat[10], wave->fmt.SubFormat[11], wave->fmt.SubFormat[12], wave->fmt.SubFormat[13], wave->fmt.SubFormat[14], wave->fmt.SubFormat[15]);
                }
                else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP1 && fmt_header->dwSize >= 40)
                {
                    fprintf(wave->xml, "  <fwHeadLayer>%u</fwHeadLayer>\n", wave->fmt.fwHeadLayer);
                    fprintf(wave->xml, "  <dwHeadBitrate>%u</dwHeadBitrate>\n", wave->fmt.dwHeadBitrate);
                    fprintf(wave->xml, "  <fwHeadMode>%u</fwHeadMode>\n", wave->fmt.fwHeadMode);
                    fprintf(wave->xml, "  <fwHeadModeExt>%u</fwHeadModeExt>\n", wave->fmt.fwHeadModeExt);
                    fprintf(wave->xml, "  <wHeadEmphasis>%u</wHeadEmphasis>\n", wave->fmt.wHeadEmphasis);
                    fprintf(wave->xml, "  <fwHeadFlag>%u</fwHeadFlag>\n", wave->fmt.fwHeadFlag);
                    fprintf(wave->xml, "  <dwPTSLow>%u</dwPTSLow>\n", wave->fmt.dwPTSLow);
                    fprintf(wave->xml, "  <dwPTSHigh>%u</dwPTSHigh>\n", wave->fmt.dwPTSHigh);
                }
                else if (wave->fmt.wFormatTag == WAVE_FORMAT_MP3 && fmt_header->dwSize >= 30)
                {
                    fprintf(wave->xml, "  <wID>%u</wID>\n", wave->fmt.wID);
                    fprintf(wave->xml, "  <fdwFlags>%u</fdwFlags>\n", wave->fmt.fdwFlags);
                    fprintf(wave->xml, "  <nBlockSize>%u</nBlockSize>\n", wave->fmt.nBlockSize);
                    fprintf(wave->xml, "  <nFramesPerBlock>%u</nFramesPerBlock>\n", wave->fmt.nFramesPerBlock);
                    fprintf(wave->xml, "  <nCodecDelay>%u</nCodecDelay>\n", wave->fmt.nCodecDelay);
                }
                else if (wave->fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt_header->dwSize >= 44)
                {
                    fprintf(wave->xml, "  <wValidBitsPerSample>%u</wValidBitsPerSample>\n", wave->fmt.wValidBitsPerSample);
                    fprintf(wave->xml, "  <wSamplesPerBlock>%u</wSamplesPerBlock>\n", wave->fmt.wSamplesPerBlock);
                    fprintf(wave->xml, "  <wReserved>%u</wReserved>\n", wave->fmt.wReserved);
                    fprintf(wave->xml, "  <dwChannelMask>%u</dwChannelMask>\n", wave->fmt.dwChannelMask);
                    fprintf(wave->xml, "  <SubFormat>%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X</SubFormat>\n",
                            wave->fmt.SubFormat[0], wave->fmt.SubFormat[1], wave->fmt.SubFormat[2], wave->fmt.SubFormat[3],
                            wave->fmt.SubFormat[4], wave->fmt.SubFormat[5],
                            wave->fmt.SubFormat[6], wave->fmt.SubFormat[7],
                            wave->fmt.SubFormat[8], wave->fmt.SubFormat[9],
                            wave->fmt.SubFormat[10], wave->fmt.SubFormat[11], wave->fmt.SubFormat[12], wave->fmt.SubFormat[13], wave->fmt.SubFormat[14], wave->fmt.SubFormat[15]);
                }
            }
            fprintf(wave->xml, "  </atom>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse fact chunk.
 */
static int parse_fact(Bitstream_t *bitstr, RiffChunk_t *fact_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_fact()" CLR_RESET);
    int retcode = SUCCESS;

    if (fact_header == NULL || fact_header->dwSize < 4)
    {
        TRACE_ERROR(WAV, "Invalid fact_header structure!");
        retcode = FAILURE;
    }
    else
    {
        wave->fact.dwSampleLength = endian_flip_32(read_bits(bitstr, 32));

#if ENABLE_DEBUG
        print_chunk_header(fact_header);
        TRACE_1(WAV, "> dwSampleLength     : %u", wave->fact.dwSampleLength);
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(fact_header, wave->xml);
            fprintf(wave->xml, "  <dwSampleLength>%li</dwSampleLength>\n", wave->fact.dwSampleLength);
            fprintf(wave->xml, "  </atom>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse cue chunk.
 */
static int parse_cue(Bitstream_t *bitstr, RiffChunk_t *data_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_cue()" CLR_RESET);
    int retcode = SUCCESS;

    if (data_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid data_header structure!");
        retcode = FAILURE;
    }
    else
    {
        // TODO

#if ENABLE_DEBUG
        print_chunk_header(data_header);
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(data_header, wave->xml);
            fprintf(wave->xml, "  <title>Cue Points</title>\n");
            fprintf(wave->xml, "  </atom>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse bext chunk.
 */
static int parse_bext(Bitstream_t *bitstr, RiffChunk_t *data_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_bext()" CLR_RESET);
    int retcode = SUCCESS;

    if (data_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid data_header structure!");
        retcode = FAILURE;
    }
    else
    {
        // TODO

#if ENABLE_DEBUG
        print_chunk_header(data_header);
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(data_header, wave->xml);
            fprintf(wave->xml, "  </atom>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse data chunk.
 */
static int parse_data(Bitstream_t *bitstr, RiffChunk_t *data_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_data()" CLR_RESET);
    int retcode = SUCCESS;

    if (data_header == NULL)
    {
        TRACE_ERROR(WAV, "Invalid data_header structure!");
        retcode = FAILURE;
    }
    else
    {
        wave->data.datasOffset = bitstream_get_absolute_byte_offset(bitstr);
        wave->data.datasSize = data_header->dwSize;

#if ENABLE_DEBUG
        print_chunk_header(data_header);
        TRACE_1(WAV, "> datasOffset     : %u", wave->data.datasOffset);
        TRACE_1(WAV, "> datasSize       : %u", wave->data.datasSize);
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(data_header, wave->xml);
            fprintf(wave->xml, "  <datasOffset>%li</datasOffset>\n", wave->data.datasOffset);
            fprintf(wave->xml, "  <datasSize>%li</datasSize>\n", wave->data.datasSize);
            fprintf(wave->xml, "  </atom>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int wave_fileParse(MediaFile_t *media)
{
    TRACE_INFO(WAV, BLD_GREEN "wave_fileParse()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init a wave structure
        wave_t wave;
        memset(&wave, 0, sizeof(wave));

        // A convenient way to stop the parser
        wave.run = true;

        // xmlMapper
        xmlMapperOpen(media, &wave.xml);

        // Loop on 1st level list
        while (wave.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - 8))
        {
            // Read RIFF header
            RiffList_t RIFF_header;
            retcode = parse_list_header(bitstr, &RIFF_header);
            print_list_header(&RIFF_header);
            write_list_header(&RIFF_header, wave.xml);

            // First level chunk
            if (RIFF_header.dwList == fcc_RIFF &&
                RIFF_header.dwFourCC == fcc_WAVE)
            {
                // Loop on 2st level chunks
                while (wave.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
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
                        break;
                    case fcc_bext:
                        retcode = parse_bext(bitstr, &chunk_header, &wave);
                        break;
                    case fcc_JUNK:
                        retcode = parse_JUNK(bitstr, &chunk_header, wave.xml);
                        break;
                    default:
                        retcode = parse_unkn_chunk(bitstr, &chunk_header, wave.xml);
                        break;
                    }

                    jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                }
            }
            else
            {
                // Loop on 2st level chunks
                while (wave.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
                {
                    RiffChunk_t chunk_header;
                    retcode = parse_chunk_header(bitstr, &chunk_header);

                    switch (chunk_header.dwFourCC)
                    {
                    case fcc_JUNK:
                        retcode = parse_JUNK(bitstr, &chunk_header, wave.xml);
                        break;
                    default:
                        retcode = parse_unkn_chunk(bitstr, &chunk_header, wave.xml);
                        break;
                    }

                    jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                }
            }

            if (wave.xml) fprintf(wave.xml, "  </atom>\n");
        }

        // xmlMapper
        xmlMapperClose(&wave.xml);

        // Go for the indexation
        retcode = wave_indexer(bitstr, media, &wave),

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
