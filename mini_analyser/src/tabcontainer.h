/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of mini_analyser.
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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef TABCONTAINER_H
#define TABCONTAINER_H
/* ************************************************************************** */

#include "mediawrapper.h"

// MiniVideo
#include "minivideo_mediafile.h"

// pugixml
#include "thirdparty/pugixml/pugixml.hpp"

#include <QWidget>
#include <QTreeWidgetItem>
#include <QResizeEvent>
#include <QFile>
#include <QIcon>

namespace Ui {
class tabContainer;
}

class tabContainer : public QWidget
{
    Q_OBJECT

public:
    explicit tabContainer(QWidget *parent = nullptr);
    ~tabContainer();

    bool loadMedia(MediaWrapper *wrapper);
    unsigned loadTracks();
    void closeMedia();

private slots:
    void clearContent();
    void clearPreviews();
    void tabSwitch(int intex);
    void loadSamples(int track_id);
    void previewSample(int sid);
    void sampleSelection();
    void sampleSelection(int sample_id);
    void samplePacketSelection(int64_t selected_offset);
    void containerSelectionEmpty();
    void containerSelectionChanged();
    void containerSelection(QTreeWidgetItem *item, int column);
    void containerSelection(int64_t selected_offset);

    bool loadXmlMap_media();
    bool loadXmlMap_samples();
        bool xmlFileParser(pugi::xml_node &root);
        void xmlHeaderParser(pugi::xml_node &root);
        void xmlStructureParser(pugi::xml_node &root);
        void xmlAtomParser(pugi::xml_node &root, QTreeWidgetItem *item);
        bool xmlAtomFinder(const pugi::xml_node &elem, const QString &attr,
                           int64_t value, pugi::xml_node &foundElement);

private slots:
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tracks_currentChanged(int index);

private:
    Ui::tabContainer *ui;

    MediaFile_t *media = nullptr;
    MediaWrapper *wrapper = nullptr;

    MediaStream_t *track = nullptr;
    MediaStream_t *tracks[64] = { nullptr };

    std::map <unsigned, OutputSurface_t *> thumbnails;

    QFile mediaHexFile;
    QByteArray mediaHexData;

    QFile mediaMapFile;
    pugi::xml_document mediaMapData;

    FILE *samplesMapFd;
    QFile samplesMapFile;
    pugi::xml_document samplesMapData;

    QIcon icon_atom;
    QIcon icon_data;
    QIcon icon_data_ext;
    QIcon icon_ext;
    QIcon icon_track;

    void resizeEvent(QResizeEvent *event);
};

/* ************************************************************************** */
#endif // TABCONTAINER_H
