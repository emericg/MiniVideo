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
 * \file      h265_parameterset.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "h265_parameterset.h"
#include "h265_expgolomb.h"
#include "../../demuxer/xml_mapper.h"
#include "../../minitraces.h"
#include "../../minivideo_typedef.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cinttypes>

/* ************************************************************************** */
/* ************************************************************************** */

void profile_tier_level(Bitstream_t *bitstr, h265_ptl_t *ptl,
                        bool profilePresentFlag, uint8_t maxNumSubLayersMinus1)
{
    if (!ptl)
    {
        TRACE_ERROR(PARAM, "NULL PTL!");
        return;
    }

    if (profilePresentFlag)
    {
        ptl->general_profile_space = read_bits(bitstr, 2);
        ptl->general_tier_flag = read_bit(bitstr);
        ptl->general_profile_idc = read_bits(bitstr, 5);

        for (int j = 0; j < 32; j++)
        {
            ptl->general_profile_compatibility_flag[j] = read_bit(bitstr);
        }

        ptl->general_progressive_source_flag = read_bit(bitstr);
        ptl->general_interlaced_source_flag = read_bit(bitstr);
        ptl->general_non_packed_constraint_flag = read_bit(bitstr);
        ptl->general_frame_only_constraint_flag = read_bit(bitstr);

        if (ptl->general_profile_idc ==  4 || ptl->general_profile_compatibility_flag[ 4] ||
            ptl->general_profile_idc ==  5 || ptl->general_profile_compatibility_flag[ 5] ||
            ptl->general_profile_idc ==  6 || ptl->general_profile_compatibility_flag[ 6] ||
            ptl->general_profile_idc ==  7 || ptl->general_profile_compatibility_flag[ 7] ||
            ptl->general_profile_idc ==  8 || ptl->general_profile_compatibility_flag[ 8] ||
            ptl->general_profile_idc ==  9 || ptl->general_profile_compatibility_flag[ 9] ||
            ptl->general_profile_idc == 10 || ptl->general_profile_compatibility_flag[10] ||
            ptl->general_profile_idc == 11 || ptl->general_profile_compatibility_flag[11] )
        {
            ptl->general_max_12bit_constraint_flag = read_bit(bitstr);
            ptl->general_max_10bit_constraint_flag = read_bit(bitstr);
            ptl->general_max_8bit_constraint_flag = read_bit(bitstr);
            ptl->general_max_422chroma_constraint_flag = read_bit(bitstr);
            ptl->general_max_420chroma_constraint_flag = read_bit(bitstr);
            ptl->general_max_monochrome_constraint_flag = read_bit(bitstr);
            ptl->general_intra_constraint_flag = read_bit(bitstr);
            ptl->general_one_picture_only_constraint_flag = read_bit(bitstr);
            ptl->general_lower_bit_rate_constraint_flag = read_bit(bitstr);

            if (ptl->general_profile_idc ==  5 || ptl->general_profile_compatibility_flag[ 5] ||
                ptl->general_profile_idc ==  9 || ptl->general_profile_compatibility_flag[ 9] ||
                ptl->general_profile_idc == 10 || ptl->general_profile_compatibility_flag[10] ||
                ptl->general_profile_idc == 11 || ptl->general_profile_compatibility_flag[11])
            {
                ptl->general_max_14bit_constraint_flag = read_bit(bitstr);
                skip_bits(bitstr, 32);
                skip_bits(bitstr, 1);
            }
            else
            {
                skip_bits(bitstr, 32);
                skip_bits(bitstr, 2);
            }
        }
        else if (ptl->general_profile_idc == 2 || ptl->general_profile_compatibility_flag[2])
        {
            skip_bits(bitstr, 7);
            ptl->general_one_picture_only_constraint_flag = read_bit(bitstr);
            skip_bits(bitstr, 32);
            skip_bits(bitstr, 3);
        }
        else
        {
            skip_bits(bitstr, 32);
            skip_bits(bitstr, 11);
        }

        if (ptl->general_profile_idc ==  1 || ptl->general_profile_compatibility_flag[ 1] ||
            ptl->general_profile_idc ==  2 || ptl->general_profile_compatibility_flag[ 2] ||
            ptl->general_profile_idc ==  3 || ptl->general_profile_compatibility_flag[ 3] ||
            ptl->general_profile_idc ==  4 || ptl->general_profile_compatibility_flag[ 4] ||
            ptl->general_profile_idc ==  5 || ptl->general_profile_compatibility_flag[ 5] ||
            ptl->general_profile_idc ==  9 || ptl->general_profile_compatibility_flag[ 9] ||
            ptl->general_profile_idc == 11 || ptl->general_profile_compatibility_flag[11])
        {
            ptl->general_inbld_flag = read_bit(bitstr);
        }
        else
        {
            skip_bits(bitstr, 1);
        }
    }

    ptl->general_level_idc = read_bits(bitstr, 8);

    // sub layer
    if (maxNumSubLayersMinus1 > 0)
    {
        //ptl->sub_layer = (h265_ptl_sublayer_t**)calloc(maxNumSubLayersMinus1, sizeof(h265_ptl_sublayer_t*));
        ptl->sub_layer = (h265_ptl_sublayer_t*)calloc(maxNumSubLayersMinus1, sizeof(h265_ptl_sublayer_t));

        for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
        {
            //ptl->sub_layer[i] = (h265_ptl_sublayer_t*)calloc(maxNumSubLayersMinus1, sizeof(h265_ptl_sublayer_t));

            ptl->sub_layer[i].profile_present_flag = read_bit(bitstr);
            ptl->sub_layer[i].level_present_flag = read_bit(bitstr);
        }

        for (unsigned i = maxNumSubLayersMinus1; i < 8; i++)
        {
            skip_bits(bitstr, 2);
        }

        for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
        {
            if (ptl->sub_layer[i].profile_present_flag)
            {
                ptl->sub_layer[i].profile_space = read_bits(bitstr, 2);
                ptl->sub_layer[i].tier_flag = read_bit(bitstr);
                ptl->sub_layer[i].profile_idc = read_bits(bitstr, 5);

                for (unsigned j = 0; j < 32; j++)
                {
                    ptl->sub_layer[i].profile_compatibility_flag[j] = read_bit(bitstr);
                }

                ptl->sub_layer[i].progressive_source_flag = read_bit(bitstr);
                ptl->sub_layer[i].interlaced_source_flag = read_bit(bitstr);
                ptl->sub_layer[i].non_packed_constraint_flag = read_bit(bitstr);
                ptl->sub_layer[i].frame_only_constraint_flag = read_bit(bitstr);

                if (ptl->sub_layer[i].profile_idc ==  4 || ptl->sub_layer[i].profile_compatibility_flag[ 4] ||
                    ptl->sub_layer[i].profile_idc ==  5 || ptl->sub_layer[i].profile_compatibility_flag[ 5] ||
                    ptl->sub_layer[i].profile_idc ==  6 || ptl->sub_layer[i].profile_compatibility_flag[ 6] ||
                    ptl->sub_layer[i].profile_idc ==  7 || ptl->sub_layer[i].profile_compatibility_flag[ 7] ||
                    ptl->sub_layer[i].profile_idc ==  8 || ptl->sub_layer[i].profile_compatibility_flag[ 8] ||
                    ptl->sub_layer[i].profile_idc ==  9 || ptl->sub_layer[i].profile_compatibility_flag[ 9] ||
                    ptl->sub_layer[i].profile_idc == 10 || ptl->sub_layer[i].profile_compatibility_flag[10] ||
                    ptl->sub_layer[i].profile_idc == 11 || ptl->sub_layer[i].profile_compatibility_flag[11] )
                {
                    ptl->sub_layer[i].max_12bit_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].max_10bit_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].max_8bit_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].max_422chroma_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].max_420chroma_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].max_monochrome_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].intra_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].one_picture_only_constraint_flag = read_bit(bitstr);
                    ptl->sub_layer[i].lower_bit_rate_constraint_flag = read_bit(bitstr);

                    if (ptl->sub_layer[i].profile_idc ==  5 || ptl->sub_layer[i].profile_compatibility_flag[ 5] ||
                        ptl->sub_layer[i].profile_idc ==  9 || ptl->sub_layer[i].profile_compatibility_flag[ 9] ||
                        ptl->sub_layer[i].profile_idc == 10 || ptl->sub_layer[i].profile_compatibility_flag[10] ||
                        ptl->sub_layer[i].profile_idc == 11 || ptl->sub_layer[i].profile_compatibility_flag[11])
                    {
                        ptl->sub_layer[i].max_14bit_constraint_flag = read_bit(bitstr);
                        skip_bits(bitstr, 32);
                        skip_bits(bitstr, 1);
                    }
                    else
                    {
                        skip_bits(bitstr, 32);
                        skip_bits(bitstr, 2);
                    }
                }
                else if (ptl->sub_layer[i].profile_idc == 2 || ptl->sub_layer[i].profile_compatibility_flag[2])
                {
                    skip_bits(bitstr, 7);
                    ptl->sub_layer[i].one_picture_only_constraint_flag = read_bit(bitstr);
                    skip_bits(bitstr, 32);
                    skip_bits(bitstr, 3);
                }
                else
                {
                    skip_bits(bitstr, 32);
                    skip_bits(bitstr, 11);
                }

                if (ptl->sub_layer[i].profile_idc ==  1 || ptl->sub_layer[i].profile_compatibility_flag[ 1] ||
                    ptl->sub_layer[i].profile_idc ==  2 || ptl->sub_layer[i].profile_compatibility_flag[ 2] ||
                    ptl->sub_layer[i].profile_idc ==  3 || ptl->sub_layer[i].profile_compatibility_flag[ 3] ||
                    ptl->sub_layer[i].profile_idc ==  4 || ptl->sub_layer[i].profile_compatibility_flag[ 4] ||
                    ptl->sub_layer[i].profile_idc ==  5 || ptl->sub_layer[i].profile_compatibility_flag[ 5] ||
                    ptl->sub_layer[i].profile_idc ==  9 || ptl->sub_layer[i].profile_compatibility_flag[ 9] ||
                    ptl->sub_layer[i].profile_idc == 11 || ptl->sub_layer[i].profile_compatibility_flag[11])
                {
                    ptl->sub_layer[i].inbld_flag = read_bit(bitstr);
                }
                else
                {
                    skip_bits(bitstr, 1);
                }
            }

            if (ptl->sub_layer[i].level_present_flag)
            {
                ptl->sub_layer[i].level_idc = read_bits(bitstr, 8);
            }
        }
    }
}

