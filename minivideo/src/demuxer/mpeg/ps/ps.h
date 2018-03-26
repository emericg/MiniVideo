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
 * \file      ps.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2012
 */

#ifndef PARSER_MPEG_PS_H
#define PARSER_MPEG_PS_H

// minivideo headers
#include "../../../import.h"
#include "../../../bitstream.h"

/* ************************************************************************** */

/*!
 * \brief Parse a MPEG "Program Stream" file.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * This parser implements the 'MPEG Program Stream file format', from section 2.5
 * of MPEG-1/2 Part 1 specification 'Transmission multiplexing and synchronization'.
 *
 * (DVD version allows only 1 video and 8 audio streams)
 *
 * You can find this specification under different names:
 * - ISO/IEC 11172-1 (MPEG-1 Part 1).
 * - ISO/IEC 13818-1 (MPEG-1 Part 2).
 * - ITU-T H.222.0.
 */
int ps_fileParse(MediaFile_t *media);

/* ************************************************************************** */
#endif // PARSER_MPEG_PS_H
