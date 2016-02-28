/*!
 * COPYRIGHT (C) 2010-2016 Emeric Grange - All Rights Reserved
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
 * \date      2016
 */

#ifndef MINIVIDEO_H
#define MINIVIDEO_H

// Needed in order to use this library from a C++ software without problems.
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "avcodecs.h"
#include "fourcc.h"
#include "mediafile_struct.h"

/* ************************************************************************** */

/*!
 * \brief Error codes returned to clients.
 */
typedef enum MiniVideoErrorCodes_e
{
    ERROR_UNKNOWN               = 1,

    ERROR_CONTAINER_UNKNOWN     = 10,
    ERROR_CONTAINER_FAILURE     = 11,

    ERROR_CODEC_UNKNOWN         = 20,
    ERROR_CODEC_FAILURE         = 21

} MiniVideoErrorCodes_e;

/* ************************************************************************** */

/*!
 * \brief Print informations about the library (version, enabled features, traces levels...) into standard output.
 */
void minivideo_infos(void);

/*!
 * \brief Determine endianness of the current system.
 * \return 4321 for big endian, 1234 for little endian, -1 if unable to determine endianness.
 *
 * To determine endianness, we use a character pointer to the bytes of an int,
 * and then check its first byte to see if it is 0 (meaning big endianness)
 * or 1 (meaning little endianness).
 */
int minivideo_endianness(void);

/*!
 * \brief Open a file and return a MediaFile_t context.
 * \param[in] *input_filepath: The file path of the video we want to open.
 * \param[out] **input_media: The MediaFile_t context to create.
 * \return TODO ERROR CODE (0 if file opening is a success, 1 otherwise).
 *
 * The first step in the decoding process is to open the file with the given fileptath.
 * If the file is successfully opened, the program start gathering informations about
 * the file, print them if in debug mode.
 */
int minivideo_open(const char *input_filepath, MediaFile_t **input_media);

/*!
 * \brief Parse a media file and fill the MediaFile_t context with the extracted infos.
 * \param *input_media: The MediaFile_t context to use.
 * \param extract_audio: Enable audio track parsing.
 * \param extract_video: Enable video track parsing.
 * \param extract_subtitles: Enable subtitles track parsing.
 * \return TODO ERROR CODE (0 if container parsing is a success, 1 otherwise).
 *
 * The second step in the decoding process is to parse the file's container infos
 * (if appropriate parser is available) and fill the MediaFile_t structure with
 * the tracks samples informations.
 */
int minivideo_parse(MediaFile_t *input_media,
                    const bool extract_audio, const bool extract_video, const bool extract_subtitles);

/*!
 * \brief Decode a video file and export thumbnails.
 * \param *input_media: The MediaFile_t context to use.
 * \param *output_directory: The directory where we want to save decoded thumbnail(s).
 * \param picture_format: The picture format for thumbnail(s) we want to extract.
 * \param picture_quality: The quality of thumbnail(s) we want to extract.
 * \param picture_number: The number of thumbnail(s) we want to extract.
 * \param picture_extractionmode: The method of distribution for thumbnails extraction.
 * \return TODO ERROR CODE (0 if picture(s) extraction is a success, 1 otherwise).
 *
 * Start decoding one or more picture(s) from the video (if an appropriate decoder
 * is available). Picture(s) is/are exported into selected file format.
 *
 * \todo Split this function into filtering /decoding / extraction stages.
 */
int minivideo_decode(MediaFile_t *input_media,
                     const char *output_directory,
                     const int picture_format,
                     const int picture_quality,
                     const int picture_number,
                     const int picture_extractionmode);

/*!
 * \brief Extract selected tracks from a video file and export in separated ES files.
 * \param *input_media: The MediaFile_t context to use.
 * \param *output_directory: The directory where we want to export track(s).
 * \param extract_audio: True to extract audio tracks.
 * \param extract_video: True to extract video tracks.
 * \param extract_subtitles: True to extract subtitles tracks.
 * \param output_format: Can be '1' for PES format or '2' for ES format.
 * \return TODO ERROR CODE (0 if picture(s) extraction is a success, 1 otherwise).
 */
int minivideo_extract(MediaFile_t *input_media,
                      const char *output_directory,
                      const bool extract_audio,
                      const bool extract_video,
                      const bool extract_subtitles,
                      const int output_format);

/*!
 * \brief Close a video file.
 * \param input_media: The MediaFile_t context to destroy.
 */
int minivideo_close(MediaFile_t **input_media);

/* ************************************************************************** */
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MINIVIDEO_H
