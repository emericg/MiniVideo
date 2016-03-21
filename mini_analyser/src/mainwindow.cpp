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
 * \file      mainwindow.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

#include <QDropEvent>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QMimeData>

#include <QTimer>

#include <iostream>
#include <cmath>

/* ************************************************************************** */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    textexporter = NULL;
    hexeditor = NULL;
    fcchelper = NULL;
    aboutwindows = NULL;

    emptyFileList = true;

    ui->statusLabel->hide();
    statusTimer = new QTimer;
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(hideStatus()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadFileDialog()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));
    connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(openExporter()));
    connect(ui->actionHexEditor, SIGNAL(triggered()), this, SLOT(openHexEditor()));
    connect(ui->actionFourCC, SIGNAL(triggered()), this, SLOT(openFourccHelper()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutWindows()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->file_comboBox, SIGNAL(activated(int)), this, SLOT(printDatas()));

    connect(ui->pushButton_file_detach, SIGNAL(clicked(bool)), this, SLOT(detachFile()));
    connect(ui->pushButton_file_reload, SIGNAL(clicked(bool)), this, SLOT(reloadFile()));
    connect(ui->pushButton_file_exit, SIGNAL(clicked(bool)), this, SLOT(closeFile()));

    // Save tabs titles and icons
    tabDropZoneText = ui->tabWidget->tabText(0);
    tabDropZoneIcon = ui->tabWidget->tabIcon(0);
    tabInfosText = ui->tabWidget->tabText(1);
    tabInfosIcon = ui->tabWidget->tabIcon(1);
    tabAudioText = ui->tabWidget->tabText(2);
    tabAudioIcon = ui->tabWidget->tabIcon(2);
    tabVideoText = ui->tabWidget->tabText(3);
    tabVideoIcon = ui->tabWidget->tabIcon(3);
    tabSubsText = ui->tabWidget->tabText(4);
    tabSubsIcon = ui->tabWidget->tabIcon(4);
    tabOtherText = ui->tabWidget->tabText(5);
    tabOtherIcon = ui->tabWidget->tabIcon(5);

    // "Drop zone" is the default tab when starting up
    handleTabWidget();

    // Accept video files "drag & drop"
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    delete statusTimer;
    delete ui;
}

/* ************************************************************************** */

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    // Check if the object dropped has an url (and is a file)
    if (e->mimeData()->hasUrls())
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        // TODO Filter by MimeType or file extension
        // Use QMimeDatabase // Qt 5.5
#endif

        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        const QString &fileName = url.toLocalFile();
        qDebug() << "Dropped file: "<< fileName;

        loadFile(fileName);
    }
}

/* ************************************************************************** */

int MainWindow::setAppPath(const QString &path)
{
    int status = 0;

    if (path.isEmpty() == false)
    {
        applicationPath = path;
        status = 1;
    }

    return status;
}

void MainWindow::loadFileDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a multimedia file"),
                                                    "", tr("Files (*.*)"));

    loadFile(fileName);
}

int MainWindow::loadFile(const QString &file)
{
    int retcode = FAILURE;

    if (file.isEmpty() == false)
    {
        // Check if this is a duplicate
        for (unsigned i = 0; i < mediaList.size(); i++)
        {
            QString name = mediaList.at(i)->file_path;
            if (file == name)
            {
                closeFile(file);
                break;
            }
        }

        // Load file
        setStatus("Working...", SUCCESS, 0);

        retcode = analyseFile(file);
        if (retcode == 1)
        {
            handleComboBox(file);
            cleanDatas();
            printDatas();
            hideStatus();
        }
        else
        {
            handleComboBox(file);
            cleanDatas();
            printDatas();
            hideStatus();

            setStatus("The following file cannot be opened (UNKNOWN ERROR):\n'" + file + "'", FAILURE, 7500);
        }
    }

    return retcode;
}

void MainWindow::closeFile(const QString &file)
{
    if (mediaList.empty() == false)
    {
        // Find the index of the given file
        for (unsigned i = 0; i < mediaList.size(); i++)
        {
            QString name = mediaList.at(i)->file_path;
            if (file == name)
            {
                minivideo_close(&mediaList.at(i));

                mediaList.erase(mediaList.begin() + i);

                // Remove the entry from the comboBox
                ui->file_comboBox->removeItem(i);
                return;
            }
        }
    }
}

