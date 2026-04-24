/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2010 Thomas Martitz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 *****************************************************************************/


#include "button.h"
#include "android_keyevents.h"

static bool ignore_back_button = false;
void android_ignore_back_button(bool yes)
{
    ignore_back_button = yes;
}

int key_to_button(int keyboard_key)
{
    switch (keyboard_key)
    {
        case KEYCODE_BACK:
#ifndef INNIOASIS_Y1
            return ignore_back_button ? BUTTON_NONE : BUTTON_BACK;
        case KEYCODE_MENU:
#endif
            return BUTTON_MENU;
        case KEYCODE_ENTER:
            return BUTTON_SELECT;
#ifndef INNIOASIS_Y1
        case KEYCODE_VOLUME_UP:
            return BUTTON_VOL_UP;
        case KEYCODE_VOLUME_DOWN:
            return BUTTON_VOL_DOWN;
#endif
        default:
            return BUTTON_NONE;
    }
}

unsigned multimedia_to_button(int keyboard_key)
{
    switch (keyboard_key)
    {
        case KEYCODE_MEDIA_PLAY_PAUSE:
#ifdef INNIOASIS_Y1
            return BUTTON_PLAY;
#else
        case KEYCODE_MEDIA_STOP:
            return BUTTON_MULTIMEDIA_STOP;
#endif
        case KEYCODE_MEDIA_NEXT:
#ifdef INNIOASIS_Y1
            return BUTTON_RIGHT;
#else
            return BUTTON_MULTIMEDIA_NEXT;
#endif
        case KEYCODE_MEDIA_PREVIOUS:
#ifdef INNIOASIS_Y1
            return BUTTON_LEFT;
#else
            return BUTTON_MULTIMEDIA_PREV;
#endif
#ifdef INNIOASIS_Y1
        /* KEYCODE_MEDIA_PLAY and KEYCODE_MEDIA_PAUSE are mapped to the the scroll buttons.
           This way Android recognizes these buttons even when the screen is off. */
        case KEYCODE_MEDIA_PLAY:
            return BUTTON_SCROLL_BACK;
        case KEYCODE_MEDIA_PAUSE:
            return BUTTON_SCROLL_FWD;
#else
        case KEYCODE_MEDIA_REWIND:
            return BUTTON_MULTIMEDIA_REW;
        case KEYCODE_MEDIA_FAST_FORWARD:
            return BUTTON_MULTIMEDIA_FFWD;
#endif
        default:
            return 0;
    }
}

unsigned dpad_to_button(int keyboard_key)
{
    switch (keyboard_key)
    {
        /* These buttons only post a single release event.
         * doing otherwise will cause action.c to lock up waiting for
         * a release (because android sends press/unpress to us too quickly
         */
        
        /*
        case KEYCODE_DPAD_UP:
            return BUTTON_DPAD_UP;
        case KEYCODE_DPAD_DOWN:
            return BUTTON_DPAD_DOWN;
        case KEYCODE_DPAD_LEFT:
            return BUTTON_DPAD_LEFT;
        case KEYCODE_DPAD_RIGHT:
            return BUTTON_DPAD_RIGHT;
        */
        default:
            return BUTTON_NONE;
    }
}
    
