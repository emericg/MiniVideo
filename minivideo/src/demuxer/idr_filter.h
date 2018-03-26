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
 * \file      idr_filter.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef IDR_FILTER_H
#define IDR_FILTER_H

// minivideo headers
#include "../minivideo_mediastream.h"

/* ************************************************************************** */

/*!
 * \brief Depending on picture_extractionmode parameter, choose some IDR from the MediaStream_t structure and delete the others.
 * \param stream_ptr: The address of the pointer to the MediaStream_t structure to initialize.
 * \param picture_number: The number of thumbnail(s) we want to extract.
 * \param picture_extraction_mode: The method of distribution for thumbnails extraction.
 * \return The number of picture available in the bitstream map (0 means error).
 *
 * \todo This needs work...
 *
 * The IDR filter aim to remove irrelevant frames from the decode stream. By irrelevant we mean:
 * - Unicolor images (black or green screen) like the very first or very last frames of a stream.
 * - Images carrying less visual informations than the average.
 * - If specified, the filter select images spread over the duration of the film.
 */
int idr_filtering(MediaStream_t **stream_ptr,
                  unsigned picture_number, const int picture_extraction_mode);

/* ************************************************************************** */
#endif // IDR_FILTER_H
