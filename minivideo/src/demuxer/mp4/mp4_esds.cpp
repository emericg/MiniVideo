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
 * \file      mp4_esds.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2024
 */

// minivideo headers
#include "mp4_stsd.h"
#include "mp4_box.h"
#include "mp4_struct.h"
#include "../xml_mapper.h"
#include "../../bitstream.h"
#include "../../bitstream_utils.h"
#include "../../minitraces.h"

// C standard libraries
#include <cstdio>
#include <cstdlib>

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

        //xmlSpacer(mp4->xml, "tag sz ()", tag_datasize);
        //TRACE_ERROR(MP4, "esds NEXT TAG %X (size: %i) @ %lli", tag, tag_datasize, tag_offset);

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

            // This object type shall be used for all streamTypes defined in ISO/IEC 14496-1 except IPMP streams.
            switch (objectTypeIndication)
            {
            //case 0x01:
            //case 0x02: // Systems ISO/IEC 14496-1
            case 0x20: // Visual ISO/IEC 14496-2
                track->codec = CODEC_MPEG4_ASP;
                break;
            case 0x40: // Audio ISO/IEC 14496-3
                // Need futher infos
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
            case 0x67: // Audio ISO/IEC 13818-7 Low Complexity Profile
                track->codec = CODEC_AAC;
                track->codec_profile = PROF_AAC_LC;
                break;
            case 0x68: // Audio ISO/IEC 13818-7 Scalable Sampling Rate Profile
                track->codec = CODEC_AAC;
                track->codec_profile = PROF_AAC_SSR;
                break;
            case 0x69: // Audio ISO/IEC 13818-3
                // Need futher infos
                track->codec = CODEC_MPEG_L3;
                break;
            case 0x6A: // Visual ISO/IEC 11172-2
                track->codec = CODEC_MPEG1;
                break;
            case 0x6B: // Audio ISO/IEC 11172-3
                // Need futher infos
                track->codec = CODEC_MPEG_L1;
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
                    track->codec = CODEC_MPEG4_TwinVQ;
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
                    track->codec_profile = CODEC_MPEG_L1;
                    break;
                case 33:
                    track->codec_profile = CODEC_MPEG_L2;
                    break;
                case 34:
                    track->codec_profile = CODEC_MPEG_L3;
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
