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
 * \file      containerexplorer.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "containerexplorer.h"
#include "ui_explorer.h"
#include "utils.h"

#include <QByteArray>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QDebug>

ContainerExplorer::ContainerExplorer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ContainerExplorer)
{
    ui->setupUi(this);

    ui->widget_hex->setReadOnly(true);

    connect(ui->tabWidget_tracks, SIGNAL(currentChanged(int)), this, SLOT(loadSamples(int)));

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(sampleSelection(int)));
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(containerSelection(QTreeWidgetItem *, int)));
}

ContainerExplorer::~ContainerExplorer()
{
    delete ui;
}

void ContainerExplorer::resizeEvent(QResizeEvent *event)
{
    //ui->widget_hex->resize(event->size());
}

void ContainerExplorer::loadMedia(const MediaFile_t *media)
{
    if (media)
    {
        this->media = (MediaFile_t *)media;
        file.setFileName(QString::fromLocal8Bit(media->file_path));

        qDebug() << "LOADING MEDIA:" << QString::fromLocal8Bit(media->file_path);

        if (!ui->widget_hex->setData(file))
        {
            return;
        }

        loadTracks();
        loadSamples(0);
    }
}

void ContainerExplorer::closeMedia()
{
    // Clean the tabWidget_tracks
    ui->tabWidget_tracks->clear();

    // Clean the TreeWidget content
    while (QWidget *item = ui->treeWidget->childAt(0,0))
    {
        delete item;
    }
    ui->treeWidget->clear();

    // Clean the ListWidget content
    while (QWidget *item = ui->listWidget->childAt(0,0))
    {
        delete item;
    }
    ui->listWidget->clear();

    // Clean the hex view
    ui->widget_hex->close();

    //
    media = NULL;
}

void ContainerExplorer::loadTracks()
{
    ui->tabWidget_tracks->clear();

    qDebug() << "LOADING TRACKS";

    if (media)
    {
        unsigned index = 0;

        for (unsigned i = 0; i < media->tracks_video_count; i++)
        {
            if (media->tracks_video[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, tr("Video #") + QString::number(i));
                tracks[index++] = media->tracks_video[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_audio_count; i++)
        {
            if (media->tracks_audio[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, tr("Audio #") + QString::number(i));
                tracks[index++] = media->tracks_audio[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
        {
            if (media->tracks_subt[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, tr("Subtitles #") + QString::number(i));
                tracks[index++] = media->tracks_subt[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_others_count; i++)
        {
            if (media->tracks_others[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, tr("Other #") + QString::number(i));
                tracks[index++] = media->tracks_others[i];
            }
        }
    }
}

void ContainerExplorer::loadSamples(int tid)
{
    ui->listWidget->clear();

    qDebug() << "loadSamples(track #" << tid << ")";

    if (media && tracks[tid])
    {
        track = tracks[tid];

        for (uint32_t i = 0; i < tracks[tid]->sample_count; i++)
        {
            ui->listWidget->addItem("Sample #" + QString::number(i));
        }
    }
    else
    {
        track = NULL;
    }

    ui->listWidget->setCurrentRow(0);
}

void ContainerExplorer::sampleSelection(int sid)
{
    //qDebug() << "sampleSelection(sample #" << sid << ")";

    clearContent();

    if (track && static_cast<uint32_t>(sid) < track->sample_count)
    {
        int64_t offset = track->sample_offset[sid];
        int64_t size = track->sample_size[sid];
        QString pts = QString::number(static_cast<double>(track->sample_pts[sid] / 1000.0), 'f', 3) + " ms";
        QString dts = QString::number(static_cast<double>(track->sample_dts[sid] / 1000.0), 'f', 3) + " ms";
        pts += "   (" + getTimestampString(track->sample_pts[sid]) + ")";
        dts += "   (" + getTimestampString(track->sample_dts[sid]) + ")";

        // Infos
        ui->labelTitle->setText(tr("Sample #") + QString::number(sid));
        QLabel *lt = new QLabel(tr("> Type"));
        QLabel *lo = new QLabel(tr("> Offset"));
        QLabel *ls = new QLabel(tr("> Size"));

        QLineEdit *dt = new QLineEdit(getSampleTypeString(track->sample_type[sid]));
        dt->setEnabled(false);
        dt->setReadOnly(true);
        QLineEdit *doo = new QLineEdit(QString::number(offset));
        doo->setReadOnly(true);
        QLineEdit *ds = new QLineEdit(QString::number(size) + tr(" bytes"));
        ds->setReadOnly(true);

        ui->gridLayout_content->addWidget(lt, 0, 0);
        ui->gridLayout_content->addWidget(ls, 1, 0);
        ui->gridLayout_content->addWidget(lo, 2, 0);
        ui->gridLayout_content->addWidget(dt, 0, 1);
        ui->gridLayout_content->addWidget(ds, 1, 1);
        ui->gridLayout_content->addWidget(doo, 2, 1);

        if (track->sample_pts[sid] >= 0 && track->sample_dts[sid] >= 0)
        {
            QLabel *lp = new QLabel(tr("> PTS"));
            QLabel *ld = new QLabel(tr("> DTS"));
            QLineEdit *dp = new QLineEdit(pts);
            dp->setReadOnly(true);
            QLineEdit *dd = new QLineEdit(dts);
            dd->setReadOnly(true);

            ui->gridLayout_content->addWidget(lp, 3, 0);
            ui->gridLayout_content->addWidget(ld, 4, 0);
            ui->gridLayout_content->addWidget(dp, 3, 1);
            ui->gridLayout_content->addWidget(dd, 4, 1);
        }

        // HexEditor
        ui->widget_hex->setReadOnly(true);
        ui->widget_hex->setData(file);
        ui->widget_hex->setData(ui->widget_hex->dataAt(offset, size));
    }
}

void ContainerExplorer::containerSelection()
{
    qDebug() << "Container selection";

    clearContent();

    // Infos
    ui->labelTitle->setText(QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));

    // Infos

    // HexEditor
    ui->widget_hex->setReadOnly(true);
    ui->widget_hex->setData(file);
}

void ContainerExplorer::containerSelection(QTreeWidgetItem *item, int column)
{
    qDebug() << "Container selection";

    clearContent();

    // Infos
    ui->labelTitle->setText(QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));

    // Infos

    // HexEditor
    ui->widget_hex->setReadOnly(true);
    ui->widget_hex->setData(file);
}

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

void ContainerExplorer::clearContent()
{
    //qDebug() << "CLEARING CONTENT";
    remove(ui->gridLayout_content, 0, 0, true);
    remove(ui->gridLayout_content, 1, 1, true);
}