void map_ptl(h265_ptl_t *ptl, int64_t offset, int64_t size, FILE *xml)
{
    if (!ptl || !xml) return;

    fprintf(xml, "  <a tt=\"PTL\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
            offset, size);

    xmlSpacer(xml, "Profile, tier and level", -1);

    fprintf(xml, "  <general_profile_space>%u</general_profile_space>\n", ptl->general_profile_space);
    fprintf(xml, "  <general_tier_flag>%u</general_tier_flag>\n", ptl->general_tier_flag);
    fprintf(xml, "  <general_profile_idc>%u</general_profile_idc>\n", ptl->general_profile_idc);
    for (int j = 0; j < 32; j++)
    {
        fprintf(xml, "  <general_profile_compatibility_flag index=\"%u\">%u</general_profile_compatibility_flag>\n", j, ptl->general_profile_compatibility_flag[j]);
    }
    fprintf(xml, "  <general_progressive_source_flag>%u</general_progressive_source_flag>\n", ptl->general_progressive_source_flag);
    fprintf(xml, "  <general_interlaced_source_flag>%u</general_interlaced_source_flag>\n", ptl->general_interlaced_source_flag);
    fprintf(xml, "  <general_non_packed_constraint_flag>%u</general_non_packed_constraint_flag>\n", ptl->general_non_packed_constraint_flag);
    fprintf(xml, "  <general_frame_only_constraint_flag>%u</general_frame_only_constraint_flag>\n", ptl->general_frame_only_constraint_flag);

    fprintf(xml, "  <general_max_14bit_constraint_flag>%u</general_max_14bit_constraint_flag>\n", ptl->general_max_14bit_constraint_flag);
    fprintf(xml, "  <general_max_12bit_constraint_flag>%u</general_max_12bit_constraint_flag>\n", ptl->general_max_12bit_constraint_flag);
    fprintf(xml, "  <general_max_10bit_constraint_flag>%u</general_max_10bit_constraint_flag>\n", ptl->general_max_10bit_constraint_flag);
    fprintf(xml, "  <general_max_8bit_constraint_flag>%u</general_max_8bit_constraint_flag>\n", ptl->general_max_8bit_constraint_flag);
    fprintf(xml, "  <general_max_422chroma_constraint_flag>%u</general_max_422chroma_constraint_flag>\n", ptl->general_max_422chroma_constraint_flag);
    fprintf(xml, "  <general_max_420chroma_constraint_flag>%u</general_max_420chroma_constraint_flag>\n", ptl->general_max_420chroma_constraint_flag);
    fprintf(xml, "  <general_max_monochrome_constraint_flag>%u</general_max_monochrome_constraint_flag>\n", ptl->general_max_monochrome_constraint_flag);
    fprintf(xml, "  <general_intra_constraint_flag>%u</general_intra_constraint_flag>\n", ptl->general_intra_constraint_flag);
    fprintf(xml, "  <general_one_picture_only_constraint_flag>%u</general_one_picture_only_constraint_flag>\n", ptl->general_one_picture_only_constraint_flag);
    fprintf(xml, "  <general_lower_bit_rate_constraint_flag>%u</general_lower_bit_rate_constraint_flag>\n", ptl->general_lower_bit_rate_constraint_flag);

    fprintf(xml, "  <general_inbld_flag>%u</general_inbld_flag>\n", ptl->general_inbld_flag);

    fprintf(xml, "  <general_level_idc>%u</general_level_idc>\n", ptl->general_level_idc);

    fprintf(xml, "  </a>\n");
}

