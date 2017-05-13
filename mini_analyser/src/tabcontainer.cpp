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

#include "utils.h"

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
        static_cast<uint32_t>(sid) < track->sample_count)
    {
        int64_t offset = track->sample_offset[sid];
        int64_t size = track->sample_size[sid];

        // Infos
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

    // Infos
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

    QDomElement eSelected;
    if (findElement(xmlMapDatas.documentElement(), "offset", selected_offset, eSelected) == true)
    {
        QString selected_title = eSelected.attributeNode("title").value();
        QString selected_fcc = eSelected.attributeNode("fcc").value();
        QString selected_id = eSelected.attributeNode("id").value();
        int selected_size = eSelected.attributeNode("size").value().toInt();
        int selected_version = eSelected.attributeNode("version").value().toInt();
        int selected_flag = eSelected.attributeNode("flag").value().toInt();
        QString selected_uuid = eSelected.attributeNode("uuid").value();
        //qDebug() << "Atom fcc:" << selected_fcc << "@" << selected_offset << "clicked";

        // Set atom title (if it's an attribute of the selected element)
        ////////////////////////////////////////////////////////////////////////
        if (eSelected.attributeNode("title").isAttr())
        {
            if (selected_fcc.isEmpty() == false)
                ui->labelTitle->setText(eSelected.attributeNode("title").value() + "  <font color=\"black\">(" + selected_fcc + ")</font>");
            else if (selected_id.isEmpty() == false)
                ui->labelTitle->setText(eSelected.attributeNode("title").value() + "  <font color=\"black\">(" + selected_id + ")</font>");
            else
                ui->labelTitle->setText(eSelected.attributeNode("title").value() + "</font>");
        }
        else
        {
            ui->labelTitle->setText(selected_fcc);
        }

        // Set atom type
        ////////////////////////////////////////////////////////////////////////
        int fieldCount = 0;
        QLabel *atom_title = NULL;
        if (eSelected.attributeNode("type").value() == "MP4 box")
        {
            atom_title = new QLabel(tr("<b>> MP4/MOV Atom</b>"));
        }
        else if (eSelected.attributeNode("type").value() == "MP4 fullbox")
        {
            atom_title = new QLabel(tr("<b>> MP4/MOV \"full\" Atom</b>"));
        }
        else if (eSelected.attributeNode("type").value() == "RIFF header" ||
                 eSelected.attributeNode("type").value() == "RIFF list")
        {
            atom_title = new QLabel(tr("<b>> RIFF List</b>"));
        }
        else if (eSelected.attributeNode("type").value() == "RIFF chunk")
        {
            atom_title = new QLabel(tr("<b>> RIFF Chunk</b>"));
        }
        else if (eSelected.attributeNode("type").value() == "EBML element")
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

        if (eSelected.attributeNode("type").value() == "MP4 fullbox")
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
        if (eSelected.attributeNode("uuid").value().isEmpty() == false)
        {
            QLabel *atom_uuid_label = new QLabel(tr("UUID"));
            QLineEdit *atom_uuid_data = new QLineEdit(selected_uuid);
            atom_uuid_data->setReadOnly(true);

            ui->gridLayout_header->addWidget(atom_uuid_label, fieldCount, 0);
            ui->gridLayout_header->addWidget(atom_uuid_data, fieldCount++, 1, 1, 4);
        }

        // Parse element and set atom fields
        ////////////////////////////////////////////////////////////////////////
        QDomNode structure_node = eSelected.firstChild();
        while (structure_node.isNull() == false)
        {
            QDomElement e = structure_node.toElement();
            if (e.isNull() == false)
            {
                if (e.tagName() == "title")
                {
                    // Set atom title (if it's a field of the selected element)
                    ui->labelTitle->setText(e.text() + "  <font color=\"black\">(" + selected_fcc + ")</font>");
                }
                else if (e.tagName() == "desc")
                {
                    QLabel *fl = new QLabel(e.tagName());
                    ui->gridLayout_content->addWidget(fl, fieldCount++, 1);
                }
                else if (e.tagName() != "atom")
                {
                    QLabel *fl = new QLabel(e.tagName());
                    if (e.attributeNode("index").isAttr())
                        fl->setText(fl->text() + " #" + e.attributeNode("index").value());

                    QLineEdit *fv = new QLineEdit(e.text());
                    if (e.attributeNode("unit").isAttr())
                        fv->setText(fv->text() + "  (unit: " + e.attributeNode("unit").value() + ")");
                    if (e.attributeNode("note").isAttr())
                        fv->setText(fv->text() + "  (note: " + e.attributeNode("note").value() + ")");

                    fl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
                    fv->setReadOnly(true);

                    ui->gridLayout_content->addWidget(fl, fieldCount, 0);
                    ui->gridLayout_content->addWidget(fv, fieldCount++, 1);
                }
            }
            structure_node = structure_node.nextSibling();
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

    if (status == true)
    {
        // Make it a QDomDocument
        xmlMapFile.seek(0);
        if (!xmlMapDatas.setContent(&xmlMapFile))
        {
            qDebug() << "xmlDoc.setContent() > error";
            xmlMapFile.close();
            return false;
        }
        xmlMapFile.close();

        // Actual XML data parsing
        QDomNode root_node = xmlMapDatas.documentElement().firstChild();
        while (root_node.isNull() == false)
        {
            QDomElement e = root_node.toElement(); // try to convert the node to an element
            if (e.isNull() == false)
            {
                if (e.tagName() == "header")
                {
                    xmlHeaderParser(e);
                }
                else if (e.tagName() == "structure")
                {
                    xmlStructureParser(e);
                }
                else
                {
                    // qDebug() << "[0]" << qPrintable(e.tagName()); // Unknown element...
                }
            }
            root_node = root_node.nextSibling();
        }
    }

    return status;
}

void tabContainer::xmlHeaderParser(QDomNode &root)
{
    //qDebug() << "xmlHeaderParser() >>>" << root.toElement().tagName();

    QDomNode header_node = root.firstChild();
    while (header_node.isNull() == false)
    {
        QDomElement e = header_node.toElement();
        if (e.isNull() == false)
        {
            //qDebug() << "h " << qPrintable(e.tagName()); // structure fields parsing
        }
        header_node = header_node.nextSibling();
    }
}

void tabContainer::xmlStructureParser(QDomNode &root)
{
    //qDebug() << "xmlStructureParser() >>>" << root.toElement().tagName();

    QDomNode structure_node = root.firstChild();
    while (structure_node.isNull() == false)
    {
        QDomElement e = structure_node.toElement();
        if (e.isNull() == false)
        {
            if (e.tagName() == "atom")
            {
                xmlAtomParser(e, nullptr);
            }
            else
            {
                //qDebug() << "s " << qPrintable(e.tagName()); // structure fields parsing
            }
        }
        structure_node = structure_node.nextSibling();
    }
}

void tabContainer::xmlAtomParser(QDomNode &root, QTreeWidgetItem *item)
{
    QString fcc = root.toElement().attributeNode("fcc").value();
    QString id = root.toElement().attributeNode("id").value();
    QString title = root.toElement().attributeNode("title").value();
    QString offset = root.toElement().attributeNode("offset").value();

    //qDebug() << "> xmlAtomParser() >" << fcc;

    QTreeWidgetItem *child_item;
    if (fcc.isEmpty() == false)
        child_item = createChildItem(item, fcc, offset);
    else if (id.isEmpty() == false)
        child_item = createChildItem(item, title, offset);

    child_item->setIcon(0, QIcon(":/img/img/C.png"));
    if (fcc != "trak" && fcc != "moof") // don't expand tracks && moof
    {
        ui->treeWidget->setItemExpanded(child_item, true);
    }

    QDomNode structure_node = root.firstChild();
    while (structure_node.isNull() == false)
    {
        QDomElement e = structure_node.toElement();
        if (e.isNull() == false)
        {
            if (e.tagName() == "atom")
            {
                xmlAtomParser(e, child_item);
                if (fcc == "trak" || fcc == "strl")
                    child_item->setIcon(0, QIcon(":/img/img/T.png"));
                else
                    child_item->setIcon(0, QIcon(":/img/img/L.png"));
            }
/*
            else if (e.tagName() == "title")
            {
                child_item->setText(0, child_item->text(0) + " (" + e.text() + ")");
            }
*/
            else
            {
                //qDebug() << "a " << qPrintable(e.tagName()); // ATOM fields parsing
            }
        }
        structure_node = structure_node.nextSibling();
    }
}

QTreeWidgetItem *tabContainer::createChildItem(QTreeWidgetItem *item, QString &fcc, QString &offset)
{
    QTreeWidgetItem *childItem;
    if (item)
    {
        childItem = new QTreeWidgetItem(item);
    }
    else
    {
        childItem = new QTreeWidgetItem(ui->treeWidget);
    }
    childItem->setData(0, Qt::UserRole, offset);
    childItem->setText(0, fcc);

    return childItem;
}

void tabContainer::findElementsWithAttribute(const QDomElement &elem, const QString &attr, QList<QDomElement> &foundElements)
{
    if (elem.attributes().contains(attr))
        foundElements.append(elem);

    QDomElement child = elem.firstChildElement();
    while (!child.isNull())
    {
        findElementsWithAttribute(child, attr, foundElements);
        child = child.nextSiblingElement();
    }
}

bool tabContainer::findElement(const QDomElement &elem, const QString &attr, int value, QDomElement &foundElement)
{
    bool status = false;
    Q_UNUSED(elem);

    QList<QDomElement> eCandidates;
    findElementsWithAttribute(xmlMapDatas.documentElement(), attr, eCandidates);

    for (QDomElement e: eCandidates)
    {
        if (value == e.attributeNode(attr).value().toInt())
        {
            foundElement = e;
            status = true;
            break;
        }
    }

    return status;
}
