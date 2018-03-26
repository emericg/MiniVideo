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
 * \file      mp4.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_MP4_H
#define PARSER_MP4_H

// minivideo headers
#include "../../import.h"

/* ************************************************************************** */

/*!
 * \brief Parse mp4/mov files.
 * \param *media[in,out]: A pointer to a MediaFile_t structure.
 * \return 1 if succeed, 0 otherwise.
 *
 * This parser is based on the 'ISO/IEC 14496-12' international standard, part 12:
 * 'ISO base media file format'.
 * Fully compatible with the 'QuickTime File Format Specification' from Apple,
 * available at https://developer.apple.com/library/mac/documentation/QuickTime/QTFF/QTFFPreface/qtffPreface.html
 *
 * \todo Stop parsing if we are after the end of the 'moov' box.
 */
int mp4_fileParse(MediaFile_t *media);

/* ************************************************************************** */
#endif // PARSER_MP4_H
