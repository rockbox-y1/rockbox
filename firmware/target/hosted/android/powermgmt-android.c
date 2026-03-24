/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 by Thomas Martitz
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


#include <jni.h>
#include <stdbool.h>
#ifdef PLATFORM_INNIOASIS_Y1
#include <stdlib.h>
#endif
#include "config.h"
#ifdef PLATFORM_INNIOASIS_Y1
#include "power.h"
#include "kernel.h"
#include "misc.h"
#include "splash.h"
#include "lang.h"
#include "settings.h"
#include "../firmware/target/hosted/android/shutdown-android.h"
#endif

extern JNIEnv *env_ptr;
extern jclass  RockboxService_class;
extern jobject RockboxService_instance;

static jfieldID __battery_level;
#ifdef PLATFORM_INNIOASIS_Y1
static jfieldID __plugged_status;
static jfieldID __is_charging;
static jfieldID __battery_time_remaining;
#endif
static jobject BatteryMonitor_instance;

static void new_battery_monitor(void)
{
    JNIEnv e = *env_ptr;
    jclass class = e->FindClass(env_ptr, "org/rockbox/monitors/BatteryMonitor");
    jmethodID constructor = e->GetMethodID(env_ptr, class,
                                            "<init>",
                                            "(Landroid/content/Context;)V");
    BatteryMonitor_instance = e->NewObject(env_ptr, class,
                                            constructor,
                                            RockboxService_instance);

    /* cache the battery level field id */
    __battery_level = (*env_ptr)->GetFieldID(env_ptr,
                                            class,
                                            "mBattLevel", "I");
#ifdef PLATFORM_INNIOASIS_Y1
    /* cache the charging type field id */
    __plugged_status = (*env_ptr)->GetFieldID(env_ptr,
                                            class,
                                            "mPlugged", "I");
    /* cache the charging status field id */
    __is_charging = (*env_ptr)->GetFieldID(env_ptr,
                                            class,
                                            "mCharging", "I");

    /* cache the battery time field id */
    __battery_time_remaining = (*env_ptr)->GetFieldID(env_ptr, 
                                            class, "mEstimatedMinutes", "I");
#endif
}

int _battery_level(void)
{
    if (!BatteryMonitor_instance)
        new_battery_monitor();
    return (*env_ptr)->GetIntField(env_ptr, BatteryMonitor_instance, __battery_level);
}

#ifdef PLATFORM_INNIOASIS_Y1
unsigned int power_input_status(void)
{
    if (!BatteryMonitor_instance)
        new_battery_monitor();

    int plugged_status = (*env_ptr)->GetIntField(env_ptr, BatteryMonitor_instance, __plugged_status);
    if (plugged_status == 1){
        return POWER_INPUT_USB_CHARGER;
    } else if (plugged_status == 2){
        return POWER_INPUT_MAIN_CHARGER;
    } else {
        return POWER_INPUT_NONE;
    }
}

bool charging_state(void)
{
    if (!BatteryMonitor_instance)
        new_battery_monitor();

    int is_charging = (*env_ptr)->GetIntField(env_ptr, BatteryMonitor_instance, __is_charging);

    return is_charging == 1;
}

void sys_poweroff(void)
{
    list_stop_handler();
    splash(2*HZ, ID2P(LANG_SHUTTINGDOWN));
    sleep(1);
    android_shutdown_device(0);
}

int _battery_time(void)
{
    if (!BatteryMonitor_instance)
        new_battery_monitor();
    
    int level = _battery_level();
    if (level < 0 || charging_state())
        return -1;
    
    int estimated = (*env_ptr)->GetIntField(env_ptr, BatteryMonitor_instance, 
                                           __battery_time_remaining);
    return estimated > 0 ? estimated : -1;
}

int android_acquire_wakelock(void)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID firmware_switch_method = NULL;
    if (firmware_switch_method == NULL) {
        firmware_switch_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "acquireWakeLock", "()V");
        if (firmware_switch_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, firmware_switch_method);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}

int android_release_wakelock(void)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID firmware_switch_method = NULL;
    if (firmware_switch_method == NULL) {
        firmware_switch_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "releaseWakeLock", "()V");
        if (firmware_switch_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, firmware_switch_method);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}
#endif