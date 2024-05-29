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
 * \date      2016
 */

#include "about.h"
#include "ui_about.h"

#include "minivideo.h"

#include <QFile>
#include <QTextStream>

AboutWindows::AboutWindows(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    QString title = "<p align=\"left\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:28pt; font-weight:600;\">mini_analyser</span><span style=\" font-size:16pt; font-weight:600;\"> (" + QString::fromUtf8(VERSION_STR) + ")</span></p></body></html>";
    ui->textBrowser_title->setHtml(title);

    tabAbout();

    connect(ui->pushButton_about, SIGNAL(clicked(bool)), this, SLOT(tabAbout()));
    connect(ui->pushButton_license, SIGNAL(clicked(bool)), this, SLOT(tabLicense()));
    connect(ui->pushButton_authors, SIGNAL(clicked(bool)), this, SLOT(tabAuthors()));
    connect(ui->pushButton_thirdparties, SIGNAL(clicked(bool)), this, SLOT(tabThirdParties()));
}

AboutWindows::~AboutWindows()
{
    delete ui;
}

void AboutWindows::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
}

void AboutWindows::tabAbout()
{
#ifdef QT_DEBUG
    bool minianalyser_debug = true;
#else
    bool minianalyser_debug = false;
#endif

    int minivideo_major, minivideo_minor, minivideo_patch;
    const char *minivideo_builddate, *minivideo_buildtime;
    bool minivideo_builddebug;
    minivideo_get_infos(&minivideo_major, &minivideo_minor, &minivideo_patch,
                        &minivideo_builddate, &minivideo_buildtime, &minivideo_builddebug);

    QFile file(":/about/about/about.html");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QString about = file.readAll();

        about.replace("$$VERSION_STR", QString::fromUtf8(VERSION_STR));
        if (minianalyser_debug) about.replace("$$DEBUG_STR", " / <b>DEBUG</b>");
        else about.replace("$$DEBUG_STR", "");
        about.replace("$$DATE_STR", QString::fromUtf8(__DATE__));
        about.replace("$$TIME_STR", QString::fromUtf8(__TIME__));

        about.replace("$$minivideo_major", QString::number(minivideo_major));
        about.replace("$$minivideo_minor", QString::number(minivideo_minor));
        about.replace("$$minivideo_patch", QString::number(minivideo_patch));
        if (minivideo_builddebug) about.replace("$$minivideo_debug", " / <b>DEBUG</b>");
        else about.replace("$$minivideo_debug", "");
        about.replace("$$minivideo_builddate", QString::fromUtf8(minivideo_builddate));
        about.replace("$$minivideo_buildtime", QString::fromUtf8(minivideo_buildtime));

        ui->textBrowser_content->setHtml(about);
    }
}

void AboutWindows::tabAuthors()
{
    QFile file(":/about/about/authors.html");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        ui->textBrowser_content->setHtml(file.readAll());
    }
}

void AboutWindows::tabLicense()
{
    QFile file(":/about/about/gpl-3.0-standalone.html");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        ui->textBrowser_content->setHtml(file.readAll());
    }
}

void AboutWindows::tabThirdParties()
{
    QFile file(":/about/about/thirdparties.html");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        ui->textBrowser_content->setHtml(file.readAll());
    }
}
