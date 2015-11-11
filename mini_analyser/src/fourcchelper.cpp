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
 * \file      fourcchelper.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

#include "fourcchelper.h"
#include "ui_fourcchelper.h"

#include <QDesktopServices>
#include <QUrl>

#include <iostream>

FourccHelper::FourccHelper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FourccHelper)
{
    ui->setupUi(this);

    connect(ui->pushButton_exit, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->pushButton_endianness, SIGNAL(clicked(bool)), this, SLOT(endiannessInfo()));

    connect(ui->lineEdit_ascii, SIGNAL(textEdited(QString)), this, SLOT(asciiEdited()));
    connect(ui->lineEdit_hex, SIGNAL(textEdited(QString)), this, SLOT(hexEdited()));
    //connect(ui->lineEdit_dec, SIGNAL(textEdited(QString)), this, SLOT(decEdited()));
    connect(ui->lineEdit_int32, SIGNAL(textEdited(QString)), this, SLOT(int32Edited()));
    //connect(ui->lineEdit_bin, SIGNAL(textEdited(QString)), this, SLOT(binEdited()));

    connect(ui->pushButton_copy_ascii, SIGNAL(clicked(bool)), this, SLOT(asciiCopy()));
    connect(ui->pushButton_copy_hex, SIGNAL(clicked(bool)), this, SLOT(hexCopy()));
    connect(ui->pushButton_copy_dec, SIGNAL(clicked(bool)), this, SLOT(decCopy()));
    connect(ui->pushButton_copy_int32, SIGNAL(clicked(bool)), this, SLOT(int32Copy()));
    connect(ui->pushButton_copy_bin, SIGNAL(clicked(bool)), this, SLOT(binCopy()));

    connect(ui->radioButton_le, SIGNAL(clicked(bool)), this, SLOT(endiannessSwitch()));
    connect(ui->radioButton_be, SIGNAL(clicked(bool)), this, SLOT(endiannessSwitch()));
}

FourccHelper::~FourccHelper()
{
    close();
    delete ui;
}

void FourccHelper::endiannessInfo()
{
    QDesktopServices::openUrl(QUrl("https://en.wikipedia.org/wiki/Endianness"));
}

void FourccHelper::endiannessSwitch()
{
    // TODO
}

void FourccHelper::asciiCopy()
{
    ui->lineEdit_ascii->selectAll();
    ui->lineEdit_ascii->copy();
    ui->lineEdit_ascii->deselect();
}

void FourccHelper::asciiEdited()
{
    QString ascii = ui->lineEdit_ascii->text();

    // ASCI to hex
    QString hex = QString::fromLatin1(ascii.toLatin1().toHex());

    QString dec;
    unsigned i32 = 0;
    for (int i = 0; i < ascii.size(); i++)
    {
        // ASCII to decimal
        dec += QString::number(ascii.at(i).toLatin1());

        // ASCII to packet int32
        i32 += ascii.at(i).toLatin1() << (i*8);
    }

    ui->lineEdit_hex->setText(hex);
    ui->lineEdit_dec->setText(dec);
    ui->lineEdit_int32->setText(QString::number(i32, 10));
    ui->lineEdit_bin->setText(QString::number(i32, 2));
}

void FourccHelper::hexCopy()
{
    ui->lineEdit_hex->selectAll();
    ui->lineEdit_hex->copy();
    ui->lineEdit_hex->deselect();
}

void FourccHelper::hexEdited()
{
    QString hexstr = ui->lineEdit_hex->text();
    QByteArray hex = ui->lineEdit_hex->text().toLocal8Bit();

    // Hex to ASCII (not always possible)
    QString ascii = QByteArray::fromHex(hex).data();

    // Hex to int32
    unsigned i32 = hexstr.toUInt(0, 16);

    // ASCII to decimal
    QString dec;
    for (int i = 0; i < ascii.size(); i++)
    {
        dec += QString::number(ascii.at(i).toLatin1());
    }

    ui->lineEdit_ascii->setText(ascii);
    ui->lineEdit_dec->setText(dec);
    ui->lineEdit_int32->setText(QString::number(i32, 10));
    ui->lineEdit_bin->setText(QString::number(i32, 2));
}

void FourccHelper::decCopy()
{
    ui->lineEdit_dec->selectAll();
    ui->lineEdit_dec->copy();
    ui->lineEdit_dec->deselect();
}

void FourccHelper::decEdited()
{
    // TODO
}

void FourccHelper::int32Copy()
{
    ui->lineEdit_int32->selectAll();
    ui->lineEdit_int32->copy();
    ui->lineEdit_int32->deselect();
}

void FourccHelper::int32Edited()
{
    QString int32 = ui->lineEdit_int32->text();
    unsigned i32 = int32.toUInt(0, 10);

    // int32 to ASCII
    char fcc_str[4];
    {
        fcc_str[0] = (i32 >>  0) & 0xFF;
        fcc_str[1] = (i32 >>  8) & 0xFF;
        fcc_str[2] = (i32 >> 16) & 0xFF;
        fcc_str[3] = (i32 >> 24) & 0xFF;
    }
    QString ascii = QString::fromLatin1(fcc_str);

    // ASCII to decimal
    QString dec;
    for (int i = 0; i < ascii.size(); i++)
    {
        dec += QString::number(ascii.at(i).toLatin1());
    }

    ui->lineEdit_ascii->setText(ascii);
    ui->lineEdit_hex->setText(QString::number(i32, 16));
    ui->lineEdit_dec->setText(dec);
    ui->lineEdit_bin->setText(QString::number(i32, 2));
}

void FourccHelper::binCopy()
{
    ui->lineEdit_bin->selectAll();
    ui->lineEdit_bin->copy();
    ui->lineEdit_bin->deselect();
}

void FourccHelper::binEdited()
{
    // TODO
}
