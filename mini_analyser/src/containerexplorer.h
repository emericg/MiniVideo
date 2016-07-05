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
 * \file      containerexplorer.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef CONTAINER_EXPLORER_H
#define CONTAINER_EXPLORER_H
/* ************************************************************************** */

// MiniVideo
#include "mediafile_struct.h"

// QHexEdit widget
#include "thirdparty/qhexedit2/qhexedit.h"

#include <QWidget>
#include <QMainWindow>
#include <QResizeEvent>

namespace Ui {
class ContainerExplorer;
}
class QTreeWidgetItem;

class ContainerExplorer : public QMainWindow
{
    Q_OBJECT

    Ui::ContainerExplorer *ui;

    MediaFile_t *media = NULL;
    BitstreamMap_t *track = NULL;
    BitstreamMap_t *tracks[16] = {0};

    QString curFile;
    QFile file;
    QByteArray fileDatas;

    void resizeEvent(QResizeEvent *event);

public:
    explicit ContainerExplorer(QWidget *parent = 0);
    ~ContainerExplorer();

    void loadMedia(const MediaFile_t *media);
    void closeMedia();

    void loadTracks();

public slots:
    void tabSwitch(int intex);
    void loadSamples(int track_id);
    void sampleSelection();
    void sampleSelection(int sample_id);
    void containerSelection();
    void containerSelection(QTreeWidgetItem *item, int column);
    void clearContent();
};

/* ************************************************************************** */
#endif // CONTAINER_EXPLORER_H
