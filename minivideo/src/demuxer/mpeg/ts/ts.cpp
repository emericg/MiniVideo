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
 * \file      ts.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "ts.h"
#include "ts_struct.h"
#include "../pes/pes.h"
#include "../pes/pes_struct.h"
#include "../../../minivideo_typedef.h"
#include "../../../minitraces.h"
#include "../../../bitstream_utils.h"

// C standard libraries
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

int ts_fileParse(MediaFile_t *media)
{
    int retcode = SUCCESS;

    TRACE_INFO(MPS, BLD_GREEN "ts_fileParse()" CLR_RESET);

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(media, NULL);

    if (bitstr != NULL)
    {
        // Init an MpegPs structure
        MpegTs_t mts;
        memset(&mts, 0, sizeof(MpegTs_t));

        // A convenient way to stop the parser
        mts.run = true;

        {
            // TODO
        }

        // Free bitstream
        free_bitstream(&bitstr);
    }
    else
    {
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */
