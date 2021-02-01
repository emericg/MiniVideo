/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
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
 * \date      2017
 */

#ifndef MINIVIDEO_TEXTEXPORT_H
#define MINIVIDEO_TEXTEXPORT_H
/* ************************************************************************** */

#include "minivideo_mediafile.h"

#include <QString>

/* ************************************************************************** */

typedef enum TextExportFormat_e
{
    EXPORT_TEXT  = 0,
    EXPORT_XML   = 1,
    EXPORT_JSON  = 2,

} TextExportFormat_e;

class textExport
{
public:
    textExport();
    ~textExport();

    static int generateSubtitlesData_text(MediaFile_t &media, QString &exportData, unsigned track);

    static int generateExportData_text(MediaFile_t &media, QString &exportData, bool detailed);
    static int generateExportData_json(MediaFile_t &media, QString &exportData, bool detailed);
    static int generateExportData_xml(MediaFile_t &media, QString &exportData, bool detailed);
    static int generateExportMapping_xml(MediaFile_t &media, QString &exportData);
};

/* ************************************************************************** */
#endif // MINIVIDEO_TEXTEXPORT_H
