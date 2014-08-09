/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      import.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef IMPORT_H
#define IMPORT_H

// C standard library
#include <stdio.h>

// minivideo headers
#include "videofile_struct.h"

/* ************************************************************************** */

int import_fileOpen(const char *filepath, VideoFile_t **video_ptr);

int import_fileClose(VideoFile_t **video_ptr);

void import_fileStatus(VideoFile_t *videoFile);

/* ************************************************************************** */
#endif /* IMPORT_H */
