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
 * \file      h264.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2010
 */

// minivideo headers
#include "h264.h"
#include "h264_parameterset.h"
#include "h264_slice.h"
#include "h264_macroblock.h"
#include "h264_transform.h"
#include "../../depacketizer/depack.h"
#include "../../export.h"
#include "../../bitstream_utils.h"
#include "../../minivideo_typedef.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstring>
#include <cstdlib>

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *dc The current DecodingContext.
 * \return 0 if failed, 1 otherwise.
 *
 * Compute some values needed during the quantization of the coefficients
 * contained in the bitstream.
 */
static int computeNormAdjust(DecodingContext_t *dc)
{
    TRACE_2(TRANS, "  > " BLD_GREEN "computeNormAdjust()" CLR_RESET);
    int retcode = FAILURE;

    if (dc != NULL)
    {
        int q = 0, i = 0, j = 0;

        int v4x4[6][3] =
        {
            {10, 16, 13},
            {11, 18, 14},
            {13, 20, 16},
            {14, 23, 18},
            {16, 25, 20},
            {18, 29, 23},
        };

        int v8x8[6][6] =
        {
            {20, 18, 32, 19, 25, 24},
            {22, 19, 35, 21, 28, 26},
            {26, 23, 42, 24, 33, 31},
            {28, 25, 45, 26, 35, 33},
            {32, 28, 51, 30, 40, 38},
            {36, 32, 58, 34, 46, 43},
        };

        for (q = 0; q < 6; q++)
        {
            // Compute complete normAdjust4x4[] table
            for (i = 0; i < 4; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    if ((i % 2 == 0) && (j % 2 == 0))
                        dc->normAdjust4x4[q][i][j] = v4x4[q][0];
                    else if ((i % 2 == 1) && (j % 2 == 1))
                        dc->normAdjust4x4[q][i][j] = v4x4[q][1];
                    else
                        dc->normAdjust4x4[q][i][j] = v4x4[q][2];

                    //TRACE_3(TRANS, "normAdjust4x4[%i][%i][%i] = %i", q, ii, jj, normAdjust4x4[q][ii][jj]);
                }
            }

            // Compute complete normAdjust8x8[] table
            for (i = 0; i < 8; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if ((i % 4 == 0) && (j % 4 == 0))
                        dc->normAdjust8x8[q][i][j] = v8x8[q][0];
                    else if ((i % 2 ==  1) && (j % 2 == 1))
                        dc->normAdjust8x8[q][i][j] = v8x8[q][1];
                    else if ((i % 4 == 2) && (j % 4 == 2))
                        dc->normAdjust8x8[q][i][j] = v8x8[q][2];
                    else if (((i % 4 == 0) && (j % 2 == 1)) || ((i % 2 == 1) && (j % 4 == 0)))
                        dc->normAdjust8x8[q][i][j] = v8x8[q][3];
                    else if (((i % 4 == 0) && (j % 4 == 2)) || ((i % 4 == 2) && (j % 4 == 0)))
                        dc->normAdjust8x8[q][i][j] = v8x8[q][4];
                    else
                        dc->normAdjust8x8[q][i][j] = v8x8[q][5];

                    //TRACE_3(TRANS, "normAdjust8x8[%i][%i][%i] = %i", q, ii, jj, normAdjust8x8[q][ii][jj]);
                }
            }
        }

        retcode = SUCCESS;
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \param *media: A pointer to a MediaFile_t structure, containing every informations available about the current media file.
 * \param tid: Track ID.
 * \return A pointer to the newlee allocated DecodingContext.
 *
 * Initialize the DecodingContext and it's bitstream (with a MediaFile_t passed in parameters),
 * then NAL Unit structure, then init all pointer's (sps, pps, sei) to NULL.
 */
DecodingContext_t *initDecodingContext(MediaFile_t *media, unsigned tid)
{
    TRACE_INFO(H264, BLD_GREEN "initDecodingContext()" CLR_RESET);
    DecodingContext_t *dc = NULL;

    if (media != NULL &&
        tid < media->tracks_video_count &&
        media->tracks_video[tid] != NULL)
    {
        // DecodingContext allocation
        dc = (DecodingContext_t*)calloc(1, sizeof(DecodingContext_t));

        if (dc == NULL)
        {
            TRACE_ERROR(H264, "Unable to alloc new DecodingContext!");
        }
        else
        {
            // Select media and track
            dc->MediaFile = media;
            dc->VideoStream = media->tracks_video[tid];
            dc->active_tid = tid;

            // Init some quantization parameters
            computeNormAdjust(dc);

            // Init input bitstream
            dc->bitstr = init_bitstream(media, media->tracks_video[tid]);

            if (dc->bitstr == NULL)
            {
                free(dc);
                dc = NULL;
            }
            else
            {
                // Init NAL Unit
                dc->active_nalu = initNALU();

                if (dc->active_nalu == NULL)
                {
                    free(dc->bitstr);
                    dc->bitstr = NULL;

                    free(dc);
                    dc = NULL;
                }
            }
        }
    }

    // Return the DecodingContext
    return dc;
}

