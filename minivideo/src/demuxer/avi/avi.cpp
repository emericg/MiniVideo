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
 * \file      avi.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "avi.h"
#include "avi_convert.h"
#include "avi_struct.h"
#include "../riff/riff.h"
#include "../xml_mapper.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_uuid.h"
#include "../../minivideo_twocc.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

static int parse_string(Bitstream_t *bitstr, RiffChunk_t *chunk_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_string()" CLR_RESET);
    int retcode = SUCCESS;

    if (chunk_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid chunk_header structure!");
        retcode = FAILURE;
    }
    else
    {
        char *string = (char *)malloc(chunk_header->dwSize);

        if (string == NULL)
        {
            TRACE_ERROR(AVI, "Unable to allocate string!");
            retcode = FAILURE;
        }
        else
        {
            for (uint32_t i = 0; i < chunk_header->dwSize; i++)
            {
                string[i] = read_bits(bitstr, 8);
            }

            // Check for additional 0x00 bytes at the end of the string. This is
            // a hack based on the fact that AVI string chunk often return erroneous size.
            // We read one byte until its not equal to 0x00.
            while (next_bits(bitstr, 8) == 0)
            {
                //TRACE_WARNING(AVI, "SKIPPING ONE BYTE!");
                skip_bits(bitstr, 8);
                chunk_header->dwSize++;
                chunk_header->offset_end++;
            }

#if ENABLE_DEBUG
            print_chunk_header(chunk_header);
/*
            TRACE_1(AVI, "> '");
            for (uint32_t i = 0; i < chunk_header->dwSize; i++)
            {
                printf("%c", string[i]);
            }
            printf("'\n");
*/
#endif // ENABLE_DEBUG

            // xmlMapper
            if (avi->xml)
            {
                write_chunk_header(chunk_header, avi->xml);
                if (chunk_header->dwFourCC == fcc_strn) fprintf(avi->xml, "  <title>Stream Name</title>\n");
                if (chunk_header->dwFourCC == fcc_ISFT) fprintf(avi->xml, "  <title>Encoder Name</title>\n");
                fprintf(avi->xml, "  <string>%s</string>\n", string);
                fprintf(avi->xml, "  </a>\n");
            }

            free(string);
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_avih(Bitstream_t *bitstr, RiffChunk_t *avih_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_avih()" CLR_RESET);
    int retcode = SUCCESS;

    if (avih_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid avih_header structure!");
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

#if ENABLE_DEBUG
        print_chunk_header(avih_header);
        TRACE_1(AVI, "> dwMicroSecPerFrame  : %u", avi->avih.dwMicroSecPerFrame);
        TRACE_1(AVI, "> dwMaxBytesPerSec    : %u", avi->avih.dwMaxBytesPerSec);
        TRACE_1(AVI, "> dwPaddingGranularity: %u", avi->avih.dwPaddingGranularity);
        TRACE_1(AVI, "> dwFlags             : %u", avi->avih.dwFlags);
        TRACE_1(AVI, "> dwTotalFrames       : %u", avi->avih.dwTotalFrames);
        TRACE_1(AVI, "> dwInitialFrames     : %u", avi->avih.dwInitialFrames);
        TRACE_1(AVI, "> dwStreams           : %u", avi->avih.dwStreams);
        TRACE_1(AVI, "> dwSuggestedBufferSize : %u", avi->avih.dwSuggestedBufferSize);
        TRACE_1(AVI, "> dwWidth             : %u", avi->avih.dwWidth);
        TRACE_1(AVI, "> dwHeight            : %u", avi->avih.dwHeight);
#endif
        // xmlMapper
        if (avi->xml)
        {
            write_chunk_header(avih_header, avi->xml, "AVI header");
            fprintf(avi->xml, "  <dwMicroSecPerFrame>%u</dwMicroSecPerFrame>\n", avi->avih.dwMicroSecPerFrame);
            fprintf(avi->xml, "  <dwMaxBytesPerSec>%u</dwMaxBytesPerSec>\n", avi->avih.dwMaxBytesPerSec);
            fprintf(avi->xml, "  <dwPaddingGranularity>%u</dwPaddingGranularity>\n", avi->avih.dwPaddingGranularity);
            fprintf(avi->xml, "  <dwFlags>%u</dwFlags>\n", avi->avih.dwFlags);
            fprintf(avi->xml, "  <dwTotalFrames>%u</dwTotalFrames>\n", avi->avih.dwTotalFrames);
            fprintf(avi->xml, "  <dwInitialFrames>%u</dwInitialFrames>\n", avi->avih.dwInitialFrames);
            fprintf(avi->xml, "  <dwStreams>%u</dwStreams>\n", avi->avih.dwStreams);
            fprintf(avi->xml, "  <dwSuggestedBufferSize>%u</dwSuggestedBufferSize>\n", avi->avih.dwSuggestedBufferSize);
            fprintf(avi->xml, "  <dwWidth>%u</dwWidth>\n", avi->avih.dwWidth);
            fprintf(avi->xml, "  <dwHeight>%u</dwHeight>\n", avi->avih.dwHeight);
            fprintf(avi->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_dmlh(Bitstream_t *bitstr, RiffChunk_t *dmlh_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_dmlh()" CLR_RESET);
    int retcode = SUCCESS;

    if (dmlh_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid dmlh_header structure!");
        retcode = FAILURE;
    }
    else
    {
        // Parse chunk content
        // Update the (unreliable) dwTotalFrames from the avi header
        avi->avih.dwTotalFrames = endian_flip_32(read_bits(bitstr, 32));

#if ENABLE_DEBUG
        print_chunk_header(dmlh_header);
        TRACE_1(AVI, "> dwTotalFrames: %u", avi->avih.dwTotalFrames);
#endif
        // xmlMapper
        if (avi->xml)
        {
            write_chunk_header(dmlh_header, avi->xml, "Extended AVI Header");
            fprintf(avi->xml, "  <dwTotalFrames>%u</dwTotalFrames>\n", avi->avih.dwTotalFrames);
            fprintf(avi->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_vprp(Bitstream_t *bitstr, RiffChunk_t *vprp_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_vprp()" CLR_RESET);
    int retcode = SUCCESS;

    if (vprp_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid parse_vprp structure!");
        retcode = FAILURE;
    }
    else
    {
        // Parse chunk content
        uint32_t _VideoFormatToken = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _VideoStandard = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwVerticalRefreshRate = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwHTotalInT = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwVTotalInLines = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwFrameAspectRatio = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwFrameWidthInPixels = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwFrameHeightInLines = endian_flip_32(read_bits(bitstr, 32));
        uint32_t _dwFieldPerFrame = endian_flip_32(read_bits(bitstr, 32));

#if ENABLE_DEBUG
        print_chunk_header(vprp_header);
        TRACE_1(AVI, "> _VideoFormatToken: %u", _VideoFormatToken);
        TRACE_1(AVI, "> _VideoStandard: %u", _VideoStandard);
        TRACE_1(AVI, "> _dwVerticalRefreshRate: %u", _dwVerticalRefreshRate);
        TRACE_1(AVI, "> _dwHTotalInT: %u", _dwHTotalInT);
        TRACE_1(AVI, "> _dwVTotalInLines: %u", _dwVTotalInLines);
        TRACE_1(AVI, "> _dwFrameAspectRatio: %u", _dwFrameAspectRatio);
        TRACE_1(AVI, "> _dwFrameWidthInPixels: %u", _dwFrameWidthInPixels);
        TRACE_1(AVI, "> _dwFrameHeightInLines: %u", _dwFrameHeightInLines);
        TRACE_1(AVI, "> _dwFieldPerFrame: %u", _dwFieldPerFrame);
#endif
        // xmlMapper
        if (avi->xml)
        {
            write_chunk_header(vprp_header, avi->xml, "Video Properties Header");
            fprintf(avi->xml, "  <_VideoFormatToken>%u</_VideoFormatToken>\n", _VideoFormatToken);
            fprintf(avi->xml, "  <_VideoStandard>%u</_VideoStandard>\n", _VideoStandard);
            fprintf(avi->xml, "  <_dwVerticalRefreshRate>%u</_dwVerticalRefreshRate>\n", _dwVerticalRefreshRate);
            fprintf(avi->xml, "  <_dwHTotalInT>%u</_dwHTotalInT>\n", _dwHTotalInT);
            fprintf(avi->xml, "  <_dwVTotalInLines>%u</_dwVTotalInLines>\n", _dwVTotalInLines);
            fprintf(avi->xml, "  <_dwFrameAspectRatio>%u</_dwFrameAspectRatio>\n", _dwFrameAspectRatio);
            fprintf(avi->xml, "  <_dwFrameWidthInPixels>%u</_dwFrameWidthInPixels>\n", _dwFrameWidthInPixels);
            fprintf(avi->xml, "  <_dwFrameHeightInLines>%u</_dwFrameHeightInLines>\n", _dwFrameHeightInLines);
            fprintf(avi->xml, "  <_dwFieldPerFrame>%u</_dwFieldPerFrame>\n", _dwFieldPerFrame);
            fprintf(avi->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strh(Bitstream_t *bitstr, RiffChunk_t *strh_header, avi_t *avi, AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_strh()" CLR_RESET);
    int retcode = SUCCESS;

    if (strh_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid strh_header structure!");
        retcode = FAILURE;
    }
    else
    {
        char fcc[5];

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
        print_chunk_header(strh_header);
        TRACE_1(AVI, "> fccType     : 0x%08X ('%s')", track->strh.fccType, getFccString_le(track->strh.fccType, fcc));
        TRACE_1(AVI, "> fccHandler  : 0x%08X ('%s')", track->strh.fccHandler, getFccString_le(track->strh.fccHandler, fcc));
        TRACE_1(AVI, "> dwFlags     : %u", track->strh.dwFlags);
        TRACE_1(AVI, "> wPriority   : %u", track->strh.wPriority);
        TRACE_1(AVI, "> wLanguage   : %u", track->strh.wLanguage);
        TRACE_1(AVI, "> dwInitialFrames: %u", track->strh.dwInitialFrames);
        TRACE_1(AVI, "> dwScale     : %u", track->strh.dwScale);
        TRACE_1(AVI, "> dwRate      : %u", track->strh.dwRate);
        TRACE_1(AVI, "> dwStart     : %u", track->strh.dwStart);
        TRACE_1(AVI, "> dwLength    : %u", track->strh.dwLength);
        TRACE_1(AVI, "> dwSuggestedBufferSize: %u", track->strh.dwSuggestedBufferSize);
        TRACE_1(AVI, "> dwQuality   : %u", track->strh.dwQuality);
        TRACE_1(AVI, "> dwSampleSize: %u", track->strh.dwSampleSize);
        TRACE_1(AVI, "> rcFrame_x   : %u", track->strh.rcFrame_x);
        TRACE_1(AVI, "> rcFrame_y   : %u", track->strh.rcFrame_y);
        TRACE_1(AVI, "> rcFrame_w   : %u", track->strh.rcFrame_w);
        TRACE_1(AVI, "> rcFrame_h   : %u", track->strh.rcFrame_h);
#endif
        // xmlMapper
        if (avi->xml)
        {
            write_chunk_header(strh_header, avi->xml, "Stream Header");
            fprintf(avi->xml, "  <fccType>%s</fccType>\n", getFccString_le(track->strh.fccType, fcc));
            fprintf(avi->xml, "  <fccHandler>%s</fccHandler>\n", getFccString_le(track->strh.fccHandler, fcc));
            fprintf(avi->xml, "  <dwFlags>%u</dwFlags>\n", track->strh.dwFlags);
            fprintf(avi->xml, "  <wPriority>%u</wPriority>\n", track->strh.wPriority);
            fprintf(avi->xml, "  <wLanguage>%u</wLanguage>\n", track->strh.wLanguage);
            fprintf(avi->xml, "  <dwInitialFrames>%u</dwInitialFrames>\n", track->strh.dwInitialFrames);
            fprintf(avi->xml, "  <dwScale>%u</dwScale>\n", track->strh.dwScale);
            fprintf(avi->xml, "  <dwRate>%u</dwRate>\n", track->strh.dwRate);
            fprintf(avi->xml, "  <dwStart>%u</dwStart>\n", track->strh.dwStart);
            fprintf(avi->xml, "  <dwLength>%u</dwLength>\n", track->strh.dwLength);
            fprintf(avi->xml, "  <dwSuggestedBufferSize>%u</dwSuggestedBufferSize>\n", track->strh.dwSuggestedBufferSize);
            fprintf(avi->xml, "  <dwQuality>%u</dwQuality>\n", track->strh.dwQuality);
            fprintf(avi->xml, "  <dwSampleSize>%u</dwSampleSize>\n", track->strh.dwSampleSize);
            fprintf(avi->xml, "  <rcFrame_x>%u</rcFrame_x>\n", track->strh.rcFrame_x);
            fprintf(avi->xml, "  <rcFrame_y>%u</rcFrame_y>\n", track->strh.rcFrame_y);
            fprintf(avi->xml, "  <rcFrame_w>%u</rcFrame_w>\n", track->strh.rcFrame_w);
            fprintf(avi->xml, "  <rcFrame_h>%u</rcFrame_h>\n", track->strh.rcFrame_h);
            fprintf(avi->xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_strf(Bitstream_t *bitstr, RiffChunk_t *strf_header, avi_t *avi, AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_strf()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (strf_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid strf_header structure!");
        retcode = FAILURE;
    }
    else
    {
        print_chunk_header(strf_header);
        write_chunk_header(strf_header, avi->xml, "Stream Format");

        // Parse chunk content
        if (track->strh.fccType == fcc_vids)
        {
            // Parse BITMAPINFOHEADER
            track->strf.biSize = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biWidth = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biHeight = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biPlanes = endian_flip_16(read_bits(bitstr, 16));
            track->strf.biBitCount = endian_flip_16(read_bits(bitstr, 16));
            track->strf.biCompression = read_bits(bitstr, 32);
            track->strf.biSizeImage = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biXPelsPerMeter = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biYPelsPerMeter = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biClrUsed = endian_flip_32(read_bits(bitstr, 32));
            track->strf.biClrImportant = endian_flip_32(read_bits(bitstr, 32));

#if ENABLE_DEBUG
            TRACE_1(AVI, "> biSize      : %u", track->strf.biSize);
            TRACE_1(AVI, "> biWidth     : %i", track->strf.biWidth);
            TRACE_1(AVI, "> biHeight    : %i", track->strf.biHeight);
            TRACE_1(AVI, "> biPlanes    : %u", track->strf.biPlanes);
            TRACE_1(AVI, "> biBitCount  : %u", track->strf.biBitCount);
            TRACE_1(AVI, "> biCompression   : %u ('%s')", track->strf.biCompression, getFccString_le(track->strf.biCompression, fcc));
            TRACE_1(AVI, "> biSizeImage     : %u", track->strf.biSizeImage);
            TRACE_1(AVI, "> biXPelsPerMeter : %i", track->strf.biXPelsPerMeter);
            TRACE_1(AVI, "> biYPelsPerMeter : %i", track->strf.biYPelsPerMeter);
            TRACE_1(AVI, "> biClrUsed       : %u", track->strf.biClrUsed);
            TRACE_1(AVI, "> biClrImportant  : %u", track->strf.biClrImportant);
#endif
            if (avi->xml)
            {
                fprintf(avi->xml, "  <biSize>%u</biSize>\n", track->strf.biSize);
                fprintf(avi->xml, "  <biWidth>%i</biWidth>\n", track->strf.biWidth);
                fprintf(avi->xml, "  <biHeight>%i</biHeight>\n", track->strf.biHeight);
                fprintf(avi->xml, "  <biPlanes>%u</biPlanes>\n", track->strf.biPlanes);
                fprintf(avi->xml, "  <biBitCount>%u</biBitCount>\n", track->strf.biBitCount);
                fprintf(avi->xml, "  <biCompression>%s</biCompression>\n", getFccString_le(track->strf.biCompression, fcc));
                fprintf(avi->xml, "  <biSizeImage>%u</biSizeImage>\n", track->strf.biSizeImage);
                fprintf(avi->xml, "  <biXPelsPerMeter>%i</biXPelsPerMeter>\n", track->strf.biXPelsPerMeter);
                fprintf(avi->xml, "  <biYPelsPerMeter>%i</biYPelsPerMeter>\n", track->strf.biYPelsPerMeter);
                fprintf(avi->xml, "  <biClrUsed>%u</biClrUsed>\n", track->strf.biClrUsed);
                fprintf(avi->xml, "  <biClrImportant>%u</biClrImportant>\n", track->strf.biClrImportant);
            }
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

#if ENABLE_DEBUG
            TRACE_1(AVI, "> wFormatTag  : %u", track->strf.wFormatTag);
            TRACE_1(AVI, "> nChannels   : %u", track->strf.nChannels);
            TRACE_1(AVI, "> nSamplesPerSec  : %u", track->strf.nSamplesPerSec);
            TRACE_1(AVI, "> nAvgBytesPerSec : %u", track->strf.nAvgBytesPerSec);
            TRACE_1(AVI, "> nBlockAlign     : %u", track->strf.nBlockAlign);
            TRACE_1(AVI, "> wBitsPerSample  : %u", track->strf.wBitsPerSample);
#endif
            if (avi->xml)
            {
                fprintf(avi->xml, "  <wFormatTag>0x%04X (%s)</wFormatTag>\n", track->strf.wFormatTag, getTccString(track->strf.wFormatTag));
                fprintf(avi->xml, "  <nChannels>%u</nChannels>\n", track->strf.nChannels);
                fprintf(avi->xml, "  <nSamplesPerSec>%u</nSamplesPerSec>\n", track->strf.nSamplesPerSec);
                fprintf(avi->xml, "  <nAvgBytesPerSec>%u</nAvgBytesPerSec>\n", track->strf.nAvgBytesPerSec);
                fprintf(avi->xml, "  <nBlockAlign>%u</nBlockAlign>\n", track->strf.nBlockAlign);
                fprintf(avi->xml, "  <wBitsPerSample>%u</wBitsPerSample>\n", track->strf.wBitsPerSample);
            }

            // Parse WAVEFORMATEX extention
            if (track->strf.wFormatTag == 0)
            {
                TRACE_2(AVI, "> EXT > No wave format TAG");
                track->strh.fccHandler = CODEC_UNKNOWN;
            }
            else
            {
                int byte_left = strf_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

                if (track->strf.wFormatTag == WAVE_FORMAT_MP1)
                {
                    TRACE_1(AVI, "> EXT > MPEG1WAVEFORMAT");
                    track->strh.fccHandler = CODEC_MPEG_L1;

                    if (byte_left >= 24)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));

                        uint16_t fwHeadLayer = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwHeadBitrate = endian_flip_32(read_bits(bitstr, 32));
                        uint16_t fwHeadMode = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t fwHeadModeExt = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t wHeadEmphasis = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t fwHeadFlags = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwPTSLow = endian_flip_32(read_bits(bitstr, 32));
                        uint32_t dwPTSHigh = endian_flip_32(read_bits(bitstr, 32));
#if ENABLE_DEBUG
                        TRACE_1(AVI, "> cbSize      : %u", cbSize);
                        TRACE_1(AVI, "> fwHeadLayer : %u", fwHeadLayer);
                        TRACE_1(AVI, "> dwHeadBitrate: %u", dwHeadBitrate);
                        TRACE_1(AVI, "> fwHeadMode  : %u", fwHeadMode);
                        TRACE_1(AVI, "> fwHeadModeExt: %u", fwHeadModeExt);
                        TRACE_1(AVI, "> wHeadEmphasis: %u", wHeadEmphasis);
                        TRACE_1(AVI, "> fwHeadFlags : %u", fwHeadFlags);
                        TRACE_1(AVI, "> dwPTSLow    : %u", dwPTSLow);
                        TRACE_1(AVI, "> dwPTSHigh   : %u", dwPTSHigh);
#endif
                        if (avi->xml)
                        {
                            fprintf(avi->xml, "  <cbSize>%u</cbSize>\n", cbSize);
                            fprintf(avi->xml, "  <fwHeadLayer>%u</fwHeadLayer>\n", fwHeadLayer);
                            fprintf(avi->xml, "  <dwHeadBitrate>%u</dwHeadBitrate>\n", dwHeadBitrate);
                            fprintf(avi->xml, "  <fwHeadMode>%u</fwHeadMode>\n", fwHeadMode);
                            fprintf(avi->xml, "  <fwHeadModeExt>%u</fwHeadModeExt>\n", fwHeadModeExt);
                            fprintf(avi->xml, "  <wHeadEmphasis>%u</wHeadEmphasis>\n", wHeadEmphasis);
                            fprintf(avi->xml, "  <fwHeadFlags>%u</fwHeadFlags>\n", fwHeadFlags);
                            fprintf(avi->xml, "  <dwPTSLow>%u</dwPTSLow>\n", dwPTSLow);
                            fprintf(avi->xml, "  <dwPTSHigh>%u</dwPTSHigh>\n", dwPTSHigh);
                        }
                    }
                }
                else if (track->strf.wFormatTag == WAVE_FORMAT_MP3)
                {
                    TRACE_1(AVI, "> EXT > MPEGLAYER3WAVEFORMAT");
                    track->strh.fccHandler = CODEC_MPEG_L3;

                    if (byte_left >= 11)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));

                        uint16_t wID = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t fdwFlags = endian_flip_32(read_bits(bitstr, 32));
                        uint16_t nBlockSize = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t nFramesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t nCodecDelay = endian_flip_16(read_bits(bitstr, 16));
#if ENABLE_DEBUG
                        TRACE_1(AVI, "> cbSize          : %u", cbSize);
                        TRACE_1(AVI, "> wID             : %u", wID);
                        TRACE_1(AVI, "> fdwFlags        : %u", fdwFlags);
                        TRACE_1(AVI, "> nBlockSize      : %u", nBlockSize);
                        TRACE_1(AVI, "> nFramesPerBlock : %u", nFramesPerBlock);
                        TRACE_1(AVI, "> nCodecDelay     : %u", nCodecDelay);
#endif
                        if (avi->xml)
                        {
                            fprintf(avi->xml, "  <cbSize>%u</cbSize>\n", cbSize);
                            fprintf(avi->xml, "  <wID>%u</wID>\n", wID);
                            fprintf(avi->xml, "  <fdwFlags>%u</fdwFlags>\n", fdwFlags);
                            fprintf(avi->xml, "  <nBlockSize>%u</nBlockSize>\n", nBlockSize);
                            fprintf(avi->xml, "  <nFramesPerBlock>%u</nFramesPerBlock>\n", nFramesPerBlock);
                            fprintf(avi->xml, "  <nCodecDelay>%u</nCodecDelay>\n", nCodecDelay);
                        }
                    }
                }
                else if (track->strf.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                {
                    TRACE_1(AVI, "> EXT > MPEG1WAVEFORMAT");
                    track->strh.fccHandler = CODEC_LPCM;

                    if (byte_left >= 28)
                    {
                        uint16_t cbSize = endian_flip_16(read_bits(bitstr, 16));

                        uint16_t samples_wValidBitsPerSample = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t samples_wSamplesPerBlock = endian_flip_16(read_bits(bitstr, 16));
                        uint16_t samples_wReserved = endian_flip_16(read_bits(bitstr, 16));
                        uint32_t dwChannelMask = endian_flip_32(read_bits(bitstr, 32));

                        uint8_t SubFormat_GUID[16];
                        read_uuid_le(bitstr, SubFormat_GUID);

                        char SubFormat_GUID_str[39];
                        getGuidString(SubFormat_GUID, SubFormat_GUID_str);

#if ENABLE_DEBUG
                        TRACE_1(AVI, "> cbSize: %u", cbSize);
                        TRACE_1(AVI, "> samples_wValidBitsPerSample : %u", samples_wValidBitsPerSample);
                        TRACE_1(AVI, "> samples_wSamplesPerBlock    : %u", samples_wSamplesPerBlock);
                        TRACE_1(AVI, "> samples_wReserved   : %u", samples_wReserved);
                        TRACE_1(AVI, "> dwChannelMask       : %u", dwChannelMask);
                        TRACE_1(AVI, "> SubFormat_GUID      : %s", SubFormat_GUID_str);
#endif
                        if (avi->xml)
                        {
                            fprintf(avi->xml, "  <cbSize>%u</cbSize>\n", cbSize);
                            fprintf(avi->xml, "  <samples_wValidBitsPerSample>%u</samples_wValidBitsPerSample>\n", samples_wValidBitsPerSample);
                            fprintf(avi->xml, "  <samples_wSamplesPerBlock>%u</samples_wSamplesPerBlock>\n", samples_wSamplesPerBlock);
                            fprintf(avi->xml, "  <samples_wReserved>%u</samples_wReserved>\n", samples_wReserved);
                            fprintf(avi->xml, "  <dwChannelMask>%u</dwChannelMask>\n", dwChannelMask);
                            fprintf(avi->xml, "  <SubFormat_GUID>%s</SubFormat_GUID>\n", SubFormat_GUID_str);
                        }
                    }
                }
                else
                {
                    TRACE_1(AVI, "> EXT > %s", getTccString(track->strf.wFormatTag));
                    track->strh.fccHandler = getCodecFromTwoCC(track->strf.wFormatTag);
                }
            }
        }

        if (avi->xml) fprintf(avi->xml, "  </a>\n");
    }

    bitstream_print_absolute_byte_offset(bitstr);

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
    TRACE_INFO(AVI, BLD_GREEN "parse_strl()" CLR_RESET);
    int retcode = SUCCESS;
    int track_id = 0;
    char fcc[5];

    if (strl_header != NULL &&
        strl_header->dwFourCC == fcc_strl)
    {
        // Print list header
        print_list_header(strl_header);
        write_list_header(strl_header, avi->xml, "Stream List", "track");

        // Init a new AviTrack_t structure to store strl content
        track_id = avi->tracks_count;
        avi->tracks[track_id] = (AviTrack_t*)calloc(1, sizeof(AviTrack_t));

        if (avi->tracks[track_id] == NULL)
        {
            TRACE_ERROR(AVI, "Unable to allocate a new avi track!");
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
               bitstream_get_absolute_byte_offset(bitstr) < (strl_header->offset_end - 8))
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    retcode = parse_unkn_list(bitstr, &list_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, strl_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_strh:
                    retcode = parse_strh(bitstr, &chunk_header, avi, avi->tracks[track_id]);
                    break;
                case fcc_strf:
                    retcode = parse_strf(bitstr, &chunk_header, avi, avi->tracks[track_id]);
                    break;
                //case fcc_strd:
                //    retcode = parse_strd(bitstr, &chunk_header, avi->tracks[track_id]);
                //    break;
                case fcc_strn:
                    retcode = parse_string(bitstr, &chunk_header, avi);
                    break;
                case fcc_indx:
                    retcode = parse_indx(bitstr, &chunk_header, avi, avi->tracks[track_id]);
                    break;
                case fcc_JUNK:
                    retcode = parse_JUNK(bitstr, &chunk_header, avi->xml);
                    break;
                default:
                    retcode = parse_unkn_chunk(bitstr, &chunk_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, strl_header, chunk_header.offset_end);
            }
        }

        if (avi->xml) fprintf(avi->xml, "  </a>\n");
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a strl box");
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
    TRACE_INFO(AVI, BLD_GREEN "parse_odml()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (odml_header != NULL &&
        odml_header->dwFourCC == fcc_odml)
    {
        // Set OpenDML profile
        avi->profile = PROF_AVI_OpenDML;

        // Print list header
        print_list_header(odml_header);
        write_list_header(odml_header, avi->xml, "Open DML extension");

        // Loop on "odml" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (odml_header->offset_end - 8))
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    retcode = parse_unkn_list(bitstr, &list_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, odml_header, list_header.offset_end);
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
                case fcc_vprp:
                    retcode = parse_vprp(bitstr, &chunk_header, avi);
                    break;
                default:
                    retcode = parse_unkn_chunk(bitstr, &chunk_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, odml_header, chunk_header.offset_end);
            }
        }

        if (avi->xml) fprintf(avi->xml, "  </a>\n");
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a odml box");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Movie Datas.
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
    TRACE_INFO(AVI, BLD_GREEN "parse_movi()" CLR_RESET);
    int retcode = SUCCESS;

    if (movi_header != NULL &&
        movi_header->dwFourCC == fcc_movi)
    {
        // Print list header
        print_list_header(movi_header);
        write_list_header(movi_header, avi->xml);
        if (avi->xml) fprintf(avi->xml, "  <title>Movie Datas</title>\n");
        if (avi->xml) fprintf(avi->xml, "  </a>\n");

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
                    retcode = parse_unkn_list(bitstr, &list_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, movi_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                default:
                    retcode = parse_unkn_chunk(bitstr, &chunk_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, movi_header, chunk_header.offset_end);
            }
        }
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a movi box");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_INFO(Bitstream_t *bitstr, RiffList_t *INFO_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_INFO()" CLR_RESET);
    int retcode = SUCCESS;

    if (INFO_header != NULL &&
        INFO_header->dwFourCC == fcc_INFO)
    {
        // Print INFO list header
        print_list_header(INFO_header);
        write_list_header(INFO_header, avi->xml, "INFO");

        // Loop on "INFO" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (INFO_header->offset_end - 8))
        {
            if (next_bits(bitstr, 32) == fcc_LIST)
            {
                RiffList_t list_header;
                retcode = parse_list_header(bitstr, &list_header);

                switch (list_header.dwFourCC)
                {
                default:
                    retcode = parse_unkn_list(bitstr, &list_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, INFO_header, list_header.offset_end);
            }
            else
            {
                RiffChunk_t chunk_header;
                retcode = parse_chunk_header(bitstr, &chunk_header);

                switch (chunk_header.dwFourCC)
                {
                case fcc_JUNK:
                    retcode = parse_JUNK(bitstr, &chunk_header, avi->xml);
                    break;

                case fcc_ISFT:
                case fcc_INAM:
                case fcc_IART:
                case fcc_IPRD:
                    retcode = parse_string(bitstr, &chunk_header, avi);
                    break;
                default:
                    retcode = parse_unkn_chunk(bitstr, &chunk_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, INFO_header, chunk_header.offset_end);
            }
        }

        if (avi->xml) fprintf(avi->xml, "  </a>\n");
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a INFO box");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

static int parse_hdrl(Bitstream_t *bitstr, RiffList_t *hdrl_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_hdrl()" CLR_RESET);
    int retcode = SUCCESS;

    if (hdrl_header != NULL &&
        hdrl_header->dwFourCC == fcc_hdrl)
    {
        // Print list header
        print_list_header(hdrl_header);
        write_list_header(hdrl_header, avi->xml, "Main AVI Header");

        // Loop on "hdrl" content
        while (avi->run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (hdrl_header->offset_end - 8))
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
                    retcode = parse_unkn_list(bitstr, &list_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, hdrl_header, list_header.offset_end);
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
                    retcode = parse_JUNK(bitstr, &chunk_header, avi->xml);
                    break;
                case fcc_ISFT:
                    retcode = parse_string(bitstr, &chunk_header, avi);
                    break;
                default:
                    retcode = parse_unkn_chunk(bitstr, &chunk_header, avi->xml);
                    break;
                }

                retcode = jumpy_riff(bitstr, hdrl_header, chunk_header.offset_end);
            }
        }

        if (avi->xml) fprintf(avi->xml, "  </a>\n");
    }
    else
    {
        TRACE_ERROR(AVI, "We are not in a hdrl box");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int avi_fileParse(MediaFile_t *media)
{
    TRACE_INFO(AVI, BLD_GREEN "avi_fileParse()" CLR_RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an AVI structure
        avi_t avi;
        memset(&avi, 0, sizeof(avi));

        // A convenient way to stop the parser
        avi.run = true;

        // xmlMapper
        xmlMapperOpen(media, &avi.xml);

        // Loop on 1st level list
        while (avi.run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (media->file_size - 8))
        {
            // Read RIFF header
            RiffList_t RIFF_header;
            retcode = parse_list_header(bitstr, &RIFF_header);
            print_list_header(&RIFF_header);
            write_list_header(&RIFF_header, avi.xml);

            if (RIFF_header.dwList == fcc_RIFF &&
                RIFF_header.dwFourCC == fcc_AVI_)
            {
                // Loop on 2nd level list/chunk
                while (avi.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
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
                            retcode = parse_unkn_list(bitstr, &list_header, avi.xml);
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
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, media, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &chunk_header, avi.xml);
                            break;
                        default:
                            retcode = parse_unkn_chunk(bitstr, &chunk_header, avi.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }
            else if (RIFF_header.dwList == fcc_RIFF &&
                     RIFF_header.dwFourCC == fcc_AVIX)
            {
                // Loop on 2nd level list/chunk
                while (avi.run == true &&
                       retcode == SUCCESS &&
                       bitstream_get_absolute_byte_offset(bitstr) < (RIFF_header.offset_end - 8))
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
                            retcode = parse_unkn_list(bitstr, &list_header, avi.xml);
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
                        case fcc_idx1:
                            retcode = parse_idx1(bitstr, media, &chunk_header, &avi);
                            break;
                        case fcc_JUNK:
                            retcode = parse_JUNK(bitstr, &chunk_header, avi.xml);
                            break;
                        default:
                            retcode = parse_unkn_chunk(bitstr, &chunk_header, avi.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }
            else // unknown list, we still want to map it
            {
                // Loop on 2st level chunks
                while (avi.run == true &&
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
                            retcode = parse_unkn_list(bitstr, &list_header, avi.xml);
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
                            retcode = parse_JUNK(bitstr, &chunk_header, avi.xml);
                            break;
                        default:
                            retcode = parse_unkn_chunk(bitstr, &chunk_header, avi.xml);
                            break;
                        }

                        retcode = jumpy_riff(bitstr, &RIFF_header, chunk_header.offset_end);
                    }
                }
            }

            if (avi.xml) fprintf(avi.xml, "  </a>\n");

            // Check if we have our super index, or if the tracks have been indexed
            int track_indexed = 0, track_superindexed = 0;

            for (unsigned i = 0; i < avi.tracks_count; i++)
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
        media->container_profile = avi.profile;

        // xmlMapper
        if (xmlMapperFinalize(avi.xml) == SUCCESS)
            media->container_mapper_fd = avi.xml;

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