void hrd_parameters(Bitstream_t *bitstr, h265_hrd_t *hrd,
                     bool profilePresentFlag, uint8_t maxNumSubLayersMinus1)
{
    if (!hrd)
    {
        TRACE_ERROR(PARAM, "NULL HRD!");
        return;
    }
/*
    if (profilePresentFlag)
    {
        hrd->nal_hrd_parameters_present_flag = read_bit(bitstr);
        hrd->vcl_hrd_parameters_present_flag = read_bit(bitstr);

        if (hrd->nal_hrd_parameters_present_flag ||
            hrd->vcl_hrd_parameters_present_flag)
        {
            hrd->sub_pic_hrd_params_present_flag = read_bit(bitstr);
            if (hrd->sub_pic_hrd_params_present_flag)
            {
                    tick_divisor_minus2
                        du_cpb_removal_delay_increment_length_minus1
                            sub_pic_cpb_params_in_pic_timing_sei_flag
                                dpb_output_delay_du_length_minus1
                }
            bit_rate_scale
                cpb_size_scale
                if( sub_pic_hrd_params_present_flag )
                cpb_size_du_scale
                    initial_cpb_removal_delay_length_minus1
                        au_cpb_removal_delay_length_minus1
                            dpb_output_delay_length_minus1
        }
    }*/
}

void hrd_sublayer_parameters(Bitstream_t *bitstr, h265_hrd_t *hrd,
                             h265_hrd_t *sublayer_hrd, int subLayerId)
{
    if (!sublayer_hrd)
    {
        TRACE_ERROR(PARAM, "NULL SUB LAYER HRD!");
        return;
    }
/*
    for (i = 0; i < CpbCnt; i++)
    {
        bit_rate_value_minus1[i] = h265_read_ue(bitstr);
        cpb_size_value_minus1[i] = h265_read_ue(bitstr);
        if (sub_pic_hrd_params_present_flag)
        {
            cpb_size_du_value_minus1[i] = h265_read_ue(bitstr);
            bit_rate_du_value_minus1[i] = h265_read_ue(bitstr);
        }
        cbr_flag[i] = read_bit(bitstr);
    }
*/
}

void map_hrd(h265_hrd_t *hrd, int64_t offset, int64_t size, FILE *xml)
{
    if (!hrd || !xml) return;

    fprintf(xml, "  <a tt=\"HRD\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
            offset, size);

    xmlSpacer(xml, "Hypotetical Reference Decoder", -1);

    fprintf(xml, "  <nal_hrd_parameters_present_flag>%u</nal_hrd_parameters_present_flag>\n", hrd->nal_hrd_parameters_present_flag);
    fprintf(xml, "  <vcl_hrd_parameters_present_flag>%u</vcl_hrd_parameters_present_flag>\n", hrd->vcl_hrd_parameters_present_flag);

    fprintf(xml, "  </a>\n");
}

/* ************************************************************************** */
/* ************************************************************************** */

int h265_decodeVPS(Bitstream_t *bitstr, h265_vps_t *vps)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "h265_decodeVPS()" CLR_RESET);
    int retcode = SUCCESS;

    if (!vps)
    {
        TRACE_ERROR(PARAM, "NULL VPS!");
        return FAILURE;
    }

    vps->vps_video_parameter_set_id = read_bits(bitstr, 4);
    vps->vps_base_layer_internal_flag = read_bit(bitstr);
    vps->vps_base_layer_available_flag = read_bit(bitstr);
    vps->vps_max_layers_minus1 = read_bits(bitstr, 6);
    vps->vps_max_sub_layers_minus1 = read_bits(bitstr, 3);
    vps->vps_temporal_id_nesting_flag = read_bit(bitstr);

    skip_bits(bitstr, 16); // vps_reserved_0xffff_16bits

    profile_tier_level(bitstr, &vps->ptl,
                       true, vps->vps_max_sub_layers_minus1);

    vps->vps_sub_layer_ordering_info_present_flag = read_bit(bitstr);
    for (unsigned i = (vps->vps_sub_layer_ordering_info_present_flag ? 0 : vps->vps_max_sub_layers_minus1);
         i <= vps->vps_max_sub_layers_minus1; i++)
    {
        vps->vps_max_dec_pic_buffering_minus1[i] = h265_read_ue(bitstr);
        vps->vps_max_num_reorder_pics[i] = h265_read_ue(bitstr);
        vps->vps_max_latency_increase_plus1[i] = h265_read_ue(bitstr);
    }

    vps->vps_max_layer_id = read_bits(bitstr, 6);
    vps->vps_num_layer_sets_minus1 = h265_read_ue(bitstr);
    vps->layer_id_included_flag = (bool **)calloc(vps->vps_num_layer_sets_minus1, sizeof(bool*));
    for (unsigned i = 1; i <= vps->vps_num_layer_sets_minus1; i++)
    {
        vps->layer_id_included_flag[i] = (bool *)calloc(vps->vps_max_layer_id, sizeof(bool));
        for (unsigned j = 0; j <= vps->vps_max_layer_id; j++)
        {
            vps->layer_id_included_flag[i][j] = read_bit(bitstr);
        }
    }

    vps->vps_timing_info_present_flag = read_bit(bitstr);
    if (vps->vps_timing_info_present_flag)
    {
        vps->vps_num_units_in_tick = read_bits(bitstr, 32);
        vps->vps_time_scale = read_bits(bitstr, 32);
        vps->vps_poc_proportional_to_timing_flag = read_bit(bitstr);
        if (vps->vps_poc_proportional_to_timing_flag)
        {
            vps->vps_num_ticks_poc_diff_one_minus1 = h265_read_ue(bitstr);
        }

        vps->vps_num_hrd_parameters = h265_read_ue(bitstr);
        vps->hrd_layer_set_idx = (uint32_t*)calloc(vps->vps_num_hrd_parameters, sizeof(uint32_t));
        vps->cprms_present_flag = (bool *)calloc(vps->vps_num_hrd_parameters, sizeof(bool));
        vps->hrd = (h265_hrd_t *)calloc(vps->vps_num_hrd_parameters, sizeof(h265_hrd_t));

        for (unsigned i = 0; i < vps->vps_num_hrd_parameters; i++)
        {
            vps->hrd_layer_set_idx[i] = h265_read_ue(bitstr);
            if (i > 0)
            {
                vps->cprms_present_flag[i] = read_bit(bitstr);
            }

            hrd_parameters(bitstr, &vps->hrd[i],
                           vps->cprms_present_flag[i], vps->vps_max_sub_layers_minus1);
        }
    }

    vps->vps_extension_flag = read_bit(bitstr);
    if (vps->vps_extension_flag)
    {
        //while (h264_more_rbsp_data())
        //{
        //    vps->vps_extension_data_flag = read_bit(bitstr);
        //}
    }

    return retcode;
}

/* ************************************************************************** */

