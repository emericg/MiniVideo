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
 * \file      muxer.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

// minivideo headers
#include "muxer.h"
#include "pes_packetizer.h"
#include "../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

/* ************************************************************************** */

static int write_pes(FILE *f_src, FILE *f_dst, MediaStream_t *stream)
{
    TRACE_INFO(MUXER, BLD_GREEN "> write_pes()" CLR_RESET);
    int retcode = SUCCESS;

    // Write packets
    for (unsigned i = 0; i < stream->sample_count; i++)
    {
        size_t size   = (size_t)stream->sample_size[i];
        size_t offset = (size_t)stream->sample_offset[i];

        TRACE_2(MUXER, " > Sample id     : %i", i);
        TRACE_2(MUXER, " | sample type   : %i", stream->sample_type[i]);
        TRACE_2(MUXER, " | sample size   : %i", size);
        TRACE_2(MUXER, " | sample offset : %i", offset);

        if (fseek(f_src, offset, SEEK_SET) != 0)
        {
            TRACE_ERROR(MUXER, "Unable to seek through the input file!");
            retcode = FAILURE;
        }
        else
        {
            uint8_t *pes_buffer = (uint8_t *)malloc(size);

            if (pes_buffer == NULL)
            {
                TRACE_ERROR(MUXER, "Unable to allocate pes_buffer!");
                retcode = FAILURE;
            }
            else
            {
                size_t read  = fread(pes_buffer, sizeof(uint8_t), size, f_src);
                size_t write = 0;

                if (read != size)
                {
                    TRACE_ERROR(MUXER, "read != size (%i / %i)", read, size);
                    retcode = FAILURE;
                }
                else
                {
                    write = fwrite(pes_buffer, sizeof(uint8_t), size, f_dst);

                    if (write != size)
                    {
                        TRACE_ERROR(MUXER, "write != size (%i / %i)", write, size);
                        retcode = FAILURE;
                    }
                }

                free(pes_buffer);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

static int write_es(FILE *f_src, FILE *f_dst, MediaStream_t *stream)
{
    TRACE_INFO(MUXER, BLD_GREEN "> write_es()" CLR_RESET);
    int retcode = SUCCESS;

    for (unsigned i = 0; i < stream->sample_count; i++)
    {
        size_t size   = (size_t)stream->sample_size[i];
        size_t offset = (size_t)stream->sample_offset[i];

        TRACE_2(MUXER, " > Sample id     : %i", i);
        TRACE_2(MUXER, " | sample type   : %i", stream->sample_type[i]);
        TRACE_2(MUXER, " | sample size   : %i", size);
        TRACE_2(MUXER, " | sample offset : %i", offset);

        if (fseek(f_src, offset, SEEK_SET) != 0)
        {
            TRACE_ERROR(MUXER, "Unable to seek through the input file!");
            retcode = FAILURE;
        }
        else
        {
            uint8_t *pes_buffer = (uint8_t *)malloc(size);

            if (pes_buffer == NULL)
            {
                TRACE_ERROR(MUXER, "Unable to allocate pes_buffer!");
                retcode = FAILURE;
            }
            else
            {
                size_t read  = fread(pes_buffer, sizeof(uint8_t), size, f_src);
                size_t write = 0;

                if (read != size)
                {
                    TRACE_ERROR(MUXER, "read != size (%i / %i)", read, size);
                    retcode = FAILURE;
                }
                else
                {
                    // Add 'Annex B' start code
                    if (stream->stream_codec == CODEC_H264)
                    {
                        uint8_t startcode[4] = { 0x00, 0x00, 0x00, 0x01 };
                        write = fwrite(startcode, sizeof(uint8_t), 4, f_dst);
                        if (write != 4)
                        {
                            TRACE_ERROR(MUXER, "fwrite != size (%i / %i)", write, 4);
                            retcode = FAILURE;
                        }
                    }

                    // Write ES
                    write = fwrite(pes_buffer, sizeof(uint8_t), size, f_dst);
                    if (write != size)
                    {
                        TRACE_ERROR(MUXER, "fwrite != size (%i / %i)", write, size);
                        retcode = FAILURE;
                    }
                }

                free(pes_buffer);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Check MediaFile_t and MediaStream_t structures, then track infos.
 * \return SUCCESS if so.
 */
static int stream_infos(MediaFile_t *media, MediaStream_t *stream)
{
    TRACE_INFO(MUXER, BLD_GREEN "> stream_infos()" CLR_RESET);
    int retcode = SUCCESS;

    // Check structures
    if (media && stream)
    {
        if (stream->stream_type == stream_AUDIO)
        {
            TRACE_1(MUXER, " > stream type : AUDIO");
        }
        else if (stream->stream_type == stream_VIDEO)
        {
            TRACE_1(MUXER, " > stream type : VIDEO");
        }
        else if (stream->stream_type == stream_TEXT)
        {
            TRACE_1(MUXER, " > stream type : VIDEO");
        }
        else
        {
            TRACE_ERROR(MUXER, " > This is not an AUDIO / VIDEO / TEXT stream!");
            retcode = FAILURE;
        }

        if (stream->sample_count > 0)
        {
            TRACE_1(MUXER, " > number of samples: %i", stream->sample_count);
        }
        else
        {
            TRACE_ERROR(MUXER, " > This stream does not contain any sample!?");
            retcode = FAILURE;
        }
    }
    else
    {
        TRACE_ERROR(MUXER, " > Void stream: cannot mux anything!");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Export a PES packet into a file.
 * \return SUCCESS if so.
 */
static int stream_output_filename(MediaFile_t *media, MediaStream_t *stream, char output_filename[255], const int output_format)
{
    TRACE_INFO(MUXER, BLD_GREEN "> stream_output_filename()" CLR_RESET);
    int retcode = SUCCESS;

    // Generate stream name
    strncpy(output_filename, media->file_name, 254);

    // Track number
    // TODO
    //strncat(output_filename, "_track1", 254);

    // File extension
    if (stream->stream_type == stream_AUDIO)
    {
        if (stream->stream_codec == CODEC_MPEG_L3)
        {
            strncat(output_filename, ".mp3", 254);
        }
        else if (stream->stream_codec == CODEC_AC3)
        {
            strncat(output_filename, ".ac3", 254);
        }
        else if (stream->stream_codec == CODEC_AAC)
        {
            strncat(output_filename, ".aac", 254);
        }
        else
        {
            strncat(output_filename, ".audio", 254);
            TRACE_WARNING(MUXER, " > Unknown AUDIO codec, using generic file extension");
        }
    }
    else // if (stream->stream_type == stream_VIDEO)
    {

        if (stream->stream_codec == CODEC_MPEG1 ||
            stream->stream_codec == CODEC_MPEG2)
        {
            strncat(output_filename, ".mpegv", 254);
        }
        else if (stream->stream_codec == CODEC_H264)
        {
            if (output_format == 0)
                strncat(output_filename, ".h264", 254);
            else
                strncat(output_filename, ".h264", 254);
        }
        else if (stream->stream_codec == CODEC_MPEG4_ASP)
        {
            if (output_format == 0)
                strncat(output_filename, ".mpgv", 254);
            else
                strncat(output_filename, ".mpgv", 254);
        }
        else
        {
            strncat(output_filename, ".video", 254);
            TRACE_WARNING(MUXER, " > Unknown VIDEO codec, using generic file extension");
        }
    }

    // Print generated file name
    TRACE_INFO(MUXER, " > File name : '%s'", output_filename);

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Export a PES packet into a file.
 * \return SUCCESS if so.
 */
int muxer_export_samples(MediaFile_t *media,
                         MediaStream_t *stream,
                         const int output_format)
{
    TRACE_INFO(MUXER, BLD_GREEN "> muxer_export_sample()" CLR_RESET);

    // Check stream
    int retcode = stream_infos(media, stream);

    if (retcode == SUCCESS)
    {
        // Stream output file name
        char output_filename[255];
        stream_output_filename(media, stream, output_filename, output_format);

        // Create & open media files
        FILE *f_src = media->file_pointer;
        FILE *f_dst = fopen(output_filename, "wb");

        if (f_dst == NULL)
        {
            TRACE_ERROR(MUXER, " > Cannot create or open output file!");
            retcode = FAILURE;
        }
        else
        {
/*
            if (stream->stream_level == stream_level_PES)
            {
                // Write PES packets
                retcode = write_pes(f_src, f_dst, stream);
            }
            else if (stream->stream_level == stream_level_ES)
*/
            {
                if (output_format == 0)
                {
                    // Just write ES
                    retcode = write_es(f_src, f_dst, stream);
                }
                else // if (output_format == 1)
                {
                    // Packetize ES into PES
                    retcode = pes_packetizer(f_src, f_dst, stream);
                }
            }

            // Close stream file
            if (fclose(f_dst) != 0)
            {
                TRACE_ERROR(MUXER, " > Cannot close output file!");
                retcode = FAILURE;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
