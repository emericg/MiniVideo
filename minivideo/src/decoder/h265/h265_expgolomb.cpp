/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \file      h265_expgolomb.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "h265_expgolomb.h"
#include "../../minitraces.h"

// C standard libraries
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Read the codeNum (k) for exp-golomb value decoding.
 * \param *bitstr The bitstream to read.
 * \return codeNum (k).
 */
static unsigned int get_codeNum(Bitstream_t *bitstr)
{
    // Loop until a non zero bit is found
    int leadingZeroBits = -1;
    for (int b = 0; !b; leadingZeroBits++)
    {
        b = read_bit(bitstr);
    }

    unsigned int codeNum = 0;
    if (leadingZeroBits > 0)
    {
        codeNum = std::pow(2, leadingZeroBits) - 1 + read_bits(bitstr, leadingZeroBits);
    }

    TRACE_1(EXPGO, "get_codeNum() = %i", codeNum);
    return codeNum;
}

/* ************************************************************************** */

/*!
 * \brief Read an exp-golomb coded unsigned integer.
 * \param *bitstr The bitstream to read.
 * \return Unsigned integer syntax element, left bit first.
 */
unsigned int h265_read_ue(Bitstream_t *bitstr)
{
    TRACE_1(EXPGO, "h265_read_ue()");
    return get_codeNum(bitstr);
}

/* ************************************************************************** */

/*!
 * \brief Read an exp-golomb coded integer.
 * \param *bitstr The bitstream to read.
 * \return Signed integer syntax element, left bit first.
 * \todo performance optimisation (see 9.1.1 Mapping process for signed Exp-Golomb code)
 */
int h265_read_se(Bitstream_t *bitstr)
{
    unsigned int codeNum = get_codeNum(bitstr);
    int se = std::pow(-1.0, codeNum+1) * std::ceil(codeNum/2.0);

    TRACE_1(EXPGO, "h265_read_se(k:%i) = %i", codeNum, se);
    return se;
}

/* ************************************************************************** */
