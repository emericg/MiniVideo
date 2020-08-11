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

#include "tabcontainer.h"
#include "ui_tabcontainer.h"

// minianalyser
#include "minivideo_utils_qt.h"

// minivideo library
#include <minivideo.h>
#include <bitstream.h>
#include <depacketizer/depack.h>

#ifdef _MSC_VER
#include <windows.h>
#include <Lmcons.h>
#pragma comment(lib, "User32.lib")
#endif

#include <QLayout>
#include <QLayoutItem>
#include <QFontDatabase>
#include <QByteArray>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>

#define THUMBNAILS_ENABLED 0

tabContainer::tabContainer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabContainer)
{
    ui->setupUi(this);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSwitch(int)));
    connect(ui->tabWidget_tracks, SIGNAL(currentChanged(int)), this, SLOT(loadSamples(int)));

    connect(ui->listWidget_samples, SIGNAL(currentRowChanged(int)), this, SLOT(sampleSelection(int)));
    connect(ui->treeWidget_structure, SIGNAL(itemSelectionChanged()), this, SLOT(containerSelectionChanged()));

    // Preload icons
    icon_atom.addFile(":/img/img/A.png");
    icon_datas.addFile(":/img/img/P.png");
    icon_ext.addFile(":/img/img/E.png");
    icon_track.addFile(":/img/img/T.png");

    // Setup HEX widget
    ui->widget_hex->setReadOnly(true);
    ui->widget_hex->setAddressArea(false);
    ui->widget_hex->setBytesPerLine(16);
    ui->widget_hex->setHexCaps(true);

#if defined(Q_OS_WINDOWS)
    ui->widget_hex->setFont(QFont("Lucida Console", 12));
#elif defined(Q_OS_MACOS)
    ui->widget_hex->setFont(QFont("Andale Mono", 12));
#elif defined(Q_OS_LINUX)
    ui->widget_hex->setFont(QFont("Monospace", 12));
#else
    //int id = QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    //ui->widget_hex->setFont(QFont("DejaVu Sans Mono", 12));
    ui->widget_hex->setFont(QFont("Monospace", 12));
#endif
}

tabContainer::~tabContainer()
{
    closeMedia();

    delete ui;
}

/* ************************************************************************** */

void tabContainer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    // Make sure the scrollAreas don't get wider than our windows
    int newwidth = width() - ui->tabWidget->width() - 12;
    ui->scrollAreaWidgetContents_2->setMaximumWidth(newwidth);
    ui->labelTitle->setMaximumWidth(newwidth);

    // Resize thumbnail (if needed)
    if (ui->label_pic->isVisible())
    {
        previewSample(ui->listWidget_samples->currentRow());
    }
}

void tabContainer::on_tabWidget_currentChanged(int index)
{
    // Save current mode
    if (wrapper)
    {
        wrapper->containerMode = index;
    }
}

void tabContainer::on_tabWidget_tracks_currentChanged(int index)
{
    // Save current track
    if (wrapper)
    {
        wrapper->containerTrack = index;
    }
}

/* ************************************************************************** */

