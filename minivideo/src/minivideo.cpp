/*!
 * COPYRIGHT (C) 2010-2020 Emeric Grange - All Rights Reserved
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
 * \date      2016
 */

// Library public header
#include "minivideo.h"

// Library privates headers
#include "minitraces.h"
#include "minivideo_typedef.h"
#include "import.h"
#include "thirdparty/portable_endian.h"

// Demuxers
#include "demuxer/avi/avi.h"
#include "demuxer/asf/asf.h"
#include "demuxer/mkv/mkv.h"
#include "demuxer/mp4/mp4.h"
#include "demuxer/mp3/mp3.h"
#include "demuxer/wave/wave.h"
#include "demuxer/mpeg/ps/ps.h"
#include "demuxer/esparser/esparser.h"
#include "demuxer/idr_filter.h"

// Decoder
#include "decoder/h264/h264.h"

// Muxer
#include "muxer/muxer.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* ************************************************************************** */

void minivideo_print_infos(void)
{
    printf(BLD_GREEN "minivideo_print_infos()\n" CLR_RESET);
    printf("* Library version '%d.%d-%d'\n", minivideo_VERSION_MAJOR,
                                             minivideo_VERSION_MINOR,
                                             minivideo_VERSION_PATCH);

    printf("* Library built with C++ '%li'\n", __cplusplus);

#if defined(__ICC) || defined(__INTEL_COMPILER)
    printf("* Library built with ICC '%d / %s'\n", __INTEL_COMPILER, __INTEL_COMPILER_BUILD_DATE);
#elif defined(_MSC_VER)
    printf("* Library built with MSVC '%d'\n", _MSC_VER);
#elif defined(__clang__)
    printf("* Library built with CLANG '%d.%d'\n", __clang_major__, __clang_minor__);
#elif defined(__GNUC__) || defined(__GNUG__)
    printf("* Library built with GCC '%d.%d.%d'\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    printf("* Library built with an unknown compiler\n");
#endif

    printf("* Library built on '%s, %s'\n", __DATE__ , __TIME__);

#if ENABLE_DEBUG == 1
    printf("* This is a " BLD_YELLOW "DEBUG" CLR_RESET " build\n" CLR_RESET);
#endif

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
    printf(OUT_YELLOW "* Address Sanitizer is ENABLED\n" CLR_RESET);
#endif
#else
#if defined(__SANITIZE_ADDRESS__)
    printf(OUT_YELLOW "* Address Sanitizer is ENABLED\n" CLR_RESET);
#endif
#endif // defined(__has_feature)
}

/* ************************************************************************** */

void minivideo_print_features(void)
{
    printf(BLD_GREEN "minivideo_print_features()\n" CLR_RESET);

#if ENABLE_STBIMWRITE
    printf("* STB_IMAGE_WRITE support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* STB_IMAGE_WRITE support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_WEBP
    printf("* EXTERNAL WEBP support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL WEBP support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_JPEG
    printf("* EXTERNAL JPEG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL JPEG support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_PNG
    printf("* EXTERNAL PNG support is " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* EXTERNAL PNG support is " BLD_RED "OFF\n" CLR_RESET);
#endif

#if ENABLE_COLORS
    printf("* COLORS are " BLD_GREEN "ON\n" CLR_RESET);
#else
    printf("* COLORS are OFF\n");
#endif

#if ENABLE_DEBUG
    printf("* DEBUG traces are " BLD_GREEN "ON\n" CLR_RESET);
    TRACE_ERROR(MAIN, "TEST error");
    TRACE_WARNING(MAIN, "TEST warning");
    TRACE_INFO(MAIN, "TEST info");
    TRACE_1(MAIN, "TEST lvl 1");
    TRACE_2(MAIN, "TEST lvl 2");
    TRACE_3(MAIN, "TEST lvl 3");
#else
    printf("* DEBUG traces are " BLD_RED "OFF\n" CLR_RESET);
#endif
}

/* ************************************************************************** */

void minivideo_get_infos(int *minivideo_major, int *minivideo_minor, int *minivideo_patch,
                         const char **minivideo_builddate,
                         const char **minivideo_buildtime,
                         bool *minivideo_builddebug)
{
    if (minivideo_major && minivideo_minor && minivideo_patch)
    {
        *minivideo_major = minivideo_VERSION_MAJOR;
        *minivideo_minor = minivideo_VERSION_MINOR;
        *minivideo_patch = minivideo_VERSION_PATCH;
    }

    if (minivideo_builddate && minivideo_buildtime)
    {
        *minivideo_builddate = __DATE__;
        *minivideo_buildtime = __TIME__;
    }

    if (minivideo_builddebug)
    {
#if ENABLE_DEBUG == 1
        *minivideo_builddebug = true;
#else
        *minivideo_builddebug = false;
#endif
    }
}

/* ************************************************************************** */

int minivideo_endianness(void)
{
    printf(BLD_GREEN "minivideo_endianness()\n" CLR_RESET);

    int endianness = -1;
    int i = 1;
    char *p = (char *)&i;

    if (p[0] == 1)
    {
        printf("* Host system is " BLD_BLUE "LITTLE_ENDIAN" CLR_RESET "\n");
        endianness = __LITTLE_ENDIAN; // 1234
    }
    else
    {
        printf("* Host system is " BLD_BLUE "BIG_ENDIAN" CLR_RESET "\n");
        endianness = __BIG_ENDIAN; // 4321
    }

    return endianness;
}

/* ************************************************************************** */

int minivideo_open(const char *input_filepath,
                   MediaFile_t **input_media)
{
    return import_fileOpen(input_filepath, input_media);
}

/* ************************************************************************** */

int minivideo_parse(MediaFile_t *input_media,
                    const bool compute_metadata,
                    const bool container_mapping)
{
    int retcode = FAILURE;

    if (input_media == nullptr)
    {
        TRACE_ERROR(MAIN, "Unable to parse NULL MediaFile_t struct!");
    }
    else if (input_media->file_size == 0)
    {
        TRACE_ERROR(MAIN, "Unable to parse emtpy file!");
    }
    else
    {
        // Container mapping
        input_media->container_mapper = container_mapping;

        // Start container parsing
        switch (input_media->container)
        {
            case CONTAINER_UNKNOWN:
                TRACE_ERROR(MAIN, "Unwknown container file format: unable to parse this file!");
                break;
            case CONTAINER_MP4:
                retcode = mp4_fileParse(input_media);
                break;
            case CONTAINER_MKV:
                retcode = mkv_fileParse(input_media);
                break;
            case CONTAINER_AVI:
                retcode = avi_fileParse(input_media);
                break;
            case CONTAINER_ASF:
                retcode = asf_fileParse(input_media);
                break;
            case CONTAINER_WAVE:
                retcode = wave_fileParse(input_media);
                break;
            case CONTAINER_MPEG_PS:
                retcode = ps_fileParse(input_media);
                break;
            case CONTAINER_ES:
                retcode = es_fileParse(input_media, CODEC_H264);
                break;
            case CONTAINER_ES_MP3:
                retcode = mp3_fileParse(input_media);
                break;
            default:
                TRACE_ERROR(MAIN, "Unable to parse given container format '%s': no parser available!",
                            getContainerString(input_media->container, false));
                break;
        }

        // Compute some additional metadata
        computeCodecs(input_media);
        computeAspectRatios(input_media);

        if (compute_metadata)
        {
            computeSamplesDatas(input_media);
#if ENABLE_DEBUG
            computeMediaMemory(input_media);
#endif
        }
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_thumbnail(MediaFile_t *input_media,
                        const char *output_directory,
                        const int picture_format,
                        const int picture_quality,
                        const int picture_count,
                        const int picture_extractionmode)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_thumbnail()" CLR_RESET);

    if (input_media != nullptr)
    {
        // Print status
        //import_fileStatus(input_media);

        int tid = 0;

        // IDR frame filtering
        int picture_number_filtered = idr_filtering(&input_media->tracks_video[tid],
                                                    picture_count, picture_extractionmode);

        if (picture_number_filtered == 0)
        {
            TRACE_ERROR(MAIN, "No picture to decode after filtering!");
        }

        for (unsigned sid = 0; sid < (unsigned)picture_number_filtered; sid++)
        {
            OutputFile_t out;
            memset(&out, 0, sizeof (OutputFile_t));

            if (output_directory && strlen(output_directory))
                strncpy(out.file_directory, output_directory, 254);
            else
                strncpy(out.file_directory, input_media->file_directory, 254);

            out.picture_format = picture_format;
            out.picture_quality = picture_quality;

            // Start video decoding
            switch (input_media->tracks_video[tid]->stream_codec)
            {
                case CODEC_H264:
                {
                    DecodingContext_t *dc = h264_init(input_media, tid);
                    retcode = h264_decode(dc, sid);
                    retcode = h264_export_file(dc, &out);
                    h264_cleanup(dc);
                } break;

                case CODEC_UNKNOWN:
                    TRACE_ERROR(MAIN, "Unknown video format: unable to decode this file!");
                    break;
                default:
                    TRACE_ERROR(MAIN, "Unable to decode given file format '%s': no decoder available!",
                                getCodecString(stream_VIDEO, input_media->tracks_video[0]->stream_codec, true));
                    break;
            }
        }
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to thumbnail from a NULL MediaFile_t struct!");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_extract(MediaFile_t *input_media,
                      const char *output_directory,
                      const bool extract_audio,
                      const bool extract_video,
                      const bool extract_subtitles)
{
    int retcode = FAILURE;

    TRACE_INFO(MAIN, BLD_GREEN "minivideo_extract()" CLR_RESET);

    if (input_media != NULL && output_directory != NULL)
    {
        // Print status
        //import_fileStatus(input_media);

        int tid = 0;

        // Export audio and video PES stream
        if (extract_audio)
            retcode = muxer_export_samples(input_media, input_media->tracks_audio[tid], 0);

        if (extract_video)
            retcode = muxer_export_samples(input_media, input_media->tracks_video[tid], 0);

        if (extract_subtitles)
            retcode = muxer_export_samples(input_media, input_media->tracks_subt[tid], 0);
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to extract from a NULL MediaFile_t struct!");
    }

    return retcode;
}

/* ************************************************************************** */

int minivideo_close(MediaFile_t **input_media)
{
    return import_fileClose(input_media);
}

/* ************************************************************************** */
/* ************************************************************************** */

OutputSurface_t *minivideo_decode_frame(MediaFile_t *input_media,
                                        const unsigned frame_id)
{
    OutputSurface_t *frame = nullptr;

    TRACE_1(MAIN, BLD_GREEN "minivideo_decode_frame()" CLR_RESET);

    if (input_media != nullptr)
    {
        int tid = 0;
        int retcode = 0;

        frame = new OutputSurface_t;

        // Start video decoding
        switch (input_media->tracks_video[tid]->stream_codec)
        {
            case CODEC_H264:
            {
                DecodingContext_t *dc = h264_init(input_media, tid);
                retcode = h264_decode(dc, frame_id);
                retcode &= h264_export_surface(dc, frame);
                h264_cleanup(dc);
            } break;

            case CODEC_UNKNOWN:
                TRACE_ERROR(MAIN, "Unknown video format: unable to decode this file!");
                break;

            default:
                TRACE_ERROR(MAIN, "Unable to decode given file format '%s': no decoder available!",
                            getCodecString(stream_VIDEO, input_media->tracks_video[0]->stream_codec, true));
                break;
        }

        if (retcode != 1)
            minivideo_destroy_frame(&frame);
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to extract a frame from a NULL MediaFile_t struct!");
    }

    return frame;
}

void minivideo_destroy_frame(OutputSurface_t **frame_ptr)
{
    if ((*frame_ptr) != nullptr)
    {
        // A malloc is performed by export_idr_surface()
        free((*frame_ptr)->surface);

        delete *frame_ptr;
        *frame_ptr = nullptr;
    }
}

/* ************************************************************************** */

MediaSample_t *minivideo_get_sample(MediaFile_t *input_media,
                                    MediaStream_t *input_stream,
                                    const unsigned sample_id)
{
    MediaSample_t *sample = nullptr;

    TRACE_1(MAIN, BLD_GREEN "minivideo_get_sample()" CLR_RESET);

    if (input_media != nullptr && input_stream != nullptr)
    {
        if (sample_id < input_stream->sample_count)
        {
            sample = new MediaSample_t;
            if (sample)
            {
                sample->type = input_stream->sample_type[sample_id];
                sample->size = input_stream->sample_size[sample_id];
                sample->offset = input_stream->sample_offset[sample_id];
                sample->pts = input_stream->sample_pts[sample_id];
                sample->dts = input_stream->sample_dts[sample_id];

                sample->data = new uint8_t[sample->size];
                if (sample->data)
                {
                    //TRACE_ERROR(MAIN, "error () sz: %u\n" , sample->size);
                    //TRACE_ERROR(MAIN, "error () of: %u\n" , sample->offset);

                    fseek(input_media->file_pointer, sample->offset, SEEK_SET);
                    size_t sz = fread(sample->data, sizeof(uint8_t), sample->size, input_media->file_pointer);
                    if (sz != sample->size)
                    {
                        TRACE_ERROR(MAIN, "error () sz: %u\n" , sz);
                    }
                }
                else
                {
                    TRACE_ERROR(MAIN, BLD_GREEN "minivideo_get_sample()" CLR_RESET);
                }
            }
            else
            {
                TRACE_ERROR(MAIN, "Unable to allocate sample!");
            }
        }
        else
        {
            TRACE_ERROR(MAIN, "Sample ID is invalid!");
        }
    }
    else
    {
        TRACE_ERROR(MAIN, "Unable to extract a sample from a NULL MediaFile_t or MediaStream_t struct!");
    }

    return sample;
}

void minivideo_destroy_sample(MediaSample_t **sample_ptr)
{
    if ((*sample_ptr) != nullptr)
    {
        delete [] (*sample_ptr)->data;

        delete *sample_ptr;
        *sample_ptr = nullptr;
    }
}

/* ************************************************************************** */
