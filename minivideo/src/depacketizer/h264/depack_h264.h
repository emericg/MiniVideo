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
 * \file      depack_h264.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef DEPACK_H264_H
#define DEPACK_H264_H

// minivideo headers
#include "../depack_struct.h"
#include "../../import.h"
#include "../../bitstream.h"

/* ************************************************************************** */

unsigned depack_h264_sample(Bitstream_t *bitstr, MediaStream_t *track,
                            unsigned sample_index, es_sample_t *essample_list);

/* ************************************************************************** */
#endif // DEPACK_H264_H