bool tabContainer::loadMedia(MediaWrapper *wrap)
{
    bool status = false;

    closeMedia();

    if (wrap && wrap->media)
    {
        ui->tabWidget->blockSignals(true);
        ui->tabWidget_tracks->blockSignals(true);
        ui->treeWidget_structure->blockSignals(true);
        ui->listWidget_samples->blockSignals(true);

        wrapper = static_cast<MediaWrapper *>(wrap);
        media = static_cast<MediaFile_t *>(wrap->media);

        mediaFile.setFileName(QString::fromUtf8(media->file_path));

        bool structure_loaded = loadXmlFile();
        bool tracks_loaded = loadTracks();

        if (structure_loaded)
        {
            // make sure tab is visible
            bool found = false;
            for (int tabId = 0; tabId < ui->tabWidget->count(); tabId++)
            {
                if (ui->tabWidget->tabText(tabId) == "Structure")
                {
                    found = true;
                }
            }
            if (found == false) ui->tabWidget->addTab(ui->tabStructure, "Structure");
        }
        else
        {
            // delete tab
            for (int tabId = 0; tabId < ui->tabWidget->count(); tabId++)
            {
                if (ui->tabWidget->tabText(tabId) == "Structure")
                {
                    ui->tabWidget->removeTab(tabId);
                }
            }
        }

        if (tracks_loaded)
        {
            // make sure tab is visible
            bool found = false;
            for (int tabId = 0; tabId < ui->tabWidget->count(); tabId++)
            {
                if (ui->tabWidget->tabText(tabId) == "Samples")
                {
                    found = true;
                }
            }
            if (found == false) ui->tabWidget->addTab(ui->tabSamples, "Samples");

            loadSamples(wrapper->containerTrack);
            ui->tabWidget_tracks->setCurrentIndex(wrapper->containerTrack);
        }
        else
        {
            // delete tab
            for (int tabId = 0; tabId < ui->tabWidget->count(); tabId++)
            {
                if (ui->tabWidget->tabText(tabId) == "Samples")
                {
                    ui->tabWidget->removeTab(tabId);
                }
            }
        }

        ui->tabWidget->setCurrentIndex(wrapper->containerMode);
        ui->tabWidget->setVisible(structure_loaded || tracks_loaded);

        if (wrapper->containerMode == 0)
        {
            if (wrapper->containerExplorerAtom >= 0)
                containerSelection(wrapper->containerExplorerAtom);
            else
                containerSelectionEmpty();
        }
        else if (wrapper->containerMode == 1)
        {
            if (wrapper->containerTrackSample >= 0)
                ui->listWidget_samples->setCurrentRow(wrapper->containerTrackSample);
            else
                ui->listWidget_samples->setCurrentRow(0);

            sampleSelection();
        }

        ui->tabWidget->blockSignals(false);
        ui->tabWidget_tracks->blockSignals(false);
        ui->treeWidget_structure->blockSignals(false);
        ui->listWidget_samples->blockSignals(false);

        // Force a resize event, so the scrollAreas don't get wider than our windows
        resizeEvent(nullptr);
    }

    return status;
}

void tabContainer::closeMedia()
{
    media = nullptr;
    wrapper = nullptr;
    track = nullptr;
    for (unsigned i = 0; i < 16; i++)
        tracks[i] = nullptr;

    // Clean the tabWidget_tracks
    ui->tabWidget_tracks->clear();

    // Clean the TreeWidget content
    while (QWidget *item = ui->treeWidget_structure->childAt(0,0))
    {
        delete item;
    }
    ui->treeWidget_structure->clear();

    // Clean the ListWidget content
    while (QWidget *item = ui->listWidget_samples->childAt(0,0))
    {
        delete item;
    }
    ui->listWidget_samples->clear();

    // Clean thumbnails
    clearPreviews();
}

/* ************************************************************************** */

void tabContainer::tabSwitch(int index)
{
    if (index == 1)
    {
        sampleSelection();
    }
    else
    {
        containerSelectionChanged();
    }
}

unsigned tabContainer::loadTracks()
{
    ui->tabWidget_tracks->clear();

    unsigned track_loaded = 0;

    if (media)
    {
        unsigned index = 0;

        for (unsigned i = 0; i < media->tracks_video_count; i++)
        {
            if (media->tracks_video[i] && media->tracks_video[i]->sample_count > 0)
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_video[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_video[i];
                track_loaded++;
            }
        }
        for (unsigned i = 0; i < media->tracks_audio_count; i++)
        {
            if (media->tracks_audio[i] && media->tracks_audio[i]->sample_count > 0)
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_audio[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_audio[i];
                track_loaded++;
            }
        }
        for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
        {
            if (media->tracks_subt[i] && media->tracks_subt[i]->sample_count > 0)
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_subt[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_subt[i];
                track_loaded++;
            }
        }
        for (unsigned i = 0; i < media->tracks_others_count; i++)
        {
            if (media->tracks_others[i] && media->tracks_others[i]->sample_count > 0)
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_others[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_others[i];
                track_loaded++;
            }
        }
    }

    return track_loaded;
}

void tabContainer::loadSamples(int tid)
{
    //qDebug() << "loadSamples(track #" << tid << ")";

    ui->listWidget_samples->clear();

    if (media && tracks[tid])
    {
        track = tracks[tid];

        uint32_t sample_to_load_max = 512000;
        uint32_t sample_to_load = track->sample_count;
        if (sample_to_load > sample_to_load_max)
            sample_to_load = sample_to_load_max;

        uint32_t i = 0;
        for (i = 0; i < sample_to_load; i++)
        {
            if (track->sample_type && track->sample_type[i] != sample_OTHER)
                ui->listWidget_samples->addItem(getSampleTypeString(track->sample_type[i]) + " #" + QString::number(i));
            else
                ui->listWidget_samples->addItem(tr("Sample #") + QString::number(i));
        }

        if (i == sample_to_load_max)
        {
            ui->listWidget_samples->addItem(tr("Maximum of 512k samples reached!"));
        }

        // Force initial selection
        ui->listWidget_samples->setCurrentRow(0);
    }
    else
    {
        track = nullptr;
    }
}