/* ************************************************************************** */

/*!
 * \param *dc The DecodingContext structure we want to check.
 * \return The error code.
 *
 * This function aim to the validity of DecodingContext.
 * If any of the following checks failed, we can assume that something prior to
 * this function call had gone wrong and that we cannot safely pursue decoding.
 */
int checkDecodingContext(DecodingContext_t *dc)
{
    TRACE_INFO(H264, "> " BLD_GREEN "checkDecodingContext()" CLR_RESET);
    int retcode = SUCCESS;

    if (dc->bitstr == NULL)
    {
        TRACE_WARNING(H264, "* Bitstream reader structure is invalid!");
        retcode = FAILURE;
    }

    if (dc->active_nalu == NULL)
    {
        TRACE_WARNING(H264, "* NAL Unit structure is invalid!");
        retcode = FAILURE;
    }

    if (dc->sps_array[dc->active_sps] == NULL)
    {
        TRACE_WARNING(H264, "* SPS structure currently in use is invalid!");
        retcode = FAILURE;
    }

    if (dc->pps_array[dc->active_pps] == NULL)
    {
        TRACE_WARNING(H264, "* PPS structure currently in use is invalid!");
        retcode = FAILURE;
    }

    if (dc->active_slice == NULL)
    {
        TRACE_WARNING(H264, "* Slice structure is invalid!");
        retcode = FAILURE;
    }
    else
    {
        pps_t *pps = dc->pps_array[dc->active_slice->pic_parameter_set_id];
        if (pps == NULL)
        {
            TRACE_WARNING(H264, "* The slice structure refer to an invalid PPS!");
            retcode = FAILURE;
        }
        else
        {
            sps_t *sps = dc->sps_array[pps->seq_parameter_set_id];
            if (sps == NULL)
            {
                TRACE_WARNING(H264, "* The slice structure refer to an invalid SPS!");
                retcode = FAILURE;
            }
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \param **dc_ptr The decoding context to freed.
 *
 * The MediaFile_t is not freed by this call, only the DecodingContext_t and it's attached content:
 * - Bitstream reader.
 * - Active NAL Unit.
 * - All existing SPS.
 * - All existing PPS.
 * - Active SEI.
 * - Active Slice.
 * - All existing macroblocks.
 */
void freeDecodingContext(DecodingContext_t **dc_ptr)
{
    TRACE_INFO(H264, BLD_GREEN "freeDecodingContext()" CLR_RESET);

    // free DecodingContext content
    if ((*dc_ptr) != NULL)
    {
        int i = 0;
        free_bitstream(&(*dc_ptr)->bitstr);

        if ((*dc_ptr)->active_nalu != NULL)
        {
            free((*dc_ptr)->active_nalu);
            (*dc_ptr)->active_nalu = NULL;

            TRACE_1(H264, ">> NAL Unit freed");
        }

        for (i = 0; i < MAX_SPS; i++)
        {
            freeSPS(&(*dc_ptr)->sps_array[i]);
        }

        for (i = 0; i < MAX_PPS; i++)
        {
            freePPS(&(*dc_ptr)->pps_array[i]);
        }

        freeSEI(&(*dc_ptr)->active_sei);

        free_slice(&(*dc_ptr)->active_slice);

        freeMbArray(*dc_ptr);

        free(*dc_ptr);
        *dc_ptr = NULL;
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

int h264_decode_nalu(DecodingContext_t *dc, const int64_t nalu_offset, const int64_t nalu_size)
{
    TRACE_1(H264, BLD_GREEN "h264_decode_nalu()" CLR_RESET);
    int retcode = FAILURE;

    // Goto correct data offset
    //bitstream_goto_offset(dc->bitstr, nalu_offset);
    buffer_feed_manual(dc->bitstr, nalu_offset, nalu_size);

    // Check header validity
    if (nalu_parse_header(dc->bitstr, dc->active_nalu))
    {
        TRACE_1(H264, "decode: " BLD_GREEN "NALU_TYPE %i (at %lli) " CLR_RESET,
                dc->active_nalu->nal_unit_type,
                bitstream_get_absolute_byte_offset(dc->bitstr));

        // Decode NAL Unit content
        switch (dc->active_nalu->nal_unit_type)
        {
            case NALU_TYPE_SLICE: //////////////////////////////////////
            {
                TRACE_1(H264, "This decoder only support IDR slice decoding!");
            }
            break;

            case NALU_TYPE_IDR: ////////////////////////////////////////
            {
                TRACE_INFO(H264, "> " BLD_GREEN "decodeIDR(%i at %lli)" CLR_RESET,
                           dc->idrCounter, bitstream_get_absolute_byte_offset(dc->bitstr));

                nalu_clean_sample(dc->bitstr);
                dc->IdrPicFlag = true;

                if (decode_slice(dc))
                {
                    retcode = SUCCESS;
                    dc->errorCounter = 0;
                    dc->idrCounter++;
                    dc->frameCounter++;
                }
                else
                    dc->errorCounter++;
            }
            break;

            case NALU_TYPE_AUD: ////////////////////////////////////////
            {
                nalu_clean_sample(dc->bitstr);

                aud_t aud;
                if (decodeAUD(dc->bitstr, &aud))
                {
                    retcode = SUCCESS;
                }
                else
                    dc->errorCounter++;
            }
            break;

            case NALU_TYPE_SEI: ////////////////////////////////////////
            {
                nalu_clean_sample(dc->bitstr);

                if (dc->active_sei != NULL)
                    free(dc->active_sei);

                dc->active_sei = (sei_t*)calloc(1, sizeof(sei_t));
                if (dc->active_sei)
                {
                    if (decodeSEI(dc->bitstr, dc->active_sei))
                    {
                        retcode = SUCCESS;
                        printSEI(NULL);
                    }
                    else
                        dc->errorCounter++;
                }
            }
            break;

            case NALU_TYPE_SPS: ////////////////////////////////////////
            {
                nalu_clean_sample(dc->bitstr);

                retcode = decodeSPS_legacy(dc);
/*
                sps_t *sps = (sps_t*)calloc(1, sizeof(sps_t));
                if (sps)
                {
                    if (decodeSPS(dc->bitstr, sps))
                    {
                        dc->sps_array[sps->seq_parameter_set_id] = sps;

                        dc->active_sps = sps->seq_parameter_set_id;
                        dc->profile_idc = sps->profile_idc;
                        dc->ChromaArrayType = sps->ChromaArrayType;

                        // Init some quantization tables
                        computeLevelScale4x4(dc, sps);
                        computeLevelScale8x8(dc, sps);

                        // Macroblocks "table" allocation (on macroblock **mbs_data):
                        dc->PicSizeInMbs = sps->FrameHeightInMbs * sps->PicWidthInMbs;
                        dc->mb_array = (Macroblock_t**)calloc(dc->PicSizeInMbs, sizeof(Macroblock_t*));

                        //printSPS(sps);
                        retcode = SUCCESS;
                    }
                    else
                        dc->errorCounter++;
                }
*/
            }
            break;

            case NALU_TYPE_PPS: ////////////////////////////////////////
            {
                nalu_clean_sample(dc->bitstr);

                pps_t *pps = (pps_t*)calloc(1, sizeof(pps_t));
                if (pps)
                {
                    if (decodePPS(dc->bitstr, pps, dc->sps_array))
                    {
                        dc->pps_array[pps->pic_parameter_set_id] = pps;
                        dc->active_pps = pps->pic_parameter_set_id;
                        dc->entropy_coding_mode_flag = pps->entropy_coding_mode_flag,

                        //printPPS(pps, dc->sps_array);
                        retcode = SUCCESS;
                    }
                    else
                        dc->errorCounter++;
                }
            }
            break;

            default:
            {
                TRACE_ERROR(NALU, "Unsupported NAL Unit! (nal_unit_type %i)", dc->active_nalu->nal_unit_type);
                dc->errorCounter++;
            }
            break;
        }

        // Reset NAL Unit structure
        nalu_reset(dc->active_nalu);
    }
    else
    {
        retcode = FAILURE;
        dc->errorCounter++;
        TRACE_WARNING(NALU, "No valid NAL Unit to decode! (errorCounter = %i)", dc->errorCounter);
    }

    return retcode;
}

/* ************************************************************************** */

DecodingContext_t *h264_init(MediaFile_t *input_video, unsigned tid)
{
    TRACE_INFO(H264, BLD_GREEN "h264_init()" CLR_RESET);

    DecodingContext_t *dc = NULL;

    if (input_video == NULL)
    {
        TRACE_ERROR(H264, "MediaFile_t is NULL, exiting decoder");
    }
    else
    {
        // Init decoding context
        dc = initDecodingContext(input_video, tid);

        if (dc == NULL)
        {
            TRACE_ERROR(H264, "Unable to allocate DecodingContext_t, exiting decoder");
        }
        else
        {
            int error_count = 0;

            // Start the decoding process
            dc->decoderRunning = true;

            // Load "parameter sets"
            for (unsigned pid = 0; pid < dc->MediaFile->tracks_video[tid]->parameter_count; pid++)
            {
                int64_t samplesoffset = dc->MediaFile->tracks_video[tid]->parameter_offset[pid];
                int64_t samplesize = dc->MediaFile->tracks_video[tid]->parameter_size[pid];

                if (buffer_feed_manual(dc->bitstr, samplesoffset-1, samplesize+1) != SUCCESS)
                    error_count++;
                else
                    if (h264_decode_nalu(dc, samplesoffset, samplesize) != SUCCESS)
                        error_count++;
            }

            if (error_count != 0)
            {
                TRACE_ERROR(H264, "%i errors happened when decoding SPS and PPS content!", error_count);

                // Stop the decoding process and free resources
                dc->decoderRunning = false;
                freeDecodingContext(&dc);
            }
        }
    }

    return dc;
}

int h264_decode(DecodingContext_t *dc, unsigned sid)
{
    TRACE_INFO(H264, BLD_GREEN "h264_decode()" CLR_RESET);
    int retcode = SUCCESS;

    if (!dc)
        return FAILURE;

    // Loop until the end of the decoding
    while (dc->decoderRunning && retcode == SUCCESS)
    {
        buffer_feed_manual(dc->bitstr,
                           dc->MediaFile->tracks_video[dc->active_tid]->sample_offset[sid],
                           dc->MediaFile->tracks_video[dc->active_tid]->sample_size[sid]);

        // Depacketize
        es_sample_t essample_list[16];
        int essample_count = depack_loaded_sample(dc->bitstr, dc->MediaFile,
                                                  dc->MediaFile->tracks_video[dc->active_tid],
                                                  sid, essample_list);

        if (essample_count < 1)
        {
            retcode = FAILURE;
            dc->decoderRunning = false;
            break;
        }

        for (int i = 0; i < essample_count && dc->decoderRunning; i++)
        {
            int64_t current_nalu_offset = essample_list[i].offset;
            int64_t current_nalu_size = essample_list[i].size;
            retcode = h264_decode_nalu(dc, current_nalu_offset, current_nalu_size);

            if (dc->idrCounter == 1 && retcode == SUCCESS)
            {
                TRACE_INFO(H264, ">> " BLD_YELLOW "Decoding of %i IDR successfull!" CLR_RESET, dc->idrCounter);
                TRACE_INFO(H264, "H.264 decoding ended");
                retcode = SUCCESS;
                dc->decoderRunning = false;
            }
/*
            if (dc->errorCounter > 64 || retcode == FAILURE)
            {
                TRACE_ERROR(H264, "Error inside NAL Unit decoding loop! (errorCounter = %i) (current nal_unit_type = %i)", dc->errorCounter, dc->active_nalu->nal_unit_type);
                TRACE_ERROR(H264, "H.264 decoding aborted");
                retcode = FAILURE;
                dc->decoderRunning = false;
            }
*/
        }
    }

    return retcode;
}

int h264_export_file(DecodingContext_t *dc, OutputFile_t *out)
{
    TRACE_INFO(H264, BLD_GREEN "h264_export_file()" CLR_RESET);
    int retcode = FAILURE;

    if (dc)
    {
        // Export IDR slice
        if (dc->IdrPicFlag == true)
        {
            TRACE_INFO(H264, BLD_GREEN "h264_export_file()" CLR_RESET);

            retcode = export_idr_file(dc, out);
        }

        // Free slice
        //free_slice(&dc->active_slice);
    }

    return retcode;
}

int h264_export_surface(DecodingContext_t *dc, OutputSurface_t *out)
{
    TRACE_INFO(H264, BLD_GREEN "h264_export_surface()" CLR_RESET);
    int retcode = FAILURE;

    if (dc)
    {
        // Export IDR slice
        if (dc->IdrPicFlag == true)
        {
            TRACE_INFO(H264, BLD_GREEN "h264_export_file()" CLR_RESET);

            retcode = export_idr_surface(dc, out);
        }

        // Free slice
        //free_slice(&dc->active_slice);
    }

    return retcode;
}

void h264_cleanup(DecodingContext_t *dc)
{
    TRACE_INFO(H264, BLD_GREEN "h264_cleanup()" CLR_RESET);
    freeDecodingContext(&dc);
}

/* ************************************************************************** */
