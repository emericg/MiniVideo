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
#include "mp4_spatial.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../minivideo_fourcc.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"
#include "../../decoder/h264/h264_parameterset.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

/* ************************************************************************** */

/*!
 * \brief QuickTime Format Atom ('frma')
 * \param bitstr
 * \param box_header
 * \param track
 * \param mp4
 * \return
 *
 * https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-75770
 */
int parse_frma(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_frma()" CLR_RESET);
    int retcode = SUCCESS;

    char fcc[5];
    track->pcm_format = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> format  : %s", getFccString_le(track->pcm_format, fcc));
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Format");
        fprintf(mp4->xml, "  <format>%s</format>\n", getFccString_le(track->pcm_format, fcc));
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

int parse_enda(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_enda()" CLR_RESET);
    int retcode = SUCCESS;

    track->pcm_endianness = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> endian  : %u", track->pcm_endianness);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Format");
        fprintf(mp4->xml, "  <endian>%u</endian>\n", track->pcm_endianness);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

int parse_chan(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_chan()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    unsigned int mChannelBitmap = read_bits(bitstr, 16);
    unsigned int mChannelDescriptions = read_bits(bitstr, 16);
    unsigned int mChannelLayoutTag = read_bits(bitstr, 32);
    unsigned int mNumberChannelDescriptions = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> mChannelBitmap  : %u", mChannelBitmap);
    TRACE_1(MP4, "> mChannelDescriptions  : %u", mChannelDescriptions);
    TRACE_1(MP4, "> mChannelLayoutTag  : %u", mChannelLayoutTag);
    TRACE_1(MP4, "> mNumberChannelDescriptions  : %u", mNumberChannelDescriptions);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Audio Channel Layout");
        fprintf(mp4->xml, "  <mChannelBitmap>%u</mChannelBitmap>\n", mChannelBitmap);
        fprintf(mp4->xml, "  <mChannelDescriptions>%u</mChannelDescriptions>\n", mChannelDescriptions);
        fprintf(mp4->xml, "  <mChannelLayoutTag>%u</mChannelLayoutTag>\n", mChannelLayoutTag);
        fprintf(mp4->xml, "  <mNumberChannelDescriptions>%u</mNumberChannelDescriptions>\n", mNumberChannelDescriptions);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/*!
 * \brief siDecompressionParam Atom ('wave')
 * \param bitstr
 * \param box_header
 * \param track
 * \param mp4
 * \return
 */
int parse_wave(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_wave()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "siDecompressionParam");

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
                case BOX_FRMA:
                    retcode = parse_frma(bitstr, &box_subheader, track, mp4);
                    break;
                case BOX_ENDA:
                    retcode = parse_enda(bitstr, &box_subheader, track, mp4);
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
 * \brief AMR parameters Atom ('damr')
 *
 * 3GPP TS 26.244 specification
 */
int parse_damr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_damr()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AMR parameters");

    char *vendor = read_mp4_string(bitstr, 4, mp4->xml, "vendor");
    free(vendor);

    uint8_t decoder_version = read_mp4_uint8(bitstr, mp4->xml, "decoder_version");
    uint16_t mode_set = read_mp4_uint16(bitstr, mp4->xml, "mode_set"); // A bitmask indicating which AMR modes are supported.
    uint8_t mode_change_period = read_mp4_uint8(bitstr, mp4->xml, "mode_change_period"); // Indicates the number of frames between mode changes.
    uint8_t frames_per_sample = read_mp4_uint8(bitstr, mp4->xml, "frames_per_sample");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief AC3 parameters Atom ('dac3')
 */
int parse_dac3(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dac3()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "AC3 parameters");

    uint8_t fscod = read_mp4_uint(bitstr, 2, mp4->xml, "fscod");
    uint8_t bsid = read_mp4_uint(bitstr, 5, mp4->xml, "bsid");
    uint8_t bsmod = read_mp4_uint(bitstr, 3, mp4->xml, "bsmod");
    uint8_t acmod = read_mp4_uint(bitstr, 3, mp4->xml, "acmod");
    uint8_t lfeon = read_mp4_uint(bitstr, 1, mp4->xml, "lfeon");
    uint16_t bit_rate_code = read_mp4_uint(bitstr, 5, mp4->xml, "bit_rate_code");
    uint8_t reserved = read_mp4_uint(bitstr, 5, mp4->xml, "reserved");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/*!
 * \brief Enhanced AC3 parameters Atom ('dec3')
 *
 * Annex F of ETSI TS 102 366 and Deriving the contents of the EC3Specific box section of this information set.
 */
int parse_dec3(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dec3()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "E-AC3 parameters");

    uint16_t data_rate = read_mp4_uint(bitstr, 13, mp4->xml, "data_rate");
    uint8_t num_ind_sub = read_mp4_uint(bitstr, 3, mp4->xml, "num_ind_sub");

    for (uint8_t i = 0; i < num_ind_sub + 1; i++)
    {
        xmlSpacer(mp4->xml, "Stream", i+1);
        uint8_t fscod = read_mp4_uint(bitstr, 2, mp4->xml, "fscod");
        uint8_t bsid = read_mp4_uint(bitstr, 5, mp4->xml, "bsid");
        uint8_t reserved1 = read_mp4_uint(bitstr, 1, mp4->xml, "reserved");
        uint8_t asvc = read_mp4_uint(bitstr, 1, mp4->xml, "asvc");
        uint8_t bsmod = read_mp4_uint(bitstr, 3, mp4->xml, "bsmod");
        uint8_t acmod = read_mp4_uint(bitstr, 3, mp4->xml, "acmod");
        uint8_t lfeon = read_mp4_uint(bitstr, 1, mp4->xml, "lfeon");
        uint8_t reserved2 = read_mp4_uint(bitstr, 3, mp4->xml, "reserved");
        uint8_t num_dep_sub = read_mp4_uint(bitstr, 4, mp4->xml, "num_dep_sub");
        if (num_dep_sub > 0)
        {
            uint16_t chan_loc = read_mp4_uint(bitstr, 9, mp4->xml, "chan_loc");
        }
        else
        {
            uint8_t reserved3 = read_mp4_uint(bitstr, 1, mp4->xml, "reserved");
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Sample Table Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.1 Sample Table Box.
 *
 * Parse the sample table box, container for the time/space map.
 * This box does not contain informations, only other boxes.
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

/*!
 * \brief Sample Description Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box.
 *
 * The SampleDescriptionBox contains information about codec types and some
 * initialization parameters needed to start decoding.
 */
int parse_stsd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse stsd box content
    unsigned int entry_count = read_bits(bitstr, 32);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <entry_count>%u</entry_count>\n", entry_count);

    // Parse subbox header (> SampleEntry)
    for (unsigned int i = 0; i < entry_count; i++)
    {
        Mp4Box_t box_subheader;
        retcode = parse_box_header(bitstr, &box_subheader);
        track->fcc = box_subheader.boxtype; // save fourcc as backup

        // Parse subbox content (> SampleEntry)
        unsigned int reserved[6] = {0};
        for (int j = 0; j < 6; j++)
        {
            reserved[j] = read_bits(bitstr, 8);
        }
        unsigned int data_reference_index = read_bits(bitstr, 16);

        write_box_header(&box_subheader, mp4->xml);
        if (mp4->xml) fprintf(mp4->xml, "  <data_reference_index>%u</data_reference_index>\n", data_reference_index);

        // Parse subbox content (> SampleEntry extensions)
        switch (track->handlerType)
        {
            case MP4_HANDLER_AUDIO:
                parse_stsd_audio(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_VIDEO:
                parse_stsd_video(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_TMCD:
                parse_stsd_tmcd(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_TEXT:
                parse_stsd_text(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_META:
                parse_stsd_meta(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_HINT:
                parse_stsd_hint(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_SDSM:
                parse_stsd_sdsm(bitstr, &box_subheader, track, mp4);
                break;

            case MP4_HANDLER_ODSM:
                parse_stsd_odsm(bitstr, &box_subheader, track, mp4);
                break;

            default:
                TRACE_1(MP4, "Unknown track type, skipped...");
                break;
        }

        if (mp4->xml) fprintf(mp4->xml, "  </a>\n");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

int parse_stsd_audio(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_audio()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // AudioSampleEntry
    {
        if (box_header->boxtype == fcc_AC3 ||
            box_header->boxtype == fcc_ac3)
        {
            track->codec = CODEC_AC3;
            TRACE_1(MP4, "> Audio track is using AC3 codec");
        }
        if (box_header->boxtype == fcc_ec3)
        {
            track->codec = CODEC_EAC3;
            TRACE_1(MP4, "> Audio track is using Enhanced AC-3 codec");
        }
        else if (box_header->boxtype == fcc_AC4 ||
                 box_header->boxtype == fcc_ac4)
        {
            track->codec = CODEC_AC4;
            TRACE_1(MP4, "> Audio track is using AC4 codec");
        }
        else if (box_header->boxtype == fcc_sowt)
        {
            track->codec = CODEC_LPCM;
            TRACE_1(MP4, "> Audio track is using PCM audio");
        }
        else if (box_header->boxtype == fcc_samr)
        {
            track->codec = CODEC_AMR;
            TRACE_1(MP4, "> Audio track is using AMR audio");
        }

        unsigned int reserved0 = read_bits(bitstr, 32);
        unsigned int reserved1 = read_bits(bitstr, 32);

        track->channel_count = read_bits(bitstr, 16);
        track->sample_size_bits = read_bits(bitstr, 16);

        unsigned int pre_defined = read_bits(bitstr, 16);
        unsigned short reserved3 = read_bits(bitstr, 16);

        track->sample_rate_hz = read_bits(bitstr, 16);
        unsigned short reserved4 = read_bits(bitstr, 16);

#if ENABLE_DEBUG
        TRACE_1(MP4, "> channel_count   : %u", track->channel_count);
        TRACE_1(MP4, "> sample_size_bits: %u", track->sample_size_bits);
        TRACE_1(MP4, "> sample_rate_hz  : %u", track->sample_rate_hz);
#endif
        if (mp4->xml) // xmlMapper
        {
            fprintf(mp4->xml, "  <title>Audio Sample Entry</title>\n");
            xmlSpacer(mp4->xml, "Sound Sample Description (Version 0)", -1);
            fprintf(mp4->xml, "  <reserved0>%u</reserved0>\n", reserved0);
            fprintf(mp4->xml, "  <reserved1>%u</reserved1>\n", reserved1);
            fprintf(mp4->xml, "  <channel_count>%u</channel_count>\n", track->channel_count);
            fprintf(mp4->xml, "  <sample_size_bits>%u</sample_size_bits>\n", track->sample_size_bits);
            fprintf(mp4->xml, "  <pre_defined>%u</pre_defined>\n", pre_defined);
            fprintf(mp4->xml, "  <reserved3>%u</reserved3>\n", reserved3);
            fprintf(mp4->xml, "  <sample_rate_hz>%u</sample_rate_hz>\n", track->sample_rate_hz);
            fprintf(mp4->xml, "  <reserved4>%u</reserved4>\n", reserved4);
        }

        int atom_version = reserved0 >> 16;
        if (atom_version == 1)
        {
            unsigned samples_per_packet = read_bits(bitstr, 32);
            unsigned bytes_per_packet = read_bits(bitstr, 32);
            unsigned bytes_per_frame = read_bits(bitstr, 32);
            unsigned bytes_per_sample = read_bits(bitstr, 32);

#if ENABLE_DEBUG
            //
#endif
            if (mp4->xml) // xmlMapper
            {
                xmlSpacer(mp4->xml, "Sound Sample Description (Version 1)", -1);
                fprintf(mp4->xml, "  <samples_per_packet>%u</samples_per_packet>\n", samples_per_packet);
                fprintf(mp4->xml, "  <bytes_per_packet>%u</bytes_per_packet>\n", bytes_per_packet);
                fprintf(mp4->xml, "  <bytes_per_frame>%u</bytes_per_frame>\n", bytes_per_frame);
                fprintf(mp4->xml, "  <bytes_per_sample>%u</bytes_per_sample>\n", bytes_per_sample);
            }
        }
        else if (atom_version == 2)
        {
            unsigned version = read_bits(bitstr, 16);
            unsigned revision_level = read_bits(bitstr, 16);
            unsigned vendor = read_bits(bitstr, 32);
            unsigned always3 = read_bits(bitstr, 16);
            unsigned always16 = read_bits(bitstr, 16);
            unsigned alwaysMinus2 = read_bits(bitstr, 16);
            unsigned always0 = read_bits(bitstr, 16);
            unsigned always65536 = read_bits(bitstr, 32);
            unsigned sizeOfStructOnly = read_bits(bitstr, 32);
            uint64_t asr = read_bits_64(bitstr, 64);
            double audioSampleRate = *((double *)&asr);
            unsigned numAudioChannels = read_bits(bitstr, 32);
            unsigned always7F000000 = read_bits(bitstr, 32);
            unsigned constBitsPerChannel = read_bits(bitstr, 32);
            unsigned formatSpecificFlags = read_bits(bitstr, 32);
            unsigned constBytesPerAudioPacket = read_bits(bitstr, 32);
            unsigned constLPCMFramesPerAudioPacket = read_bits(bitstr, 32);

#if ENABLE_DEBUG
            //
#endif
            if (mp4->xml) // xmlMapper
            {
                xmlSpacer(mp4->xml, "Sound Sample Description (Version 2)", -1);
                fprintf(mp4->xml, "  <version>%u</version>\n", version);
                fprintf(mp4->xml, "  <revision_level>%u</revision_level>\n", revision_level);
                fprintf(mp4->xml, "  <vendor>%u</vendor>\n", vendor);
                fprintf(mp4->xml, "  <always3>%u</always3>\n", always3);
                fprintf(mp4->xml, "  <always16>%u</always16>\n", always16);
                fprintf(mp4->xml, "  <alwaysMinus2>%u</alwaysMinus2>\n", alwaysMinus2);
                fprintf(mp4->xml, "  <always0>%u</always0>\n", always0);
                fprintf(mp4->xml, "  <always65536>%u</always65536>\n", always65536);
                fprintf(mp4->xml, "  <sizeOfStructOnly>%u</sizeOfStructOnly>\n", sizeOfStructOnly);
                fprintf(mp4->xml, "  <audioSampleRate>%f</audioSampleRate>\n", audioSampleRate);
                fprintf(mp4->xml, "  <numAudioChannels>%u</numAudioChannels>\n", numAudioChannels);
                fprintf(mp4->xml, "  <always7F000000>%u</always7F000000>\n", always7F000000);
                fprintf(mp4->xml, "  <constBitsPerChannel>%u</constBitsPerChannel>\n", constBitsPerChannel);
                fprintf(mp4->xml, "  <formatSpecificFlags>%u</formatSpecificFlags>\n", formatSpecificFlags);
                fprintf(mp4->xml, "  <constBytesPerAudioPacket>%u</constBytesPerAudioPacket>\n", constBytesPerAudioPacket);
                fprintf(mp4->xml, "  <constLPCMFramesPerAudioPacket>%u</constLPCMFramesPerAudioPacket>\n", constLPCMFramesPerAudioPacket);
            }
        }
        else if (atom_version > 2)
        {
            TRACE_WARNING(MP4, "Unknown atom_version value: %i", atom_version);
        }
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                case BOX_WAVE:
                    retcode = parse_wave(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_CHAN:
                    retcode = parse_chan(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_SA3D:
                    retcode = parse_sa3d(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_SAND:
                    retcode = parse_sand(bitstr, &box_subsubheader, track, mp4);
                    break;

                case BOX_DAMR:
                    retcode = parse_damr(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_DAC3:
                    retcode = parse_dac3(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_DEC3:
                    retcode = parse_dec3(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Visual Sample Entry.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2.2 Syntax
 *
 * If an AVC box (AVCDecoderConfigurationRecord) is present, it also contains the
 * diferents SPS and PPS of the video.
 */
int parse_stsd_video(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_video()" CLR_RESET);
    int retcode = SUCCESS;

    // VisualSampleEntry
    {
        if (box_header->boxtype == fcc_avc1 || box_header->boxtype == fcc_avc3)
        {
            track->codec = CODEC_H264;
            TRACE_1(MP4, "> Video track is using H.264 codec");
        }
        else if (box_header->boxtype == fcc_hvc1 ||
                 box_header->boxtype == fcc_dvh1 || box_header->boxtype == fcc_dvhe)
        {
            track->codec = CODEC_H265;
            TRACE_1(MP4, "> Video track is using H.265 codec");
        }
        else if (box_header->boxtype == fcc_vvc1)
        {
            track->codec = CODEC_H266;
            TRACE_1(MP4, "> Video track is using H.266 codec");
        }
        else if (box_header->boxtype == fcc_vp08)
        {
            track->codec = CODEC_VP8;
            TRACE_1(MP4, "> Video track is using VP8 codec");
        }
        else if (box_header->boxtype == fcc_vp09)
        {
            track->codec = CODEC_VP9;
            TRACE_1(MP4, "> Video track is using VP9 codec");
        }
        else if (box_header->boxtype == fcc_CFHD)
        {
            track->codec = CODEC_CINEFORM;
            TRACE_1(MP4, "> Video track is using CineForm codec");
        }

        else if (box_header->boxtype == fcc_jpeg)
        {
            track->codec = CODEC_JPEG;
            TRACE_1(MP4, "> Video track is using JPEG picture(s)");
        }
        else if (box_header->boxtype == fcc_png)
        {
            track->codec = CODEC_PNG;
            TRACE_1(MP4, "> Video track is using PNG picture(s)");
        }
        else if (box_header->boxtype == fcc_tga)
        {
            track->codec = CODEC_TGA;
            TRACE_1(MP4, "> Video track is using TGA picture(s)");
        }
        else if (box_header->boxtype == fcc_gif)
        {
            track->codec = CODEC_GIF;
            TRACE_1(MP4, "> Video track is using GIF picture(s)");
        }

        skip_bits(bitstr, 16); // pre_defined
        skip_bits(bitstr, 16); // reserved

        unsigned int pre_defined[3];
        pre_defined[0] = read_bits(bitstr, 32);
        pre_defined[1] = read_bits(bitstr, 32);
        pre_defined[2] = read_bits(bitstr, 32);

        track->width = read_bits(bitstr, 16);
        track->height = read_bits(bitstr, 16);

        // 0x00480000; // 72 dpi
        unsigned int horizresolution = read_bits(bitstr, 32);
        unsigned int vertresolution = read_bits(bitstr, 32);

        skip_bits(bitstr, 32); // reserved

        unsigned int frame_count = read_bits(bitstr, 16);

        uint8_t compressorsize = (uint8_t)read_bits(bitstr, 8);
        for (int i = 0; i < 31; i++)
        {
            track->compressorname[i] = (char)read_bits(bitstr, 8);
        }
        track->compressorname[compressorsize] = '\0';

        unsigned int color_depth = read_bits(bitstr, 16);
        track->color_depth = color_depth / 3;
        skip_bits(bitstr, 16); // pre_defined

#if ENABLE_DEBUG
        TRACE_1(MP4, "> width  : %u", track->width);
        TRACE_1(MP4, "> height : %u", track->height);
        TRACE_1(MP4, "> horizresolution : 0x%X", horizresolution);
        TRACE_1(MP4, "> vertresolution  : 0x%X", vertresolution);
        TRACE_1(MP4, "> frame_count     : %u", frame_count);
        TRACE_1(MP4, "> compressor      : '%s'", track->compressorname);
        TRACE_1(MP4, "> color depth     : %u", color_depth);
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Visual Sample Entry</title>\n");
            fprintf(mp4->xml, "  <width>%u</width>\n", track->width);
            fprintf(mp4->xml, "  <height>%u</height>\n", track->height);
            fprintf(mp4->xml, "  <horizresolution>%u</horizresolution>\n", horizresolution);
            fprintf(mp4->xml, "  <vertresolution>%u</vertresolution>\n", vertresolution);
            fprintf(mp4->xml, "  <frame_count>%u</frame_count>\n", frame_count);
            fprintf(mp4->xml, "  <compressorname>%s</compressorname>\n", track->compressorname);
            fprintf(mp4->xml, "  <color_depth>%u</color_depth>\n", color_depth);
        }
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                case BOX_D263:
                    retcode = parse_d263(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_AVCC:
                    retcode = parse_avcC(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_HVCC:
                    retcode = parse_hvcC(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_VVCC:
                    retcode = parse_vvcC(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_VPCC:
                    retcode = parse_vpcC(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_AV1C:
                    retcode = parse_av1C(bitstr, &box_subsubheader, track, mp4);
                    break;

                case BOX_DVCC:
                    retcode = parse_dvcC(bitstr, &box_subsubheader, track, mp4);
                    break;

                case BOX_BTRT:
                    retcode = parse_btrt(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_CLAP:
                    retcode = parse_clap(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_COLR:
                    retcode = parse_colr(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_CHRM:
                    retcode = parse_chrm(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_FIEL:
                    retcode = parse_fiel(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_GAMA:
                    retcode = parse_gama(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_PASP:
                    retcode = parse_pasp(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_ST3D:
                    retcode = parse_st3d(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_SV3D:
                    retcode = parse_sv3d(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_COLL:
                    retcode = parse_coll(bitstr, &box_subsubheader, track, mp4);
                    break;
                case BOX_SMDM:
                    retcode = parse_smdm(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Timecode Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Timecode Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-6983
 */
int parse_stsd_tmcd(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_tmcd()" CLR_RESET);
    int retcode = SUCCESS;

    {
        skip_bits(bitstr, 32); // reserved
        unsigned int flags = read_bits(bitstr, 32);
        unsigned int time_scale = read_bits(bitstr, 32);
        unsigned int frame_duration = read_bits(bitstr, 32);
        track->number_of_frames = read_bits(bitstr, 8);
        skip_bits(bitstr, 8); // reserved

#if ENABLE_DEBUG
        TRACE_1(MP4, "> flags  : %u", flags);
        TRACE_1(MP4, "> time_scale : %u", time_scale);
        TRACE_1(MP4, "> frame_duration : %u", frame_duration);
        TRACE_1(MP4, "> number_of_frames  : %u", track->number_of_frames);
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Timecode Sample Description</title>\n");
            fprintf(mp4->xml, "  <flags>%u</flags>\n", flags);
            fprintf(mp4->xml, "  <time_scale>%u</time_scale>\n", time_scale);
            fprintf(mp4->xml, "  <frame_duration>%u</frame_duration>\n", frame_duration);
            fprintf(mp4->xml, "  <number_of_frames>%u</number_of_frames>\n", track->number_of_frames);
        }
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Text Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Text Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-69835
 */
int parse_stsd_text(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_text()" CLR_RESET);
    int retcode = SUCCESS;

    {
        unsigned int display_flags = read_bits(bitstr, 32);
        unsigned int text_justification = read_bits(bitstr, 32);
        unsigned int backgroundcolor_r = read_bits(bitstr, 16);
        unsigned int backgroundcolor_g = read_bits(bitstr, 16);
        unsigned int backgroundcolor_b = read_bits(bitstr, 16);
        uint64_t default_text_box = read_bits_64(bitstr, 64);
        /*uint64_t reserved =*/ read_bits_64(bitstr, 64);
        unsigned int font_number = read_bits(bitstr, 16);
        unsigned int font_face = read_bits(bitstr, 16);
        /*unsigned int reserved =*/ read_bits(bitstr, 8);
        /*unsigned int reserved =*/ read_bits(bitstr, 16);
        unsigned int foregroundcolor_r = read_bits(bitstr, 16);
        unsigned int foregroundcolor_g = read_bits(bitstr, 16);
        unsigned int foregroundcolor_b = read_bits(bitstr, 16);
        //char text_name[128]; // TODO

#if ENABLE_DEBUG
        TRACE_1(MP4, "> display_flags  : %u", display_flags);
        TRACE_1(MP4, "> text_justification  : %u", text_justification);
        TRACE_1(MP4, "> background_color  : 0x%X%X%X",
                backgroundcolor_r, backgroundcolor_g, backgroundcolor_b);
        TRACE_1(MP4, "> default_text_box  : %llu", default_text_box);
        TRACE_1(MP4, "> font_number  : %u", font_number);
        TRACE_1(MP4, "> font_face  : %u", font_face);
        TRACE_1(MP4, "> foreground_color  : 0x%X%X%X",
                foregroundcolor_r, foregroundcolor_g, foregroundcolor_b);
        //TRACE_1(MP4, "> text_name  : '%s'", text_name); // TODO
#endif // ENABLE_DEBUG

        // xmlMapper
        if (mp4->xml)
        {
            fprintf(mp4->xml, "  <title>Text Sample Description</title>\n");
            fprintf(mp4->xml, "  <display_flags>%u</display_flags>\n", display_flags);
            fprintf(mp4->xml, "  <text_justification>%u</text_justification>\n", text_justification);
            fprintf(mp4->xml, "  <background_color>0x%X%X%X</background_color>\n",
                    backgroundcolor_r, backgroundcolor_g, backgroundcolor_b);
            fprintf(mp4->xml, "  <default_text_box>%" PRIu64 "</default_text_box>\n", default_text_box);
            fprintf(mp4->xml, "  <font_number>%u</font_number>\n", font_number);
            fprintf(mp4->xml, "  <font_face>%u</font_face>\n", font_face);
            fprintf(mp4->xml, "  <foreground_color>0x%X%X%X</foreground_color>\n",
                    foregroundcolor_r, foregroundcolor_g, foregroundcolor_b);
            //fprintf(mp4->xml, "  <text_name>%s</text_name>\n", text_name); // TODO
        }
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Timed Metadata Sample Description
 *
 * From 'QuickTime File Format' specification:
 * - "Timed Metadata Sample Description"
 * - https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap3/qtff3.html#//apple_ref/doc/uid/TP40000939-CH205-SW130
 */
int parse_stsd_meta(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_meta()" CLR_RESET);
    int retcode = SUCCESS;

    {
        /*unsigned int reserved =*/ read_bits(bitstr, 32);

        print_box_header(box_header);
        if (mp4->xml) fprintf(mp4->xml, "  <title>Timed Metadata Sample Description</title>\n");
    }

    while (mp4->run == true &&
           retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Hint
 */
int parse_stsd_hint(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_hint()" CLR_RESET);
    int retcode = SUCCESS;

    {
        /*unsigned int reserved =*/ read_bits(bitstr, 32);

        print_box_header(box_header);
        if (mp4->xml) fprintf(mp4->xml, "  <title>Hint</title>\n");
    }

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief MPEG-4 Systems
 */
int parse_stsd_sdsm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_sdsm()" CLR_RESET);
    int retcode = SUCCESS;

    // Print box header
    print_box_header(box_header);
    //write_box_header(box_header, mp4->xml, "MPEG-4 Systems Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <title>MPEG-4 Systems Sample Description</title>\n");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
                case BOX_ESDS:
                    retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                    break;

                default:
                    retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                    break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief MPEG-4 Objects
 */
int parse_stsd_odsm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_stsd_odsm()" CLR_RESET);
    int retcode = SUCCESS;

    // Print box header
    print_box_header(box_header);
    //write_box_header(box_header, mp4->xml, "MPEG-4 Objects Sample Description");
    if (mp4->xml) fprintf(mp4->xml, "  <title>MPEG-4 Objects Sample Description</title>\n");

    while (mp4->run == true && retcode == SUCCESS &&
           bitstream_get_absolute_byte_offset(bitstr) < (box_header->offset_end - 8))
    {
        // Parse subbox header
        Mp4Box_t box_subsubheader;
        retcode = parse_box_header(bitstr, &box_subsubheader);

        // Parse subbox content ////////////////////////////////////////////////
        if (mp4->run == true && retcode == SUCCESS)
        {
            switch (box_subsubheader.boxtype)
            {
            case BOX_ESDS:
                retcode = parse_esds(bitstr, &box_subsubheader, track, mp4);
                break;

            default:
                retcode = parse_unknown_box(bitstr, &box_subsubheader, mp4->xml);
                break;
            }

            retcode = jumpy_mp4(bitstr, box_header, &box_subsubheader);
        }
    }

    return retcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Elementary Stream Descriptor box - FullBox.
 *
 * Probably the worst part of ISO BMF...
 *
 * From 'ISO/IEC 14496-1' specification:
 * - 8.6 Object Descriptor Components:
 * - 8.6.2 ObjectDescriptorBase
 * - 8.6.3 ObjectDescriptor
 * - 8.6.4 InitialObjectDescriptor
 * - 8.6.5 ESDescriptor (13.2.2.1 Elementary Stream Tracks?)
 * - 8.6.6 DecoderConfigDescriptor
 * - 8.6.7 DecoderSpecificInfo
 * - 8.6.8 SLConfigDescriptor
 * - 8.6.9 IP_IdentificationDataSet
 * - 8.6.10 ContentIdentificationDescriptor
 * - 8.6.11 SupplementaryContentIdentificationDescriptor
 * - 8.6.12 IPI_DescrPointer
 * - 8.6.13 IPMP_DescriptorPointer
 * - 8.6.14 IPMP Descriptor
 * - 8.6.15 QoS_Descriptor
 * - 8.6.16 ExtensionDescriptor
 * - 8.6.17 RegistrationDescriptor
 *
 * From 'ISO/IEC 14496-1' Table 1 - List of Class Tags for Descriptors
 *
 * From 'ISO/IEC 14496-3 Subpart 1' specification:
 * - 1.6.2.1 AudioSpecificConfig.
 * From 'ISO/IEC 14496-3 Subpart 3' specification:
 * - x.x.x GASpecificConfig
 */
int parse_esds(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_esds()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Elementary Stream Descriptor");

    uint8_t objectTypeIndication = 0;
    uint8_t streamType = 0;

    // Parse box content
    while (bitstream_get_absolute_byte_offset(bitstr) < box_header->offset_end)
    {
        // we may have some bits left unread... so we play it safe...
        bitstream_force_alignment(bitstr);

        // tag_delimiters_pre // documentation?
        uint8_t tag_delimiters_pre = 0;
        while (next_bits(bitstr, 8) == 0x00)
        {
            skip_bits(bitstr, 8);
            tag_delimiters_pre++;
        }

        int64_t tag_offset = bitstream_get_absolute_byte_offset(bitstr);
        uint8_t tag = (uint8_t)read_bits(bitstr, 8);

        // tag_delimiters_post // optional extended descriptor type tag string
        uint8_t tag_delimiters_post = 0;
        while (next_bits(bitstr, 8) == 0x80 /*|| next_bits(bitstr, 8) == 0x40 ||
               next_bits(bitstr, 8) == 0xFE ||
               next_bits(bitstr, 8) == 0x40 || next_bits(bitstr, 8) == 0x5F*/)
        {
            skip_bits(bitstr, 8);
            tag_delimiters_post++;
        }

        uint8_t tag_datasize = (uint8_t)read_bits(bitstr, 8);

        TRACE_1(MP4, "esds NEXT TAG %X (size: %i) @ %lli", tag, tag_datasize, tag_offset);

        // data

        if (tag == 0x01) // ObjectDescriptor TAG ///////////////////////////////
        {
            xmlSpacer(mp4->xml, "ObjectDescriptor (0x01)", -1);

            uint16_t ObjectDescriptorID = read_mp4_uint(bitstr, 10, mp4->xml, "ObjectDescriptorID");
            bool urlFlag = read_mp4_flag(bitstr, mp4->xml, "urlFlag");
            uint16_t reserved = read_mp4_uint(bitstr, 5, mp4->xml, "reserved");

            if (urlFlag)
            {
                uint8_t urlLength = read_mp4_uint8(bitstr, mp4->xml, "urlLength");
                char *urlString = read_mp4_string(bitstr, urlLength, mp4->xml, "urlString");
                free(urlString);
            }
        }
        else if (tag == 0x02) // InitialObjectDescriptor TAG ///////////////////
        {
            xmlSpacer(mp4->xml, "InitialObjectDescriptor (0x02)", -1);

            uint16_t ObjectDescriptorID = read_mp4_uint(bitstr, 10, mp4->xml, "ObjectDescriptorID");
            bool urlFlag = read_mp4_flag(bitstr, mp4->xml, "urlFlag");
            bool includeInlineProfileLevelFlag = read_mp4_flag(bitstr, mp4->xml, "includeInlineProfileLevelFlag");
            uint16_t reserved = read_mp4_uint(bitstr, 4, mp4->xml, "reserved");

            if (urlFlag)
            {
                uint8_t urlLength = read_mp4_uint8(bitstr, mp4->xml, "urlLength");
                char *urlString = read_mp4_string(bitstr, (box_header->size - 8), mp4->xml, "urlString");
                free(urlString);
            }
            else
            {
                uint8_t ODProfileLevelIndication = read_mp4_uint8(bitstr, mp4->xml, "ODProfileLevelIndication");
                uint8_t sceneProfileLevelIndication = read_mp4_uint8(bitstr, mp4->xml, "sceneProfileLevelIndication");
                uint8_t audioProfileLevelIndication = read_mp4_uint8(bitstr, mp4->xml, "audioProfileLevelIndication");
                uint8_t visualProfileLevelIndication = read_mp4_uint8(bitstr, mp4->xml, "visualProfileLevelIndication");
                uint8_t graphicsProfileLevelIndication = read_mp4_uint8(bitstr, mp4->xml, "graphicsProfileLevelIndication");
            }
        }
        else if (tag == 0x03) // ESDescriptor TAG //////////////////////////////
        {
            xmlSpacer(mp4->xml, "ESDescriptor (0x03)", -1);

            uint16_t esId = read_mp4_uint16(bitstr, mp4->xml, "esId");
            bool streamDependenceFlag = read_mp4_flag(bitstr, mp4->xml, "streamDependenceFlag");
            bool urlFlag = read_mp4_flag(bitstr, mp4->xml, "urlFlag");
            bool OCRstreamFlag = read_mp4_flag(bitstr, mp4->xml, "OCRstreamFlag");
            uint8_t streamPriority = read_mp4_uint(bitstr, 5, mp4->xml, "streamPriority");

            if (streamDependenceFlag)
            {
                uint16_t dependsOn_esId = read_mp4_uint16(bitstr, mp4->xml, "dependsOn_esId");
            }
            if (urlFlag)
            {
                uint8_t urlLength = read_mp4_uint8(bitstr, mp4->xml, "urlLength");
                char *urlString = read_mp4_string(bitstr, urlLength, mp4->xml, "urlString");
                free(urlString);
            }
            if (OCRstreamFlag)
            {
                uint16_t dependsOn_esId = read_mp4_uint16(bitstr, mp4->xml, "OCR_esId");
            }
        }
        else if (tag == 0x04) // DecoderConfigDescriptor TAG ///////////////////
        {
            xmlSpacer(mp4->xml, "DecoderConfigDescriptor (0x04)", -1);

            objectTypeIndication = read_mp4_uint8(bitstr, mp4->xml, "objectTypeIndication");
            streamType = read_mp4_uint(bitstr, 6, mp4->xml, "streamType");
            bool upStream = read_mp4_flag(bitstr, mp4->xml, "upStream");
            bool reserved = read_mp4_flag(bitstr, mp4->xml, "reserved");
            uint32_t bufferSizeDB = read_mp4_uint(bitstr, 24, mp4->xml, "bufferSizeDB");
            int32_t maxBitRate = static_cast<int32_t>(read_mp4_uint(bitstr, 32, mp4->xml, "maxBitRate"));
            int32_t avgBitRate = static_cast<int32_t>(read_mp4_uint(bitstr, 32, mp4->xml, "avgBitRate"));

            switch (objectTypeIndication)
            {
                // This object type shall be used for all streamTypes defined in ISO/IEC 14496-1 except IPMP streams.

                //case 0x01:
                //case 0x02: // Systems ISO/IEC 14496-1
                case 0x20: // Visual ISO/IEC 14496-2
                    track->codec = CODEC_MPEG4_ASP;
                    break;
                case 0x40: // Audio ISO/IEC 14496-3
                    track->codec = CODEC_AAC;
                    break;
                case 0x60: // Visual ISO/IEC 13818-2 Simple Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_SP;
                    break;
                case 0x61: // Visual ISO/IEC 13818-2 Main Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_MP;
                    break;
                case 0x62: // Visual ISO/IEC 13818-2 SNR Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_SNR;
                    break;
                case 0x63: // Visual ISO/IEC 13818-2 Spatial Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_Spatial;
                    break;
                case 0x64: // Visual ISO/IEC 13818-2 High Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_HP;
                    break;
                case 0x65: // Visual ISO/IEC 13818-2 422 Profile
                    track->codec = CODEC_H262;
                    track->codec_profile = PROF_H262_422;
                    break;
                case 0x66: // Audio ISO/IEC 13818-7 Main Profile
                    track->codec = CODEC_AAC;
                    track->codec_profile = PROF_AAC_Main;
                    break;
                case 0x67: // Audio ISO/IEC 13818-7 LowComplexity Profile
                    track->codec = CODEC_AAC;
                    track->codec_profile = PROF_AAC_LC;
                    break;
                case 0x68: // Audio ISO/IEC 13818-7 Scalable Sampling Rate Profile
                    track->codec = CODEC_AAC;
                    track->codec_profile = PROF_AAC_SSR;
                    break;
                case 0x69: // Audio ISO/IEC 13818-3
                    track->codec = CODEC_MPEG_L3;
                    break;
                case 0x6A: // Visual ISO/IEC 11172-2
                    track->codec = CODEC_MPEG1;
                    break;
                case 0x6B: // Audio ISO/IEC 11172-3
                    track->codec = CODEC_MPEG_L1; // ?
                    break;
                case 0x6C: // Visual ISO/IEC 10918-1
                    track->codec = CODEC_JPEG;
                    break;
            }
/*
            switch (streamType)
            {
                case 0x01: // ObjectDescriptorStream (see 8.5)
                case 0x02: // ClockReferenceStream (see 10.2.5)
                case 0x03: // SceneDescriptionStream (see 9.2.1)
                case 0x04: // VisualStream
                case 0x05: // AudioStream
                case 0x06: // MPEG7Stream
                case 0x07: // IPMPStream (see 8.3.2)
                case 0x08: // ObjectContentInfoStream (see 8.4.2)
                case 0x09: // MPEGJStream
                default:
                    break;
            }
*/
        }
        else if (tag == 0x05) // DecoderSpecificInfo TAG ///////////////////////
        {
            xmlSpacer(mp4->xml, "DecoderSpecificInfo (0x05)", -1);

            // depends on the values of DecoderConfigDescriptor.streamType and DecoderConfigDescriptor.objectTypeIndication

            // For values of DecoderConfigDescriptor.objectTypeIndication that refer to scene description streams
            // the semantics of decoder specific information is defined in 9.2.1.2.

            if (objectTypeIndication == 0x01 || objectTypeIndication == 0x01)
            {
                xmlSpacer(mp4->xml, "(Systems ISO/IEC 14496-1)", -1);
            }
            if (objectTypeIndication == 0x20)
            {
                // For values of DecoderConfigDescriptor.objectTypeIndication that refer to streams complying with
                // ISO/IEC 14496-2 the syntax and semantics of decoder specific information are defined in Annex K of that part.

                xmlSpacer(mp4->xml, "(Visual ISO/IEC 14496-2)", -1);

                // VisualObjectSequence()
                // VisualObject()
                // video_signal_type()
                // user_data()
                // VideoObjectLayer()
                // Group_of_VideoObjectPlane()
                // VideoObjectPlane()
                // macroblock()
                // block( i )
                // StillTextureObject()
                // MeshObject()
                // mesh_motion()
                // 3D_Mesh_Object ()
                // upstream_message()
            }
            if (objectTypeIndication == 0x40)
            {
                // For values of DecoderConfigDescriptor.objectTypeIndication that refer to streams complying with
                // ISO/IEC 14496-3 the syntax and semantics of decoder specific information are defined in section 1, clause 1.6 of that part.

                xmlSpacer(mp4->xml, "(Audio ISO/IEC 14496-3)", -1);

                uint8_t audioObjectType = read_mp4_uint(bitstr, 5, mp4->xml, "audioObjectType");
                uint8_t audioObjectTypeExt = 0;
                if (audioObjectType == 31)
                {
                    audioObjectTypeExt = read_mp4_uint(bitstr, 6, mp4->xml, "audioObjectTypeExt");
                    audioObjectType = 32 + audioObjectTypeExt;
                }

                uint8_t samplingFrequencyIndex = read_mp4_uint(bitstr, 4, mp4->xml, "samplingFrequencyIndex");
                uint32_t samplingFrequency = 0;
                if (samplingFrequencyIndex == 0xF)
                {
                    samplingFrequency = read_mp4_uint(bitstr, 24, mp4->xml, "samplingFrequency");
                }
                uint8_t channelConfiguration = read_mp4_uint(bitstr, 4, mp4->xml, "channelConfiguration");

                uint8_t extensionAudioObjectType = 0;
                uint8_t extensionSamplingFrequencyIndex = 0;
                uint8_t extensionSamplingFrequency = 0;
                if (audioObjectType == 5)
                {
                    extensionAudioObjectType = audioObjectType;
                    extensionSamplingFrequencyIndex = read_mp4_uint(bitstr, 4, mp4->xml, "extensionSamplingFrequencyIndex");
                    if (extensionSamplingFrequencyIndex == 0xF)
                    {
                        extensionSamplingFrequency = read_mp4_uint(bitstr, 24, mp4->xml, "extensionSamplingFrequency");
                    }
                }

                switch (audioObjectType)
                {
                case 1:
                    track->codec_profile = PROF_AAC_Main;
                    break;
                case 2:
                    track->codec_profile = PROF_AAC_LC;
                    break;
                case 3:
                    track->codec_profile = PROF_AAC_SSR;
                    break;
                case 4:
                    track->codec_profile = PROF_AAC_LTP;
                    break;
                case 5:
                    track->codec_profile = PROF_AAC_HE; // SBR (Spectral Band Replication)
                    break;
                case 6:
                    track->codec_profile = PROF_AAC_Scalable;
                    break;
                case 7:
                    //track->codec = CODEC_MPEG4_TwinVQ;
                    break;
                case 8:
                    track->codec = CODEC_MPEG4_CELP;
                    break;
                case 9:
                    track->codec = CODEC_MPEG4_HVXC;
                    break;
                case 29:
                    track->codec_profile = PROF_AAC_HEv2; // PS (Parametric Stereo)
                    break;
                case 32:
                    //track->codec_profile = Layer 1;
                    break;
                case 33:
                    //track->codec_profile = Layer 2;
                    break;
                case 34:
                    //track->codec_profile = Layer 3;
                    break;
                case 35:
                    track->codec = CODEC_MPEG4_DST;
                    break;
                case 36:
                    track->codec = CODEC_MPEG4_ALS;
                    break;
                case 37:
                case 38:
                    track->codec = CODEC_MPEG4_SLS;
                    break;
                }

                switch (audioObjectType)
                {
                case 1:
                case 2:
                case 3:
                case 4:
                case 6:
                case 7:
                case 17:
                case 19:
                case 20:
                case 21:
                case 22:
                case 23: {
                    // Audio ISO/IEC 14496-3
                    // 4.4.1 Decoder configuration (GASpecificConfig)
                    // 4.5.1.1 GASpecificConfig()

                    bool frameLengthFlag = read_mp4_flag(bitstr, mp4->xml, "frameLengthFlag");
                    bool dependsOnCoreCoder = read_mp4_flag(bitstr, mp4->xml, "dependsOnCoreCoder");
                    if (dependsOnCoreCoder)
                    {
                        uint16_t coreCoderDelay = read_mp4_uint(bitstr, 14, mp4->xml, "coreCoderDelay");
                    }
                    bool extensionFlag = read_mp4_flag(bitstr, mp4->xml, "extensionFlag");

                    if (!channelConfiguration)
                    {
                        // program_config_element()
                        uint8_t element_instance_tag = read_mp4_uint(bitstr, 4, mp4->xml, "element_instance_tag");
                        uint8_t object_type = read_mp4_uint(bitstr, 2, mp4->xml, "object_type");
                        uint8_t sampling_frequency_index = read_mp4_uint(bitstr, 4, mp4->xml, "sampling_frequency_index");
                        uint8_t num_front_channel_elements = read_mp4_uint(bitstr, 4, mp4->xml, "num_front_channel_elements");
                        uint8_t num_side_channel_elements = read_mp4_uint(bitstr, 4, mp4->xml, "num_side_channel_elements");
                        uint8_t num_back_channel_elements = read_mp4_uint(bitstr, 4, mp4->xml, "num_back_channel_elements");
                        uint8_t num_lfe_channel_elements = read_mp4_uint(bitstr, 2, mp4->xml, "num_lfe_channel_elements");
                        uint8_t num_assoc_data_elements = read_mp4_uint(bitstr, 3, mp4->xml, "num_assoc_data_elements");
                        uint8_t num_valid_cc_elements = read_mp4_uint(bitstr, 4, mp4->xml, "num_valid_cc_elements");

                        bool mono_mixdown_present = read_mp4_flag(bitstr, mp4->xml, "mono_mixdown_present");
                        if (mono_mixdown_present == 1)
                        {
                            uint8_t mono_mixdown_element_number = read_mp4_uint(bitstr, 4, mp4->xml, "mono_mixdown_element_number");
                        }
                        bool stereo_mixdown_present = read_mp4_flag(bitstr, mp4->xml, "stereo_mixdown_present");
                        if (stereo_mixdown_present == 1)
                        {
                            uint8_t stereo_mixdown_element_number = read_mp4_uint(bitstr, 4, mp4->xml, "stereo_mixdown_element_number");
                        }
                        bool matrix_mixdown_idx_present = read_mp4_flag(bitstr, mp4->xml, "matrix_mixdown_idx_present");
                        if (matrix_mixdown_idx_present == 1)
                        {
                            uint8_t matrix_mixdown_idx = read_mp4_uint(bitstr, 2, mp4->xml, "matrix_mixdown_idx");
                            bool pseudo_surround_enable = read_mp4_flag(bitstr, mp4->xml, "pseudo_surround_enable");
                        }
                        for (uint8_t i = 0; i < num_front_channel_elements; i++)
                        {
                            char fieldname[64];
                            sprintf(fieldname, "front_element_is_cpe[%i]", i);

                            /*uint8_t front_element_is_cpe[i] =*/ read_mp4_uint(bitstr, 1, mp4->xml, fieldname);
                            /*uint8_t front_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, fieldname);
                        }
                        for (uint8_t i = 0; i < num_side_channel_elements; i++)
                        {
                            /*uint8_t side_element_is_cpe[i] =*/ read_mp4_uint(bitstr, 1, mp4->xml, "");
                            /*uint8_t side_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, "");
                        }
                        for (uint8_t i = 0; i < num_back_channel_elements; i++)
                        {
                            /*uint8_t back_element_is_cpe[i] =*/ read_mp4_uint(bitstr, 1, mp4->xml, "");
                            /*uint8_t back_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, "");
                        }
                        for (uint8_t i = 0; i < num_lfe_channel_elements; i++)
                        {
                            /*uint8_t lfe_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, "");
                        }
                        for (uint8_t i = 0; i < num_assoc_data_elements; i++)
                        {
                            /*uint8_t assoc_data_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, "");
                        }
                        for (uint8_t i = 0; i < num_valid_cc_elements; i++)
                        {
                            /*uint8_t cc_element_is_ind_sw[i] =*/ read_mp4_uint(bitstr, 1, mp4->xml, "");
                            /*uint8_t valid_cc_element_tag_select[i] =*/ read_mp4_uint(bitstr, 4, mp4->xml, "");
                        }

                        // we may have some bits left unread... so we play it safe...
                        bitstream_force_alignment(bitstr);

                        uint8_t comment_field_bytes = read_mp4_uint8(bitstr, mp4->xml, "comment_field_bytes");
                        for (uint8_t i = 0; i < comment_field_bytes; i++)
                        {
                            /*uint8_t comment_field_data[i] =*/ read_mp4_uint8(bitstr, mp4->xml, "comment_field_bytes");
                        }

                        switch (object_type)
                        {
                        case 0:
                            track->codec_profile = PROF_AAC_Main;
                            break;
                        case 1:
                            track->codec_profile = PROF_AAC_LC;
                            break;
                        case 2:
                            track->codec_profile = PROF_AAC_SSR;
                            break;
                        case 3:
                            track->codec_profile = PROF_AAC_LTP;
                            break;
                        }
                    }

                    if ((audioObjectType == 6) || (audioObjectType == 20))
                    {
                        uint8_t layerNr = read_mp4_uint(bitstr, 3, mp4->xml, "layerNr");
                    }

                    if (extensionFlag)
                    {
                        if (audioObjectType == 22)
                        {
                            uint8_t numOfSubFrame = read_mp4_uint(bitstr, 5, mp4->xml, "numOfSubFrame");
                            uint16_t layer_length = read_mp4_uint(bitstr, 11, mp4->xml, "layer_length");
                        }
                        if (audioObjectType == 17 || audioObjectType == 19 || audioObjectType == 20 || audioObjectType == 23)
                        {
                            bool aacSectionDataResilienceFlag = read_mp4_flag(bitstr, mp4->xml, "aacSectionDataResilienceFlag");
                            bool aacScalefactorDataResilienceFlag = read_mp4_flag(bitstr, mp4->xml, "aacScalefactorDataResilienceFlag");
                            bool aacSpectralDataResilienceFlag = read_mp4_flag(bitstr, mp4->xml, "aacSpectralDataResilienceFlag");
                        }
                        bool extensionFlag3 = read_mp4_flag(bitstr, mp4->xml, "extensionFlag3");;
                        if (extensionFlag3)
                        {
                            // reserved for future version
                        }
                    }

                    switch (samplingFrequencyIndex)
                    {
                    case 0:
                        track->sample_rate_hz = 96000;
                        break;
                    case 1:
                        track->sample_rate_hz = 88200;
                        break;
                    case 2:
                        track->sample_rate_hz = 64000;
                        break;
                    case 3:
                        track->sample_rate_hz = 48000;
                        break;
                    case 4:
                        track->sample_rate_hz = 44100;
                        break;
                    case 5:
                        track->sample_rate_hz = 32000;
                        break;
                    case 6:
                        track->sample_rate_hz = 24000;
                        break;
                    case 7:
                        track->sample_rate_hz = 22050;
                        break;
                    case 8:
                        track->sample_rate_hz = 16000;
                        break;
                    case 9:
                        track->sample_rate_hz = 12000;
                        break;
                    case 10:
                        track->sample_rate_hz = 11025;
                        break;
                    case 11:
                        track->sample_rate_hz = 8000;
                        break;
                    case 12:
                        track->sample_rate_hz = 7350;
                        break;
                    case 13:
                    case 14:
                        track->sample_rate_hz = 0; // 'reserved'
                        break;
                    case 15:
                        track->sample_rate_hz = samplingFrequency;
                        break;
                    }

                    switch (channelConfiguration)
                    {
                    case 0:
                        //track->channel_count = x; // 'AOT Specifc Config'
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        track->channel_count = channelConfiguration;
                        break;
                    case 7:
                        track->channel_count = 8;
                        break;
                    default:
                        //track->channel_count = x; // 'reserved'
                        break;
                    }
                }
                    break;

                case 8:
                    // TODO // CelpSpecificConfig();
                    break;
                case 9:
                    // TODO // HvxcSpecificConfig();
                    break;
                case 12:
                    // TODO // TTSSpecificConfig();
                    break;

                case 13:
                case 14:
                case 15:
                case 16:
                    // TODO // StructuredAudioSpecificConfig();
                    break;

                case 24:
                    // TODO // ErrorResilientCelpSpecificConfig();
                    break;
                case 25:
                    // TODO // ErrorResilientHvxcSpecificConfig();
                    break;

                case 26:
                case 27:
                    // TODO // ParametricSpecificConfig();
                    break;
                case 28:
                    // TODO // SSCSpecificConfig();();
                    break;

                case 32:
                case 33:
                case 34:
                    // TODO // MPEG_1_2_SpecificConfig();
                    break;
                case 35:
                    // TODO // DSTSpecificConfig();
                    break;
                }
            }
            if (objectTypeIndication == 0x66 || objectTypeIndication == 0x67 || objectTypeIndication == 0x68)
            {
                // For values of DecoderConfigDescriptor.objectTypeIndication that refer to streams complying with
                // ISO/IEC 13818-7 the decoder specific information consists of the ADIF -header if it is present
                // (or none if it is not present) and an access unit is a „raw_data_block()“ as defined in ISO/IEC 13818-7.
                xmlSpacer(mp4->xml, "(ISO/IEC 13818-7)", -1);
            }
            if (objectTypeIndication == 0x69)
            {
                // ISO/IEC 13818-3 the decoder specific information is empty since all necessary data is in the bitstream frames itself.
                // The access units in this case are the „frame()“ bitstream element as is defined in ISO/IEC 11172-3.
                xmlSpacer(mp4->xml, "(ISO/IEC 13818-3)", -1);
            }
            if (objectTypeIndication == 0x6C)
            {
                // class JPEG_DecoderConfig extends DecoderSpecificInfo : bit(8) tag=DecSpecificInfoTag
                xmlSpacer(mp4->xml, "JPEG_DecoderConfig", -1);
                uint16_t headerLength = read_mp4_uint16(bitstr, mp4->xml, "headerLength");
                uint16_t Xdensity = read_mp4_uint16(bitstr, mp4->xml, "Xdensity");
                uint16_t Ydensity = read_mp4_uint16(bitstr, mp4->xml, "Ydensity");
                uint8_t numComponents = read_mp4_uint8(bitstr, mp4->xml, "numComponents");
            }
        }
        else if (tag == 0x06) // SLConfigDescriptor TAG ////////////////////////
        {
            xmlSpacer(mp4->xml, "SLConfigDescriptor (0x06)", -1);

            uint8_t predefined = read_bits(bitstr, 8);

            if (mp4->xml)
            {
                fprintf(mp4->xml, "  <predefined>%u</predefined>\n", predefined);
            }
        }
/*
        else if (tag == 0x07) // ContentIdentDescrTag
        else if (tag == 0x08) // SupplContentIdentDescrTag
        else if (tag == 0x09) // IPI_DescrPointerTag
        else if (tag == 0x0A) // IPMP_DescrPointerTag
        else if (tag == 0x0B) // IPMP_DescrTag
        else if (tag == 0x0C) // QoS_DescrTag
        else if (tag == 0x0D) // RegistrationDescrTag
        else if (tag == 0x0E) // ES_ID_IncTag
        else if (tag == 0x0F) // ES_ID_RefTag
        else if (tag == 0x10) // MP4_IODTag
        else if (tag == 0x11) // MP4_ODTag
        else if (tag == 0x12) // IPL_DescrPointerRefTag
        else if (tag == 0x13) // ExtendedProfileLevelDescrTag
        else if (tag == 0x14) // profileLevelIndicationIndexDescrTag
        else if (tag == 0x40) // ContentClassificationDescrTag
        else if (tag == 0x41) // KeyWordDescrTag
        else if (tag == 0x42) // RatingDescrTag
        else if (tag == 0x43) // LanguageDescrTag
        else if (tag == 0x44) // ShortTextualDescrTag
        else if (tag == 0x45) // ExpandedTextualDescrTag
        else if (tag == 0x46) // ContentCreatorNameDescrTag
        else if (tag == 0x47) // ContentCreationDateDescrTag
        else if (tag == 0x48) // OCICreatorNameDescrTag
        else if (tag == 0x49) // OCICreationDateDescrTag
        else if (tag == 0x4A) // SmpteCameraPositionDescrTag
        {
            // TODO
        }
*/
        else
        {
            xmlSpacer(mp4->xml, "UNKNOWN TAG", -1);

            TRACE_WARNING(MP4, "esds UNKNOWN TAG %X", tag);
            if (mp4->xml) fprintf(mp4->xml, "  <UNKNOWN_TAG>0x%X</UNKNOWN_TAG>\n", tag);

            // jumpy esds
            if (bitstream_get_absolute_byte_offset(bitstr) < (tag_offset + 1 + tag_delimiters_post + tag_datasize) &&
                (tag_offset + 1 + tag_delimiters_post + tag_datasize) <= box_header->offset_end)
            {
                TRACE_1(MP4, "wrong position after esds tag %X", tag);
                TRACE_1(MP4, "pos %lli", bitstream_get_absolute_byte_offset(bitstr));
                TRACE_1(MP4, "instead of %lli", (tag_offset + 1 + tag_delimiters_post + tag_datasize));
                skip_bits(bitstr, ((tag_offset + 1 +  tag_delimiters_post + tag_datasize) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
            }
        }
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15' specification:
 * 5.2.4 Decoder configuration information.
 *
 * This subclause specifies the decoder configuration information for ISO/IEC
 * 14496-10 video content.
 * Contain AVCDecoderConfigurationRecord data structure (5.2.4.1.1 Syntax, 5.2.4.1.2 Semantics).
 */
int parse_avcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_avcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned configurationVersion = read_bits(bitstr, 8);
    unsigned AVCProfileIndication = read_bits(bitstr, 8);
    unsigned profile_compatibility = read_bits(bitstr, 8);
    unsigned AVCLevelIndication = read_bits(bitstr, 8);
    skip_bits(bitstr, 6); // reserved
    unsigned lengthSizeMinusOne = read_bits(bitstr, 2);
    skip_bits(bitstr, 3); // reserved

    // SPS
    track->sps_count = read_bits(bitstr, 5);
    track->sps_sample_offset = (int64_t *)calloc(track->sps_count, sizeof(int64_t));
    track->sps_sample_size = (unsigned int *)calloc(track->sps_count, sizeof(unsigned int));

    for (unsigned i = 0; i < track->sps_count && i < MAX_SPS; i++) // MAX_SPS = 32
    {
        track->sps_sample_size[i] = read_bits(bitstr, 16);
        track->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);
        track->sps_array[i] = (h264_sps_t *)calloc(1, sizeof(h264_sps_t));

        skip_bits(bitstr, 8); // skip NAL header
        decodeSPS(bitstr, track->sps_array[i]);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->sps_sample_offset[i] + track->sps_sample_size[i]))
        {
            TRACE_WARNING(MP4, "SPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->sps_sample_offset[i] + track->sps_sample_size[i]));

            skip_bits(bitstr, ((track->sps_sample_offset[i] + track->sps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // PPS
    track->pps_count = read_bits(bitstr, 8);
    track->pps_sample_offset = (int64_t *)calloc(track->pps_count, sizeof(int64_t));
    track->pps_sample_size = (unsigned int *)calloc(track->pps_count, sizeof(unsigned int));

    for (unsigned i = 0; i < track->pps_count && i < MAX_PPS; i++) // MAX_PPS = 256
    {
        track->pps_sample_size[i] = read_bits(bitstr, 16);
        track->pps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);
        track->pps_array[i] = (h264_pps_t *)calloc(1, sizeof(h264_pps_t));

        skip_bits(bitstr, 8); // skip NAL header
        decodePPS(bitstr, track->pps_array[i], track->sps_array);
        bitstream_force_alignment(bitstr); // we might end up parsing in the middle of a byte

        if (bitstream_get_absolute_byte_offset(bitstr) != (track->pps_sample_offset[i] + track->pps_sample_size[i]))
        {
            TRACE_WARNING(MP4, "PPS OFFSET ERROR  %lli vs %lli",
                          bitstream_get_absolute_byte_offset(bitstr),
                          (track->pps_sample_offset[i] + track->pps_sample_size[i]));
            skip_bits(bitstr, ((track->pps_sample_offset[i] + track->pps_sample_size[i]) - bitstream_get_absolute_byte_offset(bitstr)) * 8);
        }
    }

    // Handle H.264 profile & level
    if (track->sps_count > 0 && track->sps_array[0])
    {
        track->codec_profile = getH264CodecProfile(track->sps_array[0]->profile_idc,
                                                   track->sps_array[0]->constraint_setX_flag[0],
                                                   track->sps_array[0]->constraint_setX_flag[1],
                                                   track->sps_array[0]->constraint_setX_flag[2],
                                                   track->sps_array[0]->constraint_setX_flag[3],
                                                   track->sps_array[0]->constraint_setX_flag[4],
                                                   track->sps_array[0]->constraint_setX_flag[5]);
        track->codec_level = static_cast<double>(AVCLevelIndication) / 10.0;

        track->max_ref_frames = track->sps_array[0]->max_num_ref_frames;
        track->color_depth = track->sps_array[0]->BitDepthY;

        if (track->sps_array[0]->vui)
        {
            track->color_range = track->sps_array[0]->vui->video_full_range_flag;
            track->color_primaries = track->sps_array[0]->vui->colour_primaries;
            track->color_transfer = track->sps_array[0]->vui->transfer_characteristics;
            track->color_matrix = track->sps_array[0]->vui->matrix_coefficients;
        }

        if (track->pps_count && track->pps_array[0])
        {
            track->use_cabac = track->pps_array[0]->entropy_coding_mode_flag;
            track->use_8x8_blocks = track->pps_array[0]->transform_8x8_mode_flag;
        }
    }
    else
    {
        track->codec_profile = getH264CodecProfile(AVCProfileIndication);
        track->codec_level = static_cast<double>(AVCLevelIndication) / 10.0;
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> configurationVersion  : %u", configurationVersion);
    TRACE_1(MP4, "> AVCProfileIndication  : %u", AVCProfileIndication);
    TRACE_1(MP4, "> profile_compatibility : %u", profile_compatibility);
    TRACE_1(MP4, "> AVCLevelIndication    : %u", AVCLevelIndication);
    TRACE_1(MP4, "> lengthSizeMinusOne    : %u", lengthSizeMinusOne);

    TRACE_1(MP4, "> numOfSequenceParameterSets    = %u", track->sps_count);
    for (unsigned i = 0; i < track->sps_count; i++)
    {
        TRACE_1(MP4, "> sequenceParameterSetLength[%u] : %u", i, track->sps_sample_size[i]);
        TRACE_1(MP4, "> sequenceParameterSetOffset[%u] : %li", i, track->sps_sample_offset[i]);
    }

    TRACE_1(MP4, "> numOfPictureParameterSets     = %u", track->pps_count);
    for (unsigned i = 0; i < track->pps_count; i++)
    {
        TRACE_1(MP4, "> pictureParameterSetLength[%u]  : %u", i, track->pps_sample_size[i]);
        TRACE_1(MP4, "> pictureParameterSetOffset[%u]  : %li", i, track->pps_sample_offset[i]);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "AVC Configuration");
        fprintf(mp4->xml, "  <configurationVersion>%u</configurationVersion>\n", configurationVersion);
        fprintf(mp4->xml, "  <AVCProfileIndication>%u</AVCProfileIndication>\n", AVCProfileIndication);
        fprintf(mp4->xml, "  <profile_compatibility>%u</profile_compatibility>\n", profile_compatibility);
        fprintf(mp4->xml, "  <AVCLevelIndication>%u</AVCLevelIndication>\n", AVCLevelIndication);
        fprintf(mp4->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", lengthSizeMinusOne);

        fprintf(mp4->xml, "  <numOfSequenceParameterSets>%u</numOfSequenceParameterSets>\n", track->sps_count);
        for (unsigned i = 0; i < track->sps_count; i++)
        {
            xmlSpacer(mp4->xml, "SequenceParameterSet infos", i);
            fprintf(mp4->xml, "  <sequenceParameterSetLength index=\"%u\">%u</sequenceParameterSetLength>\n", i, track->sps_sample_size[i]);
            fprintf(mp4->xml, "  <sequenceParameterSetOffset index=\"%u\">%" PRId64 "</sequenceParameterSetOffset>\n", i, track->sps_sample_offset[i]);
        }

        fprintf(mp4->xml, "  <numOfPictureParameterSets>%u</numOfPictureParameterSets>\n", track->pps_count);
        for (unsigned i = 0; i < track->pps_count; i++)
        {
            xmlSpacer(mp4->xml, "PictureParameterSet", i);
            fprintf(mp4->xml, "  <pictureParameterSetLength index=\"%u\">%u</pictureParameterSetLength>\n", i, track->pps_sample_size[i]);
            fprintf(mp4->xml, "  <pictureParameterSetOffset index=\"%u\">%" PRId64 "</pictureParameterSetOffset>\n", i, track->pps_sample_offset[i]);
        }

        for (unsigned i = 0; i < track->sps_count; i++)
        {
            printPPS(track->pps_array[i], track->sps_array);

            mapSPS(track->sps_array[i],
                   track->sps_sample_offset[i],
                   track->sps_sample_size[i],
                   mp4->xml);
        }
        for (unsigned i = 0; i < track->pps_count; i++)
        {
            printPPS(track->pps_array[i], track->sps_array);

            mapPPS(track->pps_array[i],
                   track->sps_array,
                   track->pps_sample_offset[i],
                   track->pps_sample_size[i],
                   mp4->xml);
        }

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief HEVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15 (2015)' specification:
 * 8.3.3.1 Decoder configuration information.
 */
int parse_hvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_hvcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned configurationVersion = read_bits(bitstr, 8);
    unsigned general_profile_space = read_bits(bitstr, 2);
    bool general_tier_flag = read_bits(bitstr, 1);
    unsigned general_profile_idc = read_bits(bitstr, 5);
    uint32_t general_profile_compatibility_flags = read_bits(bitstr, 32);
    uint64_t general_constraint_indicator_flags = read_bits_64(bitstr, 48);
    unsigned general_level_idc = read_bits(bitstr, 8);
    skip_bits(bitstr, 4); // reserved
    unsigned min_spatial_segmentation_idc = read_bits(bitstr, 12);
    skip_bits(bitstr, 6); // reserved
    unsigned parallelismType = read_bits(bitstr, 2);
    skip_bits(bitstr, 6); // reserved
    unsigned chromaFormat = read_bits(bitstr, 2);
    skip_bits(bitstr, 5); // reserved
    unsigned bitDepthLumaMinus8 = read_bits(bitstr, 3);
    skip_bits(bitstr, 5); // reserved
    unsigned bitDepthChromaMinus8 = read_bits(bitstr, 3);

    unsigned avgFrameRate = read_bits(bitstr, 16);
    unsigned constantFrameRate = read_bits(bitstr, 2);
    unsigned numTemporalLayers = read_bits(bitstr, 3);
    bool temporalIdNested = read_bits(bitstr, 1);
    unsigned lengthSizeMinusOne = read_bits(bitstr, 1);

    unsigned numOfArrays = read_bits(bitstr, 1);
    bool *array_completeness = NULL;
    uint8_t *NAL_unit_type = NULL;
    uint16_t *numNalus = NULL;
    uint16_t **nalUnitLength = NULL;
    uint8_t ***nalUnit = NULL;

    for (unsigned i = 0; i < numOfArrays; i++)
    {
        break;

        array_completeness = (bool *)malloc(numOfArrays);
        NAL_unit_type = (uint8_t *)malloc(numOfArrays);
        numNalus = (uint16_t *)malloc(numOfArrays);
        nalUnitLength = (uint16_t **)malloc(numOfArrays);
        nalUnit = (uint8_t ***)malloc(numOfArrays);
        if (array_completeness && NAL_unit_type && numNalus && nalUnitLength && nalUnit)
        {
            for (unsigned j = 0; j < numOfArrays; j++)
            {
                array_completeness[j] = read_bits(bitstr, 1);
                skip_bits(bitstr, 1); // reserved
                NAL_unit_type[j] = read_bits(bitstr, 6); // can be VPS, SPS, PPS, or SEI NAL unit
                numNalus[j] = read_bits(bitstr, 16);

                if (numNalus[j] > 0)
                {
                    nalUnitLength[j] = (uint16_t *)malloc(numNalus[j]);
                    nalUnit[j] = (uint8_t **)malloc(numNalus[j]);
                    if (nalUnitLength[j] && nalUnit[j])
                    {
                        for (unsigned k = 0; k < numNalus[j]; k++)
                        {
                            nalUnitLength[j][k] = read_bits(bitstr, 16);
/*
                            // TODO register SPS & PPS
                            if (NAL_unit_type[j] == SPS)
                            {
                                track->sps_count = read_bits(bitstr, 5); // MAX_SPS = 32
                                track->sps_sample_offset = (int64_t*)calloc(track->sps_count, sizeof(int64_t));
                                track->sps_sample_size = (unsigned int*)calloc(track->sps_count, sizeof(unsigned int));
                                for (i = 0; i < track->sps_count; i++)
                                {
                                    track->sps_sample_size[i] = read_bits(bitstr, 16);
                                    track->sps_sample_offset[i] = bitstream_get_absolute_byte_offset(bitstr);

                                    skip_bits(bitstr, track->sps_sample_size[i] * 8); // sequenceParameterSetNALUnit
                                }
                            }
*/
                        }
                    }
                    else
                        retcode = FAILURE;
                }
            }
        }
        else
            retcode = FAILURE;
    }

    // Handle H.265 profile & level
    track->codec_profile = getH265CodecProfile(general_profile_idc);
    track->codec_level = static_cast<double>(general_level_idc) / 30.0;
    track->color_depth = bitDepthLumaMinus8 + 8;

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> configurationVersion : %u", configurationVersion);
    TRACE_1(MP4, "> general_profile_space: %u", general_profile_space);
    TRACE_1(MP4, "> general_tier_flag    : %u", general_tier_flag);
    TRACE_1(MP4, "> general_profile_idc  : %u", general_profile_idc);
    TRACE_1(MP4, "> general_profile_compatibility_flags: %u", general_profile_compatibility_flags);
    TRACE_1(MP4, "> general_constraint_indicator_flags : %lu", general_constraint_indicator_flags);
    TRACE_1(MP4, "> general_level_idc    : %u", general_level_idc);

    TRACE_1(MP4, "> min_spatial_segmentation_idc: %u", min_spatial_segmentation_idc);
    TRACE_1(MP4, "> parallelismType      : %u", parallelismType);
    TRACE_1(MP4, "> chromaFormat         : %u", chromaFormat);
    TRACE_1(MP4, "> bitDepthLumaMinus8   : %u", bitDepthLumaMinus8);
    TRACE_1(MP4, "> bitDepthChromaMinus8 : %u", bitDepthChromaMinus8);

    TRACE_1(MP4, "> avgFrameRate         : %u", avgFrameRate);
    TRACE_1(MP4, "> constantFrameRate    : %u", constantFrameRate);
    TRACE_1(MP4, "> numTemporalLayers    : %u", numTemporalLayers);
    TRACE_1(MP4, "> temporalIdNested     : %u", temporalIdNested);
    TRACE_1(MP4, "> lengthSizeMinusOne   : %u", lengthSizeMinusOne);
    TRACE_1(MP4, "> numOfArrays          : %u", numOfArrays);

    for (unsigned j = 0; j < numOfArrays; j++)
    {
        // TODO // NAL unit table
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "HEVC Configuration");
        fprintf(mp4->xml, "  <configurationVersion>%u</configurationVersion>\n", configurationVersion);
        fprintf(mp4->xml, "  <general_profile_space>%u</general_profile_space>\n", general_profile_space);
        fprintf(mp4->xml, "  <general_tier_flag>%u</general_tier_flag>\n", general_tier_flag);
        fprintf(mp4->xml, "  <general_profile_idc>%u</general_profile_idc>\n", general_profile_idc);
        fprintf(mp4->xml, "  <general_profile_compatibility_flags>%u</general_profile_compatibility_flags>\n", general_profile_compatibility_flags);
        fprintf(mp4->xml, "  <general_constraint_indicator_flags>%" PRId64 "</general_constraint_indicator_flags>\n", general_constraint_indicator_flags);
        fprintf(mp4->xml, "  <general_level_idc>%u</general_level_idc>\n", general_level_idc);

        fprintf(mp4->xml, "  <min_spatial_segmentation_idc>%u</min_spatial_segmentation_idc>\n", min_spatial_segmentation_idc);
        fprintf(mp4->xml, "  <parallelismType>%u</parallelismType>\n", parallelismType);
        fprintf(mp4->xml, "  <chromaFormat>%u</chromaFormat>\n", chromaFormat);
        fprintf(mp4->xml, "  <bitDepthLumaMinus8>%u</bitDepthLumaMinus8>\n", bitDepthLumaMinus8);
        fprintf(mp4->xml, "  <bitDepthChromaMinus8>%u</bitDepthChromaMinus8>\n", bitDepthChromaMinus8);

        fprintf(mp4->xml, "  <avgFrameRate>%u</avgFrameRate>\n", avgFrameRate);
        fprintf(mp4->xml, "  <constantFrameRate>%u</constantFrameRate>\n", constantFrameRate);
        fprintf(mp4->xml, "  <numTemporalLayers>%u</numTemporalLayers>\n", numTemporalLayers);
        fprintf(mp4->xml, "  <temporalIdNested>%u</temporalIdNested>\n", temporalIdNested);
        fprintf(mp4->xml, "  <lengthSizeMinusOne>%u</lengthSizeMinusOne>\n", lengthSizeMinusOne);

        for (unsigned j = 0; j < numOfArrays; j++)
        {
            // TODO // NAL unit table
        }

        fprintf(mp4->xml, "  </a>\n");
    }

    if (array_completeness && NAL_unit_type && numNalus && nalUnitLength && nalUnit)
    {
        for (unsigned j = 0; j < numOfArrays; j++)
        {
            if (nalUnitLength[j] && nalUnit[j])
            {
                for (unsigned i = 0; i < numNalus[j]; i++)
                {
                    //
                }

                free(nalUnitLength[j]);
                free(nalUnit[j]);
            }
        }

        free(array_completeness);
        free(NAL_unit_type);
        free(numNalus);
        free(nalUnitLength);
        free(nalUnit);
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Dolby Vision Configuration Box.
 */
int parse_dvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_dvcC()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Dolby Vision Configuration Box");

    // Parse box content
    uint8_t dv_version_major = read_mp4_uint8(bitstr, mp4->xml, "dv_version_major");
    uint8_t dv_version_minor = read_mp4_uint8(bitstr, mp4->xml, "dv_version_minor");
    uint8_t dv_profile = read_mp4_uint(bitstr, 7, mp4->xml, "dv_profile");
    uint8_t dv_level = read_mp4_uint(bitstr, 6, mp4->xml, "dv_level");
    uint8_t rpu_present_flag = read_mp4_flag(bitstr, mp4->xml, "rpu_present_flag");
    uint8_t el_present_flag = read_mp4_flag(bitstr, mp4->xml, "el_present_flag");
    uint8_t bl_present_flag = read_mp4_flag(bitstr, mp4->xml, "bl_present_flag");

    if (bl_present_flag)
    {
        uint8_t dv_bl_signal_compatibility_id = read_mp4_uint(bitstr, 4, mp4->xml, "dv_bl_signal_compatibility_id");
    }

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief VVC Configuration Box.
 *
 * From 'ISO/IEC 14496-15 (2015)' specification:
 * 8.3.3.1 Decoder configuration information.
 */
int parse_vvcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_vvcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    // TODO

#if ENABLE_DEBUG
    print_box_header(box_header);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "VVC Configuration");

        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief vpcC Configuration Box.
 *
 * From 'VP Codec ISO Media File Format Binding':
 * VP Codec Sample Entry Box
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_vpcC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_vpcC()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned profile = read_bits(bitstr, 8);
    unsigned level = read_bits(bitstr, 8);
    unsigned bitDepth = read_bits(bitstr, 4);
    unsigned chromaSubsampling = read_bits(bitstr, 3);
    bool videoFullRangeFlag = read_bits(bitstr, 1);
    unsigned colourPrimaries = read_bits(bitstr, 8);
    unsigned transferCharacteristics = read_bits(bitstr, 8);
    unsigned matrixCoefficients = read_bits(bitstr, 8);

    unsigned codecIntializationDataSize = read_bits(bitstr, 16);
    if (codecIntializationDataSize > 0)
    {
        // Note: must be 0 for VP8 and VP9
        // TODO // unsigned codecIntializationData = read_bits(bitstr, 8);
    }

    // Handle VPx profile & level
    if (track->codec == CODEC_VP8)
    {
        if (profile == 0)
            track->codec_profile = PROF_VP8_0;
        else if (profile == 1)
            track->codec_profile = PROF_VP8_1;
    }
    if (track->codec == CODEC_VP9)
    {
        if (profile == 0)
            track->codec_profile = PROF_VP9_0;
        else if (profile == 1)
            track->codec_profile = PROF_VP9_1;
        else if (profile == 2)
            track->codec_profile = PROF_VP9_2;
        else if (profile == 3)
            track->codec_profile = PROF_VP9_3;
    }

    track->codec_level = static_cast<double>(level) / 10.0;

    track->color_depth = bitDepth;
    track->color_range = videoFullRangeFlag;

    track->color_primaries = colourPrimaries;
    track->color_transfer = transferCharacteristics;
    track->color_matrix = matrixCoefficients;

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> profile : %u", profile);
    TRACE_1(MP4, "> level: %u", level);
    TRACE_1(MP4, "> bitDepth: %u", bitDepth);
    TRACE_1(MP4, "> chromaSubsampling: %u", chromaSubsampling);
    TRACE_1(MP4, "> videoFullRangeFlag: %u", videoFullRangeFlag);
    TRACE_1(MP4, "> colourPrimaries: %u", colourPrimaries);
    TRACE_1(MP4, "> transferCharacteristics: %u", transferCharacteristics);
    TRACE_1(MP4, "> matrixCoefficients: %u", matrixCoefficients);

    TRACE_1(MP4, "> codecIntializationDataSize: %u", codecIntializationDataSize);
    if (codecIntializationDataSize)
    {
        // TODO
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "vpcC Configuration");
        fprintf(mp4->xml, "  <profile>%u</profile>\n", profile);
        fprintf(mp4->xml, "  <level>%u</level>\n", level);
        fprintf(mp4->xml, "  <bitDepth>%u</bitDepth>\n", bitDepth);
        fprintf(mp4->xml, "  <chromaSubsampling>%u</chromaSubsampling>\n", chromaSubsampling);
        fprintf(mp4->xml, "  <videoFullRangeFlag>%u</videoFullRangeFlag>\n", videoFullRangeFlag);
        fprintf(mp4->xml, "  <colourPrimaries>%u</colourPrimaries>\n", colourPrimaries);
        fprintf(mp4->xml, "  <transferCharacteristics>%u</transferCharacteristics>\n", transferCharacteristics);
        fprintf(mp4->xml, "  <matrixCoefficients>%u</matrixCoefficients>\n", matrixCoefficients);

        fprintf(mp4->xml, "  <codecIntializationDataSize>%u</codecIntializationDataSize>\n", codecIntializationDataSize);
        if (codecIntializationDataSize)
        {
            // TODO
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief AV1C Configuration Box.
 *
 * From 'AV1 Codec ISO Media File Format Binding':
 * v1.2.0, 12 December 2019
 * 2.3. AV1 Codec Configuration Box
 * - https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox
 */
int parse_av1C(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_av1C()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    bool marker = read_bit(bitstr);
    unsigned version = read_bits(bitstr, 7);
    unsigned seq_profile = read_bits(bitstr, 3);
    unsigned seq_level_idx_0 = read_bits(bitstr, 5);
    bool seq_tier_0 = read_bit(bitstr);
    bool high_bitdepth = read_bit(bitstr);
    bool twelve_bit = read_bit(bitstr);
    bool monochrome = read_bit(bitstr);
    bool chroma_subsampling_x = read_bit(bitstr);
    bool chroma_subsampling_y = read_bit(bitstr);
    unsigned chroma_sample_position = read_bits(bitstr, 2);
    read_bits(bitstr, 3); // reserved

    bool initial_presentation_delay_present = read_bit(bitstr);
    unsigned int initial_presentation_delay_minus_one = 0;
    if (initial_presentation_delay_present)
    {
        initial_presentation_delay_minus_one = read_bits(bitstr, 4);
    }
    else
    {
        read_bits(bitstr, 4); // reserved
    }

    // TODO // Parse OBUs
    //unsigned int (8)[] configOBUs = read_bits(bitstr, 8);

    // Handle AV1 profile & level
    if (seq_profile == 0)
        track->codec_profile = PROF_AV1_Main;
    else if (seq_profile == 1)
        track->codec_profile = PROF_AV1_High;
    else if (seq_profile == 2)
        track->codec_profile = PROF_AV1_Professional;

    if (seq_level_idx_0 == 0) track->codec_level = 2.0;
    if (seq_level_idx_0 == 1) track->codec_level = 2.1;
    if (seq_level_idx_0 == 2) track->codec_level = 2.2;
    if (seq_level_idx_0 == 3) track->codec_level = 2.3;
    if (seq_level_idx_0 == 4) track->codec_level = 3.0;
    if (seq_level_idx_0 == 5) track->codec_level = 3.1;
    if (seq_level_idx_0 == 6) track->codec_level = 3.1;
    if (seq_level_idx_0 == 7) track->codec_level = 3.1;
    if (seq_level_idx_0 == 8) track->codec_level = 4.0;
    if (seq_level_idx_0 == 9) track->codec_level = 4.1;
    if (seq_level_idx_0 == 10) track->codec_level = 4.2;
    if (seq_level_idx_0 == 11) track->codec_level = 4.3;
    if (seq_level_idx_0 == 12) track->codec_level = 5.0;
    if (seq_level_idx_0 == 13) track->codec_level = 5.1;
    if (seq_level_idx_0 == 14) track->codec_level = 5.2;
    if (seq_level_idx_0 == 15) track->codec_level = 5.3;
    if (seq_level_idx_0 == 16) track->codec_level = 6.0;
    if (seq_level_idx_0 == 17) track->codec_level = 6.1;
    if (seq_level_idx_0 == 18) track->codec_level = 6.2;
    if (seq_level_idx_0 == 19) track->codec_level = 6.3;
    if (seq_level_idx_0 == 20) track->codec_level = 7.0;
    if (seq_level_idx_0 == 21) track->codec_level = 7.1;
    if (seq_level_idx_0 == 22) track->codec_level = 7.2;
    if (seq_level_idx_0 == 23) track->codec_level = 7.3;

    if (high_bitdepth) track->color_depth = 10;
    else if (twelve_bit) track->color_depth = 12;
    else track->color_depth = 8;

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> version : %u", version);
    TRACE_1(MP4, "> seq_profile: %u", seq_profile);
    TRACE_1(MP4, "> seq_level_idx_0: %u", seq_level_idx_0);
    TRACE_1(MP4, "> seq_tier_0: %u", seq_tier_0);
    TRACE_1(MP4, "> high_bitdepth: %u", high_bitdepth);
    TRACE_1(MP4, "> twelve_bit: %u", twelve_bit);
    TRACE_1(MP4, "> monochrome: %u", monochrome);
    TRACE_1(MP4, "> chroma_subsampling_x: %u", chroma_subsampling_x);
    TRACE_1(MP4, "> chroma_subsampling_y: %u", chroma_subsampling_y);
    TRACE_1(MP4, "> chroma_sample_position: %u", chroma_sample_position);
    TRACE_1(MP4, "> initial_presentation_delay_present: %u", initial_presentation_delay_present);
    if (initial_presentation_delay_present)
    {
        TRACE_1(MP4, "> initial_presentation_delay_minus_one: %u", initial_presentation_delay_minus_one);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "AV1C Configuration");
        fprintf(mp4->xml, "  <version>%u</version>\n", version);
        fprintf(mp4->xml, "  <seq_profile>%u</seq_profile>\n", seq_profile);
        fprintf(mp4->xml, "  <seq_level_idx_0>%u</seq_level_idx_0>\n", seq_level_idx_0);
        fprintf(mp4->xml, "  <seq_tier_0>%u</seq_tier_0>\n", seq_tier_0);
        fprintf(mp4->xml, "  <high_bitdepth>%u</high_bitdepth>\n", high_bitdepth);
        fprintf(mp4->xml, "  <twelve_bit>%i</twelve_bit>\n", twelve_bit);
        fprintf(mp4->xml, "  <monochrome>%u</monochrome>\n", monochrome);
        fprintf(mp4->xml, "  <chroma_subsampling_x>%u</chroma_subsampling_x>\n", chroma_subsampling_x);
        fprintf(mp4->xml, "  <chroma_subsampling_y>%u</chroma_subsampling_y>\n", chroma_subsampling_y);
        fprintf(mp4->xml, "  <chroma_sample_position>%u</chroma_sample_position>\n", chroma_sample_position);
        fprintf(mp4->xml, "  <initial_presentation_delay_present>%u</initial_presentation_delay_present>\n", initial_presentation_delay_present);
        if (initial_presentation_delay_present)
        {
            fprintf(mp4->xml, "  <initial_presentation_delay_minus_one>%u</initial_presentation_delay_minus_one>\n", initial_presentation_delay_minus_one);
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief JPGC Configuration Box.
 *
 * From 'AV1 Codec ISO Media File Format Binding':
 * v1.2.0, 12 December 2019
 * 2.3. AV1 Codec Configuration Box
 * - https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox
 */
int parse_jpgC(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_jpgC()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
#if ENABLE_DEBUG
    print_box_header(box_header);
    // TODO
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "JPGC Configuration");
        // TODO
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief H.263 configuration box ('d263')
 *
 * 3GPP TS 26.244 specification
 */
int parse_d263(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_d263()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "H.263 parameters");

    char *vendor = read_mp4_string(bitstr, 4, mp4->xml, "vendor");
    free(vendor);

    uint8_t decoder_version = read_mp4_uint8(bitstr, mp4->xml, "decoder_version");
    uint8_t level = read_mp4_uint8(bitstr, mp4->xml, "level");
    uint8_t profil = read_mp4_uint8(bitstr, mp4->xml, "profil");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Bitrate Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_btrt(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_btrt()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    unsigned int bufferSizeDB = read_bits(bitstr, 32);
    track->bitrate_max = read_bits(bitstr, 32);
    track->bitrate_avg = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> bufferSizeDB : %u", bufferSizeDB);
    TRACE_1(MP4, "> maxBitrate   : %u", track->bitrate_max);
    TRACE_1(MP4, "> avgBitrate   : %u", track->bitrate_avg);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Bitrate");
        fprintf(mp4->xml, "  <bufferSizeDB>%u</bufferSizeDB>\n", bufferSizeDB);
        fprintf(mp4->xml, "  <bitrate_max>%u</bitrate_max>\n", track->bitrate_max);
        fprintf(mp4->xml, "  <bitrate_avg>%u</bitrate_avg>\n", track->bitrate_avg);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Clean Aperture Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_clap(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_clap()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    //unsigned int clap_size = read_bits(bitstr, 32);
    //unsigned int clap_type = read_bits(bitstr, 32);

    unsigned int cleanApertureWidthN = read_bits(bitstr, 32);
    unsigned int cleanApertureWidthD = read_bits(bitstr, 32);
    unsigned int cleanApertureHeightN = read_bits(bitstr, 32);
    unsigned int cleanApertureHeightD = read_bits(bitstr, 32);
    unsigned int horizOffN = read_bits(bitstr, 32);
    unsigned int horizOffD = read_bits(bitstr, 32);
    unsigned int vertOffN = read_bits(bitstr, 32);
    unsigned int vertOffD = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    //TRACE_1(MP4, "> clap_size   : %u", clap_size);
    //TRACE_1(MP4, "> clap_type   : %u", clap_type);
    TRACE_1(MP4, "> cleanApertureWidthN   : %u", cleanApertureWidthN);
    TRACE_1(MP4, "> cleanApertureWidthD   : %u", cleanApertureWidthD);
    TRACE_1(MP4, "> cleanApertureHeightN  : %u", cleanApertureHeightN);
    TRACE_1(MP4, "> cleanApertureHeightD  : %u", cleanApertureHeightD);
    TRACE_1(MP4, "> horizOffN  : %u", horizOffN);
    TRACE_1(MP4, "> horizOffD  : %u", horizOffD);
    TRACE_1(MP4, "> vertOffN   : %u", vertOffN);
    TRACE_1(MP4, "> vertOffD   : %u", vertOffD);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Clean Aperture");
        fprintf(mp4->xml, "  <cleanApertureWidthN>%u</cleanApertureWidthN>\n", cleanApertureWidthN);
        fprintf(mp4->xml, "  <cleanApertureWidthD>%u</cleanApertureWidthD>\n", cleanApertureWidthD);
        fprintf(mp4->xml, "  <cleanApertureHeightN>%u</cleanApertureHeightN>\n", cleanApertureHeightN);
        fprintf(mp4->xml, "  <cleanApertureHeightD>%u</cleanApertureHeightD>\n", cleanApertureHeightD);
        fprintf(mp4->xml, "  <horizOffN>%u</horizOffN>\n", horizOffN);
        fprintf(mp4->xml, "  <horizOffD>%u</horizOffD>\n", horizOffD);
        fprintf(mp4->xml, "  <vertOffN>%u</vertOffN>\n", vertOffN);
        fprintf(mp4->xml, "  <vertOffD>%u</vertOffD>\n", vertOffD);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Colour Information Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 *
 * - https://developer.apple.com/library/mac/technotes/tn2227/_index.html
 */
int parse_colr(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_colr()" CLR_RESET);
    int retcode = SUCCESS;
    char fcc[5];

    // Parse box content
    unsigned int colour_type = read_bits(bitstr, 32);
    unsigned int colour_primaries = 0;
    unsigned int transfer_characteristics = 0;
    unsigned int matrix_coefficients = 0;
    bool colour_range = 0;

    if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
    {
        colour_primaries = read_bits(bitstr, 16);
        transfer_characteristics = read_bits(bitstr, 16);
        matrix_coefficients = read_bits(bitstr, 16);

        if (colour_type == fourcc_be("nclx"))
        {
            colour_range = read_bit(bitstr);
            /*unsigned int reserved =*/ read_bits(bitstr, 7);

            if (track->color_range) track->color_range = colour_range;
        }

        if (track->color_matrix == 0 && track->color_matrix && track->color_matrix)
        {
            track->color_primaries = colour_primaries;
            track->color_transfer = transfer_characteristics;
            track->color_matrix = colour_primaries;
        }
    }
    else if (colour_type == fourcc_be("rICC"))
    {
        // TODO // restricted ICC profile
    }
    else if (colour_type == fourcc_be("prof"))
    {
        // TODO // unrestricted ICC profile
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> colour_type             : %s", getFccString_le(colour_type, fcc));
    if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
    {
        TRACE_1(MP4, "> colour_primaries        : %u", colour_primaries);
        TRACE_1(MP4, "> transfer_characteristics: %u", transfer_characteristics);
        TRACE_1(MP4, "> matrix_coefficients     : %u", matrix_coefficients);
    }
    if (colour_type == fourcc_be("nclx"))
    {
        TRACE_1(MP4, "> full_range_flag         : %u", track->color_range);
    }
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Colour Information");
        fprintf(mp4->xml, "  <colour_type>%s</colour_type>\n", getFccString_le(colour_type, fcc));
        if (colour_type == fourcc_be("nclc") || colour_type == fourcc_be("nclx"))
        {
            fprintf(mp4->xml, "  <colour_primaries>%u</colour_primaries>\n", colour_primaries);
            fprintf(mp4->xml, "  <transfer_characteristics>%u</transfer_characteristics>\n", transfer_characteristics);
            fprintf(mp4->xml, "  <matrix_coefficients>%u</matrix_coefficients>\n", matrix_coefficients);
        }
        if (colour_type == fourcc_be("nclx"))
        {
            fprintf(mp4->xml, "  <full_range_flag>%u</full_range_flag>\n", track->color_range);
        }
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Content Light Level Box.
 *
 * From 'VP Codec ISO Media File Format Binding' specification:
 * Carriage of HDR Metadata
 * Content Light Level Box
 *
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_coll(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_coll()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int maxCLL = read_bits(bitstr, 16);
    unsigned int maxFALL = read_bits(bitstr, 16);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> maxCLL : %u", maxCLL);
    TRACE_1(MP4, "> maxFALL: %u", maxFALL);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Content Light Level");
        fprintf(mp4->xml, "  <maxCLL>%u</maxCLL>\n", maxCLL);
        fprintf(mp4->xml, "  <maxFALL>%u</maxFALL>\n", maxFALL);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}
/* ************************************************************************** */

/*!
 * \brief SMPTE-2086 Mastering Display Metadata Box.
 *
 * From 'VP Codec ISO Media File Format Binding' specification:
 * Carriage of HDR Metadata
 * SMPTE-2086 Mastering Display Metadata Box
 *
 * - https://www.webmproject.org/vp9/mp4/
 */
int parse_smdm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_smdm()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned primaryRChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryRChromaticity_y = read_bits(bitstr, 16);
    unsigned primaryGChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryGChromaticity_y = read_bits(bitstr, 16);
    unsigned primaryBChromaticity_x = read_bits(bitstr, 16);
    unsigned primaryBChromaticity_y = read_bits(bitstr, 16);
    unsigned whitePointChromaticity_x = read_bits(bitstr, 16);
    unsigned whitePointChromaticity_y = read_bits(bitstr, 16);
    unsigned luminanceMax = read_bits(bitstr, 32);
    unsigned luminanceMin = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> primaryRChromaticity_x: %u", primaryRChromaticity_x);
    TRACE_1(MP4, "> primaryRChromaticity_y: %u", primaryRChromaticity_y);
    TRACE_1(MP4, "> primaryGChromaticity_x: %u", primaryGChromaticity_x);
    TRACE_1(MP4, "> primaryGChromaticity_y: %u", primaryGChromaticity_y);
    TRACE_1(MP4, "> primaryBChromaticity_x: %u", primaryBChromaticity_x);
    TRACE_1(MP4, "> primaryBChromaticity_y: %u", primaryBChromaticity_y);
    TRACE_1(MP4, "> whitePointChromaticity_x: %u", whitePointChromaticity_x);
    TRACE_1(MP4, "> whitePointChromaticity_y: %u", whitePointChromaticity_y);
    TRACE_1(MP4, "> luminanceMax: %u", luminanceMax);
    TRACE_1(MP4, "> luminanceMin: %u", luminanceMin);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "SMPTE-2086 Mastering Display Metadata");
        fprintf(mp4->xml, "  <primaryRChromaticity_x>%u</primaryRChromaticity_x>\n", primaryRChromaticity_x);
        fprintf(mp4->xml, "  <primaryRChromaticity_y>%u</primaryRChromaticity_y>\n", primaryRChromaticity_y);
        fprintf(mp4->xml, "  <primaryGChromaticity_x>%u</primaryGChromaticity_x>\n", primaryGChromaticity_x);
        fprintf(mp4->xml, "  <primaryGChromaticity_y>%u</primaryGChromaticity_y>\n", primaryGChromaticity_y);
        fprintf(mp4->xml, "  <primaryBChromaticity_x>%u</primaryBChromaticity_x>\n", primaryBChromaticity_x);
        fprintf(mp4->xml, "  <primaryBChromaticity_y>%u</primaryBChromaticity_y>\n", primaryBChromaticity_y);
        fprintf(mp4->xml, "  <whitePointChromaticity_x>%u</whitePointChromaticity_x>\n", whitePointChromaticity_x);
        fprintf(mp4->xml, "  <whitePointChromaticity_y>%u</whitePointChromaticity_y>\n", whitePointChromaticity_y);
        fprintf(mp4->xml, "  <luminanceMax>%u</luminanceMax>\n", luminanceMax);
        fprintf(mp4->xml, "  <luminanceMin>%u</luminanceMin>\n", luminanceMin);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Field information box.
 *
 * Quick Time File Format - Video Sample Description Extensions
 */
int parse_fiel(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_fiel()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Field information");

    uint8_t fieldCount = read_mp4_uint8(bitstr, mp4->xml, "fieldCount");
    uint8_t fieldOrder = read_mp4_uint8(bitstr, mp4->xml, "fieldOrder");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Gamma box.
 *
 * Quick Time File Format - Video Sample Description Extensions
 */
int parse_gama(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_gama()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Gamma");

    read_mp4_f1616(bitstr, 1, mp4->xml, "GammaValue");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Chromaticity box.
 */
int parse_chrm(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_chrm()" CLR_RESET);
    int retcode = SUCCESS;

    print_box_header(box_header);
    write_box_header(box_header, mp4->xml, "Chromaticity");

    if (mp4->xml) fprintf(mp4->xml, "  </a>\n");

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Padding Bits box - FullBox.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.7.6 Padding Bits Box.
 */
int parse_padb(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_padb()" CLR_RESET);
    int retcode = SUCCESS;

    // Read FullBox attributes
    box_header->version = (uint8_t)read_bits(bitstr, 8);
    box_header->flags = read_bits(bitstr, 24);

    // Parse box content
    unsigned int i;
    unsigned int sample_count = read_bits(bitstr, 32);

    for (i = 0; i < ((sample_count + 1)/2); i++)
    {
        /*const int reserved1 =*/ read_bit(bitstr);
        /*int pad1 =*/ read_bits(bitstr, 3);
        /*const int reserved2 =*/ read_bit(bitstr);
        /*int pad2 =*/ read_bits(bitstr, 3);
    }

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> sample_count  : %u", sample_count);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Padding Bits");
        fprintf(mp4->xml, "  <sample_count>%u</sample_count>\n", sample_count);
        fprintf(mp4->xml, "  </a>\n");
    }

    return retcode;
}

/* ************************************************************************** */

/*!
 * \brief Pixel Aspect Ratio Box.
 *
 * From 'ISO/IEC 14496-12' specification:
 * 8.5.2 Sample Description Box
 * 8.5.2.2 Syntax
 * 8.5.2.3 Semantics
 */
int parse_pasp(Bitstream_t *bitstr, Mp4Box_t *box_header, Mp4Track_t *track, Mp4_t *mp4)
{
    TRACE_INFO(MP4, BLD_GREEN "parse_pasp()" CLR_RESET);
    int retcode = SUCCESS;

    // Parse box content
    track->par_h = read_bits(bitstr, 32);
    track->par_v = read_bits(bitstr, 32);

#if ENABLE_DEBUG
    print_box_header(box_header);
    TRACE_1(MP4, "> hSpacing  : %u", track->par_h);
    TRACE_1(MP4, "> vSpacing  : %u", track->par_v);
#endif // ENABLE_DEBUG

    // xmlMapper
    if (mp4->xml)
    {
        write_box_header(box_header, mp4->xml, "Pixel Aspect Ratio");
        fprintf(mp4->xml, "  <hSpacing>%u</hSpacing>\n", track->par_h);
        fprintf(mp4->xml, "  <vSpacing>%u</vSpacing>\n", track->par_v);
        fprintf(mp4->xml, "  </a>\n");
    }

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