void tabContainer::sampleSelection()
{
    sampleSelection(ui->listWidget_samples->currentRow());
}

void tabContainer::sampleSelection(int sid)
{
    //qDebug() << "sampleSelection(sample #" << sid << ")";

    if (wrapper)
    {
        // save currently selected item
        wrapper->containerTrackSample = sid;
    }

    clearContent();

    if (media && track &&
        sid >= 0 && static_cast<uint32_t>(sid) < track->sample_count)
    {
        int64_t offset = track->sample_offset[sid];
        int64_t size = track->sample_size[sid];

        // Header infos
        ui->widgetHeader->show();
        ui->labelTitle->setText(getSampleTypeString(track->sample_type[sid]) + " #" + QString::number(sid));
        QLabel *lo = new QLabel(tr("> Offset"));
        QLabel *ls = new QLabel(tr("> Size"));

        QLineEdit *doo = new QLineEdit(QString::number(offset));
        doo->setReadOnly(true);
        QLineEdit *ds = new QLineEdit(QString::number(size) + tr(" bytes"));
        ds->setReadOnly(true);

        ui->gridLayout_header->addWidget(ls, 1, 0);
        ui->gridLayout_header->addWidget(lo, 2, 0);
        ui->gridLayout_header->addWidget(ds, 1, 1);
        ui->gridLayout_header->addWidget(doo, 2, 1);

        if (track->sample_pts[sid] >= 0 || track->sample_dts[sid] >= 0)
        {
            QString pts = QString::number(static_cast<double>(track->sample_pts[sid] / 1000.0), 'f', 3) + " ms";
            pts += "   (" + getTimestampPreciseString(track->sample_pts[sid]) + ")";
            if (track->stream_type == stream_VIDEO)
                pts += "   (SMTPE: " + getTimestampSmtpeString(track->sample_pts[sid], track->framerate) + ")";
            QLabel *lp = new QLabel(tr("> PTS"));
            QLineEdit *dp = new QLineEdit(pts);
            dp->setReadOnly(true);
            ui->gridLayout_header->addWidget(lp, 3, 0);
            ui->gridLayout_header->addWidget(dp, 3, 1);

            QString dts = QString::number(static_cast<double>(track->sample_dts[sid] / 1000.0), 'f', 3) + " ms";
            dts += "   (" + getTimestampPreciseString(track->sample_dts[sid]) + ")";
            QLabel *ld = new QLabel(tr("> DTS"));
            QLineEdit *dd = new QLineEdit(dts);
            dd->setReadOnly(true);
            ui->gridLayout_header->addWidget(ld, 4, 0);
            ui->gridLayout_header->addWidget(dd, 4, 1);
        }

        // Content infos
        ui->widgetSamples->show();

        es_sample_t essample_list[16];
        int essample_count = depack_sample(media, track, sid, essample_list);

        if (essample_count)
        {
            for (int i = 0; i < essample_count && i < 16; i++)
            {
                QLabel *a = new QLabel("Sample #" + QString::number(i) + " @ " + QString::number(essample_list[i].offset - track->sample_offset[sid]) + " / " + QString::number(essample_list[i].size) + " bytes");
                QLineEdit *b = new QLineEdit(QString::fromUtf8(essample_list[i].type_str));
                b->setReadOnly(true);

                ui->gridLayout_samples->addWidget(a, i, 0);
                ui->gridLayout_samples->addWidget(b, i, 1);
            }
        }
        else
        {
            QLabel *a = new QLabel("Sample #0 @ 0 / " + QString::number(track->sample_size[sid]) + " bytes");
            QLineEdit *b = new QLineEdit(tr("Unknown..."));
            b->setReadOnly(true);

            ui->gridLayout_samples->addWidget(a, 0, 0);
            ui->gridLayout_samples->addWidget(b, 0, 1);
        }

        // Preview thumbnail
        if (track->sample_type[sid] == sample_VIDEO_SYNC)
        {
            previewSample(sid);
        }

        // HexEditor
        //if (mediaFile.isOpen())
        {
            ui->widget_hex->setData(mediaFile);
            ui->widget_hex->setData(ui->widget_hex->dataAt(offset, size));
        }
    }
}

