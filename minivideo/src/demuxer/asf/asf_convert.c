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
 * \file      asf_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "asf_convert.h"
#include "asf_object.h"
#include "asf_struct.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_twocc.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ************************************************************************** */

int asf_convert(Bitstream_t *bitstr, MediaFile_t *media, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "asf_convert()" CLR_RESET);
    int retcode = SUCCESS;

    // File metadata
    media->duration = asf->asfh.fp.SendDuration / 10000; // period of 100ns to ms
    media->creation_time = WindowsTickToUnixSeconds(asf->asfh.fp.CreationDate); // period of 100ns from january 1 1601 to Unix time

    // Tracks metadata
    if (asf->tracks_count == 0) // Check if we have extracted tracks
    {
        TRACE_WARNING(ASF, "No tracks extracted from ASF file!");
    }
    else // Convert tracks
    {
        for (unsigned tid = 0; tid < asf->tracks_count; tid++)
        {
            //if (asf->asfh.sp[tid])
            {
                retcode = asf_convert_track(media, asf, tid);
            }
        }

        if (media->tracks_video_count == 0 && media->tracks_audio_count == 0)
        {
            TRACE_WARNING(ASF, "No tracks converted successfully!");
        }
    }

    asf_clean(asf);

    return retcode;
}

/* ************************************************************************** */

int asf_convert_track(MediaFile_t *media, asf_t *asf, uint32_t tid)
{
    TRACE_INFO(ASF, BLD_GREEN "asf_convert_track()" CLR_RESET);
    int retcode = SUCCESS;
    MediaStream_t *map = NULL;

    if (media == NULL)
    {
        TRACE_ERROR(MKV, "Cannot access audio or video tracks from the ASF parser!");
        retcode = FAILURE;
    }

    // Select and init a bitstream map (A or V)
    ////////////////////////////////////////////////////////////////////////////

    int sample_count = 1;

    if (memcmp(asf->asfh.sp[tid].StreamType, ASF_object_GUIDs[ASF_Audio_Media], 16) == 0)
    {
        retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], sample_count);
        if (retcode == SUCCESS)
        {
            map = media->tracks_audio[media->tracks_audio_count];
            media->tracks_audio_count++;
        }
    }
    else if (memcmp(asf->asfh.sp[tid].StreamType, ASF_object_GUIDs[ASF_Video_Media], 16) == 0)
    {
        retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], sample_count);
        if (retcode == SUCCESS)
        {
            map = media->tracks_video[media->tracks_video_count];
            media->tracks_video_count++;
        }
    }
    else
    {
        //
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Free the allocated content of a asf_t structure.
 * \param *asf A pointer to a asf_t structure.
 */
void asf_clean(asf_t *asf)
{
    if (asf)
    {
/*
        for (unsigned i = 0; i < asf->tracks_count; i++)
        {
            free(asf->asfh.sp[i].TypeSpecificData);
            free(asf->asfh.sp[i].ErrorCorrectionData);
        }
*/
        if (asf->asfh.cl)
        {
            //for (int j = 0; j < asf->asfh.cl->CodecEntriesCount; j++)
            //    free(asf->asfh.cl->CodecEntries[j]);
            //free(asf->asfh.cl->CodecEntries);
            free(asf->asfh.cl);
        }

        //
        for (unsigned i = 0; i < asf->tracks_count; i++)
        {
            if (asf->tracks[i])
            {
                //free(asf->tracks[i]->index_entries);
                free(asf->tracks[i]);
            }
        }
    }
}

/* ************************************************************************** */
