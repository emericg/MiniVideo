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
#include "../../minivideo_typedef.h"
#include "../../minivideo_uuid.h"
#include "../../minivideo_twocc.h"
#include "../../minivideo_fourcc.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

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

                read_uuid_le(bitstr, wave->fmt.SubFormat);
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
            else if (wave->fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt_header->dwSize >= 40)
            {
                wave->fmt.wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                //wave->fmt.wSamplesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                //wave->fmt.wReserved = endian_flip_16(read_bits(bitstr, 16));
                wave->fmt.dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                read_uuid_le(bitstr, wave->fmt.SubFormat);

                if (memcmp(wave->fmt.SubFormat, WAVE_SubFormat_GUIDs[WAVE_AMBISONIC_SUBTYPE_PCM], 16) == 0 ||
                    memcmp(wave->fmt.SubFormat, WAVE_SubFormat_GUIDs[WAVE_AMBISONIC_SUBTYPE_IEEE_FLOAT], 16) == 0)
                {
                    wave->profile = PROF_WAVE_AMB;
                }
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
        // extensions
        if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 18)
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
            //TRACE_1(WAV, "> wSamplesPerBlock   : %u", wave->fmt.wSamplesPerBlock);
            //TRACE_1(WAV, "> wReserved          : %u", wave->fmt.wReserved);
            TRACE_1(WAV, "> dwChannelMask      : %u", wave->fmt.dwChannelMask);

            char SubFormat_str[39];
            getGuidString(wave->fmt.SubFormat, SubFormat_str);
            TRACE_1(WAV, "> SubFormat: %s", SubFormat_str);
        }
