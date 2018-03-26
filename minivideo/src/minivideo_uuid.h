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
 * \file      minivideo_uuid.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

#ifndef MINIVIDEO_UUID_H
#define MINIVIDEO_UUID_H

// minivideo headers
#include "bitstream.h"

/* ************************************************************************** */

/*!
 * \brief Get a printable GUID string
 * \param uuid_in[in]: A 16 characters UUID.
 * \param guid_out[in,out]: A 39 character C string.
 * \return A pointer to the provided guid_out[39] so this function can be used directly inside a printf().
 *
 * A Microsoft "GUIDs" prints as follows:
 * {00112233-4455-6677-8899-aabbccddeeff}
 */
minivideo_EXPORT char *getGuidString(const uint8_t uuid_in[16], char guid_out[39]);

/*!
 * \brief Get a printable URN string
 * \param uuid_in[in]: A 16 characters UUID.
 * \param guid_out[in,out]: A 45 character C string.
 * \return A pointer to the provided urn_out[45] so this function can be used directly inside a printf().
 *
 * RFC 4122 defines a Uniform Resource Name (URN) namespace for UUIDs.
 * A UUID presented as a URN prints as follows:
 * urn:uuid:123e4567-e89b-12d3-a456-426655440000
 */
minivideo_EXPORT char *getUrnString(const uint8_t uuid_in[16], char urn_out[45]);

/*!
 * \brief Read UUID with big endian order (use for everything NOT from Microsoft).
 * \param bitstr[in]: Our bitstream reader.
 * \param uuid[in,out]: A 16 bytes UUID.
 *
 * A universally unique identifier (UUID) is a 128-bit number used to identify information in computer systems.
 * About UUID/GUID: https://en.wikipedia.org/wiki/Universally_unique_identifier.
 *
 * Encoded bytes will look like this: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff.
 */
void read_uuid_be(Bitstream_t *bitstr, uint8_t uuid[16]);

/*!
 * \brief Read UUID stored as mixed little/big endianness (use for everything from Microsoft).
 * \param bitstr[in]: Our bitstream reader.
 * \param uuid[in,out]: A 16 bytes UUID.
 *
 * A universally unique identifier (UUID) is a 128-bit number used to identify information in computer systems.
 * About UUID/GUID: https://en.wikipedia.org/wiki/Universally_unique_identifier.
 *
 * Use a mixed-endian format, whereby the first three components of the UUID are
 * little-endian, and the last two are big-endian.
 * Encoded bytes will look like this: 33 22 11 00 55 44 77 66 88 99 aa bb cc dd ee ff.
 */
void read_uuid_le(Bitstream_t *bitstr, uint8_t uuid[16]);

/* ************************************************************************** */
#endif // MINIVIDEO_UUID_H
