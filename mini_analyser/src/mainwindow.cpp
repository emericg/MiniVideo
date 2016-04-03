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
#include <QTimer>

#include <QDropEvent>
#include <QFile>
#include <QUrl>
#include <QMimeData>

#include <iostream>
#include <cmath>

/* ************************************************************************** */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    hexeditor = NULL;
    fcchelper = NULL;
    aboutwindows = NULL;

    emptyFileList = true;
    exportFormat = EXPORT_TEXT;

    ui->statusLabel->hide();
    statusTimer = new QTimer;
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(hideStatus()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadFileDialog()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));
    connect(ui->actionHexEditor, SIGNAL(triggered()), this, SLOT(openHexEditor()));
    connect(ui->actionFourCC, SIGNAL(triggered()), this, SLOT(openFourccHelper()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutWindows()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->file_comboBox, SIGNAL(activated(int)), this, SLOT(printDatas()));

    connect(ui->comboBox_video_selector, SIGNAL(activated(int)), this, SLOT(printVideoDetails()));
    connect(ui->comboBox_audio_selector, SIGNAL(activated(int)), this, SLOT(printAudioDetails()));
    connect(ui->comboBox_sub_selector, SIGNAL(activated(int)), this, SLOT(printSubtitlesDetails()));

    connect(ui->comboBox_export_details, SIGNAL(activated(int)), this, SLOT(generateExportDatas()));
    connect(ui->comboBox_export_details, SIGNAL(activated(int)), this, SLOT(generateExportDatas()));

    connect(ui->pushButton_file_detach, SIGNAL(clicked(bool)), this, SLOT(detachFile()));
    connect(ui->pushButton_file_reload, SIGNAL(clicked(bool)), this, SLOT(reloadFile()));
    connect(ui->pushButton_file_exit, SIGNAL(clicked(bool)), this, SLOT(closeFile()));

    connect(ui->pushButton_export_filechooser, SIGNAL(clicked(bool)), this, SLOT(saveFileDialog()));
    connect(ui->pushButton_export, SIGNAL(clicked(bool)), this, SLOT(saveDatas()));

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
    tabExportText = ui->tabWidget->tabText(6);
    tabExportIcon = ui->tabWidget->tabIcon(6);

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
        std::cout << "Dropped file: "<< fileName.toStdString() << std::endl;

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

/* ************************************************************************** */

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
        for (size_t i = 0; i < mediaList.size(); i++)
        {
            QString name = mediaList.at(i)->file_path;
            if (file == name)
            {
                fileIndex = static_cast<int>(i);

                minivideo_close(&mediaList.at(i));

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
    QWidget *tab_widget_saved = ui->tabWidget->currentWidget();

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

        if (media->tracks_video_count || media->tracks_audio_count)
        {
            // Add the export tab
            ui->tabWidget->addTab(ui->tab_exporter, tabExportIcon, tabExportText);
        }

        // Restore the focus (if the same tab is available)
        for (int i = 0; i < ui->tabWidget->count(); i++)
        {
            if (ui->tabWidget->widget(i) == tab_widget_saved)
            {
                ui->tabWidget->setCurrentIndex(i);
                break;
            }
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
        statusTimer->setInterval(15000);
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

/* ************************************************************************** */

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
