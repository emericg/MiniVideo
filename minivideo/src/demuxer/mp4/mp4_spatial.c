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
 * \file      mp4_spatial.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mp4_spatial.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* ************************************************************************** */

/*!
 * \brief Spatial Audio Box
 *
 * From Google 'Spatial Media' RFC's:
 * Spatial Audio Box (SA3D)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spatial-audio-rfc.md
 */
int parse_sa3d(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_sa3d()" CLR_RESET);
    int retcode = SUCCESS;

    unsigned int version = read_bits(bitstr, 8);
    unsigned int ambisonic_type = read_bits(bitstr, 8);
    unsigned int ambisonic_order = read_bits(bitstr, 32);
    unsigned int ambisonic_channel_ordering = read_bits(bitstr, 8);
    unsigned int ambisonic_normalization = read_bits(bitstr, 8);
    unsigned int num_channels = read_bits(bitstr, 32);

    unsigned int *channel_map = NULL;
    if (num_channels > 0)
    {
        channel_map = malloc(num_channels);
        for (unsigned i = 0; i < num_channels; i++)
        {
            channel_map[i] = read_bits(bitstr, 32);
        }
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> version         : %u", version);
    TRACE_1(MP4, "> ambisonic_type  : %u", ambisonic_type);
    TRACE_1(MP4, "> ambisonic_order : %u", ambisonic_order);
    TRACE_1(MP4, "> ambisonic_channel_ordering  : %u", ambisonic_channel_ordering);
    TRACE_1(MP4, "> ambisonic_normalization     : %u", ambisonic_normalization);
    TRACE_1(MP4, "> num_channels    : %u", num_channels);
    for (unsigned i = 0; i < num_channels; i++)
    {
        TRACE_1(MP4, "  > channel_map[%u] : %u", i, channel_map[i]);
    }

#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Spatial Audio</title>\n");
        fprintf(mp4->xml, "  <version>%u</version>\n", version);
        fprintf(mp4->xml, "  <ambisonic_type>%u</ambisonic_type>\n", ambisonic_type);
        fprintf(mp4->xml, "  <ambisonic_order>%u</ambisonic_order>\n", ambisonic_order);
        fprintf(mp4->xml, "  <ambisonic_channel_ordering>%u</ambisonic_channel_ordering>\n", ambisonic_channel_ordering);
        fprintf(mp4->xml, "  <ambisonic_normalization>%u</ambisonic_normalization>\n", ambisonic_normalization);
        fprintf(mp4->xml, "  <num_channels>%u</num_channels>\n", num_channels);

        fprintf(mp4->xml, "  <channel_map>");
        for (unsigned i = 0; i < num_channels; i++)
        {
            fprintf(mp4->xml, "%u ", channel_map[i]);
        }
        fprintf(mp4->xml, "  </channel_map>");
        fprintf(mp4->xml, "  </a>\n");
    }

    free(channel_map);

    return retcode;
}

/*!
 * \brief Non-Diegetic Audio Box
 *
 * From Google 'Spatial Media' RFC's:
 * Non-Diegetic Audio Box (SAND)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spatial-audio-rfc.md
 */
