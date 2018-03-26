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

#ifndef TABDEV_H
#define TABDEV_H

#include <QWidget>

namespace Ui {
class tabDev;
}

class tabDev : public QWidget
{
    Q_OBJECT

public:
    explicit tabDev(QWidget *parent = 0);
    ~tabDev();

public slots:
    void clean();

    bool addFile(const QString &path, const QString &name,
                 uint64_t process, uint64_t parse, uint64_t mem);
    bool removeFile(const QString &path);

private:
    Ui::tabDev *ui;
    int fileCount = 0;
};

#endif // TABDEV_H
