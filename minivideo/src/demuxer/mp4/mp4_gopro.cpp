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
 * \file      mp4_gopro.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 */

// minivideo headers
#include "mp4_gopro.h"
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

int parse_firm(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_firm()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Firmware");

    char *firmware = read_mp4_string(bitstr, 12, mp4->xml, "Firmware");
    free(firmware);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_lens(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_lens()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Lens");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_came(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_came()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Camera");

    char *serial = read_mp4_string(bitstr, 16, mp4->xml, "SerialNumber");
    free(serial);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_sett(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_sett()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Settings");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_amba(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_amba()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Ambarella");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_muid(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_muid()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Media Unique ID");

    uint8_t *muid = read_mp4_data(bitstr, 32, mp4->xml, "Media_Unique_ID");
    free(muid);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_hmmt(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hmmt()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro HiLight Tags");

    unsigned hmmt_count = read_mp4_uint32(bitstr, mp4->xml, "hmmt_count");

    for (unsigned i = 0; i < hmmt_count && i < 100; i++)
    {
        char tagname[16];
        snprintf(tagname, 16, "tag_%u", i);

        read_mp4_uint32(bitstr, mp4->xml, tagname);
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_bcid(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_bcid()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Broadcast ID");

    char *bcid = read_mp4_string(bitstr, 36, mp4->xml, "Broadcast_ID");
    free(bcid);

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_guri(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_guri()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro GURI");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_gusi(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gusi()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro GUSI");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_gumi(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gumi()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro GUMI");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_gpmf(Bitstream_t *bitstr, Mp4Box_t *box_header,  Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gpmf()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "GoPro Metadata Format");

    //

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
