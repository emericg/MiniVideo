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
 * \file      bitstream_map_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef BITSTREAM_MAP_STRUCT_H
#define BITSTREAM_MAP_STRUCT_H

// minivideo headers
#include "typedef.h"
#include "avcodecs.h"

/* ************************************************************************** */

/*!
 * \struct BitstreamMap_t
 * \brief Structure used to keep tracks of audio and video payload data extracted from container file.
 *
 * \todo massive cleanup / reorganization
 * \todo clarify units and metadatas 'availability'
 * \todo split into 'per container' metadatas structures
 * \todo split into 'per codec' metadatas structures
 *
 * This structure basically represent an audio or a video track. It contains
 * general informations about the track type and the positions of all the
 * samples of the track inside the bitstream.
 */
typedef struct BitstreamMap_t
{
    // Generic metadatas
    StreamType_e stream_type;       //!< Is this an audio / video / subtitles stream?
    uint64_t stream_size;           //!< Size (in bytes) of the raw datas of this stream, used for stats

    uint32_t stream_fcc;            //!< FourCC
    AVCodec_e stream_codec;         //!< Codec used by this track
    bool stream_intracoded;         //!< True if the stream is intra coded
    char *stream_encoder;           //!< Encoder used to generate the track's datas

    // Track metadatas (if the container support them)
    unsigned int track_id;          //!< Id of the track (as set by the container)
    char *track_title;              //!< Title
    char *track_languagecode;       //!< Language code (ISO 639-1 or ISO 639-2 format)
    char track_language[3];         //!< Language code (ISO 639-1 or ISO 639-2 format)
    bool track_default;             //!<
    bool track_forced;              //!<

    unsigned int bitrate;           //!< Average bitrate (in bit/s)
    unsigned int bitrate_mode;      //!< Bitrate mode
    unsigned int bitrate_min;       //!< Minimum bitrate (in bit/s)
    unsigned int bitrate_max;       //!< Maximum bitrate (in bit/s)

    unsigned int duration_ms;       //!< Stream duration (in milliseconds)
    unsigned int creation_time;     //!< Stream creation time (ms?)
    unsigned int modification_time; //!< Stream modification time (ms?)

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
        unsigned int sample_per_frames;     //!< audio samples per audio frame

        // PCM specific metadatas
        unsigned int pcm_sample_size;       //!< PCM sample size (in bytes)
        unsigned int pcm_sample_format;     //!< PCM sample format (signed, unsigned, float, ...)
        unsigned int pcm_sample_endianness; //!< PCM samples endianness (little, big)

    // Subtitles specific metadatas
    char *subtitles_name;           //!< Subtitles name?
    unsigned int subtitles_encoding;//!< Text encoding

    // Sample infos
    bool sample_alignment;          //!< True if every container sample is a proper audio/video frame

    uint32_t sample_count;          //!< The total number of samples in this track
    uint32_t frame_count;           //!< Number of audio/video frames
    uint32_t frame_count_idr;       //!< Number of video IDR frames

    // Samples arrays
    uint32_t *sample_type;          //!< Type (for each samples of the track)
    uint32_t *sample_size;          //!< Size (in byte)
    int64_t *sample_offset;         //!< Offset (in byte)
    int64_t *sample_pts;            //!< Presentation timestamp (in milliseconds?)
    int64_t *sample_dts;            //!< Decoding timestamp (in milliseconds?)

} BitstreamMap_t;

/* ************************************************************************** */
#endif // BITSTREAM_MAP_STRUCT_H
