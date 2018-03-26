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
 * \file      esparser.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_ES_H
#define PARSER_ES_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

//! Start codes used by elementary streams of different codec families.
typedef enum StartCodes_e
{
    STARTCODE_MPEG = 0x000001,
    STARTCODE_VP8  = 0x9D012A

} StartCodes_e;

/* ************************************************************************** */

/*!
 * \brief Parse elementary stream files, the "brute-force" way.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \param video_codec: docme.
 * \return 1 if succeed, 0 otherwise.
 *
 * \todo Handle idr_only == false.
 * \todo Handle INITIAL_SEARCH_WINDOW.
 *
 * This parser is just brutally searching for 3 bytes start codes, for instance
 * 0x000001 for mpeg videos and 0x9D012A for VP8 videos.
 *
 * It will only successfuly parse "ES" files containing only one audio or video
 * track.
 */
int es_fileParse(MediaFile_t *media, Codecs_e video_codec);

/* ************************************************************************** */
#endif // PARSER_ES_H