void h265_mapVPS(h265_vps_t *vps, int64_t offset, int64_t size, FILE *xml)
{
    if (!vps || !xml) return;

    fprintf(xml, "  <a tt=\"VPS\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
            offset, size);

    xmlSpacer(xml, "Video Parameter Set", -1);

    fprintf(xml, "  <vps_video_parameter_set_id>%u</vps_video_parameter_set_id>\n", vps->vps_video_parameter_set_id);
    fprintf(xml, "  <vps_base_layer_internal_flag>%u</vps_base_layer_internal_flag>\n", vps->vps_base_layer_internal_flag);
    fprintf(xml, "  <vps_base_layer_available_flag>%u</vps_base_layer_available_flag>\n", vps->vps_base_layer_available_flag);
    fprintf(xml, "  <vps_max_layers_minus1>%u</vps_max_layers_minus1>\n", vps->vps_max_layers_minus1);
    fprintf(xml, "  <vps_max_sub_layers_minus1>%u</vps_max_sub_layers_minus1>\n", vps->vps_max_sub_layers_minus1);
    fprintf(xml, "  <vps_temporal_id_nesting_flag>%u</vps_temporal_id_nesting_flag>\n", vps->vps_temporal_id_nesting_flag);

    xmlSpacer(xml, "Profile, tier and level >>", -1);
    map_ptl(&vps->ptl, offset+1, size-1, xml);

    fprintf(xml, "  <vps_sub_layer_ordering_info_present_flag>%u</vps_sub_layer_ordering_info_present_flag>\n", vps->vps_sub_layer_ordering_info_present_flag);
    for (unsigned i = (vps->vps_sub_layer_ordering_info_present_flag ? 0 : vps->vps_max_sub_layers_minus1);
         i <= vps->vps_max_sub_layers_minus1; i++)
    {
        fprintf(xml, "  <vps_max_dec_pic_buffering_minus1 index=\"%u\">%u</vps_max_dec_pic_buffering_minus1>\n", i, vps->vps_max_dec_pic_buffering_minus1[i]);
        fprintf(xml, "  <vps_max_num_reorder_pics index=\"%u\">%u</vps_max_num_reorder_pics>\n", i, vps->vps_max_num_reorder_pics[i]);
        fprintf(xml, "  <vps_max_latency_increase_plus1 index=\"%u\">%u</vps_max_latency_increase_plus1>\n", i, vps->vps_max_latency_increase_plus1[i]);
    }

    fprintf(xml, "  <vps_max_layer_id>%u</vps_max_layer_id>\n", vps->vps_max_layer_id);
    fprintf(xml, "  <vps_num_layer_sets_minus1>%u</vps_num_layer_sets_minus1>\n", vps->vps_num_layer_sets_minus1);
    for (unsigned i = 1; i <= vps->vps_num_layer_sets_minus1; i++)
    {
        for (unsigned j = 0; j <= vps->vps_max_layer_id; j++)
        {
            //fprintf(xml, "  <layer_id_included_flag[%u][%u]>%u</vps_num_layer_sets_minus1>\n",
            //        i, j, vps->layer_id_included_flag[i][j]);
        }
    }

    fprintf(xml, "  <vps_timing_info_present_flag>%u</vps_timing_info_present_flag>\n", vps->vps_timing_info_present_flag);
    if (vps->vps_timing_info_present_flag)
    {
        fprintf(xml, "  <vps_num_units_in_tick>%u</vps_num_units_in_tick>\n", vps->vps_num_units_in_tick);
        fprintf(xml, "  <vps_time_scale>%u</vps_time_scale>\n", vps->vps_time_scale);
        fprintf(xml, "  <vps_poc_proportional_to_timing_flag>%u</vps_poc_proportional_to_timing_flag>\n", vps->vps_poc_proportional_to_timing_flag);

        if (vps->vps_poc_proportional_to_timing_flag)
        {
            fprintf(xml, "  <vps_num_ticks_poc_diff_one_minus1>%u</vps_num_ticks_poc_diff_one_minus1>\n", vps->vps_num_ticks_poc_diff_one_minus1);
        }

        fprintf(xml, "  <vps_num_hrd_parameters>%u</vps_num_hrd_parameters>\n", vps->vps_num_hrd_parameters);
        for (unsigned i = 0; i < vps->vps_num_hrd_parameters; i++)
        {
            fprintf(xml, "  <hrd_layer_set_idx index=\"%u\">%u</hrd_layer_set_idx>\n", i, vps->hrd_layer_set_idx[i]);
            if (i > 0)
            {
                fprintf(xml, "  <cprms_present_flag index=\"%u\">%u</cprms_present_flag>\n", i, vps->cprms_present_flag[i]);
            }

            xmlSpacer(xml, "Hypotetical Reference Decoder >>", -1);
            //map_hrd(&vps->ptl, offset+1, size-1, xml);
        }
    }

    fprintf(xml, "  <vps_extension_flag>%u</vps_extension_flag>\n", vps->vps_extension_flag);
    fprintf(xml, "  <vps_extension_data_flag>%u</vps_extension_data_flag>\n", vps->vps_extension_data_flag);

    fprintf(xml, "  </a>\n");
}

/* ************************************************************************** */
/* ************************************************************************** */

