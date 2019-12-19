/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
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
 * \file      videobackends_vda.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 *
 * This file is heavily based on 'VDADecoderChecker.cpp' from Andy Breuhan.
 * 'VDADecoderChecker.cpp' is using the same GPL v3+ license as mini_analyser.
 *
 * https://developer.apple.com/library/content/technotes/tn2267/_index.html#//apple_ref/doc/uid/DTS40009798-CH1-TNTAG1
 * https://www.objc.io/issues/23-video/videotoolbox/
 */

#include "videobackends_vda.h"
#include "videobackends_h264.h"
#include <minivideo_codecs.h>

#include <stdlib.h>
#include <string.h>

#include <QDebug>

#ifdef VIDEOBACKEND_VDA

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#include <VideoDecodeAcceleration/VDADecoder.h>

/* ************************************************************************** */

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

Desc decoder_profiles_vda[] =
{
    { CODEC_H264,   PROF_H264_CBP,  1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_XP,   1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_BP,   1280,  720, 8, avcC_BP_720 },
    { CODEC_H264,   PROF_H264_MP,   1280,  720, 8, nullptr },

    { CODEC_H264,   PROF_H264_HiP,  1280,  720, 8, avcC_HiP_720 },
    { CODEC_H264,   PROF_H264_HiP,  1920, 1080, 8, avcC_HiP_1080 },
    { CODEC_H264,   PROF_H264_HiP,  3840, 2160, 8, avcC_HiP_2160 },
    { CODEC_H264,   PROF_H264_HiP,  4096, 2048, 8, avcC_HiP_2048 },
    { CODEC_H264,   PROF_H264_HiP,  4096, 2160, 8, avcC_HiP_2048 },
    { CODEC_H264,   PROF_H264_HiP,  7680, 3840, 8, avcC_HiP_3840 },
    { CODEC_H264,   PROF_H264_HiP,  8192, 4096, 8, nullptr },

    { CODEC_H264,   PROF_H264_PHiP, 1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_CHiP, 1280,  720, 8, nullptr },
    { CODEC_H264,   PROF_H264_Hi444PP, 1280, 720, 8, nullptr },
};

const size_t decoder_profile_count = sizeof(decoder_profiles_vda) / sizeof(Desc);

/* ************************************************************************** */

// Needed for a valid configuration
void myDecoderOutputCallback();
void myDecoderOutputCallback() {}

typedef struct OpaqueVDADecoder *VDADecoder;

// tracks a frame in and output queue in display order
typedef struct myDisplayFrame {
    int64_t frameDisplayTime;
    CVPixelBufferRef frame;
    struct myDisplayFrame *nextFrame;
} myDisplayFrame, *myDisplayFramePtr;

// some user data
typedef struct MyUserData
{
    myDisplayFramePtr displayQueue; // display-order queue - next display frame is always at the queue head
    int32_t queueDepth; // we will try to keep the queue depth around 10 frames
    pthread_mutex_t queueMutex; // mutex protecting queue manipulation

} MyUserData, *MyUserDataPtr;

