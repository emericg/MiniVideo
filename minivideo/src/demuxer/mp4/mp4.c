/*!
 * COPYRIGHT (C) 2011 Emeric Grange - All Rights Reserved
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
 * \file      mp4.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// minivideo headers
#include "../../minitraces.h"
#include "../../typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"

#include "mp4.h"
#include "mp4_struct.h"

/* ************************************************************************** */

static int parse_mvhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4);

static int parse_trak(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4);
static int parse_tkhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
static int parse_mdhd(Bitstream_t *bitstr, Mp4Box_t *box_header);
static int parse_mdia(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
        static int parse_hdlr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
        static int parse_minf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
            static int parse_stbl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stsd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                    static int parse_avcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                    static int parse_btrt(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_ctts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stss(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stsc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stsz(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);
                static int parse_stco(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track);

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Convert a videoTrack_t structure into a bitstreamMap_t.
 * \param *video A pointer to a VideoFile_t structure, containing every informations available about the current video file.
 * \param *track A pointer to the videoTrack_t structure we want to extract data from.
 * \param idr_only Set to true if we only want to extract IDR samples.
 *
 * - Use STSZ box content to get back all samples.
 * - Use STSS box content to get back IDR samples only.
 */
static bool convertTrack(VideoFile_t *video, Mp4_t *mp4, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "convertTrack()\n" CLR_RESET);
    bool retcode = SUCCESS;
    BitstreamMap_t *map = NULL;

    if (video == NULL || track == NULL)
    {
        TRACE_ERROR(MP4, "Cannot access audio or video tracks from the MP4 parser!\n");
        retcode = FAILURE;
    }

    // Select and init a bitstream map (A or V)
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS)
    {
        if (track->handlerType == HANDLER_AUDIO)
        {
            retcode = init_bitstream_map(&video->tracks_audio[video->tracks_audio_count], track->stsz_sample_count);
            if (retcode == SUCCESS)
            {
                map = video->tracks_audio[video->tracks_audio_count];
                video->tracks_audio_count++;
            }
        }
        else if (track->handlerType == HANDLER_VIDEO)
        {
            retcode = init_bitstream_map(&video->tracks_video[video->tracks_video_count], track->stsz_sample_count + track->sps_count + track->pps_count);
            if (retcode == SUCCESS)
            {
                map = video->tracks_video[video->tracks_video_count];
                video->tracks_video_count++;
            }
        }
        else
        {
            TRACE_ERROR(MP4, "We can only build bitstream map for audio and video tracks!\n");
            retcode = FAILURE;
        }
    }

    // Build bitstream map
    ////////////////////////////////////////////////////////////////////////////

    if (retcode == SUCCESS)
    {
        unsigned int i = 0, j = 0;

        map->stream_level = stream_level_ES;
        map->stream_codec = track->codec;

        map->duration = (float)track->duration / mp4->timescale * 1000;
        map->creation_time = (float)track->creation_time / mp4->timescale * 1000;
        map->modification_time = (float)track->modification_time / mp4->timescale * 1000;

        map->bitrate = track->bitrate_avg;
        //track->bitrate_max

        map->sample_alignment = true;
        map->sample_count = track->stsz_sample_count + track->sps_count + track->pps_count;

        if (track->handlerType == HANDLER_AUDIO)
        {
            map->stream_type = stream_AUDIO;
            map->sampling_rate = track->sample_rate;
            map->channel_count = track->channel_count;
        }
        else if (track->handlerType == HANDLER_VIDEO)
        {
            map->stream_type = stream_VIDEO;
            map->width = track->width;
            map->height = track->height;
            map->color_depth = track->color_depth;
            map->sample_count_idr = track->stss_entry_count;

            if (track->codec == CODEC_H264)
            {
                // Set SPS
                for (i = 0; i < track->sps_count; i++)
                {
                    map->sample_type[i] = sample_VIDEO_PARAM;
                    map->sample_offset[i] = track->sps_sample_offset[i];
                    map->sample_size[i] = track->sps_sample_size[i];
                    map->sample_pts[i] = -1;
                    map->sample_dts[i] = -1;
                }
                // Set PPS
                for (i = 0; i < track->pps_count; i++)
                {
                    map->sample_type[i] = sample_VIDEO_PARAM;
                    map->sample_offset[i + track->sps_count] = track->pps_sample_offset[i];
                    map->sample_size[i + track->sps_count] = track->pps_sample_size[i];
                    map->sample_pts[i] = -1;
                    map->sample_dts[i] = -1;
                }
            }
        }

        unsigned int chunk = 0;
        unsigned int posinchunk = 0;

        // Set samples into the bitstream map
        ////////////////////////////////////////////////////////////////
        for (i = 0; i < track->stsz_sample_count; i++)
        {
            int sid = i + track->sps_count + track->pps_count; // Sample id

            unsigned int tempSample = 0;
            int range = 0, rangestart = 0, rangestop = 0, spc = 0;
            unsigned int e = 0; // current entry on the Sample to Chunk Box (stsc) // range e-1 e
            int f = 0; // chunk on a chunk range

            // Find the appropriate chunk for a sample i
            if (track->stsc_entry_count > 1)
            {
                chunk = 0;
                posinchunk = 0;

                // Loop on each "chunk ranges"
                for (e = 0; e < track->stsc_entry_count; e++)
                {
                    { // WIP
                        rangestart = track->stsc_first_chunk[e] - 1;

                        if (e == (track->stsc_entry_count - 1))
                            rangestop = track->stco_entry_count;//track->stsc_first_chunk[track->stsc_entry_count-1] - 1;
                        else
                            rangestop = track->stsc_first_chunk[e+1] - 1;

                        range = rangestop - rangestart;
                        spc = track->stsc_samples_per_chunk[e];

                        TRACE_3(MP4, " * Using range [%i-%i]  (%i chunk in this range)\n", rangestart, rangestop, range);
                        TRACE_3(MP4, " * Sample per chunk: %i\n", spc);
                        TRACE_3(MP4, " * number of chunk: %i\n", track->stco_entry_count);
                    }

                    TRACE_3(MP4, "[%i] chunk %u / spc %u \t (range %i)\n", e+1, chunk+1, track->stsc_samples_per_chunk[e], range);

                    // loop on chunks inside a chunk range
                    for (f = 0; f < range; f++)
                    {
                        // loop on samples inside a chunk
                        for (posinchunk = 0; posinchunk < track->stsc_samples_per_chunk[e]; posinchunk++)
                        {
                            if (tempSample == i)
                            {
                                //TRACE_2(MP4, "sample %i \t (%u/%u) (chunk / posinchunk)\n", i, chunk, posinchunk);
                                goto chunk_found;
                            }
                            else
                            {
                                tempSample++;
                            }
                        }

                        chunk++;
                    }
                }
            }
            else
            {
                chunk = (unsigned int)(i / track->stsc_samples_per_chunk[0]);
                posinchunk = (unsigned int)(i % track->stsc_samples_per_chunk[0]);
            }

            chunk_found:

            // Set sample type
            if (track->handlerType == HANDLER_VIDEO)
            {
                map->sample_type[sid] = sample_VIDEO;
                for (j = 0; j < track->stss_entry_count; j++)
                {
                    if (i == (track->stss_sample_number[j] - 1))
                        map->sample_type[sid] = sample_VIDEO_IDR;
                }
            }
            else if (track->handlerType == HANDLER_AUDIO)
            {
                map->sample_type[sid] = sample_AUDIO;
            }

            // Set sample size
            map->sample_size[sid] = track->stsz_entry_size[i];

            // Set sample offset
            map->sample_offset[sid] = track->stco_chunk_offset[chunk] + 4;

            for (j = 1; j <= posinchunk; j++)
            {
                map->sample_offset[sid] += track->stsz_entry_size[i - j];
            }

            // Set sample presentation timecode
            map->sample_pts[sid] = -1;

            // Set sample decoding timecode
            map->sample_dts[sid] = -1;
        }

#if ENABLE_DEBUG
        TRACE_INFO(MP4, BLD_GREEN ">> track content recap:\n" CLR_RESET);
        if (map->stream_type == stream_VIDEO)
        {
            TRACE_1(MP4, "Video Stream\n");
        }
        else if (map->stream_type == stream_AUDIO)
        {
            TRACE_1(MP4, "Audio Stream\n");
        }

        TRACE_1(MP4, "sample_count\t: %u\n", map->sample_count);
        TRACE_1(MP4, "sample_count_idr\t: %u\n", map->sample_count_idr);
/*
        for (i = 0; i < map->sample_count; i++)
        {
            TRACE_2(MP4, "[%u] sample type\t> %u\n", i, map->sample_type[i]);
            TRACE_1(MP4, "[%u] sample size\t> %u\n", i, map->sample_size[i]);
            TRACE_1(MP4, "[%u] sample offset\t> %lli\n", i, map->sample_offset[i]);
            TRACE_2(MP4, "[%u] sample pts\t> %lli\n", i, map->sample_timecode_presentation[i]);
            TRACE_2(MP4, "[%u] sample dts\t> %lli\n", i, map->sample_timecode_decoding[i]);
        }
*/
#endif /* ENABLE_DEBUG */
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Free a videoTrack_t structure.
 * \param **track_ptr A pointer to the videoTrack_t structure we want to freed.
 */
static void freeTrack(Mp4Track_t **track_ptr)
{
    if (*track_ptr != NULL)
    {
        // sps
        if ((*track_ptr)->sps_sample_offset != NULL)
        {
            free((*track_ptr)->sps_sample_offset);
        }

        if ((*track_ptr)->sps_sample_offset != NULL)
        {
            free((*track_ptr)->sps_sample_size);
        }

        // pps
        if ((*track_ptr)->pps_sample_offset != NULL)
        {
            free((*track_ptr)->pps_sample_offset);
        }

        if ((*track_ptr)->pps_sample_offset != NULL)
        {
            free((*track_ptr)->pps_sample_size);
        }

        // stss
        if ((*track_ptr)->stss_sample_number != NULL)
        {
            free((*track_ptr)->stss_sample_number);
        }

        // stsc
        if ((*track_ptr)->stsc_first_chunk != NULL)
        {
            free((*track_ptr)->stsc_first_chunk);
        }

        if ((*track_ptr)->stsc_samples_per_chunk != NULL)
        {
            free((*track_ptr)->stsc_samples_per_chunk);
        }

        if ((*track_ptr)->stsc_sample_description_index != NULL)
        {
            free((*track_ptr)->stsc_sample_description_index);
        }

        // stsz / stz2
        if ((*track_ptr)->stsz_entry_size != NULL)
        {
            free((*track_ptr)->stsz_entry_size);
        }

        // stco / co64
        if ((*track_ptr)->stco_chunk_offset != NULL)
        {
            free((*track_ptr)->stco_chunk_offset);
        }

        // track
        free(*track_ptr);
        *track_ptr = NULL;
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse box header.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 4.2 Object Structure.
 *
 * bitstr pointer is not checked for performance reason.
 */
static int parse_box_header(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_3(MP4, "parse_box_header()\n");
    int retcode = SUCCESS;

    if (box_header == NULL)
    {
        TRACE_ERROR(MP4, "Invalid Mp4Box_t structure!\n");
        retcode = FAILURE;
    }
    else
    {
        // Set box offset
        box_header->offset_start = bitstream_get_absolute_byte_offset(bitstr);

        // Read box size
        box_header->size = read_bits(bitstr, 32);

        if (box_header->size == 0)
        {
            box_header->largesize = bitstr->bitstream_size - box_header->offset_start;

            if (box_header->largesize > 0xFFFFFFFF)
            {
                box_header->size = 1;
            }
            else
            {
                box_header->size = (uint32_t)box_header->largesize;
            }
        }
        else if (box_header->size == 1)
        {
            box_header->largesize = (int64_t)read_bits_64(bitstr, 64);
        }

        // Set end offset
        box_header->offset_end = box_header->offset_start + box_header->size;

        // Read box type
        box_header->type = read_bits(bitstr, 32);

        if (box_header->type == BOX_UUID)
        {
            int i = 0;
            for (i = 0; i < 16; i++)
            {
                box_header->type_uuid[i] = (uint8_t)read_bits(bitstr, 8);
            }
        }

        // Init "FullBox" parameters
        box_header->version = 0;
        box_header->flags = 0;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Print a box header content.
 */
static void print_box_header(Mp4Box_t *box_header)
{
#if ENABLE_DEBUG
    TRACE_2(MP4, "* start offset\t: %u\n", box_header->offset_start);
    TRACE_2(MP4, "* end offset\t: %u\n", box_header->offset_end);

    // Print Box header
    TRACE_2(MP4, "* box size\t\t: %u\n", box_header->size);
    if (box_header->size == 1)
    {
        TRACE_2(MP4, "* box largesize\t: %u\n", box_header->largesize);
    }

    TRACE_2(MP4, "* box type\t\t: 0x%X\n", box_header->type);
    if (box_header->type == BOX_UUID) // "uuid"
    {
        TRACE_2(MP4, "* box uuid\t\t: %s\n", box_header->type_uuid);
    }

    // Print FullBox header
    if (box_header->version != 0 || box_header->flags != 0)
    {
        TRACE_2(MP4, "* version\t\t: %u\n", box_header->version);
        TRACE_2(MP4, "* flags\t\t: 0x%X\n", box_header->flags);
    }
#endif /* ENABLE_DEBUG */
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Padding bits box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.7.6 Padding Bits Box.

 * In some streams the media samples do not occupy all bits of the bytes given
 * by the sample size, and are padded at the end to a byte boundary. In some
 * cases, it is necessary to record externally the number of padding bits used.
 */
static int parse_padb(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_padb()\n" CLR_RESET);
    int retcode = SUCCESS;

    unsigned int i;
    unsigned int sample_count = read_bits(bitstr, 32);

    for (i = 0; i < ((sample_count + 1)/2); i++)
    {
        const int reserved1 = read_bit(bitstr);
        int pad1 = read_bits(bitstr, 3);
        const int reserved2 = read_bit(bitstr);
        int pad2 = read_bits(bitstr, 3);
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Unknown box, just parse header.
 *
 * When encountering an unknown box type, just print the header infos, the box
 * will be automatically skipped.
 */
static int parse_unknown_box(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_unknown_box()\n" CLR_RESET);
    int retcode = SUCCESS;

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief File Type Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 4.3 File Type Box.
 */
static int parse_ftyp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ftyp()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read brand identifier
    unsigned int major_brand = read_bits(bitstr, 32);

    // Read informative integer for the minor version of the major brand
    unsigned int minor_version = read_bits(bitstr, 32);

    // Read a list of brands, until the end of the box
    unsigned int compatible_brands[8] = {0};

    unsigned int i = 0;
    unsigned int nb_compatible_brands = (box_header->size - 16) / 4;

    if (nb_compatible_brands > 8)
    {
        TRACE_WARNING(MP4, "Too much compatible_brands! Consider handling more than 8.");
        nb_compatible_brands = 8;
    }

    for (i = 0; i < nb_compatible_brands; i++)
    {
        compatible_brands[i] = read_bits(bitstr, 32);
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> major_brand\t: 0x%08X\n", major_brand);
        TRACE_1(MP4, "> minor_version\t: %i\n", minor_version);
        for (i = 0; i < nb_compatible_brands; i++)
        {
            TRACE_1(MP4, "> compatible_brands[%i]\t: 0x%X\n", i, compatible_brands[i]);
        }
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}
/* ************************************************************************** */

/*!
 * \brief Progressive Download Information Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.3.1 Progressive Download Information Box.
 *
 * The Progressive download information box aids the progressive download of an
 * ISO file. The box contains pairs of numbers (to the end of the box)
 * specifying combinations of effective file download bitrate in units of
 * bytes/sec and a suggested initial playback delay in units of milliseconds.
 */
static int parse_pdin(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pdin()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read rate/initial_delay
    int tbr = box_header->offset_end - bitstream_get_absolute_byte_offset(bitstr);

    int i = 0;
    for (i = 0; i < tbr; i++)     // to end of box
    {
        unsigned int rate = read_bits(bitstr, 32);
        unsigned int initial_delay = read_bits(bitstr, 32);

        TRACE_1(MP4, "[i] > rate\t: %u\n", i, rate);
        TRACE_1(MP4, "    > initial_delay\t: %u\n", initial_delay);
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "pdin contains %i pairs of values\n", tbr);
/*
        for (i = 0; i < tbr; i++)
        {
            TRACE_1(MP4, "[i] > rate\t: %u\n", i, rate[i]);
            TRACE_1(MP4, "    > initial_delay\t: %u\n", initial_delay[i]);
        }
*/
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the container for metadata.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.2.1 Movie Box.
 *
 * The metadata for a presentation is stored in the single Movie Box which occurs
 * at the top-level of a file.
 * Normally this box is close to the beginning or end of the file, though this is not required.
 * This box does not contain informations, only other boxes.
 */
static int parse_moov(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_moov()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print moov box header
    print_box_header(box_header);
    mp4->box_moov_end = box_header->offset_end;

    while (retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS)
        {
            switch (box_subheader.type)
            {
                case BOX_MVHD:
                    retcode = parse_mvhd(bitstr, &box_subheader, mp4);
                    break;
                case BOX_IODS:
                    //retcode = parse_iods(bitstr, &box_subheader, mp4); //FIXME
                    break;
                case BOX_TRAK:
                {
                    retcode = parse_trak(bitstr, &box_subheader, mp4);
/*
                    {
                        if (convertTrack(video, track))
                        {
                            parsing_done = true;
                            retcode = SUCCESS;
                        }
                    }
                    freeTrack(&track);
*/
                }
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader);
                    break;
            }

            // Go to the next box
            int jump = box_subheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the Media Data Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.1.1 Media Data Box.
 *
 * This box contains the media data. In video tracks, this box would contain
 * video frames.
 * The parser doesn't really care for this box as long as we have already
 * indexed the A/V samples.
 */
static int parse_mdat(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdat()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print mdat box header
    print_box_header(box_header);

    // Skip mdat box content
    bitstream_goto_offset(bitstr, box_header->offset_end);

    return retcode;
}


/* ************************************************************************** */

/*!
 * \brief Track Header Box - Fullbox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.3.2 Track Header Box.
 *
 * This box specifies the characteristics of a single track.
 * Exactly one Track Header Box is contained in a track.
 */
static int parse_tkhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_tkhd()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    if (box_header->version == 1)
    {
        track->creation_time = read_bits_64(bitstr, 64);
        track->modification_time = read_bits_64(bitstr, 64);
        track->id = read_bits(bitstr, 32);
        /*const unsigned int reserved =*/ read_bits(bitstr, 32);
        track->duration = read_bits_64(bitstr, 64);
    }
    else // if (box_header->version == 0)
    {
        track->creation_time = read_bits(bitstr, 32);
        track->modification_time = read_bits(bitstr, 32);
        track->id = read_bits(bitstr, 32);
        /*const unsigned int reserved =*/ read_bits(bitstr, 32);
        track->duration = read_bits(bitstr, 32);
    }

    unsigned int reserved[2] = {0};
    reserved[0] = read_bits(bitstr, 32);
    reserved[1] = read_bits(bitstr, 32);

    int layer = read_bits(bitstr, 16);
    int alternate_group = read_bits(bitstr, 16);
    int volume = read_bits(bitstr, 16);
    /*const unsigned int reserved =*/ read_bits(bitstr, 16);

    int i = 0, matrix[9] = {0};
    for (i = 0; i < 9; i++)
    {
        matrix[i] = read_bits(bitstr, 32);
    }

    unsigned int width = read_bits(bitstr, 32);
    unsigned int height = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    {
        // Print tkhd box header
        print_box_header(box_header);

        // Print tkhd box content
        TRACE_1(MP4, "> creation_time\t: %llu\n", track->creation_time);
        TRACE_1(MP4, "> modification_time\t: %llu\n", track->modification_time);
        TRACE_1(MP4, "> track_ID\t\t: %u\n", track->id);
        TRACE_1(MP4, "> duration\t\t: %llu\n", track->duration);
        TRACE_1(MP4, "> layer\t\t: %i\n", layer);
        TRACE_1(MP4, "> alternate_group\t: %i\n", alternate_group);
        TRACE_1(MP4, "> volume\t\t: %i\n", volume);
        TRACE_1(MP4, "> matrix\t: [%u, %u, %u, %u, %u, %u, %u, %u, %u]\n",
                matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8]);
        TRACE_1(MP4, "> width\t: %u\n", width);
        TRACE_1(MP4, "> height\t: %u\n", height);
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse the Movie Header Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.2.2 Movie Header Box.
 *
 * This box defines overall information which is media-independent, and relevant
 * to the entire presentation considered as a whole.
 */
static int parse_mvhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mvhd()\n" CLR_RESET);
    int retcode = SUCCESS;
    int i = 0;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    if (box_header->version == 1)
    {
        mp4->creation_time = read_bits_64(bitstr, 64);
        mp4->modification_time = read_bits_64(bitstr, 64);
        mp4->timescale = read_bits(bitstr, 32);
        mp4->duration = read_bits_64(bitstr, 64);
    }
    else // if (version == 0)
    {
        mp4->creation_time = read_bits(bitstr, 32);
        mp4->modification_time = read_bits(bitstr, 32);
        mp4->timescale = read_bits(bitstr, 32);
        mp4->duration = read_bits(bitstr, 32);
    }

    uint32_t rate = read_bits(bitstr, 32);
    uint32_t volume = read_bits(bitstr, 16);
    /*unsigned int reserved =*/ read_bits(bitstr, 16);
    /*unsigned int reserved =*/ read_bits(bitstr, 32);
    /*unsigned int reserved =*/ read_bits(bitstr, 32);

    // Provides a transformation matrix for the video;
    int32_t matrix[9];
    for (i = 0; i < 9; i++)
    {
        matrix[i] = read_bits(bitstr, 32);
    }

    // ?
    int32_t predefined[6];
    for (i = 0; i < 6; i++)
    {
        predefined[i] = read_bits(bitstr, 32);
    }

    uint32_t next_track_ID = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    {
        // Print mvhd box header
        print_box_header(box_header);

        // Print mvhd box content
        TRACE_1(MP4, "> creation_time\t: %llu\n", mp4->creation_time);
        TRACE_1(MP4, "> modification_time\t: %llu\n", mp4->modification_time);
        TRACE_1(MP4, "> timescale\t\t: %u\n", mp4->timescale);
        TRACE_1(MP4, "> duration\t\t: %llu\n", mp4->duration);
        TRACE_1(MP4, "> rate\t\t: %u\n", rate);
        TRACE_1(MP4, "> volume\t\t: %llu\n", volume);
        for (i = 0; i < 9; i++)
        {
            TRACE_1(MP4, "> matrix[%i]\t: %i\n", i, matrix[i]);
        }
        TRACE_1(MP4, "> next track ID\t: %u\n", next_track_ID);

    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

// mdio?

/* ************************************************************************** */

/*!
 * \brief Parse the container for individual track or stream.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.3.3 Track Reference Box.
 *
 * This box provides a reference from the containing track to another track in the
 * presentation. These references are typed. A ‘hint’ reference links from the
 * containing hint track to the media data that it hints. A content description
 * reference ‘cdsc’ links a descriptive or metadata track to the content which it describes.
 * Exactly one Track Reference Box can be contained within the Track Box.
 * If this box is not present, the track is not referencing any other track in any
 * way. The reference array is sized to fill the reference type box.
 *
 * This box does not contain informations, only other boxes.
 */
static int parse_trak(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_trak()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print trak box header
    print_box_header(box_header);

    // Init a track structure
    int track_id = mp4->tracks_count;
    mp4->tracks[track_id] = (Mp4Track_t*)calloc(1, sizeof(Mp4Track_t));

    if (mp4->tracks[track_id] == NULL)
    {
        TRACE_ERROR(MP4, "Unable to allocate a new mp4 track!\n");
        retcode = FAILURE;
    }
    else
    {
        mp4->tracks[track_id]->id = track_id;
        mp4->tracks_count++;
    }

    while (retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS)
        {
            switch (box_subheader.type)
            {
                case BOX_TKHD:
                    retcode = parse_tkhd(bitstr, &box_subheader, mp4->tracks[track_id]);
                    break;
                case BOX_EDTS:
                    //retcode = parse_edts(bitstr, &box_subheader, mp4->tracks[track_id]);
                    break;
                case BOX_MDIA:
                    retcode = parse_mdia(bitstr, &box_subheader, mp4->tracks[track_id]);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader);
                    break;
            }

            // Go to the next box
            int jump = box_subheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Media Header Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.4.2 Media Header Box.
 *
 * The media header box declares overall information that is media-independent,
 * and relevant to characteristics of the media in a track.
 */
static int parse_mdhd(Bitstream_t *bitstr, Mp4Box_t *box_header)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdhd()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    uint64_t creation_time = 0;
    uint64_t modification_time = 0;
    uint32_t timescale = 0;
    uint64_t duration = 0;
    uint32_t language[3] = {0};

    // Read box content
    if (box_header->version == 1)
    {
        creation_time = read_bits_64(bitstr, 64);
        modification_time = read_bits_64(bitstr, 64);
        timescale = read_bits(bitstr, 32);
        duration = read_bits_64(bitstr, 64);
    }
    else // if (version == 0)
    {
        creation_time = read_bits(bitstr, 32);
        modification_time = read_bits(bitstr, 32);
        timescale = read_bits(bitstr, 32);
        duration = read_bits(bitstr, 32);
    }

    /*unsigned int pad =*/ read_bit(bitstr);

    // ISO-639-2/T language code
    language[0] = read_bits(bitstr, 5);
    language[1] = read_bits(bitstr, 5);
    language[2] = read_bits(bitstr, 5);

    /*unsigned int pre_defined =*/ read_bits(bitstr, 16);

#if ENABLE_DEBUG
    {
        // Print mdhd box header
        print_box_header(box_header);

        // Print mdhd box content
        TRACE_1(MP4, "> creation_time\t: %llu\n", creation_time);
        TRACE_1(MP4, "> modification_time\t: %llu\n", modification_time);
        TRACE_1(MP4, "> timescale\t\t: %u\n", timescale);
        TRACE_1(MP4, "> duration\t\t: %llu\n", duration);
        TRACE_1(MP4, "> language[0]\t: %u\n", language[0]);
        TRACE_1(MP4, "> language[1]\t: %u\n", language[1]);
        TRACE_1(MP4, "> language[2]\t: %u\n", language[2]);
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Media Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.4.1 Media Box.
 *
 * The media declaration container contains all the objects that declare information
 * about the media data within a track.
 * This box does not contain informations, only other boxes.
 */
static int parse_mdia(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mdia()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print mdia box header
    print_box_header(box_header);

    while (retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS)
        {
            switch (box_subheader.type)
            {
                case BOX_MDHD:
                    retcode = parse_mdhd(bitstr, &box_subheader);
                    break;
                case BOX_HDLR:
                    retcode = parse_hdlr(bitstr, &box_subheader, track);
                    break;
                case BOX_MINF:
                    retcode = parse_minf(bitstr, &box_subheader, track);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader);
                    break;
            }

            // Go to the next box
            int jump = box_subheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Handler Reference Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.4.3 Handler Reference Box.
 *
 * This box within a Media Box declares the process by which the media-data in the
 * track is presented, and thus, the nature of the media in a track. For example,
 * a video track would be handled by a video handler.
 */
static int parse_hdlr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hdlr()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Read box content
    unsigned int pre_defined = read_bits(bitstr, 32);
    track->handlerType = read_bits(bitstr, 32);

    unsigned int reserved[3] = {0};
    reserved[0] = read_bits(bitstr, 32);
    reserved[1] = read_bits(bitstr, 32);
    reserved[2] = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    {
        // Print hdlr box header
        print_box_header(box_header);

        // Print hdlr box content
        TRACE_1(MP4, "> pre_defined\t: %u\n", pre_defined);
        TRACE_1(MP4, "> handler_type\t: 0x%X\n", track->handlerType);
    }
#endif /* ENABLE_DEBUG */

    if (track->handlerType != HANDLER_AUDIO &&
        track->handlerType != HANDLER_VIDEO)
    {
        TRACE_1(MP4, "Not an audio or video track, ignoring\n");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Media Information Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.4.4 Media Information Box.
 *
 * This box contains all the objects that declare characteristic information of
 * the media in the track.
 * This box does not contain informations, only other boxes.
 */
static int parse_minf(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_minf()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print minf box header
    print_box_header(box_header);

    // Subbox allocation
    Mp4Box_t box_subheader;

    while (retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS &&
            bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
        {
            switch (box_subheader.type)
            {
                case BOX_DINF:
                    //retcode = parse_unknown_box(bitstr, box_subheader); //FIXME
                    break;
                case BOX_STBL:
                    retcode = parse_stbl(bitstr, &box_subheader, track);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader);
                    break;
            }

            // Go to the next box
            int jump = box_subheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Table Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.5.1 Sample Table Box.
 *
 * Parse the sample table box, container for the time/space map.
 * This box does not contain informations, only other boxes.
 */
static int parse_stbl(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stbl()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Print stbl box header
    print_box_header(box_header);

    while (retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (retcode == SUCCESS)
        {
            switch (box_subheader.type)
            {
                case BOX_STSD:
                    retcode = parse_stsd(bitstr, &box_subheader, track);
                    break;
                case BOX_STTS:
                    retcode = parse_stts(bitstr, &box_subheader, track);
                    break;
                case BOX_CTTS:
                    retcode = parse_ctts(bitstr, &box_subheader, track);
                    break;
                case BOX_STSS:
                    retcode = parse_stss(bitstr, &box_subheader, track);
                    break;
                case BOX_STSC:
                    retcode = parse_stsc(bitstr, &box_subheader, track);
                    break;
                case BOX_STSZ:
                    retcode = parse_stsz(bitstr, &box_subheader, track);
                    break;
                case BOX_STZ2:
                    retcode = parse_stsz(bitstr, &box_subheader, track);
                    break;
                case BOX_STCO:
                    retcode = parse_stco(bitstr, &box_subheader, track);
                    break;
                case BOX_CO64:
                    retcode = parse_stco(bitstr, &box_subheader, track);
                    break;
                default:
                    retcode = parse_unknown_box(bitstr, &box_subheader);
                    break;
            }

            // Go to the next box
            unsigned jump = box_subheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
            if (jump > 0)
            {
                skip_bits(bitstr, jump*8);
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Description Box.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.5.2 Sample Description Box.
 *
 * The SampleDescriptionBox contains information about codec types and some
 * initialization parameters needed to start decoding.
 * If an AVC box (AVCDecoderConfigurationRecord) is present, it also contains the
 * diferents SPS and PPS of the video.
 */
static int parse_stsd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int reserved[6] = {0};
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        reserved[i] = read_bits(bitstr, 8);
    }
    /*unsigned int data_reference_index =*/ read_bits(bitstr, 16);

    // Parse subbox header
    Mp4Box_t box_subheader;
    retcode = parse_box_header(bitstr, &box_subheader);

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    /*unsigned int entry_count =*/ read_bits(bitstr, 32);

    // Then parse subbox content
    switch (track->handlerType)
    {
        case HANDLER_AUDIO:

            // AudioSampleEntry
            // Box Types: ‘mp4a’

            if (box_subheader.type == SAMPLE_MP4A)
            {
                track->codec = CODEC_AAC;
                TRACE_1(MP4, "> Audio track is using AAC codec\n");
            }
            else if (box_subheader.type == SAMPLE_AC3)
            {
                track->codec = CODEC_AC3;
                TRACE_1(MP4, "> Audio track is using AC3 codec\n");
            }
            else
            {
                track->codec = CODEC_UNKNOWN;
                TRACE_WARNING(MP4, "> Unknown codec in audio track\n");
            }

            /*const unsigned int reserved[0] =*/ read_bits(bitstr, 32);
            /*const unsigned int reserved[1] =*/ read_bits(bitstr, 32);

            track->channel_count = read_bits(bitstr, 16);
            track->sample_size = read_bits(bitstr, 16);

            /*unsigned int pre_defined =*/ read_bits(bitstr, 16);
            /*const unsigned int(16) reserved =*/ read_bits(bitstr, 16);

            track->sample_rate = read_bits(bitstr, 32);

        break;

        case HANDLER_VIDEO:

            // VisualSampleEntry
            // Box Types: ‘avc1’, ‘avcC’, ‘m4ds’, ’btrt’

            if (box_subheader.type == SAMPLE_AVC1)
            {
                track->codec = CODEC_H264;
                TRACE_1(MP4, "> Video track is using H.264 codec\n");
            }
            else if (box_subheader.type == SAMPLE_MP4V)
            {
                track->codec = CODEC_MPEG4;
                TRACE_1(MP4, "> Video track is using XVID codec\n");
            }
            else
            {
                track->codec = CODEC_UNKNOWN;
                TRACE_WARNING(MP4, "> Unknown codec in video track\n");
            }

            /*unsigned int pre_defined =*/ read_bits(bitstr, 16);
            /*const unsigned int reserved =*/ read_bits(bitstr, 16);

            unsigned int pre_defined[3] = {0};
            pre_defined[0] = read_bits(bitstr, 32);
            pre_defined[1] = read_bits(bitstr, 32);
            pre_defined[2] = read_bits(bitstr, 32);

            track->width = read_bits(bitstr, 16);
            track->height = read_bits(bitstr, 16);

            // 0x00480000; // 72 dpi
            unsigned int horizresolution = read_bits(bitstr, 32);
            unsigned int vertresolution = read_bits(bitstr, 32);

            /*const unsigned int reserved =*/ read_bits(bitstr, 32);

            unsigned int frame_count = read_bits(bitstr, 16);

            uint8_t compressorname[32] = {0};
            for (i = 0; i < 32; i++)
            {
                compressorname[i] = (uint8_t)read_bits(bitstr, 8);
            }

            track->color_depth = read_bits(bitstr, 16);
            /*int pre_defined = */ read_bits(bitstr, 16);

#if ENABLE_DEBUG
            {
                // Print box header
                print_box_header(box_header);

                // Print VisualSampleEntry box header
                print_box_header(&box_subheader);

                // Print VisualSampleEntry box content
                TRACE_1(MP4, "> width\t\t: %u\n", track->width);
                TRACE_1(MP4, "> height\t\t: %u\n", track->height);
                TRACE_1(MP4, "> horizresolution\t: 0x%X\n", horizresolution);
                TRACE_1(MP4, "> vertresolution\t: 0x%X\n", vertresolution);
                TRACE_1(MP4, "> frame_count\t: %u\n", frame_count);
                TRACE_1(MP4, "> compressor\t: '%s'\n", compressorname);
                TRACE_1(MP4, "> color depth\t: %u\n", track->color_depth);
            }
#endif /* ENABLE_DEBUG */

            while (retcode == SUCCESS &&
                   bitstream_get_absolute_byte_offset(bitstr) < box_subheader.offset_end)
            {
                // Parse subbox header
                Mp4Box_t box_subsubheader;
                retcode = parse_box_header(bitstr, &box_subsubheader);

                // Then parse subbox content
                ////////////////////////////////////////////////////////////////
                if (retcode == SUCCESS)
                {
                    switch (box_subsubheader.type)
                    {
                        case BOX_AVCC:
                            retcode = parse_avcC(bitstr, &box_subsubheader, track);
                            break;
                        case BOX_BTRT:
                            retcode = parse_btrt(bitstr, &box_subsubheader, track);
                            break;
                        default:
                            retcode = parse_unknown_box(bitstr, &box_subsubheader);
                            break;
                    }

                    // Go to the next box
                    int jump = box_subsubheader.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                    if (jump > 0)
                    {
                        skip_bits(bitstr, jump*8);
                    }
                }
            }
/*
            // CleanApertureBox clap; // optional
            ////////////////////////////////////////////////////////////
            unsigned int clap_size = read_bits(bitstr, 32);
            unsigned int clap_type = read_bits(bitstr, 32);

            unsigned int cleanApertureWidthN = read_bits(bitstr, 32);
            unsigned int cleanApertureWidthD = read_bits(bitstr, 32);
            unsigned int cleanApertureHeightN = read_bits(bitstr, 32);
            unsigned int cleanApertureHeightD = read_bits(bitstr, 32);
            unsigned int horizOffN = read_bits(bitstr, 32);
            unsigned int horizOffD = read_bits(bitstr, 32);
            unsigned int vertOffN = read_bits(bitstr, 32);
            unsigned int vertOffD = read_bits(bitstr, 32);

            // PixelAspectRatioBox pasp; // optional
            ////////////////////////////////////////////////////////////
            unsigned int pasp_size = read_bits(bitstr, 32);
            unsigned int pasp_type = read_bits(bitstr, 32);

            unsigned int hSpacing = read_bits(bitstr, 32);
            unsigned int vSpacing = read_bits(bitstr, 32);
*/
        break;

        default:
            TRACE_1(MP4, "Not a video track, skipped...\n");
        break;
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AVCConfigurationBox.
 *
 * From 'ISO/IEC 14496-15' specification :
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.1 Syntax, 5.2.4.1.2 Semantics).
 */
static int parse_avcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_avcC()\n" CLR_RESET);
    int retcode = SUCCESS;

    // avcC box means H.264 codec
    track->codec = CODEC_H264;

    // Parse box content
    unsigned int i = 0;

    unsigned int configurationVersion = read_bits(bitstr, 8);
    unsigned int AVCProfileIndication = read_bits(bitstr, 8);
    unsigned int profile_compatibility = read_bits(bitstr, 8);
    unsigned int AVCLevelIndication = read_bits(bitstr, 8);
    /*int reserved =*/ read_bits(bitstr, 6);
    unsigned int lengthSizeMinusOne = read_bits(bitstr, 2);
    /*int reserved =*/ read_bits(bitstr, 3);

    // SPS
    track->sps_count = read_bits(bitstr, 5); // MAX_SPS = 32
    track->sps_sample_offset = (int64_t*)calloc(track->sps_count, sizeof(int64_t));
    track->sps_sample_size = (unsigned int*)calloc(track->sps_count, sizeof(unsigned int));
    for (i = 0; i < track->sps_count; i++)
    {
        track->sps_sample_size[i] = read_bits(bitstr, 16);
        track->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

        skip_bits(bitstr, track->sps_sample_size[i] * 8); // sequenceParameterSetNALUnit
    }

    // PPS
    track->pps_count = read_bits(bitstr, 8); // MAX_PPS = 256
    track->pps_sample_offset = (int64_t*)calloc(track->pps_count, sizeof(int64_t));
    track->pps_sample_size = (unsigned int*)calloc(track->pps_count, sizeof(unsigned int));
    for (i = 0; i < track->pps_count; i++)
    {
       track->pps_sample_size[i] = read_bits(bitstr, 16);
       track->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

       skip_bits(bitstr, track->pps_sample_size[i] * 8); // pictureParameterSetNALUnit
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> configurationVersion\t: %u\n", configurationVersion);
        TRACE_1(MP4, "> AVCProfileIndication\t: %u\n", AVCProfileIndication);
        TRACE_1(MP4, "> profile_compatibility\t: %u\n", profile_compatibility);
        TRACE_1(MP4, "> AVCLevelIndication\t: %u\n", AVCLevelIndication);
        TRACE_1(MP4, "> lengthSizeMinusOne\t: %u\n", lengthSizeMinusOne);

        TRACE_1(MP4, "> numOfSequenceParameterSets    = %u\n", track->sps_count);
        for (i = 0; i < track->sps_count; i++)
        {
            TRACE_1(MP4, "> sequenceParameterSetLength[%u] : %u\n", i, track->sps_sample_size[i]);
            TRACE_1(MP4, "> sequenceParameterSetOffset[%u] : %u\n", i, track->sps_sample_offset[i]);
        }

        TRACE_1(MP4, "> numOfPictureParameterSets     = %u\n", track->pps_count);
        for (i = 0; i < track->pps_count; i++)
        {
            TRACE_1(MP4, "> pictureParameterSetLength[%u]  : %u\n", i, track->pps_sample_size[i]);
            TRACE_1(MP4, "> pictureParameterSetOffset[%u]  : %u\n", i, track->pps_sample_offset[i]);
        }
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief BitRateBox.
 *
 * From 'ISO/IEC 14496-15' specification :
 * 8.5.2 Sample Description Box.
 */
static int parse_btrt(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_btrt()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int bufferSizeDB = read_bits(bitstr, 32);
    track->bitrate_max = read_bits(bitstr, 32);
    track->bitrate_avg = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> bufferSizeDB\t: %u\n", bufferSizeDB);
        TRACE_1(MP4, "> maxBitrate\t: %u\n", track->bitrate_max);
        TRACE_1(MP4, "> avgBitrate\t: %u\n", track->bitrate_avg);
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Decoding Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.6.1.2 Decoding Time to Sample Box.
 *
 * This box contains a compact version of a table that allows indexing from
 * decoding time to sample number.
 * Other tables give sample sizes and pointers, from the sample number. Each
 * entry in the table gives the number of consecutive samples with the same time
 * delta, and the delta of those samples. By adding the deltas a complete
 * time-to-sample map may be built.
 *
 * The Decoding Time to Sample Box contains decode time delta's:
 * DT(n+1) = DT(n) + STTS(n) where STTS(n) is the (uncompressed) table entry for
 * sample n.
 *
 * The sample entries are ordered by decoding time stamps; therefore the deltas
 * are all non-negative.
 *
 * The DT axis has a zero origin; DT(i) = SUM(for j=0 to i-1 of delta(j)), and
 * the sum of all deltas gives the length of the media in the track (not mapped
 * to the overall timescale, and not considering any edit list).
 *
 * The Edit List Box provides the initial CT value if it is non-empty (non-zero).
 */
static int parse_stts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stts()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    int i = 0;

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Composition Time to Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.6.1.3 Composition Time to Sample Box.
 *
 * This box provides the offset between decoding time and composition time.
 * Since decoding time must be less than the composition time, the offsets are
 * expressed as unsigned numbers such that CT(n) = DT(n) + CTTS(n) where CTTS(n)
 * is the (uncompressed) table entry for sample n.
 *
 * The composition time to sample table is optional and must only be present if
 * DT and CT differ for any samples.
 */
static int parse_ctts(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_ctts()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    int i = 0;

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sync Sample Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.6.2 Sync Sample Box.
 *
 * This box provides a compact marking of the random access points within the stream.
 * The table is arranged in strictly increasing order of sample number.
 * If the sync sample box is not present, every sample is a random access point.
 */
static int parse_stss(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stss()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stss_entry_count = read_bits(bitstr, 32);
    track->stss_sample_number = (unsigned int*)calloc(track->stss_entry_count, sizeof(unsigned int));

    if (track->stss_sample_number == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc entry_table table!\n");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->stss_entry_count; i++)
        {
            track->stss_sample_number[i] = read_bits(bitstr, 32);
        }

#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);

            // Print box content
            TRACE_1(MP4, "> entry_count\t: %u\n", track->stss_entry_count);
/*
            TRACE_1(MP4, "> sample_number\t: [");
            for (i = 0; i < track->stss_entry_count; i++)
            {
                printf("%u, ", track->stss_sample_number[i]);
            }
            printf("]\n");
*/
        }
#endif /* ENABLE_DEBUG */
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample To Chunk Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.7.4 Sample To Chunk Box.
 *
 * Samples within the media data are grouped into chunks. Chunks can be of different
 * sizes, and the samples within a chunk can have different sizes. This table can
 * be used to find the chunk that contains a sample, its position, and the associated
 * sample description.
 * The table is compactly coded. Each entry gives the index of the first chunk of
 * a run of chunks with the same characteristics. By subtracting one entry here from
 * the previous one, you can compute how many chunks are in this run.
 * You can convert this to a sample count by multiplying by the appropriate samples-per-chunk.
 */
static int parse_stsc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsc()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stsc_entry_count = read_bits(bitstr, 32);
    track->stsc_first_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
    track->stsc_samples_per_chunk = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));
    track->stsc_sample_description_index = (unsigned int*)calloc(track->stsc_entry_count, sizeof(unsigned int));

    if (track->stsc_first_chunk == NULL ||
        track->stsc_samples_per_chunk == NULL ||
        track->stsc_sample_description_index == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc first_chunk, samples_per_chunk or sample_description_index tables!\n");
        retcode = FAILURE;
    }
    else
    {
        for (i = 0; i < track->stsc_entry_count; i++)
        {
            track->stsc_first_chunk[i] = read_bits(bitstr, 32);
            track->stsc_samples_per_chunk[i] = read_bits(bitstr, 32);
            track->stsc_sample_description_index[i] = read_bits(bitstr, 32);
        }

#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);

            // Print box content
            TRACE_1(MP4, "> entry_count\t: %u\n", track->stsc_entry_count);
/*
            TRACE_1(MP4, "> first_chunk\t: [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_first_chunk[i]);
            }
            printf("]\n");

            TRACE_1(MP4, "> samples_per_chunk\t: [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_samples_per_chunk[i]);
            }
            printf("]\n");

            TRACE_1(MP4, "> sample_description_index : [");
            for (i = 0; i < track->stsc_entry_count; i++)
            {
                printf("%u, ", track->stsc_sample_description_index[i]);
            }
            printf("]\n");
*/
        }
#endif /* ENABLE_DEBUG */
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Size Boxes - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.7.3 Sample Size Box
 *
 * This box contains the sample count and a table giving the size in bytes of each
 * sample. This allows the media data itself to be unframed. The total number of
 * samples in the media is always indicated in the sample count.
 *
 * This box has two variants : STSZ and STZ2.
 * - This variant has a fixed size 32-bit field for representing the sample
 *   sizes; it permits defining a constant size for all samples in a track.
 * - The STZ2 variant permits smaller size fields, to save space when the sizes
 *   are varying but small.
 */
static int parse_stsz(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsz()\n" CLR_RESET);
    int retcode = SUCCESS;
    unsigned int i = 0;
    int field_size = 32;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    if (box_header->type == BOX_STSZ)
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
             TRACE_ERROR(MP4, "Unable to alloc entry_size table!\n");
             retcode = FAILURE;
        }
        else
        {
            for (i = 0; i < track->stsz_sample_count; i++)
            {
                track->stsz_entry_size[i] = read_bits(bitstr, field_size);
            }
        }
    }

#if ENABLE_DEBUG
    {
        // Print box header
        print_box_header(box_header);

        // Print box content
        TRACE_1(MP4, "> sample_count\t: %u\n", track->stsz_sample_count);
        TRACE_1(MP4, "> sample_size\t: %u\n", track->stsz_sample_size);
/*
        if (track->stsz_sample_size == 0)
        {
            TRACE_1(MP4, "> entry_size : [");
            for (i = 0; i < track->stsz_sample_count; i++)
            {
                printf("%u, ", track->stsz_entry_size[i]);
            }
            printf("]\n");
        }
*/
    }
#endif /* ENABLE_DEBUG */

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Chunk Offset Box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification :
 * 8.7.5 Chunk Offset Box.
 *
 * The chunk offset table gives the index of each chunk into the containing file.
 * There are two variants, permitting the use of 32-bit (STCO variant) or 64-bit
 * offsets (CO64 variant).
 * The latter is useful when managing very large presentations. At most one of
 * these variants will occur in any single instance of a sample table.
 *
 * Offsets are file offsets, not the offset into any box within the file (e.g. Media
 * Data Box). This permits referring to media data in files without any box structure.
 * It does also mean that care must be taken when constructing a self-contained ISO
 * file with its metadata (Movie Box) at the front, as the size of the Movie Box will
 * affect the chunk offsets to the media data.
 */
static int parse_stco(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stco()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i = 0;
    track->stco_entry_count = read_bits(bitstr, 32);
    track->stco_chunk_offset = (int64_t*)calloc(track->stco_entry_count, sizeof(int64_t));

    if (track->stco_chunk_offset == NULL)
    {
        TRACE_ERROR(MP4, "Unable to alloc chunk_offset table!\n");
        retcode = FAILURE;
    }
    else
    {
        if (box_header->type == BOX_CO64)
        {
            for (i = 0; i < track->stco_entry_count; i++)
            {
                track->stco_chunk_offset[i] = (int64_t)read_bits_64(bitstr, 64);
            }
        }
        else //if (box_header->type == BOX_STCO)
        {
            for (i = 0; i < track->stco_entry_count; i++)
            {
                track->stco_chunk_offset[i] = (int64_t)read_bits(bitstr, 32);
            }
        }
#if ENABLE_DEBUG
        {
            // Print box header
            print_box_header(box_header);

            // Print box content
            TRACE_1(MP4, "> entry_count\t: %u\n", track->stco_entry_count);
/*
            TRACE_1(MP4, "> chunk_offset\t: [");
            for (i = 0; i < track->stco_entry_count; i++)
            {
                printf("%lli, ", track->stco_chunk_offset[i]);
            }
            printf("]\n");
*/
        }
#endif /* ENABLE_DEBUG */
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Parse a mp4 file.
 * \param *video A pointer to a VideoFile_t structure.
 * \return retcode 1 if succeed, 0 otherwise.
 *
 * This parser is based on the 'ISO/IEC 14496-12' international standard, part 12:
 * 'ISO base media file format'.
 *
 * \todo Stop parsing if we are after the end of the 'moov' box.
 * \todo Fix convertTrack() algorithms complexity.
 */
int mp4_fileParse(VideoFile_t *video)
{
    TRACE_INFO(MP4, BLD_GREEN "mp4_fileParse()\n" CLR_RESET);
    int retcode = SUCCESS;

    // Init bitstream to parse container infos
    Bitstream_t *bitstr = init_bitstream(video, NULL);

    if (bitstr != NULL)
    {
        // Init an MP4 structure
        Mp4_t mp4;
        mp4.tracks_count = 0;
        mp4.box_moov_end = video->file_size;

        while (retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < mp4.box_moov_end &&
               bitstream_get_absolute_byte_offset(bitstr) < video->file_size)
        {
            // Read box header
            Mp4Box_t box_header;
            retcode = parse_box_header(bitstr, &box_header);

            // Then parse box content
            if (retcode == SUCCESS)
            {
                switch (box_header.type)
                {
                    case BOX_FTYP:
                        retcode = parse_ftyp(bitstr, &box_header, &mp4);
                        break;
                    case BOX_PDIN:
                        retcode = parse_pdin(bitstr, &box_header, &mp4);
                        break;
                    case BOX_MOOV:
                        retcode = parse_moov(bitstr, &box_header, &mp4);
                        break;
                    case BOX_MOOF:
                        //retcode = parse_unknown_box(bitstr, &box_header); //FIXME
                        break;
                    case BOX_MDAT:
                        retcode = parse_mdat(bitstr, &box_header);
                        break;
                    case BOX_FREE:
                        //retcode = parse_unknown_box(bitstr, &box_header); //FIXME
                        break;
                    default:
                        retcode = parse_unknown_box(bitstr, &box_header);
                        break;
                }

                // Go to the next box
                int jump = box_header.offset_end - bitstream_get_absolute_byte_offset(bitstr);
                if (jump > 0)
                {
                    skip_bits(bitstr, jump*8);
                }
            }
        }

        // File metadatas
        video->duration = (float)mp4.duration / mp4.timescale * 1000;
        video->creation_time = (float)mp4.creation_time ;// / mp4.timescale * 1000;
        video->modification_time = (float)mp4.modification_time ;// / mp4.timescale * 1000;

        // Tracks metadatas
        // Check if we have extracted tracks
        if (mp4.tracks_count == 0)
        {
            TRACE_WARNING(MP4, "No tracks extracted!");
            retcode = FAILURE;
        }
        else // Convert tracks
        {
            unsigned int i = 0;
            for (i = 0; i < mp4.tracks_count; i++)
            {
                retcode = convertTrack(video, &mp4, mp4.tracks[i]);
            }
        }

        // Free bitstream
        free_bitstream(&bitstr);
    }

    return retcode;
}

/* ************************************************************************** */
