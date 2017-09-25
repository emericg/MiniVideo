/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      videobackends_vtb.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 *
 * https://developer.apple.com/library/content/technotes/tn2267/_index.html#//apple_ref/doc/uid/DTS40009798-CH1-TNTAG1
 * https://www.objc.io/issues/23-video/videotoolbox/
 */

#include "videobackends_vtb.h"
#include "videobackends_h264.h"
#include <minivideo_codecs.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <QDebug>

#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VideoToolbox.h>

// Generic description structure
struct Desc
{
    uint32_t codec;
    uint32_t profile;

    uint32_t width;
    uint32_t height;
    uint32_t bitdepth;

    const uint8_t *privateDatas;
};

Desc decoder_profiles_vtb[] =
{
    { CODEC_MPEG1,  0,              1280,  720, 8, nullptr },
    { CODEC_MPEG2,  PROF_H262_SP,   1280,  720, 8, nullptr },
    { CODEC_MPEG2,  PROF_H262_MP,   1280,  720, 8, nullptr },

    { CODEC_H264,   PROF_H264_CBP,  1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_XP,   1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_BP,   1280,  720, 8, avcC_BP_720 },
    { CODEC_H264,   PROF_H264_MP,   1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_HiP,  1280,  720, 8, avcC_HiP_720 },
    { CODEC_H264,   PROF_H264_HiP,  1920, 1080, 8, avcC_HiP_1080 },
    { CODEC_H264,   PROF_H264_HiP,  3840, 2160, 8, avcC_HiP_2160 },
    { CODEC_H264,   PROF_H264_HiP,  4096, 2048, 8, avcC_HiP_2048 },
    { CODEC_H264,   PROF_H264_HiP,  7680, 3840, 8, avcC_HiP_3840 },
    { CODEC_H264,   PROF_H264_PHiP, 1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_CHiP, 1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_Hi444PP, 1280, 720, 8, nullptr },

    { CODEC_H265,   PROF_H265_Main,     1280, 720, 8, nullptr },
    { CODEC_H265,   PROF_H265_Main10,   1280, 720, 8, nullptr },
    { CODEC_H265,   PROF_H265_MainStill,1280, 720, 8, nullptr },
    { CODEC_H265,   PROF_H265_Main12,   1280, 720, 8, nullptr },
    { CODEC_H265,   PROF_H265_Main444,  1280, 720, 8, nullptr },
};

const size_t decoder_profile_count = sizeof(decoder_profiles_vtb) / sizeof(Desc);

/* ************************************************************************** */

const CFStringRef kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder = CFSTR ("EnableHardwareAcceleratedVideoDecoder");
const CFStringRef kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder = CFSTR ("RequireHardwareAcceleratedVideoDecoder");

// Needed for a valid configuration
static void session_output_callback (void *decompression_output_ref_con,
                                     void *source_frame_ref_con,
                                     OSStatus status,
                                     VTDecodeInfoFlags info_flags,
                                     CVImageBufferRef image_buffer,
                                     CMTime pts, CMTime duration)
{
    //
}

CMFormatDescriptionRef format_description = NULL;

void dict_set_boolean (CFMutableDictionaryRef dict, CFStringRef key, bool value)
{
    CFDictionarySetValue (dict, key, value ? kCFBooleanTrue : kCFBooleanFalse);
}

void dict_set_i32 (CFMutableDictionaryRef dict, CFStringRef key, int32_t value)
{
    CFNumberRef number = CFNumberCreate (NULL, kCFNumberSInt32Type, &value);
    CFDictionarySetValue (dict, key, number);
    CFRelease (number);
}

void dict_set_string (CFMutableDictionaryRef dict, CFStringRef key, const char *value)
{
    CFStringRef string = CFStringCreateWithCString (NULL, value, kCFStringEncodingASCII);
    CFDictionarySetValue (dict, key, string);
    CFRelease (string);
}

void dict_set_data (CFMutableDictionaryRef dict, CFStringRef key, uint8_t *value, uint64_t length)
{
    CFDataRef data = CFDataCreate (NULL, value, length);
    CFDictionarySetValue (dict, key, data);
    CFRelease (data);
}

void dict_set_object (CFMutableDictionaryRef dict, CFStringRef key, CFTypeRef *value)
{
    CFDictionarySetValue (dict, key, value);
    CFRelease (value);
}


static CMFormatDescriptionRef
create_format_description (int width, int height, CMVideoCodecType cm_format)
{
    CMFormatDescriptionRef fmt_desc;
    OSStatus status = CMVideoFormatDescriptionCreate (kCFAllocatorDefault, cm_format, width, height, NULL, &fmt_desc);

    if (status != noErr)
    {
        qDebug() << "error creating format :(";
        return NULL;
    }

    return fmt_desc;
}

