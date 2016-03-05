/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      about.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "about.h"
#include "ui_about.h"

AboutWindows::AboutWindows(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(close()));
}

AboutWindows::~AboutWindows()
{
    delete ui;
}

void AboutWindows::setMinivideoVersion(int minivideo_major, int minivideo_minor, int minivideo_patch,
                                       const char *minivideo_builddate, const char*minivideo_buildtime)
{
    QString text = tr("Using MiniVideo version") + " " + QString::number(minivideo_major) + "." + QString::number(minivideo_minor) + "-" + QString::number(minivideo_patch);
    text += "\n" + tr("Builded on") + " " + minivideo_builddate + " at " + minivideo_buildtime;

    ui->textBrowser_version->setText(text);
}

void AboutWindows::resizeEvent(QResizeEvent *event)
{
    //
}
