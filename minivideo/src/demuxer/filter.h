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
 * \file      filter.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSING_H
#define PARSING_H

// minivideo headers
#include "../import.h"

/* ************************************************************************** */

//! Picture repartition mode
typedef enum PictureRepartition_e
{
    PICTURE_UNFILTERED  = 0,
    PICTURE_ORDERED     = 1,
    PICTURE_DISTRIBUTED = 2

} PictureRepartition_e;

/* ************************************************************************** */

int idr_filtering(BitstreamMap_t **bitstream_map_pointer,
                  int picture_number, const int picture_extractionmode);

/* ************************************************************************** */
#endif /* PARSING_H */
