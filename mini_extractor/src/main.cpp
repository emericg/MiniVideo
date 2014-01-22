/*!
 * COPYRIGHT (C) 2012 Emeric Grange - All Rights Reserved
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
 * \date      2012
 */

// mini_extractor header
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
 * - output directory (where we want to save our PES stream file(s)).
 * - number and type of stream to extract from the input file.
 * - output format (ES or PES).
 */
int main(int argc, char *argv[])
{
    int retcode = EXIT_FAILURE;
    bool goodtogo = false;
    bool printhelp = false;

    char *input_filepath = NULL;
    char *output_directory = NULL;

    bool extract_audio = true;
    bool extract_video = true;
    bool extract_subtitles = true;

    int extract_audio_tracks = 0;
    int extract_video_tracks = 0;
    int extract_subtitles_tracks = 0;

    bool output_format_specified = false;
    int output_format = 0; // 0: PES / 1: ES

#if ENABLE_DEBUG
    std::cout << GREEN "main()" RESET << std::endl;
    std::cout << "* This is DEBUG from mini_extractor()" << std::endl;
    std::cout << "* mini_extractor version " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
    std::cout << std::endl;
    std::cout << GREEN "main() arguments" RESET << std::endl;
#endif /* ENABLE_DEBUG */

    // Argument(s) parsing
    if (argc == 1)
    {
        std::cerr << "mini_extractor: " RED "no argument" RESET  "!" << std::endl;
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
        else if (strcmp(argv[i], "-a") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 17)
                {
                    extract_audio_tracks = atoi(argv[i+1]);
                    std::cout << "* extract_audio_tracks : " << extract_audio_tracks << std::endl;
                }
                else
                {
                    std::cerr << "-a : Invalid number of audio track(s) to extract (between 1 and 16)" << std::endl;
                }

                i++;
            }
            else
            {
                // extract all audio tracks
                extract_audio_tracks = 16;
            }
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 17)
                {
                    extract_video_tracks = atoi(argv[i+1]);
                    std::cout << "* extract_video_tracks : " << extract_video_tracks << std::endl;
                }
                else
                {
                    std::cerr << "-v : Invalid number of video track(s) to extract (between 1 and 16)" << std::endl;
                }

                i++;
            }
            else
            {
                // extract all video tracks
                extract_video_tracks = 16;
            }
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            if (argv[i+1] != NULL)
            {
                if (atoi(argv[i+1]) > 0 && atoi(argv[i+1]) < 17)
                {
                    extract_subtitles_tracks = atoi(argv[i+1]);
                    std::cout << "* extract_subtitles_tracks : " << extract_subtitles_tracks << std::endl;
                }
                else
                {
                    std::cerr << "-v : Invalid number of subtitles track(s) to extract (between 1 and 16)" << std::endl;
                }

                i++;
            }
            else
            {
                // extract all video tracks
                extract_subtitles_tracks = 16;
            }
        }
        else if (strcmp(argv[i], "-pes") == 0)
        {
            if (output_format_specified)
            {
                std::cerr << "* " YELLOW "Be careful " RESET " you already specified an output format" << std::endl;
            }
            else
            {
                output_format = 0;
                output_format_specified = true;
            }
        }
        else if (strcmp(argv[i], "-es") == 0)
        {
            if (output_format_specified)
            {
                std::cerr << "* " YELLOW "Be careful " RESET " you already specified an output format" << std::endl;
            }
            else
            {
                output_format = 1;
                output_format_specified = true;
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

        retcode = minivideo_extractor(input_filepath, output_directory, extract_audio, extract_video, extract_subtitles, output_format);
    }
    else
    {
        std::cout << "* Usage: " << std::endl;
        std::cout << "mini_extractor -i <filepath> [-o <directory>] [-a audio_stream_count][-v video_stream_count][-s subtitles_stream_count] [-es] [-pes]" << std::endl;
        std::cout << GREEN " -h" RESET " : print help" << std::endl;
        std::cout << GREEN " -i" RESET " : filepath of the input video" << std::endl;
        std::cout << GREEN " -o" RESET " : directory where ES/PES output stream(s) will be saved" << std::endl;
        std::cout << GREEN " -a" RESET " : extract audio track(s)" << std::endl;
        std::cout << GREEN " -v" RESET " : extract video track(s)" << std::endl;
        std::cout << GREEN " -s" RESET " : extract subtitles file(s)" << std::endl;
        std::cout << GREEN " -es" RESET "  : Elementary Stream output format" << std::endl;
        std::cout << GREEN " -pes" RESET " : Packetized Elementary Stream output format" << std::endl;
    }

    // Check exit code from the video backend
    if (retcode == EXIT_SUCCESS)
    {
        std::cout << YELLOW "Exiting mini_extractor without errors. Have a nice day." RESET << std::endl;
    }
    else
    {
        std::cerr << YELLOW "Exiting mini_extractor " RED "with errors!" YELLOW " Have a nice day nonetheless." RESET << std::endl;
    }

    // Let's get to rest
    return retcode;
}

/* ************************************************************************** */
