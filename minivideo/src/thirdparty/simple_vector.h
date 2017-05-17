/*!
 * \file      simple_vector.h
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

#ifndef SIMPLE_VECTOR_H
#define SIMPLE_VECTOR_H
/* ************************************************************************** */

/*!
 * The simple_vector_t structure.
 */
typedef struct simple_vector_t
{
    void **data;    //!< Array of void pointers
    int size;       //!< Number of array elements currently allocated
    int count;      //!< Number of elements currently in use

} vector;

/* ************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*!
 * \brief vector_init
 * \param v
 */
void vector_init(vector *v);

/*!
 * \brief vector_count
 * \param v
 * \return
 */
int vector_count(vector *v);

/*!
 * \brief vector_add
 * \param v
 * \param e
 */
void vector_add(vector *v, void *e);

/*!
 * \brief vector_get
 * \param v
 * \param index
 * \return
 */
void *vector_get(vector *v, int index);

/*!
 * \brief vector_set
 * \param v
 * \param index
 * \param e
 */
void vector_set(vector *v, int index, void *e);

/*!
 * \brief vector_delete
 * \param v
 * \param index
 */
void vector_delete(vector *v, int index);

/*!
 * \brief vector_free
 * \param v
 */
void vector_free(vector *v);

#ifdef __cplusplus
}
#endif // __cplusplus

/* ************************************************************************** */
#endif // SIMPLE_VECTOR_H
