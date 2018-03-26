/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
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
 * \file      cli.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#include "cli.h"
#include "utils.h"
#include "textexport.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>

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
    if (file.isEmpty() == false)
    {
        MediaFile_t *media = nullptr;

        char input_filepath[4096];
        strncpy(input_filepath, file.toLocal8Bit(), 4095);

        // Create and open the media file
        int retcode = minivideo_open(input_filepath, &media);
        if (retcode == SUCCESS)
        {
            // Parse media file
            retcode = minivideo_parse(media, true);
            if (retcode == SUCCESS)
            {
                // Generate text output and print it on console
                QString exportDatas;
                retcode = textExport::generateExportDatas_text(*media, exportDatas, details);

                QTextStream(stdout) << exportDatas << endl;
            }
            else
            {
                qDebug() << "minivideo_parse() failed with retcode: " << retcode;
            }
        }
        else
        {
            qDebug() << "minivideo_open() failed with retcode: " << retcode;
        }
    }

    return 0;
}

/* ************************************************************************** */
