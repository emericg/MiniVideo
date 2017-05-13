/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
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
 * \file      mkv_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mkv.h"
#include "mkv_struct.h"
#include "ebml.h"

#include "mkv_tracks.h"

#include "../xml_mapper.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

int mkv_convert(MediaFile_t *media, mkv_t *mkv)
{
    int status = SUCCESS;

    media->container_profile = mkv->profile;

    return status;
}

/* ************************************************************************** */

int mkv_convert_track(MediaFile_t *media, mkv_t *mkv)
{
    int status = SUCCESS;

    //

    return status;
}

/* ************************************************************************** */

void mkv_clean(mkv_t *mkv)
{
    // ebml_header_t
    free(mkv->ebml.DocType);

    // mkv_info_t
    free(mkv->info.SegmentUID);
    free(mkv->info.SegmentFilename);
    free(mkv->info.PrevUID);
    free(mkv->info.PrevFilename);
    free(mkv->info.NextUID);
    free(mkv->info.NextFilename);
    free(mkv->info.SegmentFamily);
    for (int i = 0; i < mkv->info.chapter_count; i++)
    {
        // mkv_info_chapter_t // TODO
        //free(mkv->info.chapter[i].ChapterTranslateID);
    }
    free(mkv->info.Title);
    free(mkv->info.MuxingApp);
    free(mkv->info.WritingApp);
}

/* ************************************************************************** */
