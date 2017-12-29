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
#include "mediawrapper.h"
#include "fourcchelper.h"
#include "about.h"
#include "hw_apis/videobackends_ui.h"

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

    int setActiveFile();
    int printDatas();
        int printAudioDetails();
        int printVideoDetails();
        int printSubtitlesDetails();
        int printOtherDetails();

    void on_tabWidget_currentChanged(int index);
    void on_comboBox_audio_selector_currentIndexChanged(int index);
    void on_comboBox_video_selector_currentIndexChanged(int index);
    void on_comboBox_sub_selector_currentIndexChanged(int index);
    void xAxisRangeChanged(const QCPRange &newRange);
    void yAxisRangeChanged(const QCPRange &newRange);

    void hideStatus();

    void detachFile();
    void reloadFile();
    void closeFile(const QString &fileToClose);
    void closeFile();
    void closeFiles();

    void openFourccHelper();
    void openVideoBackends();
    void openAbout();
    void About();
    void AboutQt();

public slots:
    void mediaReady(QString mediaPath);

protected:
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow *ui;

    FourccHelper *fcchelper = nullptr;
    AboutWindows *aboutwindows = nullptr;
    VideoBackendsUI *videobackends = nullptr;

    QTimer *statusTimer = nullptr;
    QString applicationPath;

    bool mediaListEmpty = true;
    std::vector <MediaWrapper *> mediaList;
    QString currentMediaLoaded;

    // Saved tabs infos
    QString tabDropZoneText;
    QIcon tabDropZoneIcon;
    QString tabLoadingText;
    QIcon tabLoadingIcon;
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

    QIcon icon_empty;
    QIcon icon_load;
    QIcon icon_movie;
    QIcon icon_music;
    QIcon icon_error;

    MediaWrapper *currentMediaWrapper();
    MediaFile_t *currentMediaFile();

    MediaWrapper *namedMediaWrapper(QString &filePath);
    MediaFile_t *namedMediaFile(QString &filePath);

    void setStatus(const QString &text, int type, int duration = 0);
    void handleComboBox();
    void handleComboBox(const QString &file);
    void handleTabWidget();
    void loadingTab();
    void cleanDatas();
    void cleanGui();
};

/* ************************************************************************** */
#endif // MAINWINDOW_H
