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
#include "hexeditor.h"
#include "containerexplorer.h"
#include "fourcchelper.h"
#include "about.h"

#include <QMainWindow>
#include <vector>

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
    void saveFileDialog();
    void saveDatas();

    int printDatas();
        int printAudioDetails();
        int printVideoDetails();
        int printSubtitlesDetails();
        int printOtherDetails();

    int generateExportDatas();
        int generateExportDatas_text(MediaFile_t *media, bool detailed);
        int generateExportDatas_json(MediaFile_t *media, bool detailed);
        int generateExportDatas_xml(MediaFile_t *media, bool detailed);
        int generateExportDatas_yaml(MediaFile_t *media, bool detailed);

    void xAxisRangeChanged(const QCPRange &newRange);
    void yAxisRangeChanged(const QCPRange &newRange);

    void hideStatus();
    void detachFile();
    void reloadFile(const QString &file);
    void reloadFile();
    void closeFile(const QString &file);
    void closeFile();

    void openExplorer();
    void openHexEditor();
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

    ContainerExplorer *explorer;
    HexEditor *hexeditor;
    FourccHelper *fcchelper;
    AboutWindows *aboutwindows;

    QTimer *statusTimer;
    QString applicationPath;

    bool emptyFileList;
    std::vector <MediaFile_t *> mediaList; // This might need to be smart pointers at some point

    // Text export feature
    int exportFormat;
    QString exportDatas;
    QFile exportFile;

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

    typedef enum TextExportFormat_e
    {
        EXPORT_TEXT  = 0,
        EXPORT_JSON  = 1,
        EXPORT_XML   = 2,
        EXPORT_YAML  = 3

    } TextExportFormat_e;
};

/* ************************************************************************** */
#endif // MAINWINDOW_H