int parse_sand(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_sand()" CLR_RESET);
    int retcode = SUCCESS;

    unsigned int version = read_bits(bitstr, 8);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> version  : %u", version);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Non-Diegetic Audio</title>\n");
        fprintf(mp4->xml, "  <version>%u</version>\n", version);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Stereoscopic 3D Video Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Stereoscopic 3D Video Box (st3d)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_st3d(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_st3d()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned int stereo_mode = read_bits(bitstr, 8);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> stereo_mode  : %u", stereo_mode);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Stereoscopic 3D Video</title>\n");
        fprintf(mp4->xml, "  <stereo_mode>%u</stereo_mode>\n", stereo_mode);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Spherical Video V2 Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Spherical Video Box (sv3d)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_sv3d(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_sv3d()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml);
    if (mp4->xml) fprintf(mp4->xml, "  <title>Spherical Video V2</title>\n");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_SVHD:
                    retcode = parse_svhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_PROJ:
                    retcode = parse_proj(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Spherical Video Header Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Spherical Video Header Box (svhd)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_svhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_svhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    char *metadata_source = NULL;
    // TODO read 'metadata_source' string

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> metadata_source  : %s", metadata_source);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Spherical Video Header</title>\n");
        fprintf(mp4->xml, "  <metadata_source>%s</metadata_source>\n", metadata_source);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief Projection Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Projection Box (proj)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_proj(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_proj()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml);
    if (mp4->xml) fprintf(mp4->xml, "  <title>Projection</title>\n");

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);

        // Then parse subbox content
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subheader.boxtype)
            {
                case BOX_PRHD:
                    retcode = parse_prhd(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_CBMP:
                    retcode = parse_cbmp(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_EQUI:
                    retcode = parse_equi(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_MSPH:
                    retcode = parse_mshp(bitstr, &box_subheader, track, mp4);
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

/*!
 * \brief Projection Header Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Projection Header Box (prhd)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_prhd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_prhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    int pose_yaw_degrees = read_bits(bitstr, 32);
    int pose_pitch_degrees = read_bits(bitstr, 32);
    int pose_roll_degrees = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> pose_yaw_degrees  : %i", pose_yaw_degrees);
    TRACE_1(MP4, "> pose_pitch_degrees: %i", pose_pitch_degrees);
    TRACE_1(MP4, "> pose_roll_degrees : %i", pose_roll_degrees);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Projection Header</title>\n");
        fprintf(mp4->xml, "  <pose_yaw_degrees>%i</pose_yaw_degrees>\n", pose_yaw_degrees);
        fprintf(mp4->xml, "  <pose_pitch_degrees>%i</pose_pitch_degrees>\n", pose_pitch_degrees);
        fprintf(mp4->xml, "  <pose_roll_degrees>%i</pose_roll_degrees>\n", pose_roll_degrees);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief Cubemap Projection Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Cubemap Projection Box (cbmp)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_cbmp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_cbmp()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned layout = read_bits(bitstr, 32);
    unsigned padding = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> layout : %u", layout);
    TRACE_1(MP4, "> padding: %u", padding);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Projection Header</title>\n");
        fprintf(mp4->xml, "  <layout>%u</layout>\n", layout);
        fprintf(mp4->xml, "  <padding>%u</padding>\n", padding);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief Equirectangular Projection Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Equirectangular Projection Box (equi)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_equi(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_prhd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned projection_bounds_top = read_bits(bitstr, 32);
    unsigned projection_bounds_bottom = read_bits(bitstr, 32);
    unsigned projection_bounds_left = read_bits(bitstr, 32);
    unsigned projection_bounds_right = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> projection_bounds_top   : %u", projection_bounds_top);
    TRACE_1(MP4, "> projection_bounds_bottom: %u", projection_bounds_bottom);
    TRACE_1(MP4, "> projection_bounds_left  : %u", projection_bounds_left);
    TRACE_1(MP4, "> projection_bounds_right : %u", projection_bounds_right);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Equirectangular Projection</title>\n");
        fprintf(mp4->xml, "  <projection_bounds_top>%u</projection_bounds_top>\n", projection_bounds_top);
        fprintf(mp4->xml, "  <projection_bounds_bottom>%u</projection_bounds_bottom>\n", projection_bounds_bottom);
        fprintf(mp4->xml, "  <projection_bounds_left>%u</projection_bounds_left>\n", projection_bounds_left);
        fprintf(mp4->xml, "  <projection_bounds_right>%u</projection_bounds_right>\n", projection_bounds_right);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief Mesh Projection Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Mesh Projection Box (mshp)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_mshp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mshp()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributs
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned crc = read_bits(bitstr, 32);
    unsigned encoding_four_cc = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> crc             : %u", crc);
    TRACE_1(MP4, "> encoding_four_cc: %u", encoding_four_cc);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml);
        fprintf(mp4->xml, "  <title>Mesh Projection</title>\n");
        fprintf(mp4->xml, "  <crc>%u</crc>\n", crc);
        fprintf(mp4->xml, "  <encoding_four_cc>%u</encoding_four_cc>\n", encoding_four_cc);
    }

    if (encoding_four_cc == fcc_raw)
    {
        while (mp4->run == true &&
               retcode == SUCCESS &&
               bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
        {
            // Parse subbox header
            Mp4Box_t box_subheader;
            retcode = parse_box_header(bitstr, &box_subheader);

            // Then parse subbox content
            if (mp4->run == true && retcode == SUCCESS)
            {
                switch (box_subheader.boxtype)
                {
                    default:
                        retcode = parse_unknown_box(bitstr, &box_subheader, mp4->xml);
                        break;
                }

                retcode = jumpy_mp4(bitstr, box_header, &box_subheader);
            }
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief Mesh Box
 *
 * From Google 'Spherical Video V2' RFC (draft):
 * Mesh Box (mesh)
 *
 * https://github.com/google/spatial-media/blob/master/docs/spherical-video-v2-rfc.md
 */
int parse_mesh(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_mesh()" CLR_RESET);
    int retcode = SUCCESS;

    // TODO

    return retcode;
}

/* ************************************************************************** */