int h265_decodeSPS(Bitstream_t *bitstr, h265_sps_t *sps)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "h265_decodeSPS()" CLR_RESET);
    int retcode = SUCCESS;

    if (!sps)
    {
        TRACE_ERROR(PARAM, "NULL SPS!");
        return FAILURE;
    }

    sps->sps_video_parameter_set_id = read_bits(bitstr, 4);
    sps->sps_max_sub_layers_minus1 = read_bits(bitstr, 3);
    sps->sps_temporal_id_nesting_flag = read_bit(bitstr);

    profile_tier_level(bitstr, &sps->ptl,
                       true, sps->sps_max_sub_layers_minus1);

    sps->sps_seq_parameter_set_id = h265_read_ue(bitstr);
    sps->chroma_format_idc = h265_read_ue(bitstr);

    if (sps->chroma_format_idc == 3)
    {
        sps->separate_colour_plane_flag = read_bit(bitstr);
    }

    sps->pic_width_in_luma_samples = h265_read_ue(bitstr);
    sps->pic_height_in_luma_samples = h265_read_ue(bitstr);

    sps->conformance_window_flag = read_bit(bitstr);
    if (sps->conformance_window_flag)
    {
        sps->conf_win_left_offset = h265_read_ue(bitstr);
        sps->conf_win_right_offset = h265_read_ue(bitstr);
        sps->conf_win_top_offset = h265_read_ue(bitstr);
        sps->conf_win_bottom_offset = h265_read_ue(bitstr);
    }

    sps->bit_depth_luma_minus8 = h265_read_ue(bitstr);
    sps->bit_depth_chroma_minus8 = h265_read_ue(bitstr);
    sps->log2_max_pic_order_cnt_lsb_minus4 = h265_read_ue(bitstr);
    sps->sps_sub_layer_ordering_info_present_flag = read_bit(bitstr);

    sps->sps_max_dec_pic_buffering_minus1 = (uint32_t*)calloc(sps->sps_max_sub_layers_minus1, sizeof(uint32_t));
    sps->sps_max_num_reorder_pics = (uint32_t*)calloc(sps->sps_max_sub_layers_minus1, sizeof(uint32_t));
    sps->sps_max_latency_increase_plus1 = (uint32_t*)calloc(sps->sps_max_sub_layers_minus1, sizeof(uint32_t));

    for (int i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : sps->sps_max_sub_layers_minus1);
         i <= sps->sps_max_sub_layers_minus1; i++)
    {
        sps->sps_max_dec_pic_buffering_minus1[i] = h265_read_ue(bitstr);
        sps->sps_max_num_reorder_pics[i] = h265_read_ue(bitstr);
        sps->sps_max_latency_increase_plus1[i] = h265_read_ue(bitstr);
    }

    sps->log2_min_luma_coding_block_size_minus3 = h265_read_ue(bitstr);
    sps->log2_diff_max_min_luma_coding_block_size = h265_read_ue(bitstr);
    sps->log2_min_luma_transform_block_size_minus2 = h265_read_ue(bitstr);
    sps->log2_diff_max_min_luma_transform_block_size = h265_read_ue(bitstr);
    sps->max_transform_hierarchy_depth_inter = h265_read_ue(bitstr);
    sps->max_transform_hierarchy_depth_intra = h265_read_ue(bitstr);

    sps->scaling_list_enabled_flag = read_bit(bitstr);
    if (sps->scaling_list_enabled_flag)
    {
        sps->sps_scaling_list_data_present_flag = read_bit(bitstr);
        if (sps->sps_scaling_list_data_present_flag)
        {
            //scaling_list_data() // TODO
        }
    }

    sps->amp_enabled_flag = read_bit(bitstr);
    sps->sample_adaptive_offset_enabled_flag = read_bit(bitstr);
    sps->pcm_enabled_flag = read_bit(bitstr);
    if (sps->pcm_enabled_flag)
    {
    sps->pcm_sample_bit_depth_luma_minus1 = read_bits(bitstr, 4);
    sps->pcm_sample_bit_depth_chroma_minus1 = read_bits(bitstr, 4);
    sps->log2_min_pcm_luma_coding_block_size_minus3 = h265_read_ue(bitstr);
    sps->log2_diff_max_min_pcm_luma_coding_block_size = h265_read_ue(bitstr);
    sps->pcm_loop_filter_disabled_flag = read_bit(bitstr);
    }

    sps->num_short_term_ref_pic_sets = h265_read_ue(bitstr);
    for (unsigned i = 0; i < sps->num_short_term_ref_pic_sets; i++)
    {
        //st_ref_pic_set(i) // TODO
    }

    sps->long_term_ref_pics_present_flag = read_bit(bitstr);
    if (sps->long_term_ref_pics_present_flag)
    {
        sps->num_long_term_ref_pics_sps = h265_read_ue(bitstr);

        for (unsigned i = 0; i < sps->num_long_term_ref_pics_sps; i++)
        {
            //TODO
        }
    }

    sps->sps_temporal_mvp_enabled_flag = read_bit(bitstr);
    sps->strong_intra_smoothing_enabled_flag = read_bit(bitstr);

    sps->vui_parameters_present_flag = read_bit(bitstr);
    if (sps->vui_parameters_present_flag)
    {
        //vui_parameters() // TODO
    }

    sps->sps_extension_present_flag = read_bit(bitstr);
    if (sps->sps_extension_present_flag)
    {
        sps->sps_range_extension_flag = read_bit(bitstr);
        sps->sps_multilayer_extension_flag = read_bit(bitstr);
        sps->sps_3d_extension_flag = read_bit(bitstr);
        sps->sps_scc_extension_flag = read_bit(bitstr);
        sps->sps_extension_4bits = read_bits(bitstr, 4);
    }

    if (sps->sps_range_extension_flag)
    {
        //sps_range_extension() // TODO
    }
    if (sps->sps_multilayer_extension_flag)
    {
        //sps_multilayer_extension() // TODO
    }
    if (sps->sps_3d_extension_flag)
    {
        //sps_3d_extension() // TODO
    }
    if (sps->sps_scc_extension_flag)
    {
        //sps_scc_extension() // TODO
    }
    if (sps->sps_extension_4bits)
    {
        //while (h264_more_rbsp_data())
        //{
        //    sps->sps_extension_data_flag = read_bit(bitstr);
        //}
    }

    return retcode;
}

/* ************************************************************************** */

