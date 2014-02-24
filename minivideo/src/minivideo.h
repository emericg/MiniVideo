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
 * \file      minivideo.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef MINIVIDEO_H
#define MINIVIDEO_H

// Needed in order to use this library from a C++ software without problems.
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ************************************************************************** */

void minivideo_infos(void);

int minivideo_endianness(void);

int minivideo_thumbnailer(const char *input_filepath,
                          const char *output_directory,
                          const int picture_format,
                          const int picture_quality,
                          const int picture_number,
                          const int picture_extractionmode);

int minivideo_extractor(const char *input_filepath,
                        const char *output_directory,
                        const bool extract_audio,
                        const bool extract_video,
                        const bool extract_subtitles,
                        const int output_format);

/* ************************************************************************** */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* ************************************************************************** */
#endif /* MINIVIDEO_H */
