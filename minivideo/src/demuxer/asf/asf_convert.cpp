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
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

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
            retcode = asf_convert_track(media, asf, tid);
        }

        if (media->tracks_video_count == 0 && media->tracks_audio_count == 0)
        {
            TRACE_WARNING(ASF, "No tracks converted successfully!");
        }
    }

    // Free asf_t structure content
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
        retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], 0, sample_count);
        if (retcode == SUCCESS)
        {
            map = media->tracks_audio[media->tracks_audio_count];
            media->tracks_audio_count++;
        }
    }
    else if (memcmp(asf->asfh.sp[tid].StreamType, ASF_object_GUIDs[ASF_Video_Media], 16) == 0)
    {
        retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], 0, sample_count);
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
        if (asf->asfh.cl)
        {
            for (int j = 0; j < asf->asfh.cl->CodecEntriesCount; j++)
            {
                free(asf->asfh.cl->CodecEntries[j]->CodecName);
                free(asf->asfh.cl->CodecEntries[j]->CodecDescription);
                free(asf->asfh.cl->CodecEntries[j]->CodecInformation);

                free(asf->asfh.cl->CodecEntries[j]);
            }
            free(asf->asfh.cl->CodecEntries);

            free(asf->asfh.cl);
        }

        if (asf->asfh.sc)
        {
            if (asf->asfh.sc->CommandTypes)
            {
                free(asf->asfh.sc->CommandTypes->CommandTypeName);
                free(asf->asfh.sc->CommandTypes);
            }
            if (asf->asfh.sc->Commands)
            {
                free(asf->asfh.sc->Commands->CommandName);
                free(asf->asfh.sc->Commands);
            }

            free(asf->asfh.sc);
        }

        if (asf->asfh.md)
        {
            free(asf->asfh.md->Name);

            if (asf->asfh.md->Markers)
            {
                free(asf->asfh.md->Markers->MarkerDescription);
                free(asf->asfh.md->Markers);
            }

            free(asf->asfh.md);
        }

        if (asf->asfh.bme)
        {
            free(asf->asfh.bme);
        }

        if (asf->asfh.ec)
        {
            free(asf->asfh.ec->ErrorCorrectionData);
            free(asf->asfh.ec);
        }

        if (asf->asfh.cd)
        {
            free(asf->asfh.cd->Title);
            free(asf->asfh.cd->Author);
            free(asf->asfh.cd->Copyright);
            free(asf->asfh.cd->Varies);
            free(asf->asfh.cd->Description);
            free(asf->asfh.cd->Rating);

            free(asf->asfh.cd);
        }

        if (asf->asfh.ecd)
        {
            for (int i = 0; i < asf->asfh.ecd->ContentDescriptorsCount; i++)
            {
                free(asf->asfh.ecd->ContentDescriptors[i]->DescriptorName);
                free(asf->asfh.ecd->ContentDescriptors[i]->DescriptorValue_data);

                free(asf->asfh.ecd->ContentDescriptors[i]);
            }
            free(asf->asfh.ecd->ContentDescriptors);
            free(asf->asfh.ecd);
        }

        if (asf->asfh.sbp)
        {
            for (int i = 0; i < asf->asfh.sbp->BitrateRecordsCount; i++)
            {
                free(asf->asfh.sbp->BitrateRecords[i]);
            }
            free(asf->asfh.sbp->BitrateRecords);

            free(asf->asfh.sbp);
        }

        if (asf->asfh.cb)
        {
            free(asf->asfh.cb->BannerImageData);
            free(asf->asfh.cb->BannerImageURL);
            free(asf->asfh.cb->CopyrightURL);

            free(asf->asfh.cb);
        }

        if (asf->asfh.ce)
        {
            free(asf->asfh.ce->SecretData);
            free(asf->asfh.ce->ProtectionType);
            free(asf->asfh.ce->KeyID);
            free(asf->asfh.ce->LicenseURL);

            free(asf->asfh.ce);
        }

        if (asf->asfh.ece)
        {
            free(asf->asfh.ece->Data);
            free(asf->asfh.ece);
        }

        if (asf->asfh.ds)
        {
            free(asf->asfh.ds->SignatureData);
            free(asf->asfh.ds);
        }

        if (asf->asfh.pad)
        {
            free(asf->asfh.pad->PaddingData);
            free(asf->asfh.pad);
        }

        // Tracks
        for (unsigned i = 0; i < asf->tracks_count; i++)
        {
            free(asf->asfh.sp[i].TypeSpecificData);
            free(asf->asfh.sp[i].ErrorCorrectionData);

            if (asf->tracks[i])
            {
                //free(asf->tracks[i]->index_entries);
                free(asf->tracks[i]);
            }
        }

        // Header Extensions
        free(asf->asfh.ex.HeaderExtensionData);
    }
}

/* ************************************************************************** */
