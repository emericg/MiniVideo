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
 * \file      minivideo_typedef.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef MINIVIDEO_TYPEDEF_H
#define MINIVIDEO_TYPEDEF_H

// Contains some settings generated by CMake
#include "minivideo_settings.h"

// CLI color output
#include "colors.h"

/* ************************************************************************** */
// Custom return codes

#define UNSUPPORTED -1
#define FAILURE      0
#define SUCCESS      1

/* ************************************************************************** */
// Windows large file support

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
#if defined(_MSC_VER) || defined(__MINGW32__)
    #include <cstdio>

    #undef stat64
    #define stat64 _stat64

    #undef fseek
    #define fseek(x, y, z) _fseeki64(x, y, z)

    #undef ftell
    #define ftell(x)       _ftelli64(x)
#endif // defined(_MSC_VER) || defined(__MINGW32__)
#endif // defined(_WIN16) || defined(_WIN32) || defined(_WIN64)

/* ************************************************************************** */
#endif // MINIVIDEO_TYPEDEF_H
