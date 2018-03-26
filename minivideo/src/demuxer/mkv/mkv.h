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
 * \file      mkv.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_MKV_H
#define PARSER_MKV_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

/*!
 * \brief Parse MKV files.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * This parser is based on the official MKV specification.
 * - http://www.matroska.org/technical/specs/index.html
 * - http://www.matroska.org/files/matroska.pdf
 */
int mkv_fileParse(MediaFile_t *media);

/* ************************************************************************** */
#endif // PARSER_MKV_H