void tabContainer::previewSample(int sid)
{
#if THUMBNAILS_ENABLED == 1

    if (media && track &&
        sid >= 0 && static_cast<uint32_t>(sid) < track->sample_count &&
        track->sample_type[sid] == sample_VIDEO_SYNC)
    {
        OutputSurface_t *out = nullptr;

        // Searching element in std::map by key (with sample ID).
        if (thumbnails.find(sid) != thumbnails.end())
        {
            out = thumbnails[sid];
        }
        else
        {
            out = minivideo_decode_frame(media, sid);
            if (out)
            {
                // Clean first thumbnail if we have more than x thumbnails already
                if (thumbnails.size() > 4)
                {
                    OutputSurface_t *s = thumbnails.begin()->second;
                    minivideo_destroy_frame(&s);
                    thumbnails.erase(thumbnails.begin());
                }
                thumbnails.insert(std::make_pair(sid, out));
            }
        }

        if (out)
        {
            QImage img((uchar *)out->surface, out->width, out->height, QImage::Format_RGB888);

            double w = (ui->scrollArea_2->width() - 24);
            int h = out->height / (out->width / w);

            ui->label_pic->setPixmap(QPixmap::fromImage(img).scaled(w,h,Qt::KeepAspectRatio));
            ui->label_pic->show();
        }
    }
#else
    Q_UNUSED(sid)
#endif //THUMBNAILS_ENABLED
}

void tabContainer::containerSelectionEmpty()
{
    clearContent();

    if (!media)
        return;

    // Header infos
    ui->labelTitle->setText(QString::fromUtf8(media->file_name) + "." + QString::fromUtf8(media->file_extension));
    QLabel *fpl = new QLabel(tr("File path:"));
    QLabel *fp = new QLabel(QString::fromUtf8(media->file_path));
    QLabel *fsl = new QLabel(tr("File size:"));
    QLineEdit *fs = new QLineEdit(QString::number(media->file_size));
    fs->setReadOnly(true);

    ui->gridLayout_header->addWidget(fpl, 0, 0);
    ui->gridLayout_header->addWidget(fp, 0, 1);
    ui->gridLayout_header->addWidget(fsl, 1, 0);
    ui->gridLayout_header->addWidget(fs, 1, 1);

    // Sample infos
    ui->widgetSamples->hide();

    // HexEditor
    //ui->widget_hex->setData(mediaFile);
}

void tabContainer::containerSelectionChanged()
{
    QTreeWidgetItem *item = ui->treeWidget_structure->currentItem();
    if (!item) return;

    int64_t selected_offset = item->data(0, Qt::UserRole).toLongLong();

    if (wrapper)
    {
        // save currently selected item
        wrapper->containerExplorerAtom = selected_offset;
    }

    containerSelection(selected_offset);
}

void tabContainer::containerSelection(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (!item) return;

    int64_t selected_offset = item->data(0, Qt::UserRole).toLongLong();

    containerSelection(selected_offset);
}

