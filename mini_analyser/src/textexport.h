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
 * \file      textexport.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef TEXT_EXPORT_H
#define TEXT_EXPORT_H

// minivideo library
#include "minivideo_mediafile.h"

#include <QString>

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

    static int generateExportDatas_text(MediaFile_t &media, QString &exportDatas, bool detailed);
    static int generateExportDatas_json(MediaFile_t &media, QString &exportDatas, bool detailed);
    static int generateExportDatas_xml(MediaFile_t &media, QString &exportDatas, bool detailed);
    static int generateExportMapping_xml(MediaFile_t &media, QString &exportDatas);
};

#endif // TEXT_EXPORT_H
