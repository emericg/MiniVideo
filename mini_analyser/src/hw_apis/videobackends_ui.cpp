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
 * \file      videobackends_ui.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#include "videobackends_ui.h"
#include "ui_videobackends.h"

#if defined(__linux__)
#if defined(VIDEOBACKEND_VDPAU)
    #include "videobackends_vdpau.h"
#endif
#if defined(VIDEOBACKEND_VAAPI)
    #include "videobackends_vaapi.h"
#endif
#elif defined(__APPLE__)
#if defined(VIDEOBACKEND_VDA)
    #include "videobackends_vda.h"
#endif
#if defined(VIDEOBACKEND_VTB)
    #include "videobackends_vtb.h"
#endif
#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    //
#endif

#include "minivideo_codecs.h"

VideoBackendsUI::VideoBackendsUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoBackendsUI)
{
    ui->setupUi(this);

    ui->comboBox_device_selection->setEnabled(false);
    ui->comboBox_device_selection->setVisible(false);
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

void VideoBackendsUI::setInfos()
{
#if defined(VIDEOBACKEND_VDPAU)
    VideoBackendsVDPAU vdpau;
    VideoBackendInfos infos_vdpau;
    vdpau.load(infos_vdpau);

    ui->label_vdpau_string->setText(infos_vdpau.api_info);
    ui->label_vdpau_version->setText(infos_vdpau.api_version);
    setInfosTable(infos_vdpau, ui->tableWidget_vdpau_decoding, nullptr);
#else
    removeTab("VDPAU");
#endif

#if defined(VIDEOBACKEND_VAAPI)
    VideoBackendsVAAPI vaapi;
    VideoBackendInfos infos_vaapi;
    vaapi.load(infos_vaapi);

    ui->label_vaapi_string->setText(infos_vaapi.api_info);
    ui->label_vaapi_version->setText(infos_vaapi.api_version);
    setInfosTable(infos_vaapi, ui->tableWidget_vaapi_decoding, nullptr);
#else
    removeTab("VA-API");
#endif

#if defined(VIDEOBACKEND_VDA)
    VideoBackendsVDA vda;
    VideoBackendInfos infos_vda;
    vda.load(infos_vda);

    setInfosTable(infos_vda, ui->tableWidget_vda_decoding, nullptr);
#else
    removeTab("Video Decode Acceleration");
#endif

#if defined(VIDEOBACKEND_VTB)
    VideoBackendsVideoToolBox vtb;
    VideoBackendInfos infos_vtb;
    vtb.load(infos_vtb);

    setInfosTable(infos_vtb, ui->tableWidget_vtb_decoding, nullptr);
#else
    removeTab("VideoToolBox");
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)

    // TODO

#endif
/*
    ui->label_as->setVisible(!infos.api_info.isEmpty());
    ui->label_api_string->setVisible(!infos.api_info.isEmpty());
    ui->label_api_string->setText(infos.api_info);
    ui->label_av->setVisible(!infos.api_version.isEmpty());
    ui->label_api_version->setVisible(!infos.api_version.isEmpty());
    ui->label_api_version->setText(infos.api_version);
    ui->label_ds->setVisible(!infos.driver_info.isEmpty());
    ui->label_driver_string->setVisible(!infos.driver_info.isEmpty());
    ui->label_driver_string->setText(infos.driver_info);
    ui->label_dv->setVisible(!infos.driver_version.isEmpty());
    ui->label_driver_version->setVisible(!infos.driver_version.isEmpty());
    ui->label_driver_version->setText(infos.driver_version);
*/
    ui->tabWidget->setCurrentIndex(0);
}

void VideoBackendsUI::removeTab(QString tabName)
{
    for (int tabId = 0; tabId < ui->tabWidget->count(); tabId++)
    {
        if (ui->tabWidget->tabText(tabId) == tabName)
        {
            ui->tabWidget->removeTab(tabId);
        }
    }
}

void VideoBackendsUI::setInfosTable(VideoBackendInfos &infos, QTableWidget *tabDec, QTableWidget *tabEnc)
{
    if (tabDec)
    {
        tabDec->setColumnCount(4);
        tabDec->setSortingEnabled(true);
        tabDec->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tabDec->setSelectionBehavior(QAbstractItemView::SelectRows);
        tabDec->setSelectionMode(QAbstractItemView::SingleSelection);
        tabDec->verticalHeader()->setVisible(false);

        for (unsigned i = 0; i < infos.decodingSupport.size(); i++)
        {
            CodecSupport *c = &infos.decodingSupport.at(i);

            // Table
            int row = tabDec->rowCount();
            tabDec->insertRow(row);

            // Table item
            QTableWidgetItem *item1 = new QTableWidgetItem(getCodecString(stream_VIDEO, (Codecs_e)c->codec, false));
            QTableWidgetItem *item2 = new QTableWidgetItem(getCodecProfileString((CodecProfiles_e)c->profile, false));
            QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(c->max_width));
            QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(c->max_height));

            tabDec->setItem(row, 0, item1);
            tabDec->setItem(row, 1, item2);
            tabDec->setItem(row, 2, item3);
            tabDec->setItem(row, 3, item4);
        }

        tabDec->resizeColumnsToContents();
    }

    if (tabEnc)
    {
        tabEnc->setColumnCount(4);
        tabEnc->setSortingEnabled(true);
        tabEnc->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tabEnc->setSelectionBehavior(QAbstractItemView::SelectRows);
        tabEnc->setSelectionMode(QAbstractItemView::SingleSelection);
        tabEnc->verticalHeader()->setVisible(false);

        for (unsigned i = 0; i < infos.decodingSupport.size(); i++)
        {
            CodecSupport *c = &infos.decodingSupport.at(i);

            // Table
            int row = tabEnc->rowCount();
            tabEnc->insertRow(row);

            // Table item
            QTableWidgetItem *item1 = new QTableWidgetItem(getCodecString(stream_VIDEO, (Codecs_e)c->codec, false));
            QTableWidgetItem *item2 = new QTableWidgetItem(getCodecProfileString((CodecProfiles_e)c->profile, false));
            QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(c->max_width));
            QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(c->max_height));

            tabEnc->setItem(row, 0, item1);
            tabEnc->setItem(row, 1, item2);
            tabEnc->setItem(row, 2, item3);
            tabEnc->setItem(row, 3, item4);
        }

        tabEnc->resizeColumnsToContents();
    }
}
