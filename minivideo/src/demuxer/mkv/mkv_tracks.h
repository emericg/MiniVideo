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
 * \file      mkv_tracks.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_MKV_TRACKS_H
#define PARSER_MKV_TRACKS_H

// minivideo headers
#include "../../bitstream.h"
#include "ebml.h"
#include "mkv.h"

/* ************************************************************************** */

int mkv_parse_tracks(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv);

/* ************************************************************************** */
#endif // PARSER_MKV_TRACKS_H
