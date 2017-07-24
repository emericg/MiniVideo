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
 * \file      tabexport.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "tabexport.h"
#include "ui_tabexport.h"

// minivideo library
#include <minivideo.h>

// minianalyser
#include "utils.h"

#include <QMessageBox>
#include <QFontDatabase>
#include <QFileDialog>
#include <QDateTime>
#include <QFile>
#include <QDebug>

tabExport::tabExport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabExport)
{
    ui->setupUi(this);

    connect(ui->comboBox_export_modes, SIGNAL(activated(int)), this, SLOT(generateExportDatas()));

    connect(ui->pushButton_export_filechooser, SIGNAL(clicked(bool)), this, SLOT(saveFileDialog()));
    connect(ui->pushButton_export, SIGNAL(clicked(bool)), this, SLOT(saveDatas()));

    // Monospace fonts for the export tab
#ifdef Q_OS_LINUX
    /*int id =*/ QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    ui->textBrowser_export->setFont(QFont("DejaVu Sans Mono", 12));
#endif
#ifdef Q_OS_OSX
    ui->textBrowser_export->setFont(QFont("Andale Mono", 12));
#endif
#ifdef Q_OS_WIN32
    ui->textBrowser_export->setFont(QFont("Lucida Console", 12));
#endif
}

tabExport::~tabExport()
{
    delete ui;
}

/* ************************************************************************** */

void tabExport::clean()
{
    media = nullptr;
    wrapper = nullptr;

    exportFormat = EXPORT_TEXT;
    exportDatas.clear();
    //exportFile.close(); // ?

    ui->comboBox_export_modes->setCurrentIndex(0);
    ui->comboBox_export_formats->setCurrentIndex(0);
    ui->lineEdit_export_filename->clear();
    ui->textBrowser_export->clear();
}

/* ************************************************************************** */

void tabExport::on_comboBox_export_modes_currentIndexChanged(int index)
{
    // Save current export mode
    if (wrapper)
    {
        wrapper->exportMode = index;
    }
}

void tabExport::on_comboBox_export_formats_currentIndexChanged(int index)
{
    // Save current export format
    if (wrapper)
    {
        wrapper->exportFormat = index;
    }
}

/* ************************************************************************** */

void tabExport::saveFileDialog()
{
    QString fileExtension = ".txt";
    QString fileType = tr("Files (*.txt)");
    QString filePath = ui->lineEdit_export_filename->text();
    int exportFormat = ui->comboBox_export_formats->currentIndex();

    if (exportFormat == EXPORT_JSON)
    {
        fileExtension = ".json";
        fileType = tr("Files (*.json)");
    }
    else if (exportFormat == EXPORT_XML)
    {
        fileExtension = ".xml";
        fileType = tr("Files (*.xml)");
    }

    filePath = QFileDialog::getSaveFileName(this, tr("Save media informations in a text file"),
                                            filePath, fileExtension);

    if (filePath.isEmpty() == false)
    {
        filePath += fileExtension;
        ui->lineEdit_export_filename->setText(filePath);
    }
}

void tabExport::saveDatas()
{
    if (exportDatas.isEmpty() == false)
    {
        exportFile.setFileName(ui->lineEdit_export_filename->text());

        if (exportFile.exists() == true)
        {
            // Confirmation prompt
            QMessageBox::StandardButton messageReply;
            QString messageText = tr("This file already exist:\n");
            messageText += ui->lineEdit_export_filename->text();
            messageText += tr("\nAre you sure you want to overwrite it?");

            messageReply = QMessageBox::warning(this, tr("Confirm file overwrite"),
                                                messageText,
                                                QMessageBox::Yes | QMessageBox::No);

            if (messageReply == QMessageBox::No)
            {
                return;
            }
        }

        if (exportFile.open(QIODevice::WriteOnly) == true &&
            exportFile.isWritable() == true)
        {
            exportFile.write(exportDatas.toLocal8Bit());
            exportFile.close();
        }
    }
}

/* ************************************************************************** */

int tabExport::loadMedia(const MediaFile_t *media)
{
    int retcode = 0;

    if (media)
    {
        this->media = (MediaFile_t *)media;
        retcode = generateExportDatas();
    }

    return retcode;
}

