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
 * \file      h264_expgolomb.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_EXPGOLOMB_H
#define H264_EXPGOLOMB_H

// minivideo headers
#include "../../bitstream.h"

/* ************************************************************************** */

int read_se(Bitstream_t *bitstr);
unsigned int read_ue(Bitstream_t *bitstr);
unsigned int read_me(Bitstream_t *bitstr, unsigned int ChromaArrayType, bool intracoding_flag);
unsigned int read_te(Bitstream_t *bitstr, int range);

/* ************************************************************************** */
#endif // H264_EXPGOLOMB_H
