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
#include "hexeditor.h"

#include <QMainWindow>
#include <vector>

/* ************************************************************************** */

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int setAppPath(const QString &path);
    int loadFile(const QString &file);

public slots:
    void loadFileDialog();

private slots:
    int printDatas();
    void closeFile(const QString &file);
    void closeFile();
    void reloadFile(const QString &file);
    void reloadFile();
    void detachFile();
    void hideStatus();
    void openExporter();
    void openHexEditor();
    void openFourCC();
    void About();
    void AboutQt();

protected:
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);

private:
    Ui::MainWindow *ui;
    FourccHelper *fcc;
    HexEditor *hexeditor;
    QTimer *statusTimer;

    QString applicationPath;

    bool emptyFileList;
    std::vector <MediaFile_t *> mediaList;

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

    MediaFile_t *currentMediaFile();
    int analyseFile(const QString &file);

    void handleComboBox(const QString &file);
    void handleTabWidget();
    void setStatus(const QString &text, int type, int duration = 0);
    void cleanDatas();
};

/* ************************************************************************** */
#endif // MAINWINDOW_H
