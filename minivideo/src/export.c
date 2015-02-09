/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      export.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>

// C POSIX library
#include <unistd.h>

#include "export.h"

// minivideo headers
#include "minitraces.h"
#include "utils.h"
#include "typedef.h"
#include "import.h"
#include "export_utils.h"
#include "decoder/h264/h264_decodingcontext.h"

#if ENABLE_JPEG
    // libjpeg
    #include <jpeglib.h>
#endif

#if ENABLE_PNG
    // libpng
    #define PNG_DEBUG 0
    #include <png.h>
#endif

#if ENABLE_STBIMWRITE
    // stbiw
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "stb_image_write.h"
#endif

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile The file pointer to write to.
 * \return 1 if success, 0 otherwise.
 */
static int export_idr_yuv420(DecodingContext_t *dc, FILE *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_yuv420()\n" CLR_RESET);
    int retcode = FAILURE;
    unsigned int missing_mbs = 0;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];

    // Loops init
    unsigned int i = 0;

    const unsigned int mb_lines_total = sps->PicHeightInMapUnits;
    const unsigned int mb_columns_total = sps->PicWidthInMbs;

    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

    unsigned int mb_lines = 0, mb_columns = 0, mb_x = 0, mb_y = 0;

    // YCbCr 4:2:0 buffers
    unsigned char *buffer_y = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));
    unsigned char *buffer_cb = (unsigned char *)calloc((img_width*img_height)/4, sizeof(unsigned char));
    unsigned char *buffer_cr = (unsigned char *)calloc((img_width*img_height)/4, sizeof(unsigned char));

    if (buffer_y == NULL ||
        buffer_cb == NULL ||
        buffer_cr == NULL)
    {
        TRACE_ERROR(IO, "Y Cb Cr buffers allocation failure!\n");
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

        // Cb and Cr extraction
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
                            buffer_cb[i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCb[mb_x][mb_y];
                            buffer_cr[i] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCr[mb_x][mb_y];
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

        // Write buffers to file
        fwrite(buffer_y, sizeof(unsigned char), img_width*img_height, PictureFile);
        fwrite(buffer_cb, sizeof(unsigned char), (img_width*img_height)/4, PictureFile);
        fwrite(buffer_cr, sizeof(unsigned char), (img_width*img_height)/4, PictureFile);

        // If error(s) occurred
        if (missing_mbs == 0)
        {
            retcode = SUCCESS;
        }
        else if (missing_mbs == dc->PicSizeInMbs)
        {
            TRACE_ERROR(IO, "* Picture written on disk ommiting all macroblocks!\n");
            retcode = FAILURE;
        }
        else
        {
            TRACE_WARNING(IO, "* Picture written on disk ommiting approximately %i/%i macroblock(s)!\n", missing_mbs, dc->PicSizeInMbs);
            retcode = SUCCESS;
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

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile The file pointer to write to.
 * \return 1 if success, 0 otherwise.
 */
static int export_idr_yuv444(DecodingContext_t *dc, FILE *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_yuv444()\n" CLR_RESET);
    int retcode = FAILURE;
    unsigned int missing_mbs = 0;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];

    // Loops init
    int i = 0;

    const unsigned int mb_lines_total = sps->PicHeightInMapUnits;
    const unsigned int mb_columns_total = sps->PicWidthInMbs;

    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

    unsigned int mb_lines = 0, mb_columns = 0, mb_x = 0, mb_y = 0;

    // YCbCr 4:4:4 buffers
    unsigned char *buffer_y = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));
    unsigned char *buffer_cb = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));
    unsigned char *buffer_cr = (unsigned char *)calloc(img_width*img_height, sizeof(unsigned char));

    if (buffer_y == NULL ||
        buffer_cb == NULL ||
        buffer_cr == NULL)
    {
        TRACE_ERROR(IO, "Y Cb Cr buffers allocation failure!\n");
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
                            buffer_cb[i] = buffer_cb[i+1] = buffer_cb[i + img_width] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCb[mb_x][mb_y];
                            buffer_cr[i] = buffer_cr[i+1] = buffer_cr[i + img_width] = dc->mb_array[mb_columns + mb_columns_total*mb_lines]->SprimeCr[mb_x][mb_y];

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

        // Write buffers to file
        fwrite(buffer_y, sizeof(unsigned char), img_width*img_height, PictureFile);
        fwrite(buffer_cb, sizeof(unsigned char), img_width*img_height, PictureFile);
        fwrite(buffer_cr, sizeof(unsigned char), img_width*img_height, PictureFile);

        // If error(s) occurred
        if (missing_mbs == 0)
        {
            retcode = SUCCESS;
        }
        else if (missing_mbs == dc->PicSizeInMbs)
        {
            TRACE_ERROR(IO, "* Picture written on disk ommiting all macroblocks!\n");
            retcode = FAILURE;
        }
        else
        {
            TRACE_WARNING(IO, "* Picture written on disk ommiting approximately %i/%i macroblock(s)!\n", missing_mbs, dc->PicSizeInMbs);
            retcode = SUCCESS;
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

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile The file pointer to write to.
 * \return 1 if success, 0 otherwise.
 *
 * From http://www.ijg.org/files/README:
 * jpegaltui.vN.tar.gz contains source code for an alternate user interface for
 * cjpeg/djpeg in Unix format.
 *
 * For RAW write : http://www.jpegcameras.com/libjpeg/libjpeg-3.html#ss3.11
 */
static int export_idr_jpg(DecodingContext_t *dc, FILE *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_jpg()\n" CLR_RESET);
    int retcode = FAILURE;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];
    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

#if ENABLE_JPEG

    // 0. we will be using this uninitialized pointer later to store raw, uncompressd image

    JSAMPROW y[16], cb[16], cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane)
    JSAMPARRAY data[3]; // t[0][2][5] = color sample 0 of row 2 and column 5

    data[0] = y;
    data[1] = cb;
    data[2] = cr;

    // convert macroblocks to ycbcr buffer
    unsigned char *buffer_ycbcr = (unsigned char *)calloc((img_width*img_height/2)*3, sizeof(unsigned char));
    mb_to_ycbcr(dc, buffer_ycbcr);

    // 1. Allocate and initialize a JPEG compression object

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr); // Errors will be written to stderr
    jpeg_create_compress(&cinfo);

    // 2. Specify the destination for the compressed data (eg, a file)

    jpeg_stdio_dest(&cinfo, PictureFile);

    // 3. Set parameters for compression, including image size & colorspace

    bool force_baseline = TRUE;
    cinfo.image_width = img_width;
    cinfo.image_height = img_height;
    cinfo.input_components = 3; // # of color components per pixel
    jpeg_set_defaults(&cinfo);

    cinfo.dct_method = JDCT_FASTEST;
    cinfo.raw_data_in = true; // true if JCS_YCbCr, false if JCS_RGB
    cinfo.in_color_space = JCS_YCbCr; // colorspace of input image

    cinfo.comp_info[0].h_samp_factor = 2; // for Y
    cinfo.comp_info[0].v_samp_factor = 2;
    cinfo.comp_info[1].h_samp_factor = 1; // for Cb
    cinfo.comp_info[1].v_samp_factor = 1;
    cinfo.comp_info[2].h_samp_factor = 1; // for Cr
    cinfo.comp_info[2].v_samp_factor = 1;

    jpeg_set_colorspace(&cinfo, JCS_YCbCr);
    jpeg_set_quality(&cinfo, dc->picture_quality, force_baseline);

    // 4. jpeg_start_compress(...);

    jpeg_start_compress(&cinfo, TRUE);

    // 5. writing data (JCS_YCbCr colorspace)
    unsigned int i = 0, j = 0;
    unsigned char *offset_cb = buffer_ycbcr + img_width * img_height;
    unsigned char *offset_cr = buffer_ycbcr + img_width * img_height + img_width * img_height / 4 ;
    for (j = 0; j < img_height; j += 16)
    {
        for (i = 0; i < 16; i++)
        {
            y[i] = buffer_ycbcr + img_width * (i + j);

            if (i % 2 == 0)
            {
                cb[i / 2] = offset_cb + img_width / 2 * ((i + j) /2);
                cr[i / 2] = offset_cr + img_width / 2 * ((i + j) / 2);
            }
        }

        jpeg_write_raw_data(&cinfo, data, 16);
    }

    // 6. jpeg_finish_compress(...);

    jpeg_finish_compress(&cinfo);

    // 7. Release the JPEG compression object

    jpeg_destroy_compress(&cinfo);

    free(buffer_ycbcr);

    retcode = SUCCESS;
