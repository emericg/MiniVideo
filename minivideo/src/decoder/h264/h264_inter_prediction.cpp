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
 * \file      h264_inter_prediction.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_inter_prediction.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

/* ************************************************************************** */

/*!
 * \brief Inter prediction process.
 * \param *dc The current DecodingContext.
 * \param *mb The current macroblock.
 * \return 0 if failed, 1 otherwise.
 *
 * From 'ITU-T H.264' recommendation:
 * 8.4 Inter prediction process.
 */
int inter_prediction_process(DecodingContext_t *dc, Macroblock_t *mb)
{
    TRACE_INFO(INTER, "<> " BLD_GREEN "inter_prediction_process()" CLR_RESET);
    int retcode = FAILURE;

    TRACE_ERROR(INTER, ">>> UNSUPPORTED (inter_prediction_process())");

    return retcode;
}

/* ************************************************************************** */
