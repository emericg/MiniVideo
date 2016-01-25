/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      avutils.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef AV_UTILS_H
#define AV_UTILS_H

#include "typedef.h"

/* ************************************************************************** */

//! Stream type
typedef enum StreamType_e
{
    stream_UNKNOWN = 0,

    stream_AUDIO   = 1,
    stream_VIDEO   = 2,
    stream_TEXT    = 3

} StreamType_e;

//! Stream level
typedef enum StreamLevel_e
{
    stream_level_UNKNOWN = 0,

    stream_level_PES     = 1,
    stream_level_ES      = 2

} StreamLevel_e;

//! Sample type
typedef enum SampleType_e
{
    sample_AUDIO = 0,

    sample_VIDEO,
    sample_VIDEO_IDR,
    sample_VIDEO_PARAM,

    sample_TEXT_FILE

} SampleType_e;

//! Bitrate mode
typedef enum BitrateMode_e
{
    BITRATE_UNKNOWN = 0,

    BITRATE_CBR     = 1, //!< Constant Bitrate
    BITRATE_VBR     = 2, //!< Variable Bitrate
    BITRATE_ABR     = 3, //!< Average Bitrate
    BITRATE_CVBR    = 4  //!< Constrained Variable Bitrate

} BitrateMode_e;

/* ************************************************************************** */

//! Audio speakers from WAVEFORMATEXTENSIBLE 'dwChannelMask' field
typedef enum AudioSpeakers_e
{
    SPEAKER_FRONT_LEFT              = 0x1,
    SPEAKER_FRONT_RIGHT             = 0x2,
    SPEAKER_FRONT_CENTER            = 0x4,
    SPEAKER_LOW_FREQUENCY           = 0x8,
    SPEAKER_BACK_LEFT               = 0x10,
    SPEAKER_BACK_RIGHT              = 0x20,
    SPEAKER_FRONT_LEFT_OF_CENTER    = 0x40,
    SPEAKER_FRONT_RIGHT_OF_CENTER   = 0x80,
    SPEAKER_BACK_CENTER             = 0x100,
    SPEAKER_SIDE_LEFT               = 0x200,
    SPEAKER_SIDE_RIGHT              = 0x400,
    SPEAKER_TOP_CENTER              = 0x800,
    SPEAKER_TOP_FRONT_LEFT          = 0x1000,
    SPEAKER_TOP_FRONT_CENTER        = 0x2000,
    SPEAKER_TOP_FRONT_RIGHT         = 0x4000,
    SPEAKER_TOP_BACK_LEFT           = 0x8000,
    SPEAKER_TOP_BACK_CENTER         = 0x10000,
    SPEAKER_TOP_BACK_RIGHT          = 0x20000

} AudioSpeakers_e;

//! MPEG audio channel modes
typedef enum MPEGChannelMode_e
{
    CHAN_MONO           = 0,
    CHAN_STEREO         = 1,
    CHAN_STEREO_JOINT   = 2,
    CHAN_DUAL           = 3

} MPEGChannelMode_e;

//! Audio channel modes
typedef enum ChannelMode_e
{
    CHANS_UNKNOWN       = 0,

    CHANS_MONO          = 1,
    CHANS_STEREO        = 2,
    CHANS_30            = 3,
    CHANS_QUAD          = 4,
    CHANS_50            = 5,
    CHANS_60            = 6,
    CHANS_70            = 7,

    CHANS_SURROUND_21   = 16,
    CHANS_SURROUND_31   = 17,
    CHANS_SURROUND_41   = 18,
    CHANS_SURROUND_51   = 19,
    CHANS_SURROUND_71   = 20,
    CHANS_SURROUND_91   = 21,
    CHANS_SURROUND_111  = 22,

    CHANS_SURROUND_102  = 32,
    CHANS_SURROUND_222  = 33

} ChannelMode_e;

/* ************************************************************************** */

//! Picture extraction repartition mode
typedef enum PictureRepartition_e
{
    PICTURE_UNFILTERED  = 0,
    PICTURE_ORDERED     = 1,
    PICTURE_DISTRIBUTED = 2

} PictureRepartition_e;

//! The color space used by the video
typedef enum ColorSpace_e
{
    CS_UNKNOWN = 0,     //!< Unknown color space

    CS_YUV     = 1,     //!< YUV (YCbCr) color space
    CS_RGB     = 2      //!< RGB color space

} ColorSpace_e;

//! The subsampling format used by the video
typedef enum SubSampling_e
{
    SS_UNKNOWN = 0,     //!< Unknown subsampling

    SS_400     = 1,     //!< Greyscale
    SS_420     = 2,     //!< 4:2:0 subsampling
    SS_422     = 3,     //!< 4:2:2 subsampling
    SS_444     = 4      //!< 4:4:4 subsampling

} SubSampling_e;

//! Frame rate (picture per second)
typedef enum FrameRate_e
{
    FR_UNKNOWN = 0,   //!< Unknown frame rate

    FR_24p,           //!< 24 frames
    FR_24p_NTSC,      //!< (24 frames * 1000) / 1001 = 23.976 frames/s
    FR_25p,
    FR_30p,
    FR_30p_NTSC,      //!< (30 frames * 1000) / 1001 = 29.970 frames/s
    FR_48p,
    FR_48p_NTSC,      //!< (48 frames * 1000) / 1001 = 47.952 frames/s
    FR_50p,
    FR_50i,           //!< 50 interlaced fields (25 frames)
    FR_60p,
    FR_60p_NTSC,      //!< (60 frames * 1000) / 1001 = 59.970 frames/s
    FR_60i,           //!< 60 interlaced fields (30 frames)
    FR_60i_NTSC,      //!< 60 interlaced fields * 1000/1001 = 59.940 interlaced fields (29.970 frames)
    FR_72p,
    FR_120p,
    FR_240p

} FrameRate_e;

/* ************************************************************************** */
#endif // AV_UTILS_H
