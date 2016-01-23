/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
 * \file      bruteforce.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_BRUTEFORCE_H
#define PARSER_BRUTEFORCE_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

//! Start code used by codec family
typedef enum StartCodeFamily_e
{
    STARTCODE_MPEG = 0x000001,
    STARTCODE_VP8  = 0x9D012A

} StartCodeFamily_e;

/* ************************************************************************** */

int bruteforce_fileParse(MediaFile_t *video, AVCodec_e video_codec);

/* ************************************************************************** */
#endif // PARSER_BRUTEFORCE_H
