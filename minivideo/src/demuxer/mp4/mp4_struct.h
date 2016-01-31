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
 * \file      mp4_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_MP4_STRUCT_H
#define PARSER_MP4_STRUCT_H

// minivideo headers
#include "../../typedef.h"

/* ************************************************************************** */

//! MP4 Box header structure
typedef struct Mp4Box_t
{
    int64_t offset_start;   //!< Absolute position of the first byte of this box
    int64_t offset_end;     //!< Absolute position of the last byte of this box

    // Box parameters
    int64_t size;           //!< Box size in bytes, including all its fields and contained boxes
    uint32_t boxtype;       //!< A fourCC identifying the box type, see ISO/IEC 14496-12 Table 1
    uint8_t usertype[16];   //!< 16 bytes "extended type"; its bloat but I don't want to handle each potential deallocation

    // FullBox parameters
    uint8_t version;        //!< Specifies the version of the format used by this box (used for compatibility)
    uint32_t flags;         //!< 24b bitfield

} Mp4Box_t;

/* ************************************************************************** */
/* ************************************************************************** */

//! Structure for video tracks data
typedef struct Mp4Track_t
{
    unsigned int id;
    unsigned int fcc;
    unsigned int codec;
    unsigned int handlerType;

    char name[128];
    char compressor[32];

    uint32_t timescale;
    uint32_t mediatime;
    uint64_t duration;
    uint64_t creation_time;
    uint64_t modification_time;
    uint8_t language[3];

    unsigned int bitrate_max;
    unsigned int bitrate_avg;

        // Audio specific parameters
        unsigned int channel_count;
        unsigned int sample_size;
        unsigned int sample_rate;

        // Video specific parameters
        unsigned int width;
        unsigned int height;
        unsigned int color_depth;

        // AVC specific parameters
        unsigned int profile;
        unsigned int level;

        unsigned int sps_count;
        unsigned int *sps_sample_size;
        int64_t *sps_sample_offset;
        unsigned int pps_count;
        unsigned int *pps_sample_size;
        int64_t *pps_sample_offset;

    // stss
    unsigned int stss_entry_count;                //!< IDR frame count
    unsigned int *stss_sample_number;

    // stsc
    unsigned int stsc_entry_count;
    unsigned int *stsc_first_chunk;
    unsigned int *stsc_samples_per_chunk;         //!< An integer that gives the number of samples in each chunk
    unsigned int *stsc_sample_description_index;

    // stsz / stz2
    unsigned int stsz_sample_count;               //!< Frame count
    unsigned int stsz_sample_size;
    unsigned int *stsz_entry_size;

    // stco / co64
    unsigned int stco_entry_count;
    int64_t *stco_chunk_offset;

} Mp4Track_t;

typedef struct Mp4_t
{
    bool run; //!< A convenient way to stop the parser from any sublevel

    uint32_t timescale;
    uint64_t duration;
    uint64_t creation_time;
    uint64_t modification_time;

    uint32_t tracks_count;
    Mp4Track_t *tracks[16];

} Mp4_t;

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \enum Mp4BoxType_e
 * \brief Identifies the box type.
 *
 * The field 'type' in a box header identifies the box type; standard boxes use
 * a compact type, which is normally four printable characters, to permit ease
 * of identification, and is shown so in the boxes below. User extensions use
 * an extended type; in this case, the type field is set to 'uuid'.
 *
 * The boxes marked with an * are mandatory.
 */
