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
 * \file      mp4_gopro.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef PARSER_MP4_GOPRO_H
#define PARSER_MP4_GOPRO_H

// minivideo headers
#include "mp4_struct.h"
#include "../../bitstream.h"

/* ************************************************************************** */

int parse_firm(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_lens(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_came(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4);
int parse_sett(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_amba(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_muid(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_hmmt(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_bcid(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_guri(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_gusi(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_gumi(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);
int parse_gpmf(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4);

/* ************************************************************************** */
#endif // PARSER_MP4_GOPRO_H