int tabExport::loadMedia(const MediaWrapper *wrapper)
{
    int retcode = 0;

    if (wrapper)
    {
        if (wrapper->media)
        {
            this->wrapper = (MediaWrapper *)wrapper;
            this->media = (MediaFile_t *)media;

            retcode = generateExportDatas();
        }
    }

    return retcode;
}

int tabExport::generateExportDatas()
{
    int retcode = 0;

    if (media)
    {
        // Clear datas
        exportDatas.clear();

        // Output path is file path + another extension
        QString outputFilePath = media->file_path;

        // Read file extension and details
        int exportMode = ui->comboBox_export_modes->currentIndex();
        int exportFormat = ui->comboBox_export_formats->currentIndex();

        if (exportMode == 0 || exportMode == 1)
        {
            ui->comboBox_export_formats->setCurrentIndex(EXPORT_TEXT);
            exportFormat = EXPORT_TEXT;

            if (exportFormat == EXPORT_JSON)
            {
                outputFilePath += ".json";
                retcode = generateExportDatas_json(exportMode);
            }
            else if (exportFormat == EXPORT_XML)
            {
                outputFilePath += ".xml";
                retcode = generateExportDatas_xml(exportMode);
            }
            else // if (exportFormat == EXPORT_TEXT)
            {
                outputFilePath += ".txt";
                retcode = generateExportDatas_text(exportMode);
            }
        }
        else if (exportMode == 2)
        {
            ui->comboBox_export_formats->setCurrentIndex(EXPORT_XML);
            exportFormat = EXPORT_XML;

            {
                outputFilePath += ".xml";
                retcode = generateExportMapping_xml();
            }
        }

        // Print it
        ui->lineEdit_export_filename->setText(outputFilePath);
        ui->textBrowser_export->setText(exportDatas);
    }

    return retcode;
}

