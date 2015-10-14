/*!
 * COPYRIGHT (C) 2015 Emeric Grange - All Rights Reserved
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
 * \file      wav.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2015
 */

// minivideo headers
#include "wav.h"
#include "wav_struct.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ************************************************************************** */

int wav_fileParse(VideoFile_t *video)
{
    int retcode = FAILURE;

    TRACE_INFO(WAV, BLD_GREEN "wav_fileParse()\n" CLR_RESET);

    return retcode;
}

/* ************************************************************************** */
