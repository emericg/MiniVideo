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

#ifndef TABEXPORT_H
#define TABEXPORT_H
/* ************************************************************************** */

// minivideo library
#include "minivideo_mediafile.h"

#include "mediawrapper.h"

#include <QWidget>
#include <QFile>

namespace Ui {
class tabExport;
}

class tabExport : public QWidget
{
    Q_OBJECT

    Ui::tabExport *ui;

    // Save current media file
    MediaFile_t *media = nullptr;
    MediaWrapper *wrapper = nullptr;

    // Data export feature
    QString exportData;
    QFile exportFile;

public:
    explicit tabExport(QWidget *parent = nullptr);
    ~tabExport();

public slots:
    void clean();
    int loadMedia(const MediaWrapper *wrapper);
    int generateExportData();

private slots:
    void saveFileDialog();
    void saveData();

    void on_comboBox_export_modes_currentIndexChanged(int index);
    void on_comboBox_export_formats_currentIndexChanged(int index);
};

/* ************************************************************************** */
#endif // TABEXPORT_H
