/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
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
 * \file      tabcontainer.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#include "tabcontainer.h"
#include "ui_tabcontainer.h"

// minianalyser
#include "utils.h"

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

tabContainer::tabContainer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabContainer)
{
    ui->setupUi(this);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSwitch(int)));
    connect(ui->tabWidget_tracks, SIGNAL(currentChanged(int)), this, SLOT(loadSamples(int)));

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(sampleSelection(int)));
    connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(containerSelectionChanged()));

    // Preload icons
    icon_atom.addFile(":/img/img/A.png");
    icon_ext.addFile(":/img/img/E.png");
    icon_track.addFile(":/img/img/T.png");

    // Setup HEX widget
    ui->widget_hex->setReadOnly(true);
#ifdef Q_OS_LINUX
    /*int id =*/ QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    ui->widget_hex->setFont(QFont("DejaVu Sans Mono", 11));
#endif
#ifdef Q_OS_OSX
    ui->widget_hex->setFont(QFont("Andale Mono", 14));
#endif
#ifdef Q_OS_WIN32
    ui->widget_hex->setFont(QFont("Lucida Console", 11));
#endif
}

tabContainer::~tabContainer()
{
    delete ui;
}

void tabContainer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    // Make sure the scrollAreas don't get wider than our windows
    int newwidth = this->width() - ui->tabWidget->width() - 12;
    ui->scrollAreaWidgetContents_2->setMaximumWidth(newwidth);
    ui->labelTitle->setMaximumWidth(newwidth);
}

void tabContainer::loadMedia(const MediaFile_t *media)
{
    this->media = (MediaFile_t *)media;

    if (media)
    {
        mediaFile.setFileName(QString::fromLocal8Bit(media->file_path));

        setWindowTitle(tr("Container Explorer: ") + QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));

        if (!ui->widget_hex->setData(mediaFile))
        {
            return;
        }

        loadXmlFile();
        loadTracks();
        loadSamples(0);
        containerSelectionEmpty();

        // Force a resize event, so the scrollAreas don't get wider than our windows
        resizeEvent(NULL);
    }
}

void tabContainer::closeMedia()
{
    //
    media = nullptr;
    track = nullptr;
    for (unsigned i = 0; i < 16; i++)
        tracks[i] = nullptr;

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
}

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

