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
 * \file      about.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "about.h"
#include "ui_about.h"

#include <QFile>
#include <QTextStream>

AboutWindows::AboutWindows(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    tabAbout();

    connect(ui->pushButton_about, SIGNAL(clicked(bool)), this, SLOT(tabAbout()));
    connect(ui->pushButton_license, SIGNAL(clicked(bool)), this, SLOT(tabLicense()));
    connect(ui->pushButton_authors, SIGNAL(clicked(bool)), this, SLOT(tabAuthors()));
    connect(ui->pushButton_thirdparties, SIGNAL(clicked(bool)), this, SLOT(tabThirdParties()));
    //connect(ui->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(close()));
}

AboutWindows::~AboutWindows()
{
    delete ui;
}

void AboutWindows::setMinivideoVersion(int minivideo_major, int minivideo_minor, int minivideo_patch,
                                       const char *minivideo_builddate, const char*minivideo_buildtime,
                                       bool minivideo_builddebug)
{
    QString title = "<p align=\"left\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Cantarell'; font-size:28pt; font-weight:600;\">mini_analyser</span><span style=\" font-family:'Cantarell'; font-size:16pt; font-weight:600;\"> (" + QString::fromLocal8Bit(VERSION_STR) + ")</span></p></body></html>";

    QString text = "<html>";
    text += tr("MiniAnalyser") + " " + QString::fromLocal8Bit(VERSION_STR);
#ifdef QT_DEBUG
    text += " / <b>DEBUG</b>";
#endif
    text += " / " + tr("builded on:") + " " + QString::fromLocal8Bit(__DATE__) + " at " + QString::fromLocal8Bit(__TIME__);

    text += "<br>" + tr("MiniVideo library") + " " + QString::number(minivideo_major) + "." + QString::number(minivideo_minor) + "-" + QString::number(minivideo_patch);
    if (minivideo_builddebug) text += " / <b>DEBUG</b>";
    text += " / " + tr("builded on:") + " " + minivideo_builddate + " at " + minivideo_buildtime;
    text += "</html>";

    ui->textBrowser_title->setText(title);
    ui->textBrowser_version->setText(text);
}

void AboutWindows::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // we may want to resize the app icon?
}

void AboutWindows::tabAbout()
{
    ui->textBrowser_desc->show();
    ui->textBrowser_version->show();
    ui->textBrowser_license->hide();
    ui->textBrowser_copyright->hide();
    ui->textBrowser_thirdparties->hide();
}

void AboutWindows::tabAuthors()
{
    ui->textBrowser_desc->hide();
    ui->textBrowser_version->hide();
    ui->textBrowser_license->hide();
    ui->textBrowser_copyright->show();
    ui->textBrowser_thirdparties->hide();
}

void AboutWindows::tabLicense()
{
    ui->textBrowser_desc->hide();
    ui->textBrowser_version->hide();
    ui->textBrowser_license->show();
    ui->textBrowser_copyright->hide();
    ui->textBrowser_thirdparties->hide();

    if (licenseLoaded == false)
    {
        QFile file(":/licenses/LICENSE");
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream ReadFile(&file);
            ui->textBrowser_license->setText(ReadFile.readAll());

            licenseLoaded = true;
        }
    }
}

void AboutWindows::tabThirdParties()
{
    ui->textBrowser_desc->hide();
    ui->textBrowser_version->hide();
    ui->textBrowser_license->hide();
    ui->textBrowser_copyright->hide();
    ui->textBrowser_thirdparties->show();
}
