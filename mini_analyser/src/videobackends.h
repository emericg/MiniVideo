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
 * \file      videobackends.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef VIDEOBACKENDS_H
#define VIDEOBACKENDS_H
/* ************************************************************************** */

#include <vector>
#include <QString>

typedef struct CodecSupport
{
    unsigned codec;
    unsigned profile;

    bool hardware = false;

    int max_width = -1;
    int max_height = -1;
    int max_bitdepth = -1;
} CodecSupport;

class VideoBackendInfos
{
public:
    VideoBackendInfos();
    ~VideoBackendInfos();

    QString api_name;

    QString api_info;
    QString api_version;

    QString driver_info;
    QString driver_version;

    std::vector<CodecSupport> decodingSupport;
    std::vector<CodecSupport> encodingSupport;
};

/* ************************************************************************** */
#endif // VIDEOBACKENDS_H
