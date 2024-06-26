/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef DEPACKETIZER_STRUCT_H
#define DEPACKETIZER_STRUCT_H
/* ************************************************************************** */

#include <cstdint>
#include <string>

/* ************************************************************************** */

typedef struct es_sample_t
{
    int64_t offset;
    uint32_t size;

    uint32_t type;
    const char *type_cstr;
    //std::string type_str;

} es_sample_t;

/* ************************************************************************** */
#endif // DEPACKETIZER_STRUCT_H
