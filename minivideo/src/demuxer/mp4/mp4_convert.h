/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      mp4_convert.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef PARSER_MP4_CONVERT_H
#define PARSER_MP4_CONVERT_H

// minivideo headers
#include "mp4_struct.h"
#include "../../mediafile_struct.h"

/* ************************************************************************** */

bool convertTrack(MediaFile_t *media, Mp4_t *mp4, Mp4Track_t *track);

void freeTrack(Mp4Track_t **track_ptr);

/* ************************************************************************** */
#endif // PARSER_MP4_CONVERT_H
