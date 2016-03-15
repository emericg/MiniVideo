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
#include <QDebug>
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>

TextExporter::TextExporter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextExporter)
{
    ui->setupUi(this);

    exportFormat = EXPORT_TEXT;

    connect(ui->pushButton_filechooser, SIGNAL(clicked(bool)), this, SLOT(saveFileDialog()));
    connect(ui->pushButton_export, SIGNAL(clicked(bool)), this, SLOT(saveDatas()));
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

void TextExporter::saveFileDialog()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save media informations in a text file"),
                                                    ui->lineEdit_filename->text(),
                                                    tr("Files (*.txt)"));

    if (filePath.isEmpty() == false)
        setOutputFile(filePath);
}

void TextExporter::setOutputFile(QString &filePath)
{
    if (exportFormat == EXPORT_XML)
        filePath += ".xml";
    else if (exportFormat == EXPORT_JSON)
        filePath += ".json";
    else // if (exportFormat == EXPORT_TEXT)
        filePath += ".txt";

    ui->lineEdit_filename->setText(filePath);
}

void TextExporter::generateDatas(MediaFile_t *media)
{
    if (media)
    {
        // First generate output path
        QString outputFilePath = media->file_path;
        setOutputFile(outputFilePath);

        // Clear datas
        exportDatas.clear();

        // Fill datas
        exportDatas += "Full path   : ";
        exportDatas += media->file_path;
        exportDatas += "\n\n";

        exportDatas += "Title       : ";
        exportDatas += media->file_name;
        exportDatas += "\n";
        exportDatas += "Size        : ";
        exportDatas += getSizeString(media->file_size);
        exportDatas += "\n";
        exportDatas += "Duration    : ";
        exportDatas += getDurationString(media->duration);
        exportDatas += "\n";
        exportDatas += "Container   : ";
        exportDatas += getContainerString(media->container, true);
        exportDatas += "\n";
        if (media->container == CONTAINER_MP4)
        {
            QDate date(1904, 1, 1);
            QTime time(0, 0, 0, 0);
            QDateTime datetime(date, time);
            datetime = datetime.addSecs(media->creation_time);
            exportDatas += "Date        : ";
            exportDatas += datetime.toString("dddd d MMMM yyyy, hh:mm:ss");
            exportDatas += "\n";
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
                exportDatas += "\nVIDEO\n-----\n";
            }
            else
            {
                exportDatas += "\nVIDEO TRACK #";
                exportDatas += QString::number(i);
                exportDatas += "\n--------------\n";
            }

            // Datas
            exportDatas += "Codec       : ";
            exportDatas += getCodecString(stream_VIDEO, t->stream_codec, true);
            exportDatas += "\n";
            exportDatas += "FourCC      : ";
            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            exportDatas += QString::fromLatin1(fcc_str, 4);
            exportDatas += "\n";
            exportDatas += "Size        : ";
            exportDatas += getSizeString(t->stream_size);
            exportDatas += "\n";
            exportDatas += "Duration    : ";
            exportDatas += getDurationString(t->duration_ms);
            exportDatas += "\n";
        }

        // AUDIO TRACKS
        for (int i = 0; i < media->tracks_audio_count; i++)
        {
            BitstreamMap_t *t = media->tracks_audio[i];
            if (t == NULL)
                break;

            // Section title
            exportDatas += "\nAUDIO TRACK #";
            exportDatas += QString::number(i);
            exportDatas += "\n--------------\n";

            // Datas
            exportDatas += "Codec       : ";
            exportDatas += getCodecString(stream_AUDIO, t->stream_codec, true);
            exportDatas += "\n";
            exportDatas += "FourCC      : ";
            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            exportDatas += QString::fromLatin1(fcc_str, 4);
            exportDatas += "\n";
            exportDatas += "Size        : ";
            exportDatas += getSizeString(t->stream_size);
            exportDatas += "\n";
            exportDatas += "Duration    : ";
            exportDatas += getDurationString(t->duration_ms);
            exportDatas += "\n";
        }

        // Print it
        ui->textBrowser->setText(exportDatas);
    }
}

void TextExporter::saveDatas()
{
    if (exportDatas.isEmpty() == false)
    {
        exportFile.setFileName(ui->lineEdit_filename->text());

        if (exportFile.open(QIODevice::WriteOnly) == true &&
            exportFile.isWritable() == true)
        {
            if (exportFile.exists() == true)
            {
                // Confirmation prompt
                QMessageBox::StandardButton messageReply;
                QString messageText = tr("This file already exist:\n");
                messageText += ui->lineEdit_filename->text();
                messageText += tr("\nAre you sure you want to overwrite it?");

                messageReply = QMessageBox::warning(this, tr("Confirm file overwrite"),
                                                    messageText,
                                                    QMessageBox::Yes | QMessageBox::No);

                if (messageReply == QMessageBox::No)
                {
                    return;
                }
            }

            exportFile.write(exportDatas.toLocal8Bit());
            exportFile.close();
        }
    }
}
