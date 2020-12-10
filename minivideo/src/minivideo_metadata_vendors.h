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
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2020
 */

#ifndef MINIVIDEO_METADATA_VENDORS_H
#define MINIVIDEO_METADATA_VENDORS_H
/* ************************************************************************** */

#include <cstdint>

typedef struct metadata_gopro_t {
    char camera_firmware[32];
    char camera_serial[32];

    // settings
    uint8_t startup_mode;
    uint8_t photo_mode;
    uint8_t timelapse_interval;
    uint8_t image_flip;
    uint8_t exposure_style;

    uint8_t white_balance;
    uint8_t iso_limit;
    uint8_t one_button_mode;
    uint8_t osd;
    uint8_t leds_active;
    uint8_t beep_active;
    uint8_t auto_off;
    uint8_t stereo_mode;

    uint8_t protune;
    uint8_t cam_raw;

    // settings
    uint8_t broadcast_range;
    uint8_t video_mode_fov;
    uint8_t lens_type;
    uint8_t lowlight;
    uint8_t superview;
    uint8_t sharpening;
    uint8_t color_curve;
    uint8_t iso_limit2;
    uint8_t EV_compensation;
    uint8_t white_balance2;
    uint8_t unnamed;
    bool eis;

    // settings
    uint8_t media_type;

    // settings
    uint32_t upload_status;

} metadata_gopro_t;

/* ************************************************************************** */
#endif // MINIVIDEO_METADATA_VENDORS_H
