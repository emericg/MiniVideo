/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      xml_mapper.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef XML_MAPPER_H
#define XML_MAPPER_H

// minivideo headers
#include "../import.h"

/* ************************************************************************** */

/*!
 * \brief Open the xmlMapper.
 * \param *media[in]: A pointer to a MediaFile_t structure.
 * \param **xml[in,out]: Mapping file.
 */
int xmlMapperOpen(MediaFile_t *media, FILE **xml);

/*!
 * \brief Close the xmlMapper.
 * \param **xml[in]: Mapping file.
 */
int xmlMapperClose(FILE **xml);

/* ************************************************************************** */
#endif // XML_MAPPER_H
