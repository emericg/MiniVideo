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
 * \file      wave.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef PARSER_WAVE_H
#define PARSER_WAVE_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

/*!
 * \brief Parse WAVE files.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * This WAVE parser supports:
 * - WAVE (from Microsoft "Multimedia Programming Interface and Data Specifications 1.0"
 *   as well as Microsoft "New Multimedia Data Types and Data Techniques")
 * - BWF (EBU Tech 3285 and ITU-R BS.1352)
 * - BWF64 (ITU-R BS.2088)
 * - RF64 (EBU Tech 3306)
 */
int wave_fileParse(MediaFile_t *media);

/* ************************************************************************** */
#endif // PARSER_WAVE_H