void h265_mapSPS(h265_sps_t *sps, int64_t offset, int64_t size, FILE *xml)
{
    if (!sps || !xml) return;

    fprintf(xml, "  <a tt=\"SPS\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
            offset, size);

    xmlSpacer(xml, "Sequence Parameter Set", -1);

    fprintf(xml, "  <sps_video_parameter_set_id>%u</sps_video_parameter_set_id>\n", sps->sps_video_parameter_set_id);
    fprintf(xml, "  <sps_max_sub_layers_minus1>%u</sps_max_sub_layers_minus1>\n", sps->sps_max_sub_layers_minus1);
    fprintf(xml, "  <sps_temporal_id_nesting_flag>%u</sps_temporal_id_nesting_flag>\n", sps->sps_temporal_id_nesting_flag);

    xmlSpacer(xml, "Profile, tier and level >>", -1);
    map_ptl(&sps->ptl, offset+1, size-1, xml);

    fprintf(xml, "  <sps_seq_parameter_set_id>%u</sps_seq_parameter_set_id>\n", sps->sps_seq_parameter_set_id);

    fprintf(xml, "  <chroma_format_idc>%u</chroma_format_idc>\n", sps->chroma_format_idc);
    if (sps->chroma_format_idc == 3)
        fprintf(xml, "  <separate_colour_plane_flag>%u</separate_colour_plane_flag>\n", sps->separate_colour_plane_flag);

    fprintf(xml, "  <pic_width_in_luma_samples>%u</pic_width_in_luma_samples>\n", sps->pic_width_in_luma_samples);
    fprintf(xml, "  <pic_height_in_luma_samples>%u</pic_height_in_luma_samples>\n", sps->pic_height_in_luma_samples);

    fprintf(xml, "  <conformance_window_flag>%u</conformance_window_flag>\n", sps->conformance_window_flag);
    if (sps->conformance_window_flag)
    {
        fprintf(xml, "  <conf_win_left_offset>%u</conf_win_left_offset>\n", sps->conf_win_left_offset);
        fprintf(xml, "  <conf_win_right_offset>%u</conf_win_right_offset>\n", sps->conf_win_right_offset);
        fprintf(xml, "  <conf_win_top_offset>%u</conf_win_top_offset>\n", sps->conf_win_top_offset);
        fprintf(xml, "  <conf_win_bottom_offset>%u</conf_win_bottom_offset>\n", sps->conf_win_bottom_offset);
    }

    fprintf(xml, "  <bit_depth_luma_minus8>%u</bit_depth_luma_minus8>\n", sps->bit_depth_luma_minus8);
    fprintf(xml, "  <bit_depth_chroma_minus8>%u</bit_depth_chroma_minus8>\n", sps->bit_depth_chroma_minus8);
    fprintf(xml, "  <log2_max_pic_order_cnt_lsb_minus4>%u</log2_max_pic_order_cnt_lsb_minus4>\n", sps->log2_max_pic_order_cnt_lsb_minus4);
    fprintf(xml, "  <sps_sub_layer_ordering_info_present_flag>%u</sps_sub_layer_ordering_info_present_flag>\n", sps->sps_sub_layer_ordering_info_present_flag);

    for (int i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : sps->sps_max_sub_layers_minus1);
         i <= sps->sps_max_sub_layers_minus1; i++)
    {
        fprintf(xml, "  <sps_max_dec_pic_buffering_minus1 index=\"%u\">%u</sps_max_dec_pic_buffering_minus1>\n", i, sps->sps_max_dec_pic_buffering_minus1[i]);
        fprintf(xml, "  <sps_max_num_reorder_pics index=\"%u\">%u</sps_max_num_reorder_pics>\n", i, sps->sps_max_num_reorder_pics[i]);
        fprintf(xml, "  <sps_max_latency_increase_plus1 index=\"%u\">%u</sps_max_latency_increase_plus1>\n", i, sps->sps_max_latency_increase_plus1[i]);
    }

    fprintf(xml, "  <log2_min_luma_coding_block_size_minus3>%u</log2_min_luma_coding_block_size_minus3>\n", sps->log2_min_luma_coding_block_size_minus3);
    fprintf(xml, "  <log2_diff_max_min_luma_coding_block_size>%u</log2_diff_max_min_luma_coding_block_size>\n", sps->log2_diff_max_min_luma_coding_block_size);
    fprintf(xml, "  <log2_min_luma_transform_block_size_minus2>%u</log2_min_luma_transform_block_size_minus2>\n", sps->log2_min_luma_transform_block_size_minus2);
    fprintf(xml, "  <log2_diff_max_min_luma_transform_block_size>%u</log2_diff_max_min_luma_transform_block_size>\n", sps->log2_diff_max_min_luma_transform_block_size);
    fprintf(xml, "  <max_transform_hierarchy_depth_inter>%u</max_transform_hierarchy_depth_inter>\n", sps->max_transform_hierarchy_depth_inter);
    fprintf(xml, "  <max_transform_hierarchy_depth_intra>%u</max_transform_hierarchy_depth_intra>\n", sps->max_transform_hierarchy_depth_intra);

    fprintf(xml, "  <scaling_list_enabled_flag>%u</scaling_list_enabled_flag>\n", sps->scaling_list_enabled_flag);
    if (sps->scaling_list_enabled_flag)
    {
        fprintf(xml, "  <sps_scaling_list_data_present_flag>%u</sps_scaling_list_data_present_flag>\n", sps->sps_scaling_list_data_present_flag);

        if (sps->sps_scaling_list_data_present_flag)
        {
            xmlSpacer(xml, "scaling_list_data >>", -1);
            //scaling_list_data() // TODO
        }
    }

    fprintf(xml, "  <amp_enabled_flag>%u</amp_enabled_flag>\n", sps->amp_enabled_flag);
    fprintf(xml, "  <sample_adaptive_offset_enabled_flag>%u</sample_adaptive_offset_enabled_flag>\n", sps->sample_adaptive_offset_enabled_flag);
    fprintf(xml, "  <pcm_enabled_flag>%u</pcm_enabled_flag>\n", sps->pcm_enabled_flag);

    if (sps->pcm_enabled_flag)
    {
        fprintf(xml, "  <pcm_sample_bit_depth_luma_minus1>%u</pcm_sample_bit_depth_luma_minus1>\n", sps->pcm_sample_bit_depth_luma_minus1);
        fprintf(xml, "  <pcm_sample_bit_depth_chroma_minus1>%u</pcm_sample_bit_depth_chroma_minus1>\n", sps->pcm_sample_bit_depth_chroma_minus1);
        fprintf(xml, "  <log2_min_pcm_luma_coding_block_size_minus3>%u</log2_min_pcm_luma_coding_block_size_minus3>\n", sps->log2_min_pcm_luma_coding_block_size_minus3);
        fprintf(xml, "  <log2_diff_max_min_pcm_luma_coding_block_size>%u</log2_diff_max_min_pcm_luma_coding_block_size>\n", sps->log2_diff_max_min_pcm_luma_coding_block_size);
        fprintf(xml, "  <pcm_loop_filter_disabled_flag>%u</pcm_loop_filter_disabled_flag>\n", sps->pcm_loop_filter_disabled_flag);
    }

    fprintf(xml, "  <num_short_term_ref_pic_sets>%u</num_short_term_ref_pic_sets>\n", sps->num_short_term_ref_pic_sets);
    for (unsigned i = 0; i < sps->num_short_term_ref_pic_sets; i++)
    {
        //st_ref_pic_set(i) // TODO
    }

    fprintf(xml, "  <long_term_ref_pics_present_flag>%u</long_term_ref_pics_present_flag>\n", sps->long_term_ref_pics_present_flag);
    if (sps->long_term_ref_pics_present_flag)
    {
        fprintf(xml, "  <num_long_term_ref_pics_sps>%u</num_long_term_ref_pics_sps>\n", sps->num_long_term_ref_pics_sps);
        for (unsigned i = 0; i < sps->num_long_term_ref_pics_sps; i++)
        {
            //fprintf(xml, "  <lt_ref_pic_poc_lsb_sps index=\"%u\">%u</lt_ref_pic_poc_lsb_sps>\n", i, sps->lt_ref_pic_poc_lsb_sps[i]);
            //fprintf(xml, "  <used_by_curr_pic_lt_sps_flag index=\"%u\">%u</used_by_curr_pic_lt_sps_flag>\n", i, sps->used_by_curr_pic_lt_sps_flag[i]);
        }
    }

    fprintf(xml, "  <sps_temporal_mvp_enabled_flag>%u</sps_temporal_mvp_enabled_flag>\n", sps->sps_temporal_mvp_enabled_flag);
    fprintf(xml, "  <strong_intra_smoothing_enabled_flag>%u</strong_intra_smoothing_enabled_flag>\n", sps->strong_intra_smoothing_enabled_flag);

    fprintf(xml, "  <vui_parameters_present_flag>%u</vui_parameters_present_flag>\n", sps->vui_parameters_present_flag);
    if (sps->vui_parameters_present_flag)
    {
        xmlSpacer(xml, "vui_parameters >>", -1);
        //vui_parameters() // TODO
    }

    fprintf(xml, "  <sps_extension_present_flag>%u</sps_extension_present_flag>\n", sps->sps_extension_present_flag);
    if (sps->sps_extension_present_flag)
    {
        fprintf(xml, "  <sps_range_extension_flag>%u</sps_range_extension_flag>\n", sps->sps_range_extension_flag);
        fprintf(xml, "  <sps_multilayer_extension_flag>%u</sps_multilayer_extension_flag>\n", sps->sps_multilayer_extension_flag);
        fprintf(xml, "  <sps_3d_extension_flag>%u</sps_3d_extension_flag>\n", sps->sps_3d_extension_flag);
        fprintf(xml, "  <sps_scc_extension_flag>%u</sps_scc_extension_flag>\n", sps->sps_scc_extension_flag);
        fprintf(xml, "  <sps_extension_4bits>%u</sps_extension_4bits>\n", sps->sps_extension_4bits);
    }

    fprintf(xml, "  </a>\n");
}

