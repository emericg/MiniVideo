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
 * \file      mp3.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_MP3_H
#define PARSER_MP3_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

/*!
 * \brief Parser MP3 files.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * This parser index the content of MP1, MP2 and MP3 Elementary Streams.
 * It implements ISO/IEC 11172-3 and ISO/IEC 13818-3 specifications.
 *
 * ID3 tags (v1, v2 and Lyrics) are ignored:
 * - http://www.id3.org/ID3v1
 * - http://www.id3.org/id3v2.4.0-structure
 * - http://www.id3.org/id3v2.4.0-frames
 *
 * APE tags are ignored:
 * - http://wiki.hydrogenaudio.org/index.php?title=APEv2_specification
 *
 * Xing and Lame header are ignored:
 * - http://www.mp3-tech.org/programmer/sources/vbrheadersdk.zip
 * - http://gabriel.mp3-tech.org/mp3infotag.html
 *
 * VBR Info header are not supported.
 * - http://www.iis.fraunhofer.de/bf/amm/download/MP3%20VBR-Header%20Software%20Development%20Kit.zip
 */
int mp3_fileParse(MediaFile_t *media);

/* ************************************************************************** */
#endif // PARSER_MP3_H
