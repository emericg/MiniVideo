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
#include <iostream>

QString getDurationString(const int duration_int)
{
    QString duration_qstr;

    if (duration_int > 0)
    {
        int hours = duration_int / 3600000;
        int minutes = (duration_int - (hours * 3600000)) / 60000;
        int seconds = (duration_int - (hours * 3600000) - (minutes * 60000)) / 1000;
        int ms = (duration_int - (hours * 3600000) - (minutes * 60000)) - (seconds * 1000);

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
            duration_qstr += QString::number(seconds) + " sec";
        }
        if (ms > 0)
        {
            duration_qstr += " " + QString::number(ms) + " ms";
        }
    }
    else
    {
        duration_qstr = "NULL duration";
    }

    //std::cout << "getDurationString(" << duration_int << ") > '" << duration_qstr.toStdString() << "'" << std::endl;

    return duration_qstr;
}

QString getSizeString(const int64_t size_int)
{
    QString size_qstr;

    if (size_int > 0)
    {
        if (size_int < 1024) // < 1 KiB
        {
            size_qstr = QString::number(size_int) + " bytes";
        }
        else if (size_int < 1048576) // < 1 MiB
        {
            size_qstr = QString::number(size_int / 1024.0, 'f', 2) + " KiB   /   "
                      + QString::number(size_int / 1000.0, 'f', 2) + " KB   /   ("
                      + QString::number(size_int) + " bytes)";
        }
        else if (size_int < 1073741824) // < 1 GiB
        {
            size_qstr = QString::number(size_int / 1024.0 / 1024.0, 'f', 2) + " MiB   /   "
                      + QString::number(size_int / 1000.0 / 1000.0, 'f', 2) + " MB   /   ("
                      + QString::number(size_int) + " bytes)";
        }
        else // < 1 GiB
        {
            size_qstr = QString::number(size_int / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB   /   "
                      + QString::number(size_int / 1000.0 / 1000.0 / 1000.0, 'f', 2) + " GB   /   ("
                      + QString::number(size_int) + " bytes)";
        }
    }
    else
    {
        size_qstr = "NULL size";
    }

    //std::cout << "getDurationString(" << size_int << ") > '" << size_qstr.toStdString() << "'" << std::endl;

    return size_qstr;
}

QString getTrackSizeString(const BitstreamMap_t *track, int64_t file_size)
{
    QString size_qstr;
    int64_t size_int = 0;

    if (track != NULL)
    {
        for (unsigned i = 0; i < track->sample_count; i++)
        {
            size_int += track->sample_size[i];
        }

        if (size_int > 0)
        {
            if (size_int < 1024) // < 1 KiB
            {
                size_qstr = QString::number(size_int) + " bytes";
            }
            else if (size_int < 1048576) // < 1 MiB
            {
                size_qstr = QString::number(size_int / 1024.0, 'f', 2) + " KiB";
            }
            else if (size_int < 1073741824) // < 1 GiB
            {
                size_qstr = QString::number(size_int / 1024.0 / 1024.0, 'f', 2) + " MiB";
            }
            else // < 1 GiB
            {
                size_qstr = QString::number(size_int / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB";
            }

            // Percentage
            int64_t sizepercent = ((double)size_int / (double)file_size) * 100.0;
            size_qstr += " (" + QString::number(sizepercent, 'g', 3) + " %)";
        }
    }
    else
    {
        size_qstr = "NULL track size";
    }

    //std::cout << "getTrackSizeString(" << size_int << ") > '" << size_qstr.toStdString() << "'" << std::endl;

    return size_qstr;
}

QString getAspectRatioString(const int x, const int y)
{
    QString aspectratio_qstr;

    double ar_d = static_cast<double>(x) / static_cast<double>(y);

    if (ar_d > 1.75 && ar_d < 1.79)
    {
        aspectratio_qstr = "16/9";
    }
    else if (ar_d > 1.65 && ar_d < 1.67)
    {
        aspectratio_qstr = "5/3";
    }
    else if (ar_d > 1.59 && ar_d < 1.61)
    {
        aspectratio_qstr = "16/10";
    }
    else if (ar_d > 1.49 && ar_d < 1.51)
    {
        aspectratio_qstr = "3/2";
    }
    else if (ar_d > 1.32 && ar_d < 1.34)
    {
        aspectratio_qstr = "4/3";
    }
    else if (ar_d > 1.24 && ar_d < 1.26)
    {
        aspectratio_qstr = "5/4";
    }
    else
    {
        aspectratio_qstr = QString::number(ar_d, 'g', 3);
    }

    //std::cout << "getAspectRatioString(" << x << "," << y << ") > '" << aspectratio_qstr.toStdString() << "'" << std::endl;

    return aspectratio_qstr;
}

QString getBitrateString(const int bitrate_int)
{
    QString bitrate_qstr;

    if (bitrate_int > 0)
    {
        if (bitrate_int < 1048576) // < 1 MiB
        {
            bitrate_qstr = QString::number(bitrate_int / 1024.0, 'f', 2) + " KiB/s";
        }
        else if (bitrate_int < 1073741824) // < 1 GiB
        {
            bitrate_qstr = QString::number(bitrate_int / 1024.0 / 1024.0, 'f', 2) + " MiB/s";
        }
    }
    else
    {
        bitrate_qstr = "NULL bitrate";
    }

    //std::cout << "getBitrateString(" << bitrate_int << ") > '" << bitrate_qstr.toStdString() << "'" << std::endl;

    return bitrate_qstr;
}
