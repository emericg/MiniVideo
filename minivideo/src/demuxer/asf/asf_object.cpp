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
 * \file      asf_object.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "asf_object.h"
#include "asf_struct.h"

#include "../xml_mapper.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cmath>
#include <climits>
#include <cinttypes>

/* ************************************************************************** */

#define WINDOWS_TICK 10000000LL
#define SEC_TO_UNIX_EPOCH 11644473600LL

/*!
 * \param windowsTicks: Seconds since January 1, 1904
 * \return Unix time
 *
 * For infos on "Windows ticks":
 * - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284(v=vs.85).aspx
 * - https://en.wikipedia.org/wiki/Epoch_(reference_date)
 */
uint64_t WindowsTickToUnixSeconds(int64_t windowsTicks)
{
     return (uint64_t)(windowsTicks / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
}

/* ************************************************************************** */

int read_asf_guid(Bitstream_t *bitstr, uint8_t guid[16],
                  FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_guid()");
    int status = SUCCESS;

    read_uuid_le(bitstr, guid);

    if (name)
    {
        char guid_str[39];
        getGuidString(guid, guid_str);
        TRACE_1(ASF, "* %s  = %s", name, guid_str);
        if (xml)
        {
            fprintf(xml, "  <%s>%s</%s>\n", name, guid_str, name);
        }
    }

    return status;
}

int8_t read_asf_int8(Bitstream_t *bitstr,
                     FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_int8()");
    int8_t value = (uint8_t)read_bits(bitstr, 8);

    if (name)
    {
        TRACE_1(ASF, "* %s  = %i", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%i</%s>\n", name, value, name);
        }
    }

    return value;
}

int16_t read_asf_int16(Bitstream_t *bitstr,
                       FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_int16()");
    int16_t value = (int16_t)endian_flip_16(read_bits(bitstr, 16));

    if (name)
    {
        TRACE_1(ASF, "* %s  = %i", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%i</%s>\n", name, value, name);
        }
    }

    return value;
}

int32_t read_asf_int32(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_int32()");
    int32_t value = (int32_t)endian_flip_32(read_bits(bitstr, 32));

    if (name)
    {
        TRACE_1(ASF, "* %s  = %i", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%i</%s>\n", name, value, name);
        }
    }

    return value;
}

int64_t read_asf_int64(Bitstream_t *bitstr, FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_int64()");
    int64_t value = (int64_t)endian_flip_64(read_bits_64(bitstr, 64));

    if (name)
    {
        TRACE_1(ASF, "* %s  = %" PRId64 "", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;
}

int64_t read_asf_int(Bitstream_t *bitstr, const int n,
                     FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_int32()");
    int64_t value = (int64_t)read_bits(bitstr, n);

    if (n > 32)
    {
        value = (int64_t)endian_flip_64(value);
    }
    else if (n > 16)
    {
        value = (int64_t)endian_flip_32(value);
    }
    else if (n > 8)
    {
        value = (int64_t)endian_flip_16(value);
    }

    if (name)
    {
        TRACE_1(ASF, "* %s  = %" PRId64 "", name, value);
        if (xml)
        {
            fprintf(xml, "  <%s>%" PRId64 "</%s>\n", name, value, name);
        }
    }

    return value;
}

uint8_t *read_asf_binary(Bitstream_t *bitstr, const int sizeBytes,
                         FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_binary(%i bytes)", sizeBytes);

    uint8_t *data = NULL;

    if (sizeBytes > 0)
    {
        data = (uint8_t *)malloc(sizeBytes+1);
        if (data)
        {
            for (int i = 0; i < sizeBytes; i++)
                data[i] = read_bits(bitstr, 8);
            data[sizeBytes] = '\0';

            if (name)
            {
#if ENABLE_DEBUG
                TRACE_1(ASF, "* %s  = 0x", name);

                if (sizeBytes > 1023)
                    TRACE_1(ASF, "* %s  = (first 1024B) 0x", name);
                else
                    TRACE_1(ASF, "* %s  = 0x", name);
                for (int i = 0; i < sizeBytes && i < 1024; i++)
                    printf("%02X", data[i]);
#endif // ENABLE_DEBUG

                if (xml)
                {
                    if (sizeBytes > 1023)
                        fprintf(xml, "  <%s>(first 1024B) 0x", name);
                    else
                        fprintf(xml, "  <%s>0x", name);
                    for (int i = 0; i < sizeBytes && i < 1024; i++)
                        fprintf(xml, "%02X", data[i]);
                    fprintf(xml, "</%s>\n", name);
                }
            }
        }
    }

    return data;
}

char *read_asf_string_ascii(Bitstream_t *bitstr, const int sizeChar,
                            FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_string_ascii(%i ASCII char)", sizeChar);

    char *string = NULL;

    if (sizeChar > 0)
    {
        string = (char*)malloc(sizeChar+1);
        if (string)
        {
            for (int i = 0; i < sizeChar; i++)
            {
                string[i] = read_bits(bitstr, 8);
            }
            string[sizeChar] = '\0';

            if (name)
            {
#if ENABLE_DEBUG
                TRACE_1(ASF, "* %s  = '%s'", name, string);
#endif // ENABLE_DEBUG

                if (xml)
                {
                    fprintf(xml, "  <%s>%s</%s>\n", name, string,name);
                }
            }
        }
    }

    return string;
}

char *read_asf_string_utf16(Bitstream_t *bitstr, const int sizeChar,
                            FILE *xml, const char *name)
{
    TRACE_2(ASF, "read_asf_string_utf16(%i UTF16 char)", sizeChar);

    char *string = NULL;

    if (sizeChar > 0)
    {
        //int sizeBytes = sizeChar * 2;

        string = (char*)malloc(sizeChar+1);
        if (string)
        {
            for (int i = 0; i < sizeChar; i++)
            {
                string[i] = read_bits(bitstr, 8);
                skip_bits(bitstr, 8);
            }
            string[sizeChar] = '\0';

            if (name)
            {
#if ENABLE_DEBUG
                TRACE_1(ASF, "* %s  = '%s'", name, string);
#endif // ENABLE_DEBUG

                if (xml)
                {
                    fprintf(xml, "  <%s>%s</%s>\n", name, string,name);
                }
            }
        }
    }

    return string;
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_asf_object(Bitstream_t *bitstr, AsfObject_t *asf_object)
{
    TRACE_INFO(ASF, "parse_asf_object()");
    int retcode = SUCCESS;

    if (asf_object == NULL)
    {
        TRACE_ERROR(ASF, "Invalid AsfObject_t structure!");
        retcode = FAILURE;
    }
    else
    {
        // Set object offset
        asf_object->offset_start = bitstream_get_absolute_byte_offset(bitstr);

        // Read object GUID
        for (int i = 3; i > -1; i--)
            asf_object->guid[i] = (uint8_t)read_bits(bitstr, 8);
        for (int i = 5; i > 3; i--)
            asf_object->guid[i] = (uint8_t)read_bits(bitstr, 8);
        for (int i = 7; i > 5; i--)
            asf_object->guid[i] = (uint8_t)read_bits(bitstr, 8);
        for (int i = 8; i < 10; i++)
            asf_object->guid[i] = (uint8_t)read_bits(bitstr, 8);
        for (int i = 10; i < 16; i++)
            asf_object->guid[i] = (uint8_t)read_bits(bitstr, 8);

        // Read object size
        asf_object->size = (int64_t)endian_flip_64(read_bits_64(bitstr, 64));

        // Set end offset
        asf_object->offset_end = asf_object->offset_start + asf_object->size;
    }

    return retcode;
}

void print_asf_object(AsfObject_t *asf_object)
{
#if ENABLE_DEBUG
    if (asf_object == NULL)
    {
        TRACE_ERROR(RIF, "Invalid AsfObject_t structure!");
    }
    else
    {
        TRACE_2(ASF, "* start offset  : %lli", asf_object->offset_start);
        TRACE_2(ASF, "* end offset    : %lli", asf_object->offset_end);

        // Print Box header
        TRACE_2(ASF, "* object guid   : {%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                asf_object->guid[0], asf_object->guid[1], asf_object->guid[2], asf_object->guid[3],
                asf_object->guid[4], asf_object->guid[5],
                asf_object->guid[6], asf_object->guid[7],
                asf_object->guid[8], asf_object->guid[9],
                asf_object->guid[10], asf_object->guid[11], asf_object->guid[12], asf_object->guid[13], asf_object->guid[14], asf_object->guid[15]);
        TRACE_2(ASF, "* object size   : %lli", asf_object->size);
    }
#endif // ENABLE_DEBUG
}

void write_asf_object(AsfObject_t *asf_object, FILE *xml,
                      const char *title, const char *additional)
{
    if (xml)
    {
        if (asf_object == NULL)
        {
            TRACE_ERROR(ASF, "Invalid AsfObject_t structure!");
        }
        else
        {
            char boxtitle[64];
            char boxtypeadd[16];

            if (title != nullptr)
                snprintf(boxtitle, 63, "tt=\"%s\" ", title);
            else
                boxtitle[0] = '\0';

            if (additional != nullptr)
                snprintf(boxtypeadd, 15, "add=\"%s\" ", additional);
            else
                boxtypeadd[0] = '\0';

            fprintf(xml, "  <a %s%sguid=\"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\" tp=\"ASF obj\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
                boxtitle, boxtypeadd,
                asf_object->guid[0], asf_object->guid[1], asf_object->guid[2], asf_object->guid[3],
                asf_object->guid[4], asf_object->guid[5],
                asf_object->guid[6], asf_object->guid[7],
                asf_object->guid[8], asf_object->guid[9],
                asf_object->guid[10], asf_object->guid[11], asf_object->guid[12], asf_object->guid[13], asf_object->guid[14], asf_object->guid[15],
                asf_object->offset_start,
                asf_object->size);
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

int parse_unknown_object(Bitstream_t *bitstr, AsfObject_t *asf_object, FILE *xml)
{
#if ENABLE_DEBUG
    TRACE_WARNING(ASF, BLD_GREEN "parse_unknown_object({%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X} @ %lli; size is %llu)" CLR_RESET,
                  asf_object->guid[0], asf_object->guid[1], asf_object->guid[2], asf_object->guid[3],
                  asf_object->guid[4], asf_object->guid[5],
                  asf_object->guid[6], asf_object->guid[7],
                  asf_object->guid[8], asf_object->guid[9],
                  asf_object->guid[10], asf_object->guid[11], asf_object->guid[12],
                  asf_object->guid[13], asf_object->guid[14], asf_object->guid[15],
                  asf_object->offset_start, asf_object->offset_end - asf_object->offset_start);

    print_asf_object(asf_object);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (xml)
    {
        write_asf_object(asf_object, xml, "Unknown");
        fprintf(xml, "  </a>\n");
    }

    return SUCCESS;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Jumpy protect your parsing - ASF edition.
 * \param bitstr: Our bitstream reader.
 * \param parent: The box containing the current box we're in.
 * \param current: The current box we're in.
 * \return SUCCESS or FAILURE if the jump could not be done.
 *
 * 'Jumpy' is in charge of checking your position into the stream after your
 * parser finish parsing a box / list / chunk / element, never leaving you
 * stranded  in the middle of nowhere with no easy way to get back on track.
 * It will check available informations to known if the current element has been
 * fully parsed, and if not perform a jump (or even a rewind) to the next known
 * element.
 */
int jumpy_asf(Bitstream_t *bitstr, AsfObject_t *parent, AsfObject_t *current)
{
    int retcode = SUCCESS;

    // Done as a precaution, because the parsing of some boxes (like ESDS...)
    // can leave us in the middle of a byte and that will never be caught by
    // offset checks (cause they works on the assumption that we are byte aligned)
    bitstream_force_alignment(bitstr);

    // Check if we need a jump
    int64_t current_pos = bitstream_get_absolute_byte_offset(bitstr);
    if (current_pos != current->offset_end)
    {
        int64_t file_size = bitstream_get_full_size(bitstr);
        int64_t offset_end = current->offset_end;

        // Check offset_end
        if (parent && parent->offset_end < file_size) // current box has valid parent
        {
            // Validate offset_end against parent's (parent win)
            if (offset_end > parent->offset_end)
                offset_end = parent->offset_end;
        }
        else // current box has no parent (or parent with invalid offset_end)
        {
            // Validate offset_end against file's (file win)
            if (offset_end > file_size)
                offset_end = file_size;
        }

        // If the offset_end is past the last byte of the file, we do not need to jump
        // The parser will pick that fact and finish up...
        if (offset_end >= file_size)
        {
            TRACE_WARNING(ASF, "JUMPY > going EOF (%lli)", file_size);
            bitstr->bitstream_offset = file_size;
            return SUCCESS;
        }

        TRACE_WARNING(ASF, "JUMPY > going from %lli to %lli", current_pos, offset_end);

        // Now, do we need to go forward or backward to reach our goal?
        // Then, can we move in our current buffer or do we need to reload a new one?
        if (current_pos < offset_end)
        {
            int64_t jump = offset_end - current_pos;

            if (jump < (UINT_MAX/8))
                retcode = skip_bits(bitstr, (unsigned int)(jump*8));
            else
                retcode = bitstream_goto_offset(bitstr, offset_end);
        }
        else
        {
            int64_t rewind = current_pos - offset_end;

            if (rewind > 0)
            {
                if (rewind > (UINT_MAX/8))
                    retcode = rewind_bits(bitstr, (unsigned int)(rewind*8));
                else
                    retcode = bitstream_goto_offset(bitstr, offset_end);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