/* ************************************************************************** */
/* ************************************************************************** */

int h265_decodePPS(Bitstream_t *bitstr, h265_pps_t *pps, h265_sps_t **sps_array)
{
    TRACE_INFO(PARAM, "<> " BLD_GREEN "h265_decodePPS()" CLR_RESET);
    int retcode = SUCCESS;

    if (!pps)
    {
        TRACE_ERROR(PARAM, "NULL PPS!");
        return FAILURE;
    }

    pps->pps_pic_parameter_set_id = h265_read_ue(bitstr);
    pps->pps_seq_parameter_set_id = h265_read_ue(bitstr);
    pps->dependent_slice_segments_enabled_flag = read_bit(bitstr);
    pps->output_flag_present_flag = read_bit(bitstr);
    pps->num_extra_slice_header_bits = read_bits(bitstr, 3);
    pps->sign_data_hiding_enabled_flag = read_bit(bitstr);
    pps->cabac_init_present_flag = read_bit(bitstr);
    pps->num_ref_idx_l0_default_active_minus1 = h265_read_ue(bitstr);
    pps->num_ref_idx_l1_default_active_minus1 = h265_read_ue(bitstr);
    pps->init_qp_minus26 = h265_read_se(bitstr);
    pps->constrained_intra_pred_flag = read_bit(bitstr);
    pps->transform_skip_enabled_flag = read_bit(bitstr);
    pps->cu_qp_delta_enabled_flag = read_bit(bitstr);
    if (pps->cu_qp_delta_enabled_flag)
    {
        pps->diff_cu_qp_delta_depth = h265_read_ue(bitstr);
    }

    pps->pps_cb_qp_offset = h265_read_se(bitstr);
    pps->pps_cr_qp_offset = h265_read_se(bitstr);
    pps->pps_slice_chroma_qp_offsets_present_flag = read_bit(bitstr);
    pps->weighted_pred_flag = read_bit(bitstr);
    pps->weighted_bipred_flag = read_bit(bitstr);
    pps->transquant_bypass_enabled_flag = read_bit(bitstr);
    pps->tiles_enabled_flag = read_bit(bitstr);
    pps->entropy_coding_sync_enabled_flag = read_bit(bitstr);
    if (pps->tiles_enabled_flag)
    {
        pps->num_tile_columns_minus1 = h265_read_ue(bitstr);
        pps->num_tile_rows_minus1 = h265_read_ue(bitstr);
        pps->uniform_spacing_flag = h265_read_ue(bitstr);
        if (!pps->uniform_spacing_flag)
        {
            pps->column_width_minus1 = (uint32_t *)calloc(pps->num_tile_columns_minus1, sizeof(uint32_t));
            pps->row_height_minus1 = (uint32_t *)calloc(pps->num_tile_columns_minus1, sizeof(uint32_t));

            for (unsigned i = 0; i < pps->num_tile_columns_minus1; i++)
            {
                pps->column_width_minus1[i] = h265_read_ue(bitstr);
            }
            for (unsigned i = 0; i < pps->num_tile_rows_minus1; i++)
            {
                pps->row_height_minus1[i] = h265_read_ue(bitstr);
            }
        }

        pps->loop_filter_across_tiles_enabled_flag = read_bit(bitstr);
    }

    pps->pps_loop_filter_across_slices_enabled_flag = read_bit(bitstr);
    pps->deblocking_filter_control_present_flag = read_bit(bitstr);
    if (pps->deblocking_filter_control_present_flag)
    {
        pps->deblocking_filter_override_enabled_flag = read_bit(bitstr);
        pps->pps_deblocking_filter_disabled_flag = read_bit(bitstr);
        if (!pps->pps_deblocking_filter_disabled_flag)
        {
            pps->pps_beta_offset_div2 = h265_read_se(bitstr);
            pps->pps_tc_offset_div2 = h265_read_se(bitstr);
        }
    }

    pps->pps_scaling_list_data_present_flag = read_bit(bitstr);
    if (pps->pps_scaling_list_data_present_flag)
    {
        //scaling_list_data() // TODO
    }

    pps->lists_modification_present_flag = read_bit(bitstr);
    pps->log2_parallel_merge_level_minus2 = h265_read_ue(bitstr);
    pps->slice_segment_header_extension_present_flag = read_bit(bitstr);

    pps->pps_extension_present_flag = read_bit(bitstr);
    if (pps->pps_extension_present_flag)
    {
        pps->pps_range_extension_flag = read_bit(bitstr);
        pps->pps_multilayer_extension_flag = read_bit(bitstr);
        pps->pps_3d_extension_flag = read_bit(bitstr);
        pps->pps_scc_extension_flag = read_bit(bitstr);
        pps->pps_extension_4bits = read_bits(bitstr, 4);
    }

    if (pps->pps_range_extension_flag)
    {
        //pps->pps_range_extension() // TODO
    }
    if (pps->pps_multilayer_extension_flag)
    {
        //pps_multilayer_extension() // TODO // specified in Annex F
    }
    if (pps->pps_3d_extension_flag)
    {
        //pps_3d_extension() // TODO // specified in Annex I
    }
    if (pps->pps_scc_extension_flag)
    {
        //pps_scc_extension() // TODO
    }
    if (pps->pps_extension_4bits)
    {
        //while (h264_more_rbsp_data())
        //{
        //    pps->pps_extension_data_flag = read_bit(bitstr);
        //}
    }

    return retcode;
}

/* ************************************************************************** */

