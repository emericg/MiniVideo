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
 * \file      mainwindow.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/* ************************************************************************** */

// minivideo library
#include <minivideo.h>

// minianalyser
#include "fourcchelper.h"
#include "about.h"

#include <vector>

#include <QMainWindow>
#include <QString>
#include <QFile>

/* ************************************************************************** */

namespace Ui {
class MainWindow;
}

class QCPRange;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int setAppPath(const QString &path);
    int loadFile(const QString &file);

private slots:
    void loadFileDialog();

    int printFile();
    int printDatas();
        int printAudioDetails();
        int printVideoDetails();
        int printSubtitlesDetails();
        int printOtherDetails();

    void xAxisRangeChanged(const QCPRange &newRange);
    void yAxisRangeChanged(const QCPRange &newRange);

    void hideStatus();

    void detachFile();
    void reloadFile();
    void closeFile(const QString &file);
    void closeFile();
    void closeFiles();

    void openFourccHelper();
    void openAbout();
    void About();
    void AboutQt();

protected:
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow *ui;

    FourccHelper *fcchelper = nullptr;
    AboutWindows *aboutwindows = nullptr;

    QTimer *statusTimer = nullptr;
    QString applicationPath;

    bool emptyFileList = true;
    std::vector <MediaFile_t *> mediaList;

    // Bitrate graph
    double xRangeMax;
    double yRangeMax;

    // Save tabs
    QString tabDropZoneText;
    QIcon tabDropZoneIcon;
    QString tabInfosText;
    QIcon tabInfosIcon;
    QString tabVideoText;
    QIcon tabVideoIcon;
    QString tabAudioText;
    QIcon tabAudioIcon;
    QString tabSubsText;
    QIcon tabSubsIcon;
    QString tabOtherText;
    QIcon tabOtherIcon;
    QString tabContainerText;
    QIcon tabContainerIcon;
    QString tabExportText;
    QIcon tabExportIcon;
    QString tabDevText;
    QIcon tabDevIcon;

    MediaFile_t *currentMediaFile();
    int analyseFile(const QString &file);

    void setStatus(const QString &text, int type, int duration = 0);
    void handleComboBox(const QString &file);
    void handleTabWidget();
    void cleanDatas();
    void cleanGui();
};

/* ************************************************************************** */
#endif // MAINWINDOW_H