void tabContainer::containerSelection(int64_t selected_offset)
{
    //qDebug() << "containerSelection(offset) " << selected_offset;

    clearContent();

    // we find the item with given offset
    if (ui->treeWidget_structure->currentItem() == nullptr)
    {
        QTreeWidgetItemIterator it(ui->treeWidget_structure);
        while (*it)
        {
          if ((*it)->data(0, Qt::UserRole).toLongLong() == selected_offset)
          {
              ui->treeWidget_structure->setCurrentItem((*it));
            break;
          }
          ++it;
        }
    }

    // we need to find the atom with given offset
    pugi::xml_node eSelected;
    pugi::xml_node root = xmlMapDatas.document_element();

    if (findAtom(root.child("structure"), "off", selected_offset, eSelected))
    {
        //QString selected_title = eSelected.attributeNode("tt").value();
        QString selected_fcc = QString::fromLatin1(eSelected.attribute("fcc").value());
        QString selected_id = QString::fromLatin1(eSelected.attribute("id").value());
        QString selected_guid = QString::fromLatin1(eSelected.attribute("guid").value());
        QString selected_title = QString::fromLatin1(eSelected.attribute("tt").value());
        int selected_size = eSelected.attribute("sz").as_int();
        int selected_version = eSelected.attribute("v").as_int();
        int selected_flag = eSelected.attribute("f").as_int();
        QString selected_uuid = QString::fromLatin1(eSelected.attribute("uuid").value());

        //qDebug() << "Atom :" << selected_title << "@" << selected_offset << "clicked";

        // Set atom title (if it's an attribute of the selected element)
        ////////////////////////////////////////////////////////////////////////
        if (eSelected.attribute("tt"))
        {
            if (selected_fcc.isEmpty() == false)
                ui->labelTitle->setText(selected_title + "  <font color=\"black\">(" + selected_fcc + ")</font>");
            else if (selected_id.isEmpty() == false)
                ui->labelTitle->setText(selected_title + "  <font color=\"black\">(" + selected_id + ")</font>");
            else if (selected_guid.isEmpty() == false)
                ui->labelTitle->setText(selected_title + "  <font color=\"black\">{" + selected_guid + "}</font>");
            else
                ui->labelTitle->setText(selected_title);
        }
        else
        {
            ui->labelTitle->setText(selected_fcc);
        }

        // Set atom type
        ////////////////////////////////////////////////////////////////////////
        int fieldCount = 0;
        QLabel *atom_title = nullptr;
        QString type = QString::fromLatin1(eSelected.attribute("tp").value());
        if (type == "MP4 box")
        {
            atom_title = new QLabel(tr("<b>> MP4/MOV Atom</b>"));
        }
        else if (type == "MP4 fullbox")
        {
            atom_title = new QLabel(tr("<b>> MP4/MOV \"full\" Atom</b>"));
        }
        else if (type == "RIFF header" || type == "RIFF list")
        {
            atom_title = new QLabel(tr("<b>> RIFF List</b>"));
        }
        else if (type == "RIFF chunk")
        {
            atom_title = new QLabel(tr("<b>> RIFF Chunk</b>"));
        }
        else if (type == "EBML")
        {
            atom_title = new QLabel(tr("<b>> EBML Element</b>"));
        }
        else if (type == "ASF obj")
        {
            atom_title = new QLabel(tr("<b>> ASF Object</b>"));
        }
        else if (type == "datas")
        {
            atom_title = new QLabel(tr("<b>> Raw datas</b>"));
        }
        else
        {
            atom_title = new QLabel(tr("<b>> Unknown container element</b>"));
        }

        // Set atom settings
        ////////////////////////////////////////////////////////////////////////
        QLabel *atom_offset_label = new QLabel(tr("Offset"));
        QLineEdit *atom_offset_data = new QLineEdit(QString::number(selected_offset));
        atom_offset_data->setReadOnly(true);
        //atom_offset_data->setMaximumWidth(256);
        QLabel *atom_size_label = new QLabel("Size");
        QLineEdit *atom_size_data = new QLineEdit(QString::number(selected_size));
        atom_size_data->setReadOnly(true);
        //atom_size_data->setMaximumWidth(256);

        ui->gridLayout_header->addWidget(atom_title, fieldCount++, 0, 1, 4);
        ui->gridLayout_header->addWidget(atom_offset_label, fieldCount, 0);
        ui->gridLayout_header->addWidget(atom_offset_data, fieldCount, 1);
        ui->gridLayout_header->addWidget(atom_size_label, fieldCount, 2);
        ui->gridLayout_header->addWidget(atom_size_data, fieldCount++, 3);

        if (type == "MP4 fullbox")
        {
            QLabel *atom_version_label = new QLabel(tr("Version"));
            QLineEdit *atom_version_data = new QLineEdit(QString::number(selected_version));
            atom_version_data->setReadOnly(true);
            //atom_version_data->setMaximumWidth(256);
            QLabel *atom_flag_label = new QLabel(tr("Flag"));
            QLineEdit *atom_flag_data = new QLineEdit(QString::number(selected_flag));
            atom_flag_data->setReadOnly(true);
            //atom_flag_data->setMaximumWidth(256);

            ui->gridLayout_header->addWidget(atom_version_label, fieldCount, 0);
            ui->gridLayout_header->addWidget(atom_version_data, fieldCount, 1);
            ui->gridLayout_header->addWidget(atom_flag_label, fieldCount, 2);
            ui->gridLayout_header->addWidget(atom_flag_data, fieldCount++, 3);
        }
        if (selected_uuid.isEmpty() == false)
        {
            QLabel *atom_uuid_label = new QLabel(tr("UUID"));
            QLineEdit *atom_uuid_data = new QLineEdit(selected_uuid);
            atom_uuid_data->setReadOnly(true);

            ui->gridLayout_header->addWidget(atom_uuid_label, fieldCount, 0);
            ui->gridLayout_header->addWidget(atom_uuid_data, fieldCount++, 1, 1, 4);
        }

        // Set sample settings
        ////////////////////////////////////////////////////////////////////////

        ui->widgetSamples->hide();

        // Parse element and set atom fields
        ////////////////////////////////////////////////////////////////////////

        for (pugi::xml_node e = eSelected.first_child(); e; e = e.next_sibling())
        {
            if (strncmp(e.name(), "title", 5) == 0)
            {
                // Set atom title (if it's a field of the selected element)
                ui->labelTitle->setText(QString::fromLatin1(e.child_value()) + "  <font color=\"black\">(" + selected_fcc + ")</font>");
            }
            else if (strncmp(e.name(), "desc", 4) == 0)
            {
                QLabel *fl = new QLabel(QString::fromLatin1(e.child_value()));
                ui->gridLayout_content->addWidget(fl, fieldCount++, 1);
            }
            else if (strncmp(e.name(), "spacer", 4) == 0)
            {
                QLabel *fl = new QLabel("<strong>" + QString::fromLatin1(e.child_value()) + "</strong>");
                fl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                QLabel *sp = new QLabel();

                ui->gridLayout_content->addWidget(fl, fieldCount, 0);
                ui->gridLayout_content->addWidget(sp, fieldCount++, 1);
            }
            else if (strcmp(e.name(), "a"))
            {
                QLabel *fl = new QLabel(QString::fromUtf8(e.name()));
                if (e.attribute("index"))
                    fl->setText(fl->text() + " #" + QString::fromUtf8(e.attribute("index").value()));

                QString value = QString::fromUtf8(e.child_value());
                if (e.attribute("string"))
                {
                    value.replace("&quot", "\"");
                    value.replace("&apos", "'");
                    value.replace("&lt", "<");
                    value.replace("&gt", ">");
                    value.replace("&amp", "&");
                }

                QLineEdit *fv = new QLineEdit(value);
                if (e.attribute("unit"))
                    fv->setText(fv->text() + "  (unit: " + QString::fromUtf8(e.attribute("unit").value()) + ")");
                if (e.attribute("note"))
                    fv->setText(fv->text() + "  (note: " + QString::fromUtf8(e.attribute("note").value()) + ")");

                fl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                fv->setReadOnly(true);

                ui->gridLayout_content->addWidget(fl, fieldCount, 0);
                ui->gridLayout_content->addWidget(fv, fieldCount++, 1);
            }
        }

        // HexEditor
        //if (mediaFile.isOpen())
        {
            ui->widget_hex->setData(mediaFile);
            ui->widget_hex->setData(ui->widget_hex->dataAt(selected_offset, selected_size));
        }
    }
}