void tabContainer::loadTracks()
{
    ui->tabWidget_tracks->clear();

    if (media)
    {
        unsigned index = 0;

        for (unsigned i = 0; i < media->tracks_video_count; i++)
        {
            if (media->tracks_video[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_video[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_video[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_audio_count; i++)
        {
            if (media->tracks_audio[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_audio[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_audio[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
        {
            if (media->tracks_subt[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_subt[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_subt[i];
            }
        }
        for (unsigned i = 0; i < media->tracks_others_count; i++)
        {
            if (media->tracks_others[i])
            {
                QWidget *placeholder = new QWidget();

                ui->tabWidget_tracks->addTab(placeholder, getTrackTypeString(media->tracks_others[i]) + " #" + QString::number(i));
                tracks[index++] = media->tracks_others[i];
            }
        }
    }
}

void tabContainer::loadSamples(int tid)
{
    //qDebug() << "loadSamples(track #" << tid << ")";

    ui->listWidget->clear();

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
                ui->listWidget->addItem(getSampleTypeString(track->sample_type[i]) + " #" + QString::number(i));
            else
                ui->listWidget->addItem(tr("Sample #") + QString::number(i));
        }

        if (i == sample_to_load_max)
        {
            ui->listWidget->addItem(tr("Maximum of 512k samples reached!"));
        }

        // Force initial selection
        ui->listWidget->setCurrentRow(0);
    }
    else
    {
        track = NULL;
    }
}

void tabContainer::sampleSelection()
{
    sampleSelection(ui->listWidget->currentRow());
}

void tabContainer::sampleSelection(int sid)
{
    //qDebug() << "sampleSelection(sample #" << sid << ")";

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
            pts += "   (" + getTimestampString(track->sample_pts[sid]) + ")";
            QLabel *lp = new QLabel(tr("> PTS"));
            QLineEdit *dp = new QLineEdit(pts);
            dp->setReadOnly(true);
            ui->gridLayout_header->addWidget(lp, 3, 0);
            ui->gridLayout_header->addWidget(dp, 3, 1);

            QString dts = QString::number(static_cast<double>(track->sample_dts[sid] / 1000.0), 'f', 3) + " ms";
            dts += "   (" + getTimestampString(track->sample_dts[sid]) + ")";
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
            for (int i = 0; i < essample_count; i++)
            {
                QLabel *a = new QLabel("Sample #" + QString::number(i) + " @ " + QString::number(essample_list[i].offset - track->sample_offset[sid]) + " / " + QString::number(essample_list[i].size) + " bytes");
                QLineEdit *b = new QLineEdit(QString::fromLocal8Bit(essample_list[i].type_str));
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

        // HexEditor
        ui->widget_hex->setReadOnly(true);
        ui->widget_hex->setData(mediaFile);
        ui->widget_hex->setData(ui->widget_hex->dataAt(offset, size));
    }
}

void tabContainer::containerSelectionEmpty()
{
    clearContent();

    if (!media)
        return;

    // Header infos
    ui->labelTitle->setText(QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));
    QLabel *fpl = new QLabel(tr("File path:"));
    QLabel *fp = new QLabel(QString::fromLocal8Bit(media->file_path));
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
    ui->widget_hex->setReadOnly(true);
    ui->widget_hex->setData(mediaFile);
}

void tabContainer::containerSelectionChanged()
{
    containerSelection(ui->treeWidget->currentItem(), 0);
}

void tabContainer::containerSelection(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item)
        return;

    clearContent();

    int selected_offset = item->data(0, Qt::UserRole).toInt();

    // we need to find the atom with given offset
    pugi::xml_node eSelected;
    pugi::xml_node root = xmlMapDatas.document_element();

    if (findAtom(root.child("structure"), "off", selected_offset, eSelected) == true)
    {
        //QString selected_title = eSelected.attributeNode("tt").value();
        QString selected_fcc = QString::fromLatin1(eSelected.attribute("fcc").value());
        QString selected_id = QString::fromLatin1(eSelected.attribute("id").value());
        QString selected_title = QString::fromLatin1(eSelected.attribute("tt").value());
        int selected_size = eSelected.attribute("sz").as_int();
        int selected_version = eSelected.attribute("version").as_int();
        int selected_flag = eSelected.attribute("flag").as_int();
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
            else
                ui->labelTitle->setText(selected_title + "</font>");
        }
        else
        {
            ui->labelTitle->setText(selected_fcc);
        }

        // Set atom type
        ////////////////////////////////////////////////////////////////////////
        int fieldCount = 0;
        QLabel *atom_title = NULL;
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
            atom_title = new QLabel(tr("<b>> EBML element</b>"));
        }
        else
        {
            atom_title = new QLabel(tr("<b>> Atom</b>"));
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
            else if (strcmp(e.name(), "a"))
            {
                QLabel *fl = new QLabel(QString::fromLatin1(e.name()));
                if (e.attribute("index"))
                    fl->setText(fl->text() + " #" + QString::fromLatin1(e.attribute("index").value()));

                QLineEdit *fv = new QLineEdit(QString::fromLatin1(e.child_value()));
                if (e.attribute("unit"))
                    fv->setText(fv->text() + "  (unit: " + QString::fromLatin1(e.attribute("unit").value()) + ")");
                if (e.attribute("note"))
                    fv->setText(fv->text() + "  (note: " + QString::fromLatin1(e.attribute("note").value()) + ")");

                fl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                fv->setReadOnly(true);

                ui->gridLayout_content->addWidget(fl, fieldCount, 0);
                ui->gridLayout_content->addWidget(fv, fieldCount++, 1);
            }
        }

        // HexEditor
        ui->widget_hex->setReadOnly(true);
        ui->widget_hex->setData(mediaFile);
        ui->widget_hex->setData(ui->widget_hex->dataAt(selected_offset, selected_size));
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
    //qDebug() << "CLEARING CONTENT";
    for (int i = 0; i < 32; i++)
    {
        removeRow(ui->gridLayout_header, i, true);
        removeRow(ui->gridLayout_samples, i, true);
        removeRow(ui->gridLayout_content, i, true);
    }
}

/* ************************************************************************** */

bool tabContainer::loadXmlFile()
{
    //qDebug() << "loadXmlFile()";
    bool status = true;

    if (!media)
        return false;

    ui->treeWidget->clear();
    xmlMapFile.close();

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    QString filename;
    wchar_t xmlMapPath[256] = {0};
#if defined(_MSC_VER)
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
    char *tempdir = getenv("TEMP");
    if (tempdir)
    {
        snprintf(xmlMapPath, 256, "%s\%s", tempdir, media->file_name);
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
    if (media->container_mapper_fd == NULL ||
        xmlMapFile.open(media->container_mapper_fd, QIODevice::ReadOnly) == false)
    {
        status = false;
        qDebug() << "xmlFile.open(FILE*) > error";
    }

    // Load XML file (fallback from file path / DEPRECATED)
    if (status == false)
    {
        QString filename = "/tmp/" + QString::fromLocal8Bit(media->file_name) + "_mapped.xml";
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
        char *b = new char[xmlMapFile.size()];
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
            qDebug() << " - xmlMapper:" << root.attribute("xmlMapper").value();
            if (strcmp(root.attribute("xmlMapper").value(), "0.2") == 0)
                status = true;
        }

        if (root.attribute("minivideo"))
        {
            qDebug() << " - minivideo:" << root.attribute("minivideo").value();
        }
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
            qDebug() << " -" << e.name() << ":" << e.child_value();
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
    QString title = QString::fromLatin1(a.attribute("tt").value()); // TODO not qstring
    QString offset = QString::fromLatin1(a.attribute("off").value()); // TODO not qstring

    //qDebug() << "> xmlAtomParser() >" << fcc << id;

    QTreeWidgetItem *child_item;
    if (item)
        child_item = new QTreeWidgetItem(item);
    else
        child_item = new QTreeWidgetItem(ui->treeWidget);

    if (child_item)
    {
        child_item->setData(0, Qt::UserRole, offset);
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
                    if (fcc != "trak" && fcc != "moof" && title != "Cluster" && title != "Cues" && title != "Tags")
                    {
                        ui->treeWidget->setItemExpanded(child_item, true);
                    }

                    if (fcc == "trak" || fcc == "strl" || title == "Track Entry")
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

bool tabContainer::findAtom(const pugi::xml_node &elem, const QString &attr, int value, pugi::xml_node &foundElement)
{
    bool status = false;

    for (pugi::xml_node atom: elem.children("a"))
    {
        if (atom)
        {
            if (atom.attribute("off").as_int() == value)
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
