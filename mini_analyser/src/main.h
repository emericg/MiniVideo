/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
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
 * \file      main.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

#ifndef MINI_ANALYSER_H
#define MINI_ANALYSER_H

#include <QCoreApplication>
#include <QApplication>
#include <QMainWindow>

#include "mainwindow.h"
#include "cli.h"

/* ************************************************************************** */

int main(int argc, char *argv[]);

class MiniAnalyserCLI : public QCoreApplication
{
public:
    MiniAnalyserCLI(int &argc, char **argv);
    ~MiniAnalyserCLI();

    CLI cli;
};

class MiniAnalyserGUI : public QApplication
{
    bool event(QEvent *e);

public:
    MiniAnalyserGUI(int &argc, char **argv);
    ~MiniAnalyserGUI();

    MainWindow gui;
};

/* ************************************************************************** */

// Colors for console output
#define RESET  "\e[0;m" //!< Reset colored output to default terminal color
#define RED    "\e[1;31m"
#define GREEN  "\e[1;32m"
#define YELLOW "\e[1;33m"
#define BLUE   "\e[1;34m"
#define PURPLE "\e[1;35m"
#define CYAN   "\e[1;36m"
#define WHITE  "\e[1;37m"

/* ************************************************************************** */
#endif // MINI_ANALYSER_H