/* ************************************************************************** */

/**
 * Helper function. Deletes all child widgets of the given layout @a item.
 */
void deleteChildWidgets(QLayoutItem *item) {
    if (item->layout()) {
        // Process all child items recursively.
        for (int i = 0; i < item->layout()->count(); i++) {
            deleteChildWidgets(item->layout()->itemAt(i));
        }
    }
    delete item->widget();
}

/**
 * Helper function. Removes all layout items within the given @a layout
 * which either span the given @a row or @a column. If @a deleteWidgets
 * is true, all concerned child widgets become not only removed from the
 * layout, but also deleted.
 */
void remove(QGridLayout *layout, int row, int column, bool deleteWidgets) {
    // We avoid usage of QGridLayout::itemAtPosition() here to improve performance.
    for (int i = layout->count() - 1; i >= 0; i--) {
        int r, c, rs, cs;
        layout->getItemPosition(i, &r, &c, &rs, &cs);
        if ((r <= row && r + rs - 1 >= row) || (c <= column && c + cs - 1 >= column)) {
            // This layout item is subject to deletion.
            QLayoutItem *item = layout->takeAt(i);
            if (deleteWidgets) {
                deleteChildWidgets(item);
            }
            delete item;
        }
    }
}

/**
 * Removes all layout items on the given @a row from the given grid
 * @a layout. If @a deleteWidgets is true, all concerned child widgets
 * become not only removed from the layout, but also deleted. Note that
 * this function doesn't actually remove the row itself from the grid
 * layout, as this isn't possible (i.e. the rowCount() and row indices
 * will stay the same after this function has been called).
 */
