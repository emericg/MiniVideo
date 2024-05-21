/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of mini_analyser.
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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 *
 * This file is heavily based on 'vadumpcaps.c' from Mark Thompson <sw@jkqxz.net>
 * 'vadumpcaps.c' is using the same GPL v3+ license as mini_analyser.
 */

#include "videobackends_vaapi.h"
#include <minivideo_codecs.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <QDebug>

#include <X11/Xlib.h>
#include <va/va.h>
#include <va/va_drm.h>
#include <va/va_drmcommon.h>
#include <va/va_x11.h>

/* ************************************************************************** */

static VADisplay open_device_drm(const char *drm_device)
{
    int drm_fd = open(drm_device, O_RDWR);
    if (drm_fd < 0)
        qDebug() << "Failed to open '" << drm_device << "'";

    VADisplay display = vaGetDisplayDRM(drm_fd);
    if (!display)
        qDebug() << "Failed to open VA display from DRM device.";

    return display;
}

static VADisplay open_device_x11(const char *x11_display_name)
{
    Display *x11_display = XOpenDisplay(x11_display_name);
    if (!x11_display)
        qDebug() << "Failed to open '" << x11_display_name << "'";

    VADisplay display = vaGetDisplay(x11_display);
    if (!display)
        qDebug() << "Failed to open VA display from X11 device.";

    return display;
}

// Generic description structure
struct Desc
{
    int id;

    uint32_t codec;
    uint32_t profile;
    uint32_t bitdepth;
};

static Desc decoder_profiles_vaapi[] =
{
    { VAProfileNone,                        CODEC_UNKNOWN,  CODEC_PROF_UNKNOWN, 0 },

    { VAProfileMPEG2Simple,                 CODEC_H262,     PROF_H262_SP, 8 },
    { VAProfileMPEG2Main,                   CODEC_H262,     PROF_H262_MP, 8 },
    { VAProfileMPEG4Simple,                 CODEC_MPEG4_ASP,PROF_MPEG4_SP, 8 },
    { VAProfileMPEG4AdvancedSimple,         CODEC_MPEG4_ASP,PROF_MPEG4_ASP, 8 },
    { VAProfileMPEG4Main,                   CODEC_MPEG4_ASP,CODEC_PROF_UNKNOWN, 8 },
    { VAProfileH264Baseline,                CODEC_H264,     PROF_H264_BP, 8 }, // DEPRECATED starting libva 2.0.0
    { VAProfileH264Main,                    CODEC_H264,     PROF_H264_MP, 8 },
    { VAProfileH264High,                    CODEC_H264,     PROF_H264_HiP, 8 },
    { VAProfileVC1Simple,                   CODEC_VC1,      PROF_VC1_SIMPLE, 8 },
    { VAProfileVC1Main,                     CODEC_VC1,      PROF_VC1_MAIN, 8 },
    { VAProfileVC1Advanced,                 CODEC_VC1,      PROF_VC1_ADVANCED, 8 },
    { VAProfileH263Baseline,                CODEC_H263,     CODEC_PROF_UNKNOWN, 8 },
    { VAProfileJPEGBaseline,                CODEC_JPEG,     CODEC_PROF_UNKNOWN, 8 },
    { VAProfileH264ConstrainedBaseline,     CODEC_H264,     PROF_H264_CBP, 8 },
#if VA_CHECK_VERSION(0, 35, 0) // libva 1.3
    { VAProfileVP8Version0_3,               CODEC_VP8,      PROF_VP8_0, 8 },
#endif
#if VA_CHECK_VERSION(0, 36, 0) // libva 1.4
    { VAProfileH264MultiviewHigh,           CODEC_H264,     PROF_H264_MvHiP, 8 },
    { VAProfileH264StereoHigh,              CODEC_H264,     PROF_H264_StHiP, 8 },
#endif
#if VA_CHECK_VERSION(0, 37, 0) // libva 1.5
    { VAProfileHEVCMain,                    CODEC_H265,     PROF_H265_Main, 8 },
    { VAProfileHEVCMain10,                  CODEC_H265,     PROF_H265_Main10, 10 },
#endif
#if VA_CHECK_VERSION(0, 38, 0) // libva 1.6
    { VAProfileVP9Profile0,                 CODEC_VP9,      PROF_VP9_0, 8 },
#endif
#if VA_CHECK_VERSION(0, 39, 0) // libva 1.7
    { VAProfileVP9Profile1,                 CODEC_VP9,      PROF_VP9_1, 8 },
    { VAProfileVP9Profile2,                 CODEC_VP9,      PROF_VP9_2, 12 },
    { VAProfileVP9Profile3,                 CODEC_VP9,      PROF_VP9_3, 12 },
#endif
#if VA_CHECK_VERSION(1, 2, 0) // libva 2.2
    { VAProfileHEVCMain12,                  CODEC_H265,     PROF_H265_Main12, 12 },
    { VAProfileHEVCMain422_10,              CODEC_H265,     PROF_H265_Main422_10, 10 },
    { VAProfileHEVCMain422_12,              CODEC_H265,     PROF_H265_Main422_12, 12 },
    { VAProfileHEVCMain444,                 CODEC_H265,     PROF_H265_Main444, 8 },
    { VAProfileHEVCMain444_10,              CODEC_H265,     PROF_H265_Main444_10, 10 },
    { VAProfileHEVCMain444_12,              CODEC_H265,     PROF_H265_Main444_12, 12 },
    { VAProfileHEVCSccMain,                 CODEC_H265,     PROF_H265_ScreenExtended_Main, 8 },
    { VAProfileHEVCSccMain10,               CODEC_H265,     PROF_H265_ScreenExtended_Main10, 10 },
    { VAProfileHEVCSccMain444,              CODEC_H265,     PROF_H265_ScreenExtended_Main_444, 8 },
#endif
#if VA_CHECK_VERSION(1, 7, 0) // libva 2.7
    { VAProfileAV1Profile0,                 CODEC_AV1,      PROF_AV1_Main, 8 },
    { VAProfileAV1Profile1,                 CODEC_AV1,      PROF_AV1_High, 8 },
#endif
#if VA_CHECK_VERSION(1, 8, 0) // libva 2.8
    { VAProfileHEVCSccMain444_10,           CODEC_H265,     PROF_H265_ScreenExtended_Main_444_10, 10 },
#endif
};

