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
 * \file      utils.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "utils.h"
#include "minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>

// C++ standard libraries
#include <limits>
#include <algorithm>

/* ************************************************************************** */

int **malloc2d(const int x, const int y)
{
    int **array2d = (int **)malloc(x * sizeof(int *));

    if (array2d == NULL)
    {
        TRACE_ERROR(TOOL, "malloc2d: memory allocation failed on array2d[]!");
    }
    else
    {
        for (int i = 0; i < x; i++)
        {
            array2d[i] = (int *)malloc(y * sizeof(int));

            if (array2d[i] == NULL)
            {
                TRACE_ERROR(TOOL, "malloc2d: memory allocation failed on array2d[%i][]!", i);
                free2d(&array2d, x);
            }
        }
    }

    return array2d;
}

/* ************************************************************************** */

int **calloc2d(const int x, const int y)
{
    int **array2d = (int **)calloc(x, sizeof(int *));

    if (array2d == NULL)
    {
        TRACE_ERROR(TOOL, "calloc2d: memory allocation failed on array2d[]!");
    }
    else
    {
        for (int i = 0; i < x; i++)
        {
            array2d[i] = (int *)calloc(y, sizeof(int));

            if (array2d[i] == NULL)
            {
                TRACE_ERROR(TOOL, "calloc2d: memory allocation failed on array2d[%i][]!", i);
                free2d(&array2d, x);
            }
        }
    }

    return array2d;
}

/* ************************************************************************** */

void free2d(int ***array2d_ptr, const int x)
{
    if (**array2d_ptr)
    {
        for (int i = 0; i < x; i++)
        {
            if (*array2d_ptr[i])
            {
                free(*array2d_ptr[i]);
                *array2d_ptr[i] = NULL;
            }
        }

        free(**array2d_ptr);
        **array2d_ptr = NULL;
    }
}

/* ************************************************************************** */

