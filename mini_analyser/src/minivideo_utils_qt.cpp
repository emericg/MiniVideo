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

#include "minivideo_utils_qt.h"
#include "minivideo_fourcc.h"

#include <cstdint>
#include <cstring>
#include <cmath>

#include <QDebug>

/* ************************************************************************** */

QString getDurationString(const uint64_t duration)
{
    QString duration_qstr;

    if (duration > 0)
    {
        uint64_t hours = duration / 3600000;
        uint64_t minutes = (duration - (hours * 3600000)) / 60000;
        uint64_t seconds = (duration - (hours * 3600000) - (minutes * 60000)) / 1000;
        uint64_t ms = (duration - (hours * 3600000) - (minutes * 60000)) - (seconds * 1000);

        if (hours > 0)
        {
            duration_qstr += QString::number(hours);

            if (hours > 1)
                duration_qstr += QObject::tr(" hours ");
            else
                duration_qstr += QObject::tr(" hour ");
        }
        if (minutes > 0)
        {
            duration_qstr += QString::number(minutes) + QObject::tr(" min ");
        }
        if (seconds > 0)
        {
            duration_qstr += QString::number(seconds) + QObject::tr(" sec ");
        }
        if (ms > 0)
        {
            duration_qstr += QString::number(ms) + QObject::tr(" ms");
        }
    }
    else
    {
        duration_qstr = QObject::tr("NULL duration");
    }

    //qDebug() << "getDurationString(" << duration << ") >" << duration_qstr;

    return duration_qstr;
}

QString getTimestampPreciseString(const uint64_t timestamp)
{
    QString timestamp_qstr;

    if (timestamp == 0)
    {
        timestamp_qstr = "0 µs";
    }
    else
    {
        uint64_t hours = timestamp / 3600000000;
        uint64_t minutes = (timestamp - (hours * 3600000000)) / 60000000;
        uint64_t seconds = (timestamp - (hours * 3600000000) - (minutes * 60000000)) / 1000000;
        uint64_t ms = (timestamp - (hours * 3600000000) - (minutes * 60000000) - (seconds * 1000000)) / 1000;
        uint64_t us = (timestamp - (hours * 3600000000) - (minutes * 60000000) - (seconds * 1000000) - (ms * 1000));

        if (hours > 0)
        {
            timestamp_qstr += QString::number(hours) + " h";
        }
        if (minutes > 0)
        {
            if (hours > 0) timestamp_qstr += " ";
            timestamp_qstr += QString::number(minutes) + " m";
        }
        if (seconds > 0)
        {
            if (minutes > 0) timestamp_qstr += " ";
            timestamp_qstr += QString::number(seconds) + " s";
        }
        if (ms > 0)
        {
            if (seconds > 0) timestamp_qstr += " ";
            timestamp_qstr += QString::number(ms) + " ms";
        }
        if (us > 0)
        {
            if (ms > 0) timestamp_qstr += " ";
            timestamp_qstr += QString::number(us) + " µs";
        }
    }

    //qDebug() << "getTimestampPreciseString(" << timestamp << ") >" << timestamp_qstr;

    return timestamp_qstr;
}

QString getTimestampSmtpeString(const uint64_t timestamp, const double framerate)
{
    QString timestamp_qstr;

    if (timestamp == 0)
    {
        timestamp_qstr = "00:00:00-000";
    }
    else
    {
        uint64_t hours = timestamp / 3600000000;
        uint64_t minutes = (timestamp - (hours * 3600000000)) / 60000000;
        uint64_t seconds = (timestamp - (hours * 3600000000) - (minutes * 60000000)) / 1000000;

        double us = (timestamp - (hours * 3600000000) - (minutes * 60000000) - (seconds * 1000000));
        uint64_t frames = static_cast<uint64_t>(std::floor(us / std::floor(1000000.0 / framerate)));

        timestamp_qstr = QString::number(hours) + ":" + QString::number(minutes) + ":" + QString::number(seconds) + "-" + QString::number(frames);
        timestamp_qstr = QString("%1:%2:%3-%4")\
                            .arg(hours, 2, 10, QChar('0'))\
                            .arg(minutes, 2, 10, QChar('0'))\
                            .arg(seconds, 2, 10, QChar('0'))\
                            .arg(frames, 3, 10, QChar('0'));
    }

    //qDebug() << "getTimestampSmtpeString(" << timestamp << ") >" << timestamp_qstr;

    return timestamp_qstr;
}

