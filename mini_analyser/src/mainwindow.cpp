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
#include <QWidget>
#include <QTimer>

#include <QDropEvent>
#include <QFile>
#include <QUrl>
#include <QMimeData>

#include <iostream>
#include <chrono>
#include <cmath>

/* ************************************************************************** */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->statusLabel->hide();
    statusTimer = new QTimer;
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(hideStatus()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadFileDialog()));
    connect(ui->actionReload, SIGNAL(triggered()), this, SLOT(reloadFile()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));
    connect(ui->actionFourCC, SIGNAL(triggered()), this, SLOT(openFourccHelper()));
    connect(ui->actionVideoBackends, SIGNAL(triggered()), this, SLOT(openVideoBackends()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->file_comboBox, SIGNAL(activated(int)), this, SLOT(printFile()));

    connect(ui->comboBox_video_selector, SIGNAL(activated(int)), this, SLOT(printVideoDetails()));
    connect(ui->comboBox_audio_selector, SIGNAL(activated(int)), this, SLOT(printAudioDetails()));
    connect(ui->comboBox_sub_selector, SIGNAL(activated(int)), this, SLOT(printSubtitlesDetails()));

    connect(ui->pushButton_file_detach, SIGNAL(clicked(bool)), this, SLOT(detachFile()));
    //connect(ui->pushButton_file_reload, SIGNAL(clicked(bool)), this, SLOT(reloadFile()));
    //connect(ui->pushButton_file_exit, SIGNAL(clicked(bool)), this, SLOT(closeFile()));

    // Save tabs titles and icons
    tabDropZoneText = ui->tabWidget->tabText(0);
    tabDropZoneIcon = ui->tabWidget->tabIcon(0);
    tabLoadingText = ui->tabWidget->tabText(1);
    tabLoadingIcon = ui->tabWidget->tabIcon(1);
    tabInfosText = ui->tabWidget->tabText(2);
    tabInfosIcon = ui->tabWidget->tabIcon(2);
    tabVideoText = ui->tabWidget->tabText(3);
    tabVideoIcon = ui->tabWidget->tabIcon(3);
    tabAudioText = ui->tabWidget->tabText(4);
    tabAudioIcon = ui->tabWidget->tabIcon(4);
    tabSubsText = ui->tabWidget->tabText(5);
    tabSubsIcon = ui->tabWidget->tabIcon(5);
    tabOtherText = ui->tabWidget->tabText(6);
    tabOtherIcon = ui->tabWidget->tabIcon(6);
    tabContainerText = ui->tabWidget->tabText(7);
    tabContainerIcon = ui->tabWidget->tabIcon(7);
    tabExportText = ui->tabWidget->tabText(8);
    tabExportIcon = ui->tabWidget->tabIcon(8);
    tabDevText = ui->tabWidget->tabText(9);
    tabDevIcon = ui->tabWidget->tabIcon(9);

    // "Drop zone" is the default tab when starting up
    handleTabWidget();

    // Loading animation
    //QString anim = ":/img/img/loading.svg";
    //ui->widget_animation->load(anim);

    // Accept video files "drag & drop"
    setAcceptDrops(true);

#ifdef Q_OS_OSX
    ui->file_comboBox->setIconSize(QSize(16,16));
    ui->mainToolBar->setStyleSheet("");
#endif
}

