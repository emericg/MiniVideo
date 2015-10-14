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
 * This structure basically represent an audio or a video track. It contains
 * general informations about the track type and the positions of all the
 * samples of the track inside the bitstream.
 */
typedef struct BitstreamMap_t
{
    StreamType_e stream_type;       //!< Is this an audio / video / subtitles stream?
    StreamLevel_e stream_level;     //!< Does the stream contains PES or ES elements?

    // Generic metadatas
    AVCodec_e stream_codec;         //!< Stream codec
    unsigned int stream_size;       //!< Size of the raw datas of this stream, used for stats

    bool track_default;             //!<
    bool track_forced;              //!<
    char language_code[3];          //!< Language code (ISO 639-1 or ISO 639-2 format)

    unsigned int bitrate;           //!< Bitrate (in bit/s)
    unsigned int bitrate_mode;      //!< Bitrate mode

    unsigned int duration;          //!< Stream duration (in milliseconds)
    unsigned int creation_time;     //!< Stream creation time (ms?)
    unsigned int modification_time; //!< Stream modification time (ms?)

    // Video specific metadatas
    unsigned int width;             //!< Horizontal size (in pixels)
    unsigned int height;            //!< Vertical size (in pixels)
    unsigned int visible_width;     //!< Horizontal size (in pixels, without alignment)
    unsigned int visible_height;    //!< Vertical size (in pixels, without alignment)
    unsigned int color_depth;       //!< Color resolution per channel
    unsigned int color_subsampling; //!< Chroma sub-sampling
    unsigned int color_encoding;    //!< Internal color encoding
    double frame_rate;              //!< Frame rate (in frame/s)
    unsigned int framerate_num;     //!< Frame rate
    unsigned int framerate_base;    //!< Frame rate base

    // Audio specific metadatas
    unsigned int sampling_rate;     //!< Sampling rate (in Hertz)
    unsigned int channel_count;     //!< Number of audio channels

    // Subtitles specific metadatas
    char *subtitles_name;           //!< Subtitles name?
    unsigned int subtitles_encoding;//!< Text encoding

    // Samples infos
    bool sample_alignment;          //!< Is every sample begining on an elementary stream / a NAL Header?
    uint32_t sample_count;          //!< The total number of samples in this track
    uint32_t sample_count_idr;      //!< The total number of IDR samples in the (video) track, providing random access points within the stream

    // Samples arrays
    uint32_t *sample_type;          //!< Type (for each samples of the track)
    uint32_t *sample_size;          //!< Size in byte
    int64_t *sample_offset;         //!< Offset in byte
    int64_t *sample_pts;            //!< Presentation timestamp (in milliseconds)
    int64_t *sample_dts;            //!< Decoding timestamp (in milliseconds)

} BitstreamMap_t;

/* ************************************************************************** */
#endif // BITSTREAM_MAP_STRUCT_H
