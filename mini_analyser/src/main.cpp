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
 * \file      main.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

// mini_analyser
#include "main.h"
#include "mainwindow.h"

// minivideo library
#include <minivideo.h>

// C++ standard libraries
#include <iostream>
#include <cstdlib>
#include <cstring>

// Qt
#include <QApplication>

/* ************************************************************************** */

/*!
 * \brief Main (and only) function of this test software.
 * \param argc The number of argument contained in *argv[].
 * \param *argv[] A table containing program arguments.
 * \return 0 if everything went fine, the number of error(s) otherwise.
 *
 * This test software uses the minivideo library.
 */
int main(int argc, char *argv[])
{
    std::cout << GREEN "main()" RESET << std::endl;
    std::cout << "* This is DEBUG from mini_analyser()" << std::endl;
    std::cout << "* mini_analyser version " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
    std::cout << std::endl;
    std::cout << GREEN "main() arguments" RESET << std::endl;

    // Print informations about libMiniVideo and system endianness
    minivideo_infos();
    minivideo_endianness();

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}

/* ************************************************************************** */
