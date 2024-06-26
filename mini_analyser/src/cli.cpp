/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of mini_analyser.
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
 * \date      2017
 */

#include "cli.h"
#include "minivideo_textexport_qt.h"

#include <QtGlobal>
#include <QFile>
#include <QTextStream>
#include <QDebug>

/* ************************************************************************** */

CLI::CLI()
{
    //
}

CLI::~CLI()
{
    //
}

/* ************************************************************************** */

int CLI::printFile(const QString &file, bool details)
{
    int status = 0;

    if (!file.isEmpty())
    {
        MediaFile_t *media = nullptr;

        char input_filepath[4096];
        strncpy(input_filepath, file.toLocal8Bit(), 4095);

        // Create and open the media file
        status = minivideo_open(input_filepath, &media);
        if (status == 1)
        {
            // Parse media file
            status = minivideo_parse(media, true, true);
            if (status == 1)
            {
                // Generate text output and print it on console
                QString exportData;
                status = textExport::generateExportData_text(*media, exportData, details);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                QTextStream(stdout) << exportData << endl;
#else
                QTextStream(stdout) << exportData << Qt::endl;
#endif
            }
            else
            {
                qWarning() << "minivideo_parse() failed with retcode: " << status;
            }
        }
        else
        {
            qWarning() << "minivideo_open() failed with retcode: " << status;
        }
    }

    return status;
}

/* ************************************************************************** */
