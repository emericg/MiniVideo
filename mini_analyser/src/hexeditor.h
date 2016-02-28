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
 * \file      hexeditor.h
 * \author    Christophe Kassabji <kassabji@gmail.com>
 * \date      2016
 */

#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include "thirdparty/qhexedit2/qhexedit.h"

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class HexEditor;
}

class HexEditor : public QWidget
{
    Q_OBJECT

public:
    explicit HexEditor(QWidget *parent = 0);
    ~HexEditor();
    void loadFile(const QString &fileName);

private:
    Ui::HexEditor *ui;
    QHexEdit *hexEdit;

    QString curFile;
    QFile file;

    void resizeEvent(QResizeEvent *event);
};

#endif // HEXEDITOR_H
