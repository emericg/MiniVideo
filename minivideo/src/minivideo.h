/*!
 * COPYRIGHT (C) 2010-2020 Emeric Grange - All Rights Reserved
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
 * \date      2016
 */

#ifndef MINIVIDEO_H
#define MINIVIDEO_H
/* ************************************************************************** */

#include "minivideo_export.h"
#include "minivideo_codecs.h"
#include "minivideo_fourcc.h"
#include "minivideo_mediafile.h"

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
 * \brief Print information about the library (version, date of the build, ...) into standard output.
 */
minivideo_EXPORT void minivideo_print_infos(void);

/*!
 * \brief Print information about the library (enabled features, traces levels, ...) into standard output.
 */
minivideo_EXPORT void minivideo_print_features(void);

/*!
 * \brief Get information about the library (version and build date/time).
 *
 * The date and time strings are static data and do not need to be freed.
 */
minivideo_EXPORT void minivideo_get_infos(int *minivideo_major,
                                          int *minivideo_minor,
                                          int *minivideo_patch,
                                          const char **minivideo_builddate,
                                          const char **minivideo_buildtime,
                                          bool *minivideo_builddebug);

/*!
 * \brief Print endianness of the current system.
 * \return 4321 for big endian, 1234 for little endian, -1 if unable to determine endianness.
 *
 * To determine endianness, we use a character pointer to the bytes of an int,
 * and then check its first byte to see if it is 0 (meaning big endianness)
 * or 1 (meaning little endianness).
 */
minivideo_EXPORT int minivideo_endianness(void);

/* ************************************************************************** */

/*!
 * \brief Open a file and return a MediaFile_t context.
 * \param[in] *input_filepath: The file path of the video we want to open.
 * \param[in,out] **input_media: The MediaFile_t context to create.
 * \return TODO ERROR CODE (1 if file opening is a success, 0 otherwise).
 *
 * The first step in the decoding process is to open the file with the given fileptath.
 * If the file is successfully opened, the program start gathering information about
 * the file, print them if in debug mode.
 */
minivideo_EXPORT int minivideo_open(const char *input_filepath,
                                    MediaFile_t **input_media);

/*!
 * \brief Parse a media file and fill the MediaFile_t context with the extracted infos.
 * \param[in] *input_media: The MediaFile_t context to use.
 * \param compute_metadata: Enable extra metadata computations.
 * \param container_mapping: Enable container mapping to xml file.
 * \return TODO ERROR CODE (1 if container parsing is a success, 0 otherwise).
 *
 * The second step in the decoding process is to parse the file's container infos
 * (if appropriate parser is available) and fill the MediaFile_t structure with
 * the tracks samples information.
 */
minivideo_EXPORT int minivideo_parse(MediaFile_t *input_media,
                                     const bool compute_metadata,
                                     const bool container_mapping);

/*!
 * \brief Thumnbail a video file and export thumbnails.
 * \param *input_media[in]: The MediaFile_t context to use.
 * \param *output_directory: The directory where we want to save decoded thumbnail(s).
 * \param picture_format: The picture format for thumbnail(s) we want to extract.
 * \param picture_quality: The quality of thumbnail(s) we want to extract.
 * \param picture_count: The number of thumbnail(s) we want to extract.
 * \param picture_extractionmode: The method of distribution for thumbnails extraction.
 * \return TODO ERROR CODE (1 if picture(s) extraction is a success, 0 otherwise).
 *
 * \todo Split minivideo_thumbnail() into filtering /decoding / extraction stages.
 *
 * Start decoding one or more picture(s) from the video (if an appropriate decoder
 * is available). Pictures are exported into selected file format.
 */
minivideo_EXPORT int minivideo_thumbnail(MediaFile_t *input_media,
                                         const char *output_directory,
                                         const int picture_format,
                                         const int picture_quality,
                                         const int picture_count,
                                         const int picture_extractionmode);

/*!
 * \brief Extract selected tracks from a video file and export in separated ES files.
 * \param *input_media[in]: The MediaFile_t context to use.
 * \param *output_directory: The directory where we want to export track(s).
 * \param extract_audio: True to extract audio tracks.
 * \param extract_video: True to extract video tracks.
 * \param extract_subtitles: True to extract subtitles tracks.
 * \return TODO ERROR CODE (1 if picture(s) extraction is a success, 0 otherwise).
 */
minivideo_EXPORT int minivideo_extract(MediaFile_t *input_media,
                                       const char *output_directory,
                                       const bool extract_audio,
                                       const bool extract_video,
                                       const bool extract_subtitles);

/*!
 * \brief Close a video file.
 * \param input_media[in,out]: The MediaFile_t context to destroy.
 * \return 1 if success, 0 otherwise.
 */
minivideo_EXPORT int minivideo_close(MediaFile_t **input_media);

/* ************************************************************************** */

/*!
 * \brief Decode a video frame.
 * \param *input_media[in]: The MediaFile_t context to use.
 * \param frame_id: Frame ID.
 * \return An allocated OutputSurface_t.
 *
 * MediaFile_t must be opened and parsed.
 * Allocate an OutputSurface_t, which must be destroyed by minivideo_destroy_frame().
 */
minivideo_EXPORT OutputSurface_t *minivideo_decode_frame(MediaFile_t *input_media,
                                                         const unsigned frame_id);

/*!
 * \brief Destroy a frame allocated by minivideo_decode_frame().
 * \param frame_ptr: Address of an OutputSurface_t pointer.
 */
minivideo_EXPORT void minivideo_destroy_frame(OutputSurface_t **frame_ptr);

/*!
 * \brief Extract a sample.
 * \param *input_media[in]: The MediaFile_t context to use.
 * \param *input_stream[in]: The MediaStream_t context to use.
 * \param sample_id: Sample ID.
 * \return An allocated MediaSample_t.
 *
 * MediaFile_t must be opened and parsed.
 * Allocate a MediaSample_t, which must be destroyed by minivideo_destroy_sample().
 */
minivideo_EXPORT MediaSample_t *minivideo_get_sample(MediaFile_t *input_media,
                                                     MediaStream_t *input_stream,
                                                     const unsigned sample_id);
/*!
 * \brief Destroy a sample allocated by minivideo_get_sample().
 * \param sample_ptr:  Address of a MediaSample_t pointer.
 */
minivideo_EXPORT void minivideo_destroy_sample(MediaSample_t **sample_ptr);

/* ************************************************************************** */
#endif // MINIVIDEO_H
