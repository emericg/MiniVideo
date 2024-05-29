/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
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
 * \file      mp4_stbl.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "mp4_stbl.h"
#include "mp4_stsd.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

/* ************************************************************************** */

/*!
 * \brief Sample Table Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.1 Sample Table Box.
 *
 * Parse the sample table box, container for the time/space map.
 * This box does not contain information, only other boxes.
 */
int parse_stbl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stbl()" CLR_RESET);
    int retcode = SUCCESS;

    // Print box header
    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Sample Table");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_STSD:
                    retcode = parse_stsd(bitstr, &box_subheader, track, mp4);
                    break;

                case BOX_STTS:
                    retcode = parse_stts(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_CTTS:
                    retcode = parse_ctts(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STSS:
                    retcode = parse_stss(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STSC:
                    retcode = parse_stsc(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STSZ:
                    retcode = parse_stsz(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STZ2:
                    retcode = parse_stsz(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_STCO:
                    retcode = parse_stco(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_CO64:
                    retcode = parse_stco(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_SDTP:
                    retcode = parse_sdtp(bitstr, &box_subheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subheader);
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Decoding Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.1.2 Decoding Time to Sample Box.
 *
 * This box contains a compact version of a table that allows indexing from
 * decoding time to sample number.
 */
int parse_stts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stts()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->stts_entry_count = read_bits(bitstr, 32);
    if (track->stts_entry_count > 0)
    {
        track->stts_sample_count = (unsigned int*)calloc(track->stts_entry_count, sizeof(unsigned int));
        track->stts_sample_delta = (unsigned int*)calloc(track->stts_entry_count, sizeof(unsigned int));

        if (track->stts_sample_count == NULL || track->stts_sample_delta == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
            retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < track->stts_entry_count; i++)
            {
                track->stts_sample_count[i] = read_bits(bitstr, 32);
                track->stts_sample_delta[i] = read_bits(bitstr, 32);
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> entry_count   : %u", track->stts_entry_count);
/*
    TRACE_1(MP4, "> sample_number : [");
    for (unsigned i = 0; i < track->stts_entry_count; i++)
        for (unsigned j = 0; j < track->stts_sample_count[i]; j++)
            printf("(%u / %u),", track->stts_sample_count[i], track->stts_sample_delta[i]);
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Decoding Time to Sample");
        fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", track->stts_entry_count);
        fprintf(mp4->xml, "  <stts_sample_delta>[");
        for (unsigned i = 0; i < track->stts_entry_count; i++)
            for (unsigned j = 0; j < track->stts_sample_count[i]; j++)
                fprintf(mp4->xml, "%u, ", track->stts_sample_delta[i]);
        fprintf(mp4->xml, "]</stts_sample_delta>\n");
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Composition Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.1.3 Composition Time to Sample Box.
 *
 * This box provides the offset between decoding time and composition time.
 */
int parse_ctts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ctts()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->ctts_entry_count = read_bits(bitstr, 32);
    if (track->ctts_entry_count > 0)
    {
        track->ctts_sample_count = (uint32_t*)calloc(track->ctts_entry_count, sizeof(uint32_t));
        track->ctts_sample_offset = (int64_t*)calloc(track->ctts_entry_count, sizeof(int64_t));

        if (track->ctts_sample_count == NULL || track->ctts_sample_offset == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
            retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < track->ctts_entry_count; i++)
            {
                track->ctts_sample_count[i] = read_bits(bitstr, 32);

                if (box_header->version == 0)
                    track->ctts_sample_offset[i] = (int64_t)read_bits(bitstr, 32); // read uint
                else if (box_header->version == 1)
                    track->ctts_sample_offset[i] = (int64_t)read_bits(bitstr, 32); // read int
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> entry_count   : %u", track->ctts_entry_count);
/*
    TRACE_1(MP4, "> sample_number : [");
    for (unsigned i = 0; i < track->ctts_entry_count; i++)
        for (unsigned j = 0; j < track->ctts_sample_count[i]; j++)
            printf("(%u / %li),", track->ctts_sample_count[i], track->ctts_sample_offset[i]);
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Composition Time to Sample");
        fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", track->ctts_entry_count);
        fprintf(mp4->xml, "  <ctts_sample_delta>[");
        for (unsigned i = 0; i < track->ctts_entry_count; i++)
            for (unsigned j = 0; j < track->ctts_sample_count[i]; j++)
                fprintf(mp4->xml, "%" PRId64 ", ", track->ctts_sample_offset[i]);
        fprintf(mp4->xml, "]</ctts_sample_delta>\n");
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sync Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.2 Sync Sample Box.
 *
 * This box provides a compact marking of the random access points within the stream.
 */
int parse_stss(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stss()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->stss_entry_count = read_bits(bitstr, 32);
    if (track->stss_entry_count > 0)
    {
        track->stss_sample_number = (unsigned int*)calloc(track->stss_entry_count, sizeof(unsigned int));

        if (track->stss_sample_number == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc entry_table table!");
            retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < track->stss_entry_count; i++)
            {
                track->stss_sample_number[i] = read_bits(bitstr, 32);
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> entry_count   : %u", track->stss_entry_count);
/*
    TRACE_1(MP4, "> sample_number : [");
    for (unsigned i = 0; i < track->stss_entry_count; i++)
    {
        printf("%u, ", track->stss_sample_number[i]);
    }
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Sync Sample");
        fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", track->stss_entry_count);
        fprintf(mp4->xml, "  <stss_sample_number>[");
        for (unsigned i = 0; i < track->stss_entry_count; i++)
            fprintf(mp4->xml, "%u, ", track->stss_sample_number[i]);
        fprintf(mp4->xml, "]</stss_sample_number>\n");
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample To Chunk Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.4 Sample To Chunk Box.
 *
 * Samples within the media data are grouped into chunks. Chunks can be of different
 * sizes, and the samples within a chunk can have different sizes. This table can
 * be used to find the chunk that contains a sample, its position, and the associated
 * sample description.
 */
int parse_stsc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsc()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->stsc_entry_count = read_bits(bitstr, 32);
    if (track->stsc_entry_count > 0)
    {
        track->stsc_first_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
        track->stsc_samples_per_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
        track->stsc_sample_description_index = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));

        if (track->stsc_first_chunk == NULL ||
            track->stsc_samples_per_chunk == NULL ||
            track->stsc_sample_description_index == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc first_chunk, samples_per_chunk or sample_description_index tables!");
            retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < track->stsc_entry_count; i++)
            {
                track->stsc_first_chunk[i] = read_bits(bitstr, 32);
                track->stsc_samples_per_chunk[i] = read_bits(bitstr, 32);
                track->stsc_sample_description_index[i] = read_bits(bitstr, 32);
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
/*
    // Print box content
    TRACE_1(MP4, "> entry_count : %u", track->stsc_entry_count);

    TRACE_1(MP4, "> first_chunk : [");
    for (unsigned i = 0; i < track->stsc_entry_count; i++)
    {
        printf("%u, ", track->stsc_first_chunk[i]);
    }
    printf("]\n");

    TRACE_1(MP4, "> samples_per_chunk : [");
    for (unsigned i = 0; i < track->stsc_entry_count; i++)
    {
        printf("%u, ", track->stsc_samples_per_chunk[i]);
    }
    printf("]\n");

    TRACE_1(MP4, "> sample_description_index : [");
    for (unsigned i = 0; i < track->stsc_entry_count; i++)
    {
        printf("%u, ", track->stsc_sample_description_index[i]);
    }
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Sample To Chunk");
        fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", track->stsc_entry_count);

        fprintf(mp4->xml, "  <first_chunk>[");
        for (unsigned i = 0; i < track->stsc_entry_count; i++)
            fprintf(mp4->xml, "%u, ", track->stsc_first_chunk[i]);
        fprintf(mp4->xml, "]</first_chunk>\n");

        fprintf(mp4->xml, "  <samples_per_chunk>[");
        for (unsigned i = 0; i < track->stsc_entry_count; i++)
            fprintf(mp4->xml, "%u, ", track->stsc_samples_per_chunk[i]);
        fprintf(mp4->xml, "]</samples_per_chunk>\n");

        fprintf(mp4->xml, "  <stsc_sample_description_index>[");
        for (unsigned i = 0; i < track->stsc_entry_count; i++)
            fprintf(mp4->xml, "%u, ", track->stsc_sample_description_index[i]);
        fprintf(mp4->xml, "]</stsc_sample_description_index>\n");

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Size Boxes - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.3 Sample Size Box
 *
 * This box contains the sample count and a table giving the size in bytes of each
 * sample. This allows the media data itself to be unframed. The total number of
 * samples in the media is always indicated in the sample count.
 *
 * This box has two variants: STSZ and STZ2.
 * - This variant has a fixed size 32-bit field for representing the sample
 *   sizes; it permits defining a constant size for all samples in a track.
 * - The STZ2 variant permits smaller size fields, to save space when the sizes
 *   are varying but small.
 */
int parse_stsz(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsz()" CLR_RESET);
    int retcode = SUCCESS;
    unsigned int field_size = 32;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    if (box_header->boxtype == BOX_STSZ)
    {
        track->stsz_sample_size = read_bits(bitstr, 32);
        track->stsz_sample_count = read_bits(bitstr, 32);
    }
    else //if (box_header->type == BOX_STZ2)
    {
        /*unsigned int reserved =*/ read_bits(bitstr, 24);
        field_size = read_bits(bitstr, 8);
        track->stsz_sample_count = read_bits(bitstr, 32);
    }

    if (track->stsz_sample_size == 0)
    {
        track->stsz_entry_size = (unsigned int*)calloc(track->stsz_sample_count, sizeof(unsigned int));

        if (track->stsz_entry_size == NULL)
        {
             TRACE_ERROR(MP4, "Unable to alloc entry_size table!");
             retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < track->stsz_sample_count; i++)
            {
                track->stsz_entry_size[i] = read_bits(bitstr, field_size);
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
/*
    TRACE_1(MP4, "> sample_count : %u", track->stsz_sample_count);
    TRACE_1(MP4, "> sample_size  : %u", track->stsz_sample_size);
    if (track->stsz_sample_size == 0)
    {
        TRACE_1(MP4, "> entry_size : [");
        for (unsigned i = 0; i < track->stsz_sample_count; i++)
            printf("%u, ", track->stsz_entry_size[i]);
        printf("]\n");
    }
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Sample Size");
        fprintf(mp4->xml, "  <sample_count>%u</sample_count>\n", track->stsz_sample_count);
        fprintf(mp4->xml, "  <sample_size>%u</sample_size>\n", track->stsz_sample_size);
        fprintf(mp4->xml, "  <entry_size>[");
        if (track->stsz_sample_size == 0)
            for (unsigned i = 0; i < track->stsz_sample_count; i++)
                fprintf(mp4->xml, "%u, ", track->stsz_entry_size[i]);
        fprintf(mp4->xml, "]</entry_size>\n");
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Chunk Offset Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.5 Chunk Offset Box.
 *
 * The chunk offset table gives the index of each chunk into the containing file.
 * There are two variants, permitting the use of 32-bit (STCO variant) or 64-bit
 * offsets (CO64 variant).
 */
int parse_stco(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stco()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    track->stco_entry_count = read_bits(bitstr, 32);
    if (track->stco_entry_count)
    {
        track->stco_chunk_offset = (int64_t*)calloc(track->stco_entry_count, sizeof(int64_t));

        if (track->stco_chunk_offset == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc chunk_offset table!");
            retcode = FAILURE;
        }
        else
        {
            if (box_header->boxtype == BOX_CO64)
            {
                for (unsigned i = 0; i < track->stco_entry_count; i++)
                {
                    track->stco_chunk_offset[i] = (int64_t)read_bits_64(bitstr, 64);
                }
            }
            else //if (box_header->type == BOX_STCO)
            {
                for (unsigned i = 0; i < track->stco_entry_count; i++)
                {
                    track->stco_chunk_offset[i] = (int64_t)read_bits(bitstr, 32);
                }
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
/*
    TRACE_1(MP4, "> entry_count  : %u", track->stco_entry_count);

    TRACE_1(MP4, "> chunk_offset : [");
    for (unsigned i = 0; i < track->stco_entry_count; i++)
    {
        printf("%li, ", track->stco_chunk_offset[i]);
    }
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Chunk Offset");
        fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", track->stco_entry_count);
        fprintf(mp4->xml, "  <chunk_offset>[");
        for (unsigned i = 0; i < track->stco_entry_count; i++)
            fprintf(mp4->xml, "%" PRId64 ", ", track->stco_chunk_offset[i]);
        fprintf(mp4->xml, "]</chunk_offset>\n");
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Independent and Disposable Samples - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.6.4 Independent and Disposable Samples Box.
 */
int parse_sdtp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_sdtp()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned sample_count = track->stsz_sample_count;
    if (sample_count == 0)
        sample_count = track->ctts_entry_count;

    // Parse box content
    if (sample_count > 0)
    {
        track->sdtp_is_leading = (uint8_t*)calloc(sample_count, sizeof(uint8_t));
        track->sdtp_sample_depends_on = (uint8_t*)calloc(sample_count, sizeof(uint8_t));
        track->sdtp_sample_is_depended_on = (uint8_t*)calloc(sample_count, sizeof(uint8_t));
        track->sdtp_sample_has_redundancy = (uint8_t*)calloc(sample_count, sizeof(uint8_t));

        if (track->sdtp_is_leading == NULL || track->sdtp_sample_depends_on == NULL ||
            track->sdtp_sample_is_depended_on == NULL || track->sdtp_sample_has_redundancy == NULL)
        {
            TRACE_ERROR(MP4, "Unable to alloc sdtp tables!");
            retcode = FAILURE;
        }
        else
        {
            for (unsigned i = 0; i < sample_count; i++)
            {
                track->sdtp_is_leading[i] = (int8_t)read_bits(bitstr, 2);
                track->sdtp_sample_depends_on[i] = (int8_t)read_bits(bitstr, 2);
                track->sdtp_sample_is_depended_on[i] = (int8_t)read_bits(bitstr, 2);
                track->sdtp_sample_has_redundancy[i] = (int8_t)read_bits(bitstr, 2);
            }
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
/*
    TRACE_1(MP4, "> sdtp_is_leading : [");
    for (unsigned i = 0; i < sample_count; i++)
    {
        printf("%u, ", track->sdtp_is_leading[i]);
    }
    printf("]\n");
    TRACE_1(MP4, "> sdtp_is_leading : [");
    for (unsigned i = 0; i < sample_count; i++)
    {
        printf("%u, ", track->sdtp_is_leading[i]);
    }
    printf("]\n");
    TRACE_1(MP4, "> sdtp_is_leading : [");
    for (unsigned i = 0; i < sample_count; i++)
    {
        printf("%u, ", track->sdtp_is_leading[i]);
    }
    printf("]\n");
    TRACE_1(MP4, "> sdtp_is_leading : [");
    for (unsigned i = 0; i < sample_count; i++)
    {
        printf("%u, ", track->sdtp_is_leading[i]);
    }
    printf("]\n");
*/
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Independent and Disposable Samples");

        fprintf(mp4->xml, "  <sdtp_is_leading>[");
        for (unsigned i = 0; i < sample_count; i++)
            fprintf(mp4->xml, "%u, ", track->sdtp_is_leading[i]);
        fprintf(mp4->xml, "]</sdtp_is_leading>\n");

        fprintf(mp4->xml, "  <sdtp_sample_depends_on>[");
        for (unsigned i = 0; i < sample_count; i++)
            fprintf(mp4->xml, "%u, ", track->sdtp_sample_depends_on[i]);
        fprintf(mp4->xml, "]</sdtp_sample_depends_on>\n");

        fprintf(mp4->xml, "  <sdtp_sample_is_depended_on>[");
        for (unsigned i = 0; i < sample_count; i++)
            fprintf(mp4->xml, "%u, ", track->sdtp_sample_is_depended_on[i]);
        fprintf(mp4->xml, "]</sdtp_sample_is_depended_on>\n");

        fprintf(mp4->xml, "  <sdtp_sample_has_redundancy>[");
        for (unsigned i = 0; i < sample_count; i++)
            fprintf(mp4->xml, "%u, ", track->sdtp_sample_has_redundancy[i]);
        fprintf(mp4->xml, "]</sdtp_sample_has_redundancy>\n");

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */
