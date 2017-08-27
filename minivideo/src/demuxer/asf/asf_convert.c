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

int asf_indexer(Bitstream_t *bitstr, MediaFile_t *media, asf_t *asf)
{
    TRACE_INFO(ASF, BLD_GREEN "asf_indexer()" CLR_RESET);
    int retcode = SUCCESS;
    unsigned int i = 0, j = 0;

    for (i = 0; i < asf->tracks_count; i++)
    {
        if (asf->tracks[i]->track_indexed == 1)
        {
            TRACE_1(ASF, "> track already indexed");
        }
        else
        {
            //
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Free the allocated content of a asf_t structure.
 * \param *asf A pointer to a asf_t structure.
 */
void asf_clean(asf_t *asf)
{
    if (asf)
    {
        unsigned int i = 0;
        for (i = 0; i < asf->tracks_count; i++)
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