const size_t decoder_profile_count = sizeof(decoder_profiles_vaapi) / sizeof(Desc);

bool dump_profiles(VADisplay display, VideoBackendInfos &infos)
{
    VAStatus vas;

    int max_profile_count = vaMaxNumProfiles(display);
    VAProfile *profile_list = (VAProfile *)calloc(max_profile_count, sizeof(*profile_list));

    vas = vaQueryConfigProfiles(display, profile_list, &max_profile_count);
    if (vas != VA_STATUS_SUCCESS)
    {
        qDebug() << "Failed query VA-API profiles:" << vas << "(" << vaErrorStr(vas) << ")";
        return false;
    }

    unsigned j = 0;
    for (int i = 0; i < max_profile_count; i++)
    {
        for (j = 0; j < decoder_profile_count; j++)
        {
            if (decoder_profiles_vaapi[j].id == profile_list[i])
                break;
        }
        if (j >= decoder_profile_count)
            j = 0;

        CodecSupport c;
        c.codec = decoder_profiles_vaapi[j].codec;
        c.profile = decoder_profiles_vaapi[j].profile;
        c.max_width = -1;
        c.max_height = -1;
        c.max_bitdepth = static_cast<int>(decoder_profiles_vaapi[j].bitdepth);

        infos.decodingSupport.push_back(c);

        //dump_entrypoints(display, profile_list[i]);
    }

    free(profile_list);

    return true;
}

/* ************************************************************************** */

VideoBackendsVAAPI::VideoBackendsVAAPI()
{
    //
}

VideoBackendsVAAPI::~VideoBackendsVAAPI()
{
    //
}

bool VideoBackendsVAAPI::load(VideoBackendInfos &infos)
{
    bool status = false;

    VADisplay display;
    const char *drm_device  = nullptr;
    const char *x11_display = nullptr;

    // Open a display
    if (drm_device)
    {
        display = open_device_drm(drm_device);
    }
    else //if (x11_display)
    {
        display = open_device_x11(x11_display);
    }

    if (display /*&& (x11_display || drm_device)*/)
    {
        // Init API
        VAStatus vas;
        int major = 0, minor = 0;
        vas = vaInitialize(display, &major, &minor);
        if (vas != VA_STATUS_SUCCESS)
        {
            qDebug() << "Failed to initialise VA-API:" << vas << "(" << vaErrorStr(vas) << ")";
        }
        else
        {
            // Gather infos
            infos.api_name = "VA-API";
            infos.api_version = QString::number(major) + "." + QString::number(minor);

            const char *vendor_string = vaQueryVendorString(display);
            if (vendor_string)
                infos.api_info = QString::fromUtf8(vendor_string);

            dump_profiles(display, infos);

            status = true;
        }
    }

    return status;
}
