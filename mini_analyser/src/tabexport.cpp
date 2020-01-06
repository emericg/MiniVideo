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
 * \date      2016
 */

#include "tabexport.h"
#include "ui_tabexport.h"

// minivideo library
#include <minivideo.h>

// mini_analyser
#include "minivideo_textexport_qt.h"
#include "minivideo_utils_qt.h"

#include <QMessageBox>
#include <QFontDatabase>
#include <QFileDialog>
#include <QDateTime>
#include <QFile>
#include <QDebug>

tabExport::tabExport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabExport)
{
    ui->setupUi(this);

    connect(ui->comboBox_export_modes, SIGNAL(activated(int)), this, SLOT(generateExportDatas()));

    connect(ui->pushButton_export_filechooser, SIGNAL(clicked(bool)), this, SLOT(saveFileDialog()));
    connect(ui->pushButton_export, SIGNAL(clicked(bool)), this, SLOT(saveDatas()));

    // Monospace fonts for the export tab
#if defined(Q_OS_WINDOWS)
    ui->textBrowser_export->setFont(QFont("Lucida Console", 12));
#elif defined(Q_OS_MACOS)
    ui->textBrowser_export->setFont(QFont("Andale Mono", 12));
#else
    //int id = QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    //ui->textBrowser_export->setFont(QFont("DejaVu Sans Mono", 12));
    ui->textBrowser_export->setFont(QFont("Monospace", 12));
#endif
}

tabExport::~tabExport()
{
    delete ui;
}

/* ************************************************************************** */

void tabExport::clean()
{
    media = nullptr;
    wrapper = nullptr;

    exportDatas.clear();
    //exportFile.close(); // ?

    ui->comboBox_export_modes->setCurrentIndex(0);
    ui->comboBox_export_formats->setCurrentIndex(0);
    ui->lineEdit_export_filename->clear();
    ui->textBrowser_export->clear();
}

/* ************************************************************************** */

void tabExport::on_comboBox_export_modes_currentIndexChanged(int index)
{
    // Save current export mode
    if (wrapper)
    {
        wrapper->exportMode = index;
    }
}

void tabExport::on_comboBox_export_formats_currentIndexChanged(int index)
{
    // Save current export format
    if (wrapper)
    {
        wrapper->exportFormat = index;
    }
}

/* ************************************************************************** */

void tabExport::saveFileDialog()
{
    QString fileExtension = ".txt";
    QString fileType = tr("Files (*.txt)");
    QString filePath = ui->lineEdit_export_filename->text();
    int exportFormat = ui->comboBox_export_formats->currentIndex();

    if (exportFormat == EXPORT_JSON)
    {
        fileExtension = ".json";
        fileType = tr("Files (*.json)");
    }
    else if (exportFormat == EXPORT_XML)
    {
        fileExtension = ".xml";
        fileType = tr("Files (*.xml)");
    }

    filePath = QFileDialog::getSaveFileName(this, tr("Save media informations in a text file"),
                                            filePath, fileExtension);

    if (filePath.isEmpty() == false)
    {
        filePath += fileExtension;
        ui->lineEdit_export_filename->setText(filePath);
    }
}

void tabExport::saveDatas()
{
    if (exportDatas.isEmpty() == false)
    {
        exportFile.setFileName(ui->lineEdit_export_filename->text());

        if (exportFile.exists() == true)
        {
            // Confirmation prompt
            QMessageBox::StandardButton messageReply;
            QString messageText = tr("This file already exist:\n");
            messageText += ui->lineEdit_export_filename->text();
            messageText += tr("\nAre you sure you want to overwrite it?");

            messageReply = QMessageBox::warning(this, tr("Confirm file overwrite"),
                                                messageText,
                                                QMessageBox::Yes | QMessageBox::No);

            if (messageReply == QMessageBox::No)
            {
                return;
            }
        }

        if (exportFile.open(QIODevice::WriteOnly) == true &&
            exportFile.isWritable() == true)
        {
            exportFile.write(exportDatas.toLocal8Bit());
            exportFile.close();
        }
    }
}

/* ************************************************************************** */

int tabExport::loadMedia(const MediaWrapper *wrap)
{
    int status = 1;

    if (wrap && wrap->media)
    {
        ui->comboBox_export_modes->blockSignals(true);
        ui->comboBox_export_formats->blockSignals(true);

        wrapper = (MediaWrapper *)wrap;
        media = (MediaFile_t *)wrap->media;

        status = generateExportDatas();

        ui->comboBox_export_modes->blockSignals(false);
        ui->comboBox_export_formats->blockSignals(false);
    }

    return status;
}

int tabExport::generateExportDatas()
{
    int status = 1;

    if (media)
    {
        // Clear datas
        exportDatas.clear();

        // Output path is file path + another extension
        QString outputFilePath = media->file_path;

        // Read file extension and details
        int exportMode = ui->comboBox_export_modes->currentIndex();
        int exportFormat = ui->comboBox_export_formats->currentIndex();

        if (exportMode == 0 || exportMode == 1)
        {
            ui->comboBox_export_formats->setCurrentIndex(EXPORT_TEXT);
            exportFormat = EXPORT_TEXT;

            if (exportFormat == EXPORT_JSON)
            {
                outputFilePath += ".json";
                status = textExport::generateExportDatas_json(*media, exportDatas, exportMode);
            }
            else if (exportFormat == EXPORT_XML)
            {
                outputFilePath += ".xml";
                status = textExport::generateExportDatas_xml(*media, exportDatas, exportMode);
            }
            else // if (exportFormat == EXPORT_TEXT)
            {
                outputFilePath += ".txt";
                status = textExport::generateExportDatas_text(*media, exportDatas, exportMode);
            }
        }
        else if (exportMode == 2)
        {
            ui->comboBox_export_formats->setCurrentIndex(EXPORT_XML);
            exportFormat = EXPORT_XML;

            {
                outputFilePath += ".xml";
                status = textExport::generateExportMapping_xml(*media, exportDatas);
            }
        }

        // Print it
        ui->lineEdit_export_filename->setText(outputFilePath);
        ui->textBrowser_export->setText(exportDatas);
    }

    return status;
}