int tabExport::generateExportDatas_text(bool detailed)
{
    int retcode = 0;

    if (media)
    {
        exportDatas += "Full path     : ";
        exportDatas += media->file_path;

        exportDatas += "\n\nTitle         : ";
        exportDatas += media->file_name;
        exportDatas += "\nDuration      : ";
        exportDatas += getDurationString(media->duration);
        exportDatas += "\nSize          : ";
        exportDatas += getSizeString(media->file_size);
        exportDatas += "\nContainer     : ";
        exportDatas += getContainerString(media->container, true);
        if (media->creation_app)
        {
            exportDatas += "\nCreation app  : ";
            exportDatas += media->creation_app;
        }
        if (media->creation_lib)
        {
            exportDatas += "\nCreation lib  : ";
            exportDatas += media->creation_lib;
        }
        if (media->creation_time)
        {
            QDate date(1904, 1, 1);
            QTime time(0, 0, 0, 0);
            QDateTime datetime(date, time);
            datetime = datetime.addSecs(media->creation_time);
            exportDatas += "\nCreation time : ";
            exportDatas += datetime.toString("dddd d MMMM yyyy, hh:mm:ss");
        }

        // VIDEO TRACKS
        ////////////////////////////////////////////////////////////////////////

        for (unsigned i = 0; i < media->tracks_video_count; i++)
        {
            MediaStream_t *t = media->tracks_video[i];
            if (t == nullptr)
                break;

            // Section title
            if (media->tracks_video_count == 1)
            {
                exportDatas += "\n\nVIDEO\n-----";
            }
            else
            {
                exportDatas += "\nVIDEO TRACK #";
                exportDatas += QString::number(i);
                exportDatas += "\n--------------";
            }

            // Datas
            if (detailed == true)
            {
                exportDatas += "\nTrack ID      : ";
                exportDatas += QString::number(t->track_id);

                char fcc_str[4];
                {
                    fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                    fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                    fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                    fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
                }
                exportDatas += "\nFourCC        : ";
                exportDatas += QString::fromLatin1(fcc_str, 4);
            }
            exportDatas += "\nCodec         : ";
            exportDatas += getCodecString(stream_VIDEO, t->stream_codec, true);
            exportDatas += "\nSize          : ";
            exportDatas += getTrackSizeString(t, media->file_size, detailed);
            exportDatas += "\nDuration      : ";
            exportDatas += getDurationString(t->stream_duration_ms);
            exportDatas += "\nWidth         : ";
            exportDatas += QString::number(t->width);
            exportDatas += "\nHeight        : ";
            exportDatas += QString::number(t->height);

            if (detailed == true)
            {
                if (t->pixel_aspect_ratio_h || t->pixel_aspect_ratio_v)
                {
                    exportDatas += "\nPixel Aspect ratio   : ";
                    exportDatas += QString::number(t->pixel_aspect_ratio_h) + ":" + QString::number(t->pixel_aspect_ratio_v);
                }
                if (t->video_aspect_ratio > 0.0)
                {
                    exportDatas += "\nVideo Aspect ratio   : ";
                    exportDatas += getAspectRatioString(t->video_aspect_ratio, false);
                }
                exportDatas += "\nDisplay Aspect ratio : ";
                exportDatas += getAspectRatioString(t->display_aspect_ratio, true);
            }
            else
            {
                exportDatas += "\nAspect ratio  : ";
                exportDatas += getAspectRatioString(t->display_aspect_ratio, detailed);
            }

            if (detailed == true)
            {
                exportDatas += "\nFramerate     : ";
                exportDatas += QString::number(t->framerate) + " fps";
                exportDatas += "\nFramerate mode: ";
                exportDatas += getFramerateModeString(t->framerate_mode);

                exportDatas += "\nBitrate       : ";
                exportDatas += getBitrateString(t->bitrate_avg);
                exportDatas += "\nBitrate mode  : ";
                exportDatas += getBitrateModeString(t->bitrate_mode);
                if (t->bitrate_mode != BITRATE_CBR)
                {
                    exportDatas += "\nBitrate (min) : ";
                    exportDatas += getBitrateString(t->bitrate_min);
                    exportDatas += "\nBitrate (max) : ";
                    exportDatas += getBitrateString(t->bitrate_max);
                }
            }
            else
            {
                exportDatas += "\nFramerate     : ";
                exportDatas += QString::number(t->framerate) + " fps (";
                exportDatas += getFramerateModeString(t->framerate_mode) + ")";

                exportDatas += "\nBitrate       : ";
                exportDatas += getBitrateString(t->bitrate_avg);
                exportDatas += " (" + getBitrateModeString(t->bitrate_mode) + ")";
            }

            exportDatas += "\nColor depth   : ";
            exportDatas += QString::number(t->color_depth) + " bits";
/*
            exportDatas += "\nColor matrix  : ";
            if (t->color_range == 0)
                exportDatas += "\nColor range   : Limited";
            else
                exportDatas += "\nColor range   : Full";
*/
        }

        // AUDIO TRACKS
        ////////////////////////////////////////////////////////////////////////

        for (unsigned i = 0; i < media->tracks_audio_count; i++)
        {
            MediaStream_t *t = media->tracks_audio[i];
            if (t == nullptr)
                break;

            // Section title
            exportDatas += "\n\nAUDIO TRACK #";
            exportDatas += QString::number(i);
            exportDatas += "\n--------------";

            // Datas
            if (detailed == true)
            {
                exportDatas += "\nTrack ID      : ";
                exportDatas += QString::number(t->track_id);

                char fcc_str[4];
                {
                    fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                    fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                    fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                    fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
                }
                exportDatas += "\nFourCC        : ";
                exportDatas += QString::fromLatin1(fcc_str, 4);
            }
            exportDatas += "\nCodec         : ";
            exportDatas += getCodecString(stream_AUDIO, t->stream_codec, true);
            exportDatas += "\nSize          : ";
            exportDatas += getTrackSizeString(t, media->file_size, detailed);
            exportDatas += "\nDuration      : ";
            exportDatas += getDurationString(t->stream_duration_ms);
            if (t->track_title)
            {
                exportDatas += "\nTitle         : ";
                exportDatas += QString::fromLocal8Bit(t->track_title);
            }
            if (t->track_languagecode)
            {
                exportDatas += "\nLanguage      : ";
                exportDatas += QString::fromLocal8Bit(t->track_languagecode);
            }
            exportDatas += "\nChannels      : ";
            exportDatas += QString::number(t->channel_count);
            exportDatas += "\nBit per sample: ";
            exportDatas += QString::number(t->bit_per_sample);
            exportDatas += "\nSamplerate    : ";
            exportDatas += QString::number(t->sampling_rate) + " Hz";
            if (detailed == true)
            {
                exportDatas += "\nBitrate       : ";
                exportDatas += getBitrateString(t->bitrate_avg);
                exportDatas += "\nBitrate mode  : ";
                exportDatas += getBitrateModeString(t->bitrate_mode);
                if (t->bitrate_mode != BITRATE_CBR)
                {
                    exportDatas += "\nBitrate (min) : ";
                    exportDatas += getBitrateString(t->bitrate_min);
                    exportDatas += "\nBitrate (max) : ";
                    exportDatas += getBitrateString(t->bitrate_max);
                }
            }
            else
            {
                exportDatas += "\nBitrate       : ";
                exportDatas += getBitrateString(t->bitrate_avg);
                exportDatas += " (" + getBitrateModeString(t->bitrate_mode) + ")";
            }
        }

        // SUBTITLES TRACKS
        ////////////////////////////////////////////////////////////////////////

        for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
        {
            MediaStream_t *t = media->tracks_subt[i];
            if (t == nullptr)
                break;

            // Section title
            exportDatas += "\n\nSUBTITLES TRACK #";
            exportDatas += QString::number(i);
            exportDatas += "\n------------------";

            // Datas
            exportDatas += "\nFormat        : sub";
            exportDatas += "\nSize          : ";
            exportDatas += getTrackSizeString(t, media->file_size, detailed);
            exportDatas += "\nTitle         : ";
            exportDatas += QString::fromLocal8Bit(t->track_title);
            exportDatas += "\nLanguage      : ";
            exportDatas += QString::fromLocal8Bit(t->track_languagecode);
        }

        // OTHER TRACKS
        ////////////////////////////////////////////////////////////////////////

        for (unsigned i = 0; i < media->tracks_others_count; i++)
        {
            MediaStream_t *t = media->tracks_others[i];
            if (t == nullptr)
                break;

            // Section title
            if (t->stream_type == stream_TEXT)
                exportDatas += "\n\nTEXT TRACK #";
            else if (t->stream_type == stream_MENU)
                exportDatas += "\n\nMENU TRACK #";
            else if (t->stream_type == stream_TMCD)
                exportDatas += "\n\nTMCD TRACK #";
            else if (t->stream_type == stream_META)
                exportDatas += "\n\nMETA TRACK #";
            else if (t->stream_type == stream_HINT)
                exportDatas += "\n\nHINT TRACK #";

            exportDatas += QString::number(i);
            exportDatas += "\n-------------";

            // Datas
            exportDatas += "\nSize          : ";
            exportDatas += getTrackSizeString(t, media->file_size, detailed);
            exportDatas += "\nTitle         : ";
            exportDatas += QString::fromLocal8Bit(t->track_title);
            exportDatas += "\nLanguage      : ";
            exportDatas += QString::fromLocal8Bit(t->track_languagecode);
        }

        retcode = 1;
    }

    return retcode;
}