/* ************************************************************************** */

QString getSizeString(const int64_t size)
{
    QString size_qstr;

    if (size > 0)
    {
        if (size < 1024) // < 1 KiB
        {
            size_qstr = QString::number(size) + " bytes";
        }
        else if (size < 1048576) // < 1 MiB
        {
            size_qstr = QString::number(size / 1000.0, 'f', 2) + " KB  /  "
                      + QString::number(size / 1024.0, 'f', 2) + " KiB  ("
                      + QString::number(size) + " bytes)";
        }
        else if (size < 1073741824) // < 1 GiB
        {
            size_qstr = QString::number(size / 1000.0 / 1000.0, 'f', 2) + " MB  /  "
                      + QString::number(size / 1024.0 / 1024.0, 'f', 2) + " MiB  ("
                      + QString::number(size) + " bytes)";
        }
        else // < 1 GiB
        {
            size_qstr = QString::number(size / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB  /  "
                      + QString::number(size / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB  ("
                      + QString::number(size) + " bytes)";
        }
    }
    else
    {
        size_qstr = QObject::tr("NULL size");
    }

    //qDebug() << "getDurationString(" << size << ") >" << size_qstr;

    return size_qstr;
}

QString getTrackSizeString(const MediaStream_t *track, const int64_t file_size, const bool detailed)
{
    QString size_qstr;

    if (track)
    {
        quint64 size_int = track->stream_size;

        if (track->stream_size > 0)
        {
            if (track->stream_size < 1024) // < 1 KiB
            {
                size_qstr = QString::number(track->stream_size) + " bytes";
            }
            else if (track->stream_size < 1048576) // < 1 MiB
            {
                if (detailed)
                {
                    size_qstr = QString::number(size_int / 1000.0, 'f', 2) + " KB  /  "
                              + QString::number(size_int / 1024.0, 'f', 2) + " KiB  ("
                              + QString::number(size_int) + " bytes)";
                }
                else
                {
                    size_qstr = QString::number(size_int / 1000.0, 'f', 2) + " KB";
                }
            }
            else if (track->stream_size < 1073741824) // < 1 GiB
            {
                if (detailed)
                {
                    size_qstr = QString::number(size_int / 1000.0 / 1000.0, 'f', 2) + " MB  /  "
                              + QString::number(size_int / 1024.0 / 1024.0, 'f', 2) + " MiB  ("
                              + QString::number(size_int) + " bytes)";
                }
                else
                {
                    size_qstr = QString::number(size_int / 1000.0 / 1000.0, 'f', 2) + " MB";
                }
            }
            else // > 1 GiB
            {
                if (detailed)
                {
                    size_qstr = QString::number(size_int / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB  /  "
                              + QString::number(size_int / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB  ("
                              + QString::number(size_int) + " bytes)";
                }
                else
                {
                    size_qstr = QString::number(size_int / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB";
                }
            }

            // Percentage
            double sizepercent = (static_cast<double>(size_int) / static_cast<double>(file_size)) * 100.0;

            if (sizepercent > 100)
                size_qstr += " (ERR)";
            else if (sizepercent < 0.01)
                size_qstr += " (~0.01%)";
            else
                size_qstr += " (" + QString::number(sizepercent, 'g', 3) + " %)";
        }
        else
        {
            size_qstr += "0 bytes";
        }

        //qDebug() << "getTrackSizeString(" << size_int << ") >" << size_qstr;
    }
    else
    {
        size_qstr = QObject::tr("NULL track size");
    }

    return size_qstr;
}

/* ************************************************************************** */

QString getTrackTypeString(const MediaStream_t *track)
{
    QString type_qstr;

    if (track)
    {
        switch (track->stream_type)
        {
        case stream_UNKNOWN:
            type_qstr = QObject::tr("Unknown");
            break;

        case stream_AUDIO:
            type_qstr = QObject::tr("Audio");
            break;
        case stream_VIDEO:
            type_qstr = QObject::tr("Video");
            break;
        case stream_TEXT:
            type_qstr = QObject::tr("Text");
            break;

        case stream_MENU:
            type_qstr = QObject::tr("Menu");
            break;
        case stream_TMCD:
            type_qstr = QObject::tr("TimeCode Record");
            break;
        case stream_META:
            type_qstr = QObject::tr("Metadata");
            break;
        case stream_HINT:
            type_qstr = QObject::tr("Hint");
            break;
        }
    }
    else
    {
        type_qstr = QObject::tr("NULL track");
    }

    return type_qstr;
}

QString getSampleTypeString(const unsigned sampleType)
{
    QString sample_type_qstr;

    if (sampleType == sample_AUDIO)
        sample_type_qstr = QObject::tr("Audio sample");
    else if (sampleType == sample_AUDIO_TAG)
        sample_type_qstr = QObject::tr("Audio tag");
    else if (sampleType == sample_VIDEO)
        sample_type_qstr = QObject::tr("Video sample");
    else if (sampleType == sample_VIDEO_SYNC)
        sample_type_qstr = QObject::tr("Video sync sample");
    else if (sampleType == sample_VIDEO_PARAM)
        sample_type_qstr = QObject::tr("Video parameter");
    else if (sampleType == sample_TEXT)
        sample_type_qstr = QObject::tr("Text sample");
    else if (sampleType == sample_TEXT_FILE)
        sample_type_qstr = QObject::tr("Text file");
    else if (sampleType == sample_RAW_DATA)
        sample_type_qstr = QObject::tr("RAW datas");
    else if (sampleType == sample_TMCD)
        sample_type_qstr = QObject::tr("TimeCode Reference");
    else if (sampleType == sample_OTHER)
        sample_type_qstr = QObject::tr("Other sample");
    else
        sample_type_qstr = QObject::tr("Unknown sample type");

    return sample_type_qstr;
}

QString getAspectRatioString(const unsigned x, const unsigned y, const bool detailed)
{
    double ar_d = static_cast<double>(x) / static_cast<double>(y);
    return getAspectRatioString(ar_d, detailed);
}

QString getAspectRatioString(double ar_d, const bool detailed)
{
    QString aspectratio_qstr;

    if (ar_d > 1.24 && ar_d < 1.26)
    {
        aspectratio_qstr = "5:4";
    }
    else if (ar_d > 1.323 && ar_d < 1.343)
    {
        aspectratio_qstr = "4:3";
    }
    else if (ar_d > 1.42 && ar_d < 1.44)
    {
        aspectratio_qstr = "1.43:1";

        if (detailed)
            aspectratio_qstr += " (IMAX)";
    }
    else if (ar_d > 1.49 && ar_d < 1.51)
    {
        aspectratio_qstr = "3:2";
    }
    else if (ar_d > 1.545 && ar_d < 1.565)
    {
        aspectratio_qstr = "14:9";
    }
    else if (ar_d > 1.59 && ar_d < 1.61)
    {
        aspectratio_qstr = "16:10";
    }
    else if (ar_d > 1.656 && ar_d < 1.676)
    {
        aspectratio_qstr = "5:3";

        if (detailed)
            aspectratio_qstr += " (35 mm film)";
    }
    else if (ar_d > 1.767 && ar_d < 1.787)
    {
        aspectratio_qstr = "16:9";
    }
    else if (ar_d > 1.84 && ar_d < 1.86)
    {
        aspectratio_qstr = "1.85:1";

        if (detailed)
            aspectratio_qstr += " (US / UK widescreen)";
    }
    else if (ar_d > 1.886 && ar_d < 1.906)
    {
        aspectratio_qstr = "1.896:1";

        if (detailed)
            aspectratio_qstr += " (DCI / SMPTE digital cinema)";
    }
    else if (ar_d > 1.99 && ar_d < 2.01)
    {
        aspectratio_qstr = "2.0:1";

        if (detailed)
            aspectratio_qstr += " (SuperScope / Univisium)";
    }
    else if (ar_d > 2.19 && ar_d < 2.22)
    {
        aspectratio_qstr = "2.20:1";

        if (detailed)
            aspectratio_qstr += " (70 mm film)";
    }
    else if (ar_d > 2.34 && ar_d < 2.36)
    {
        aspectratio_qstr = "2.35:1";

        if (detailed)
            aspectratio_qstr += " (35 mm anamorphic)";
    }
    else if (ar_d > 2.38 && ar_d < 2.40)
    {
        aspectratio_qstr = "2.39:1";

        if (detailed)
            aspectratio_qstr += " (35 mm modern anamorphic)";
    }
    else if (ar_d > 2.54 && ar_d < 2.56)
    {
        aspectratio_qstr = "2.55:1";

        if (detailed)
            aspectratio_qstr += " (Cinemascope)";
    }
    else if (ar_d > 2.75 && ar_d < 2.77)
    {
        aspectratio_qstr = "2.76:1";

        if (detailed)
            aspectratio_qstr += " (Ultra Panavision 70)";
    }
    else
    {
        aspectratio_qstr = QString::number(ar_d, 'g', 4) + ":1";
    }

    //qDebug() << "getAspectRatioString(" << x << "," << y << ") >" << aspectratio_qstr;

    return aspectratio_qstr;
}

QString getBitrateString(const unsigned bitrate)
{
    QString bitrate_qstr;

    if (bitrate > 0)
    {
        if (bitrate < 10000000) // < 10 Mb
        {
            bitrate_qstr = QString::number(bitrate / 1000.0, 'g', 5) + " Kb/s";
        }
        else if (bitrate < 100000000) // < 100 Mb
        {
            bitrate_qstr = QString::number(bitrate / 1000.0 / 1000.0, 'f', 2) + " Mb/s";
        }
        else
        {
            bitrate_qstr = QString::number(bitrate / 1000.0 / 1000.0, 'f', 0) + " Mb/s";
        }
    }
    else
    {
        bitrate_qstr = QObject::tr("NULL bitrate");
    }

    //qDebug() << "getBitrateString(" << bitrate << ") >" << bitrate_qstr;

    return bitrate_qstr;
}

QString getBitrateModeString(const BitrateMode_e bitrateMode)
{
    QString bitrate_mode_qstr;

    if (bitrateMode == BITRATE_CBR)
        bitrate_mode_qstr = "CBR";
    else if (bitrateMode == BITRATE_VBR)
        bitrate_mode_qstr = "VBR";
    else if (bitrateMode == BITRATE_ABR)
        bitrate_mode_qstr = "ABR";
    else if (bitrateMode == BITRATE_CVBR)
        bitrate_mode_qstr = "CVBR";

    return bitrate_mode_qstr;
}

QString getFramerateModeString(const FramerateMode_e framerateMode)
{
    QString framerate_mode_qstr;

    if (framerateMode == FRAMERATE_CFR)
        framerate_mode_qstr = "CFR";
    else if (framerateMode == FRAMERATE_VFR)
        framerate_mode_qstr = "VFR";

    return framerate_mode_qstr;
}

QString getProjectionString(const Projection_e projection)
{
    QString projection_qstr;

    if (projection == PROJECTION_RECTANGULAR)
        projection_qstr = QObject::tr("Rectangular");
    else if (projection == PROJECTION_EQUIRECTANGULAR)
        projection_qstr = QObject::tr("Equirectangular");
    else if (projection == PROJECTION_EAC)
        projection_qstr = QObject::tr("Equi-Angular Cubemap(EAC)");
    else if (projection == PROJECTION_CUBEMAP_A)
        projection_qstr = QObject::tr("Cubemap (3x2: right face - left face - up face then down face - front face - back face)");
    else if (projection == PROJECTION_MESH)
        projection_qstr = QObject::tr("3D Mesh");
    else
        projection_qstr = QObject::tr("Rectangular"); // pretty safe to assumption...

    return projection_qstr;
}

QString getRotationString(const Rotation_e rotation)
{
    QString rotation_qstr;

    if (rotation == ROTATION_0)
        rotation_qstr = QObject::tr("No rotation");
    else if (rotation == ROTATION_90)
        rotation_qstr = QObject::tr("90°");
    else if (rotation == ROTATION_180)
        rotation_qstr = QObject::tr("180°");
    else if (rotation == ROTATION_270)
        rotation_qstr = QObject::tr("270°");
    else
        rotation_qstr = QObject::tr("Unknown");

    return rotation_qstr;
}

QString getChannelModeString(const ChannelMode_e channelMode)
{
    QString channel_mode_qstr;

    if (channelMode == CHANS_MONO)
        channel_mode_qstr = QObject::tr("Mono");
    else if (channelMode == CHANS_STEREO)
        channel_mode_qstr = QObject::tr("Stereo");
    else if (channelMode == CHANS_QUAD)
        channel_mode_qstr = QObject::tr("Quadraphonic");
    else if (channelMode == CHANS_SURROUND_21)
        channel_mode_qstr = "2.1";
    else if (channelMode == CHANS_SURROUND_31)
        channel_mode_qstr = "3.1";
    else if (channelMode == CHANS_SURROUND_41)
        channel_mode_qstr = "4.1";
    else if (channelMode == CHANS_SURROUND_51)
        channel_mode_qstr = "5.1";
    else if (channelMode == CHANS_SURROUND_71)
        channel_mode_qstr = "7.1";
    else if (channelMode == CHANS_SURROUND_91)
        channel_mode_qstr = "9.1";
    else if (channelMode == CHANS_SURROUND_111)
        channel_mode_qstr = "11.1";
    else if (channelMode == CHANS_SURROUND_102)
        channel_mode_qstr = "10.2";
    else if (channelMode == CHANS_SURROUND_222)
        channel_mode_qstr = "22.2";
    else if (channelMode == CHANS_AMBISONIC_FOA)
        channel_mode_qstr = QObject::tr("Ambisonic FOA");
    else if (channelMode == CHANS_AMBISONIC_SOA)
        channel_mode_qstr = QObject::tr("Ambisonic SOA");
    else if (channelMode == CHANS_AMBISONIC_TOA)
        channel_mode_qstr = QObject::tr("Ambisonic TOA");
    else
        channel_mode_qstr = QObject::tr("Unknown");

    return channel_mode_qstr;
}

QString getStereoModeString(const StereoMode_e stereoMode)
{
    QString channel_mode_qstr;

    if (stereoMode == MONOSCOPIC)
        channel_mode_qstr = QObject::tr("Monoscopic");
    else if (stereoMode == STEREO_ANAGLYPH_CR)
        channel_mode_qstr = QObject::tr("Anaglyph (cyan/red)");
    else if (stereoMode == STEREO_ANAGLYPH_GM)
        channel_mode_qstr = QObject::tr("Anaglyph (green/magenta)");
    else if (stereoMode == STEREO_SIDEBYSIDE_LEFT)
        channel_mode_qstr = QObject::tr("Side by side (left eye is first)");
    else if (stereoMode == STEREO_SIDEBYSIDE_RIGHT)
        channel_mode_qstr = QObject::tr("Side by side (righ eye is first)");
    else if (stereoMode == STEREO_TOPBOTTOM_LEFT)
        channel_mode_qstr = QObject::tr("Top-bottom (left eye is first)");
    else if (stereoMode == STEREO_TOPBOTTOM_RIGHT)
        channel_mode_qstr = QObject::tr("Top-bottom (righ eye is first)");
    else if (stereoMode == STEREO_CHECKBOARD_LEFT)
        channel_mode_qstr = QObject::tr("Checkboard (left eye is first)");
    else if (stereoMode == STEREO_CHECKBOARD_RIGHT)
        channel_mode_qstr = QObject::tr("Checkboard (righ eye is first)");
    else if (stereoMode == STEREO_ROWINTERLEAVED_LEFT)
        channel_mode_qstr = QObject::tr("Row interleaved (left eye is first)");
    else if (stereoMode == STEREO_ROWINTERLEAVED_RIGHT)
        channel_mode_qstr = QObject::tr("Row interleaved (righ eye is first)");
    else if (stereoMode == STEREO_COLUMNINTERLEAVED_LEFT)
        channel_mode_qstr = QObject::tr("Column interleaved (left eye is first)");
    else if (stereoMode == STEREO_COLUMNINTERLEAVED_RIGHT)
        channel_mode_qstr = QObject::tr("Column interleaved (righ eye is first)");
    else
        channel_mode_qstr = QObject::tr("Unknown");

    return channel_mode_qstr;
}

/* ************************************************************************** */

QString getFourccString(const unsigned fourcc)
{
    char fcc_str[5];
    return QString::fromUtf8(getFccString_le(fourcc, fcc_str));
}

/* ************************************************************************** */

QString getLanguageString(const char *languageCode)
{
    QString langage_qstr;

    if (languageCode)
    {
        size_t lng_size = strlen(languageCode);
        if (lng_size > 3) { lng_size = 3; }

        if (strncmp(languageCode, "und", lng_size) == 0)
            langage_qstr = "";
        else if (strncmp(languageCode, "```", lng_size) == 0)
            langage_qstr = "";
        else if (strncmp(languageCode, "mis", lng_size) == 0)
            langage_qstr = "mis"; // the language has no code
        else if (strncmp(languageCode, "mul", lng_size) == 0)
            langage_qstr = QObject::tr("Multilingual content");

        else if (strncmp(languageCode, "chi", lng_size) == 0 ||
                 strncmp(languageCode, "zho", lng_size) == 0 ||
                 strncmp(languageCode, "zh", lng_size) == 0)
            langage_qstr = QObject::tr("Chinese");
        else if (strncmp(languageCode, "cnm", lng_size) == 0)
            langage_qstr = QObject::tr("Mandarin");
        else if (strncmp(languageCode, "es", lng_size) == 0||
                 strncmp(languageCode, "spa", lng_size) == 0)
            langage_qstr = QObject::tr("Spanish");
        else if (strncmp(languageCode, "eng", lng_size) == 0)
            langage_qstr = QObject::tr("English");
        else if (strncmp(languageCode, "hi", lng_size) == 0 ||
                 strncmp(languageCode, "hin", lng_size) == 0)
            langage_qstr = QObject::tr("Hindi");
        else if (strncmp(languageCode, "ar", lng_size) == 0 ||
                 strncmp(languageCode, "ara", lng_size) == 0)
            langage_qstr = QObject::tr("Arabic");
        else if (strncmp(languageCode, "pt", lng_size) == 0 ||
                 strncmp(languageCode, "pro", lng_size) == 0 ||
                 strncmp(languageCode, "por", lng_size) == 0)
            langage_qstr = QObject::tr("Portuguese");
        else if (strncmp(languageCode, "bn", lng_size) == 0 ||
                 strncmp(languageCode, "ben", lng_size) == 0)
            langage_qstr = QObject::tr("Bengali");
        else if (strncmp(languageCode, "ru", lng_size) == 0 ||
                 strncmp(languageCode, "rus", lng_size) == 0)
            langage_qstr = QObject::tr("Russian");
        else if (strncmp(languageCode, "ja", lng_size) == 0 ||
                 strncmp(languageCode, "jpn", lng_size) == 0)
            langage_qstr = QObject::tr("Japanese");
        else if (strncmp(languageCode, "pa", lng_size) == 0 ||
                 strncmp(languageCode, "pan", lng_size) == 0)
            langage_qstr = QObject::tr("Punjabi");
        else if (strncmp(languageCode, "de", lng_size) == 0 ||
                 strncmp(languageCode, "ger", lng_size) == 0 ||
                 strncmp(languageCode, "deu", lng_size) == 0)
            langage_qstr = QObject::tr("German");
        else if (strncmp(languageCode, "jv", lng_size) == 0 ||
                 strncmp(languageCode, "jav", lng_size) == 0)
            langage_qstr = QObject::tr("Javanese");
        else if (strncmp(languageCode, "ta", lng_size) == 0 ||
                 strncmp(languageCode, "tam", lng_size) == 0)
            langage_qstr = QObject::tr("Tamil");
        else if (strncmp(languageCode, "ur", lng_size) == 0 ||
                 strncmp(languageCode, "urd", lng_size) == 0)
            langage_qstr = QObject::tr("Urdu");
        else if (strncmp(languageCode, "wuu", lng_size) == 0)
            langage_qstr = QObject::tr("Wu Chinese");
        else if (strncmp(languageCode, "ms", lng_size) == 0 ||
                 strncmp(languageCode, "may", lng_size) == 0 ||
                 strncmp(languageCode, "msa", lng_size) == 0)
            langage_qstr = QObject::tr("Malay");
        else if (strncmp(languageCode, "te", lng_size) == 0 ||
                 strncmp(languageCode, "tel", lng_size) == 0)
            langage_qstr = QObject::tr("Telugu");
        else if (strncmp(languageCode, "vi", lng_size) == 0 ||
                 strncmp(languageCode, "vie", lng_size) == 0)
            langage_qstr = QObject::tr("Vietnamese");
        else if (strncmp(languageCode, "ko", lng_size) == 0 ||
                 strncmp(languageCode, "kor", lng_size) == 0)
            langage_qstr = QObject::tr("Korean");
        else if (strncmp(languageCode, "fr", lng_size) == 0 ||
                 strncmp(languageCode, "fre", lng_size) == 0 ||
                 strncmp(languageCode, "fra", lng_size) == 0)
            langage_qstr = QObject::tr("French");
        else if (strncmp(languageCode, "tr", lng_size) == 0 ||
                 strncmp(languageCode, "tur", lng_size) == 0)
            langage_qstr = QObject::tr("Turkish");
        else if (strncmp(languageCode, "it", lng_size) == 0 ||
                 strncmp(languageCode, "ita", lng_size) == 0)
            langage_qstr = QObject::tr("Italian");
        else if (strncmp(languageCode, "yue", lng_size) == 0)
            langage_qstr = QObject::tr("Yue & Cantonese");
        else if (strncmp(languageCode, "th", lng_size) == 0 ||
                 strncmp(languageCode, "tha", lng_size) == 0)
            langage_qstr = QObject::tr("Thai");

        else if (strncmp(languageCode, "sv", lng_size) == 0 ||
                 strncmp(languageCode, "swe", lng_size) == 0)
            langage_qstr = QObject::tr("Swedish");
        else if (strncmp(languageCode, "pl", lng_size) == 0 ||
                 strncmp(languageCode, "pol", lng_size) == 0)
            langage_qstr = QObject::tr("Polish");
        else if (strncmp(languageCode, "ro", lng_size) == 0 ||
                 strncmp(languageCode, "ron", lng_size) == 0 ||
                 strncmp(languageCode, "rum", lng_size) == 0)
            langage_qstr = QObject::tr("Romanian");
        else if (strncmp(languageCode, "nl", lng_size) == 0 ||
                 strncmp(languageCode, "nld", lng_size) == 0 ||
                 strncmp(languageCode, "dut", lng_size) == 0)
            langage_qstr = QObject::tr("Dutch");
        else if (strncmp(languageCode, "no", lng_size) == 0 ||
                 strncmp(languageCode, "nor", lng_size) == 0)
            langage_qstr = QObject::tr("Norwegian");
        else if (strncmp(languageCode, "nb", lng_size) == 0 ||
                 strncmp(languageCode, "nob", lng_size) == 0)
            langage_qstr = QObject::tr("Norwegian Bokmål");
        else if (strncmp(languageCode, "nm", lng_size) == 0 ||
                 strncmp(languageCode, "nmo", lng_size) == 0)
            langage_qstr = QObject::tr("Norwegian Nynorsk	");
        else if (strncmp(languageCode, "fi", lng_size) == 0 ||
                 strncmp(languageCode, "fin", lng_size) == 0)
            langage_qstr = QObject::tr("Finnish");
        else if (strncmp(languageCode, "da", lng_size) == 0 ||
                 strncmp(languageCode, "dan", lng_size) == 0)
            langage_qstr = QObject::tr("Danish");
        else if (strncmp(languageCode, "hu", lng_size) == 0 ||
                 strncmp(languageCode, "hun", lng_size) == 0)
            langage_qstr = QObject::tr("Hungarian");
        else if (strncmp(languageCode, "cs", lng_size) == 0 ||
                 strncmp(languageCode, "cze", lng_size) == 0 ||
                 strncmp(languageCode, "ces", lng_size) == 0)
            langage_qstr = QObject::tr("Czech");
        else if (strncmp(languageCode, "el", lng_size) == 0 ||
                 strncmp(languageCode, "ell", lng_size) == 0 ||
                 strncmp(languageCode, "gre", lng_size) == 0)
            langage_qstr = QObject::tr("Greek");
        else if (strncmp(languageCode, "he", lng_size) == 0 ||
                 strncmp(languageCode, "heb", lng_size) == 0)
            langage_qstr = QObject::tr("Hebrew");
        else if (strncmp(languageCode, "hi", lng_size) == 0 ||
                 strncmp(languageCode, "hin", lng_size) == 0)
            langage_qstr = QObject::tr("Hindi");
        else if (strncmp(languageCode, "id", lng_size) == 0 ||
                 strncmp(languageCode, "ind", lng_size) == 0)
            langage_qstr = QObject::tr("Indonesian");
        else if (strncmp(languageCode, "et", lng_size) == 0 ||
                 strncmp(languageCode, "est", lng_size) == 0)
            langage_qstr = QObject::tr("Estonian");
        else if (strncmp(languageCode, "bg", lng_size) == 0 ||
                 strncmp(languageCode, "bul", lng_size) == 0)
            langage_qstr = QObject::tr("Bulgarian");
        else if (strncmp(languageCode, "lv", lng_size) == 0 ||
                 strncmp(languageCode, "lav", lng_size) == 0)
            langage_qstr = QObject::tr("Latvian");
        else if (strncmp(languageCode, "lt", lng_size) == 0 ||
                 strncmp(languageCode, "lit", lng_size) == 0)
            langage_qstr = QObject::tr("Lithuanian");
        else if (strncmp(languageCode, "sk", lng_size) == 0 ||
                 strncmp(languageCode, "slo", lng_size) == 0 ||
                 strncmp(languageCode, "slk", lng_size) == 0)
            langage_qstr = QObject::tr("Slovak");
        else if (strncmp(languageCode, "sl", lng_size) == 0 ||
                 strncmp(languageCode, "slv", lng_size) == 0)
            langage_qstr = QObject::tr("Slovenian");
        else if (strncmp(languageCode, "uk", lng_size) == 0 ||
                 strncmp(languageCode, "ukr", lng_size) == 0)
            langage_qstr = QObject::tr("Ukrainian");

        else
            langage_qstr = QString::fromUtf8(languageCode, static_cast<int>(lng_size));
    }

    return langage_qstr;
}

/* ************************************************************************** */

bitrateMinMax::bitrateMinMax(const double fps)
{
    if (fps > 0.0 && fps <= 240.0)
    {
        m_fps = static_cast<uint32_t>(std::round(fps));
    }
}

bitrateMinMax::bitrateMinMax(const uint32_t fps)
{
    if (fps > 0 && fps <= 240)
    {
        m_fps = fps;
    }
}

bitrateMinMax::~bitrateMinMax()
{
    //
}

uint32_t bitrateMinMax::pushSampleSize(const uint32_t sampleSize)
{
    uint32_t bitrate = 0;

    if (m_sampleCounter >= m_fps && !m_samplesData.empty())
    {
        m_samplesData.pop_front();
        m_sampleCounter--;
    }

    m_samplesData.push_back(sampleSize*8);
    m_sampleCounter++;

    if (m_sampleCounter >= m_fps)
    {
        for (int i = 0; i < m_samplesData.size(); i++)
        {
            bitrate += m_samplesData.at(i);
        }

        if (bitrate > m_bitrateMax)
            m_bitrateMax = bitrate;
        else if (bitrate < m_bitrateMin)
            m_bitrateMin = bitrate;
    }

    return bitrate;
}

void bitrateMinMax::getMinMax(uint32_t &min, uint32_t &max)
{
    min = m_bitrateMin;
    max = m_bitrateMax;
}

/* ************************************************************************** */
