/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      textexporter.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "textexporter.h"
#include "ui_textexporter.h"

#include "utils.h"

#include <QDate>
#include <QTime>

TextExporter::TextExporter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextExporter)
{
    ui->setupUi(this);

    exportFormat = EXPORT_TEXT;

    connect(ui->pushButton_export, SIGNAL(clicked(bool)), this, SLOT(exportDatas()));
    connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(close()));
}

TextExporter::~TextExporter()
{
    delete ui;
}

void TextExporter::setMediaFile(MediaFile_t *media)
{
    if (media)
    {
        //
    }
}

void TextExporter::generateDatas(MediaFile_t *media)
{
    if (media)
    {
        // First generate output path
        QString outputFile_path = media->file_path;
        outputFile_path += ".txt";

        outputFile.setFileName(outputFile_path);

        exportedDatas.clear();

        // Fill datas
        exportedDatas += "Full path   : ";
        exportedDatas += media->file_path;
        exportedDatas += "\n\n";

        exportedDatas += "Title       : ";
        exportedDatas += media->file_name;
        exportedDatas += "\n";
        exportedDatas += "Size        : ";
        exportedDatas += getSizeString(media->file_size);
        exportedDatas += "\n";
        exportedDatas += "Duration    : ";
        exportedDatas += getDurationString(media->duration);
        exportedDatas += "\n";
        exportedDatas += "Container   : ";
        exportedDatas += getContainerString(media->container, true);
        exportedDatas += "\n";
        if (media->container == CONTAINER_MP4)
        {
            QDate date(1904, 1, 1);
            QTime time(0, 0, 0, 0);
            QDateTime datetime(date, time);
            datetime = datetime.addSecs(media->creation_time);
            exportedDatas += "Date        : ";
            exportedDatas += datetime.toString("dddd d MMMM yyyy, hh:mm:ss");
            exportedDatas += "\n";
        }

        // VIDEO TRACKS
        for (int i = 0; i < media->tracks_video_count; i++)
        {
            BitstreamMap_t *t = media->tracks_video[i];
            if (t == NULL)
                break;

            // Section title
            if (media->tracks_video_count == 1)
            {
                exportedDatas += "\nVIDEO\n-----\n";
            }
            else
            {
                exportedDatas += "\nVIDEO TRACK #";
                exportedDatas += QString::number(i);
                exportedDatas += "\n--------------\n";
            }

            // Datas
            exportedDatas += "Codec       : ";
            exportedDatas += getCodecString(stream_VIDEO, t->stream_codec, true);
            exportedDatas += "\n";
            exportedDatas += "FourCC      : ";
            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            exportedDatas += QString::fromLatin1(fcc_str, 4);
            exportedDatas += "\n";
            exportedDatas += "Size        : ";
            exportedDatas += getSizeString(t->stream_size);
            exportedDatas += "\n";
            exportedDatas += "Duration    : ";
            exportedDatas += getDurationString(t->duration_ms);
            exportedDatas += "\n";
        }

        // AUDIO TRACKS
        for (int i = 0; i < media->tracks_audio_count; i++)
        {
            BitstreamMap_t *t = media->tracks_audio[i];
            if (t == NULL)
                break;

            // Section title
            exportedDatas += "\nAUDIO TRACK #";
            exportedDatas += QString::number(i);
            exportedDatas += "\n--------------\n";

            // Datas
            exportedDatas += "Codec       : ";
            exportedDatas += getCodecString(stream_AUDIO, t->stream_codec, true);
            exportedDatas += "\n";
            exportedDatas += "FourCC      : ";
            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            exportedDatas += QString::fromLatin1(fcc_str, 4);
            exportedDatas += "\n";
            exportedDatas += "Size        : ";
            exportedDatas += getSizeString(t->stream_size);
            exportedDatas += "\n";
            exportedDatas += "Duration    : ";
            exportedDatas += getDurationString(t->duration_ms);
            exportedDatas += "\n";
        }

        // Print it
        ui->textBrowser->setText(exportedDatas);
    }
}

void TextExporter::exportDatas()
{
    if (exportedDatas.isEmpty() == false)
    {
        if (outputFile.open(QIODevice::WriteOnly) == true &&
            outputFile.isWritable() == true)
        {
            if (outputFile.exists() == true)
            {
                // confirmation prompt
            }

            outputFile.write(exportedDatas.toLocal8Bit());
            outputFile.close();
        }
    }
}