static CMFormatDescriptionRef
create_format_description_from_codec_data (int width, int height, int bitdepth, CMVideoCodecType cm_format)
{
    CMFormatDescriptionRef fmt_desc;
    CFMutableDictionaryRef extensions, par, atoms;
    OSStatus status;

    // Extensions dict
    extensions = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    dict_set_string (extensions, CFSTR ("CVImageBufferChromaLocationBottomField"), "left");
    dict_set_string (extensions, CFSTR ("CVImageBufferChromaLocationTopField"), "left");
    dict_set_boolean (extensions, CFSTR ("FullRangeVideo"), FALSE);
    // CVPixelAspectRatio dict
    par = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    dict_set_i32 (par, CFSTR ("HorizontalSpacing"), 1);
    dict_set_i32 (par, CFSTR ("VerticalSpacing"), 1);
    dict_set_object (extensions, CFSTR ("CVPixelAspectRatio"), (CFTypeRef *) par);
    // SampleDescriptionExtensionAtoms dict
    atoms = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    dict_set_data (atoms, CFSTR ("avcC"), (uint8_t*)avcC_HiP_2048, sizeof(avcC_HiP_2048));
    dict_set_object (extensions, CFSTR ("SampleDescriptionExtensionAtoms"), (CFTypeRef *) atoms);

    status = CMVideoFormatDescriptionCreate (kCFAllocatorDefault, cm_format,
                                             width, height, extensions, &fmt_desc);

    if (status != noErr)
    {
        qDebug() << "error creating format :(";
        return NULL;
    }

    return fmt_desc;
}


static OSStatus
create_session (void *vtdec, int32_t width, int32_t height, CMVideoCodecType cm_format, VideoBackendInfos &infos)
{
    bool enable_hardware = true;
    bool require_hardware = true;

    CFMutableDictionaryRef output_image_buffer_attrs;
    VTDecompressionOutputCallbackRecord callback;
    CFMutableDictionaryRef videoDecoderSpecification;
    VTDecompressionSessionRef session;

    //CMFormatDescriptionRef format_description = create_format_description(width, height, cm_format);
    CMFormatDescriptionRef format_description = create_format_description_from_codec_data(width, height, 8, cm_format);

    OSStatus vtb_status;
    uint32_t cv_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange; // nv12
    //cv_format = kCVPixelFormatType_422YpCbCr8; // UYVY

    videoDecoderSpecification = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    dict_set_boolean (videoDecoderSpecification, kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder, enable_hardware);
    if (enable_hardware && require_hardware)
        dict_set_boolean (videoDecoderSpecification, kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder, TRUE);

    output_image_buffer_attrs = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    dict_set_i32 (output_image_buffer_attrs, kCVPixelBufferPixelFormatTypeKey, cv_format);
    dict_set_i32 (output_image_buffer_attrs, kCVPixelBufferWidthKey, width);
    dict_set_i32 (output_image_buffer_attrs, kCVPixelBufferHeightKey, height);

    callback.decompressionOutputCallback = session_output_callback;
    callback.decompressionOutputRefCon = vtdec;

    vtb_status = VTDecompressionSessionCreate (NULL, format_description, videoDecoderSpecification, output_image_buffer_attrs, &callback, &session);

    switch (vtb_status)
    {
        case kVTCouldNotFindVideoDecoderErr:
            qDebug() << "Could not find video decoder";
            break;
        case kVTVideoDecoderNotAvailableNowErr:
            qDebug() << "Hardware acceleration is not available now";
            break;

        case noErr:
    {
            qDebug() << "SUCCESS";

            CodecSupport c;
            c.codec = CODEC_H264;
            c.profile = PROF_H264_HiP;
            c.max_width = width;
            c.max_height = height;
            c.max_bitdepth = 8;

            infos.decodingSupport.push_back(c);

     }       break;

        case paramErr:
            qDebug() << "Parameter Error.";
            break;
        default:
            qDebug() << "Unknown Status: " << vtb_status;
            break;
    }

    VTDecompressionSessionInvalidate(session);
    CFRelease (output_image_buffer_attrs);

    return vtb_status;
}



/* ************************************************************************** */

VideoBackendsVideoToolBox::VideoBackendsVideoToolBox()
{
    //
}

VideoBackendsVideoToolBox::~VideoBackendsVideoToolBox()
{
    //
}

bool VideoBackendsVideoToolBox::load(VideoBackendInfos &infos)
{
    bool status = true;

    infos.api_name = "VideoToolBox";

    //cm_format = kCMVideoCodecType_H264;
    //cm_format = kCMVideoCodecType_HEVC;
    //cm_format = kCMVideoCodecType_MPEG4Video;
    //cm_format = kCMVideoCodecType_MPEG2Video;
    //cm_format = kCMVideoCodecType_MPEG1Video;

    OSStatus s = create_session(NULL, 1280, 720, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 1920, 800, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 1920, 1000, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 1920, 1080, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 3840, 2160, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 4096, 2048, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 7680, 3840, kCMVideoCodecType_H264, infos);
    s = create_session(NULL, 10000, 10000, kCMVideoCodecType_H264, infos);

    return status;
}