#endif // ENABLE_DEBUG

        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(fmt_header, wave->xml);
            if (fmt_header->dwSize >= 16)
            {
                fprintf(wave->xml, "  <title>Format chunk</title>\n");
                fprintf(wave->xml, "  <wFormatTag>0x%04X (%s)</wFormatTag>\n", wave->fmt.wFormatTag, getTccString(wave->fmt.wFormatTag));
                fprintf(wave->xml, "  <nChannels>%u</nChannels>\n", wave->fmt.nChannels);
                fprintf(wave->xml, "  <nSamplesPerSec>%u</nSamplesPerSec>\n", wave->fmt.nSamplesPerSec);
                fprintf(wave->xml, "  <nAvgBytesPerSec>%u</nAvgBytesPerSec>\n", wave->fmt.nAvgBytesPerSec);
                fprintf(wave->xml, "  <nBlockAlign>%u</nBlockAlign>\n", wave->fmt.nBlockAlign);
                fprintf(wave->xml, "  <wBitsPerSample>%u</wBitsPerSample>\n", wave->fmt.wBitsPerSample);
            }
            // extensions
            if (wave->fmt.wFormatTag && wave->fmt.cbSize >= 18)
            {
                fprintf(wave->xml, "  <cbSize>%u</cbSize>\n", wave->fmt.cbSize);

                if (wave->fmt.wFormatTag == WAVE_FORMAT_MS_PCM && fmt_header->dwSize >= 26)
                {
                    fprintf(wave->xml, "  <wValidBitsPerSample>%u</wValidBitsPerSample>\n", wave->fmt.wValidBitsPerSample);
                    fprintf(wave->xml, "  <dwChannelMask>%u</dwChannelMask>\n", wave->fmt.dwChannelMask);

                    char SubFormat_str[39];
                    getGuidString(wave->fmt.SubFormat, SubFormat_str);
                    fprintf(wave->xml, "  <SubFormat>%s</SubFormat>\n", SubFormat_str);
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
                else if (wave->fmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt_header->dwSize >= 40)
                {
                    fprintf(wave->xml, "  <wValidBitsPerSample>%u</wValidBitsPerSample>\n", wave->fmt.wValidBitsPerSample);
                    //fprintf(wave->xml, "  <wSamplesPerBlock>%u</wSamplesPerBlock>\n", wave->fmt.wSamplesPerBlock);
                    //fprintf(wave->xml, "  <wReserved>%u</wReserved>\n", wave->fmt.wReserved);

                    char SubFormat_str[39];
                    getGuidString(wave->fmt.SubFormat, SubFormat_str);
                    fprintf(wave->xml, "  <SubFormat>%s</SubFormat>\n", SubFormat_str);
                }
            }
            fprintf(wave->xml, "  </a>\n");
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
            fprintf(wave->xml, "  <dwSampleLength>%u</dwSampleLength>\n", wave->fact.dwSampleLength);
            fprintf(wave->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse cue chunk.
 */
static int parse_cue(Bitstream_t *bitstr, RiffChunk_t *cue_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_cue()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (cue_header == NULL || cue_header->dwSize  < 4)
    {
        TRACE_ERROR(WAV, "Invalid cue_header structure!");
        retcode = FAILURE;
    }
    else
    {
        wave->cue.dwCuePoints = endian_flip_32(read_bits(bitstr, 32));

        if (cue_header->dwSize >= (4 + wave->cue.dwCuePoints * 24))
        {
            wave->cue.dwName = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));
            wave->cue.dwPosition = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));
            wave->cue.fccChunk = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));
            wave->cue.dwChunkStart = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));
            wave->cue.dwBlockStart = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));
            wave->cue.dwSampleOffset = (uint32_t*)calloc(wave->cue.dwCuePoints, sizeof(uint32_t));

            if (wave->cue.dwName && wave->cue.dwPosition && wave->cue.fccChunk &&
                wave->cue.dwChunkStart && wave->cue.dwBlockStart && wave->cue.dwSampleOffset)
            {
                for (uint32_t i = 0; i < wave->cue.dwCuePoints; i++)
                {
                    wave->cue.dwName[i] = read_bits(bitstr, 32);
                    wave->cue.dwPosition[i] = endian_flip_32(read_bits(bitstr, 32));
                    wave->cue.fccChunk[i] = read_bits(bitstr, 32);
                    wave->cue.dwChunkStart[i] = endian_flip_32(read_bits(bitstr, 32));
                    wave->cue.dwBlockStart[i] = endian_flip_32(read_bits(bitstr, 32));
                    wave->cue.dwSampleOffset[i] = endian_flip_32(read_bits(bitstr, 32));
                }
            }
        }
        else
        {
            TRACE_ERROR(WAV, "Cue chunk is too small!");
            retcode = FAILURE;
        }

#if ENABLE_DEBUG
        print_chunk_header(cue_header);
        TRACE_1(WAV, "> dwCuePoints     : %u", wave->cue.dwCuePoints);

        if (wave->cue.dwName && wave->cue.dwPosition && wave->cue.fccChunk &&
            wave->cue.dwChunkStart && wave->cue.dwBlockStart && wave->cue.dwSampleOffset)
        {
            for (uint32_t i = 0; i < wave->cue.dwCuePoints; i++)
            {
                TRACE_1(WAV, "> dwName          : 0x%08X ('%s')",
                        wave->cue.dwName[i], getFccString_le(wave->cue.dwName[i], fcc));
                TRACE_1(WAV, "> dwPosition      : %u", wave->cue.dwPosition[i]);
                TRACE_1(WAV, "> fccChunk        : 0x%08X ('%s')",
                        wave->cue.fccChunk[i], getFccString_le(wave->cue.fccChunk[i], fcc));
                TRACE_1(WAV, "> dwChunkStart    : %u", wave->cue.dwChunkStart[i]);
                TRACE_1(WAV, "> dwBlockStart    : %u", wave->cue.dwBlockStart[i]);
                TRACE_1(WAV, "> dwSampleOffset  : %u", wave->cue.dwSampleOffset[i]);
            }
        }