void MainWindow::closeFile()
{
    // First clean the interface
    cleanDatas();

    // Then try to close the file's context
    int fileIndex = ui->file_comboBox->currentIndex();
    if (mediaList.empty() == false)
    {
        if ((int)(mediaList.size()) >= (fileIndex + 1))
        {
            minivideo_close(&mediaList.at(fileIndex));

            mediaList.erase(mediaList.begin() + fileIndex);

            // Remove the entry from the comboBox
            ui->file_comboBox->removeItem(fileIndex);

            // Update comboBox index
            if (ui->file_comboBox->count() > 0)
            {
                ui->file_comboBox->activated(fileIndex-1);
            }
            else // No more file opened?
            {
                emptyFileList = true;
                QString empty;

                handleTabWidget();
                handleComboBox(empty);
            }
        }
    }
}

void MainWindow::reloadFile(const QString &file)
{
    if (file.isEmpty() == false && mediaList.empty() == false)
    {
        // Find the index of the given file
        int fileIndex = -1;
        for (unsigned i = 0; i < mediaList.size(); i++)
        {
            QString name = mediaList.at(i)->file_path;
            if (file == name)
            {
                fileIndex = (int)i;

                minivideo_close(&mediaList.at(fileIndex));

                mediaList.erase(mediaList.begin() + fileIndex);

                // Remove the entry from the comboBox
                ui->file_comboBox->removeItem(fileIndex);

                // Load file
                setStatus("Working...", SUCCESS, 0);

                int retcode = analyseFile(file);
                if (retcode == 1)
                {
                    handleComboBox(file);
                    cleanDatas();
                    printDatas();
                    hideStatus();
                }
                else
                {
                    setStatus("The following file cannot be opened (UNKNOWN ERROR):\n'" + file + "'", FAILURE, 7500);
                }
                return;
            }
        }
    }
}

void MainWindow::reloadFile()
{
    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        QString current = media->file_path;
        closeFile();
        loadFile(current);
    }
}

void MainWindow::detachFile()
{
    if (applicationPath.isEmpty() == false)
    {
        MediaFile_t *media = currentMediaFile();
        int fileCount = ui->file_comboBox->count();

        if (media && fileCount > 1)
        {
            QString detachPath = media->file_path;
            QStringList args;
            args.push_back(detachPath);

            QProcess::startDetached(applicationPath, args);

            closeFile();
        }
    }
}

/* ************************************************************************** */

MediaFile_t *MainWindow::currentMediaFile()
{
    MediaFile_t *media = NULL;

    size_t fileIndex = ui->file_comboBox->currentIndex();

    if (mediaList.size() == 1)
    {
        media = mediaList.at(0);
    }
    else if (fileIndex < mediaList.size())
    {
        media = mediaList.at(fileIndex);
    }

    return media;
}

void MainWindow::handleComboBox(const QString &file)
{
    // Is this the first file added?
    if (emptyFileList)
    {
        if (mediaList.empty() == true)
        {
            ui->file_comboBox->addItem(QIcon(":/icons/icons/dialog-information.svg"), "Drag and drop files to analyse them!");
        }
        else
        {
            ui->file_comboBox->removeItem(0);
            emptyFileList = false;
        }
    }

    if (file.isEmpty() == false)
    {
        ui->file_comboBox->addItem(file);
        ui->file_comboBox->setCurrentIndex(ui->file_comboBox->count() - 1);

        //QIcon(":/icons/icons/audio-x-wav.svg")
        //QIcon(":/icons/icons/video-x-generic.svg")
    }
}

