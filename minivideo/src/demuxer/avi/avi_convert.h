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
 * \file      avi_convert.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef PARSER_AVI_CONVERT_H
#define PARSER_AVI_CONVERT_H

// minivideo headers
#include "avi_struct.h"
#include "../riff/riff_struct.h"
#include "../../bitstream.h"
#include "../../minivideo_mediafile.h"

/* ************************************************************************** */

int parse_idx1(Bitstream_t *bitstr, MediaFile_t *media, RiffChunk_t *idx1_header, avi_t *avi);

int parse_indx(Bitstream_t *bitstr, RiffChunk_t *indx_header, avi_t *avi,  AviTrack_t *track);

/* ************************************************************************** */

int avi_indexer(Bitstream_t *bitstr, MediaFile_t *media, avi_t *avi);

int avi_indexer_initmap(MediaFile_t *media, AviTrack_t *track, uint32_t index_entry_count);

void avi_clean(avi_t *avi);

/* ************************************************************************** */
#endif // PARSER_AVI_CONVERT_H
