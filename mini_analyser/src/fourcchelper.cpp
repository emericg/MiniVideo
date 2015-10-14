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
#include <iostream>

FourccHelper::FourccHelper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FourccHelper)
{
    ui->setupUi(this);

    connect(ui->pushButtonExit, SIGNAL(clicked(bool)), this, SLOT(close()));

    connect(ui->lineEdit_ascii, SIGNAL(textEdited(QString)), this, SLOT(asciiEdited()));
    //connect(ui->lineEdit_hex, SIGNAL(textEdited(QString)), this, SLOT(hexEdited()));
    //connect(ui->lineEdit_int32, SIGNAL(textEdited(QString)), this, SLOT(int32Edited()));
    //connect(ui->lineEdit_bin, SIGNAL(textEdited(QString)), this, SLOT(Edited()));
}

FourccHelper::~FourccHelper()
{
    close();
    delete ui;
}

void FourccHelper::hexEdited()
{
    QString hex = ui->lineEdit_hex->text();

    //QString ascii = hex.from

    QByteArray hexb = hex.toLocal8Bit();       //QByteArray::fromHex(hex);
    QString str = QString::fromUtf8(hexb);

    unsigned i32 = 0;
    bool bStatus;
    for (int i = 0; i < hex.size(); i++)
    {
        char a = hex.at(i).toLatin1();
        //char b = hex.at(i++).toLatin1();

        QString pair = QString::fromLocal8Bit(&a, 1);
        //pair += QString::fromLocal8Bit(&b, 1);

        std::cout << "moove: " << ((i)*4) << std::endl;
        std::cout << "moove += " << pair.toUInt(&bStatus, 16) << ((i)*4) << std::endl;
        i32 += pair.toUInt(&bStatus, 16) << ((i)*4);
    }

    ui->lineEdit_ascii->setText(str);
    //ui->lineEdit_hex->setText(hex.toUpper());
    //ui->lineEdit_dec->setText(QString::number(dec));
    ui->lineEdit_int32->setText(QString::number(i32));
    //ui->lineEdit_bin->setText(QString::number(i32, 2));
}

void FourccHelper::asciiEdited()
{
    QString ascii = ui->lineEdit_ascii->text();

    QString hex = QString::fromLatin1(ascii.toLatin1().toHex());
    QString dec;// = QString::number(ascii.toInt());

    for (int i = 0; i < ascii.size(); i++)
    {
        dec += QString::number(ascii.at(i).toLatin1());
    }

    int i32 = 0;
    for (int i = 0; i < ascii.size(); i++)
    {
        i32 += ascii.at(i).toLatin1() << (i*8);
    }

/*
    char fcc_str[4];
    {
        fcc_str[0] = (ii32 >>  0) & 0xFF;
        fcc_str[1] = (ii32 >>  8) & 0xFF;
        fcc_str[2] = (ii32 >> 16) & 0xFF;
        fcc_str[3] = (ii32 >> 24) & 0xFF;
    }
    printf(">>>> fcc_str: %s \n", fcc_str);
*/

    ui->lineEdit_hex->setText(hex);
    ui->lineEdit_dec->setText(dec);
    ui->lineEdit_int32->setText(QString::number(i32));
    ui->lineEdit_bin->setText(QString::number(i32, 2));
}

/*
// int to str
if (fcc_str)
{
    fcc_str[0] = (fcc >>  0) & 0xFF;
    fcc_str[1] = (fcc >>  8) & 0xFF;
    fcc_str[2] = (fcc >> 16) & 0xFF;
    fcc_str[3] = (fcc >> 24) & 0xFF;
    fcc_str[4] = '\0';
}
*/
