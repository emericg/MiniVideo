/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
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
 * \file      tabdev.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "tabdev.h"
#include "ui_tabdev.h"

// minivideo library
#include <minivideo.h>

#include <cstdint>

#include <QTableWidget>
#include <QDebug>

tabDev::tabDev(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabDev)
{
    ui->setupUi(this);

    ui->tableWidget_stats->setColumnCount(4);
    ui->tableWidget_stats->setSortingEnabled(false);
    ui->tableWidget_stats->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_stats->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_stats->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_stats->verticalHeader()->setVisible(false);

    ui->tableWidget_stats->resizeColumnsToContents();
    ui->tableWidget_stats->horizontalHeader()->resizeSection(0, 200);

    // Apps infos
    int minivideo_major, minivideo_minor, minivideo_patch;
    const char *minivideo_builddate, *minivideo_buildtime;
    bool minivideo_builddebug;
    minivideo_get_infos(&minivideo_major, &minivideo_minor, &minivideo_patch,
                        &minivideo_builddate, &minivideo_buildtime, &minivideo_builddebug);

    // Apps versions
    QString text = "<html>";
    text += tr("MiniAnalyser") + " " + QString::fromLocal8Bit(VERSION_STR);
#ifdef QT_DEBUG
    text += " / <b>DEBUG</b>";
#endif
    text += " / " + tr("builded on:") + " " + QString::fromLocal8Bit(__DATE__) + " at " + QString::fromLocal8Bit(__TIME__);

    text += "<br>" + tr("MiniVideo library") + " " + QString::number(minivideo_major) + "." + QString::number(minivideo_minor) + "-" + QString::number(minivideo_patch);
    if (minivideo_builddebug) text += " / <b>DEBUG</b>";
    text += " / " + tr("builded on:") + " " + minivideo_builddate + " at " + minivideo_buildtime;
    text += "</html>";
    ui->label_stats_appversion->setText(text);
}

tabDev::~tabDev()
{
    delete ui;
}

/* ************************************************************************** */

void tabDev::clean()
{
    ui->groupBox_dev_stats->setTitle(tr("Stats"));
    ui->tableWidget_stats->clearContents();
    ui->tableWidget_stats->setRowCount(0);
}

/* ************************************************************************** */

bool tabDev::addFile(const QString &path, const QString &name,
                     uint64_t processingTime, uint64_t parsingTime, uint64_t parsingMemory)
{
    bool status = true;

    int row = ui->tableWidget_stats->rowCount();

    //qDebug() << "addFile(" << name << ") at"<< row << "rows";

    QTableWidgetItem *itemName = nullptr;
    QTableWidgetItem *itemMem = nullptr;
    QTableWidgetItem *itemPars = nullptr;
    QTableWidgetItem *itemProc = nullptr;

    // Modify
    for (int i = 0; i < ui->tableWidget_stats->rowCount(); i++)
    {
        if (ui->tableWidget_stats->item(i, 0)->toolTip() == path)
        {
            row = i;

            // Table item
            //itemName didn't changed
            itemMem = ui->tableWidget_stats->item(i, 1);
            itemMem->setText(QString::number(parsingMemory / 1024) + " KiB");
            itemPars = ui->tableWidget_stats->item(i, 2);
            itemPars->setText(QString::number(parsingTime) + " ms");
            itemProc = ui->tableWidget_stats->item(i, 3);
            itemProc->setText(QString::number(processingTime) + " ms");
        }
    }

    // Or add
    if (row == ui->tableWidget_stats->rowCount())
    {
        // File count
        fileCount++;
        ui->groupBox_dev_stats->setTitle(tr("Stats") + "  (" + QString::number(fileCount) + tr(" media file(s) loaded)"));

        // Table
        ui->tableWidget_stats->insertRow(row);

        // Table item
        itemName = new QTableWidgetItem(name);
        itemName->setToolTip(path);
        itemMem = new QTableWidgetItem(QString::number(parsingMemory / 1024) + " KiB");
        itemPars = new QTableWidgetItem(QString::number(parsingTime) + " ms");
        itemProc = new QTableWidgetItem(QString::number(processingTime) + " ms");

        ui->tableWidget_stats->setItem(row, 0, itemName);
        ui->tableWidget_stats->setItem(row, 1, itemMem);
        ui->tableWidget_stats->setItem(row, 2, itemPars);
        ui->tableWidget_stats->setItem(row, 3, itemProc);
    }

    return status;
}

bool tabDev::removeFile(const QString &path)
{
    bool status = false;

    for (int i = 0; i < ui->tableWidget_stats->rowCount(); i++)
    {
        if (ui->tableWidget_stats->item(i, 0)->toolTip() == path)
        {
            // File count
            fileCount--;
            ui->groupBox_dev_stats->setTitle(tr("Stats") + "  (" + QString::number(fileCount) + tr(" media file(s) loaded)"));

            // Table
            ui->tableWidget_stats->removeRow(i);
            status = true;

            break;
        }
    }

    return status;
}
