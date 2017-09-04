/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
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
 * \file      minivideo_guid.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "minivideo_guid.h"
#include "minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>

/* ************************************************************************** */

char *getGuidString(const uint8_t guid_in[16], char *guid_out)
{
    if (guid_out)
    {
        sprintf(guid_out, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                guid_in[0], guid_in[1], guid_in[2], guid_in[3],
                guid_in[4], guid_in[5],
                guid_in[6], guid_in[7],
                guid_in[8], guid_in[9],
                guid_in[10], guid_in[11], guid_in[12], guid_in[13], guid_in[14], guid_in[15]);

        return guid_out;
    }

    return NULL;
}

void read_guid_be(Bitstream_t *bitstr, uint8_t guid[16])
{
    // Read GUID (be)
    for (int i = 0; i < 16; i++)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
}

void read_guid_le(Bitstream_t *bitstr, uint8_t guid[16])
{
    // Read GUID (le)
    for (int i = 3; i > -1; i--)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 5; i > 3; i--)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 7; i > 5; i--)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 8; i < 10; i++)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 10; i < 16; i++)
        guid[i] = (uint8_t)read_bits(bitstr, 8);
}

/* ************************************************************************** */
