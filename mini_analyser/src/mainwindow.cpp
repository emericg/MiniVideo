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

#include <iostream>

#include <QMessageBox>
#include <QFileDialog>

#include <QDropEvent>
#include <QUrl>
#include <QDebug>
#include <QMimeData>

#include <QTimer>

/* ************************************************************************** */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusLabel->hide();

    emptyFileList = true;
    ui->tabWidget->setDisabled(true);

    statusTimer = new QTimer;
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(hideStatus()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadFileDialog()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(AboutQt()));

    connect(ui->file_comboBox, SIGNAL(activated(int)), this, SLOT(printDatas(int)));

    // Accept drag&drop
    setAcceptDrops(true);

    // Debug helper only
    const QString file;
    loadFile(file);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* ************************************************************************** */

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    // Check if the object dropped has an url (and so is a file)
    if (e->mimeData()->hasUrls())
    {
        // TODO Filter by MimeType or file extension
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        const QString &fileName = url.toLocalFile();
        std::cout << "Dropped file: "<< fileName.toStdString() << std::endl;

        loadFile(fileName);
    }
}

/* ************************************************************************** */

void MainWindow::loadFileDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a multimedia file"),
                                                    "", tr("Files (*.*)"));

    std::cout << "Choosed file: "<< fileName.toStdString() << std::endl;

    loadFile(fileName);
}

int MainWindow::loadFile(const QString &file)
{
    int retcode = FAILURE;

    if (file.isEmpty() == false)
    {
        setStatus("Working...", 1, 0);

        retcode = getDatas(file);

        if (retcode == 1)
        {
            if (emptyFileList) // was (videosList.empty() == true)
            {
                ui->file_comboBox->removeItem(0);
                emptyFileList = false;
                ui->tabWidget->setEnabled(true);
            }

            ui->file_comboBox->addItem(file);
            ui->file_comboBox->setCurrentIndex(ui->file_comboBox->count() - 1);
            printDatas(ui->file_comboBox->currentIndex());

            hideStatus();
        }
        else
        {
            setStatus("The following file cannot be opened (UNKNOWN ERROR):\n'" + file + "'", 0, 5000);
        }
    }

    return retcode;
}

