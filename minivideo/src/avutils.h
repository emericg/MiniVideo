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
    stream_TEXT    = 3,

    stream_MENU    = 4,
    stream_TMCD    = 5

} StreamType_e;

//! Sample type
typedef enum SampleType_e
{
    sample_UNKNOWN = 0,

    sample_AUDIO,

    sample_VIDEO,
    sample_VIDEO_IDR,
    sample_VIDEO_PARAM,

    sample_TEXT_FILE,

    sample_OTHER

} SampleType_e;

//! Bitrate mode
typedef enum BitrateMode_e
{
    BITRATE_UNKNOWN = 0,

    BITRATE_CBR     = 1, //!< Constant BitRate
    BITRATE_VBR     = 2, //!< Variable BitRate
    BITRATE_ABR     = 3, //!< Average BitRate
    BITRATE_CVBR    = 4  //!< Constrained Variable BitRate

} BitrateMode_e;

//! Framerate mode
typedef enum FramerateMode_e
{
    FRAMERATE_UNKNOWN = 0,

    FRAMERATE_CFR     = 1, //!< Constant FrameRate
    FRAMERATE_VFR     = 2, //!< Variable FrameRate

} FramerateMode_e;

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

//! The color model used by the video
typedef enum ColorModel_e
{
    CLR_UNKNOWN = 0,     //!< Unknown color model

    CLR_RGB     = 1,
    CLR_YCbCr   = 2,
    CLR_YCgCo   = 3,

} ColorModel_e;

//! The color matrix used by the video
typedef enum ColorMatrix_e
{
    CM_UNKNOWN = 0,

    CM_sRGB,
    CM_sYCC,
    CM_xvYCC,
    CM_XYZ,
    CM_PAL,
    CM_NTSC,
    CM_SMPTE170M,
    CM_SMPTE240M,
    CM_bt470,
    CM_bt601,
    CM_bt709,
    CM_bt2020,

} ColorMatrix_e;

//! The subsampling format used by the video
typedef enum SubSampling_e
{
    SS_UNKNOWN = 0,     //!< Unknown subsampling

    SS_400     = 1,     //!< 4:0:0 greyscale subsampling
    SS_411     = 2,     //!< 4:1:1 subsampling
    SS_420     = 3,     //!< 4:2:0 subsampling
    SS_422     = 4,     //!< 4:2:2 subsampling
    SS_444     = 5,     //!< 4:4:4 subsampling
    SS_4444    = 6      //!< 4:4:4:4 subsampling

} SubSampling_e;

/* ************************************************************************** */
#endif // AV_UTILS_H