void removeRow(QGridLayout *layout, int row, bool deleteWidgets) {
    remove(layout, row, -1, deleteWidgets);
    layout->setRowMinimumHeight(row, 0);
    layout->setRowStretch(row, 0);
}

/**
 * Removes all layout items on the given @a column from the given grid
 * @a layout. If @a deleteWidgets is true, all concerned child widgets
 * become not only removed from the layout, but also deleted. Note that
 * this function doesn't actually remove the column itself from the grid
 * layout, as this isn't possible (i.e. the columnCount() and column
 * indices will stay the same after this function has been called).
 */
void removeColumn(QGridLayout *layout, int column, bool deleteWidgets) {
    remove(layout, -1, column, deleteWidgets);
    layout->setColumnMinimumWidth(column, 0);
    layout->setColumnStretch(column, 0);
}

void tabContainer::clearContent()
{
    //qDebug() << "tabContainer::clearContent()";

    ui->label_pic->hide();

    for (int i = 0;  i < ui->gridLayout_header->columnCount(); i++)
        removeColumn(ui->gridLayout_header, i, true);
    for (int i = 0;  i < ui->gridLayout_samples->columnCount(); i++)
        removeColumn(ui->gridLayout_samples, i, true);
    for (int i = 0;  i < ui->gridLayout_content->columnCount(); i++)
        removeColumn(ui->gridLayout_content, i, true);
}

void tabContainer::clearPreviews()
{
    //qDebug() << "tabContainer::clearPreviews(" << thumbnails.size() << ")";

    for (auto thumb: thumbnails)
    {
        OutputSurface_t *s = (thumb.second);

        free(s->surface);
        s->surface = nullptr;
        delete s;
    }

    thumbnails.clear();
}

/* ************************************************************************** */

bool tabContainer::loadXmlFile()
{
    //qDebug() << "loadXmlFile()";
    bool status = true;

    ui->treeWidget_structure->clear();
    xmlMapFile.close();

    if (!media)
        return false;

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    QString filename;
#if defined(_MSC_VER)
    wchar_t xmlMapPath[256] = {0};
    wchar_t tempdir_mv[256] = {0};
    if (GetTempPath(256, tempdir_mv) != 0)
    {
        wsprintf(xmlMapPath, L"%s\\%s_mapped.xml", tempdir_mv, media->file_name);
    }

    filename = QString::fromWCharArray(tempdir_mv) ;
    filename += "\\";
    filename += media->file_name;
    filename += "_mapped.xml";
#elif defined(__MINGW32__) || defined(__MINGW64__)
    char xmlMapPath[256] = {0};
    char *tempdir = getenv("TEMP");
    if (tempdir)
    {
        snprintf(xmlMapPath, 256, "%s\\%s", tempdir, media->file_name);
    }
#endif // _MSC_VER

    // Load XML file (fallback from windows file path)
    xmlMapFile.setFileName(filename);

    if (xmlMapFile.exists() == false ||
        xmlMapFile.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "xmlFile.open(" << filename << ") > error";
        status = false;
    }

#else

    // Load XML file (from given file descriptor)
    if (media->container_mapper_fd == nullptr ||
        xmlMapFile.open(media->container_mapper_fd, QIODevice::ReadOnly) == false)
    {
        status = false;
        qDebug() << "xmlFile.open(FILE*) > error";
    }

    // Load XML file (fallback from file path / DEPRECATED)
    if (status == false)
    {
        QString filename = "/tmp/" + QString::fromUtf8(media->file_name) + "_mapped.xml";
        xmlMapFile.setFileName(filename);

        if (xmlMapFile.exists() == false ||
            xmlMapFile.open(QIODevice::ReadOnly) == false)
        {
            qDebug() << "xmlFile.open(" << filename << ") > error";
            status = false;
        }
    }
#endif

    if (status == true)
    {
        xmlMapFile.seek(0);
        char *b = static_cast<char *>(pugi::get_memory_allocation_function()(xmlMapFile.size()));
        xmlMapFile.read(b, xmlMapFile.size());

        pugi::xml_parse_result result = xmlMapDatas.load_buffer_inplace_own(b, xmlMapFile.size());
        if (!result)
        {
            qDebug() << "xmlFile parsed with errors";
            qDebug() << "Error description: " << result.description() << "(error at [..." << (result.offset) << "]";
        }

        // Actual XML data parsing
        pugi::xml_node root = xmlMapDatas.document_element();

        pugi::xml_node fileNode = xmlMapDatas.child("file");
        xmlFileParser(fileNode);

        pugi::xml_node headerNode = root.child("header");
        xmlHeaderParser(headerNode);

        pugi::xml_node structureNode = root.child("structure");
        xmlStructureParser(structureNode);
    }

    return status;
}