void MainWindow::handleTabWidget()
{
    ui->tabWidget->setEnabled(true);

    // Save the current tab index if we want to restore it later
    int tab_saved = ui->tabWidget->currentIndex();

    // Remove all tabs
    while (ui->tabWidget->count() > 0)
    {
        ui->tabWidget->removeTab(0);
    }

    if (mediaList.empty())
    {
        // No media ? Re-add only the "drop zone" tab
        ui->tabWidget->addTab(ui->tab_dropzone, tabDropZoneIcon, tabDropZoneText);
        ui->tabWidget->tabBar()->hide();
        ui->tabWidget->setCurrentIndex(0);
    }
    else
    {
        // Otherwise, adapt tabs to selected media file content
        MediaFile_t *media = currentMediaFile();

        ui->tabWidget->tabBar()->show();
        ui->tabWidget->addTab(ui->tab_infos, tabInfosIcon, tabInfosText);

        if (media->tracks_video_count)
        {
            // Add the "video" tab
            ui->tabWidget->addTab(ui->tab_video, tabVideoIcon, tabVideoText);
        }

        if (media->tracks_audio_count)
        {
            // Add the "audio" tab
            ui->tabWidget->addTab(ui->tab_audio, tabAudioIcon, tabAudioText);
        }

        if (media->tracks_subtitles_count)
        {
            // Add the "subtitles" tab
            ui->tabWidget->addTab(ui->tab_subtitles, tabSubsIcon, tabSubsText);
        }

        if (media->tracks_others_count)
        {
            // Add the "others" tab
            ui->tabWidget->addTab(ui->tab_others, tabOtherIcon, tabOtherText);
        }

        // Restore the same tab (if possible)
        if (tab_saved > 0 && tab_saved <= ui->tabWidget->count())
        {
            ui->tabWidget->setCurrentIndex(tab_saved);
        }
    }
}

/* ************************************************************************** */

void MainWindow::setStatus(const QString &text, int status, int duration)
{
    if (status == FAILURE)
    {
        ui->statusLabel->setStyleSheet("QLabel { border: 1px solid rgb(255, 53, 3);\nbackground: rgba(255, 170, 0, 128); }");
    }
    else //if (type == SUCCESS)
    {
        ui->statusLabel->setStyleSheet("QLabel { border: 1px solid rgb(85, 170, 0);\nbackground: rgba(85, 200, 0, 128); }");
    }

    if (duration > 0)
    {
        statusTimer->setInterval(7500);
        statusTimer->start();
    }

    ui->statusLabel->setText(text);
    ui->statusLabel->show();

    ui->statusLabel->repaint();
    qApp->processEvents();
}

void MainWindow::hideStatus()
{
    ui->statusLabel->hide();
}

/* ************************************************************************** */

int MainWindow::analyseFile(const QString &file)
{
    int retcode = FAILURE;
    char input_filepath[4096];

    if (file.isEmpty() == false)
    {
        strcpy(input_filepath, file.toLocal8Bit());

        // Create and open the media file
        MediaFile_t *input_media = NULL;
        retcode = minivideo_open(input_filepath, &input_media);

        if (retcode == SUCCESS)
        {
            retcode = minivideo_parse(input_media, true, true, true);
            mediaList.push_back(input_media);

            if (retcode != SUCCESS)
            {
                std::cerr << "minivideo_parse() failed with retcode: " << retcode << std::endl;
            }
        }
        else
        {
            std::cerr << "minivideo_open() failed with retcode: " << retcode << std::endl;
        }
    }

    return retcode;
}

