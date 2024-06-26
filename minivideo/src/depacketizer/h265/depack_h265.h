/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \date      2024
 */

#ifndef DEPACK_H265_H
#define DEPACK_H265_H
/* ************************************************************************** */

// minivideo headers
#include "../depack_struct.h"
#include "../../bitstream.h"

#include <vector>

/* ************************************************************************** */

unsigned depack_h265_sample(Bitstream_t *bitstr,
                            MediaStream_t *track, unsigned sample_index,
                            std::vector <es_sample_t> &samples, FILE *xml);

/* ************************************************************************** */
#endif // DEPACK_H265_H
