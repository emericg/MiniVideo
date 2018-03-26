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
 * \file      xml_mapper.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "xml_mapper.h"
#include "../minitraces.h"
#include "../minivideo_containers.h"
#include "../minivideo_typedef.h"

// C POSIX library
#ifndef _MSC_VER
#include <unistd.h>
#endif

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cinttypes>

#if defined(_MSC_VER)
#include <windows.h>
#include <Lmcons.h>
#endif

#if defined(ENABLE_MEMFD) && defined(__linux__)
#include "../thirdparty/memfd_wrapper.h"
#endif

/* ************************************************************************** */

#define xmlMapper_VERSION_MAJOR 0
#define xmlMapper_VERSION_MINOR 2

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
        // File path selection
        char xmlMapPath[MAX_PATH_SIZE] = {0};

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
#if defined(_MSC_VER)
        char tempdir_mv[MAX_PATH_SIZE] = {0};
        if (GetTempPath(MAX_PATH_SIZE, tempdir_mv) != 0)
        {
            snprintf(xmlMapPath, MAX_PATH_SIZE, "%s\\%s_mapped.xml", tempdir_mv, media->file_name);
        }
#elif defined(__MINGW32__) || defined(__MINGW64__)
        char *tempdir = getenv("TEMP");
        if (tempdir)
        {
            snprintf(xmlMapPath, MAX_PATH_SIZE, "%s\\%s", tempdir, media->file_name);
        }
#endif // _MSC_VER
#else
        char *tempdir = getenv("TMPDIR");
        if (tempdir && strnlen(tempdir, MAX_PATH_SIZE) < MAX_PATH_SIZE)
        {
            snprintf(xmlMapPath, MAX_PATH_SIZE, "%s/%s_mapped.xml", tempdir, media->file_name);
        }
        else
        {
            snprintf(xmlMapPath, MAX_PATH_SIZE, "/tmp/%s_mapped.xml", media->file_name);
        }
#endif
        // File path fallback: put the xmlMap next to its media file
        if (strlen(xmlMapPath) == 0)
        {
            snprintf(xmlMapPath, MAX_PATH_SIZE, "%s\%s_mapped.xml", media->file_directory, media->file_name);
        }

        // File creation
        if (strlen(xmlMapPath) > 0)
        {
            TRACE_1(MAPPR, "xmlMapPath: '%s'", xmlMapPath);

#if defined(ENABLE_MEMFD) && defined(__linux__)
            *xml = memfd_fopen(xmlMapPath, "w+");
#else
            *xml = fopen(xmlMapPath, "w+");
            unlink(xmlMapPath);
#endif
        }

        // File header creation
        if (*xml)
        {
            retcode = SUCCESS;

            if (fprintf(*xml, "<?xml version=\"1.0\" encoding=\"ASCII\" standalone=\"yes\"?>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "<file xmlMapper=\"%d.%d\" minivideo=\"%d.%d-%d\">\n",
                        xmlMapper_VERSION_MAJOR, xmlMapper_VERSION_MINOR,
                        minivideo_VERSION_MAJOR, minivideo_VERSION_MINOR, minivideo_VERSION_PATCH) < 0) retcode = FAILURE;
            if (fprintf(*xml, "<header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <format>%s</format>\n", getContainerString(media->container, false)) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <size>%" PRId64 "</size>\n", media->file_size) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <path>%s</path>\n", media->file_path) < 0) retcode = FAILURE;
            if (fprintf(*xml, "</header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "<structure>\n") < 0) retcode = FAILURE;
        }
        else
        {
            TRACE_ERROR(MAPPR, "xmlMapper could not create the file '%s'", xmlMapPath);
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

void xmlSpacer(FILE *xml, const char *name, const int index)
{
    if (xml && name)
    {
        if (index < 0)
        {
            fprintf(xml, "  <spacer>%s</spacer>\n", name);
        }
        else
        {
            char SpaceTitle[32];
            snprintf(SpaceTitle, 32, "%s #%i", name, index);
            fprintf(xml, "  <spacer>%s</spacer>\n", SpaceTitle);
        }
    }
}