bool tabContainer::xmlFileParser(pugi::xml_node &root)
{
    bool status = false;

    if (root.empty() == false)
    {
        //qDebug() << "xmlFileParser()";

        if (root.attribute("xmlMapper"))
        {
            //qDebug() << " - xmlMapper:" << root.attribute("xmlMapper").value();
            if (strcmp(root.attribute("xmlMapper").value(), "0.2") == 0)
                status = true;
        }

        //if (root.attribute("minivideo")) qDebug() << " - minivideo:" << root.attribute("minivideo").value();
    }

    return status;
}

void tabContainer::xmlHeaderParser(pugi::xml_node &root)
{
    if (root.empty() == false)
    {
        //qDebug() << "xmlHeaderParser()";

        for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling())
        {
            //qDebug() << " -" << e.name() << ":" << e.child_value();
        }
    }
}

void tabContainer::xmlStructureParser(pugi::xml_node &root)
{
    if (root.empty() == false)
    {
        //qDebug() << "xmlStructureParser()";

        for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling())
        {
            if (e)
            {
                if (strcmp(e.name(), "a") == 0)
                {
                    xmlAtomParser(e, nullptr);
                }
                else
                {
                    qDebug() << "not an atom? " << e.name(); // structure fields parsing
                }
            }
        }
    }
}

void tabContainer::xmlAtomParser(pugi::xml_node &a, QTreeWidgetItem *item)
{
    QString fcc = QString::fromLatin1(a.attribute("fcc").value());
    QString id = QString::fromLatin1(a.attribute("id").value());
    QString title = QString::fromLatin1(a.attribute("tt").value());
    //QString type = QString::fromLatin1(a.attribute("tp").value());
    QString add = QString::fromLatin1(a.attribute("add").value());
    QString offset = QString::fromLatin1(a.attribute("off").value());

    //qDebug() << "> xmlAtomParser() >" << fcc << id;

    QTreeWidgetItem *child_item;
    if (item)
        child_item = new QTreeWidgetItem(item);
    else
        child_item = new QTreeWidgetItem(ui->treeWidget_structure);

    if (child_item)
    {
        child_item->setData(0, Qt::UserRole, offset);
        if (add == "private")
            child_item->setIcon(0, icon_datas);
        else if (add == "track")
            child_item->setIcon(0, icon_track);
        else
            child_item->setIcon(0, icon_atom);

        if (fcc.isEmpty())
            child_item->setText(0, title);
        else if (id.isEmpty())
            child_item->setText(0, fcc);
        else
            child_item->setText(0, "{error}");

        for (pugi::xml_node e = a.first_child(); e; e = e.next_sibling())
        {
            if (e)
            {
                if (strcmp(e.name(), "a") == 0)
                {
                    // Don't expand everything
                    if (fcc != "trak" && fcc != "moof" &&
                        title != "Cluster" && title != "Cues" && title != "Tags")
                    {
                        child_item->setExpanded(true);
                    }

                    if (add == "track")
                        child_item->setIcon(0, icon_track);
                    else
                        child_item->setIcon(0, icon_ext);

                    xmlAtomParser(e, child_item);
                }
                else
                {
                    //qDebug() << "s " << e.name(); // structure fields parsing
                }
            }
        }
    }
}

bool tabContainer::findAtom(const pugi::xml_node &elem, const QString &attr, int64_t value, pugi::xml_node &foundElement)
{
    bool status = false;

    for (pugi::xml_node atom: elem.children("a"))
    {
        if (atom)
        {
            if (atom.attribute("off").as_llong() == value)
            {
                foundElement = atom;
                return true;
            }

            if (findAtom(atom, attr, value, foundElement) == true)
                return true;
        }
    }

    return  status;
}
