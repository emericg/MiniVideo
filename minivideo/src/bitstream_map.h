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
 * \file      bitstream_map.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef BITSTREAM_MAP_H
#define BITSTREAM_MAP_H

// minivideo headers
#include "typedef.h"
#include "avcodecs.h"

/* ************************************************************************** */

//! Sample type
typedef enum SampleType_e
{
    sample_AUDIO = 0,

    sample_VIDEO,
    sample_VIDEO_IDR,
    sample_VIDEO_PARAM,

    sample_TEXT_FILE

} SampleType_e;

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
    AVCodec_e stream_codec;         //!< Stream codec

    // Video specific parameters
    unsigned int width;             //!< Horizontal size in pixels
    unsigned int height;            //!< Vertical size in pixels
    unsigned int color_depth;       //!< Color resolution per channel
    unsigned int color_subsampling; //!< Color sub-sampling format
    double frame_rate;              //!< Frame rate (in frame/s)

    // Audio specific parameters
    unsigned int bit_rate;          //!< Audio bit rate
    unsigned int sampling_rate;     //!< Audio sampling rate
    unsigned int channel_count;     //!< Number of audio channels

    // Subtitles specific parameters
    unsigned int encoding;          //!< Encoding format
    char *language_code;            //!< ISO 639-1 or ISO 639-2 language code
    char *subtitles_name;           //!< Subtitles name

    // Samples infos
    bool sample_alignment;          //!< Is every sample begining on an elementary stream / a nal header?
    uint32_t sample_count;          //!< The total number of samples in the track
    uint32_t sample_count_idr;      //!< The total number of IDR samples in the (video) track, providing random access points within the stream

    // Samples arrays
    uint32_t *sample_type;          //!< Type (for each samples of the track)
    uint32_t *sample_size;          //!< Size in byte
    int64_t *sample_offset;         //!< Offset in byte
    int64_t *sample_pts;            //!< Presentation timestamp (in milliseconds)
    int64_t *sample_dts;            //!< Decoding timestamp (in milliseconds)

} BitstreamMap_t;

/* ************************************************************************** */

int init_bitstream_map(BitstreamMap_t **bitstream_map, uint32_t entries);
void free_bitstream_map(BitstreamMap_t **bitstream_map);

void print_bitstream_map(BitstreamMap_t *bitstream_map);

/* ************************************************************************** */
#endif /* BITSTREAM_MAP_H */
