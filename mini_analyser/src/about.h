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
 * \file      about.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

class AboutWindows : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindows(QWidget *parent = 0);
    ~AboutWindows();

    void setMinivideoVersion(int minivideo_major, int minivideo_minor, int minivideo_patch,
                             const char *minivideo_builddate, const char *minivideo_buildtime,
                             bool minivideo_builddebug);

private slots:
    void tabAbout();
    void tabAuthors();
    void tabLicense();
    void tabThirdParties();

protected:
    void resizeEvent(QResizeEvent *);

private:
    Ui::About *ui;

    bool licenseLoaded = false;
};

#endif // ABOUT_H
