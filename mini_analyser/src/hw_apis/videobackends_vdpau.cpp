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
 * \file      videobackends_vdpau.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 *
 * This file is heavily based on 'vdpauinfo.cpp' from NVIDIA Corporation.
 * 'vdpauinfo.cpp' is published under the MIT license.
 */

#include "videobackends_vdpau.h"
#include <minivideo_codecs.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <QDebug>

#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>
#include "vdpau/VDPDeviceImpl.h"

/* ************************************************************************** */

// Generic description structure
struct Desc
{
    uint32_t id;

    uint32_t codec;
    uint32_t profile;
    uint32_t bitdepth;
};

Desc decoder_profiles_vdpau[] =
{
    { VDP_DECODER_PROFILE_MPEG1,                        CODEC_MPEG1,    PROF_H262_SP, 8 },
    { VDP_DECODER_PROFILE_MPEG2_SIMPLE,                 CODEC_MPEG2,    PROF_H262_SP, 8 },
    { VDP_DECODER_PROFILE_MPEG2_MAIN,                   CODEC_MPEG2,    PROF_H262_MP, 8 },
    { VDP_DECODER_PROFILE_H264_BASELINE,                CODEC_H264,     PROF_H264_BP, 8 },
    { VDP_DECODER_PROFILE_H264_MAIN,                    CODEC_H264,     PROF_H264_MP, 8 },
    { VDP_DECODER_PROFILE_H264_HIGH,                    CODEC_H264,     PROF_H264_HiP, 8 },
    { VDP_DECODER_PROFILE_VC1_SIMPLE,                   CODEC_VC1,      PROF_VC1_SIMPLE, 8 },
    { VDP_DECODER_PROFILE_VC1_MAIN,                     CODEC_VC1,      PROF_VC1_MAIN, 8 },
    { VDP_DECODER_PROFILE_VC1_ADVANCED,                 CODEC_VC1,      PROF_VC1_ADVANCED, 8 },
    { VDP_DECODER_PROFILE_MPEG4_PART2_SP,               CODEC_MPEG4_ASP,PROF_MPEG4_SP, 8 },
    { VDP_DECODER_PROFILE_MPEG4_PART2_ASP,              CODEC_MPEG4_ASP,PROF_MPEG4_ASP, 8 },
    { VDP_DECODER_PROFILE_DIVX4_QMOBILE,                CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX4_MOBILE,                 CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX4_HOME_THEATER,           CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX4_HD_1080P,               CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX5_QMOBILE,                CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX5_MOBILE,                 CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX5_HOME_THEATER,           CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_DIVX5_HD_1080P,               CODEC_MPEG4_ASP, 0, 8 },
    { VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE,    CODEC_H264,     PROF_H264_CBP, 8 },
    { VDP_DECODER_PROFILE_H264_EXTENDED,                CODEC_H264,     PROF_H264_XP, 8 },
    { VDP_DECODER_PROFILE_H264_PROGRESSIVE_HIGH,        CODEC_H264,     PROF_H264_PHiP, 8 },
    { VDP_DECODER_PROFILE_H264_CONSTRAINED_HIGH,        CODEC_H264,     PROF_H264_CHiP, 8 },
    { VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE,     CODEC_H264,     PROF_H264_Hi444PP, 8 },
    { VDP_DECODER_PROFILE_HEVC_MAIN,                    CODEC_H265,     PROF_H265_Main, 8 },
    { VDP_DECODER_PROFILE_HEVC_MAIN_10,                 CODEC_H265,     PROF_H265_Main10, 10 },
    { VDP_DECODER_PROFILE_HEVC_MAIN_STILL,              CODEC_H265,     PROF_H265_MainStill, 8 },
    { VDP_DECODER_PROFILE_HEVC_MAIN_12,                 CODEC_H265,     PROF_H265_Main12, 12 },
    { VDP_DECODER_PROFILE_HEVC_MAIN_444,                CODEC_H265,     PROF_H265_Main444, 8 },
};

const size_t decoder_profile_count = sizeof(decoder_profiles_vdpau) / sizeof(Desc);

/* ************************************************************************** */

bool queryBaseInfo(VDPDeviceImpl *device, VideoBackendInfos &infos)
{
    uint32_t api;
    const char *info;

    if (device->GetApiVersion(&api) != VDP_STATUS_OK ||
        device->GetInformationString(&info) != VDP_STATUS_OK)
    {
        qDebug("Error querying API version or information\n");
        return false;
    }
    else
    {
        infos.api_version = QString::number(api);
        infos.api_info = QString::fromLocal8Bit(info);
    }

    return true;
}

bool queryDecoderCaps(VDPDeviceImpl *device, VideoBackendInfos &infos)
{
    VdpStatus rv;

    for (size_t x = 0; x < decoder_profile_count; x++)
    {
        VdpBool is_supported;
        uint32_t max_level, max_macroblocks, max_width, max_height;

        rv = device->DecoderQueryCapabilities(device->device,
                                              decoder_profiles_vdpau[x].id,
                                              &is_supported, &max_level,
                                              &max_macroblocks,
                                              &max_width, &max_height);

        if (rv == VDP_STATUS_OK && is_supported)
        {
            CodecSupport c;
            c.codec = decoder_profiles_vdpau[x].codec;
            c.profile = decoder_profiles_vdpau[x].profile;

            c.max_width = max_width;
            c.max_height = max_height;
            c.max_bitdepth = decoder_profiles_vdpau[x].bitdepth;

            infos.decodingSupport.push_back(c);
        }
        else
        {
            //qDebug() << decoder_profiles[x].name << "is not supported...";
        }
    }

    return true;
}

/* ************************************************************************** */

VideoBackendsVDPAU::VideoBackendsVDPAU()
{
    //
}

VideoBackendsVDPAU::~VideoBackendsVDPAU()
{
    //
}

bool VideoBackendsVDPAU::load(VideoBackendInfos &infos)
{
    bool status = false;

    VdpDevice device;
    VDPDeviceImpl *deviceI = nullptr;

    // Create an X Display
    Display *display = XOpenDisplay(nullptr);
    char *display_name = XDisplayName(nullptr);
    if (!display || !display_name)
    {
        if (display_name)
            qDebug() << "vdpau: Cannot connect to X server for display:" << display_name;
        else
            qDebug() << "vdpau: Cannot connect to X server";

        return false;
    }

    int screen = 0; //(screen == -1) ? DefaultScreen(display) : -1;
/*
    if(screen >= ScreenCount(display))
        invalid_argument(argv[0], "screen %d requested but X server only has %d screen%s\n",
                         screen, ScreenCount(display),
                         ScreenCount(display) == 1 ? "" : "s");
*/
    qDebug() << "vdpau: display:" << display_name << "   screen:" << screen;

    // Create device
    VdpGetProcAddress *get_proc_address = nullptr;
    VdpStatus rv = vdp_device_create_x11(display, screen, &device, &get_proc_address);
    if (rv != VDP_STATUS_OK || !get_proc_address)
    {
        qDebug() << "vdpau: Error creating VDPAU device:" << rv;
        return false;
    }

    deviceI = new VDPDeviceImpl(device, get_proc_address);
    if (deviceI)
    {
        // Gather infos
        infos.api_name = "VDPAU";
        queryBaseInfo(deviceI, infos);
        queryDecoderCaps(deviceI, infos);

        status = true;
    }

    return status;
}