OSStatus CreateDecoderVDA(Desc &prof)
{
    OSStatus status;

    SInt32 inWidth = (SInt32)prof.width;
    SInt32 inHeight = (SInt32)prof.height;
    OSType inSourceFormat = 'avc1'; // VDA will only decode H.264
    OSType cvPixelFormatType = kCVPixelFormatType_422YpCbCr8;
    const UInt8 *privateData = prof.privateDatas;

    MyUserData myUserData;
    CFMutableDictionaryRef decoderConfiguration = nullptr;
    CFMutableDictionaryRef destinationImageBufferAttributes = nullptr;
    CFDictionaryRef emptyDictionary;

    CFNumberRef width = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inWidth);
    CFNumberRef height = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inHeight);
    CFNumberRef sourceFormat = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inSourceFormat);
    CFNumberRef pixelFormat = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &cvPixelFormatType);

    if (privateData == nullptr)
    {
        qDebug() << "avc1 decoder configuration data cannot be NULL!";
        return paramErr;
    }

    CFDataRef inAVCCData = CFDataCreate(kCFAllocatorDefault, avcC_BP_720, sizeof(avcC_BP_720));

    // the avcC data chunk from the bitstream must be present
    if (inAVCCData == nullptr)
    {
        qDebug() << "avc1 decoder configuration data cannot be NULL!";
        return paramErr;
    }

    // create a CFDictionary describing the source material for decoder configuration
    decoderConfiguration = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                     4,
                                                     &kCFTypeDictionaryKeyCallBacks,
                                                     &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_Height, height);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_Width, width);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_SourceFormat, sourceFormat);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_avcCData, inAVCCData);

    // create a CFDictionary describing the wanted destination image buffer
    destinationImageBufferAttributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                 2,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);

    emptyDictionary = CFDictionaryCreate(kCFAllocatorDefault, // our empty IOSurface properties dictionary
                                         nullptr, nullptr, 0,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(destinationImageBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormat);
    CFDictionarySetValue(destinationImageBufferAttributes,
                         kCVPixelBufferIOSurfacePropertiesKey,
                         emptyDictionary);

    // create the hardware decoder object
    VDADecoder decoderOut = nullptr;
    status = VDADecoderCreate(decoderConfiguration,
                              destinationImageBufferAttributes,
                              (VDADecoderOutputCallback*)myDecoderOutputCallback,
                              (void *)&myUserData,
                              &decoderOut);

    if (destinationImageBufferAttributes) CFRelease(destinationImageBufferAttributes);
    if (emptyDictionary) CFRelease(emptyDictionary);
    if (decoderOut) VDADecoderDestroy(decoderOut);
    if (decoderConfiguration) CFRelease(decoderConfiguration);
    if (inAVCCData) CFRelease(inAVCCData);

    return status;
}

#endif // VIDEOBACKEND_VDA

/* ************************************************************************** */

VideoBackendsVDA::VideoBackendsVDA()
{
    //
}

VideoBackendsVDA::~VideoBackendsVDA()
{
    //
}

bool VideoBackendsVDA::load(VideoBackendInfos &infos)
{
    bool status = true;
    infos.api_name = "VDA (Video Decode Acceleration)";

#ifdef VIDEOBACKEND_VDA
    for (size_t x = 0; x < decoder_profile_count; x++)
    {
        OSStatus vda_status = CreateDecoderVDA(decoder_profiles_vda[x]);

        switch (vda_status)
        {
            case kVDADecoderNoErr:
            {
                qDebug() << "CreateDecoder(VDA) SUCCESS";

                CodecSupport c;
                c.codec = decoder_profiles_vda[x].codec;
                c.profile = decoder_profiles_vda[x].profile;
                c.max_width = decoder_profiles_vda[x].width;
                c.max_height = decoder_profiles_vda[x].height;
                c.max_bitdepth = decoder_profiles_vda[x].bitdepth;

                //if (x > 0 && decoder_profiles_vda[x-1].profile == decoder_profiles_vda[x].profile)
                //{
                //    infos.decodingSupport.pop_back();
                //    infos.decodingSupport.push_back(c);
                //}
                //else
                    infos.decodingSupport.push_back(c);
            } break;

            case kVDADecoderHardwareNotSupportedErr:
                qDebug() << "The hardware does not support accelerated video services required for hardware decode.";
                break;
            case kVDADecoderFormatNotSupportedErr:
                qDebug() << "The hardware may support accelerated decode, but does not support the requested output format.";
                break;
            case kVDADecoderConfigurationError:
                qDebug() << "Invalid or unsupported configuration parameters were specified in VDADecoderCreate.";
                break;
            case kVDADecoderDecoderFailedErr:
                qDebug() << "An error was returned by the decoder layer.\n" \
                         << "This may happen for example because of bitstream/data errors during a decode operation.\n" \
                         << "This error may also be returned from VDADecoderCreate when hardware decoder resources are available on the system but currently in use by another process.";
                break;

            case paramErr:
                qDebug() << "Parameter Error.";
                break;
            default:
                qDebug() << "Unknown Status: " << vda_status;
                break;
        }
    }
#endif // VIDEOBACKEND_VDA

    return status;
}
