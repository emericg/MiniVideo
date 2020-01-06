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
 * \date      2010
 */

#ifndef EXPORT_UTILS_H
#define EXPORT_UTILS_H
/* ************************************************************************** */

// minivideo headers
#include "decoder/h264/h264_decodingcontext.h"

/* ************************************************************************** */

void make_path_absolute(const char *path, char *path_absolute);

unsigned mb_to_ycbcr(DecodingContext_t *dc, unsigned char *buffer_ycbcr);
unsigned mb_to_rgb(DecodingContext_t *dc, unsigned char *buffer_rgb);

/* ************************************************************************** */
#endif // EXPORT_UTILS_H
