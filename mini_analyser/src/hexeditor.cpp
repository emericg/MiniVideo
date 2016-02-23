/*!
 * COPYRIGHT (C) 2016 Christophe Kassabji - All Rights Reserved
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
 * \file      hexeditor.cpp
 * \author    Christophe Kassabji <kassabji@gmail.com>
 * \date      2016
 */

#include "hexeditor.h"
#include "ui_hexeditor.h"

HexEditor::HexEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HexEditor)
{
    ui->setupUi(this);
    hexEdit = new QHexEdit(this);
    hexEdit->setData("zazertyuioqsdfghjkl");

}

HexEditor::~HexEditor()
{
    delete ui;
}