#endif // ENABLE_DEBUG

        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(cue_header, wave->xml);
            fprintf(wave->xml, "  <title>Cue Points</title>\n");
            fprintf(wave->xml, "  <dwCuePoints>%u</dwCuePoints>\n", wave->cue.dwCuePoints);
            if (wave->cue.dwName && wave->cue.dwPosition && wave->cue.fccChunk &&
                wave->cue.dwChunkStart && wave->cue.dwBlockStart && wave->cue.dwSampleOffset)
            {
                for (uint32_t i = 0; i < wave->cue.dwCuePoints; i++)
                {
                    fprintf(wave->xml, "    <dwName index=\"%u\">%s</dwName>\n", i, getFccString_le(wave->cue.dwName[i], fcc));
                    fprintf(wave->xml, "    <dwPosition index=\"%u\">%u</dwPosition>\n", i, wave->cue.dwPosition[i]);
                    fprintf(wave->xml, "    <fccChunk index=\"%u\">%s</fccChunk>\n", i, getFccString_le(wave->cue.fccChunk[i], fcc));
                    fprintf(wave->xml, "    <dwChunkStart index=\"%u\">%u</dwChunkStart>\n", i, wave->cue.dwChunkStart[i]);
                    fprintf(wave->xml, "    <dwBlockStart index=\"%u\">%u</dwBlockStart>\n", i, wave->cue.dwBlockStart[i]);
                    fprintf(wave->xml, "    <dwSampleOffset index=\"%u\">%u</dwSampleOffset>\n", i, wave->cue.dwSampleOffset[i]);
                }
            }
            fprintf(wave->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse plst chunk.
 */
static int parse_plst(Bitstream_t *bitstr, RiffChunk_t *plst_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_plst()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (plst_header == NULL || plst_header->dwSize  < 4)
    {
        TRACE_ERROR(WAV, "Invalid plst_header structure!");
        retcode = FAILURE;
    }
    else
    {
        wave->plst.dwSegments = endian_flip_32(read_bits(bitstr, 32));

        if (plst_header->dwSize >= (4 + wave->plst.dwSegments * 12))
        {
            wave->plst.dwName = (uint32_t*)calloc(wave->plst.dwSegments, sizeof(uint32_t));
            wave->plst.dwLength = (uint32_t*)calloc(wave->plst.dwSegments, sizeof(uint32_t));
            wave->plst.dwLoops = (uint32_t*)calloc(wave->plst.dwSegments, sizeof(uint32_t));

            if (wave->plst.dwName && wave->plst.dwLength && wave->plst.dwLoops)
            {
                for (uint32_t i = 0; i < wave->plst.dwSegments; i++)
                {
                    wave->plst.dwName[i] = read_bits(bitstr, 32);
                    wave->plst.dwLength[i] = endian_flip_32(read_bits(bitstr, 32));
                    wave->plst.dwLoops[i] = endian_flip_32(read_bits(bitstr, 32));
                }
            }
        }
        else
        {
            TRACE_ERROR(WAV, "Cue chunk is too small!");
            retcode = FAILURE;
        }

#if ENABLE_DEBUG
        print_chunk_header(plst_header);
        TRACE_1(WAV, "> dwSegments          : %u", wave->plst.dwSegments);

        if (wave->plst.dwName && wave->plst.dwLength && wave->plst.dwLoops)
        {
            for (uint32_t i = 0; i < wave->plst.dwSegments; i++)
            {
                TRACE_1(WAV, "> dwName          : 0x%08X ('%s')",
                        wave->plst.dwName[i], getFccString_le(wave->plst.dwName[i], fcc));
                TRACE_1(WAV, "> dwLength        : %u", wave->plst.dwLength[i]);
                TRACE_1(WAV, "> dwLoops         : %u", wave->plst.dwLoops[i]);
            }
        }
#endif // ENABLE_DEBUG

        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(plst_header, wave->xml);
            fprintf(wave->xml, "  <title>Playlist</title>\n");
            fprintf(wave->xml, "  <dwCuePoints>%u</dwCuePoints>\n", wave->cue.dwCuePoints);
            if (wave->plst.dwName && wave->plst.dwLength && wave->plst.dwLoops)
            {
                for (uint32_t i = 0; i < wave->plst.dwSegments; i++)
                {
                    fprintf(wave->xml, "    <dwName index=\"%u\">%s</dwName>\n", i, getFccString_le(wave->plst.dwName[i], fcc));
                    fprintf(wave->xml, "    <dwLength index=\"%u\">%u</dwLength>\n", i, wave->plst.dwLength[i]);
                    fprintf(wave->xml, "    <dwLoops index=\"%u\">%u</dwLoops>\n", i, wave->plst.dwLoops[i]);
                }
            }
            fprintf(wave->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse bext chunk.
 *
 * From 'EBU Tech 3285' specification:
 * 2.3 Broadcast Audio Extension Chunk
 */
static int parse_bext(Bitstream_t *bitstr, RiffChunk_t *bext_header, wave_t *wave)
{
    TRACE_INFO(WAV, BLD_GREEN "parse_bext()" CLR_RESET);
    int retcode = SUCCESS;

    if (bext_header == NULL || bext_header->dwSize < 348)
    {
        TRACE_ERROR(WAV, "Invalid bext_header structure!");
        retcode = FAILURE;
    }
    else
    {
        for (uint32_t i = 0; i < 256; i++)
        {
            wave->bwf.Description[i] = read_bits(bitstr, 8);
        }
        for (uint32_t i = 0; i < 32; i++)
        {
            wave->bwf.Originator[i] = read_bits(bitstr, 8);
        }
        for (uint32_t i = 0; i < 32; i++)
        {
            wave->bwf.OriginatorReference[i] = read_bits(bitstr, 8);
        }
        for (uint32_t i = 0; i < 10; i++)
        {
            wave->bwf.OriginatorDate[i] = read_bits(bitstr, 8);
        }
        for (uint32_t i = 0; i < 8; i++)
        {
            wave->bwf.OriginationTime[i] = read_bits(bitstr, 8);
        }

        wave->bwf.TimeReferenceLow = endian_flip_32(read_bits(bitstr, 32));
        wave->bwf.TimeReferenceHigh = endian_flip_32(read_bits(bitstr, 32));
        wave->bwf.Version = endian_flip_16(read_bits(bitstr, 16));

        if (wave->profile == 0)
        {
            if (wave->bwf.Version == 1)
                wave->profile = PROF_WAVE_BWFv1;
            else
                wave->profile = PROF_WAVE_BWFv2;
        }
        else if (wave->profile == PROF_WAVE_RF64)
        {
            wave->profile = PROF_WAVE_BWF64;
        }

        if (wave->bwf.Version >= 1 && bext_header->dwSize >= 412)
        {
            for (uint32_t i = 0; i < 64; i++)
            {
                wave->bwf.UMID[i] = read_bits(bitstr, 8);
            }
        }
        if (wave->bwf.Version >= 2 && bext_header->dwSize >= 422)
        {
            wave->bwf.LoudnessVal = endian_flip_16(read_bits(bitstr, 16));
            wave->bwf.LoudnessRange = endian_flip_16(read_bits(bitstr, 16));
            wave->bwf.MaxTruePeakLevel = endian_flip_16(read_bits(bitstr, 16));
            wave->bwf.MaxMomentaryLoudnes = endian_flip_16(read_bits(bitstr, 16));
            wave->bwf.MaxShortTermLoudness = endian_flip_16(read_bits(bitstr, 16));
        }

#if ENABLE_DEBUG
        print_chunk_header(bext_header);
        TRACE_1(WAV, "> Description         : '%s'", wave->bwf.Description);
        TRACE_1(WAV, "> Originator          : '%s'", wave->bwf.Originator);
        TRACE_1(WAV, "> OriginatorReference : '%s'", wave->bwf.OriginatorReference);
        TRACE_1(WAV, "> OriginatorDate      : '%s'", wave->bwf.OriginatorDate);
        TRACE_1(WAV, "> OriginationTime     : '%s'", wave->bwf.OriginationTime);
        TRACE_1(WAV, "> TimeReferenceLow    : %u", wave->bwf.TimeReferenceLow);
        TRACE_1(WAV, "> TimeReferenceHigh   : %u", wave->bwf.TimeReferenceHigh);
        TRACE_1(WAV, "> Version             : %u", wave->bwf.Version);

        if (wave->bwf.Version >= 1)
        {
            TRACE_1(WAV, "> UMID                : '%s'", wave->bwf.UMID);
        }
        if (wave->bwf.Version >= 2)
        {
            TRACE_1(WAV, "> LoudnessVal         : %u", wave->bwf.LoudnessVal);
            TRACE_1(WAV, "> LoudnessRange       : %u", wave->bwf.LoudnessRange);
            TRACE_1(WAV, "> MaxTruePeakLevel    : %u", wave->bwf.MaxTruePeakLevel);
            TRACE_1(WAV, "> MaxMomentaryLoudnes : %u", wave->bwf.MaxMomentaryLoudnes);
            TRACE_1(WAV, "> MaxShortTermLoudness: %u", wave->bwf.MaxShortTermLoudness);
        }
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(bext_header, wave->xml);
            fprintf(wave->xml, "  <title>Broadcast Audio Extension</title>\n");
            fprintf(wave->xml, "  <Description>%s</Description>\n", wave->bwf.Description);
            fprintf(wave->xml, "  <Originator>%s</Originator>\n", wave->bwf.Originator);
            fprintf(wave->xml, "  <OriginatorReference>%s</OriginatorReference>\n", wave->bwf.OriginatorReference);
            fprintf(wave->xml, "  <OriginatorDate>%s</OriginatorDate>\n", wave->bwf.OriginatorDate);
            fprintf(wave->xml, "  <OriginationTime>%s</OriginationTime>\n", wave->bwf.OriginationTime);
            fprintf(wave->xml, "  <TimeReferenceLow>%u</TimeReferenceLow>\n", wave->bwf.TimeReferenceLow);
            fprintf(wave->xml, "  <TimeReferenceHigh>%u</TimeReferenceHigh>\n", wave->bwf.TimeReferenceHigh);
            fprintf(wave->xml, "  <Version>%u</Version>\n", wave->bwf.Version);

            if (wave->bwf.Version >= 1)
            {
                fprintf(wave->xml, "  <UMID>%s</UMID>\n", wave->bwf.UMID);
            }
            if (wave->bwf.Version >= 2)
            {
                fprintf(wave->xml, "  <LoudnessVal>%u</LoudnessVal>\n", wave->bwf.LoudnessVal);
                fprintf(wave->xml, "  <LoudnessRange>%u</LoudnessRange>\n", wave->bwf.LoudnessRange);
                fprintf(wave->xml, "  <MaxTruePeakLevel>%u</MaxTruePeakLevel>\n", wave->bwf.MaxTruePeakLevel);
                fprintf(wave->xml, "  <MaxMomentaryLoudnes>%u</MaxMomentaryLoudnes>\n", wave->bwf.MaxMomentaryLoudnes);
                fprintf(wave->xml, "  <MaxShortTermLoudness>%u</MaxShortTermLoudness>\n", wave->bwf.MaxShortTermLoudness);
            }

            fprintf(wave->xml, "  </a>\n");
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
        TRACE_1(WAV, "> datasOffset     : %lli", wave->data.datasOffset);
        TRACE_1(WAV, "> datasSize       : %lli", wave->data.datasSize);
#endif
        // xmlMapper
        if (wave->xml)
        {
            write_chunk_header(data_header, wave->xml);
            fprintf(wave->xml, "  <datasOffset>%" PRId64 "</datasOffset>\n", wave->data.datasOffset);
            fprintf(wave->xml, "  <datasSize>%" PRId64 "</datasSize>\n", wave->data.datasSize);
            fprintf(wave->xml, "  </a>\n");
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

            if (wave.profile == 0)
            {
                if (RIFF_header.dwList == fcc_RF64)
                    wave.profile = PROF_WAVE_RF64;
                if (RIFF_header.dwList == fcc_BW64)
                    wave.profile = PROF_WAVE_BWF64;
            }

            if (RIFF_header.dwList == fcc_RIFF &&
                RIFF_header.dwFourCC == fcc_WAVE)
            {
                // Loop on 2st level chunks
                while (wave.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        RiffList_t list_header;
                        retcode = parse_list_header(bitstr, &list_header);

                        switch (list_header.dwFourCC)
                        {
                        default:
                            retcode = parse_unkn_list(bitstr, &list_header, wave.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, list_header.offset_end);
                    }
                    else
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
                        case fcc_PAD:
                            retcode = parse_PAD(bitstr, &chunk_header, wave.xml);
                            break;
                        default:
                            retcode = parse_unkn_chunk(bitstr, &chunk_header, wave.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }
            else if ((RIFF_header.dwList == fcc_RF64 || RIFF_header.dwList == fcc_BW64) &&
                     RIFF_header.dwFourCC == fcc_WAVE)
            {
                if (next_bits(bitstr, 32) == fcc_LIST)
                {
                    RiffList_t list_header;
                    retcode = parse_list_header(bitstr, &list_header);

                    switch (list_header.dwFourCC)
                    {
                    default:
                        retcode = parse_unkn_list(bitstr, &list_header, wave.xml);
                        break;
                    }

                    retcode = jumpy_riff(bitstr, &RIFF_header, list_header.offset_end);
                }
                else
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
                    case fcc_plst:
                        retcode = parse_plst(bitstr, &chunk_header, &wave);
                        break;
                    case fcc_bext:
                        retcode = parse_bext(bitstr, &chunk_header, &wave);
                        break;
                    case fcc_JUNK:
                        retcode = parse_JUNK(bitstr, &chunk_header, wave.xml);
                        break;
                    case fcc_PAD:
                        retcode = parse_PAD(bitstr, &chunk_header, wave.xml);
                        break;
                    default:
                        retcode = parse_unkn_chunk(bitstr, &chunk_header, wave.xml);
                        break;
                    }

                    retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                }
            }
            else // unknown list, we still want to map it
            {
                // Loop on 2st level chunks
                while (wave.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        RiffList_t list_header;
                        retcode = parse_list_header(bitstr, &list_header);

                        switch (list_header.dwFourCC)
                        {
                        default:
                            retcode = parse_unkn_list(bitstr, &list_header, wave.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, list_header.offset_end);
                    }
                    else
                    {
                        RiffChunk_t chunk_header;
                        retcode = parse_chunk_header(bitstr, &chunk_header);

                        switch (chunk_header.dwFourCC)
                        {
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &chunk_header, wave.xml);
                            break;
                        case fcc_PAD:
                            retcode = parse_PAD(bitstr, &chunk_header, wave.xml);
                            break;
                        default:
                            retcode = parse_unkn_chunk(bitstr, &chunk_header, wave.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }

            if (wave.xml) fprintf(wave.xml, "  </a>\n");
        }

        // xmlMapper
        if (xmlMapperFinalize(wave.xml) == SUCCESS)
            media->container_mapper_fd = wave.xml;

        // Convert internal WAVE representation into a MediaFile_t
        retcode = wave_indexer(media, &wave),
        media->container_profile = wave.profile;

        // Free wave_t structure content
        wave_clean(&wave);

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