#endif /* ENABLE_JPEG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile A file structure containing useful informations.
 * \return 1 if success, 0 otherwise.
 */
static int export_idr_png(DecodingContext_t *dc, OutputFile_t *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_png()\n" CLR_RESET);
    int retcode = FAILURE;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];
    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

#if ENABLE_PNG

    // 4.1 Setup

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,NULL, NULL);

    if (!png_ptr)
    {
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return 0;
    }

    png_init_io(png_ptr, PictureFile->file_pointer);

    // 0. png buffer

    unsigned int y = 0;
/*
    png_bytep * row_pointers = NULL;
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img_height);
    for (y = 0; y < img_height; y++)
    {
        //row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
        row_pointers[y] = (png_byte*)calloc(58, png_get_rowbytes(png_ptr, info_ptr));
    }
*/
    png_bytep *row_pointers;
    row_pointers = png_malloc(png_ptr, sizeof(png_bytep) * img_height);
    for (y = 0; y < img_height; y++)
    {
        row_pointers[y] = png_malloc(png_ptr, img_width*8*3);
        memset(row_pointers[y], 255, img_width*8);
    }

    // 4.3 Setting the contents of info for output

    png_set_IHDR(png_ptr,
                 info_ptr,
                 img_width,
                 img_height,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    // 4.5 (The high-level write interface)

    png_set_rows(png_ptr, info_ptr, row_pointers); // or &row_pointers
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // 4.x (The low-level write interface)
/*
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
*/
    // 4.8 Finishing a sequential write

    png_destroy_write_struct(&png_ptr, &info_ptr);

    for (y = 0; y < img_height; y++)
        free(row_pointers[y]);

    free(row_pointers);

    retcode = SUCCESS;

#elif ENABLE_STBIMWRITE

    // convert macroblocks to rgb buffer
    unsigned char *buffer_rgb = (unsigned char *)calloc(img_width*img_height*3, sizeof(unsigned char));
    mb_to_rgb(dc, buffer_rgb);

    // export to png
    retcode = stbi_write_png(PictureFile->file_name, img_width, img_height, 3, buffer_rgb, img_width*3);

#endif /* ENABLE_PNG or ENABLE_STBIMWRITE */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile A file structure containing useful informations.
 * \return 1 if success, 0 otherwise.
 */
static int export_idr_bmp(DecodingContext_t *dc, OutputFile_t *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_bmp()\n" CLR_RESET);
    int retcode = FAILURE;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];
    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

#if ENABLE_STBIMWRITE

    // convert macroblocks to rgb buffer
    unsigned char *buffer_rgb = (unsigned char *)calloc(img_width*img_height*3, sizeof(unsigned char));
    mb_to_rgb(dc, buffer_rgb);

    // export to bmp
    retcode = stbi_write_bmp(PictureFile->file_name, img_width, img_height, 3, buffer_rgb);

#endif

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \param *PictureFile A file structure containing useful informations.
 * \return 1 if success, 0 otherwise.
 */
static int export_idr_tga(DecodingContext_t *dc, OutputFile_t *PictureFile)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr_tga()\n" CLR_RESET);
    int retcode = FAILURE;

    // Shortcut
    sps_t *sps = dc->sps_array[dc->active_sps];
    const unsigned int img_width = sps->PicWidthInMbs * 16;
    const unsigned int img_height = sps->PicHeightInMapUnits * 16;

#if ENABLE_STBIMWRITE

    // convert macroblocks to rgb buffer
    unsigned char *buffer_rgb = (unsigned char *)calloc(img_width*img_height*3, sizeof(unsigned char));
    mb_to_rgb(dc, buffer_rgb);

    // export to bmp
    retcode = stbi_write_tga(PictureFile->file_name, img_width, img_height, 3, buffer_rgb);

#endif

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 1 if success, 0 otherwise.
 *
 * Set the name of the file to export, choose file extension and check file format
 * availability, open a FILE *pointer, then call the appropriate export_idr_xxx() function.
 */
int export_idr(DecodingContext_t *dc)
{
    TRACE_INFO(IO, BLD_GREEN "export_idr()\n" CLR_RESET);
    int retcode = FAILURE;

    // Picture file
    OutputFile_t PictureFile; //!< Output file

    // Picture name
    strncpy(PictureFile.file_name, dc->VideoFile->file_name, 254);

    // Picture number (if more than one picture is gonna be exported)
    if (dc->picture_number > 1)
    {
        char framenum[8];
        strncat(PictureFile.file_name, "_", 254);

        // idr_pic_id is periodically reseted so it is not a good way for counting idr
        //sprintf(framenum, "%d", dc->active_slice->idr_pic_id);

        // picture_exported is only used by this function (incrementation is only done when picture is successfully exported)
        sprintf(framenum, "%d", dc->picture_exported);

        strncat(PictureFile.file_name, framenum, sizeof(framenum) - 1);
    }

    // Check export format availability
#if ENABLE_JPEG == 0 && ENABLE_PNG == 0 && ENABLE_STBIMWRITE == 0
    if (dc->output_format < PICTURE_YUV420)
    {
        TRACE_WARNING(DIO, "No export library available, forcing YCbCr 4:2:0\n");
        dc->output_format = PICTURE_YUV420;
    }
#else
    #if ENABLE_JPEG == 0
        if (dc->output_format == PICTURE_JPG)
        {
            TRACE_WARNING(DIO, "No jpg export library available, trying png\n");
            dc->output_format++; // to png
        }
    #endif /* ENABLE_JPEG */

    #if ENABLE_PNG == 0 && ENABLE_STBIMWRITE == 0
        if (dc->output_format == PICTURE_PNG)
        {
            TRACE_WARNING(DIO, "No png export library available, forcing YCbCr 4:2:0\n");
            dc->output_format = PICTURE_YUV420;
        }
    #endif /* ENABLE_PNG */

    #if ENABLE_STBIMWRITE == 0
        if (dc->output_format == PICTURE_BMP || dc->output_format == PICTURE_TGA)
        {
            TRACE_WARNING(DIO, "No bmp / tga export library available, forcing YCbCr 4:2:0\n");
            dc->output_format = PICTURE_YUV420;
        }
    #endif /* ENABLE_STBIMWRITE */
#endif

    // Picture file extension checker
    if (dc->output_format == PICTURE_JPG)
    {
        TRACE_1(IO, "* Picture file format : JPG\n");
        strncpy(PictureFile.file_extension, "jpg", 8);
    }
    else if (dc->output_format == PICTURE_PNG)
    {
        TRACE_1(IO, "* Picture file format : PNG\n");
        strncpy(PictureFile.file_extension, "png", 8);
    }
    else if (dc->output_format == PICTURE_BMP)
    {
        TRACE_1(IO, "* Picture file format : BMP\n");
        strncpy(PictureFile.file_extension, "bmp", 8);
    }
    else if (dc->output_format == PICTURE_TGA)
    {
        TRACE_1(IO, "* Picture file format : TGA\n");
        strncpy(PictureFile.file_extension, "tga", 8);
    }
    else
    {
        TRACE_1(IO, "* Picture file format : YUV 4:x:x\n");
        strncpy(PictureFile.file_extension, "yuv", 8);
    }

    strncat(PictureFile.file_name, ".", 1);
    strncat(PictureFile.file_name, PictureFile.file_extension, 254);

    // Open a write only (w) binary file (b)
    PictureFile.file_pointer = fopen(PictureFile.file_name, "wb");

    if (PictureFile.file_pointer == NULL)
    {
        TRACE_1(IO, "* Picture file path   : '%s'\n", PictureFile.file_name);
        TRACE_ERROR(IO, "* Unable to create/open file!\n");

        retcode = FAILURE;
    }
    else
    {
        sps_t *sps = dc->sps_array[dc->active_sps];

        TRACE_1(IO, "* Picture file name   : '%s'\n", PictureFile.file_name);
        TRACE_1(IO, "* Picture definition  : %ix%i\n", sps->PicWidthInMbs*16, sps->PicHeightInMapUnits*16);
        TRACE_1(IO, "* Picture file successfully created\n");

        // Start export
        if (dc->output_format == PICTURE_JPG)
        {
            retcode = export_idr_jpg(dc, PictureFile.file_pointer);
        }
        else if (dc->output_format == PICTURE_PNG)
        {
            retcode = export_idr_png(dc, &PictureFile);
        }
        else if (dc->output_format == PICTURE_BMP)
        {
            retcode = export_idr_bmp(dc, &PictureFile);
        }
        else if (dc->output_format == PICTURE_TGA)
        {
            retcode = export_idr_tga(dc, &PictureFile);
        }
        else if (dc->output_format == PICTURE_YUV420)
        {
            retcode = export_idr_yuv420(dc, PictureFile.file_pointer);
        }
        else if (dc->output_format == PICTURE_YUV444)
        {
            retcode = export_idr_yuv444(dc, PictureFile.file_pointer);
        }
        else
        {
            TRACE_ERROR(IO, "* No export file format specified!\n");
        }

        // Return value
        if (retcode == SUCCESS)
        {
            TRACE_1(IO, "* Picture content successfully written on disk\n");
            dc->picture_exported++;
        }

        // Close file
        fclose(PictureFile.file_pointer);
    }

    return retcode;
}

/* ************************************************************************** */
