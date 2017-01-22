/*!
 * COPYRIGHT (C) 2010 Emeric Grange - All Rights Reserved
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
 * \file      h264_cavlc_tables.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef H264_CAVLC_TABLES_H
#define H264_CAVLC_TABLES_H

#include "../../minivideo_typedef.h"

/* ************************************************************************** */

#define TOTRUN_NUM        15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

/* ************************************************************************** */

const uint8_t coefftoken_table[6][TOTRUN_NUM][32][3] =
{ // [nC][LeadingZeros][coeff_token][TotalCoeffs][TrailingOnes]
    { // nC == [0,2[
        { {0} }, // t0 unused
        { {0} }, // t1 unused
        { {0} }, // t2 unused
        {
            { 1, 1, 0}, { 0, 2, 1}, // V
        }, // t3
        {
            { 1, 3, 2}, { 0, 5, 3}, // V
        }, // t4
        {
            { 3, 2, 0}, { 2, 3, 1}, { 1, 4, 2}, { 0, 6, 3},
        }, // t5
        {
            { 3, 3, 0}, { 2, 4, 1}, { 1, 5, 2}, { 0, 7, 3},
        }, // t6
        {
            { 3, 4, 0}, { 2, 5, 1}, { 1, 6, 2}, { 0, 8, 3},
        }, // t7
        {
            { 3, 5, 0}, { 2, 6, 1}, { 1, 7, 2}, { 0, 9, 3},
        }, // t8
        {
            { 7, 6, 0}, { 6, 7, 1}, { 5, 8, 2}, { 4,10, 3},
            { 3, 7, 0}, { 2, 8, 1}, { 1, 9, 2},
            { 0, 8, 0},
        }, // t9
        {
            { 7, 9, 0}, { 6, 9, 1}, { 5,10, 2}, { 4,11, 3},
            { 3,10, 0}, { 2,10, 1}, { 1,11, 2}, { 0,12, 3},
        }, // t10
        {
            { 7,11, 0}, { 6,11, 1}, { 5,12, 2}, { 4,13, 3},
            { 3,12, 0}, { 2,12, 1}, { 1,13, 2}, { 0,14, 3},
        }, // t11
        {
            { 7,13, 0}, { 6,14, 1}, { 5,14, 2}, { 4,15, 3},
            { 3,14, 0}, { 2,15, 1}, { 1,15, 2}, { 0,16, 3},
        }, // t12
        {
            { 3,15, 0}, { 2,16, 1}, { 1,16, 2}, { 0,16, 0},
        }, // t13
        { {0} }, // t14 - unused
    },
    { // nC == [2,4[ // ========================================================
        {
            { 0, 1, 1}, { 1, 0, 0}
        }, // t0
        {
            { 0, 4, 3}, { 1, 3, 3} // V
        }, // t1
        {
            { 0, 6, 3}, { 1, 3, 2}, { 2, 3, 1}, { 3, 1, 0} // V V
        }, // t2
        {
            { 0, 7, 3}, { 1, 4, 2}, { 2, 4, 1}, { 3, 2, 0}
        }, // t3
        {
            { 0, 8, 3}, { 1, 5, 2}, { 2, 5, 1}, { 3, 3, 0},
        }, // t4
        {
            { 0, 5, 0}, { 1, 6, 2}, { 2, 6, 1}, { 3, 4, 0},
        }, // t5
        {
            { 0, 9, 3}, { 1, 7, 2}, { 2, 7, 1}, { 3, 6, 0},
        }, // t6
        {
            { 0,11, 3}, { 1, 9, 2}, { 2, 9, 1}, { 3, 8, 0},
            { 4,10, 3}, { 5, 8, 2}, { 6, 8, 1}, { 7, 7, 0}
        }, // t7
        {
            { 0,11, 0}, { 1,11, 2}, { 2,11, 1}, { 3,10, 0},
            { 4,12, 3}, { 5,10, 2}, { 6,10, 1}, { 7, 9, 0}
        }, // t8
        {
            { 0,14, 3}, { 1,13, 2}, { 2,13, 1}, { 3,13, 0},
            { 4,13, 3}, { 5,12, 2}, { 6,12, 1}, { 7,12, 0}
        }, // t9
        {
            { 0,15, 1}, { 1,15, 0}, { 2,15, 2}, { 3,14, 1} // V V
        }, // t10
        {
            { 0,16, 3}, { 1,16, 2}, { 2,16, 1}, { 3,16, 0},
        }, // t11
        { {0} }, // t12 - unused
    },
    { // nC == [4,8[ // ========================================================
        {
            { 0, 7, 3}, { 1, 6, 3}, { 2, 5, 3}, { 3, 4, 3},
            { 4, 3, 3}, { 5, 2, 2}, { 6, 1, 1}, { 7, 0, 0}
        }, // t0
        {
            { 0, 5, 1}, { 1, 5, 2}, { 2, 4, 1}, { 3, 4, 2},
            { 4, 3, 1}, { 5, 8, 3}, { 6, 3, 2}, { 7, 2, 1}
        }, // t1
        {
            { 0, 3, 0}, { 1, 7, 2}, { 2, 7, 1}, { 3, 2, 0},
            { 4, 9, 3}, { 5, 6, 2}, { 6, 6, 1}, { 7, 1, 0}
        }, // t2
        {
            { 0, 7, 0}, { 1, 6, 0}, { 2, 9, 2}, { 3, 5, 0},
            { 4,10, 3}, { 5, 8, 2}, { 6, 8, 1}, { 7, 4, 0}
        }, // t3
        {
            { 0,12, 3}, { 1,11, 2}, { 2,10, 1}, { 3, 9, 0},
            { 4,11, 3}, { 5,10, 2}, { 6, 9, 1}, { 7, 8, 0}
        }, // t4
        {
            { 0,12, 0}, { 1,13, 2}, { 2,12, 1}, { 3,11, 0},
            { 4,13, 3}, { 5,12, 2}, { 6,11, 1}, { 7,10, 0}
        }, // t5
        {
            { 0,15, 1}, { 1,14, 0}, { 2,14, 3}, { 3,14, 2},
            { 4,14, 1}, { 5,13, 0} // V
        }, // t6
        {
            { 0,16, 1}, { 1,15, 0}, { 2,15, 3}, { 3,15, 2},
        }, // t7
        {
            { 0,16, 3}, { 1,16, 2}
        }, // t8
        { {0} }, // t9 - unused
    },
    { // nC == [8,16] // =======================================================
        {
            { 0, 9, 0}, { 1, 9, 1}, { 2, 9, 2}, { 3, 9, 3},
            { 4,10, 0}, { 5,10, 1}, { 6,10, 2}, { 7,10, 3},
            { 8,11, 0}, { 9,11, 1}, {10,11, 2}, {11,11, 3},
            {12,12, 0}, {13,12, 1}, {14,12, 2}, {15,12, 3},
            {16,13, 0}, {17,13, 1}, {18,13, 2}, {19,13, 3},
            {20,14, 0}, {21,14, 1}, {22,14, 2}, {23,14, 3},
            {24,15, 0}, {25,15, 1}, {26,15, 2}, {27,15, 3},
            {28,16, 0}, {29,16, 1}, {30,16, 2}, {31,16, 3}
        }, // t0
        {
            { 0, 5, 0}, { 1, 5, 1}, { 2, 5, 2}, { 3, 5, 3},
            { 4, 6, 0}, { 5, 6, 1}, { 6, 6, 2}, { 7, 6, 3},
            { 8, 7, 0}, { 9, 7, 1}, {10, 7, 2}, {11, 7, 3},
            {12, 8, 0}, {13, 8, 1}, {14, 8, 2}, {15, 8, 3}
        }, // t1
        {
            { 0, 3, 0}, { 1, 3, 1}, { 2, 3, 2}, { 3, 3, 3},
            { 4, 4, 0}, { 5, 4, 1}, { 6, 4, 2}, { 7, 4, 3}
        }, // t2
        {
            { 0, 2, 0}, { 1, 2, 1}, { 2, 2, 2}
        }, // t3
        {
            { 1, 0, 0}
        }, // t4
        {
            { 0, 1, 1}
        }, // t5
        {
            { 0, 1, 0}
        }, // t6
    },
    { // nC == -1 // ===========================================================
        { {0} }, // t0 unused
        { {0} }, // t1 unused
        { {0} }, // t2 unused
        {
            { 0, 2, 0}, { 1, 3, 3}, { 2, 2, 1}, { 3, 1, 0}
        }, // t3
        {
            { 0, 4, 0}, { 1, 3, 0}
        }, // t4
        {
            { 0, 3, 2}, { 1, 3, 1}
        }, // t5
        {
            { 0, 4, 2}, { 1, 4, 1}
        }, // t6
        { {0} }, // t7 unused
    },
    { // nC == -2 // ===========================================================
        { {0} }, // t0 unused
        { {0} }, // t1 unused
        { {0} }, // t2 unused
        {
            { 0, 6, 3}, { 1, 5, 3}, { 2, 4, 2}, { 3, 3, 2},
            { 4, 3, 1}, { 5, 2, 1}, { 6, 2, 0}, { 7, 1, 0}
        }, // t3
        { {0} }, // t4 unused
        { {0} }, // t5 unused
        {
            { 0, 5, 2}, { 1, 4, 1}, { 2, 4, 0}, { 3, 3, 0}
        }, // t6
        {
            { 0, 7, 3}, { 1, 6, 2}, { 2, 5, 1}, { 3, 5, 0}
        }, // t7
        {
            { 0, 8, 3}, { 1, 7, 2}, { 2, 6, 1}, { 3, 6, 0}
        }, // t8
        {
            { 0, 8, 2}, { 1, 8, 1}, { 2, 7, 1}, { 3, 7, 0},
        }, // t9
        {
            { 3, 8, 0}
        }, // t10
    }
};

