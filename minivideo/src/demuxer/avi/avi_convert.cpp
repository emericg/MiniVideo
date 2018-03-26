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
 * \file      avi_convert.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

// minivideo headers
#include "avi_convert.h"
#include "avi_struct.h"
#include "../riff/riff.h"
#include "../../utils.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_twocc.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

/* ************************************************************************** */

/*!
 * \brief Parse AVI 1.0 Index.
 *
 * The index as described is the index you will find in AVI 1.0 files. It is
 * placed after the movi list in the RIFF AVI List.
 */
int parse_idx1(Bitstream_t *bitstr, MediaFile_t *media, RiffChunk_t *idx1_header, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_idx1()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    if (idx1_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid idx1_header structure!");
        retcode = FAILURE;
    }
    else
    {
        print_chunk_header(idx1_header);
        write_chunk_header(idx1_header, avi->xml, "AVI 1.0 index");

        // Compute number of index entries
        uint32_t index_entry_count = idx1_header->dwSize / 16;
        TRACE_1(AVI, "> index_entry : %u", index_entry_count);

        if (avi->xml) fprintf(avi->xml, "  <index_entry_count>%u</index_entry_count>\n", index_entry_count);
        if (avi->xml) fprintf(avi->xml, "  </a>\n");

        // Check if the tracks have already been indexed
        int track_left = 0;
        for (unsigned i = 0; i < avi->tracks_count; i++)
        {
            if (avi->tracks[i]->track_indexed == false &&
                avi->tracks[i]->superindex_count == 0)
            {
                track_left++;
                retcode = avi_indexer_initmap(media, avi->tracks[i], index_entry_count);
            }
        }

        if (track_left == 0)
        {
            TRACE_1(AVI, "> We have an other way to index the file content");
            return SUCCESS;
        }

        if (retcode == SUCCESS)
        {
            uint32_t movioffset = avi->movi_offset + 4; // +4 to skip the chunk size
            TRACE_1(AVI, "> movi_offset : %i", movioffset);

            // Parse index structure
            for (unsigned i = 0; i < index_entry_count; i++)
            {
                uint32_t dwChunkId = read_bits(bitstr, 32);
                uint32_t dwFlags = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwChunkOffset = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwChunkLength = endian_flip_32(read_bits(bitstr, 32));

                // Set sample into bitstream map
                if ((dwChunkId & 0x0000FFFF) == 0x7762) // wb: audio
                {
                    TRACE_3(AVI, BLD_BLUE "> AUDIO" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_audio[tid]->sample_count;

                    media->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                    media->tracks_audio[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_audio[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_audio[tid]->sample_dts[sid] = -1;
                    media->tracks_audio[tid]->sample_pts[sid] = -1;

                    media->tracks_audio[tid]->sample_count++;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x6462 || // db: uncompressed video frame (RGB)
                         (dwChunkId & 0x0000FFFF) == 0x6463 || // dc: video frame
                         (dwChunkId & 0x0000FFFF) == 0x6976)   // iv: indeo video frame?
                {
                    TRACE_3(AVI, BLD_BLUE "> VIDEO" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_video[tid]->sample_count;

                    if (dwFlags == AVIIF_KEYFRAME)
                        media->tracks_video[tid]->sample_type[sid] = sample_VIDEO_SYNC;
                    else
                        media->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                    media->tracks_video[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_video[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_video[tid]->sample_dts[sid] = -1;
                    media->tracks_video[tid]->sample_pts[sid] = -1;

                    media->tracks_video[tid]->sample_count++;
                }
                else if ((dwChunkId & 0x0000FFFF) == 0x7478) // tx: subtitles
                {
                    TRACE_3(AVI, BLD_BLUE "> TEXT" CLR_RESET);
                    int tid = 0;
                    int sid = media->tracks_subt[tid]->sample_count;

                    media->tracks_subt[tid]->sample_type[sid] = sample_TEXT_FILE;
                    media->tracks_subt[tid]->sample_offset[sid] = movioffset + (int64_t)dwChunkOffset;
                    media->tracks_subt[tid]->sample_size[sid] = (int64_t)dwChunkLength;
                    media->tracks_subt[tid]->sample_dts[sid] = -1;
                    media->tracks_subt[tid]->sample_pts[sid] = -1;

                    media->tracks_subt[tid]->sample_count++;
                }
/*
                else if ((dwChunkId & 0x0000FFFF) == 0x7063) // pc: Palette change
                {
                    //
                }
                else if ((dwChunkId & 0xFFFF0000) == 0x6978) // ix: index
                {
                    //
                }
*/
                else
                {
                    TRACE_WARNING(AVI, "Unknown chunk type in idx1 (0x%08X) ('%s')!", dwChunkId, getFccString_le(dwChunkId, fcc));
                }

                // Print
                TRACE_3(AVI, "> dwChunkId    : 0x%08X ('%s')", dwChunkId, getFccString_le(dwChunkId, fcc));
                TRACE_3(AVI, "> dwFlags      : %u", dwFlags);
                TRACE_3(AVI, "> dwChunkOffset: %u", dwChunkOffset);
                TRACE_3(AVI, "> dwChunkLength: %u", dwChunkLength);
            }

            for (unsigned i = 0; i < avi->tracks_count; i++)
            {
                avi->tracks[i]->track_indexed = true;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Parse Open-DML 'Super Index'.
 *
 * Upper Level Index ('Super Index') contained in each 'strl' list.
 * The actual size of the entries of the aIndex array is entered into
 * wLongsPerEntry. Between the cb field and wLongsPerEntry the actual size of
 * the array is known. The field nEntriesInUse allows a chunk to be allocated
 * longer than the actual number of used elements in the array.
 */
int parse_indx(Bitstream_t *bitstr, RiffChunk_t *indx_header, avi_t *avi,  AviTrack_t *track)
{
    TRACE_INFO(AVI, BLD_GREEN "parse_indx()" CLR_RESET);
    int retcode = SUCCESS;

    if (indx_header == NULL)
    {
        TRACE_ERROR(AVI, "Invalid indx_header structure!");
        retcode = FAILURE;
    }
    else
    {
        char fcc[5];

        // Parse chunk content
        uint16_t wLongsPerEntry = endian_flip_16(read_bits(bitstr, 16));
        uint8_t bIndexSubType = read_bits(bitstr, 8);
        uint8_t bIndexType = read_bits(bitstr, 8);
        uint32_t nEntriesInUse = endian_flip_32(read_bits(bitstr, 32));
        uint32_t dwChunkId = read_bits(bitstr, 32);

#if ENABLE_DEBUG
        print_chunk_header(indx_header);
        TRACE_1(AVI, "> wLongsPerEntry  : %u", wLongsPerEntry);
        TRACE_1(AVI, "> bIndexSubType   : %u", bIndexSubType);
        TRACE_1(AVI, "> bIndexType      : %u", bIndexType);
        TRACE_1(AVI, "> nEntriesInUse   : %u", nEntriesInUse);
        TRACE_1(AVI, "> dwChunkId       : 0x%08X ('%s')!", dwChunkId, getFccString_le(dwChunkId, fcc));
#endif
        // xmlMapper
        if (avi->xml)
        {
            write_chunk_header(indx_header, avi->xml, "Index chunk");
            fprintf(avi->xml, "  <wLongsPerEntry>%u</wLongsPerEntry>\n", wLongsPerEntry);
            fprintf(avi->xml, "  <bIndexSubType>%u</bIndexSubType>\n", bIndexSubType);
            fprintf(avi->xml, "  <bIndexType>%u</bIndexType>\n", bIndexType);
            fprintf(avi->xml, "  <nEntriesInUse>%u</nEntriesInUse>\n", nEntriesInUse);
            fprintf(avi->xml, "  <dwChunkId>%s</dwChunkId>\n", getFccString_le(dwChunkId, fcc));
            fprintf(avi->xml, "  </a>\n");
        }

        // Index specifics code
        ////////////////////////////////////////////////////////////////////////

        if (bIndexType == AVI_INDEX_OF_INDEXES)
        {
            TRACE_INFO(AVI, BLD_GREEN "> AVI Super Index Chunk" CLR_RESET);

            uint32_t dwReserved[3];
            dwReserved[0] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[1] = endian_flip_32(read_bits(bitstr, 32));
            dwReserved[2] = endian_flip_32(read_bits(bitstr, 32));

            // Parse super index entries
            unsigned int i = 0;
            for (i = 0; i < nEntriesInUse; i++)
            {
                track->superindex_count++;
                track->superindex_entries[i].offset = endian_flip_64(read_bits_64(bitstr, 64));
                track->superindex_entries[i].size = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwDuration = endian_flip_32(read_bits(bitstr, 32));

                TRACE_1(AVI, " > qwOffset   = %lli", track->superindex_entries[i].offset);
                TRACE_1(AVI, "  > dwSize    : %lli", track->superindex_entries[i].size);
                TRACE_1(AVI, "  > dwDuration: %u", dwDuration);
            }
        }
        else if (bIndexType == AVI_INDEX_OF_CHUNKS)
        {
            // AVI Standard Index Chunk // AVI Field Index Chunk
            TRACE_INFO(AVI, BLD_GREEN "> AVI Standard Index Chunk" CLR_RESET);

            // Alloc index entries
            int start = 0;
            if (track->index_count == 0)
            {
                track->index_count = nEntriesInUse;
                track->index_entries = (AviIndexEntries_t*)calloc(track->index_count, sizeof(AviIndexEntries_t));
            }
            else
            {
                start = track->index_count;
                track->index_count += nEntriesInUse;
                track->index_entries = (AviIndexEntries_t*)realloc(track->index_entries, track->index_count * sizeof(AviIndexEntries_t));
            }

            // Parse index entries
            int64_t qwOffset_base = endian_flip_64(read_bits_64(bitstr, 64));
            /*uint32_t dwReserved3 =*/ endian_flip_32(read_bits(bitstr, 32));

            TRACE_1(AVI, " > qwOffset_base  : %lli", qwOffset_base);

            unsigned int i = 0;
            for (i = start; i < track->index_count; i++)
            {
                uint32_t dwOffset = endian_flip_32(read_bits(bitstr, 32));
                uint32_t dwSize = endian_flip_32(read_bits(bitstr, 32));

                track->index_entries[i].offset = qwOffset_base + dwOffset;
                track->index_entries[i].size = dwSize & 0x7FFFFFFF;
                track->index_entries[i].pts = -1;

                if ((dwSize & 0x10000000) == 0)
                    track->index_entries[i].flags = AVIIF_KEYFRAME;

                TRACE_1(AVI, "  > dwOffset  = %lli", track->index_entries[i].offset);
                TRACE_1(AVI, "   > dwSize   : %lli", track->index_entries[i].size);
            }
        }
        else if (bIndexType == AVI_INDEX_IS_DATA)
        {
            TRACE_WARNING(AVI, "AVI_INDEX_IS_DATA index > unsupported");
            retcode = FAILURE;
        }
        else
        {
            TRACE_ERROR(AVI, "Unknown type of index: fatal error");
            retcode = FAILURE;
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

int avi_indexer_initmap(MediaFile_t *media, AviTrack_t *track, uint32_t index_entry_count)
{
    // Init a MediaStream_t for each avi track
    int retcode = SUCCESS;
    MediaStream_t *mytrack = NULL;

    if (index_entry_count == 0)
        index_entry_count = 1;

    if (track->strh.fccType == fcc_auds)
    {
        // Audio track
        retcode = init_bitstream_map(&media->tracks_audio[media->tracks_audio_count], 0, index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_audio[media->tracks_audio_count];
            media->tracks_audio_count++;

            mytrack->stream_type = stream_AUDIO;
            //mytrack->stream_fcc  = 0;
            //mytrack->stream_tcc  = track->strf.wFormatTag;

            // We cannot rely on the fccHandler from the container,
            // but we should have extracted the correct codec infos from strh
            mytrack->stream_codec = (Codecs_e)track->strh.fccHandler;

            mytrack->stream_packetized = true;

            mytrack->bitrate_avg = track->strf.wBitsPerSample;
            mytrack->sampling_rate = track->strf.nSamplesPerSec;
            mytrack->channel_count = track->strf.nChannels;

            //mytrack->frame_rate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // But do not trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_vids)
    {
        // Video track
        retcode = init_bitstream_map(&media->tracks_video[media->tracks_video_count], 0, index_entry_count);

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_video[media->tracks_video_count];
            media->tracks_video_count++;

            mytrack->stream_type = stream_VIDEO;
            if (track->strh.fccHandler)
                mytrack->stream_fcc  = track->strh.fccHandler;
            else if (track->strf.biCompression)
                mytrack->stream_fcc  = track->strf.biCompression;

            mytrack->stream_packetized = true;

            mytrack->width = track->strf.biWidth;
            mytrack->height = track->strf.biHeight;
            mytrack->color_depth = track->strf.biBitCount;

            mytrack->framerate = (double)(track->strh.dwRate) / (double)(track->strh.dwScale);
            //mytrack->frame_rate = avi->avih.dwMicroSecPerFrame; // do NOT trust dwMicroSecPerFrame
        }
    }
    else if (track->strh.fccType == fcc_txts)
    {
        // Subtitles track
        retcode = init_bitstream_map(&media->tracks_subt[media->tracks_subtitles_count], 0, index_entry_count);
        media->tracks_subtitles_count++;

        if (retcode == SUCCESS)
        {
            mytrack = media->tracks_video[media->tracks_video_count];

            mytrack->stream_type  = stream_TEXT;
            mytrack->stream_fcc   = track->strh.fccHandler;
            mytrack->stream_codec = CODEC_SRT;

            mytrack->stream_packetized = true;

            mytrack->subtitles_encoding = 0;
            //strncpy(mytrack->language_code, "UND", 3);
            mytrack->subtitles_name = NULL;
        }
    }
    else
    {
        TRACE_WARNING(AVI, "Unknown track type");
        retcode = FAILURE;
    }

    return retcode;
}

/* ************************************************************************** */

int avi_indexer(Bitstream_t *bitstr, MediaFile_t *media, avi_t *avi)
{
    TRACE_INFO(AVI, BLD_GREEN "avi_indexer()" CLR_RESET);
    int retcode = SUCCESS;

    for (unsigned i = 0; i < avi->tracks_count; i++)
    {
        if (avi->tracks[i]->track_indexed == 1)
        {
            TRACE_1(AVI, "> track already indexed");
        }
        else
        {
            for (unsigned j = 0; j < avi->tracks[i]->superindex_count; j++)
            {
                // IX offset
                bitstream_goto_offset(bitstr, avi->tracks[i]->superindex_entries[j].offset);

                // IX header
                RiffChunk_t ix_chunk;
                parse_chunk_header(bitstr, &ix_chunk);
                print_chunk_header(&ix_chunk);

                // IX content
                parse_indx(bitstr, &ix_chunk, avi, avi->tracks[i]);
            }

            // Convert index into a bitstream map
            retcode = avi_indexer_initmap(media, avi->tracks[i], avi->tracks[i]->index_count);

            if (retcode == SUCCESS)
            {
                // Set sample into bitstream map
                unsigned int tid = 0; // only support 1 audio and 1 video track for now

                if (avi->tracks[i]->strh.fccType == fcc_auds)
                {
                    for (unsigned k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = media->tracks_audio[tid]->sample_count;
                        media->tracks_audio[tid]->sample_count++;

                        media->tracks_audio[tid]->sample_type[sid] = sample_AUDIO;
                        media->tracks_audio[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        media->tracks_audio[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        media->tracks_audio[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        media->tracks_audio[tid]->sample_pts[sid] = -1;
                    }
                }
                else if (avi->tracks[i]->strh.fccType == fcc_vids)
                {
                    for (unsigned k = 0; k < avi->tracks[i]->index_count; k++)
                    {
                        int sid = media->tracks_video[tid]->sample_count;
                        media->tracks_video[tid]->sample_count++;

                        if (avi->tracks[i]->index_entries[i].flags == AVIIF_KEYFRAME)
                            media->tracks_video[tid]->sample_type[sid] = sample_VIDEO_SYNC;
                        else
                            media->tracks_video[tid]->sample_type[sid] = sample_VIDEO;

                        media->tracks_video[tid]->sample_offset[sid] = avi->tracks[i]->index_entries[k].offset;
                        media->tracks_video[tid]->sample_size[sid] = avi->tracks[i]->index_entries[k].size;
                        media->tracks_video[tid]->sample_dts[sid] = avi->tracks[i]->index_entries[k].pts;
                        media->tracks_video[tid]->sample_pts[sid] = -1;
                    }
                }
                else if (avi->tracks[i]->strh.fccType == fcc_txts)
                {
                    //
                }
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Free the allocated content of a avi_t structure.
 * \param *avi A pointer to a avi_t structure.
 */
void avi_clean(avi_t *avi)
{
    if (avi)
    {
        unsigned int i = 0;
        for (i = 0; i < avi->tracks_count; i++)
        {
            if (avi->tracks[i])
            {
                free(avi->tracks[i]->index_entries);
                free(avi->tracks[i]);
            }
        }
    }
}

/* ************************************************************************** */