void h265_mapPPS(h265_pps_t *pps, int64_t offset, int64_t size, FILE *xml)
{
    if (!pps || !xml) return;

    fprintf(xml, "  <a tt=\"PPS\" add=\"private\" tp=\"data\" off=\"%" PRId64 "\" sz=\"%" PRId64 "\">\n",
            offset, size);

    xmlSpacer(xml, "Picture Parameter Set", -1);

    fprintf(xml, "  <pps_pic_parameter_set_id>%u</pps_pic_parameter_set_id>\n", pps->pps_pic_parameter_set_id);
    fprintf(xml, "  <pps_seq_parameter_set_id>%u</pps_seq_parameter_set_id>\n", pps->pps_seq_parameter_set_id);
    fprintf(xml, "  <dependent_slice_segments_enabled_flag>%u</dependent_slice_segments_enabled_flag>\n", pps->dependent_slice_segments_enabled_flag);
    fprintf(xml, "  <output_flag_present_flag>%u</output_flag_present_flag>\n", pps->output_flag_present_flag);
    fprintf(xml, "  <num_extra_slice_header_bits>%u</num_extra_slice_header_bits>\n", pps->num_extra_slice_header_bits);
    fprintf(xml, "  <sign_data_hiding_enabled_flag>%u</sign_data_hiding_enabled_flag>\n", pps->sign_data_hiding_enabled_flag);
    fprintf(xml, "  <cabac_init_present_flag>%u</cabac_init_present_flag>\n", pps->cabac_init_present_flag);
    fprintf(xml, "  <num_ref_idx_l0_default_active_minus1>%u</num_ref_idx_l0_default_active_minus1>\n", pps->num_ref_idx_l0_default_active_minus1);
    fprintf(xml, "  <num_ref_idx_l1_default_active_minus1>%u</num_ref_idx_l1_default_active_minus1>\n", pps->num_ref_idx_l1_default_active_minus1);
    fprintf(xml, "  <init_qp_minus26>%i</init_qp_minus26>\n", pps->init_qp_minus26);
    fprintf(xml, "  <constrained_intra_pred_flag>%u</constrained_intra_pred_flag>\n", pps->constrained_intra_pred_flag);
    fprintf(xml, "  <transform_skip_enabled_flag>%u</transform_skip_enabled_flag>\n", pps->transform_skip_enabled_flag);
    fprintf(xml, "  <cu_qp_delta_enabled_flag>%u</cu_qp_delta_enabled_flag>\n", pps->cu_qp_delta_enabled_flag);
    if (pps->cu_qp_delta_enabled_flag)
    {
        fprintf(xml, "  <diff_cu_qp_delta_depth>%u</diff_cu_qp_delta_depth>\n", pps->diff_cu_qp_delta_depth);
    }

    fprintf(xml, "  <pps_cb_qp_offset>%i</pps_cb_qp_offset>\n", pps->pps_cb_qp_offset);
    fprintf(xml, "  <pps_cr_qp_offset>%i</pps_cr_qp_offset>\n", pps->pps_cr_qp_offset);
    fprintf(xml, "  <pps_slice_chroma_qp_offsets_present_flag>%u</pps_slice_chroma_qp_offsets_present_flag>\n", pps->pps_slice_chroma_qp_offsets_present_flag);
    fprintf(xml, "  <weighted_pred_flag>%u</weighted_pred_flag>\n", pps->weighted_pred_flag);
    fprintf(xml, "  <weighted_bipred_flag>%u</weighted_bipred_flag>\n", pps->weighted_bipred_flag);
    fprintf(xml, "  <transquant_bypass_enabled_flag>%u</transquant_bypass_enabled_flag>\n", pps->transquant_bypass_enabled_flag);
    fprintf(xml, "  <tiles_enabled_flag>%u</tiles_enabled_flag>\n", pps->tiles_enabled_flag);
    fprintf(xml, "  <entropy_coding_sync_enabled_flag>%u</entropy_coding_sync_enabled_flag>\n", pps->entropy_coding_sync_enabled_flag);
    if (pps->tiles_enabled_flag)
    {
        fprintf(xml, "  <num_tile_columns_minus1>%u</num_tile_columns_minus1>\n", pps->num_tile_columns_minus1);
        fprintf(xml, "  <num_tile_rows_minus1>%u</num_tile_rows_minus1>\n", pps->num_tile_rows_minus1);
        fprintf(xml, "  <uniform_spacing_flag>%u</uniform_spacing_flag>\n", pps->uniform_spacing_flag);

        if (!pps->uniform_spacing_flag)
        {
            for (unsigned i = 0; i < pps->num_tile_columns_minus1; i++)
            {
                fprintf(xml, "  <column_width_minus1 index=\"%u\">%u</column_width_minus1>\n", i, pps->column_width_minus1[i]);
            }
            for (unsigned i = 0; i < pps->num_tile_rows_minus1; i++)
            {
                fprintf(xml, "  <row_height_minus1 index=\"%u\">%u</row_height_minus1>\n", i, pps->row_height_minus1[i]);
            }
        }

        fprintf(xml, "  <loop_filter_across_tiles_enabled_flag>%u</loop_filter_across_tiles_enabled_flag>\n", pps->loop_filter_across_tiles_enabled_flag);
    }

    fprintf(xml, "  <pps_loop_filter_across_slices_enabled_flag>%u</pps_loop_filter_across_slices_enabled_flag>\n", pps->pps_loop_filter_across_slices_enabled_flag);
    fprintf(xml, "  <deblocking_filter_control_present_flag>%u</deblocking_filter_control_present_flag>\n", pps->deblocking_filter_control_present_flag);
    if (pps->deblocking_filter_control_present_flag)
    {
        fprintf(xml, "  <deblocking_filter_override_enabled_flag>%u</deblocking_filter_override_enabled_flag>\n", pps->deblocking_filter_override_enabled_flag);
        fprintf(xml, "  <pps_deblocking_filter_disabled_flag>%u</pps_deblocking_filter_disabled_flag>\n", pps->pps_deblocking_filter_disabled_flag);

        if (!pps->pps_deblocking_filter_disabled_flag)
        {
            fprintf(xml, "  <pps_beta_offset_div2>%i</pps_beta_offset_div2>\n", pps->pps_beta_offset_div2);
            fprintf(xml, "  <pps_tc_offset_div2>%i</pps_tc_offset_div2>\n", pps->pps_tc_offset_div2);
        }
    }

    fprintf(xml, "  <pps_scaling_list_data_present_flag>%u</pps_scaling_list_data_present_flag>\n", pps->pps_scaling_list_data_present_flag);
    if (pps->pps_scaling_list_data_present_flag)
    {
        //
    }

    fprintf(xml, "  <lists_modification_present_flag>%u</lists_modification_present_flag>\n", pps->lists_modification_present_flag);
    fprintf(xml, "  <log2_parallel_merge_level_minus2>%u</log2_parallel_merge_level_minus2>\n", pps->log2_parallel_merge_level_minus2);
    fprintf(xml, "  <slice_segment_header_extension_present_flag>%u</slice_segment_header_extension_present_flag>\n", pps->slice_segment_header_extension_present_flag);

    fprintf(xml, "  <pps_extension_present_flag>%u</pps_extension_present_flag>\n", pps->pps_extension_present_flag);
    if (pps->pps_extension_present_flag)
    {
        fprintf(xml, "  <pps_range_extension_flag>%u</pps_tc_offset_div2>\n", pps->pps_range_extension_flag);
        fprintf(xml, "  <pps_multilayer_extension_flag>%u</pps_multilayer_extension_flag>\n", pps->pps_multilayer_extension_flag);
        fprintf(xml, "  <pps_3d_extension_flag>%u</pps_3d_extension_flag>\n", pps->pps_3d_extension_flag);
        fprintf(xml, "  <pps_scc_extension_flag>%u</pps_scc_extension_flag>\n", pps->pps_scc_extension_flag);
        fprintf(xml, "  <pps_extension_4bits>%u</pps_extension_4bits>\n", pps->pps_extension_4bits);
    }

    if (pps->pps_range_extension_flag)
    {
        //
    }
    if (pps->pps_multilayer_extension_flag)
    {
        //
    }
    if (pps->pps_3d_extension_flag)
    {
        //
    }
    if (pps->pps_scc_extension_flag)
    {
        //
    }
    if (pps->pps_extension_4bits)
    {
        //
    }

    fprintf(xml, "  </a>\n");
}

/* ************************************************************************** */
/* ************************************************************************** */
