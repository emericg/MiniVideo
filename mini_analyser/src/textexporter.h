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
 * \file      textexporter.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef TEXTEXPORTER_H
#define TEXTEXPORTER_H
/* ************************************************************************** */

#include "minivideo.h"

#include <QDialog>
#include <QFile>

/* ************************************************************************** */

namespace Ui {
class TextExporter;
}

/*!
 * \enum TextExportFormat_e
 */
typedef enum TextExportFormat_e
{
    EXPORT_TEXT  = 0,
    EXPORT_XML   = 1,
    EXPORT_JSON  = 2,

} TextExportFormat_e;

class TextExporter : public QDialog
{
    Q_OBJECT

public:
    explicit TextExporter(QWidget *parent = 0);
    ~TextExporter();

    void setMediaFile(MediaFile_t *media);
    void generateDatas(MediaFile_t *media);

private slots:
    void saveFileDialog();
    void saveDatas();

private:
    Ui::TextExporter *ui;

    int exportFormat;
    QString exportDatas;
    QFile exportFile;

    void setOutputFile(QString &filePath);
};

/* ************************************************************************** */
#endif // TEXTEXPORTER_H
