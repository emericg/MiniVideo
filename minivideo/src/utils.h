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
 * \file      utils.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

#ifndef UTILS_H
#define UTILS_H

// C standard libraries
#include <cstdint>

/* ************************************************************************** */

#define ALIGN_ADDR(_addr,_align) {\
   if (_align>1) {\
       int _dec = (_addr%_align);\
       if (_dec != 0) _addr += _align - _dec;\
   }\
}

#define ALIGN(x, y) (((x) + ((y) - 1)) & ~((y) - 1))

#define MAX(x, y) (((x) > (y)) ? (x):(y))

#define MIN(x, y) (((x) < (y)) ? (x):(y))

#define BITS_TO_BYTES (x) (((x) + 7) >> 3)

#define BYTES_TO_BITS (x) ((x) << 3)

#define TABLESIZE(t) (sizeof(t) / sizeof(t[0]))

/* ************************************************************************** */

static const int raster_4x4[16] = {0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15};

static const int raster_4x4_2d[16][2] =
{
    {0,0}, {0,1}, {1,0}, {1,1},
    {0,2}, {0,3}, {1,2}, {1,3},
    {2,0}, {2,1}, {3,0}, {3,1},
    {2,2}, {2,3}, {3,2}, {3,3}
};

static const int raster_8x8[4] = {0, 1, 2, 3};

static const int raster_8x8_2d[4][2] = {{0,0}, {0,1}, {1,0}, {1,1}};

/* ************************************************************************** */

static const int zigzag_4x4[16] = {0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15};

static const int zigzag_4x4_2d[16][2] =
{
    {0,0}, {0,1}, {1,0}, {2,0},
    {1,1}, {0,2}, {0,3}, {1,2},
    {2,1}, {3,0}, {3,1}, {2,2},
    {1,3}, {2,3}, {3,2}, {3,3}
};

static const int zigzag_8x8[64] =
{
     0,  1,  5,  6, 14, 15, 27, 28,  2,  4,  7, 13, 16, 26, 19, 42,
     3,  8, 12, 17, 25, 30, 41, 43,  9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63
};

static const int zigzag_8x8_2d[64][2] =
{
    {0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2}, {2,1}, {3,0}, {4,0}, {3,1}, {2,2}, {1,3}, {0,4}, {0,5},
    {1,4}, {2,3}, {3,2}, {4,1}, {5,0}, {6,0}, {5,1}, {4,2}, {3,3}, {2,4}, {1,5}, {0,6}, {0,7}, {1,6}, {2,5}, {3,4},
    {4,3}, {5,2}, {6,1}, {7,0}, {7,1}, {6,2}, {5,3}, {4,4}, {3,5}, {2,6}, {1,7}, {2,7}, {3,6}, {4,5}, {5,4}, {6,3},
    {7,2}, {7,3}, {6,4}, {5,5}, {4,6}, {3,7}, {4,7}, {5,6}, {6,5}, {7,4}, {7,5}, {6,6}, {5,7}, {6,7}, {7,6}, {7,7},
};

/* ************************************************************************** */

int **malloc2d(const int x, const int y);

int **calloc2d(const int x, const int y);

void free2d(int ***array2d_ptr, const int x);

void print2d(int **array2d, int arraySize);

/* ************************************************************************** */

int is_prime(const unsigned int n);

int fast_mod8(const int n);

int fast_div8(const int n);

/* ************************************************************************** */

short smin(short a, short b);

short smax(short a, short b);

int imin(int a, int b);

int imax(int a, int b);

int imedian(int a, int b, int c);

double dmin(double a, double b);

double dmax(double a, double b);

int64_t i64min(int64_t a, int64_t b);

int64_t i64max(int64_t a, int64_t b);

short sabs(short x);

int iabs(int x);

double dabs(double x);

int64_t i64abs(int64_t x);

double dabs2(double x);

int iabs2(int x);

int64_t i64abs2(int64_t x);

int isign(int x);

int isignab(int a, int b);

int rshift_rnd(int x, int a);

int rshift_rnd_sign(int x, int a);

unsigned int rshift_rnd_us(unsigned int x, unsigned int a);

int rshift_rnd_sf(int x, int a);

int shift_off_sf(int x, int o, int a);

unsigned int rshift_rnd_us_sf(unsigned int x, unsigned int a);

int iClip1(int high, int x);

short sClip1(short high, short x);

double dClip1(double high, double x);

int iClip3(int low, int high, int x);

short sClip3(short low, short high, short x);

double dClip3(double low, double high, double x);

/* ************************************************************************** */

int iClip1_YCbCr(const int x, const int BitDepth);

int iClip1_YCbCr_8(const int x);

uint8_t Clip1_YCbCr_8(const int x);

/* ************************************************************************** */
#endif // UTILS_H
