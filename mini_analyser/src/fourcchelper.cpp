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
 * \date      2015
 */

#include "fourcchelper.h"
#include "ui_fourcchelper.h"
#include "thirdparty/portable_endian.h"

// minivideo library
#include <minivideo.h>

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

uint32_t endian_flip_32(uint32_t src)
{
    return ( ((src & 0x000000FF) << 24)
           | ((src & 0x0000FF00) <<  8)
           | ((src & 0x00FF0000) >>  8)
           | ((src & 0xFF000000) >> 24) );
}

FourccHelper::FourccHelper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FourccHelper)
{
    ui->setupUi(this);
    setMaximumSize(480, 320);

    // Set system endianness the default for the GUI
    endianness_system = BYTE_ORDER;
    endianness_gui = BYTE_ORDER;
    if (BYTE_ORDER == LITTLE_ENDIAN)
        ui->radioButton_le->toggle();
    else
        ui->radioButton_be->toggle();
    // But endianness feature is disabled...
    ui->frame_endianness->hide();
    connect(ui->radioButton_le, SIGNAL(toggled(bool)), this, SLOT(endiannessSwitch()));
    connect(ui->radioButton_be, SIGNAL(toggled(bool)), this, SLOT(endiannessSwitch()));

    // Codec finder is hidden when starting
    ui->label_fourcc_string->hide();

    connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->pushButton_endianness, SIGNAL(clicked(bool)), this, SLOT(endiannessInfo()));

    connect(ui->lineEdit_ascii, SIGNAL(textEdited(QString)), this, SLOT(asciiEdited()));
    connect(ui->lineEdit_hex, SIGNAL(textEdited(QString)), this, SLOT(hexEdited()));
    connect(ui->lineEdit_int32_LE, SIGNAL(textEdited(QString)), this, SLOT(int32LEEdited()));
    connect(ui->lineEdit_int32_BE, SIGNAL(textEdited(QString)), this, SLOT(int32BEEdited()));

    connect(ui->pushButton_switch_ascii, SIGNAL(clicked(bool)), this, SLOT(asciiSwitch()));
    connect(ui->pushButton_copy_ascii, SIGNAL(clicked(bool)), this, SLOT(asciiCopy()));
    connect(ui->pushButton_copy_hex, SIGNAL(clicked(bool)), this, SLOT(hexCopy()));
    connect(ui->pushButton_copy_bin, SIGNAL(clicked(bool)), this, SLOT(binCopy()));
    connect(ui->pushButton_copy_int32le, SIGNAL(clicked(bool)), this, SLOT(int32LECopy()));
    connect(ui->pushButton_copy_int32be, SIGNAL(clicked(bool)), this, SLOT(int32BECopy()));
}

FourccHelper::~FourccHelper()
{
    close();
    delete ui;
}

void FourccHelper::asciiSwitch()
{
    // We don't actually swap ASCII but hex, so we do not loose the unknown
    // ASCII characters during the swap

    // Swap internal_hex
    QByteArray hex_in = internal_hex;
    internal_hex.clear();
    for (int i = 0; i < hex_in.length(); i++)
    {
        internal_hex.append(hex_in.at(hex_in.length() - i-1));
    }

    fourccConvertion(0);
}

void FourccHelper::endiannessInfo()
{
    QDesktopServices::openUrl(QUrl("https://en.wikipedia.org/wiki/Endianness"));
}

void FourccHelper::endiannessSwitch()
{
    // Update internal setting
    if (endianness_gui == LITTLE_ENDIAN)
        endianness_gui = BIG_ENDIAN;
    else
        endianness_gui = LITTLE_ENDIAN;

    // Note:
    // I'm not entierly sure what to do with endianness here. Should we switch
    // everything or just the hex / i32 / binary representation of the ASCII string?
}

void FourccHelper::asciiEdited()
{
    internal_hex = ui->lineEdit_ascii->text().toLocal8Bit();
    fourccConvertion(1);
}

void FourccHelper::hexEdited()
{
    internal_hex = QByteArray::fromHex(ui->lineEdit_hex->text().toLocal8Bit());

    fourccConvertion(2);
}

void FourccHelper::int32LEEdited()
{
    QString string = ui->lineEdit_int32_LE->text();
    unsigned i32 = string.toUInt();

    internal_hex.clear();
    for (int i = 0; i < 4; i++)
    {
        char w = static_cast<char>((i32 >> (i*8)) & 0xFF);
        if (w > 0)
            internal_hex.append(QString::number(w, 16));
    }

    internal_hex = QByteArray::fromHex(internal_hex);
    fourccConvertion(3);
}

void FourccHelper::int32BEEdited()
{
    int pos = ui->lineEdit_int32_BE->cursorPosition();

    // TODO

    fourccConvertion(4);

    ui->lineEdit_int32_BE->setCursorPosition(pos);
}

