/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      xml_mapper.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "xml_mapper.h"
#include "../minivideo_typedef.h"
#include "../minitraces.h"

// C POSIX library
#ifndef _MSC_VER
#include <unistd.h>
#endif

// C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <windows.h>
#include <Lmcons.h>
#elif !defined(__MINGW32__) || !defined(__MINGW64__)
#include <sys/stat.h>
#include <errno.h>
#endif

/* ************************************************************************** */

#define xmlMapper_VERSION_MAJOR 0
#define xmlMapper_VERSION_MINOR 1

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
#define MAX_PATH_SIZE 256
#else
#define MAX_PATH_SIZE 4096
#endif

/* ************************************************************************** */

int xmlMapperOpen(MediaFile_t *media, FILE **xml)
{
    TRACE_INFO(MAPPR, BLD_GREEN "xmlMapperOpen()" CLR_RESET);
    int retcode = FAILURE;

    if (media && media->container_mapper)
    {
        char xmlPath[4096] = {0};

        // File directory
#if defined(_MSC_VER)
        char tempdir[UNLEN + 1] = {0};
        if (GetTempPath(UNLEN, tempdir) != 0)
        {
            snprintf(xmlPath, UNLEN, "/%s/%s_mapped.xml", tempdir, media->file_name);
        }
#else
        char *tempdir = getenv("TMPDIR");
        char tempdir_mv[4096] = {0};

        if (tempdir)
        {
            snprintf(tempdir_mv, 4095, "/%s/minivideo", tempdir);
            snprintf(xmlPath, 4095, "/%s/minivideo/%s_mapped.xml", tempdir, media->file_name);
        }
        else
        {
            snprintf(tempdir_mv, 4095, "/tmp/minivideo");
            snprintf(xmlPath, 4095, "/tmp/minivideo/%s_mapped.xml", media->file_name);
        }
#endif

        // File opening
        if (strlen(xmlPath) > 0)
        {
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
            *xml = fopen(xmlPath, "w+");
#else
            int err = 0;
            if (strlen(tempdir_mv) > 0)
                err = mkdir(tempdir_mv, 0775);

            if (err == 0 || errno == EEXIST)
            {
                *xml = fopen(xmlPath, "w+");
                unlink(xmlPath);
            }
#endif
        }

        // Header creation
        if (*xml)
        {
            retcode = SUCCESS;

            char fileLine[255];
            snprintf(fileLine, 254, "<file xmlMapper=\"%d.%d\" minivideo=\"%d.%d-%d\">\n",
                    xmlMapper_VERSION_MAJOR, xmlMapper_VERSION_MINOR,
                    minivideo_VERSION_MAJOR, minivideo_VERSION_MINOR, minivideo_VERSION_PATCH);

            if (fprintf(*xml, "<?xml version=\"1.0\"?>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "%s", fileLine) < 0) retcode = FAILURE;
            if (fprintf(*xml, "<header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <format>%s</format>\n", getContainerString(media->container, false)) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <size>%li</size>\n", media->file_size) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <path>%s</path>\n", media->file_path) < 0) retcode = FAILURE;
            if (fprintf(*xml, "</header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "<structure>\n") < 0) retcode = FAILURE;
        }
        else
        {
            TRACE_ERROR(MAPPR, "xmlMapper could not create the file '%s'", xmlPath);
        }
    }

    return retcode;
}

/* ************************************************************************** */

int xmlMapperFinalize(FILE *xml)
{
    TRACE_INFO(MAPPR, BLD_GREEN "xmlMapperFinalize()" CLR_RESET);
    int retcode = FAILURE;

    if (xml)
    {
        retcode = SUCCESS;

        if (fprintf(xml, "</structure>\n") < 0) retcode = FAILURE;
        if (fprintf(xml, "</file>\n") < 0) retcode = FAILURE;
        rewind(xml);
    }

    return retcode;
}

int xmlMapperClose(FILE **xml)
{
    TRACE_INFO(MAPPR, BLD_GREEN "xmlMapperClose()" CLR_RESET);
    int retcode = FAILURE;

    if (*xml)
    {
        if (fclose(*xml) == 0)
            retcode = SUCCESS;

        *xml = NULL;
    }

    return retcode;
}

/* ************************************************************************** */