void print2d(int **array2d, int arraySize)
{
#if ENABLE_DEBUG
    TRACE_1(TOOL, "print2d()");

    int x = 0, y = 0;
    for (x = 0; x < arraySize; x++)
    {
        if (x % arraySize == 0)
        {
            printf("+-------------------+\n");
        }

        for (y = 0; y < arraySize; y++)
        {
            if (y % arraySize == 0)
            {
                printf("|");
            }
            else
            {
                printf(",");
            }

            printf("%4i", array2d[x][y]);
        }
        printf("|\n");
    }
    printf("+-------------------+\n");
#endif // ENABLE_DEBUG
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Test if n is a prime number.
 * \param n The number to test.
 * \return 1 if n is a prime number, 0 otherwise.
 * \todo This function has not been tested !
 *
 * This function will only operate if n < 10000, in order to save cpu cycles.
 */
int is_prime(const unsigned int n)
{
    if (n > 9999)
    {
        TRACE_WARNING(TOOL, "is_prime(%i) will not be computed, too big!", n);
        return 0;
    }

    for (unsigned i = 2; i < sqrt(n); i += 2)
    {
        if ((n % i == 0) && (i != n))
        {
            return 0;
        }
    }

    return 1;
}

/* ************************************************************************** */

/*!
 * \brief Fast modulo 8 operation.
 * \param n The value on which to apply the modulo.
 * \return The remainder of the Euclidean division by 8.
 *
 * Operation of calculating the remainder of an Euclidean division by 8.
 * Quick mod 8 operation, particulary useful for finding out how far some bits
 * are from a byte boundary.
 */
int fast_mod8(const int n)
{
    return (n & 0x7);
}

/* ************************************************************************** */

/*!
 * \brief Fast integer division by 8.
 * \param n The value to divide.
 * \return The quotient of the integer division by 8.
 */
int fast_div8(const int n)
{
    return (n >> 3);
}

/* ************************************************************************** */
/* ************************************************************************** */

short smin(short a, short b)
{
    return (((a) < (b)) ? (a) : (b));
}

short smax(short a, short b)
{
    return (((a) > (b)) ? (a) : (b));
}

int imin(int a, int b)
{
    return ((a) < (b)) ? (a) : (b);
}

int imax(int a, int b)
{
    return ((a) > (b)) ? (a) : (b);
}

int imedian(int a, int b, int c)
{
    if (a > b) // a > b
    {
        if (b > c)
            return b; // a > b > c
        else if (a > c)
            return c; // a > c > b
        else
            return a; // c > a > b
    }
    else // b > a
    {
        if (a > c)
            return a; // b > a > c
        else if (b > c)
            return c; // b > c > a
        else
            return b; // c > b > a
    }
}

double dmin(double a, double b)
{
    return ((a) < (b)) ? (a) : (b);
}

double dmax(double a, double b)
{
    return ((a) > (b)) ? (a) : (b);
}

int64_t i64min(int64_t a, int64_t b)
{
    return ((a) < (b)) ? (a) : (b);
}

int64_t i64max(int64_t a, int64_t b)
{
    return ((a) > (b)) ? (a) : (b);
}

short sabs(short x)
{
    static const short SHORT_BITS = (sizeof(short) * CHAR_BIT) - 1;
    short y = (short) (x >> SHORT_BITS);

    return (short) ((x ^ y) - y);
}

int iabs(int x)
{
    static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
    int y = x >> INT_BITS;

    return (x ^ y) - y;
}

double dabs(double x)
{
    return ((x) < 0) ? -(x) : (x);
}

int64_t i64abs(int64_t x)
{
    static const int64_t int64_t_BITS = (sizeof(int64_t) * CHAR_BIT) - 1;
    int64_t y = x >> int64_t_BITS;
    return (x ^ y) - y;
}

double dabs2(double x)
{
    return (x) * (x);
}

int iabs2(int x)
{
    return (x) * (x);
}

int64_t i64abs2(int64_t x)
{
    return (x) * (x);
}

int isign(int x)
{
    return ((x > 0) - (x < 0));
}

int isignab(int a, int b)
{
    return ((b) < 0) ? -iabs(a) : iabs(a);
}

int rshift_rnd(int x, int a)
{
    return (a > 0) ? ((x + (1 << (a-1))) >> a) : (x << (-a));
}

int rshift_rnd_sign(int x, int a)
{
    return (x > 0) ? ((x + (1 << (a-1))) >> a) : (-((iabs(x) + (1 << (a-1))) >> a));
}

unsigned int rshift_rnd_us(unsigned int x, unsigned int a)
{
    return (a > 0) ? ((x + (1 << (a-1))) >> a) : x;
}

int rshift_rnd_sf(int x, int a)
{
    return ((x + (1 << (a-1))) >> a);
}

int shift_off_sf(int x, int o, int a)
{
    return ((x + o) >> a);
}

unsigned int rshift_rnd_us_sf(unsigned int x, unsigned int a)
{
    return ((x + (1 << (a-1))) >> a);
}

int iClip1(int high, int x)
{
    x = imax(x, 0);
    x = imin(x, high);

    return x;
}

short sClip1(short high, short x)
{
    x = smax(x, 0);
    x = smin(x, high);

    return x;
}

double dClip1(double high, double x)
{
    x = dmax(x, 0);
    x = dmin(x, high);

    return x;
}

int iClip3(int low, int high, int x)
{
    x = imax(x, low);
    x = imin(x, high);

    return x;
}

short sClip3(short low, short high, short x)
{
    x = smax(x, low);
    x = smin(x, high);

    return x;
}

double dClip3(double low, double high, double x)
{
    x = dmax(x, low);
    x = dmin(x, high);

    return x;
}

/* ************************************************************************** */

int iClip1_YCbCr(const int x, const int BitDepth)
{
    return iClip3(0, (1 << BitDepth) - 1, x);
}

int iClip1_YCbCr_8(const int x)
{
    return std::min(std::max(x, 0), 255);
}

uint8_t Clip1_YCbCr_8(const int x)
{
    return std::min(std::max(x, 0), 255);
}

/* ************************************************************************** */