int tabExport::generateExportDatas_json(bool detailed)
{
    int retcode = 0;

    if (media)
    {
        // TODO
        Q_UNUSED(detailed);
    }

    return retcode;
}

int tabExport::generateExportDatas_xml(bool detailed)
{
    int retcode = 0;

    if (media)
    {
        // TODO
        Q_UNUSED(detailed);
    }

    return retcode;
}

int tabExport::generateExportMapping_xml()
{
    int status = 1;

    if (media)
    {
        QFile xmlMapFile;
        QString filename;

        // Load XML file (from given file descriptor)
        if (media->container_mapper_fd == nullptr ||
            xmlMapFile.open(media->container_mapper_fd, QIODevice::ReadOnly) == false)
        {
            status = 0;
            qDebug() << "xmlFile.open(FILE*) > error";
        }

        // Load XML file (fallback from file path / DEPRECATED)
        if (status == 0)
        {
            filename = "/tmp/" + QString::fromLocal8Bit(media->file_name) + "_mapped.xml";
            xmlMapFile.setFileName(filename);

            if (xmlMapFile.exists() == false ||
                xmlMapFile.open(QIODevice::ReadOnly) == false)
            {
                qDebug() << "xmlFile.open(" << filename << ") > error";
                status = 0;
            }
        }

        if (status == true)
        {
            xmlMapFile.seek(0);
            exportDatas = xmlMapFile.readAll();
        }
    }

    return status;
}
