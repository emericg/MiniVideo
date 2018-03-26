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
 * \file      h264_expgolomb.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264_expgolomb.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cmath>
#include <cstdio>
#include <cstdlib>

/* ************************************************************************** */

//! Gives coded_block_pattern value from ChromaArrayType and codeNum values, both for intra and inter prediction modes.
const uint8_t NCBP[2][48][2] =
{ // [ChromaArrayType][codeNum][intra/inter coding]
    {   // 0      1        2       3       4       5       6       7       8       9      10      11
        {15, 0},{ 0, 1},{ 7, 2},{11, 4},{13, 8},{14, 3},{ 3, 5},{ 5,10},{10,12},{12,15},{ 1, 7},{ 2,11},
        { 4,13},{ 8,14},{ 6, 6},{ 9, 9},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
        { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
        { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}
    },
    {   // 0      1        2       3       4       5       6       7       8       9      10      11
        {47, 0},{31,16},{15, 1},{ 0, 2},{23, 4},{27, 8},{29,32},{30, 3},{ 7, 5},{11,10},{13,12},{14,15},
        {39,47},{43, 7},{45,11},{46,13},{16,14},{ 3, 6},{ 5, 9},{10,31},{12,35},{19,37},{21,42},{26,44},
        {28,33},{35,34},{37,36},{42,40},{44,39},{ 1,43},{ 2,45},{ 4,46},{ 8,17},{17,18},{18,20},{20,24},
        {24,19},{ 6,21},{ 9,26},{22,28},{25,23},{32,27},{33,29},{34,30},{36,22},{40,25},{38,38},{41,41}
    }
};

static unsigned int get_codeNum(Bitstream_t *bitstr);

/* ************************************************************************** */

/*!
 * \brief Read the codeNum (k) for exp-golomb value decoding.
 * \param *bitstr The bitstream to read.
 * \return codeNum (k).
 */
static unsigned int get_codeNum(Bitstream_t *bitstr)
{
    TRACE_1(EXPGO, "get_codeNum()");

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

    TRACE_1(EXPGO, "codeNum  = %i", codeNum);
    return codeNum;
}

/* ************************************************************************** */

/*!
 * \brief Read an exp-golomb coded unsigned integer.
 * \param *bitstr The bitstream to read.
 * \return Unsigned integer syntax element, left bit first.
 */
unsigned int read_ue(Bitstream_t *bitstr)
{
    TRACE_1(EXPGO, "read_ue()");

    return get_codeNum(bitstr);
}

/* ************************************************************************** */

/*!
 * \brief Read an exp-golomb coded integer.
 * \param *bitstr The bitstream to read.
 * \return Signed integer syntax element, left bit first.
 * \todo performance optimisation (see 9.1.1 Mapping process for signed Exp-Golomb code)
 */
int read_se(Bitstream_t *bitstr)
{
    TRACE_1(EXPGO, "read_se()");

    unsigned int codeNum = get_codeNum(bitstr);
    int se = std::pow(-1.0, codeNum+1) * std::ceil(codeNum/2.0);

    TRACE_1(EXPGO, "read_se(k:%i) = %i", codeNum, se);
    return se;
}

/* ************************************************************************** */

/*!
 * \brief Read an exp-golomb coded mapped element.
 * \param *bitstr The bitstream to read.
 * \param ChromaArrayType Specifies the color sampling format.
 * \param intracoding_flag Specifies if the current frame use intra or inter coding.
 * \return Mapped syntax element 'coded_block_pattern', left bit first.
 *
 * This function check if values for coded_block_pattern and ChromaArrayType are
 * valid, then fetch the syntax element value, mapped into the NCBP table.
 */
unsigned int read_me(Bitstream_t *bitstr, unsigned int ChromaArrayType, bool intracoding_flag)
{
    TRACE_1(EXPGO, "read_me()");

    unsigned cat = 1;
    if (ChromaArrayType == 0 || ChromaArrayType == 3)
        cat = 0;

    // Fetch syntax element 'coded_block_pattern' value
    unsigned coded_block_pattern = NCBP[cat][get_codeNum(bitstr)][!intracoding_flag];

    TRACE_1(EXPGO, "coded_block_pattern = %i", coded_block_pattern);
    return coded_block_pattern;
}

/* ************************************************************************** */

/*!
 * \brief Read an trunkated exp-golomb coded syntax element.
 * \param *bitstr The bitstream to read.
 * \param range The upper the range of possible values for the syntax element.
 * \return Trunkated syntax element, left bit first.
 */
unsigned int read_te(Bitstream_t *bitstr, int range)
{
    TRACE_1(EXPGO, "read_te()");
    unsigned int codeNum = 0;

    if (range > 1)
    {
        codeNum = get_codeNum(bitstr);
    }
    else
    {
        codeNum = !read_bit(bitstr);
    }

    TRACE_1(EXPGO, "te = %i", codeNum);
    return codeNum;
}

/* ************************************************************************** */
