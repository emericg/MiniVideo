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

// C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

#define xmlMapper_VERSION_MAJOR 0
#define xmlMapper_VERSION_MINOR 1

/* ************************************************************************** */

int xmlMapperOpen(MediaFile_t *media, FILE **xml)
{
    TRACE_INFO(MAPPR, BLD_GREEN "xmlMapperOpen()" CLR_RESET);
    int retcode = SUCCESS;

    if (media && media->container_mapper)
    {
        char xmlPath[4096];
        memset(xmlPath, 0, 4096);
        sprintf(xmlPath, "/tmp/%s_mapped.xml", media->file_name);

        *xml = fopen(xmlPath, "w+");
        if (*xml)
        {
            char fileLine[255];
            sprintf(fileLine, "<file xmlMapper=\"%d.%d\" minivideo=\"%d.%d-%d\">\n",
                    xmlMapper_VERSION_MAJOR, xmlMapper_VERSION_MINOR,
                    minivideo_VERSION_MAJOR, minivideo_VERSION_MINOR, minivideo_VERSION_PATCH);

            if (fprintf(*xml, "<?xml version=\"1.0\"?>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, fileLine) < 0) retcode = FAILURE;
            if (fprintf(*xml, "<header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <format>%s</format>\n", getContainerString(media->container, false)) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <size>%li</size>\n", media->file_size) < 0) retcode = FAILURE;
            if (fprintf(*xml, "  <path>%s</path>\n", media->file_path) < 0) retcode = FAILURE;
            if (fprintf(*xml, "</header>\n") < 0) retcode = FAILURE;
            if (fprintf(*xml, "<structure>\n") < 0) retcode = FAILURE;
        }
        else
        {
            TRACE_ERROR(MAPPR, "xmlMapper could not create file '%s'", xmlPath);
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */

int xmlMapperClose(FILE **xml)
{
    TRACE_INFO(MAPPR, BLD_GREEN "xmlMapperClose()" CLR_RESET);
    int retcode = SUCCESS;

    if (*xml)
    {
        if (fprintf(*xml, "</structure>\n") < 0) retcode = FAILURE;
        if (fprintf(*xml, "</file>\n") < 0) retcode = FAILURE;
        if (fclose(*xml) != 0) retcode = FAILURE;
        *xml = NULL;
    }

    return retcode;
}

/* ************************************************************************** */
