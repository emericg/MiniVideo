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
 * \file      riff_common.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "riff.h"
#include "riff_struct.h"
#include "../../utils.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minivideo_fourcc.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

/* ************************************************************************** */

/*!
 * \brief Parse JUNK chunk.
 */
int parse_JUNK(Bitstream_t *bitstr, RiffChunk_t *JUNK_header, FILE *xml)
{
    int retcode = SUCCESS;

    if (JUNK_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid JUNK_header structure!");
        retcode = FAILURE;
    }
    else
    {
        TRACE_INFO(RIF, BLD_GREEN "parse_JUNK()" CLR_RESET);

#if ENABLE_DEBUG
        print_chunk_header(JUNK_header);
#endif
        // xmlMapper
        if (xml)
        {
            write_chunk_header(JUNK_header, xml);
            fprintf(xml, "  <title>JUNK (filler datas)</title>\n");
            fprintf(xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse PAD chunk.
 */
int parse_PAD(Bitstream_t *bitstr, RiffChunk_t *PAD_header, FILE *xml)
{
    int retcode = SUCCESS;

    if (PAD_header == NULL)
    {
        TRACE_ERROR(RIF, "Invalid PAD_header structure!");
        retcode = FAILURE;
    }
    else
    {
        TRACE_INFO(RIF, BLD_GREEN "parse_PAD()" CLR_RESET);

#if ENABLE_DEBUG
        print_chunk_header(PAD_header);
#endif
        // xmlMapper
        if (xml)
        {
            write_chunk_header(PAD_header, xml);
            fprintf(xml, "  <title>PAD (padding datas)</title>\n");
            fprintf(xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse CSET chunk.
 */
int parse_CSET(Bitstream_t *bitstr, RiffChunk_t *CSET_header, CsetChunk_t *cset, FILE *xml)
{
    int retcode = SUCCESS;

    if (CSET_header == NULL || CSET_header->dwSize < 16)
    {
        TRACE_ERROR(RIF, "Invalid CSET_header structure!");
        retcode = FAILURE;
    }
    else
    {
        TRACE_INFO(RIF, BLD_GREEN "parse_CSET()" CLR_RESET);

        cset->wCodePage = read_bits(bitstr, 32);
        cset->wCountryCode = read_bits(bitstr, 32);
        cset->wLanguage = read_bits(bitstr, 32);
        cset->wDialect = read_bits(bitstr, 32);

#if ENABLE_DEBUG
        print_chunk_header(CSET_header);
        TRACE_1(WAV, "> wCodePage       : '%s'", cset->wCodePage);
        TRACE_1(WAV, "> wCountryCode    : '%s'", cset->wCountryCode);
        TRACE_1(WAV, "> wLanguage       : '%s'", cset->wLanguage);
        TRACE_1(WAV, "> wDialect        : '%s'", cset->wDialect);
#endif
        // xmlMapper
        if (xml)
        {
            write_chunk_header(CSET_header, xml);
            fprintf(xml, "  <title>Character Set</title>\n");
            fprintf(xml, "  <wCodePage>%u</wCodePage>\n", cset->wCodePage);
            fprintf(xml, "  <wCountryCode>%u</wCountryCode>\n", cset->wCountryCode);
            fprintf(xml, "  <wLanguage>%u</wLanguage>\n", cset->wLanguage);
            fprintf(xml, "  <wDialect>%u</wDialect>\n", cset->wDialect);
            fprintf(xml, "  </a>\n");
        }
    }

    return retcode;
}

/* ************************************************************************** */
