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
 * \file      bitstream_map.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef BITSTREAM_MAP_H
#define BITSTREAM_MAP_H

// minivideo headers
#include "minivideo_mediafile.h"
#include "minivideo_mediastream.h"

/* ************************************************************************** */

int init_bitstream_map(MediaStream_t **stream_ptr, uint32_t parameter_count, uint32_t sample_count);

void free_bitstream_map(MediaStream_t **stream_ptr);

void print_bitstream_map(MediaStream_t *stream);

bool computeCodecs(MediaFile_t *media);

bool computeAspectRatios(MediaFile_t *media);

bool computeSamplesDatas(MediaFile_t *media);

bool computeMediaMemory(MediaFile_t *media);

/* ************************************************************************** */
#endif // BITSTREAM_MAP_H
