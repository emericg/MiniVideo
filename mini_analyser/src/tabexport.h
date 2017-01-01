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
 * \file      tabexport.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef TABEXPORT_H
#define TABEXPORT_H

// minivideo library
#include "mediafile_struct.h"

#include <QWidget>
#include <QFile>

namespace Ui {
class tabExport;
}

class tabExport : public QWidget
{
    Q_OBJECT

public:
    explicit tabExport(QWidget *parent = 0);
    ~tabExport();

public slots:
    void clean();
    int loadMedia(const MediaFile_t *media);
    int generateExportDatas();

private slots:
    void saveFileDialog();
    void saveDatas();
    int generateExportDatas_text(bool detailed);
    int generateExportDatas_json(bool detailed);
    int generateExportDatas_xml(bool detailed);
    int generateExportMapping_xml();

private:
    Ui::tabExport *ui;

    // Save current media file
    MediaFile_t *media = nullptr;

    // Datas export feature
    int exportFormat = 0;
    QString exportDatas;
    QFile exportFile;

    typedef enum TextExportFormat_e
    {
        EXPORT_TEXT  = 0,
        EXPORT_XML   = 1,
        EXPORT_JSON  = 2,

    } TextExportFormat_e;
};

#endif // TABEXPORT_H
