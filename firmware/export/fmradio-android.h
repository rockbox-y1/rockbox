/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef FMRADIO_ANDROID_H
#define FMRADIO_ANDROID_H

#include <stdbool.h>

#define HAVE_RADIO_REGION
#define HAVE_RADIO_RSSI

#define tuner_set y1_fmradio_set
#define tuner_get y1_fmradio_get

void y1_fmradio_init(void);
int y1_fmradio_set(int setting, int value);
int y1_fmradio_get(int setting);

/* airplane mode deactivates the wifi/bt/fm chip */
bool y1_fmradio_airplane_mode_on(void);
void y1_fmradio_set_airplane_mode(bool on);

/* debug menu info */
struct y1_fmradio_dbg_info {
    unsigned chip_id;
    int rssi;
    bool powered;
};
void y1_fmradio_dbg_info(struct y1_fmradio_dbg_info *nfo);

#endif /* FMRADIO_ANDROID_H */
