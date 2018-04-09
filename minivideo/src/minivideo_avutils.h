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
 * \file      minivideo_avutils.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef MINIVIDEO_AVUTILS_H
#define MINIVIDEO_AVUTILS_H

/* ************************************************************************** */

//! Stream type
typedef enum StreamType_e
{
    stream_UNKNOWN = 0,

    stream_AUDIO   = 1,
    stream_VIDEO   = 2,
    stream_TEXT    = 3,

    stream_MENU    = 4,
    stream_TMCD    = 5,
    stream_META    = 6,
    stream_HINT    = 7

} StreamType_e;

//! Sample type
typedef enum SampleType_e
{
    sample_UNKNOWN = 0,

    sample_RAW_DATA,

    sample_AUDIO,
    sample_AUDIO_TAG,

    sample_VIDEO,
    sample_VIDEO_SYNC,
    sample_VIDEO_PARAM,

    sample_TEXT,
    sample_TEXT_FILE,

    sample_TMCD,

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

//! Projection
typedef enum Projection_e
{
    PROJECTION_RECTANGULAR = 0,

    PROJECTION_EQUIRECTANGULAR  = 1,
    PROJECTION_EAC              = 2,
    PROJECTION_CUBEMAP_A        = 3,
    PROJECTION_MESH             = 4,

} Projection_e;

//! Stereo mode
typedef enum StereoMode_e
{
    MONOSCOPIC = 0,

    STEREO_ANAGLYPH_CR      = 1,        //!< (cyan/red)
    STEREO_ANAGLYPH_GM      = 2,        //!< (green/magenta)

    STEREO_SIDEBYSIDE_LEFT  = 3,        //!< (left eye is first)
    STEREO_SIDEBYSIDE_RIGHT = 4,        //!< (right eye is first)
    STEREO_TOPBOTTOM_LEFT   = 5,        //!< (left eye is first)
    STEREO_TOPBOTTOM_RIGHT  = 6,        //!< (right eye is first)
    STEREO_CHECKBOARD_LEFT  = 7,        //!< (left eye is first)
    STEREO_CHECKBOARD_RIGHT = 8,        //!< (right eye is first)
    STEREO_ROWINTERLEAVED_LEFT     =  9,//!< (left eye is first)
    STEREO_ROWINTERLEAVED_RIGHT    = 10,//!< (right eye is first)
    STEREO_COLUMNINTERLEAVED_LEFT  = 11,//!< (left eye is first)
    STEREO_COLUMNINTERLEAVED_RIGHT = 12,//!< (right eye is first)
    // both eyes laced in one Block     //!< (left eye is first)
    // both eyes laced in one Block     //!< (right eye is first)

} StereoMode_e;

/* ************************************************************************** */

/*!
 * \brief Audio speakers from WAVEFORMATEXTENSIBLE 'dwChannelMask' field.
 *
 * With the advent of Windows 2000 Microsoft introduced the multichannel extension
 * to its RIFF/WAVE file format, called Wave Format Extensible. The main purpose
 * of this file format was to support multichannel audio in PC gaming applications.
 */
typedef enum AudioSpeakers_e
{
    SPEAKER_FRONT_LEFT              = 0x00000001,
    SPEAKER_FRONT_RIGHT             = 0x00000002,
    SPEAKER_FRONT_CENTER            = 0x00000004,
    SPEAKER_LOW_FREQUENCY           = 0x00000008,
    SPEAKER_BACK_LEFT               = 0x00000010,
    SPEAKER_BACK_RIGHT              = 0x00000020,
    SPEAKER_FRONT_LEFT_OF_CENTER    = 0x00000040,
    SPEAKER_FRONT_RIGHT_OF_CENTER   = 0x00000080,
    SPEAKER_BACK_CENTER             = 0x00000100,
    SPEAKER_SIDE_LEFT               = 0x00000200,
    SPEAKER_SIDE_RIGHT              = 0x00000400,
    SPEAKER_TOP_CENTER              = 0x00000800,
    SPEAKER_TOP_FRONT_LEFT          = 0x00001000,
    SPEAKER_TOP_FRONT_CENTER        = 0x00002000,
    SPEAKER_TOP_FRONT_RIGHT         = 0x00004000,
    SPEAKER_TOP_BACK_LEFT           = 0x00008000,
    SPEAKER_TOP_BACK_CENTER         = 0x00010000,
    SPEAKER_TOP_BACK_RIGHT          = 0x00020000,

    // (See AudioSpeakersExtended_e)

    SPEAKER_ALL                     = 0x80000000

} AudioSpeakers_e;

/*!
 * \brief Additional WAVEFORMATEXTENSIBLE fields from RF64.
 */
typedef enum AudioSpeakersExtended_e
{
    SPEAKER_BITSTREAM_1_LEFT        = 0x00800000,
    SPEAKER_BITSTREAM_1_RIGHT       = 0x01000000,
    SPEAKER_BITSTREAM_2_LEFT        = 0x02000000,
    SPEAKER_BITSTREAM_2_RIGHT       = 0x04000000,
    SPEAKER_CONTROLSAMPLE_1         = 0x08000000,
    SPEAKER_CONTROLSAMPLE_2         = 0x10000000,
    SPEAKER_STEREO_LEFT             = 0x20000000,
    SPEAKER_STEREO_RIGHT            = 0x40000000

} AudioSpeakersExtended_e;

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
    CHANS_SURROUND_222  = 33,

    CHANS_AMBISONIC_FOA = 64,
    CHANS_AMBISONIC_SOA = 65,
    CHANS_AMBISONIC_TOA = 66

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
#endif // MINIVIDEO_AVUTILS_H
