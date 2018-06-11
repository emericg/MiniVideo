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
 * \file      mp4_struct.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2011
 */

#ifndef PARSER_MP4_STRUCT_H
#define PARSER_MP4_STRUCT_H

// minivideo headers
#include "../../minivideo_typedef.h"
#include "../../minivideo_codecs.h"
#include "../../decoder/h264/h264_parameterset.h"

#include <cstdio>

/* ************************************************************************** */

//! MP4 Box header structure
typedef struct Mp4Box_t
{
    int64_t offset_start;   //!< Absolute position of the first byte of this box
    int64_t offset_end;     //!< Absolute position of the last byte of this box

    // Box parameters
    int64_t size;           //!< Box size in bytes, including all its fields and contained boxes
    uint32_t boxtype;       //!< A FourCC identifying the box type, see ISO/IEC 14496-12 Table 1
    uint8_t usertype[16];   //!< 16 bytes "extended type" (if applicable)

    // FullBox parameters (if applicable)
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
    char compressorname[32];

    uint32_t timescale;
    uint32_t mediatime;
    uint64_t duration;
    uint64_t creation_time;
    uint64_t modification_time;
    char language[3];

    unsigned int bitrate_max;
    unsigned int bitrate_avg;

        // Audio specific parameters
        unsigned int channel_count;
        unsigned int channel_mode;
        unsigned int sample_size_bits;
        unsigned int sample_rate_hz;
        unsigned int pcm_format;
        unsigned int pcm_bitdepth;
        unsigned int pcm_endianness;

        // Video specific parameters
        unsigned int width;
        unsigned int height;
        unsigned int color_depth;
        unsigned int color_space;
        unsigned int color_matrix;
        unsigned int color_range;
        unsigned int par_h;
        unsigned int par_v;
        unsigned int stereo;
        unsigned int projection;

        // AVC/HEVC specific parameters
        unsigned int codec_profile;
        unsigned int codec_level;
        unsigned int ref_frames;

        unsigned int sps_count;
        sps_t *sps_array[MAX_SPS];
        unsigned int *sps_sample_size;
        int64_t *sps_sample_offset;
        unsigned int pps_count;
        pps_t *pps_array[MAX_PPS];
        unsigned int *pps_sample_size;
        int64_t *pps_sample_offset;

        // TimeCode Record specific parameters
        uint8_t number_of_frames;

    // stss
    unsigned int stss_entry_count;                //!< IDR sample count
    unsigned int *stss_sample_number;

    // stts
    unsigned int stts_entry_count;
    unsigned int *stts_sample_count;
    unsigned int *stts_sample_delta;

    // ctts
    unsigned int ctts_entry_count;
    unsigned int *ctts_sample_count;
    int64_t *ctts_sample_offset;

    // stsc
    unsigned int stsc_entry_count;
    unsigned int *stsc_first_chunk;
    unsigned int *stsc_samples_per_chunk;         //!< An integer that gives the number of samples in each chunk
    unsigned int *stsc_sample_description_index;

    // stsz / stz2
    unsigned int stsz_sample_count;               //!< Sample count
    unsigned int stsz_sample_size;
    unsigned int *stsz_entry_size;

    // stco / co64
    unsigned int stco_entry_count;
    int64_t *stco_chunk_offset;

    // sdtp
    uint8_t *sdtp_is_leading;
    uint8_t *sdtp_sample_depends_on;
    uint8_t *sdtp_sample_is_depended_on;
    uint8_t *sdtp_sample_has_redundancy;

} Mp4Track_t;

typedef struct Mp4_t
{
    bool run;                   //!< A convenient way to stop the parser from any sublevel

    ContainerProfiles_e profile;//!< ISO BMF variant

    uint32_t timescale;
    uint64_t duration;
    uint64_t creation_time;
    uint64_t modification_time;

    uint32_t tracks_count;
    Mp4Track_t *tracks[16];

    FILE *xml;                  //!< Temporary file used by the xmlMapper

} Mp4_t;

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief Identifies ISO Base Media File Format variants.
 */
typedef enum IsoBmfVariant_e
{
    ISOBMF_UNKNOWN  = 0,

    ISOBMF_MOV      = 1,
    ISOBMF_MP4      = 2,
    ISOBMF_3GPP     = 3,
    ISOBMF_JPEG2k   = 4,
    ISOBMF_MPG21    = 5

} IsoBmfVariant_e;

/*!
 * \brief Identifies the box type.
 *
 * The field 'type' in a box header identifies the box type; standard boxes use
 * a compact type, which is normally four printable characters, to permit ease
 * of identification, and is shown so in the boxes below. User extensions use
 * an extended type; in this case, the type field is set to 'uuid'.
 *
 * The boxes marked with an (*) are mandatory.
 * The boxes marked with an (v) are vendor specific.
 */
