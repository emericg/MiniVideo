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
 * \file      minivideo_uuid.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "minivideo_uuid.h"
#include "minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

/* ************************************************************************** */

char *getGuidString(const uint8_t uuid_in[16], char guid_out[39])
{
    if (guid_out)
    {
        sprintf(guid_out, "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                uuid_in[0], uuid_in[1], uuid_in[2], uuid_in[3],
                uuid_in[4], uuid_in[5],
                uuid_in[6], uuid_in[7],
                uuid_in[8], uuid_in[9],
                uuid_in[10], uuid_in[11], uuid_in[12], uuid_in[13], uuid_in[14], uuid_in[15]);

        return guid_out;
    }

    return NULL;
}

char *getUrnString(const uint8_t uuid_in[16], char urn_out[45])
{
    if (urn_out)
    {
        sprintf(urn_out, "urn:uuid:%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                uuid_in[0], uuid_in[1], uuid_in[2], uuid_in[3],
                uuid_in[4], uuid_in[5],
                uuid_in[6], uuid_in[7],
                uuid_in[8], uuid_in[9],
                uuid_in[10], uuid_in[11], uuid_in[12], uuid_in[13], uuid_in[14], uuid_in[15]);

        return urn_out;
    }

    return NULL;
}

/* ************************************************************************** */

void read_uuid_be(Bitstream_t *bitstr, uint8_t uuid[16])
{
    // Read UUID (big endian)
    for (int i = 0; i < 16; i++)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
}

void read_uuid_le(Bitstream_t *bitstr, uint8_t uuid[16])
{
    // Read UUID (mixeg big/little endian)
    for (int i = 3; i > -1; i--)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 5; i > 3; i--)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 7; i > 5; i--)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 8; i < 10; i++)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
    for (int i = 10; i < 16; i++)
        uuid[i] = (uint8_t)read_bits(bitstr, 8);
}

/* ************************************************************************** */