MainWindow::~MainWindow()
{
    closeFiles();

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
        qDebug() << "File dropped:"<< fileName;

        loadFile(fileName);
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    // Make sure the info tab scrollArea don't get wider than our main window
    ui->groupBox_infos->setMaximumWidth(ui->tab_infos->width() - 8);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);

    // Make sure the info tab scrollArea don't get wider than our main window
    ui->groupBox_infos->setMaximumWidth(ui->tab_infos->width() - 8);
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
        setStatus("Working...", SUCCESS, 0);

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

        // Timers
        std::chrono::time_point<std::chrono::steady_clock> start, end;
        std::chrono::time_point<std::chrono::steady_clock> start_parsing, end_parsing;
        start = std::chrono::steady_clock::now();

        // Load file
        start_parsing = std::chrono::steady_clock::now();
        retcode = analyseFile(file);
        end_parsing = std::chrono::steady_clock::now();
        if (retcode == 1)
        {
            handleComboBox(file);
            printFile();
            hideStatus();

            end = std::chrono::steady_clock::now();
            int64_t tp = std::chrono::duration_cast<std::chrono::milliseconds>(end_parsing - start_parsing).count();
            int64_t tt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() - tp;
            MediaFile_t *f = currentMediaFile();
            QString name = QString::fromLocal8Bit(f->file_name) + "." + QString::fromLocal8Bit(f->file_extension);
            ui->tab_dev->addFile(file, name, tt, tp, f->parsingMemory);
        }
        else
        {
            handleComboBox(file);
            printFile();
            hideStatus();

            setStatus("The following file cannot be opened (UNKNOWN ERROR):\n'" + file + "'", FAILURE, 7500);
        }

        // Force a resize event, so the scrollAreas don't get wider than our windows
        resizeEvent(NULL);
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
        strncpy(input_filepath, file.toLocal8Bit(), 4095);

        // Create and open the media file
        MediaFile_t *input_media = NULL;
        retcode = minivideo_open(input_filepath, &input_media);

        if (retcode == SUCCESS)
        {
            retcode = minivideo_parse(input_media, true);
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

void MainWindow::cleanGui()
{
    closeFiles();
    cleanDatas();
    handleTabWidget();
}

void MainWindow::closeFiles()
{
    if (mediaList.empty() == false)
    {
        for (unsigned i = 0; i < mediaList.size(); i++)
        {
            ui->tab_dev->removeFile(mediaList.at(i)->file_path);
            ui->tab_container->closeMedia();
            minivideo_close(&mediaList.at(i));
        }
    }
}

void MainWindow::closeFile(const QString &file)
{
    if (mediaList.empty() == false)
    {
        // Find the index of the given file
        for (unsigned i = 0; i < mediaList.size(); i++)
        {
            QString path = mediaList.at(i)->file_path;
            if (file == path)
            {
                ui->tab_dev->removeFile(path);
                ui->tab_container->closeMedia();

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
    int fileIndex = ui->file_comboBox->currentIndex();

    if (mediaList.empty() == false)
    {
        QString path = mediaList.at(fileIndex)->file_path;

        if ((int)(mediaList.size()) >= (fileIndex + 1))
        {
            ui->tab_dev->removeFile(path);
            ui->tab_container->closeMedia();

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

void MainWindow::reloadFile()
{
    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        // Save the current tab index if we want to restore it later
        QWidget *tab_widget_saved = ui->tabWidget->currentWidget();

        QString current = media->file_path;
        closeFile();
        loadFile(current);

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
            ui->file_comboBox->addItem(QIcon(":/icons_material/icons_material/ic_info_48px.svg"),
                                       "Drag and drop files to analyse them!");
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
            ui->tabWidget->addTab(ui->tab_other, tabOtherIcon, tabOtherText);
        }

        {
            // Add the container tab
            ui->tabWidget->addTab(ui->tab_container, tabContainerIcon, tabContainerText);

            // Add the export tab
            ui->tabWidget->addTab(ui->tab_export, tabExportIcon, tabExportText);

#if ENABLE_DEBUG
            // Add the developer tab
            ui->tabWidget->addTab(ui->tab_dev, tabDevIcon, tabDevText);
#endif  // ENABLE_DEBUG
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
        statusTimer->setInterval(10000);
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

void MainWindow::openVideoBackends()
{
    if (videobackends)
    {
        videobackends->show();
    }
    else
    {
        videobackends = new VideoBackendsUI();
        videobackends->show();
    }
}

void MainWindow::openAbout()
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

    about.setIconPixmap(QPixmap(":/icons/app/icon.png"));
    about.exec();
}

void MainWindow::AboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

/* ************************************************************************** */