typedef enum Mp4BoxType_e
{
    BOX_FTYP = 0x66747970,                      //!< (*) file type and compatibility
    BOX_PDIN = 0x7064696E,                      //!< progressive download information

    BOX_UDTA = 0x75647461,                      //!< user data box

    BOX_MOOV = 0x6D6F6F76,                      //!< (*) container for all metadata
        BOX_MVHD = 0x6D766864,                  //!< (*) movie header, overall declarations
        BOX_IODS = 0x696F6473,                  //!<
        BOX_TRAK = 0x7472616B,                  //!< (*) container for individual track or stream
            BOX_TKHD = 0x746b6864,              //!< (*) track header, overall information about the track
            BOX_TREF = 0x74726566,              //!< track reference container
            BOX_TRGR = 0x74726772,              //!< track grouping indication
            BOX_EDTS = 0x65647473,              //!< edit list container
                BOX_ELST = 0x656C7374,          //!< an edit list
            BOX_MDIA = 0x6D646961,              //!< (*) container for all the media information in a track
                BOX_MDHD = 0x6D646864,          //!< (*) media header, overall information about the media
                BOX_HDLR = 0x68646C72,          //!< (*) handler, declares the media (handler) type
                BOX_MINF = 0x6D696E66,          //!< (*) media information container
                    BOX_VMHD = 0x766D6864,      //!< video media header
                    BOX_SMHD = 0x736D6864,      //!< sound media header
                    BOX_HMHD = 0x686D6864,      //!< hint media header
                    BOX_NMHD = 0x6E6D6864,      //!< null media header
                    BOX_DINF = 0x64696E66,              //!< (*) data information box, container
                        BOX_DREF = 0x64726566,          //!< (*) data reference box, declares source(s) of media data in track
                            BOX_URL = 0x75726C20,       //!<
                    BOX_STBL = 0x7374626C,              //!< (*) sample table box, container for the time/space map
                        BOX_STSD = 0x73747364,          //!< (*) sample descriptions (codec types, initialization, etc)
                            BOX_AVCC = 0x61766343,      //!< AVC configuration box
                            BOX_BTRT = 0x62747274,      //!< bitrate box
                            BOX_CLAP = 0x636C6170,      //!< clean aperture box
                            BOX_COLR = 0x636f6C72,      //!< color infos box
                            BOX_PASP = 0x70617370,      //!< pixel aspect ratio box
                        BOX_STTS = 0x73747473,          //!< (*) (decoding) time to sample
                        BOX_CTTS = 0x63747473,          //!< (composition / presentation) time to sample
                        BOX_CSLG = 0x63736c67,          //!< composition to decode timeline mapping
                        BOX_STSC = 0x73747363,          //!< (*) sample to chunk, partial data-offset information
                        BOX_STSZ = 0x7374737A,          //!< sample sizes (framing)
                        BOX_STZ2 = 0x73747A32,          //!< compact sample sizes (framing)
                        BOX_STCO = 0x7374636F,          //!< (*) chunk offset, partial data-offset information
                        BOX_CO64 = 0x636F3634,          //!< 64b chunk offset
                        BOX_STSS = 0x73747373,          //!< sync sample table (random access points)
                        BOX_STSH = 0x73747368,          //!< shadow sync sample table
                        BOX_PADB = 0x70616462,          //!< sample padding bits
                        BOX_STDP = 0x73746470,          //!< sample degradation priority
                        BOX_SDTP = 0x73647470,          //!< independent and disposable samples
                        BOX_SBGP = 0x73626770,          //!< sample-to-group
                        BOX_SGPD = 0x73677064,          //!< sample group description
                        BOX_SUBS = 0x73756273,          //!< sub-sample information
                        BOX_SAIZ = 0x7361697A,          //!< sample auxiliary information sizes
                        BOX_SAIO = 0x7375626F,          //!< sample auxiliary information offsets

    BOX_MOOF = 0x6D6F6F66,                      //!< movie fragment
        BOX_MFHD = 0x6D666864,                  //!< (*) movie fragment header
            BOX_TRAF = 0x74726166,              //!< track fragment
                BOX_TFHD = 0x74666866,          //!< (*) track fragment header
                BOX_TRUN = 0x7472756E,          //!< track fragment run
                BOX_TFDT = 0x74666474,          //!< track fragment decode time

    BOX_MFRA = 0x6D667261,                      //!< movie fragment random access
        BOX_TFRA = 0x74667261,                  //!< track fragment random access
        BOX_MFRO = 0x6d66726f,                  //!< (*) movie fragment random access offset

    BOX_MDAT = 0x6D646174,                      //!< media data container

    BOX_META = 0x6D657461,                      //!< metadata
        // TODO

    BOX_MECO = 0x6D65636F,                      //!< additional metadata container
        BOX_MERE = 0x6D657265,                  //!< metabox relation

    BOX_STYP = 0x73747970,                      //!< segment type
    BOX_SIDX = 0x73696478,                      //!< segment index, provides a compact index of one media stream
    BOX_SSIX = 0x73736978,                      //!< subsegment index
    BOX_PRFT = 0x70726674,                      //!< producer reference time

    BOX_FREE = 0x66726565,                      //!< free space
    BOX_SKIP = 0x736B6970,                      //!< free space
    BOX_UUID = 0x75756964                       //!< user data box

} Mp4BoxType_e;

/*!
 * \enum Mp4HandlerType_e
 * \brief Identifies the content of a track.
 *
 * There are four handler type: audio, video, hint and meta.
 */
typedef enum Mp4HandlerType_e
{
    HANDLER_AUDIO = 0x736F756E, //!< 'soun'
    HANDLER_VIDEO = 0x76696465, //!< 'vide'
    HANDLER_HINT  = 0x68696E74, //!< 'hint'
    HANDLER_META  = 0x6D657461  //!< 'meta'

    // sdsm // SceneDescriptionStream
    // odsm // ObjectDescriptorStream

} Mp4HandlerType_e;

/*!
 * \enum Mp4SampleEntry_e
 * \brief Different sample values for H.264 video.
 */
typedef enum Mp4SampleEntry_e
{
    SAMPLE_AVC1 = 0x61766331,    //!< H.264 ('avc1')
    SAMPLE_AVCC = 0x61766343,

    SAMPLE_HVC1 = 0x68766331,    //!< H.265
    SAMPLE_HEVC = 0x68657663,

    SAMPLE_CFHD = 0x43464844,

    SAMPLE_MP4V = 0x6D703476,    //!< XVID ('mp4v')
    SAMPLE_MP4A = 0x6D703461,    //!< AAC ('mp4a')
    SAMPLE_AC3  = 0x61632D33,    //!< AC-3 ('ac-3')

    SAMPLE_RAW_ = 0x72617720,    //!< unsigned linear PCM (16-bit, little endian)
    SAMPLE_TOWS = 0x746F7773,    //!< signed linear PCM (big endian)
    SAMPLE_SWOT = 0x73776F74,    //!< signed linear PCM (little endian)
/*
    SAMPLE_in24 = 'in24', //!< 24-bit, big endian, linear PCM
    SAMPLE_in32 = 'in32', //!< 32-bit, big endian, linear PCM
    SAMPLE_fl32 = 'fl32', //!< 32-bit floating point PCM (Presumably IEEE 32-bit; byte order?)
    SAMPLE_fl64 = 'fl64', //!< 64-bit floating point PCM (Presumably IEEE 64-bit; byte order?)
    SAMPLE_alaw = 'alaw', //!< A-law logarithmic PCM
    SAMPLE_ulaw = 'ulaw', //!< mu-law logarithmic PCM
*/
} Mp4SampleEntry_e;

/* ************************************************************************** */
#endif // PARSER_MP4_STRUCT_H
