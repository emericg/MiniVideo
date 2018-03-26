/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
 *
 * MiniVideo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniVideo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiniVideo.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      export_utils.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "export_utils.h"
#include "utils.h"
#include "minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>

// C POSIX library
#ifdef _MSC_VER
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <unistd.h>
#endif

/* ************************************************************************** */

/*!
 * \brief Get various from a video filepath.
 * \param *path A directory path.
 * \param *path_absolute An absolute directory path.
 *
 * Get absolute file path from a directory.
 * This function will only work with Unix-style file systems.
 */
void make_path_absolute(const char *path, char *path_absolute)
{
    TRACE_2(IO, "make_path_absolute()");

    // Check if 'path' is absolute
    const char *pos_first_slash_p = strchr(path, '/');

    if (pos_first_slash_p != NULL &&
        (pos_first_slash_p - path) == 0)
    {
        TRACE_2(IO, "* path seems to be absolute already (first caracter is /)");
    }
    else
    {
        char cwd[4096];
        char absolute_path_temp[4096];
        FILE *temp = NULL;

        // First attempt
        getcwd(cwd, sizeof(cwd));
        strncpy(absolute_path_temp, strncat(cwd, path, sizeof(cwd) - 1), sizeof(absolute_path_temp) - 1);
        TRACE_1(IO, "* t1            : '%s'", absolute_path_temp);

        temp = fopen(absolute_path_temp, "rb");
        if (temp != NULL)
        {
            TRACE_2(IO, "* New absolute path found, new using method 1: '%s'", absolute_path_temp);
            strncpy(path_absolute, absolute_path_temp, sizeof(*path_absolute) - 1);
            fclose(temp);
        }
        else
        {
            // Second attempt
            getcwd(cwd, sizeof(cwd));
            strncpy(absolute_path_temp, strncat(cwd, "/", 1), sizeof(absolute_path_temp) - 1);
            strncat(absolute_path_temp, path, sizeof(absolute_path_temp) - 1);
            TRACE_1(IO, "* t2            : '%s'", absolute_path_temp);

            temp = fopen(absolute_path_temp, "rb");
            if (temp != NULL)
            {
                TRACE_2(IO, "* New absolute path found, new using method 2");
                strncpy(path_absolute, absolute_path_temp, sizeof(*path_absolute) - 1);
                fclose(temp);
            }
            else
            {
                TRACE_2(IO, "* path seems to be absolute already");
                strncpy(path_absolute, path, sizeof(*path_absolute) - 1);
            }
        }
    }

    // Print results
    TRACE_1(IO, "* Directory path            : '%s'", path);
    TRACE_1(IO, "* Directory path (absolute) : '%s'", path_absolute);
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *buffer_ycbcr The RGB buffer to fill.
 * \return The number of missing macroblock(s).
 *
 * Convert the YCbCr 4:2:0 macroblock array to an I420 surface.
 */
int mb_to_ycbcr(DecodingContext_t *dc, unsigned char *buffer_ycbcr)
{
    TRACE_INFO(IO, " > " BLD_GREEN "mb_to_ycbcr()" CLR_RESET);
    int missing_mbs = 0;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];

    // Loops init
    int i = 0;

    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

    const unsigned int mb_lines_total = sps->PicHeightInMapUnits;
    const unsigned int mb_columns_total = sps->PicWidthInMbs;

    const unsigned int offset_cb = img_width * img_height;
    const unsigned int offset_cr = (img_width * img_height) + (img_width * img_height)/4;

    unsigned int mb_lines = 0, mb_columns = 0, mb_x = 0, mb_y = 0;

    if (buffer_ycbcr == NULL)
    {
        TRACE_ERROR(IO, "YCbCr buffer is null!");
        missing_mbs = mb_lines_total * mb_columns_total;
    }
    else
    {
        // Y extraction
        for (mb_lines = 0; mb_lines < mb_lines_total; mb_lines++)
        {
            for (mb_y = 0; mb_y < 16; mb_y++)
            {
                for (mb_columns = 0; mb_columns < mb_columns_total; mb_columns++)
                {
                    if (dc->mb_array[mb_columns + mb_columns_total*mb_lines] != NULL)
                    {
                        for (mb_x = 0; mb_x < 16; mb_x++)
                        {
                            buffer_ycbcr[i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeL[mb_x][mb_y];
                            i++;
                        }
                    }
                    else
                    {
                        i += 256;
                        missing_mbs++;
                    }
                }
            }
        }

        // Cb and Cr extraction + super sampling
        i = 0;
        for (mb_lines = 0; mb_lines < mb_lines_total; mb_lines++)
        {
            for (mb_y = 0; mb_y < 8; mb_y++)
            {
                for (mb_columns = 0; mb_columns < mb_columns_total; mb_columns++)
                {
                    if (dc->mb_array[mb_columns + mb_columns_total*mb_lines] != NULL)
                    {
                        for (mb_x = 0; mb_x < 8; mb_x++)
                        {
                            buffer_ycbcr[offset_cb + i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCb[mb_x][mb_y];
                            buffer_ycbcr[offset_cr + i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCr[mb_x][mb_y];

                            i++;
                        }
                    }
                    else
                    {
                        i += 64;
                    }
                }
            }
        }
    }

    return missing_mbs;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *buffer_rgb The RGB buffer to fill.
 * \return The number of missing macroblock(s).
 *
 * Convert the YCbCr 4:2:0 macroblock array to YCbCr 4:4:4 planar, then to packed RGB24 buffer.
 */
int mb_to_rgb(DecodingContext_t *dc, unsigned char *buffer_rgb)
{
    TRACE_INFO(IO, " > " BLD_GREEN "mb_to_rgb()" CLR_RESET);
    int missing_mbs = 0;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];

    // Loops init
    unsigned int i = 0;

    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

    const unsigned int mb_lines_total = sps->PicHeightInMapUnits;
    const unsigned int mb_columns_total = sps->PicWidthInMbs;

    unsigned int mb_lines = 0, mb_columns = 0, mb_x = 0, mb_y = 0;

    // YCbCr 4:4:4 buffers
    unsigned char *buffer_y = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));
    unsigned char *buffer_cb = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));
    unsigned char *buffer_cr = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));

    if (buffer_y == NULL || buffer_cb == NULL || buffer_cr == NULL)
    {
        TRACE_ERROR(IO, "Y Cb Cr buffers allocation failure!");
        missing_mbs = mb_lines_total * mb_columns_total;
    }
    else
    {
        // Y extraction
        for (mb_lines = 0; mb_lines < mb_lines_total; mb_lines++)
        {
            for (mb_y = 0; mb_y < 16; mb_y++)
            {
                for (mb_columns = 0; mb_columns < mb_columns_total; mb_columns++)
                {
                    if (dc->mb_array[mb_columns + mb_columns_total*mb_lines] != NULL)
                    {
                        for (mb_x = 0; mb_x < 16; mb_x++)
                        {
                            buffer_y[i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeL[mb_x][mb_y];
                            i++;
                        }
                    }
                    else
                    {
                        i += 256;
                        missing_mbs++;
                    }
                }
            }
        }

        // Cb and Cr extraction + up sampling
        for (mb_lines = 0, i = 0; mb_lines < mb_lines_total; mb_lines++)
        {
            for (mb_y = 0; mb_y < 8; mb_y++)
            {
                for (mb_columns = 0; mb_columns < mb_columns_total; mb_columns++)
                {
                    if (dc->mb_array[mb_columns + mb_columns_total*mb_lines] != NULL)
                    {
                        for (mb_x = 0; mb_x < 8; mb_x++)
                        {
                            buffer_cb[i] = buffer_cb[i+1] = buffer_cb[i + img_width] = buffer_cb[i + img_width + 1] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCb[mb_x][mb_y];
                            buffer_cr[i] = buffer_cr[i+1] = buffer_cr[i + img_width] = buffer_cr[i + img_width + 1] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCr[mb_x][mb_y];

                            i += 2;

                            if ((i % img_width) == 0)
                            {
                                i += img_width; // jump one line
                            }
                        }
                    }
                    else
                    {
                        i += 256;
                    }
                }
            }
        }

        // Color space convertion
        for (i = 0; i < (img_width * img_height * 3); i += 3)
        {
            buffer_rgb[i] = Clip1_YCbCr_8(((298 * buffer_y[i/3]) >> 8) + ((408 * buffer_cr[i/3]) >> 8) - 222); // R
            buffer_rgb[i + 1] = Clip1_YCbCr_8(((298 * buffer_y[i/3]) >> 8) - ((100 * buffer_cb[i/3]) >> 8) - ((208 * buffer_cr[i/3]) >> 8) + 135); // G
            buffer_rgb[i + 2] = Clip1_YCbCr_8(((298 * buffer_y[i/3]) >> 8) + ((516 * buffer_cb[i/3]) >> 8) - 276); // B
        }
    }

    // Cleanup YCbCr buffers
    if (buffer_y)
    {
        free(buffer_y);
        buffer_y = NULL;
    }
    if (buffer_cb)
    {
        free(buffer_cb);
        buffer_cb = NULL;
    }
    if (buffer_cr)
    {
        free(buffer_cr);
        buffer_cr = NULL;
    }

    return missing_mbs;
}

/* ************************************************************************** */