void FourccHelper::binEdited()
{
    // TODO
}

////////////////////////////////////////////////////////////////////////////////

void FourccHelper::fourccConvertion(int from)
{
    QString ascii_str = "";
    QString hex_str = "";
    unsigned i32le = 0, i32be = 0;

    if (internal_hex.size())
    {
        // We have a new QByteArray internal_hex value!
        ascii_str = QString::fromLocal8Bit(internal_hex);
        hex_str = QString::fromLocal8Bit(internal_hex.toHex());

        for (int i = 0; i < internal_hex.size(); i++)
        {
            i32be += internal_hex.at(i) << (i*8);

            i32le <<= 8;
            i32le += internal_hex.at(i);
        }
/*
        qDebug() << "QByteArray hex (size:" << internal_hex.size() << ")  >> " << internal_hex;
        qDebug() << "]]] ASCII :" << ascii_str;
        qDebug() << "]]] HEX   :" << hex_str;
        qDebug() << "]]] BIN   :" << QString::number(i32le, 2).rightJustified(32, '0');
        qDebug() << "]]] i32le :" << QString::number(i32le, 10);
        qDebug() << "]]] i32be :" << QString::number(i32be, 10);
*/
    }

    // Update the QLineEdits
    if (from != 1)
        ui->lineEdit_ascii->setText(ascii_str);
    if (from != 2)
        ui->lineEdit_hex->setText(hex_str);

    if (endianness_gui == LITTLE_ENDIAN)
        ui->lineEdit_bin->setText(QString::number(i32le, 2).rightJustified(32, '0'));
    else
        ui->lineEdit_bin->setText(QString::number(i32be, 2).rightJustified(32, '0'));

    if (i32le || i32be)
    {
        if (from != 3)
            ui->lineEdit_int32_LE->setText(QString::number(i32le, 10));
        if (from != 4)
            ui->lineEdit_int32_BE->setText(QString::number(i32be, 10));
    }
    else
    {
        if (from != 3)
            ui->lineEdit_int32_LE->setText("");
        if (from != 4)
            ui->lineEdit_int32_BE->setText("");
    }

    // Update the codec box
    findCodec();
}

void FourccHelper::findCodec()
{
    QString codec = "Unknown";
    QString int32_field;

    // Read fcc
    if (endianness_gui == LITTLE_ENDIAN)
        int32_field = ui->lineEdit_int32_LE->text();
    else
        int32_field = ui->lineEdit_int32_BE->text();
    unsigned fcc = int32_field.toUInt();

    if (fcc)
    {
        ui->label_fourcc_string->show();
        ui->label_fourcc_string->setStyleSheet("QLabel { background: rgb(201, 255, 143);\nborder-color: rgb(85, 255, 0); }");

        // Try to find a match
        codec = QString::fromLocal8Bit(getCodecString(stream_UNKNOWN, getCodecFromFourCC(fcc), true));
        if (codec != "Unknown")
        {
            ui->label_fourcc_string->setText(">> " + codec);
            return;
        }
        else
        {
            // Switch endianness
            fcc = endian_flip_32(fcc);

            // Try again
            codec = QString::fromLocal8Bit(getCodecString(stream_UNKNOWN, getCodecFromFourCC(fcc), true));
            if (codec != "Unknown")
            {
                ui->label_fourcc_string->setText(">> " + codec + + "\n>> " + tr("but the endianness is all wrong!"));
                return;
            }
        }

        // We didn't find a match...
        ui->label_fourcc_string->setText(tr(">> This FourCC is unknown..."));
        ui->label_fourcc_string->setStyleSheet("QLabel { border: 1px solid rgb(255, 53, 3);\nbackground: rgba(255, 170, 0, 128); }");
    }
    else
    {
        ui->label_fourcc_string->hide();
    }
}

////////////////////////////////////////////////////////////////////////////////

void FourccHelper::asciiCopy()
{
    ui->lineEdit_ascii->selectAll();
    ui->lineEdit_ascii->copy();
    ui->lineEdit_ascii->deselect();
}

void FourccHelper::hexCopy()
{
    ui->lineEdit_hex->selectAll();
    ui->lineEdit_hex->copy();
    ui->lineEdit_hex->deselect();
}

void FourccHelper::binCopy()
{
    ui->lineEdit_bin->selectAll();
    ui->lineEdit_bin->copy();
    ui->lineEdit_bin->deselect();
}

void FourccHelper::int32LECopy()
{
    ui->lineEdit_int32_LE->selectAll();
    ui->lineEdit_int32_LE->copy();
    ui->lineEdit_int32_LE->deselect();
}

void FourccHelper::int32BECopy()
{
    ui->lineEdit_int32_BE->selectAll();
    ui->lineEdit_int32_BE->copy();
    ui->lineEdit_int32_BE->deselect();
}
