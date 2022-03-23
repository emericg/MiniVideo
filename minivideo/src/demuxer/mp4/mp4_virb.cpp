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
 * \file      mp4_virb.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2022
 */

// minivideo headers
#include "mp4_virb.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../minivideo_typedef.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>

/* ************************************************************************** */

int parse_virb_uuid(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_virb_uuid()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "VIRB ActionCam UUID");

    fprintf(mp4->xml, "  <uuid_str string=\"utf8\">");
    for (int i = 0; i < 16; i++)
    {
        fprintf(mp4->xml, "%c", box_header->usertype[i]);
    }
    fprintf(mp4->xml, "</uuid_str>\n");

    int max_bytes = box_header->size - 24;
    char *string = (char *)malloc(max_bytes + 1);
    if (string)
    {
        for (int stringsize = 0; stringsize < max_bytes; stringsize++)
        {
            string[stringsize] = static_cast<char>(read_bits(bitstr, 8));
            if (string[stringsize] == '\0') break;
        }
        string[max_bytes + 1] = '\0'; // in any case...

        int id = 0;
        int last_pos = 0;
        int next_pos = 0;
        char *str_pos = strchr(string, '_');

        while (str_pos != NULL || last_pos < max_bytes)
        {
            if (str_pos) next_pos = str_pos - string;
            else next_pos = max_bytes;

            char name[16];
            if (id == 0) strcpy(name, "model");
            else if (id == 1) strcpy(name, "media");
            else if (id == 2) strcpy(name, "width");
            else if (id == 3) strcpy(name, "height");
            else if (id == 4) strcpy(name, "framerate");
            else if (id == 5) strcpy(name, "SN");
            else if (id == 6) strcpy(name, "unkn");
            else if (id == 7) strcpy(name, "unkn");
            else if (id == 8) strcpy(name, "unkn");
            else if (id == 9) strcpy(name, "date");
            else strcpy(name, "unkn");

            fprintf(mp4->xml, "  <%s string=\"utf8\">", name);
            for (int i = last_pos; i < next_pos; i++)
            {
                fprintf(mp4->xml, "%c", string[i]);
            }
            fprintf(mp4->xml, "</%s>\n", name);

            last_pos = next_pos + 1;
            if (str_pos) str_pos = strchr(str_pos + 1,'_');
            id++;
        }

        free(string);
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_virb_pmcc(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_virb_pmcc()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "VIRB ActionCam Settings");

    int max_bytes = box_header->size - 12;
    char *string = (char *)malloc(max_bytes + 1);
    if (string)
    {
        for (int stringsize = 0; stringsize < max_bytes; stringsize++)
        {
            string[stringsize] = static_cast<char>(read_bits(bitstr, 8));
            if (string[stringsize] == '\0') break;
        }
        string[max_bytes + 1] = '\0'; // in any case...

        int id = 0;
        int last_pos = 0;
        int next_pos = 0;
        char *str_pos = strchr(string, '_');

        while (str_pos != NULL || last_pos < max_bytes)
        {
            if (str_pos) next_pos = str_pos - string;
            else next_pos = max_bytes;

            char name[16];
            strcpy(name, "unkn");

            fprintf(mp4->xml, "  <%s string=\"utf8\">", name);
            for (int i = last_pos; i < next_pos; i++)
            {
                fprintf(mp4->xml, "%c", string[i]);
            }
            fprintf(mp4->xml, "</%s>\n", name);

            last_pos = next_pos + 1;
            if (str_pos) str_pos = strchr(str_pos + 1,'_');
            id++;
        }

        free(string);
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

int parse_virb_hmtp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_virb_hmtp()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "VIRB ActionCam HMTP");

    // pairs of 4b values?

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */
