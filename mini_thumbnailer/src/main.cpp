/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \date      2010
 */

// mini_thumbnailer
#include "main.h"

// minivideo library
#include <minivideo.h>

// C++ standard libraries
#include <iostream>
#include <cstdlib>
#include <cstring>

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
    bool printhelp = false;

    char *input_filepath = nullptr;
    char *output_directory = nullptr;

    int picture_format = PICTURE_JPG;
    int picture_quality = 75;
    int picture_number = 1;
    int picture_extraction_mode = 0;

    std::cout << GREEN "mini_thumbnailer main()" RESET << std::endl;
    std::cout << "* mini_thumbnailer version " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
    std::cout << std::endl;
    std::cout << GREEN "main() arguments" RESET << std::endl;

    // Argument(s) parsing
    ////////////////////////////////////////////////////////////////////////////

    if (argc <= 1 || !argv)
    {
        std::cerr << "mini_thumbnailer: " RED "no argument" RESET  "!" << std::endl;
    }

    for (int i = 1; i < argc; i++)
    {
        //std::cout << "argv[" << i << "] = " << argv[i] << std::endl;

        if (strcmp(argv[i], "-i") == 0)
        {
            if (argv[i+1] != nullptr)
            {
                //FIXME handle invalid intput file path
                input_filepath = argv[i+1];
                std::cout << "* input: " << input_filepath << std::endl;

                i++;
            }
            else
            {
                std::cerr << "-i : No input file specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (argv[i+1] != nullptr)
            {
                //FIXME handle invalid output directory path
                output_directory = argv[i+1];
                std::cout << "* output: " << output_directory << std::endl;

                i++;
            }
            else
            {
                std::cerr << "-o : No output folder specified" << std::endl;
            }
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if (argv[i+1] != nullptr)
            {
                if (strcmp(argv[i+1], "webp") == 0)
                {
                    picture_format = PICTURE_WEBP;
                    std::cout << "* picture_format: webp" << std::endl;
                }
                else if (strcmp(argv[i+1], "jpg") == 0 || strcmp(argv[i+1], "jpeg") == 0)
                {
                    picture_format = PICTURE_JPG;
                    std::cout << "* picture_format: jpg" << std::endl;
                }
                else if (strcmp(argv[i+1], "png") == 0)
                {
                    picture_format = PICTURE_PNG;
                    std::cout << "* picture_format: png" << std::endl;
                }
                else if (strcmp(argv[i+1], "bmp") == 0)
                {
                    picture_format = PICTURE_BMP;
                    std::cout << "* picture_format: bmp" << std::endl;
                }
                else if (strcmp(argv[i+1], "tga") == 0)
                {
                    picture_format = PICTURE_TGA;
                    std::cout << "* picture_format: tga" << std::endl;
                }
                else if (strcmp(argv[i+1], "yuv420") == 0)
                {
                    picture_format = PICTURE_YUV420;
                    std::cout << "* picture_format: yuv 4:2:0" << std::endl;
                }
                else if (strcmp(argv[i+1], "yuv444") == 0)
                {
                    picture_format = PICTURE_YUV444;
                    std::cout << "* picture_format: yuv 4:4:4" << std::endl;
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
            if (argv[i+1] != nullptr)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 100)
                {
                    picture_quality = atoi(argv[i+1]);
                    std::cout << "* picture_quality: " << picture_quality << std::endl;
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
            if (argv[i+1] != nullptr)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 1000)
                {
                    picture_number = atoi(argv[i+1]);
                    std::cout << "* picture_number: " << picture_number << std::endl;
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
            if (argv[i+1] != nullptr)
            {
                if (strcmp(argv[i+1], "unfiltered") == 0)
                {
                    picture_extraction_mode = PICTURE_UNFILTERED;
                    std::cout << "* extraction_mode : unfiltered" << std::endl;
                }
                else if (strcmp(argv[i+1], "ordered") == 0)
                {
                    picture_extraction_mode = PICTURE_ORDERED;
                    std::cout << "* extraction_mode : ordered" << std::endl;
                }
                else if (strcmp(argv[i+1], "distributed") == 0)
                {
                    picture_extraction_mode = PICTURE_DISTRIBUTED;
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
        }
        else
        {
            printhelp = true;
            std::cerr << "* " RED "Unknown argument " RESET "'" << argv[i] << "'" << std::endl;
        }
    }

    if (printhelp == true)
    {
        std::cout << "* Usage: " << std::endl;
        std::cout << "mini_thumbnailer -i <filepath> [-o <directory>] [-f picture_format][-q picture_quality][-n picture_number] [-e extraction_mode]" << std::endl;
        std::cout << GREEN " -h" RESET " : print help" << std::endl;
        std::cout << GREEN " -i" RESET " : filepath of the input video" << std::endl;
        std::cout << GREEN " -o" RESET " : directory where picture(s) will be saved" << std::endl;
        std::cout << GREEN " -f" RESET " : picture export format (can be 'webp' 'jpg' 'png' 'bmp' 'tga' 'yuv420' 'yuv444')" << std::endl;
        std::cout << GREEN " -q" RESET " : picture export quality (between 1 and 100)" << std::endl;
        std::cout << GREEN " -n" RESET " : picture export number (between 1 and 100)" << std::endl;
        std::cout << GREEN " -e" RESET " : picture extraction mode (can be 'unfiltered', 'ordered' or 'distributed')" << std::endl;
    }
    else
    {
        // Print informations about libMiniVideo and system endianness
        minivideo_print_infos();
        minivideo_print_features();
        minivideo_endianness();

        // Let's get to work
        std::cout << std::endl << YELLOW "Working..." RESET << std::endl;

        // Thumbnail(s) extraction
        ////////////////////////////////////////////////////////////////////////

        int minivideo_retcode = 0;

        // Open the video file
        MediaFile_t *input_video = nullptr;
        minivideo_retcode = minivideo_open(input_filepath, &input_video);

        if (minivideo_retcode == 1)
        {
            // Start container parsing
            minivideo_retcode = minivideo_parse(input_video, false, false);

            if (minivideo_retcode == 1)
            {
                // IDR frame filtering // Video decoding // Thumbnail export
                minivideo_retcode = minivideo_thumbnail(input_video,
                                                        output_directory,
                                                        picture_format,
                                                        picture_quality,
                                                        picture_number,
                                                        picture_extraction_mode);
            }

            // Close the video file
            minivideo_retcode = minivideo_close(&input_video);
        }

        // Convert library return code into program exit code
        if (minivideo_retcode == 1)
        {
            retcode = EXIT_SUCCESS;
            std::cout << std::endl << YELLOW "mini_thumbnailer exited without errors. Have a nice day." RESET << std::endl;
        }
        else
        {
            retcode = EXIT_FAILURE;
            std::cerr << std::endl << YELLOW "mini_thumbnailer exited " RED "with errors!" YELLOW " Have a nice day nonetheless..." RESET << std::endl;
        }
    }

    return retcode;
}

/* ************************************************************************** */
