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
 * \file      avi.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// minivideo headers
#include "../../minitraces.h"
#include "../../typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"

#include "avi.h"
#include "avi_struct.h"

/* ************************************************************************** */

static int avi_indexer_initmap(VideoFile_t *video, AviTrack_t *track, int index_entry_count);

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse a list header.
 *
 * bitstr pointer is not checked for performance reason.
 */
static int parse_list_header(Bitstream_t *bitstr, AviList_t *list_header)
{
    TRACE_3(AVI, "parse_list_header()\n");
    int retcode = SUCCESS;

    if (list_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid avi_list_t structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse AVI list header
        list_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        list_header->dwList       = read_bits(bitstr, 32);
        list_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
        list_header->dwFourCC     = read_bits(bitstr, 32);
        list_header->offset_end   = list_header->offset_start + list_header->dwSize + 8;

        if (list_header->dwList != fcc_RIFF &&
            list_header->dwList != fcc_LIST)
        {
            TRACE_1(AVI, "We are looking for a AVI list, however this is neither a LIST nor a RIFF (0x%04X)\n", list_header->dwList);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print an AVI list header.
 */
static void print_list_header(AviList_t *list_header)
{
    TRACE_2(AVI, "* offset_s\t: %u\n", list_header->offset_start);
    TRACE_2(AVI, "* offset_e\t: %u\n", list_header->offset_end);

    if (list_header->dwList == fcc_RIFF)
    {
        TRACE_2(AVI, "* RIFF header\n");
    }
    else
    {
        TRACE_2(AVI, "* LIST header\n");
    }

    TRACE_2(AVI, "* LIST size\t: %u\n", list_header->dwSize);
    TRACE_2(AVI, "* LIST fcc\t: 0x%08X\n", list_header->dwFourCC);
}

/* ************************************************************************** */

/*!
 * \brief Skip a list header and content.
 */
static int skip_list(Bitstream_t *bitstr, AviList_t *list_header_parent, AviList_t *list_header_child)
{
    int retcode = FAILURE;

    if (list_header_child->dwSize != 0)
    {
        int64_t jump = list_header_child->dwSize * 8;
        int64_t offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check that we do not jump outside the parent list boundaries
        if ((offset + jump) > list_header_parent->offset_end)
        {
            jump = list_header_parent->offset_end - offset;
        }

        if (skip_bits(bitstr, jump) == FAILURE)
        {
            TRACE_ERROR(AVI, "> skip_list() >> Unable to skip %i bytes!\n", list_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(AVI, "> skip_list() >> %i bytes\n", list_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(AVI, "> skip_list() >> do it yourself!\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse box header.
 *
 * bitstr pointer is not checked for performance reason.
 */
static int parse_chunk_header(Bitstream_t *bitstr, AviChunk_t *chunk_header)
{
    TRACE_3(AVI, "parse_chunk_header()\n");
    int retcode = SUCCESS;

    if (chunk_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid avi_chunk_t structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse AVI chunk header
        chunk_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);
        chunk_header->dwFourCC     = read_bits(bitstr, 32);
        chunk_header->dwSize       = endian_flip_32(read_bits(bitstr, 32));
        chunk_header->offset_end   = chunk_header->offset_start + chunk_header->dwSize + 8;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print an AVI chunk header.
 */
static void print_chunk_header(AviChunk_t *chunk_header)
{
    TRACE_2(AVI, "* offset_s\t: %u\n", chunk_header->offset_start);
    TRACE_2(AVI, "* offset_e\t: %u\n", chunk_header->offset_end);

    TRACE_2(AVI, "* CHUNK fcc\t: 0x%08X\n", chunk_header->dwFourCC);
    TRACE_2(AVI, "* CHUNK size: %u\n", chunk_header->dwSize);
}

/* ************************************************************************** */

/*!
 * \brief Skip a chunk header and content.
 */
static int skip_chunk(Bitstream_t *bitstr, AviList_t *list_header_parent, AviChunk_t *chunk_header_child)
{
    int retcode = FAILURE;

    if (chunk_header_child->dwSize != 0)
    {
        int64_t jump = chunk_header_child->dwSize * 8;
        int64_t offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check that we do not jump outside the parent list boundaries
        if ((offset + jump) > list_header_parent->offset_end)
        {
            jump = list_header_parent->offset_end - offset;
        }

        if (skip_bits(bitstr, jump) == FAILURE)
        {
            TRACE_ERROR(AVI, "> skip_chunk() >> Unable to skip %i bytes!\n", chunk_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(AVI, "> skip_chunk() >> %i bytes\n", chunk_header_child->dwSize);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(AVI, "> skip_chunk() >> do it yourself!\n");
        retcode = FAILURE;
    }

    print_chunk_header(chunk_header_child);

    return retcode;
}

/* ************************************************************************** */

static int parse_JUNK(Bitstream_t *bitstr, AviList_t *list_header_parent, AviChunk_t *chunk_header_child)
{
    int retcode = FAILURE;

    if (chunk_header_child->dwSize != 0)
    {
        int64_t jump = chunk_header_child->dwSize * 8;
        int64_t offset = bitstream_get_absolute_byte_offset(bitstr);

        // Check that we do not jump outside the parent list boundaries
        if ((offset + jump) > list_header_parent->offset_end)
        {
            jump = list_header_parent->offset_end - offset;
        }

        if (skip_bits(bitstr, jump) == FAILURE)
        {
            TRACE_ERROR(AVI, GREEN "parse_JUNK()" RESET " >> Unable to skip %i bytes\n", chunk_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(AVI, GREEN "parse_JUNK()\n" RESET);
            retcode = SUCCESS;
        }
    }
    else
    {
        TRACE_WARNING(AVI, "parse_JUNK() >> Unable to skip this chunk!\n");
        retcode = FAILURE;
    }

    print_chunk_header(chunk_header_child);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int parse_string(Bitstream_t *bitstr, AviChunk_t *chunk_header)
{
    TRACE_INFO(AVI, GREEN "parse_string()\n" RESET);
    int retcode = SUCCESS;

    if (chunk_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid chunk_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        char *string = malloc(chunk_header->dwSize);

        if (string == NULL)
        {
            TRACE_ERROR(AVI, "Unable to allocate string!\n");
            retcode = FAILURE;
        }
        else
        {
            int i = 0;
            for (i = 0; i < chunk_header->dwSize; i++)
            {
                string[i] = read_bits(bitstr, 8);
            }

#if ENABLE_DEBUG
            // Chunk Header
            print_chunk_header(chunk_header);

            // Chunk content
            TRACE_1(AVI, "> '");
            for (i = 0; i < chunk_header->dwSize; i++)
            {
                printf("%c", string[i]);
            }
            printf("'\n");
#endif /* ENABLE_DEBUG */
            free(string);
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_avih(Bitstream_t *bitstr, AviChunk_t *avih_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_avih()\n" RESET);
    int retcode = SUCCESS;

    if (avih_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid avih_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse avih content
        avi->avih.dwMicroSecPerFrame = endian_flip_32(read_bits(bitstr, 32)); // frame display rate (or 0)
        avi->avih.dwMaxBytesPerSec = endian_flip_32(read_bits(bitstr, 32)); // max. transfer rate
        avi->avih.dwPaddingGranularity = endian_flip_32(read_bits(bitstr, 32)); // pad to multiples of this size;
        avi->avih.dwFlags = endian_flip_32(read_bits(bitstr, 32)); // the ever-present flags
        avi->avih.dwTotalFrames = endian_flip_32(read_bits(bitstr, 32)); // # frames in file
        avi->avih.dwInitialFrames = endian_flip_32(read_bits(bitstr, 32));
        avi->avih.dwStreams = endian_flip_32(read_bits(bitstr, 32));
        avi->avih.dwSuggestedBufferSize = endian_flip_32(read_bits(bitstr, 32));
        avi->avih.dwWidth = endian_flip_32(read_bits(bitstr, 32));
        avi->avih.dwHeight = endian_flip_32(read_bits(bitstr, 32));

        uint32_t dwReserved[4];
        dwReserved[0] = endian_flip_32(read_bits(bitstr, 32));
        dwReserved[1] = endian_flip_32(read_bits(bitstr, 32));
        dwReserved[2] = endian_flip_32(read_bits(bitstr, 32));
        dwReserved[3] = endian_flip_32(read_bits(bitstr, 32));

        // Print chunk header
        print_chunk_header(avih_header);

        // Print avih content
        TRACE_1(AVI, "> dwMicroSecPerFrame\t: %u\n", avi->avih.dwMicroSecPerFrame);
        TRACE_1(AVI, "> dwMaxBytesPerSec\t\t: %u\n", avi->avih.dwMaxBytesPerSec);
        TRACE_1(AVI, "> dwPaddingGranularity\t: %u\n", avi->avih.dwPaddingGranularity);
        TRACE_1(AVI, "> dwFlags\t\t\t: %u\n", avi->avih.dwFlags);
        TRACE_1(AVI, "> dwTotalFrames\t\t: %u\n", avi->avih.dwTotalFrames);
        TRACE_1(AVI, "> dwInitialFrames\t\t: %u\n", avi->avih.dwInitialFrames);
        TRACE_1(AVI, "> dwStreams\t\t\t: %u\n", avi->avih.dwStreams);
        TRACE_1(AVI, "> dwSuggestedBufferSize\t: %u\n", avi->avih.dwSuggestedBufferSize);
        TRACE_1(AVI, "> dwWidth\t\t\t: %u\n", avi->avih.dwWidth);
        TRACE_1(AVI, "> dwHeight\t\t\t: %u\n", avi->avih.dwHeight);
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_dmlh(Bitstream_t *bitstr, AviChunk_t *dmlh_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_dmlh()\n" RESET);
    int retcode = SUCCESS;

    if (dmlh_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid dmlh_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Print chunk header
        print_chunk_header(dmlh_header);

        // Parse chunk content
        // Update the (unreliable) dwTotalFrames from the avi header
        avi->avih.dwTotalFrames = endian_flip_32(read_bits(bitstr, 32));

        // Print dmlh content
        TRACE_1(AVI, "> dwTotalFrames\t: %u\n", avi->avih.dwTotalFrames);
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strh(Bitstream_t *bitstr, AviChunk_t *strh_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, GREEN "parse_strh()\n" RESET);
    int retcode = SUCCESS;

    if (strh_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid strh_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Parse chunk content
        track->strh.fccType = read_bits(bitstr, 32);
        track->strh.fccHandler = read_bits(bitstr, 32);
        track->strh.dwFlags = endian_flip_32(read_bits(bitstr, 32));
        track->strh.wPriority = endian_flip_16(read_bits(bitstr, 16));
        track->strh.wLanguage = endian_flip_16(read_bits(bitstr, 16));
        track->strh.dwInitialFrames = endian_flip_32(read_bits(bitstr, 32));
        track->strh.dwScale = endian_flip_32(read_bits(bitstr, 32));
        track->strh.dwRate = endian_flip_32(read_bits(bitstr, 32)); /* dwRate / dwScale == samples/second */
        track->strh.dwStart = endian_flip_32(read_bits(bitstr, 32));
        track->strh.dwLength = endian_flip_32(read_bits(bitstr, 32)); /* In units above... */
        track->strh.dwSuggestedBufferSize = endian_flip_32(read_bits(bitstr, 32));
        track->strh.dwQuality = endian_flip_32(read_bits(bitstr, 32));
        track->strh.dwSampleSize = endian_flip_32(read_bits(bitstr, 32));

        track->strh.rcFrame_x = endian_flip_16(read_bits(bitstr, 16));
        track->strh.rcFrame_y = endian_flip_16(read_bits(bitstr, 16));
        track->strh.rcFrame_w = endian_flip_16(read_bits(bitstr, 16));
        track->strh.rcFrame_h = endian_flip_16(read_bits(bitstr, 16));

        // Print chunk header
        print_chunk_header(strh_header);

        // Print strh content
        TRACE_1(AVI, "> fccType\t\t: 0x%08X\n", track->strh.fccType);
        TRACE_1(AVI, "> fccHandler\t: 0x%08X\n", track->strh.fccHandler);
        TRACE_1(AVI, "> dwFlags\t\t: %u\n", track->strh.dwFlags);
        TRACE_1(AVI, "> wPriority\t\t: %u\n", track->strh.wPriority);
        TRACE_1(AVI, "> wLanguage\t\t: %u\n", track->strh.wLanguage);
        TRACE_1(AVI, "> dwInitialFrames\t: %u\n", track->strh.dwInitialFrames);
        TRACE_1(AVI, "> dwScale\t\t: %u\n", track->strh.dwScale);
        TRACE_1(AVI, "> dwRate\t\t: %u\n", track->strh.dwRate);
        TRACE_1(AVI, "> dwStart\t\t: %u\n", track->strh.dwStart);
        TRACE_1(AVI, "> dwLength\t\t: %u\n", track->strh.dwLength);
        TRACE_1(AVI, "> dwSuggestedBufferSize : %u\n", track->strh.dwSuggestedBufferSize);
        TRACE_1(AVI, "> dwQuality\t\t: %u\n", track->strh.dwQuality);
        TRACE_1(AVI, "> dwSampleSize\t: %u\n", track->strh.dwSampleSize);
        TRACE_1(AVI, "> rcFrame_x\t\t: %u\n", track->strh.rcFrame_x);
        TRACE_1(AVI, "> rcFrame_y\t\t: %u\n", track->strh.rcFrame_y);
        TRACE_1(AVI, "> rcFrame_w\t\t: %u\n", track->strh.rcFrame_w);
        TRACE_1(AVI, "> rcFrame_h\t\t: %u\n", track->strh.rcFrame_h);
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strf(Bitstream_t *bitstr, AviChunk_t *strf_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, GREEN "parse_strf()\n" RESET);
    int retcode = SUCCESS;

    if (strf_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid strf_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Print chunk header
        print_chunk_header(strf_header);

        // Parse chunk content
        if (track->strh.fccType == fcc_vids)
        {
            // Parse BITMAPINFOHEADER
            track->strf.biSize = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biWidth = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biHeight = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biPlanes = endian_flip_16(read_bits(bitstr, 16));
            track->strf.biBitCount = endian_flip_16(read_bits(bitstr, 16));
            track->strf.biCompression = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biSizeImage = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biXPelsPerMeter = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biYPelsPerMeter = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biClrUsed = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biClrImportant = endian_flip_32(read_bits(bitstr, 32));

            TRACE_1(AVI, "> biSize\t\t: %u\n", track->strf.biSize);
            TRACE_1(AVI, "> biWidth\t\t: %i\n", track->strf.biWidth);
            TRACE_1(AVI, "> biHeight\t\t: %i\n", track->strf.biHeight);
            TRACE_1(AVI, "> biPlanes\t\t: %u\n", track->strf.biPlanes);
            TRACE_1(AVI, "> biBitCount\t: %u\n", track->strf.biBitCount);
            TRACE_1(AVI, "> biCompression\t: %u\n", track->strf.biCompression);
            TRACE_1(AVI, "> biSizeImage\t: %u\n", track->strf.biSizeImage);
            TRACE_1(AVI, "> biXPelsPerMeter\t: %i\n", track->strf.biXPelsPerMeter);
            TRACE_1(AVI, "> biYPelsPerMeter\t: %i\n", track->strf.biYPelsPerMeter);
            TRACE_1(AVI, "> biClrUsed\t\t: %u\n", track->strf.biClrUsed);
            TRACE_1(AVI, "> biClrImportant\t: %u\n", track->strf.biClrImportant);
        }
        else if (track->strh.fccType == fcc_auds)
        {
            // Parse WAVEFORMATEX
            track->strf.wFormatTag = endian_flip_16(read_bits(bitstr, 16));
            track->strf.nChannels = endian_flip_16(read_bits(bitstr, 16));
            track->strf.nSamplesPerSec = endian_flip_32(read_bits(bitstr, 32));
            track->strf.nAvgBytesPerSec = endian_flip_32(read_bits(bitstr, 32));
            track->strf.nBlockAlign = endian_flip_16(read_bits(bitstr, 16));
            track->strf.wBitsPerSample = endian_flip_16(read_bits(bitstr, 16));

            TRACE_1(AVI, "> wFormatTag\t: %u\n", track->strf.wFormatTag);
            TRACE_1(AVI, "> nChannels\t\t: %u\n", track->strf.nChannels);
            TRACE_1(AVI, "> nSamplesPerSec\t: %u\n", track->strf.nSamplesPerSec);
            TRACE_1(AVI, "> nAvgBytesPerSec\t: %u\n", track->strf.nAvgBytesPerSec);
            TRACE_1(AVI, "> nBlockAlign\t: %u\n", track->strf.nBlockAlign);
            TRACE_1(AVI, "> wBitsPerSample\t: %u\n", track->strf.wBitsPerSample);

            // Parse WAVEFORMATEX extention
            if (track->strf.wFormatTag == wftag_UNKNOWN)
            {
                TRACE_2(AVI, "> EXT > No wave format TAG\n");
                track->strh.fccHandler = CODEC_UNKNOWN;
            }
            else
            {
                int byte_left = strf_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

                if (track->strf.wFormatTag == wftag_MP1)
                {
                    TRACE_1(AVI, "> EXT > MPEG1WAVEFORMAT\n");
                    track->strh.fccHandler = CODEC_MPEG_L1;

                    if (byte_left >= 24)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));
                        TRACE_1(AVI, "> cbSize\t\t: %u\n", cbSize);

                        uint16_t fwHeadLayer = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwHeadBitrate = endian_flip_32(read_bits(bitstr, 32));
                        uint16_t fwHeadMode = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t fwHeadModeExt = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t wHeadEmphasis = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t fwHeadFlags = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwPTSLow = endian_flip_32(read_bits(bitstr, 32));
                        uint32_t dwPTSHigh = endian_flip_32(read_bits(bitstr, 32));

                        TRACE_1(AVI, "> fwHeadLayer\t: %u\n", fwHeadLayer);
                        TRACE_1(AVI, "> dwHeadBitrate\t: %u\n", dwHeadBitrate);
                        TRACE_1(AVI, "> fwHeadMode\t: %u\n", fwHeadMode);
                        TRACE_1(AVI, "> fwHeadModeExt\t: %u\n", fwHeadModeExt);
                        TRACE_1(AVI, "> wHeadEmphasis\t: %u\n", wHeadEmphasis);
                        TRACE_1(AVI, "> fwHeadFlags\t: %u\n", fwHeadFlags);
                        TRACE_1(AVI, "> dwPTSLow\t: %u\n", dwPTSLow);
                        TRACE_1(AVI, "> dwPTSHigh\t: %u\n", dwPTSHigh);
                    }
                }
                else if (track->strf.wFormatTag == wftag_MP3)
                {
                    TRACE_1(AVI, "> EXT > MPEGLAYER3WAVEFORMAT\n");
                    track->strh.fccHandler = CODEC_MPEG_L3;

                    if (byte_left >= 11)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));
                        TRACE_1(AVI, "> cbSize\t\t: %u\n", cbSize);

                        uint16_t wID = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t fdwFlags = endian_flip_32(read_bits(bitstr, 32));
                        uint16_t nBlockSize = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t nFramesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t nCodecDelay = endian_flip_16(read_bits(bitstr, 16));

                        TRACE_1(AVI, "> wID\t\t: %u\n", wID);
                        TRACE_1(AVI, "> fdwFlags\t\t: %u\n", fdwFlags);
                        TRACE_1(AVI, "> nBlockSize\t: %u\n", nBlockSize);
                        TRACE_1(AVI, "> nFramesPerBlock\t: %u\n", nFramesPerBlock);
                        TRACE_1(AVI, "> nCodecDelay\t: %u\n", nCodecDelay);
                    }
                }
                else if (track->strf.wFormatTag == wftag_WAV)
                {
                    TRACE_1(AVI, "> EXT > MPEG1WAVEFORMAT\n");
                    track->strh.fccHandler = CODEC_PCM;

                    if (byte_left >= 28)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));
                        TRACE_1(AVI, "> cbSize\t\t: %u\n", cbSize);

                        uint16_t samples_wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t samples_wSamplesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t samples_wReserved = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                        uint8_t SubFormat_GUID[16];
                        int i = 0;
                        for (i = 0; i < 16; i++)
                        {
                            SubFormat_GUID[i] = read_bits(bitstr, 8);
                        }

                        TRACE_1(AVI, "> samples_wValidBitsPerSample\t: %u\n", samples_wValidBitsPerSample);
                        TRACE_1(AVI, "> samples_wSamplesPerBlock\t: %u\n", samples_wSamplesPerBlock);
                        TRACE_1(AVI, "> samples_wReserved\t: %u\n", samples_wReserved);
                        TRACE_1(AVI, "> dwChannelMask\t: %u\n", dwChannelMask);
                        TRACE_1(AVI, "> SubFormat_GUID\t: [%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u]\n",
                                SubFormat_GUID[0], SubFormat_GUID[1], SubFormat_GUID[2], SubFormat_GUID[3],
                                SubFormat_GUID[4], SubFormat_GUID[5], SubFormat_GUID[6], SubFormat_GUID[7],
                                SubFormat_GUID[8], SubFormat_GUID[9], SubFormat_GUID[10], SubFormat_GUID[11],
                                SubFormat_GUID[12], SubFormat_GUID[13], SubFormat_GUID[14], SubFormat_GUID[15]);
                    }
                }
                else if  (track->strf.wFormatTag == wftag_AAC)
                {
                    TRACE_1(AVI, "> EXT > AAC\n");
                    track->strh.fccHandler = CODEC_AAC;
                }
                else if  (track->strf.wFormatTag == wftag_AC3)
                {
                    TRACE_1(AVI, "> EXT > AC3\n");
                    track->strh.fccHandler = CODEC_AC3;
                }
                else if  (track->strf.wFormatTag == wftag_DTS)
                {
                    TRACE_1(AVI, "> EXT > DTS\n");
                    track->strh.fccHandler = CODEC_DTS;
                }
            }
        }
    }

    bitstream_print_absolute_byte_offset(bitstr);

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse AVI 1.0 Index.
 *
 * The index as described is the index you will find in AVI 1.0 files. It is
 * placed after the movi list in the RIFF AVI List.
 */
static int parse_idx1(Bitstream_t *bitstr, VideoFile_t *video, AviChunk_t *idx1_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_idx1()\n" RESET);
    int retcode = SUCCESS;
    int i = 0;

    if (idx1_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid idx1_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Print chunk header
        print_chunk_header(idx1_header);

        // Compute number of index entries
        uint32_t index_entry_count = idx1_header->dwSize / 16;
        TRACE_1(AVI, "> index_entry : %u\n", index_entry_count);

        // Check if the tracks have already been indexed
        int track_left = 0;
        for (i = 0; i < avi->tracks_count; i++)
        {
            if (avi->tracks[i]->track_indexed == false &&
                avi->tracks[i]->superindex_count == 0)
            {
                track_left++;
                retcode = avi_indexer_initmap(video, avi->tracks[i], index_entry_count);
            }
        }

        if (track_left == 0)
        {
            TRACE_1(AVI, "> We have an other way to index the file content\n");
            return SUCCESS;
        }

        if (retcode == SUCCESS)
        {
            uint32_t movioffset = avi->movi_offset + 4; // +4 to skip the chunk size
            TRACE_1(AVI, "> movi_offset : %i\n", movioffset);

            // Parse index structure
            for (i = 0; i < index_entry_count; i++)
            {
                uint32_t dwChunkId = read_bits(bitstr, 32);
                uint32_t dwFlags = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwChunkOffset = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwChunkLength = endian_flip_32(read_bits(bitstr, 32));

                // Set sample into bitstream map
                if ((dwChunkId & 0x0000FFFF) == 0x7762) // wb: audio
                {
                    TRACE_3(AVI, BLUE "> AUDIO\n" RESET);
                    int tid = 0;
                    int sid = video->tracks_audio[tid]->sample_count;
                    video->tracks_audio[tid]->sample_count++;

                    video->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                    video->tracks_audio[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    video->tracks_audio[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    video->tracks_audio[tid]->sample_dts[sid] = -1;
                    video->tracks_audio[tid]->sample_pts[sid] = -1;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x6463) // dc: video
                {
                    TRACE_3(AVI, BLUE "> VIDEO\n" RESET);
                    int tid = 0;
                    int sid = video->tracks_video[tid]->sample_count;
                    video->tracks_video[tid]->sample_count++;

                    if (dwFlags == AVIIF_KEYFRAME)
                        video->tracks_video[tid]->sample_type[sid] = sample_VIDEO_IDR;
                    else
                        video->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                    video->tracks_video[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    video->tracks_video[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    video->tracks_video[tid]->sample_dts[sid] = -1;
                    video->tracks_video[tid]->sample_pts[sid] = -1;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x7478) // tx: subtitles
                {
                    TRACE_3(AVI, BLUE "> TEXT\n" RESET);
                    int tid = 0;
                    int sid = video->tracks_subtitles[tid]->sample_count;

                    video->tracks_subtitles[tid]->sample_type[sid] = sample_TEXT_FILE;
                    video->tracks_subtitles[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    video->tracks_subtitles[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    video->tracks_subtitles[tid]->sample_dts[sid] = -1;
                    video->tracks_subtitles[tid]->sample_pts[sid] = -1;
                    video->tracks_subtitles[tid]->sample_count++;
                }
/*
                else if ((dwChunkId & 0x0000FFFF) == 0x6462) // db: Uncompressed video frame (RGB)
                {
                    //
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x7063) // pc: Palette change
                {
                    //
                }
                else if ((dwChunkId & 0xFFFF0000) == 0x6978) // ix: index
                {
                    //
                }
*/
                else
                {
                    TRACE_WARNING(AVI, "Unknown chunk type in idx1 (0x%08X)!\n", dwChunkId);
                }

                // Print
                TRACE_3(AVI, "> dwChunkId\t: 0x%08X\n", dwChunkId);
                TRACE_3(AVI, "> dwFlags\t\t: %u\n", dwFlags);
                TRACE_3(AVI, "> dwChunkOffset\t: %u\n", dwChunkOffset);
                TRACE_3(AVI, "> dwChunkLength\t: %u\n", dwChunkLength);
            }

            for (i = 0; i < avi->tracks_count; i++)
            {
                avi->tracks[i]->track_indexed = true;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse Open-DML 'Super Index'.
 *
 * Upper Level Index ('Super Index') contained in each 'strl' list.
 * The actual size of the entries of the aIndex array is entered into
 * wLongsPerEntry. Between the cb field and wLongsPerEntry the actual size of
 * the array is known. The field nEntriesInUse allows a chunk to be allocated
 * longer than the actual number of used elements in the array.
 */
static int parse_indx(Bitstream_t *bitstr, AviChunk_t *indx_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, GREEN "parse_indx()\n" RESET);
    int retcode = SUCCESS;

    if (indx_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid indx_header structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Print chunk header
        print_chunk_header(indx_header);

        // Parse chunk content
        uint16_t wLongsPerEntry = endian_flip_16(read_bits(bitstr, 16));
        uint8_t bIndexSubType = read_bits(bitstr, 8);
        uint8_t bIndexType = read_bits(bitstr, 8);
        uint32_t nEntriesInUse = endian_flip_32(read_bits(bitstr, 32));
        uint32_t dwChunkId = read_bits(bitstr, 32);

        TRACE_1(AVI, "> wLongsPerEntry\t: %u\n", wLongsPerEntry);
        TRACE_1(AVI, "> bIndexSubType\t: %u\n", bIndexSubType);
        TRACE_1(AVI, "> bIndexType\t: %u\n", bIndexType);
        TRACE_1(AVI, "> nEntriesInUse\t: %u\n", nEntriesInUse);
        TRACE_1(AVI, "> dwChunkId\t: 0x%08X\n", dwChunkId);

        // Index specifics code
        ////////////////////////////////////////////////////////////////////////

        if (bIndexType == AVI_INDEX_OF_INDEXES)
        {
            TRACE_INFO(AVI, GREEN "> AVI Super Index Chunk\n" RESET);

            uint32_t dwReserved[3];
            dwReserved[0] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[1] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[2] = endian_flip_32(read_bits(bitstr, 32));

            // Parse super index entries
            int i = 0;
            for (i = 0; i < nEntriesInUse; i++)
            {
                track->superindex_count++;
                track->superindex_entries[i].offset = endian_flip_64(read_bits_64(bitstr, 64));
                track->superindex_entries[i].size = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwDuration = endian_flip_32(read_bits(bitstr, 32));

                TRACE_1(AVI, " > qwOffset\t\t= %lli\n", track->superindex_entries[i].offset);
                TRACE_1(AVI, "  > dwSize\t\t: %lli\n", track->superindex_entries[i].size);
                TRACE_1(AVI, "  > dwDuration\t: %u\n", dwDuration);
            }
        }
        else if (bIndexType == AVI_INDEX_OF_CHUNKS)
        {
            // AVI Standard Index Chunk // AVI Field Index Chunk
            TRACE_INFO(AVI, GREEN "> AVI Standard Index Chunk\n" RESET);

            // Alloc index entries
            int start = 0;
            if (track->index_count == 0)
            {
                track->index_count = nEntriesInUse;
                track->index_entries = (AviIndexEntries_t*)calloc(track->index_count, sizeof(AviIndexEntries_t));
            }
            else
            {
                start = track->index_count;
                track->index_count += nEntriesInUse;
                track->index_entries = realloc(track->index_entries, track->index_count * sizeof(AviIndexEntries_t));
            }

            // Parse index entries
            int64_t qwOffset_base = endian_flip_64(read_bits_64(bitstr, 64));
            uint32_t dwReserved3 = endian_flip_32(read_bits(bitstr, 32));

            TRACE_1(AVI, " > qwOffset_base\t: %lli\n", qwOffset_base);

            int i = 0;
            for (i = start; i < track->index_count; i++)
            {
                uint32_t dwOffset = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwSize = endian_flip_32(read_bits(bitstr, 32));

                track->index_entries[i].offset = qwOffset_base + dwOffset;
                track->index_entries[i].size = dwSize & 0x7FFFFFFF;
                track->index_entries[i].pts = -1;

                if ((dwSize & 0x10000000) == 0)
                    track->index_entries[i].flags = AVIIF_KEYFRAME;

                TRACE_1(AVI, "  > dwOffset\t= %lli\n", track->index_entries[i].offset);
                TRACE_1(AVI, "   > dwSize\t: %lli\n", track->index_entries[i].size);
            }
        }
        else if (bIndexType == AVI_INDEX_IS_DATA)
        {
            TRACE_WARNING(AVI, "AVI_INDEX_IS_DATA index > unsupported\n");
            retcode = FAILURE;
        }
        else
        {
            TRACE_ERROR(AVI, "Unknown type of index: fatal error\n");
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief The Stream header list, general.
 *
 * There is one strl list for each stream. If the number of strl lists
 * inside the hdrl list is diferent from MainAVIHeader::dwStreams, a fatal
 * error should be reported.
 *
 * The Stream header list only contains chunk.
 */
static int parse_strl(Bitstream_t *bitstr, AviList_t *strl_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_strl()\n" RESET);
    int retcode = SUCCESS;
    int track_id = 0;

    if (strl_header != NULL ||
        strl_header->dwFourCC == fcc_strl)
    {
        // Print list header
        print_list_header(strl_header);

        // Bytes left in the strl list
        int byte_left = strl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Bytes left in the lists / chunks inside the strl list
        int jump = 0;

        // Init a new AviTrack_t structure to store strl content
        track_id = avi->tracks_count;
        avi->tracks[track_id] = (AviTrack_t*)calloc(1, sizeof(AviTrack_t));

        if (avi->tracks[track_id] == NULL)
        {
            TRACE_ERROR(AVI, "Unable to allocate a new avi track!\n");
            retcode = FAILURE;
        }
        else
        {
            avi->tracks[track_id]->track_id = track_id;
            avi->tracks_count++;

            avi->tracks[track_id]->track_indexed = 0;
            avi->tracks[track_id]->superindex_count = 0;
            avi->tracks[track_id]->index_count = 0;
        }

        // Loop on "strl" content
        while (retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < strl_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                AviList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type (0x%08X) @ %i\n", list_header.dwFourCC);
                    retcode = skip_list(bitstr, strl_header, &list_header);
                    break;
                }

                // Byte left in the list we just left?
                jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }
            else
            {
                AviChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_strh:
                    retcode = parse_strh(bitstr, &chunk_header, avi->tracks[track_id]);
                    break;
                case fcc_strf:
                    retcode = parse_strf(bitstr, &chunk_header, avi->tracks[track_id]);
                    break;
                //case fcc_strd:
                //    retcode = parse_strd(bitstr, &chunk_header, avi->tracks[track_id]);
                //    break;
                case fcc_strn:
                    TRACE_INFO(AVI, GREEN "parse_strn()\n" RESET);
                    print_chunk_header(&chunk_header);
                    retcode = parse_string(bitstr, &chunk_header);
                    break;
                case fcc_indx:
                    retcode = parse_indx(bitstr, &chunk_header, avi->tracks[track_id]);
                    break;
                case fcc_JUNK:
                    retcode = parse_JUNK(bitstr, strl_header, &chunk_header);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type!\n");
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, strl_header, &chunk_header);
                    break;
                }

                // Byte left in the chunk we just left?
                jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }

            // Skip the byte left before the next list / chunk
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }

            // Byte left in the strl list?
            byte_left = strl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a strl box\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief ODML extension list.
 *
 * ODML.
 *
 * The ODML list only contains chunk.
 */
static int parse_odml(Bitstream_t *bitstr, AviList_t *odml_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_odml()\n" RESET);
    int retcode = SUCCESS;

    if (odml_header != NULL &&
        odml_header->dwFourCC == fcc_odml)
    {
        // Print list header
        print_list_header(odml_header);

        // Bytes left in the odml list
        int byte_left = odml_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Bytes left in the lists / chunks inside the odml list
        int jump = 0;

        // Loop on "odml" content
        while (retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < odml_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                AviList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type (0x%08X) @ %i\n", list_header.dwFourCC);
                    retcode = skip_list(bitstr, odml_header, &list_header);
                    break;
                }

                // Byte left in the list we just left?
                jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }
            else
            {
                AviChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_dmlh:
                    retcode = parse_dmlh(bitstr, &chunk_header, avi);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type (0x%08X) @ %i\n", chunk_header.dwFourCC);
                    retcode = skip_chunk(bitstr, odml_header, &chunk_header);
                    break;
                }

                // Byte left in the chunk we just left?
                jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }

            // Skip the byte left before the next list / chunk
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }

            // Byte left in the odml list?
            byte_left = odml_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a odml box\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief The Stream header list, general.
 *
 * The Movi - Lists contain Video, Audio, Subtitle and (secondary) index data.
 * Those can be grouped into rec - Lists.
 *
 * Grouping chunks into rec - Lists prevents excessive seeking when using the
 * Microsoft AVI splitter for replay, but does not allow playback on some
 * standalone replay devices.
 * The maximum size of a chunk of a stream should be smaller than the
 * corresponding dwSuggestedBufferSize value. Otherwise, some players,
 * especially the Microsoft AVI splitter, could malfunction.
 *
 * The following chunk header IDs are defined:
 * - xxwb: audio chunk.
 * - xxdc: video chunk.
 * - xxtx: subtitle chunk.
 * - ix..: standard index block.
 */
static int parse_movi(Bitstream_t *bitstr, AviList_t *movi_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_movi()\n" RESET);
    int retcode = SUCCESS;

    if (movi_header != NULL &&
        movi_header->dwFourCC == fcc_movi)
    {
        // Print list header
        print_list_header(movi_header);

        // Skip "movi" content
        avi->movi_offset = movi_header->offset_start + 12; // +12 to skip movi header fields
        return bitstream_goto_offset(bitstr, movi_header->offset_end);

        ////////////////////////////////////////////////////////////////////////

        // Loop on "movi" content
        // Only useful if we want to index the content by hand
        while (retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < movi_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                AviList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                case fcc_rec_:
                default:
                    TRACE_WARNING(AVI, "Unknown list type\n");
                    print_list_header(&list_header);
                    retcode = skip_list(bitstr, movi_header, &list_header);
                    break;
                }
            }
            else
            {
                AviChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type\n");
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, movi_header, &chunk_header);
                    break;
                }

                // Go to the next box
                int jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                if (jump > 0)
                {
                    skip_bits(bitstr, jump*8);
                }
            }
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a movi box\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_INFO(Bitstream_t *bitstr, AviList_t *INFO_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_INFO()\n" RESET);
    int retcode = SUCCESS;

    if (INFO_header != NULL &&
        INFO_header->dwFourCC == fcc_INFO)
    {
        // Print INFO list header
        print_list_header(INFO_header);

        // Bytes left in the INFO list
        int byte_left = INFO_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Bytes left in the lists / chunks inside the INFO list
        int jump = 0;

        // Loop on "INFO" content
        while (retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < INFO_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                AviList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type\n");
                    print_list_header(&list_header);
                    retcode = skip_list(bitstr, INFO_header, &list_header);
                    break;
                }

                // Byte left in the list?
                jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }
            else
            {
                AviChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_ISFT:
                    retcode = parse_string(bitstr, &chunk_header);
                    break;
                case fcc_JUNK:
                    retcode = parse_JUNK(bitstr, INFO_header, &chunk_header);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type\n");
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, INFO_header, &chunk_header);
                    break;
                }

                // Byte left in the chunk?
                jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }

            // Jump to the next list / chunk
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }

            // Byte left in the INFO box?
            byte_left = INFO_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a INFO box\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_hdrl(Bitstream_t *bitstr, AviList_t *hdrl_header, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "parse_hdrl()\n" RESET);
    int retcode = SUCCESS;

    if (hdrl_header != NULL &&
        hdrl_header->dwFourCC == fcc_hdrl)
    {
        // Print list header
        print_list_header(hdrl_header);

        // Bytes left in the hdrl list
        int byte_left = hdrl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Bytes left in the lists / chunks inside the hdrl list
        int jump = 0;

        // Loop on "hdrl" content
        while (retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < hdrl_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                AviList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                case fcc_strl:
                    retcode = parse_strl(bitstr, &list_header, avi);
                    break;
                case fcc_odml:
                    retcode = parse_odml(bitstr, &list_header, avi);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown list type\n");
                    print_list_header(&list_header);
                    retcode = skip_list(bitstr, hdrl_header, &list_header);
                    break;
                }

                // Byte left in the list we just left?
                jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }
            else
            {
                AviChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_avih:
                    retcode = parse_avih(bitstr, &chunk_header, avi);
                    break;
                case fcc_JUNK:
                    retcode = parse_JUNK(bitstr, hdrl_header, &chunk_header);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type\n");
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, hdrl_header, &chunk_header);
                    break;
                }

                // Byte left in the chunk we just left?
                jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            }

            // Skip the byte left before the next list / chunk
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }

            // Byte left in the hdrl list?
            byte_left = hdrl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a hdrl box\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

static int avi_indexer_initmap(VideoFile_t *video, AviTrack_t *track, int index_entry_count)
{
    // Init a bitstreamMap_t for each avi track
    int retcode = SUCCESS;
    BitstreamMap_t *mytrack = NULL;

    if (track->strh.fccType == fcc_auds)
    {
        // Audio track
        retcode = init_bitstream_map(&video->tracks_audio[video->tracks_audio_count], index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = video->tracks_audio[video->tracks_audio_count];
            video->tracks_audio_count++;

            mytrack->stream_level = stream_level_ES;
            mytrack->stream_type  = stream_AUDIO;

            // We cannot rely on the fccHandler from the container,
            // but we should have extracted the correct codec infos from strh
            mytrack->stream_codec = track->strh.fccHandler;

            mytrack->sample_alignment = true;

            mytrack->bit_rate = track->strf.wBitsPerSample;
            mytrack->sampling_rate = track->strf.nSamplesPerSec;
            mytrack->channel_count = track->strf.nChannels;

            //mytrack->frame_rate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // But do not trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_vids)
    {
        // Video track
        retcode = init_bitstream_map(&video->tracks_video[video->tracks_video_count], index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = video->tracks_video[video->tracks_video_count];
            video->tracks_video_count++;

            mytrack->stream_level = stream_level_ES;
            mytrack->stream_type  = stream_VIDEO;

            if (track->strh.fccHandler == fcc_xvid ||
                track->strh.fccHandler == fcc_XVID ||
                track->strh.fccHandler == fcc_FMP4 ||
                track->strh.fccHandler == fcc_DIVX ||
                track->strh.fccHandler == fcc_DX50)
                mytrack->stream_codec = CODEC_MPEG4;
            else
                mytrack->stream_codec = CODEC_UNKNOWN;

            mytrack->sample_alignment = true;

            mytrack->width = track->strf.biWidth;
            mytrack->height = track->strf.biHeight;
            mytrack->color_depth = track->strf.biBitCount;

            mytrack->frame_rate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // But do not trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_txts)
    {
        // Subtitles track
        retcode = init_bitstream_map(&video->tracks_subtitles[video->tracks_subtitles_count], index_entry_count);
        video->tracks_subtitles_count++;

        if (retcode == SUCCESS)
        {
            mytrack = video->tracks_video[video->tracks_video_count];

            mytrack->stream_level = stream_level_ES;
            mytrack->stream_type  = stream_TEXT;
            mytrack->stream_codec = CODEC_SRT;

            mytrack->sample_alignment = true;

            mytrack->encoding = 0;
            mytrack->language_code = NULL;
            mytrack->subtitles_name = NULL;
        }
    }
    else
    {
        TRACE_WARNING(AVI, "Unknown track type");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

static int avi_indexer(Bitstream_t *bitstr, VideoFile_t *video, avi_t *avi)
{
    TRACE_INFO(AVI, GREEN "avi_indexer()\n" RESET);
    int retcode = SUCCESS;
    int i = 0, j = 0;

    for (i = 0; i < avi->tracks_count; i++)
    {
        if (avi->tracks[i]->track_indexed == 1)
        {
            TRACE_1(AVI, "> track already indexed\n");
        }
        else
        {
            for (j = 0; j < avi->tracks[i]->superindex_count; j++)
            {
                // IX offset
                bitstream_goto_offset(bitstr, avi->tracks[i]->superindex_entries[j].offset);

                // IX header
                AviChunk_t ix_chunk;
                parse_chunk_header(bitstr, &ix_chunk);
                print_chunk_header(&ix_chunk);

                // IX content
                parse_indx(bitstr, &ix_chunk, avi->tracks[i]);
            }

            // Convert index into a bitstream map
            retcode = avi_indexer_initmap(video, avi->tracks[i], avi->tracks[i]->index_count);

            if (retcode == SUCCESS)
            {
                // Set sample into bitstream map
                int k = 0;
                int tid = 0; // only support 1 audio and 1 video track for now

                if (avi->tracks[i]->strh.fccType == fcc_auds)
                {
                    for (k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = video->tracks_audio[tid]->sample_count;
                        video->tracks_audio[tid]->sample_count++;

                        video->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                        video->tracks_audio[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        video->tracks_audio[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        video->tracks_audio[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        video->tracks_audio[tid]->sample_pts[sid] = -1;
                    }
                }
                else if (avi->tracks[i]->strh.fccType == fcc_vids)
                {
                    for (k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = video->tracks_video[tid]->sample_count;
                        video->tracks_video[tid]->sample_count++;

                        if (avi->tracks[i]->index_entries[i].flags == AVIIF_KEYFRAME)
                            video->tracks_video[tid]->sample_type[sid] = sample_VIDEO_IDR;
                        else
                            video->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                        video->tracks_video[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        video->tracks_video[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        video->tracks_video[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        video->tracks_video[tid]->sample_pts[sid] = -1;
                    }
                }
                else if (avi->tracks[i]->strh.fccType == fcc_txts)
                {
                    //
                }
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

void avi_clean(avi_t *avi)
{
    if (avi)
    {
        int i = 0;
        for (i = 0; i < avi->tracks_count; i++)
        {
            if (avi->tracks[i])
            {
                free(avi->tracks[i]->index_entries);
                free(avi->tracks[i]);
            }
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse an avi file.
 * \param *video A pointer to a VideoFile_t structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * This parser is compatible with the 'OpenDML AVI File Format Extensions'.
 */
int avi_fileParse(VideoFile_t *video)
{
    TRACE_INFO(AVI, GREEN "avi_fileParse()\n" RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(video, NULL);

    if (bitstr != NULL)
    {
        // Init an AVI structure
        avi_t avi;
        avi.tracks_count = 0;
        avi.movi_offset = 0;

        // A convinient way to stop the parser
        bool superrun = true;

        // Loop on 1st level list
        while (superrun == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < video->file_size)
        {
            // Read RIFF header
            AviList_t RIFF_header;
            retcode = parse_list_header(bitstr, &RIFF_header);
            print_list_header(&RIFF_header);

            // Bytes left in the RIFF list
            int byte_left = RIFF_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);

            // Bytes left in the lists / chunks inside the RIFF list
            int jump = 0;

            if (RIFF_header.dwList == fcc_RIFF &&
                RIFF_header.dwFourCC == fcc_AVI_)
            {
                // Loop on 2nd level list/chunk
                while (retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < RIFF_header.dwSize)
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        AviList_t list_header;
                        retcode = parse_list_header(bitstr, &list_header);

                        switch (list_header.dwFourCC)
                        {
                        case fcc_hdrl:
                            retcode = parse_hdrl(bitstr, &list_header, &avi);
                            break;
                        case fcc_INFO:
                            retcode = parse_INFO(bitstr, &list_header, &avi);
                            break;
                        case fcc_movi:
                            retcode = parse_movi(bitstr, &list_header, &avi);
                            break;
                        default:
                            TRACE_WARNING(AVI, GREEN "Unknown liist type\n" RESET);
                            print_list_header(&list_header);
                            retcode = skip_list(bitstr, &RIFF_header, &list_header);
                            break;
                        }

                        // Byte left in the list we just left?
                        jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                    }
                    else
                    {
                        AviChunk_t chunk_header;
                        retcode = parse_chunk_header(bitstr, &chunk_header);

                        switch (chunk_header.dwFourCC)
                        {
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, video, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &RIFF_header, &chunk_header);
                            break;
                        default:
                            TRACE_WARNING(AVI, GREEN "Unknown chuunk type\n" RESET);
                            print_chunk_header(&chunk_header);
                            retcode = skip_chunk(bitstr, &RIFF_header, &chunk_header);
                            break;
                        }

                        // Byte left in the chunk we just left?
                        jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                    }

                    // Jump to the next list / chunk
                    if (jump > 0)
                    {
                        skip_bits(bitstr, jump*8);
                    }

                    // Byte left in the RIFF list?
                    byte_left = RIFF_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                }
            }
            else if (RIFF_header.dwList == fcc_RIFF &&
                     RIFF_header.dwFourCC == fcc_AVIX)
            {
                // Loop on 2nd level list/chunk
                while (retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < RIFF_header.dwSize)
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        AviList_t list_header;
                        retcode = parse_list_header(bitstr, &list_header);

                        switch (list_header.dwFourCC)
                        {
                        case fcc_movi:
                            retcode = parse_movi(bitstr, &list_header, &avi);
                            break;
                        default:
                            TRACE_WARNING(AVI, GREEN "Unknown liist type\n" RESET);
                            print_list_header(&list_header);
                            retcode = skip_list(bitstr, &RIFF_header, &list_header);
                            break;
                        }

                        // Byte left in the list we just left?
                        jump = list_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                    }
                    else
                    {
                        AviChunk_t chunk_header;
                        retcode = parse_chunk_header(bitstr, &chunk_header);

                        switch (chunk_header.dwFourCC)
                        {
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, video, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &RIFF_header, &chunk_header);
                            break;
                        default:
                            TRACE_WARNING(AVI, GREEN "Unknown chuunk type\n" RESET);
                            print_chunk_header(&chunk_header);
                            retcode = skip_chunk(bitstr, &RIFF_header, &chunk_header);
                            break;
                        }

                        // Byte left in the chunk we just left?
                        jump = chunk_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                    }

                    // Jump to the next list / chunk
                    if (jump > 0)
                    {
                        skip_bits(bitstr, jump*8);
                    }

                    // Byte left in the RIFF list?
                    byte_left = RIFF_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                }
            }
            else
            {
                TRACE_ERROR(AVI, "Unable to find RIFF AVI or AVIX headers!\n");
                retcode = FAILURE;
            }

            // Check if we have our super index, or if the tracks have been indexed
            int i = 0;
            int track_indexed = 0, track_superindexed = 0;

            for (i = 0; i < avi.tracks_count; i++)
            {
                if (avi.tracks[i]->track_indexed == 1)
                    track_indexed++;

                if (avi.tracks[i]->superindex_count > 0)
                    track_superindexed++;
            }

            if ((track_indexed + track_superindexed) == avi.tracks_count)
            {
                superrun = false;
            }
        }

        // Go for the indexation
        retcode = avi_indexer(bitstr, video, &avi),

        // Free avi_t structure content
        avi_clean(&avi);

        // Free bitstream
        free_bitstream(&bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
