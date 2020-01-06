/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2019
 */

#ifndef MINIVIDEO_UTILS_QT_H
#define MINIVIDEO_UTILS_QT_H
/* ************************************************************************** */

#include "minivideo_mediastream.h"

#include <cstdint>

#include <QString>
#include <QVector>
#include <QMetaType>

/* ************************************************************************** */

namespace Shared
{
    Q_NAMESPACE

    enum FileType
    {
        FILE_UNKNOWN    = 0,

        FILE_AUDIO      = 1,
        FILE_VIDEO      = 2,
        FILE_PICTURE    = 3,
    };
    Q_ENUM_NS(FileType)

    enum ShotType
    {
        SHOT_UNKNOWN = 0,

        SHOT_VIDEO = 8,
        SHOT_VIDEO_LOOPING,
        SHOT_VIDEO_TIMELAPSE,
        SHOT_VIDEO_NIGHTLAPSE,
        SHOT_VIDEO_3D,

        SHOT_PICTURE = 16,
        SHOT_PICTURE_MULTI,
        SHOT_PICTURE_BURST,
        SHOT_PICTURE_TIMELAPSE,
        SHOT_PICTURE_NIGHTLAPSE,

        SHOT_PICTURE_ANIMATED, // ex: gif?
    };
    Q_ENUM_NS(ShotType)
}

/* ************************************************************************** */

/*!
 * \brief Get a duration string from a duration in milliseconds.
 * \param duration: Duration in milliseconds.
 * \return Duration QString.
 *
 * Prints a duration in hours/min/sec/ms.
 */
QString getDurationString(const uint64_t duration);

/*!
 * \brief Get a timestamp string from a duration in milliseconds.
 * \param duration: Timestamp in microseconds.
 * \return Timestamp QString.
 *
 * Prints a 'precise' timestamp (ex: 1 h 2 m 3 s 40 ms 50 µs).
 */
QString getTimestampPreciseString(const uint64_t timestamp);

/*!
 * \brief Get a SMTPE timestamp string from a duration in milliseconds.
 * \param duration: Timestamp in microseconds.
 * \param framerate: Framerate of a video.
 * \return Timestamp QString.
 *
 * \note Right now this function only use the SMTPE format, but not the exact
 * computation used to produce a valid SMTPE timestamp.
 *
 * Prints a 'SMTPE' timestamp (ex: 01:02:03:32).
 * SMTPE timestamps are only valid for video streams.
 */
QString getTimestampSmtpeString(const uint64_t timestamp, const double framerate);

/*!
 * \brief Get a size string from a size in bytes.
 * \param size: File size in bytes.
 * \return File size QString.
 *
 * Prints size in (G/M/K)iB and (G/M/K)B and bytes.
 */
QString getSizeString(const int64_t size);

/*!
 * \brief Get a track type string from a track structure.
 * \param track: Track structure (from StreamType_e).
 * \return Track type QString.
 */
QString getTrackTypeString(const MediaStream_t *track);

/*!
 * \brief Compute a track size string from a track structure.
 * \param track: Track structure.
 * \param file_size: File size in bytes.
 * \param detailed: More precise sizes.
 * \return Track size QString.
 */
QString getTrackSizeString(const MediaStream_t *track, const int64_t file_size, const bool detailed = false);

/*!
 * \brief Get an aspect ratio string from a video definition.
 * \param x: Video width in pixel.
 * \param y: Video height in pixel.
 * \param detailed: Also print the name of the format.
 * \return QString with the aspect ratio.
 *
 * More infos: http://en.wikipedia.org/wiki/Aspect_ratio_(image)
 */
QString getAspectRatioString(const unsigned x, const unsigned y, const bool detailed = false);

/*!
 * \brief Get an aspect ratio string from a video aspect ratio.
 * \param ar: Video aspect ratio.
 * \param detailed: Also print the name of the format.
 * \return QString with the aspect ratio.
 *
 * More infos: http://en.wikipedia.org/wiki/Aspect_ratio_(image)
 */
QString getAspectRatioString(const double ar, const bool detailed = false);

/*!
 * \brief Get a bitrate string from a bitrate in bytes.
 * \param bitrate: Bitrate in bits.
 * \return Bitrate QString with the bitrate per second.
 *
 * Depending on the bitrate, the unit will be:
 * - Kb/s with 0 digits after the decimal point (if less than 10 Mb/s)
 * - Mb/s with 2 digits after the decimal point (if less than 100 Mb/s)
 * - Mb/s with 0 digits after the decimal point (if more than 100 Mb/s)
 */
QString getBitrateString(const unsigned bitrate);

QString getBitrateModeString(const BitrateMode_e bitrateMode);

QString getFramerateModeString(const FramerateMode_e framerateMode);

QString getProjectionString(const Projection_e projection);

QString getRotationString(const Rotation_e rotation);

QString getChannelModeString(const ChannelMode_e channelMode);

QString getStereoModeString(const StereoMode_e stereoMode);

QString getFourccString(const unsigned fourcc);

/*!
 * \brief Get a readable language string.
 * \param languageCode[in]: ISO 639-2 or ISO 639-3 language code.
 * \return Readable language (Qt translatable).
 *
 * https://en.wikipedia.org/wiki/ISO_639
 * http://www.loc.gov/standards/iso639-2/php/English_list.php
 */
QString getLanguageString(const char *languageCode);

/*!
 * \brief Get a sample type string.
 * \param sampleType: The sample type (from SampleType_e).
 * \return Sample type QString.
 */
QString getSampleTypeString(const unsigned sampleType);

/* ************************************************************************** */

/*!
 * \brief The bitrateMinMax class
 *
 * Used to compute minimal and maximal bitrate on a one second sliding window.
 */
class bitrateMinMax
{
    uint32_t m_fps = 24;
    uint32_t m_sampleCounter = 0;
    QVector<uint32_t> m_samplesData;

    uint32_t m_bitrateMin = UINT32_MAX;
    uint32_t m_bitrateMax = 0;

public:
    bitrateMinMax(const double fps);
    bitrateMinMax(const uint32_t fps);
    virtual ~bitrateMinMax();

    uint32_t pushSampleSize(const uint32_t sampleSize);
    void getMinMax(uint32_t &min, uint32_t &max);
};

/* ************************************************************************** */
#endif // MINIVIDEO_UTILS_QT_H