/* ************************************************************************** */

const uint8_t totalzeros_lentab[TOTRUN_NUM][16] =
{
    {1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
    {3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
    {4,3,3,3,4,4,3,3,4,5,5,6,5,6},
    {5,3,4,4,3,3,3,4,3,4,5,5,5},
    {4,4,4,3,3,3,3,3,4,5,4,5},
    {6,5,3,3,3,3,3,3,4,3,6},
    {6,5,3,3,3,2,3,4,3,6},
    {6,4,5,3,2,2,3,3,6},
    {6,6,4,2,2,3,2,5},
    {5,5,3,2,2,2,4},
    {4,4,3,3,1,3},
    {4,4,2,1,3},
    {3,3,1,2},
    {2,2,1},
    {1,1},
};

const uint8_t totalzeros_codtab[TOTRUN_NUM][16] =
{
    {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
    {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
    {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
    {3,7,5,4,6,5,4,3,3,2,2,1,0},
    {5,4,3,7,6,5,4,3,2,1,1,0},
    {1,1,7,6,5,4,3,2,1,1,0},
    {1,1,5,4,3,3,2,1,1,0},
    {1,1,1,3,3,2,2,1,0},
    {1,0,1,3,2,1,1,1},
    {1,0,1,3,2,1,1},
    {0,1,1,2,1,3},
    {0,1,1,1,1},
    {0,1,1,1},
    {0,1,1},
    {0,1},
};

const uint8_t totalzeros_chromadc_lentab[2][7][8] =
{
    { // YUV 420
        {1,2,3,3},
        {1,2,2},
        {1,1}
    },
    { // YUV 422
        {1,3,3,4,4,4,5,5},
        {3,2,3,3,3,3,3},
        {3,3,2,2,3,3},
        {3,2,2,2,3},
        {2,2,2,2},
        {2,2,1},
        {1,1}
    }
};

const uint8_t totalzeros_chromadc_codtab[2][7][8] =
{
    { // YUV 420
        {1,1,1,0},
        {1,1,0},
        {1,0}
    },
    { // YUV 422
        {1,2,3,2,3,1,1,0},
        {0,1,1,4,5,6,7},
        {0,1,1,2,6,7},
        {6,0,1,2,7},
        {0,1,2,3},
        {0,1,1},
        {0,1}
    }
};

/* ************************************************************************** */

const uint8_t runbefore_lentab[TOTRUN_NUM][16] =
{
    {1,1},
    {1,2,2},
    {2,2,2,2},
    {2,2,2,3,3},
    {2,2,3,3,3,3},
    {2,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
};

const uint8_t runbefore_codtab[TOTRUN_NUM][16] =
{
    {1,0},
    {1,1,0},
    {3,2,1,0},
    {3,2,1,1,0},
    {3,2,3,2,1,0},
    {3,0,1,3,2,5,4},
    {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
};

/* ************************************************************************** */
#endif // H264_CAVLC_TABLES_H
