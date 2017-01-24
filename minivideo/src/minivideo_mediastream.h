/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
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
 * \file      minivideo_mediastream.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef MINIVIDEO_MEDIASTREAM_H
#define MINIVIDEO_MEDIASTREAM_H

// minivideo headers
#include "minivideo_typedef.h"
#include "minivideo_codecs.h"

/* ************************************************************************** */

/*!
 * \struct MediaStream_t
 * \brief Infos on stream payload data & metadatas extracted from a container file.
 *
 * \todo split into 'per container' metadatas structures
 * \todo split into 'per codec' metadatas structures
 *
 * This structure basically represent an audio or a video track. It contains
 * general informations about the track type and the positions of all the
 * samples of the track inside the bitstream.
 */
typedef struct MediaStream_t
{
    StreamType_e stream_type;       //!< Is this an audio / video / subtitles stream?
    uint32_t stream_fcc;            //!< FourCC
    Codecs_e stream_codec;         //!< Codec used by this stream

    unsigned int stream_duration_ms;//!< Stream duration (rounded in milliseconds)
    unsigned int stream_duration_ns;//!< Stream duration (in nanoseconds)
    int stream_delay;               //!< Stream initial delay (in nanoseconds)

    // Generic metadatas
    char *stream_encoder;           //!< Encoder used to generate the stream's datas
    unsigned int creation_time;     //!< Stream creation time (ms?)
    unsigned int modification_time; //!< Stream modification time (ms?)

    // Track metadatas (if the container supports them)
    unsigned int track_id;          //!< Id of the track (as set by the container)
    bool track_default;             //!<
    bool track_forced;              //!<
    char *track_title;              //!< Title
    char *track_languagecode;       //!< Language code (ISO 639-1 or ISO 639-2 format, ASCII, NULL terminated)

    // Bitrate infos
    BitrateMode_e bitrate_mode;     //!< Bitrate mode
    unsigned int bitrate_avg;       //!< Average bitrate (in bit/s)
    unsigned int bitrate_min;       //!< Minimum bitrate (in bit/s)
    unsigned int bitrate_max;       //!< Maximum bitrate (in bit/s)

    // Video metadatas
    unsigned int width;             //!< Horizontal size (in pixels)
    unsigned int height;            //!< Vertical size (in pixels)
    unsigned int visible_width;     //!< Horizontal size (in pixels, without alignment)
    unsigned int visible_height;    //!< Vertical size (in pixels, without alignment)
    unsigned int color_depth;       //!< Color resolution per channel
    unsigned int color_subsampling; //!< Chroma sub-sampling
    unsigned int color_encoding;    //!< Internal color encoding
    unsigned int color_matrix;      //!< Internal color encoding
    unsigned int color_range;       //!< Colors are in restricted or full range
    double display_aspect_ratio;    //!< Display aspect ratio (if set directly by the container, otherwise computed)
        unsigned int display_aspect_ratio_h;
        unsigned int display_aspect_ratio_v;
    double video_aspect_ratio;      //!< Video aspect ratio (from video size)
        unsigned int video_aspect_ratio_h;
        unsigned int video_aspect_ratio_v;
    double pixel_aspect_ratio;      //!< Pixel aspect ratio (if set directly by the container)
        unsigned int pixel_aspect_ratio_h;
        unsigned int pixel_aspect_ratio_v;

    double framerate;               //!< Framerate (in frame/s)
        double framerate_num;       //!< Framerate numerator
        double framerate_base;      //!< Framerate denominator
    double frame_duration;          //!< Frame duration (in ms)
    unsigned int framerate_mode;    //!< Framerate mode

    // Audio metadatas
    unsigned int channel_count;     //!< Number of audio channels
    unsigned int channel_mode;      //!< Channels configuration
    unsigned int sampling_rate;     //!< Sampling rate (in Hertz)
    unsigned int bit_per_sample;    //!< Bit per sample (in bits)
    unsigned int sample_per_frames; //!< Audio samples per audio frame

    // Subtitles specific metadatas
    char *subtitles_name;           //!< Subtitles name?
    unsigned int subtitles_encoding;//!< Text encoding

    // Datas infos
    uint64_t stream_size;           //!< Size (in bytes) of the raw datas of this stream
    bool stream_intracoded;         //!< True if the stream is intra coded
    bool stream_packetized;         //!< True if a container sample isn't a complete audio/video/... frame

    // Parameter set arrays
    uint32_t parameter_count;       //!< The total number of parameter set in this stream
    uint32_t *parameter_type;       //!< Parameter set type
    uint32_t *parameter_size;       //!< Size (in byte)
    int64_t *parameter_offset;      //!< Offset (in byte)

    // Samples arrays
    uint32_t sample_count;          //!< The total number of samples in this stream
    uint32_t *sample_type;          //!< Sample type
    uint32_t *sample_size;          //!< Size (in byte)
    int64_t *sample_offset;         //!< Offset (in byte)
    int64_t *sample_pts;            //!< Presentation timestamp (in nanosecond)
    int64_t *sample_dts;            //!< Decoding timestamp (in nanosecond)

    // FIXME (frame stats)
    uint32_t frame_count;           //!< Number of audio/video frames
    uint32_t frame_count_idr;       //!< Number of audio/video IDR frames

} MediaStream_t;

/* ************************************************************************** */
#endif // MINIVIDEO_MEDIASTREAM_H
