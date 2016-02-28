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

/*!
 * \brief The FourccHelper class
 */
class FourccHelper : public QDialog
{
    Q_OBJECT

public:
    explicit FourccHelper(QWidget *parent = 0);
    ~FourccHelper();

private slots:
    void asciiSwitch();

    void endiannessInfo();
    void endiannessSwitch();

    void asciiCopy();
    void hexCopy();
    void binCopy();
    void int32LECopy();
    void int32BECopy();

    void asciiEdited();
    void hexEdited();
    void binEdited();
    void int32LEEdited();
    void int32BEEdited();

    void fourccConvertion(int from);
    void findCodec();

private:
    Ui::FourccHelper *ui;

    int endianness_system;
    int endianness_gui;

    QByteArray internal_hex;
};

#endif // FOURCCHELPER_H