void MainWindow::setStatus(const QString &text, int type, int duration)
{
    if (type == FAILURE)
    {
        ui->statusLabel->setStyleSheet("QLabel { border: 1px solid rgb(255, 53, 3);\nbackground: rgba(255, 170, 0, 128); }");
    }
    else //if (type == SUCCESS)
    {
        ui->statusLabel->setStyleSheet("QLabel { border: 1px solid rgb(85, 170, 0);\nbackground: rgba(85, 200, 0, 128); }");
    }

    if (duration > 0)
    {
        statusTimer->setInterval(5000);
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

void MainWindow::closeFile()
{
    // First clean interface
    cleanDatas();

    // Then try to close the file's context
    int fileIndex = ui->file_comboBox->currentIndex();
    if (videosList.empty() == false)
    {
        if ((int)(videosList.size()) >= (fileIndex + 1))
        {
            videosList.erase(videosList.begin() + fileIndex);

            // Remove the entry from the comboBox
            ui->file_comboBox->removeItem(fileIndex);

            // Update comboBox index
            if (ui->file_comboBox->count() > 0)
            {
                ui->file_comboBox->activated(fileIndex-1);
            }
            else // No more file opened?
            {
                ui->file_comboBox->addItem(QIcon(":/icons/icons/gtk-info.svg"), "Drag and drop files here to analyse them!");
                ui->tabWidget->setEnabled(false);
                emptyFileList = true;
            }
        }
    }
}

void MainWindow::About()
{
    QMessageBox about(QMessageBox::Information,
                      tr("About mini_analyser"),
                      tr("<big><b>mini_analyser</b></big><br><br>  mini_analyser is a software designed \
                         to help you extract the maximum of informations and meta-datas from multimedia files.<br><br>\
                         This application is part of the MiniVideo framework.<br><br>\
                         Emeric Grange (emeric.grange@gmail.com)"),
                         QMessageBox::Ok);

    about.setIconPixmap(QPixmap(":/icons/icons/icon.svg"));
    about.exec();
}

void MainWindow::AboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

/* ************************************************************************** */

int MainWindow::getDatas(const QString &file)
{
    int retcode = FAILURE;
    char input_filepath[4096];

    if (file.isEmpty() == false)
    {
        strcpy(input_filepath, file.toLocal8Bit());

        // Create and open the video file
        VideoFile_t *input_video = NULL;
        retcode = minivideo_open(input_filepath, &input_video);

        if (retcode == SUCCESS)
        {
            retcode = minivideo_parse(input_video, true, true, true);

            if (retcode == SUCCESS)
            {
                videosList.push_back(input_video);
            }
            else
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

int MainWindow::printDatas(int fileIndex)
{
    int retcode = 0;

    if ((int)(videosList.size()) >= (fileIndex+1))
    {
        retcode = 1;
        VideoFile_t *video = videosList.at(fileIndex);

        ui->label_2->setText(QString::fromLocal8Bit(video->file_name));
        ui->label_10->setText(QString::fromLocal8Bit(video->file_path));
        ui->label_11->setText(QString::fromLocal8Bit(video->file_extension));
        ui->label_12->setText(getContainerString(video->container, 1));

        QString filesize = "0";
        if (video->file_size < 1024) // < 1 KiB
        {
            filesize = QString::number(video->file_size) + " bytes";
        }
        else if (video->file_size < 1048576) // < 1 MiB
        {
            filesize = QString::number(video->file_size / 1024.0, 'f', 2) + " KiB   /   "
                       + QString::number(video->file_size / 1000, 'f', 2) + " KB   /   ("
                       + QString::number(video->file_size) + " bytes)";
        }
        else if (video->file_size < 1073741824) // < 1 GiB
        {
            filesize = QString::number(video->file_size / 1024.0 / 1024.0, 'f', 2) + " MiB   /   "
                       + QString::number(video->file_size / 1000 / 1000, 'f', 2) + " MB   /   ("
                       + QString::number(video->file_size) + " bytes)";
        }
        else // < 1 GiB
        {
            filesize = QString::number(video->file_size / 1024.0 / 1024.0 / 1024.0, 'f', 2) + " GiB   /   "
                       + QString::number(video->file_size / 1000 / 1000 / 1000, 'f', 2) + " GB   /   ("
                       + QString::number(video->file_size) + " bytes)";
        }
        ui->label_13->setText(filesize);

        //ui->label_14->setText() // duration
        //ui->label_15->setText() // creation date
        //ui->label_16->setText() // creation apps

        // AUDIO
        ////////////////////////////////////////////////////////////////////////

        int atid = 0;
        if (video->tracks_audio[atid] != NULL)
        {
            ui->label_25->setText("0"); // stream id
            ui->label_28->setText(getCodecString(stream_AUDIO, video->tracks_audio[atid]->stream_codec));

            ui->label_53->setText(QString::number(video->tracks_audio[atid]->sampling_rate));
            ui->label_54->setText(QString::number(video->tracks_audio[atid]->channel_count));
        }

        // VIDEO
        ////////////////////////////////////////////////////////////////////////

        int vtid = 0;
        if (video->tracks_video[vtid] != NULL)
        {
            ui->label_33->setText("0"); // stream id
            ui->label_49->setText(getCodecString(stream_VIDEO, video->tracks_video[vtid]->stream_codec));

            ui->label_50->setText(QString::number(video->tracks_video[vtid]->width) + " x " + QString::number(video->tracks_video[vtid]->height));
            ui->label_51->setText("16/9");
            ui->label_52->setText(QString::number(video->tracks_video[vtid]->frame_rate));
            ui->label_55->setText(QString::number(video->tracks_video[vtid]->color_depth));
            ui->label_56->setText(QString::number(video->tracks_video[vtid]->color_subsampling));
        }

        // SUBS
        ////////////////////////////////////////////////////////////////////////

        int stid = 0;
        if (video->tracks_subtitles[stid] != NULL)
        {
            // TODO
        }
    }

    return retcode;
}

void MainWindow::cleanDatas()
{
    //
}

/* ************************************************************************** */