int MainWindow::printDatas()
{
    int retcode = 0;

    handleTabWidget();

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        ui->label_filename->setText(QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));
        ui->label_filename->setToolTip(QString::fromLocal8Bit(media->file_name));
        ui->label_fullpath->setText(QString::fromLocal8Bit(media->file_path));
        ui->label_fullpath->setToolTip(QString::fromLocal8Bit(media->file_path));
        ui->label_container->setText(getContainerString(media->container, 1));
        ui->label_filesize->setText(getSizeString(media->file_size));
        ui->label_duration->setText(getDurationString(media->duration));

        if (media->creation_app)
        {
            ui->label_creationapp->setText(QString::fromLocal8Bit(media->creation_app));
        }

        if (media->container == CONTAINER_MP4)
        {
            QDate date(1904, 1, 1);
            QTime time(0, 0, 0, 0);
            QDateTime datetime(date, time);
            datetime = datetime.addSecs(media->creation_time);
            ui->label_creationdate->setText(datetime.toString("dddd d MMMM yyyy, hh:mm:ss"));
        }

        // Container efficiency
        {
            uint64_t trackssize = 0;
            for (int i = 0; i < 16; i++)
            {
                if (media->tracks_audio[i])
                    trackssize += media->tracks_audio[i]->stream_size;
                if (media->tracks_video[i])
                    trackssize += media->tracks_video[i]->stream_size;
                if (media->tracks_subt[i])
                    trackssize += media->tracks_subt[i]->stream_size;
                //if (media->tracks_others[i])
                //    trackssize += media->tracks_others[i]->stream_size;
            }

            uint64_t overhead = media->file_size - trackssize;
            double overheadpercent = (static_cast<double>(overhead) / static_cast<double>(media->file_size)) * 100.0;

            if (overheadpercent < 0.01)
                ui->label_container_overhead->setText("<b>~0.1%</b>   >   " + getSizeString(overhead));
            else if (overheadpercent <= 100)
                ui->label_container_overhead->setText("<b>" + QString::number(overheadpercent, 'f', 2) + "%</b>   >   " + getSizeString(overhead));
        }

        // AUDIO
        ////////////////////////////////////////////////////////////////////////

        int atid = 0;
        if (media->tracks_audio[atid] == NULL)
        {
            ui->groupBox_audio->hide();
        }
        else
        {
            ui->groupBox_audio->show();

            //QString title_plural = "";
            //if (media->tracks_audio_count > 1)
            //    title_plural = "s";

            ui->groupBox_tab_audio->setTitle(tr("Audio track") + " #" + QString::number(media->tracks_audio[atid]->track_id));

            ui->label_audio_id->setText(QString::number(media->tracks_audio[atid]->track_id));

            if (media->tracks_audio[atid]->track_title)
            {
                ui->label_audio_title->setText(QString::fromLocal8Bit(media->tracks_audio[atid]->track_title));
            }
            if (media->tracks_audio[atid]->stream_encoder)
            {
                ui->label_audio_encoder->setText(QString::fromLocal8Bit(media->tracks_audio[atid]->stream_encoder));
            }

            ui->label_audio_size->setText(getTrackSizeString(media->tracks_audio[atid], media->file_size));
            ui->label_audio_size_2->setText(getTrackSizeString(media->tracks_audio[atid], media->file_size, true));
            ui->label_audio_codec->setText(getCodecString(stream_AUDIO, media->tracks_audio[atid]->stream_codec, true));
            ui->label_audio_codec_2->setText(getCodecString(stream_AUDIO, media->tracks_audio[atid]->stream_codec, true));

            char fcc_str[4];
            {
                fcc_str[3] = (media->tracks_audio[atid]->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (media->tracks_audio[atid]->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (media->tracks_audio[atid]->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (media->tracks_audio[atid]->stream_fcc >> 24) & 0xFF;
            }
            ui->label_audio_fcc->setText(QString::fromLatin1(fcc_str, 4));

            ui->label_audio_duration->setText(getDurationString(media->tracks_audio[atid]->duration_ms));
            ui->label_audio_duration_2->setText(getDurationString(media->tracks_audio[atid]->duration_ms));

            ui->label_audio_bitrate->setText(getBitrateString(media->tracks_audio[atid]->bitrate));
            ui->label_audio_bitrate_gross->setText(getBitrateString(media->tracks_audio[atid]->bitrate));

            if (media->tracks_audio[atid]->bitrate_mode == BITRATE_CBR)
            {
                ui->label_audio_bitratemode->setText("CBR");
                ui->label_audio_bitratemode_2->setText("CBR");
            }
            else if (media->tracks_audio[atid]->bitrate_mode == BITRATE_VBR)
            {
                ui->label_audio_bitratemode->setText("VBR");
                ui->label_audio_bitratemode_2->setText("VBR");
            }
            else if (media->tracks_audio[atid]->bitrate_mode == BITRATE_ABR)
            {
                ui->label_audio_bitratemode->setText("ABR");
                ui->label_audio_bitratemode_2->setText("ABR");
            }
            else if (media->tracks_audio[atid]->bitrate_mode == BITRATE_CVBR)
            {
                ui->label_audio_bitratemode->setText("CVBR");
                ui->label_audio_bitratemode_2->setText("CVBR");
            }

            //ui->label_audio_samplecount->setText(QString::number(media->tracks_audio[atid]->sample_count));

            ui->label_audio_samplingrate->setText(QString::number(media->tracks_audio[atid]->sampling_rate) + " Hz");
            ui->label_audio_samplingrate_2->setText(QString::number(media->tracks_audio[atid]->sampling_rate) + " Hz");
            ui->label_audio_channels->setText(QString::number(media->tracks_audio[atid]->channel_count));
            ui->label_audio_channels_2->setText(QString::number(media->tracks_audio[atid]->channel_count));

            if (media->tracks_audio[atid]->bit_per_sample)
            {
                ui->label_audio_bitpersample->setText(QString::number(media->tracks_audio[atid]->bit_per_sample) + " bits");
                ui->label_audio_bitpersample_2->setText(QString::number(media->tracks_audio[atid]->bit_per_sample) + " bits");
            }

            uint64_t rawsize = media->tracks_audio[atid]->sampling_rate * media->tracks_audio[atid]->channel_count * (media->tracks_audio[atid]->bit_per_sample / 8);
            rawsize *= media->tracks_audio[atid]->duration_ms;
            rawsize /= 1024.0;

            uint64_t ratio = round(static_cast<double>(rawsize) / static_cast<double>(media->tracks_audio[atid]->stream_size));
            ui->label_audio_compression_ratio->setText(QString::number(ratio) + ":1");

            ui->label_audio_samplecount->setText(QString::number(media->tracks_audio[atid]->sample_count));
            ui->label_audio_framecount->setText(QString::number(media->tracks_audio[atid]->frame_count));
            ui->label_audio_frameduration->setText(QString::number(media->tracks_audio[atid]->frame_duration));
        }

        // VIDEO
        ////////////////////////////////////////////////////////////////////////

        int vtid = 0;
        if (media->tracks_video[vtid] == NULL)
        {
            ui->groupBox_video->hide();
        }
        else
        {
            ui->groupBox_video->show();

            //QString title_plural = "";
            //if (media->tracks_video_count > 1)
            //    title_plural = "s";
            //ui->tab_video->setTitle(tr("Video track") + title_plural);

            ui->groupBox_tab_video->setTitle(tr("Video track") + " #" + QString::number(media->tracks_video[vtid]->track_id));

            ui->label_video_id->setText(QString::number(media->tracks_video[vtid]->track_id));

            if (media->tracks_video[vtid]->track_title)
            {
                ui->label_video_title->setText(QString::fromLocal8Bit(media->tracks_video[vtid]->track_title));
            }
            if (media->tracks_video[vtid]->stream_encoder)
            {
                ui->label_video_encoder->setText(QString::fromLocal8Bit(media->tracks_video[vtid]->stream_encoder));
            }

            ui->label_video_size->setText(getTrackSizeString(media->tracks_video[vtid], media->file_size));
            ui->label_video_size_2->setText(getTrackSizeString(media->tracks_video[vtid], media->file_size, true));
            ui->label_video_codec->setText(getCodecString(stream_VIDEO, media->tracks_video[vtid]->stream_codec, true));
            ui->label_video_codec_2->setText(getCodecString(stream_VIDEO, media->tracks_video[vtid]->stream_codec, true));

            char fcc_str[4];
            {
                fcc_str[3] = (media->tracks_video[vtid]->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (media->tracks_video[vtid]->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (media->tracks_video[vtid]->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (media->tracks_video[vtid]->stream_fcc >> 24) & 0xFF;
            }
            ui->label_video_fcc->setText(QString::fromLatin1(fcc_str, 4));

            ui->label_video_duration->setText(getDurationString(media->tracks_video[vtid]->duration_ms));
            ui->label_video_duration_2->setText(getDurationString(media->tracks_video[vtid]->duration_ms));

            ui->label_video_bitrate->setText(getBitrateString(media->tracks_video[vtid]->bitrate));
            ui->label_video_bitrate_gross->setText(getBitrateString(media->tracks_video[vtid]->bitrate));

            if (media->tracks_video[vtid]->bitrate_mode == BITRATE_CBR)
            {
                ui->label_video_bitratemode->setText("CBR");
                ui->label_video_bitratemode_2->setText("CBR");
            }
            else if (media->tracks_video[vtid]->bitrate_mode == BITRATE_VBR)
            {
                ui->label_video_bitratemode->setText("VBR");
                ui->label_video_bitratemode_2->setText("VBR");
            }
            else if (media->tracks_video[vtid]->bitrate_mode == BITRATE_ABR)
            {
                ui->label_video_bitratemode->setText("ABR");
                ui->label_video_bitratemode_2->setText("ABR");
            }
            else if (media->tracks_video[vtid]->bitrate_mode == BITRATE_CVBR)
            {
                ui->label_video_bitratemode->setText("CVBR");
                ui->label_video_bitratemode_2->setText("CVBR");
            }

            ui->label_video_definition->setText(QString::number(media->tracks_video[vtid]->width) + " x " + QString::number(media->tracks_video[vtid]->height));
            ui->label_video_definition_2->setText(QString::number(media->tracks_video[vtid]->width) + " x " + QString::number(media->tracks_video[vtid]->height));
            ui->label_video_ar->setText(getAspectRatioString(media->tracks_video[vtid]->width, media->tracks_video[vtid]->height));
            ui->label_video_ar_2->setText(getAspectRatioString(media->tracks_video[vtid]->width, media->tracks_video[vtid]->height, true));

            ui->label_video_color_depth->setText(QString::number(media->tracks_video[vtid]->color_depth) + " bits");
            ui->label_video_color_subsampling->setText(QString::number(media->tracks_video[vtid]->color_subsampling));

            if (media->tracks_video[vtid]->color_encoding == CLR_RGB)
                ui->label_video_color_space->setText("RGB");
            else if (media->tracks_video[vtid]->color_encoding == CLR_YCgCo)
                ui->label_video_color_space->setText("YCgCo");
            else // if (media->tracks_video[vtid]->color_encoding == CLR_YCbCr)
                ui->label_video_color_space->setText("YCbCr");

            if (media->tracks_video[vtid]->color_matrix == CM_bt470)
                ui->label_video_color_matrix->setText("Rec. 470");
            else if (media->tracks_video[vtid]->color_matrix == CM_bt601)
                ui->label_video_color_matrix->setText("Rec. 601");
            else if (media->tracks_video[vtid]->color_matrix == CM_bt709)
                ui->label_video_color_matrix->setText("Rec. 709");
            else if (media->tracks_video[vtid]->color_matrix == CM_bt2020)
                ui->label_video_color_matrix->setText("Rec. 2020");

            if (media->tracks_video[vtid]->color_subsampling == SS_4444)
                ui->label_video_color_subsampling->setText("4:4:4:4");
            else if (media->tracks_video[vtid]->color_subsampling == SS_444)
                ui->label_video_color_subsampling->setText("4:4:4");
            else if (media->tracks_video[vtid]->color_subsampling == SS_422)
                ui->label_video_color_subsampling->setText("4:2:2");
            else if (media->tracks_video[vtid]->color_subsampling == SS_420)
                ui->label_video_color_subsampling->setText("4:2:0");
            else if (media->tracks_video[vtid]->color_subsampling == SS_411)
                ui->label_video_color_subsampling->setText("4:1:1");
            else if (media->tracks_video[vtid]->color_subsampling == SS_400)
                ui->label_video_color_subsampling->setText("4:0:0");
            else
                ui->label_video_color_subsampling->setText("4:2:0");

            double framerate = media->tracks_video[vtid]->frame_rate;
            if (framerate < 1.0)
            {
                if (media->tracks_video[vtid]->duration_ms && media->tracks_video[vtid]->sample_count)
                {
                    framerate = static_cast<double>(media->tracks_video[vtid]->sample_count / (static_cast<double>(media->tracks_video[vtid]->duration_ms) / 1000.0));
                }
            }
            double frameduration = 1000.0 / framerate; // in ms

            QString samplecount = "<b>" + QString::number(media->tracks_video[vtid]->sample_count) + "</b>";
            QString framecount = "<b>" + QString::number(media->tracks_video[vtid]->frame_count) + "</b>";
            QString samplerepartition;

            if (media->tracks_video[vtid]->frame_count_idr)
            {
                framecount += "      (" + QString::number(media->tracks_video[vtid]->frame_count_idr) + " IDR  /  ";
                framecount += QString::number(media->tracks_video[vtid]->frame_count - media->tracks_video[vtid]->frame_count_idr) + " others)";

                double idr_ratio = static_cast<double>(media->tracks_video[vtid]->frame_count_idr) / static_cast<double>(media->tracks_video[vtid]->sample_count) * 100.0;
                samplerepartition = tr("IDR frames makes <b>") + QString::number(idr_ratio, 'g', 2) + "%</b> " + tr("of the samples");
                samplerepartition += "<br> one every <b>X</b> ms (statistically) ";

                ui->label_video_samplerepart->setText(samplerepartition);
            }

            ui->label_video_samplecount->setText(samplecount);
            ui->label_video_framecount->setText(framecount);

            ui->label_video_framerate->setText(QString::number(framerate) + " fps");

            ui->label_video_framerate_2->setText(QString::number(framerate) + " fps");
            ui->label_video_frameduration->setText(QString::number(frameduration, 'g', 4) + " ms");

            uint64_t rawsize = media->tracks_video[vtid]->width * media->tracks_video[vtid]->height * (media->tracks_video[vtid]->color_depth / 8);
            rawsize *= media->tracks_video[vtid]->sample_count;
            uint64_t ratio = round(static_cast<double>(rawsize) / static_cast<double>(media->tracks_video[vtid]->stream_size));
            ui->label_video_compression_ratio->setText(QString::number(ratio) + ":1");
        }

        // SUBS
        ////////////////////////////////////////////////////////////////////////

        int stid = 0;
        if (media->tracks_subt[stid] != NULL)
        {
            // TODO
        }
    }
    else
    {
        retcode = 0;
    }

    return retcode;
}

void MainWindow::cleanDatas()
{
    //
}

/* ************************************************************************** */

void MainWindow::openExporter()
{
    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        if (!textexporter)
        {
            textexporter = new TextExporter();
        }

        textexporter->setMediaFile(media);
        textexporter->generateDatas(media);
        textexporter->show();
    }
}

void MainWindow::openHexEditor()
{
    QString file;
    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        // Open current MediaFile in HexEditor
        file = QString(media->file_path);

        if (hexeditor)
        {
            hexeditor->loadFile(file);
            hexeditor->show();
        }
        else
        {
            hexeditor = new HexEditor();
            hexeditor->loadFile(file);
            hexeditor->show();
        }
    }
}

void MainWindow::openFourccHelper()
{
    if (fcchelper)
    {
        fcchelper->show();
    }
    else
    {
        fcchelper = new FourccHelper();
        fcchelper->show();
    }
}

void MainWindow::openAboutWindows()
{
    if (aboutwindows)
    {
        aboutwindows->show();
    }
    else
    {
        int minivideo_major, minivideo_minor, minivideo_patch;
        const char *minivideo_builddate, *minivideo_buildtime;
        minivideo_get_infos(&minivideo_major, &minivideo_minor, &minivideo_patch,
                            &minivideo_builddate, &minivideo_buildtime);

        aboutwindows = new AboutWindows();
        aboutwindows->setMinivideoVersion(minivideo_major, minivideo_minor, minivideo_patch,
                                          minivideo_builddate, minivideo_buildtime);
        aboutwindows->show();
    }
}

void MainWindow::About()
{
    QMessageBox about(QMessageBox::Information, tr("About mini_analyser"),
                      tr("<big><b>mini_analyser</b></big> \
                         <p align='justify'>mini_analyser is a software designed \
                         to help you extract the maximum of informations and meta-datas from multimedia files.</p> \
                         <p>This application is part of the MiniVideo framework.</p> \
                         <p>Emeric Grange (emeric.grange@gmail.com)</p>"),
                         QMessageBox::Ok);

    about.setIconPixmap(QPixmap(":/icons/icons/icon.svg"));
    about.exec();
}

void MainWindow::AboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

/* ************************************************************************** */
