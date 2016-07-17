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

// minivideo headers
#include "avi.h"
#include "avi_struct.h"
#include "../riff/riff.h"
#include "../wave/wave_struct.h"
#include "../../utils.h"
#include "../../fourcc.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ************************************************************************** */

static int avi_indexer_initmap(MediaFile_t *media, AviTrack_t *track, int index_entry_count);

/* ************************************************************************** */

static int parse_JUNK(Bitstream_t *bitstr, RiffList_t *list_header_parent, RiffChunk_t *chunk_header_child)
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
            TRACE_ERROR(AVI, BLD_GREEN "parse_JUNK()" CLR_RESET " >> Unable to skip %i bytes\n", chunk_header_child->dwSize);
            retcode = FAILURE;
        }
        else
        {
            TRACE_1(AVI, BLD_GREEN "parse_JUNK()\n" CLR_RESET);
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

static int parse_string(Bitstream_t *bitstr, RiffChunk_t *chunk_header)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_string()\n" CLR_RESET);
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
            unsigned int i = 0;
            for (i = 0; i < chunk_header->dwSize; i++)
            {
                string[i] = read_bits(bitstr, 8);
            }

#if ENABLE_DEBUG
            // Chunk Header
            print_chunk_header(chunk_header);
#if TRACE_1
            // Chunk content
            TRACE_1(AVI, "> '");
            for (i = 0; i < chunk_header->dwSize; i++)
            {
                printf("%c", string[i]);
            }
            printf("'\n");
#endif
#endif // ENABLE_DEBUG
            free(string);
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_avih(Bitstream_t *bitstr, RiffChunk_t *avih_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_avih()\n" CLR_RESET);
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

static int parse_dmlh(Bitstream_t *bitstr, RiffChunk_t *dmlh_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_dmlh()\n" CLR_RESET);
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

#if ENABLE_DEBUG
        // Print dmlh content
        TRACE_1(AVI, "> dwTotalFrames\t: %u\n", avi->avih.dwTotalFrames);
#endif
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strh(Bitstream_t *bitstr, RiffChunk_t *strh_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_strh()\n" CLR_RESET);
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

#if ENABLE_DEBUG
        // Print chunk header
        print_chunk_header(strh_header);

        // Print strh content
        char fcc[5];
        TRACE_1(AVI, "> fccType\t\t: 0x%08X ('%s')\n", track->strh.fccType, getFccString_le(track->strh.fccType, fcc));
        TRACE_1(AVI, "> fccHandler\t: 0x%08X ('%s')\n", track->strh.fccHandler, getFccString_le(track->strh.fccHandler, fcc));
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
#endif // ENABLE_DEBUG
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strf(Bitstream_t *bitstr, RiffChunk_t *strf_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_strf()\n" CLR_RESET);
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

#if ENABLE_DEBUG
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
#endif // ENABLE_DEBUG
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
            if (track->strf.wFormatTag == 0)
            {
                TRACE_2(AVI, "> EXT > No wave format TAG\n");
                track->strh.fccHandler = CODEC_UNKNOWN;
            }
            else
            {
                int byte_left = strf_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

                if (track->strf.wFormatTag == WAVE_FORMAT_MP1)
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
                else if (track->strf.wFormatTag == WAVE_FORMAT_MP3)
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
                else if (track->strf.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                {
                    TRACE_1(AVI, "> EXT > MPEG1WAVEFORMAT\n");
                    track->strh.fccHandler = CODEC_LPCM;

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
                else if (track->strf.wFormatTag == WAVE_FORMAT_AAC)
                {
                    TRACE_1(AVI, "> EXT > AAC\n");
                    track->strh.fccHandler = CODEC_AAC;
                }
                else if (track->strf.wFormatTag == WAVE_FORMAT_AC3)
                {
                    TRACE_1(AVI, "> EXT > AC3\n");
                    track->strh.fccHandler = CODEC_AC3;
                }
                else if (track->strf.wFormatTag == WAVE_FORMAT_DTS)
                {
                    TRACE_1(AVI, "> EXT > DTS\n");
                    track->strh.fccHandler = CODEC_DTS;
                }
                else if (track->strf.wFormatTag == WAVE_FORMAT_WMAS ||
                         track->strf.wFormatTag == WAVE_FORMAT_WMA1 ||
                         track->strf.wFormatTag == WAVE_FORMAT_WMA2 ||
                         track->strf.wFormatTag == WAVE_FORMAT_WMAP ||
                         track->strf.wFormatTag == WAVE_FORMAT_WMAL)
                {
                    TRACE_1(AVI, "> EXT > WMA\n");
                    track->strh.fccHandler = CODEC_WMA;
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
static int parse_idx1(Bitstream_t *bitstr, MediaFile_t *media, RiffChunk_t *idx1_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_idx1()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

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
        unsigned int i = 0;
        for (i = 0; i < avi->tracks_count; i++)
        {
            if (avi->tracks[i]->track_indexed == false &&
                avi->tracks[i]->superindex_count == 0)
            {
                track_left++;
                retcode = avi_indexer_initmap(media, avi->tracks[i], index_entry_count);
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
                    TRACE_3(AVI, BLD_BLUE "> AUDIO\n" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_audio[tid]->sample_count;
                    media->tracks_audio[tid]->sample_count++;

                    media->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                    media->tracks_audio[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_audio[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_audio[tid]->sample_dts[sid] = -1;
                    media->tracks_audio[tid]->sample_pts[sid] = -1;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x6463) // dc: video
                {
                    TRACE_3(AVI, BLD_BLUE "> VIDEO\n" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_video[tid]->sample_count;
                    media->tracks_video[tid]->sample_count++;

                    if (dwFlags == AVIIF_KEYFRAME)
                        media->tracks_video[tid]->sample_type[sid] = sample_VIDEO_SYNC;
                    else
                        media->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                    media->tracks_video[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_video[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_video[tid]->sample_dts[sid] = -1;
                    media->tracks_video[tid]->sample_pts[sid] = -1;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x7478) // tx: subtitles
                {
                    TRACE_3(AVI, BLD_BLUE "> TEXT\n" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_subt[tid]->sample_count;

                    media->tracks_subt[tid]->sample_type[sid] = sample_TEXT_FILE;
                    media->tracks_subt[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_subt[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_subt[tid]->sample_dts[sid] = -1;
                    media->tracks_subt[tid]->sample_pts[sid] = -1;
                    media->tracks_subt[tid]->sample_count++;
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
                    TRACE_WARNING(AVI, "Unknown chunk type in idx1 (0x%08X) ('%s')!\n", dwChunkId, getFccString_le(dwChunkId, fcc));
                }

                // Print
                TRACE_3(AVI, "> dwChunkId\t: 0x%08X ('%s')\n", dwChunkId, getFccString_le(dwChunkId, fcc));
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
static int parse_indx(Bitstream_t *bitstr, RiffChunk_t *indx_header, AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_indx()\n" CLR_RESET);
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

        char fcc[5];
        TRACE_1(AVI, "> wLongsPerEntry\t: %u\n", wLongsPerEntry);
        TRACE_1(AVI, "> bIndexSubType\t: %u\n", bIndexSubType);
        TRACE_1(AVI, "> bIndexType\t: %u\n", bIndexType);
        TRACE_1(AVI, "> nEntriesInUse\t: %u\n", nEntriesInUse);
        TRACE_1(AVI, "> dwChunkId\t: 0x%08X ('%s')!\n", dwChunkId, getFccString_le(dwChunkId, fcc));

        // Index specifics code
        ////////////////////////////////////////////////////////////////////////

        if (bIndexType == AVI_INDEX_OF_INDEXES)
        {
            TRACE_INFO(AVI, BLD_GREEN "> AVI Super Index Chunk\n" CLR_RESET);

            uint32_t dwReserved[3];
            dwReserved[0] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[1] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[2] = endian_flip_32(read_bits(bitstr, 32));

            // Parse super index entries
            unsigned int i = 0;
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
            TRACE_INFO(AVI, BLD_GREEN "> AVI Standard Index Chunk\n" CLR_RESET);

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

            unsigned int i = 0;
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
static int parse_strl(Bitstream_t *bitstr, RiffList_t *strl_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_strl()\n" CLR_RESET);
    int retcode = SUCCESS;
    int track_id = 0;
    char fcc[5];

    if (strl_header != NULL &&
        strl_header->dwFourCC == fcc_strl)
    {
        // Print list header
        print_list_header(strl_header);

        // Bytes left in the strl list
        int byte_left = strl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

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
        while (avi->run == true &&
               retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < strl_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type (0x%08X) ('%s') @ %i\n", list_header.dwFourCC, getFccString_le(list_header.dwFourCC, fcc));
                    retcode = skip_list(bitstr, strl_header, &list_header);
                    break;
                }

                jumpy_riff(bitstr, strl_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
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
                    TRACE_INFO(AVI, BLD_GREEN "parse_strn()\n" CLR_RESET);
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

                jumpy_riff(bitstr, strl_header, chunk_header.offset_end);
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
static int parse_odml(Bitstream_t *bitstr, RiffList_t *odml_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_odml()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (odml_header != NULL &&
        odml_header->dwFourCC == fcc_odml)
    {
        // Print list header
        print_list_header(odml_header);

        // Bytes left in the odml list
        int byte_left = odml_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Loop on "odml" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < odml_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type (0x%08X) ('%s') @ %i\n", list_header.dwFourCC, getFccString_le(list_header.dwFourCC, fcc));
                    retcode = skip_list(bitstr, odml_header, &list_header);
                    break;
                }

                jumpy_riff(bitstr, odml_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_dmlh:
                    retcode = parse_dmlh(bitstr, &chunk_header, avi);
                    break;
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type (0x%08X) ('%s') @ %i\n", chunk_header.dwFourCC, getFccString_le(chunk_header.dwFourCC, fcc));
                    retcode = skip_chunk(bitstr, odml_header, &chunk_header);
                    break;
                }

                jumpy_riff(bitstr, odml_header, chunk_header.offset_end);
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
static int parse_movi(Bitstream_t *bitstr, RiffList_t *movi_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_movi()\n" CLR_RESET);
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
        while (avi->run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < movi_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
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

                jumpy_riff(bitstr, movi_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown chunk type\n");
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, movi_header, &chunk_header);
                    break;
                }

                jumpy_riff(bitstr, movi_header, chunk_header.offset_end);
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

static int parse_INFO(Bitstream_t *bitstr, RiffList_t *INFO_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_INFO()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (INFO_header != NULL &&
        INFO_header->dwFourCC == fcc_INFO)
    {
        // Print INFO list header
        print_list_header(INFO_header);

        // Bytes left in the INFO list
        int byte_left = INFO_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Loop on "INFO" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < INFO_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    TRACE_WARNING(AVI, "Unknown list type ('%s')\n", getFccString_le(list_header.dwFourCC, fcc));
                    print_list_header(&list_header);
                    retcode = skip_list(bitstr, INFO_header, &list_header);
                    break;
                }

                jumpy_riff(bitstr, INFO_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
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
                    TRACE_WARNING(AVI, "Unknown chunk type ('%s')\n", getFccString_le(chunk_header.dwFourCC, fcc));
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, INFO_header, &chunk_header);
                    break;
                }

                jumpy_riff(bitstr, INFO_header, chunk_header.offset_end);
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

static int parse_hdrl(Bitstream_t *bitstr, RiffList_t *hdrl_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_hdrl()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (hdrl_header != NULL &&
        hdrl_header->dwFourCC == fcc_hdrl)
    {
        // Print list header
        print_list_header(hdrl_header);

        // Bytes left in the hdrl list
        int byte_left = hdrl_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

        // Loop on "hdrl" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               byte_left > 12 &&
               bitstream_get_absolute_byte_offset(bitstr) < hdrl_header->offset_end)
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
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
                    TRACE_WARNING(AVI, "Unknown list type ('%s')\n", getFccString_le(list_header.dwFourCC, fcc));
                    print_list_header(&list_header);
                    retcode = skip_list(bitstr, hdrl_header, &list_header);
                    break;
                }

                jumpy_riff(bitstr, hdrl_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
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
                    TRACE_WARNING(AVI, "Unknown chunk type ('%s')\n", getFccString_le(chunk_header.dwFourCC, fcc));
                    print_chunk_header(&chunk_header);
                    retcode = skip_chunk(bitstr, hdrl_header, &chunk_header);
                    break;
                }

                jumpy_riff(bitstr, hdrl_header, chunk_header.offset_end);
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

static int avi_indexer_initmap(MediaFile_t *media, AviTrack_t *track, int index_entry_count)
{
    // Init a bitstreamMap_t for each avi track
    int retcode = SUCCESS;
    BitstreamMap_t *mytrack = NULL;

    if (track->strh.fccType == fcc_auds)
    {
        // Audio track
        retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_audio[media->tracks_audio_count];
            media->tracks_audio_count++;

            mytrack->stream_type = stream_AUDIO;
            mytrack->stream_fcc  = track->strh.fccHandler;

            // We cannot rely on the fccHandler from the container,
            // but we should have extracted the correct codec infos from strh
            mytrack->stream_codec = track->strh.fccHandler;

            mytrack->sample_alignment = true;

            mytrack->bitrate = track->strf.wBitsPerSample;
            mytrack->sampling_rate = track->strf.nSamplesPerSec;
            mytrack->channel_count = track->strf.nChannels;

            //mytrack->frame_rate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // But do not trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_vids)
    {
        // Video track
        retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_video[media->tracks_video_count];
            media->tracks_video_count++;

            mytrack->stream_type = stream_VIDEO;
            mytrack->stream_fcc  = track->strh.fccHandler;

            if (track->strh.fccHandler == fcc_xvid ||
                track->strh.fccHandler == fcc_XVID ||
                track->strh.fccHandler == fcc_FMP4 ||
                track->strh.fccHandler == fcc_DIVX ||
                track->strh.fccHandler == fcc_DX50)
                mytrack->stream_codec = CODEC_MPEG4_ASP;
            else
                mytrack->stream_codec = CODEC_UNKNOWN;

            mytrack->sample_alignment = true;

            mytrack->width = track->strf.biWidth;
            mytrack->height = track->strf.biHeight;
            mytrack->color_depth = track->strf.biBitCount;

            mytrack->framerate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // But do not trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_txts)
    {
        // Subtitles track
        retcode = init_bitstream_map(&media->tracks_subt[media->tracks_subtitles_count], index_entry_count);
        media->tracks_subtitles_count++;

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_video[media->tracks_video_count];

            mytrack->stream_type  = stream_TEXT;
            mytrack->stream_fcc   = track->strh.fccHandler;
            mytrack->stream_codec = CODEC_SRT;

            mytrack->sample_alignment = true;

            mytrack->subtitles_encoding = 0;
            //strncpy(mytrack->language_code, "UND", 3);
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

static int avi_indexer(Bitstream_t *bitstr, MediaFile_t *media, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "avi_indexer()\n" CLR_RESET);
    int retcode = SUCCESS;
    unsigned int i = 0, j = 0;

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
                RiffChunk_t ix_chunk;
                parse_chunk_header(bitstr, &ix_chunk);
                print_chunk_header(&ix_chunk);

                // IX content
                parse_indx(bitstr, &ix_chunk, avi->tracks[i]);
            }

            // Convert index into a bitstream map
            retcode = avi_indexer_initmap(media, avi->tracks[i], avi->tracks[i]->index_count);

            if (retcode == SUCCESS)
            {
                // Set sample into bitstream map
                unsigned int k = 0;
                unsigned int tid = 0; // only support 1 audio and 1 video track for now

                if (avi->tracks[i]->strh.fccType == fcc_auds)
                {
                    for (k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = media->tracks_audio[tid]->sample_count;
                        media->tracks_audio[tid]->sample_count++;

                        media->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                        media->tracks_audio[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        media->tracks_audio[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        media->tracks_audio[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        media->tracks_audio[tid]->sample_pts[sid] = -1;
                    }
                }
                else if (avi->tracks[i]->strh.fccType == fcc_vids)
                {
                    for (k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = media->tracks_video[tid]->sample_count;
                        media->tracks_video[tid]->sample_count++;

                        if (avi->tracks[i]->index_entries[i].flags == AVIIF_KEYFRAME)
                            media->tracks_video[tid]->sample_type[sid] = sample_VIDEO_SYNC;
                        else
                            media->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                        media->tracks_video[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        media->tracks_video[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        media->tracks_video[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        media->tracks_video[tid]->sample_pts[sid] = -1;
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
        unsigned int i = 0;
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

int avi_fileParse(MediaFile_t *media)
{
    TRACE_INFO(AVI, BLD_GREEN "avi_fileParse()\n" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an AVI structure
        avi_t avi;
        avi.tracks_count = 0;
        avi.movi_offset = 0;

        // A convenient way to stop the parser
        avi.run = true;

        // Loop on 1st level list
        while (avi.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < media->file_size)
        {
            // Read RIFF header
            RiffList_t RIFF_header;
            retcode = parse_list_header(bitstr, &RIFF_header);
            print_list_header(&RIFF_header);

            if (RIFF_header.dwList == fcc_RIFF &&
                RIFF_header.dwFourCC == fcc_AVI_)
            {
                // Loop on 2nd level list/chunk
                while (avi.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < RIFF_header.dwSize)
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        RiffList_t list_header;
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
                            TRACE_WARNING(AVI, BLD_GREEN "Unknown liist type (%s)\n" CLR_RESET, getFccString_le(list_header.dwFourCC, fcc));
                            print_list_header(&list_header);
                            retcode = skip_list(bitstr, &RIFF_header, &list_header);
                            break;
                        }

                        jumpy_riff(bitstr, &RIFF_header, list_header.offset_end);
                    }
                    else
                    {
                        RiffChunk_t chunk_header;
                        retcode = parse_chunk_header(bitstr, &chunk_header);

                        switch (chunk_header.dwFourCC)
                        {
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, media, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &RIFF_header, &chunk_header);
                            break;
                        default:
                            TRACE_WARNING(AVI, BLD_GREEN "Unknown chuunk type (%s)\n" CLR_RESET, getFccString_le(chunk_header.dwFourCC, fcc));
                            print_chunk_header(&chunk_header);
                            retcode = skip_chunk(bitstr, &RIFF_header, &chunk_header);
                            break;
                        }

                        jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }
            else if (RIFF_header.dwList == fcc_RIFF &&
                     RIFF_header.dwFourCC == fcc_AVIX)
            {
                // Loop on 2nd level list/chunk
                while (avi.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < RIFF_header.dwSize)
                {
                    if (next_bits(bitstr, 32) == fcc_LIST)
                    {
                        RiffList_t list_header;
                        retcode = parse_list_header(bitstr, &list_header);

                        switch (list_header.dwFourCC)
                        {
                        case fcc_movi:
                            retcode = parse_movi(bitstr, &list_header, &avi);
                            break;
                        default:
                            TRACE_WARNING(AVI, BLD_GREEN "Unknown liist type (%s)\n" CLR_RESET, getFccString_le(list_header.dwFourCC, fcc));
                            print_list_header(&list_header);
                            retcode = skip_list(bitstr, &RIFF_header, &list_header);
                            break;
                        }

                        jumpy_riff(bitstr, &RIFF_header, list_header.offset_end);
                    }
                    else
                    {
                        RiffChunk_t chunk_header;
                        retcode = parse_chunk_header(bitstr, &chunk_header);

                        switch (chunk_header.dwFourCC)
                        {
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, media, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &RIFF_header, &chunk_header);
                            break;
                        default:
                            TRACE_WARNING(AVI, BLD_GREEN "Unknown chuunk type (%s)\n" CLR_RESET, getFccString_le(chunk_header.dwFourCC, fcc));
                            print_chunk_header(&chunk_header);
                            retcode = skip_chunk(bitstr, &RIFF_header, &chunk_header);
                            break;
                        }

                        jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }
            else
            {
                TRACE_ERROR(AVI, "Unable to find RIFF AVI or AVIX headers!\n");
                retcode = FAILURE;
            }

            // Check if we have our super index, or if the tracks have been indexed
            unsigned int i = 0;
            int track_indexed = 0, track_superindexed = 0;

            for (i = 0; i < avi.tracks_count; i++)
            {
                if (avi.tracks[i]->track_indexed == 1)
                    track_indexed++;

                if (avi.tracks[i]->superindex_count > 0)
                    track_superindexed++;
            }

            if ((unsigned int)(track_indexed + track_superindexed) == avi.tracks_count)
            {
                avi.run = false;
            }
        }

        // Go for the indexation
        retcode = avi_indexer(bitstr, media, &avi),

        // Free avi_t structure content
        avi_clean(&avi);

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