typedef enum Mp4BoxType_e
{
    BOX_FTYP = 0x66747970,                      //!< (*) file type and compatibility
    BOX_STYP = 0x73747970,                      //!< (*) segment type
    BOX_PDIN = 0x7064696E,                      //!< progressive download information

    BOX_UDTA = 0x75647461,                      //!< user data box
        BOX_AMBA = 0x414D4241,                  //!< (v) Ambarella stuff
        BOX_FIRM = 0x4649524D,                  //!< (v) GoPro Firmware
        BOX_LENS = 0x4C454E53,                  //!< (v) GoPro Lens
        BOX_CAME = 0x43414D45,                  //!< (v) GoPro Camera
        BOX_SETT = 0x53455454,                  //!< (v) GoPro Settings
        BOX_MUID = 0x4D554944,                  //!< (v) GoPro Media Unique ID
        BOX_HMMT = 0x484D4D54,                  //!< (v) GoPro HiLight Tags
        BOX_BCID = 0x42434944,                  //!< (v) GoPro Broadcast ID
        BOX_GURI = 0x47555249,                  //!< (v) GoPro GURI
        BOX_GUSI = 0x47555349,                  //!< (v) GoPro GUSI
        BOX_GUMI = 0x47554D49,                  //!< (v) GoPro GUMI
        BOX_GPMF = 0x47504D46,                  //!< (v) GoPro Metadata Format

    BOX_MOOV = 0x6D6F6F76,                      //!< (*) container for all metadata
        BOX_MVHD = 0x6D766864,                  //!< (*) movie header, overall declarations
        BOX_IODS = 0x696F6473,                  //!< object descriptor box
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
                        BOX_ALIS = 0x616C6973,          //!< data entry file alias
                        BOX_URL = 0x75726C20,           //!< data entry url box
                        BOX_URN = 0x75726E20,           //!< data entry urn box
                    BOX_STBL = 0x7374626C,              //!< (*) sample table box, container for the time/space map
                        BOX_STSD = 0x73747364,          //!< (*) sample descriptions (codec types, initialization, etc)
                            BOX_AVCC = 0x61766343,      //!< (v) AVC configuration box
                            BOX_HVCC = 0x68766343,      //!< (v) HEVC configuration box
                            BOX_BTRT = 0x62747274,      //!< bitrate box
                            BOX_CLAP = 0x636C6170,      //!< clean aperture box
                            BOX_COLR = 0x636f6C72,      //!< color infos box
                            BOX_FIEL = 0x6669656C,      //!<
                            BOX_GAMA = 0x67616D61,      //!<
                            BOX_PASP = 0x70617370,      //!< pixel aspect ratio box
                            BOX_ESDS = 0x65736473,      //!< elementary stream descriptor box

                            BOX_WAVE = 0x77617665,              //!< QuickTime siDecompressionParam
                            BOX_FRMA = 0x66726D61,              //!< QuickTime Format atom
                            BOX_ENDA = 0x656E6461,              //!< QuickTime Endianness atom
                            BOX_CHAN = 0x6368616E,              //!< QuickTime Channel Layout atom

                            BOX_SA3D = 0x53413344,              //!< (v) Google Spatial Audio Box
                            BOX_SAND = 0x53414E44,              //!< (v) Non-Diegetic Audio Box
                            BOX_ST3D = 0x73743364,              //!< (v) Google Stereoscopic 3D Video Box
                            BOX_SV3D = 0x73763364,              //!< (v) Google Spherical Video Box
                                BOX_SVHD = 0x73766864,          //!< (v) Spherical Video Header
                                BOX_PROJ = 0x70726F6A,          //!< (v) Projection Box
                                    BOX_PRHD = 0x70726864,      //!< (v) Projection Header Box
                                    BOX_CBMP = 0x63626D70,      //!< (v) Cubemap Projection Box
                                    BOX_EQUI = 0x65717569,      //!< (v) Equirectangular Projection Box
                                    BOX_MSPH = 0x6D737068,      //!< (v) Mesh Projection Box
                                        BOX_MESH = 0x6D657368,  //!< (v) Mesh Box

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
        BOX_TRAF = 0x74726166,                  //!< track fragment
            BOX_TFHD = 0x74666866,              //!< (*) track fragment header
            BOX_TRUN = 0x7472756E,              //!< track fragment run
            BOX_TFDT = 0x74666474,              //!< track fragment decode time

    BOX_MFRA = 0x6D667261,                      //!< movie fragment random access
        BOX_TFRA = 0x74667261,                  //!< track fragment random access
        BOX_MFRO = 0x6d66726f,                  //!< (*) movie fragment random access offset

    BOX_MDAT = 0x6D646174,                      //!< media data container

    BOX_META = 0x6D657461,                      //!< metadata
        BOX_ILST = 0x696C7374,                  //!< (v) Apple item list box

    BOX_MECO = 0x6D65636F,                      //!< additional metadata container
        BOX_MERE = 0x6D657265,                  //!< metabox relation

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
 * There are four major handler type: audio, video, hint and meta.
 * For others, see: http://www.mp4ra.org/index.html
 */
typedef enum Mp4HandlerType_e
{
    MP4_HANDLER_AUDIO = 0x736F756E, //!< 'soun'
    MP4_HANDLER_VIDEO = 0x76696465, //!< 'vide'
    MP4_HANDLER_HINT  = 0x68696E74, //!< 'hint' // Hint track
    MP4_HANDLER_META  = 0x6D657461, //!< 'meta' // Metadata
    MP4_HANDLER_TMCD  = 0x746D6364, //!< 'tmcd' // Timecode track

    MP4_HANDLER_SUBT  = 0x73756274, //!< 'subt'
    MP4_HANDLER_SBTL  = 0x7362746c, //!< 'sbtl' // QuickTime Subtitle track
    MP4_HANDLER_TEXT  = 0x74657874  //!< 'text'

    // sdsm // SceneDescriptionStream
    // odsm // ObjectDescriptorStream

} Mp4HandlerType_e;

/* ************************************************************************** */
#endif // PARSER_MP4_STRUCT_H
