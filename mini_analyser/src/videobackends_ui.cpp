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
 * \file      videobackends_ui.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#include "videobackends_ui.h"
#include "ui_videobackends.h"

#if defined(__linux__)
#include "videobackends_vdpau.h"
#include "videobackends_vaapi.h"
#elif defined(__APPLE__)
    //
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    //
#endif

#include "minivideo_codecs.h"

VideoBackendsUI::VideoBackendsUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoBackendsUI)
{
    ui->setupUi(this);

#if defined(__linux__)
    //VideoBackendsVDPAU b;
    //b.load(infos);

    VideoBackendsVAAPI a;
    a.load(infos);
#elif defined(__APPLE__)
    //
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    //
#endif

    ui->comboBox->setEnabled(false);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), infos.api_name);

    ui->label_api_string->setText(infos.api_info);
    ui->label_api_version->setText(infos.api_version);
    ui->label_driver_string->setText(infos.driver_info);
    ui->label_driver_version->setText(infos.driver_version);

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->verticalHeader()->setVisible(false);

    for (unsigned i = 0; i < infos.decodingSupport.size(); i++)
    {
        CodecSupport *c = &infos.decodingSupport.at(i);

        // Table
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        // Table item
        QTableWidgetItem *item1 = new QTableWidgetItem(getCodecString(stream_VIDEO, (Codecs_e)c->codec, false));
        QTableWidgetItem *item2 = new QTableWidgetItem(getCodecProfileString((CodecProfiles_e)c->profile, false));
        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(c->max_width));
        QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(c->max_height));

        ui->tableWidget->setItem(row, 0, item1);
        ui->tableWidget->setItem(row, 1, item2);
        ui->tableWidget->setItem(row, 2, item3);
        ui->tableWidget->setItem(row, 3, item4);
    }

    ui->tableWidget->resizeColumnsToContents();
}

VideoBackendsUI::~VideoBackendsUI()
{
    delete ui;
}

void VideoBackendsUI::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
