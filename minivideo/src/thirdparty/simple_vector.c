/*!
 * \file      simple_vector.c
 * \license   MIT License
 *
 * Original project:
 * - Emil Hernvall (https://gist.github.com/EmilHernvall/953968)

 * Copyright (c) 2016 Emil Hernvall <emil@znaptag.com>
 * Copyright (c) 2017 Emeric Grange <emeric.grange@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "simple_vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void vector_init(vector *v)
{
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

int vector_count(vector *v)
{
    return v->count;
}

void vector_add(vector *v, void *e)
{
    if (v->size == 0)
    {
        v->size = 16;
        v->data = malloc(sizeof(void*) * v->size);
        memset(v->data, '\0', sizeof(void*) * v->size);
    }

    if (v->size == v->count)
    {
        v->size *= 2;
        v->data = realloc(v->data, sizeof(void*) * v->size);
    }

    v->data[v->count] = e;
    v->count++;
}

void *vector_get(vector *v, int index)
{
    if (index >= v->count)
        return NULL;

    return v->data[index];
}

void vector_set(vector *v, int index, void *e)
{
    if (index >= v->count)
        return;

    v->data[index] = e;
}

void vector_delete(vector *v, int index)
{
    if (index >= v->count)
        return;

    for (int i = index+1, j = index; i < v->count; i++)
    {
        v->data[j] = v->data[i];
        j++;
    }

    v->count--;
}

void vector_free(vector *v)
{
    free(v->data);
}
