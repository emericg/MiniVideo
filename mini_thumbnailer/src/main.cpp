/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \date      2010
 */

// mini_thumbnailer header
#include "main.h"

// C++ standard libraries
#include <iostream>
#include <cstdlib>
#include <cstring>

// minivideo library
#include <minivideo.h>

/* ************************************************************************** */

/*!
 * \brief Main (and only) function of this test software.
 * \param argc The number of argument contained in *argv[].
 * \param *argv[] A table containing program arguments.
 * \return 0 if everything went fine, the number of error(s) otherwise.
 *
 * This test software uses the minivideo library.
 * Start by parsing arguments:
 * - input file (the video file we want to decode).
 * - output directory (where we want to save our thumbnails).
 * - number of picture we want to extract from input file.
 */
int main(int argc, char *argv[])
{
    int retcode = EXIT_FAILURE;
    bool goodtogo = false;
    bool printhelp = false;

    char *input_filepath = NULL;
    char *output_directory = NULL;

    int picture_format = PICTURE_JPG;
    int picture_quality = 75;
    int picture_number = 1;
    int picture_extractionmode = 0;

#if ENABLE_DEBUG
    std::cout << GREEN "main()" RESET << std::endl;
    std::cout << "* This is DEBUG from mini_thumbnailer()" << std::endl;
    std::cout << "* mini_thumbnailer version " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
    std::cout << std::endl;
    std::cout << GREEN "main() arguments" RESET << std::endl;
#endif /* ENABLE_DEBUG */

    // Argument(s) parsing
    if (argc == 1)
    {
        std::cerr << "mini_thumbnailer: " RED "no argument" RESET  "!" << std::endl;
    }

    for (int i = 1; i < argc; i++)
    {
        //std::cout << "argv[" << i << "] = " << argv[i] << std::endl;

        if (strcmp(argv[i], "-i") == 0)
        {
            if (argv[i+1] != NULL)
            {
                //FIXME handle invalid intput file path
                input_filepath = argv[i+1];
                std::cout << "* input : " << input_filepath << std::endl;

                goodtogo = true;
                i++;
            }
            else
            {
                std::cerr << "-i : No input file specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (argv[i+1] != NULL)
            {
                //FIXME handle invalid output directory path
                output_directory = argv[i+1];
                std::cout << "* output : " << output_directory << std::endl;

                i++;
            }
            else
            {
                std::cerr << "-o : No output folder specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (strcmp(argv[i+1], "jpg") == 0)
                {
                    picture_format = PICTURE_JPG; // 0
                    std::cout << "* picture_format : jpg" << std::endl;
                }
                else if (strcmp(argv[i+1], "png") == 0)
                {
                    picture_format = PICTURE_PNG; // 1
                    std::cout << "* picture_format : png" << std::endl;
                }
                else if (strcmp(argv[i+1], "bmp") == 0)
                {
                    picture_format = PICTURE_BMP; // 2
                    std::cout << "* picture_format : bmp" << std::endl;
                }
                else if (strcmp(argv[i+1], "tga") == 0)
                {
                    picture_format = PICTURE_TGA; // 3
                    std::cout << "* picture_format : tga" << std::endl;
                }
                else if (strcmp(argv[i+1], "yuv420") == 0)
                {
                    picture_format = PICTURE_YUV420; // 4
                    std::cout << "* picture_format : yuv 4:2:0" << std::endl;
                }
                else if (strcmp(argv[i+1], "yuv444") == 0)
                {
                    picture_format = PICTURE_YUV444; // 5
                    std::cout << "* picture_format : yuv 4:4:4" << std::endl;
                }
                else
                {
                    std::cerr << "-f : No valid picture format specified" << std::endl;
                }

                i++;
            }
            else
            {
                std::cerr << "-f : No picture format specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-q") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 100)
                {
                    picture_quality = atoi(argv[i+1]);
                    std::cout << "* picture_quality : " << picture_quality << std::endl;
                }
                else
                {
                    std::cerr << "-q : No valid picture quality specified (between 1 and 100)" << std::endl;
                }

                i++;
            }
            else
            {
                std::cerr << "-q : No picture quality specified (between 1 and 100)" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 1000)
                {
                    picture_number = atoi(argv[i+1]);
                    std::cout << "* picture_number : " << picture_number << std::endl;
                }
                else
                {
                    std::cerr << "-n : No valid picture number specified (between 1 and 999)" << std::endl;
                }

                i++;
            }
            else
            {
                std::cerr << "-n : No picture number specified (between 1 and 999)" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (strcmp(argv[i+1], "unfiltered") == 0)
                {
                    picture_extractionmode = PICTURE_UNFILTERED; // 0
                    std::cout << "* extraction_mode : unfiltered" << std::endl;
                }
                else if (strcmp(argv[i+1], "ordered") == 0)
                {
                    picture_extractionmode = PICTURE_ORDERED; // 1
                    std::cout << "* extraction_mode : ordered" << std::endl;
                }
                else if (strcmp(argv[i+1], "distributed") == 0)
                {
                    picture_extractionmode = PICTURE_DISTRIBUTED; // 2
                    std::cout << "* extraction_mode : distributed" << std::endl;
                }
                else
                {
                    std::cerr << "-e : No valid extraction mode specified" << std::endl;
                }

                i++;
            }
            else
            {
                std::cerr << "-e : No extraction mode specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printhelp = true;
            goodtogo = false;
        }
        else
        {
            std::cerr << "* " RED "Unknown argument " RESET "'" << argv[i] << "'" << std::endl;
        }
    }

    if (goodtogo)
    {
#if ENABLE_DEBUG
        // Print informations about LibMiniVideo and system endianness
        minivideo_infos();
        minivideo_endianness();

        // Let's get to work
        std::cout << std::endl << YELLOW "Working..." RESET << std::endl;
#endif /* ENABLE_DEBUG */

        retcode = minivideo_thumbnailer(input_filepath, output_directory, picture_format, picture_quality, picture_number, picture_extractionmode);
    }
    else
    {
        std::cout << "* Usage: " << std::endl;
        std::cout << "mini_thumbnailer -i <filepath> [-o <directory>] [-f picture_format][-q picture_quality][-n picture_number] [-e extraction_mode]" << std::endl;
        std::cout << GREEN " -h" RESET " : print help" << std::endl;
        std::cout << GREEN " -i" RESET " : filepath of the input video" << std::endl;
        std::cout << GREEN " -o" RESET " : directory where picture(s) will be saved" << std::endl;
        std::cout << GREEN " -f" RESET " : picture export format (can be 'jpg' 'png' 'bmp' 'tga' 'yuv420' 'yuv444')" << std::endl;
        std::cout << GREEN " -q" RESET " : picture export quality (between 1 and 100)" << std::endl;
        std::cout << GREEN " -n" RESET " : picture export number (between 1 and 100)" << std::endl;
        std::cout << GREEN " -e" RESET " : picture extraction mode (can be 'unfiltered', 'ordered' or 'distributed')" << std::endl;
    }

    // Check exit code from the video backend
    if (retcode == EXIT_SUCCESS)
    {
        std::cout << std::endl << YELLOW "mini_thumbnailer exited without errors. Have a nice day." RESET << std::endl;
    }
    else
    {
        std::cerr << std::endl << YELLOW "mini_thumbnailer exited " RED "with errors!" YELLOW " Have a nice day nonetheless..." RESET << std::endl;
    }

    // Let's get to rest
    return retcode;
}

/* ************************************************************************** */
