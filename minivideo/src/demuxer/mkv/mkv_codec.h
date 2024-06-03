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
 * \file      mkv_codec.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_MKV_CODEC_H
#define PARSER_MKV_CODEC_H

// minivideo headers
#include "ebml.h"
#include "mkv_struct.h"

/* ************************************************************************** */

void mkv_codec_from_string(char *codec_str, Codecs_e *codec, CodecProfiles_e *profile);

/* ************************************************************************** */

int parse_codec_private(Bitstream_t *bitstr, EbmlElement_t *element, mkv_t *mkv);

int parse_h264_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);
int parse_h265_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);
int parse_h266_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);

int parse_vpx_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);
int parse_av1_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);

int parse_mvc_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);
int parse_dolbyvision_private(Bitstream_t *bitstr, mkv_track_t *track, mkv_t *mkv);

/* ************************************************************************** */
#endif // PARSER_MKV_CODEC_H
