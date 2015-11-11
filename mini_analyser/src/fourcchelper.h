/*!
 * COPYRIGHT (C) 2015 Emeric Grange - All Rights Reserved
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
 * \file      fourcchelper.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#ifndef FOURCCHELPER_H
#define FOURCCHELPER_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class FourccHelper;
}

class FourccHelper : public QDialog
{
    Q_OBJECT

public:
    explicit FourccHelper(QWidget *parent = 0);
    ~FourccHelper();

private slots:
    void endiannessInfo();
    void endiannessSwitch();

    void asciiCopy();
    void hexCopy();
    void decCopy();
    void int32Copy();
    void binCopy();

    void asciiEdited();
    void hexEdited();
    void decEdited();
    void int32Edited();
    void binEdited();

private:
    Ui::FourccHelper *ui;
};

#endif // FOURCCHELPER_H
