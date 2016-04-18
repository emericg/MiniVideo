/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
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
 * \file      utils.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#include "utils.h"
#include <QDebug>

QString getDurationString(const unsigned duration)
{
    QString duration_qstr;

    if (duration > 0)
    {
        unsigned hours = duration / 3600000;
        unsigned minutes = (duration - (hours * 3600000)) / 60000;
        unsigned seconds = (duration - (hours * 3600000) - (minutes * 60000)) / 1000;
        unsigned ms = (duration - (hours * 3600000) - (minutes * 60000)) - (seconds * 1000);

        if (hours > 0)
        {
            duration_qstr += QString::number(hours) + " hour ";
        }
        if (minutes > 0)
        {
            duration_qstr += QString::number(minutes) + " min ";
        }
        if (seconds > 0)
        {
            duration_qstr += QString::number(seconds) + " sec ";
        }
        if (ms > 0)
        {
            duration_qstr += QString::number(ms) + " ms";
        }
    }
    else
    {
        duration_qstr = "NULL duration";
    }

    //qDebug() << "getDurationString(" << duration << ") >" << duration_qstr;

    return duration_qstr;
}

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
            size_qstr = QString::number(size / 1024.0, 'f', 2) + " KiB  /  "
                      + QString::number(size / 1000.0, 'f', 2) + " KB  /  "
                      + QString::number(size) + " bytes";
        }
        else if (size < 1073741824) // < 1 GiB
        {
            size_qstr = QString::number(size / 1024.0 / 1024.0, 'f', 2) + " MiB  /  "
                      + QString::number(size / 1000.0 / 1000.0, 'f', 2) + " MB  /  "
                      + QString::number(size) + " bytes";
        }
        else // < 1 GiB
        {
            size_qstr = QString::number(size / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB  /  "
                      + QString::number(size / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB  /  "
                      + QString::number(size) + " bytes";
        }
    }
    else
    {
        size_qstr = "NULL size";
    }

    //qDebug() << "getDurationString(" << size << ") >" << size_qstr;

    return size_qstr;
}

QString getTrackSizeString(const BitstreamMap_t *track, const int64_t file_size, const bool detailed)
{
    QString size_qstr;

    if (track != NULL)
    {
        qint64 size_int = track->stream_size;

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
                    size_qstr = QString::number(size_int / 1024.0, 'f', 2) + " KiB  /  "
                              + QString::number(size_int / 1000.0, 'f', 2) + " KB  /  "
                              + QString::number(size_int) + " bytes";
                }
                else
                    size_qstr = QString::number(size_int / 1024.0, 'f', 2) + " KiB";
            }
            else if (track->stream_size < 1073741824) // < 1 GiB
            {
                if (detailed)
                {
                    size_qstr = QString::number(size_int / 1024.0 / 1024.0, 'f', 2) + " MiB  /  "
                              + QString::number(size_int / 1000.0 / 1000.0, 'f', 2) + " MB  /  "
                              + QString::number(size_int) + " bytes";
                }
                else
                    size_qstr = QString::number(size_int / 1024.0 / 1024.0, 'f', 2) + " MiB";
            }
            else // > 1 GiB
            {
                if (detailed)
                {
                    size_qstr = QString::number(size_int / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB  /  "
                              + QString::number(size_int / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB  /  "
                              + QString::number(size_int) + " bytes";
                }
                else
                    size_qstr = QString::number(size_int / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB";
            }

            // Percentage
            double sizepercent = ((double)size_int / (double)file_size) * 100.0;

            if (sizepercent < 0.01)
                size_qstr += " (~0.01%)";
            else
                size_qstr += " (" + QString::number(sizepercent, 'g', 3) + " %)";
        }

        //qDebug() << "getTrackSizeString(" << size_int << ") >" << size_qstr;
    }
    else
    {
        size_qstr = "NULL track size";
    }

    return size_qstr;
}

QString getAspectRatioString(const unsigned x, const unsigned y, bool detailed)
{
    double ar_d = static_cast<double>(x) / static_cast<double>(y);
    return getAspectRatioString(ar_d, detailed);
}

QString getAspectRatioString(double ar_d, bool detailed)
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
            aspectratio_qstr += " (35mm film)";
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
        aspectratio_qstr = QString::number(ar_d, 'g', 3) + ":1";
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
        bitrate_qstr = "NULL bitrate";
    }

    //qDebug() << "getBitrateString(" << bitrate << ") >" << bitrate_qstr;

    return bitrate_qstr;
}

QString getBitrateModeString(const unsigned bitrate_mode)
{
    QString bitrate_mode_qstr;

    if (bitrate_mode == BITRATE_CBR)
        bitrate_mode_qstr = "CBR";
    else if (bitrate_mode == BITRATE_VBR)
        bitrate_mode_qstr = "VBR";
    else if (bitrate_mode == BITRATE_ABR)
        bitrate_mode_qstr = "ABR";
    else if (bitrate_mode == BITRATE_CVBR)
        bitrate_mode_qstr = "CVBR";
    else
        bitrate_mode_qstr = "Unknown";

    return bitrate_mode_qstr;
}

QString getFramerateModeString(const unsigned framerate_mode)
{
    QString framerate_mode_qstr;

    if (framerate_mode == FRAMERATE_CFR)
        framerate_mode_qstr = "CFR";
    else if (framerate_mode == FRAMERATE_VFR)
        framerate_mode_qstr = "VFR";
    else
        framerate_mode_qstr = "Unknown";

    return framerate_mode_qstr;
}
