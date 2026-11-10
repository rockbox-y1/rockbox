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

#include "config.h"

#if defined(INNIOASIS_Y1) && CONFIG_TUNER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <jni.h>
#define _SYSTEM_WITH_JNI
#include "system.h"
#include "kernel.h"
#include "debug.h"
#include "tuner.h"

extern JNIEnv *env_ptr;

#define FM_IOC_MAGIC 0xf5

struct fm_tune_parm {
    uint8_t  err;
    uint8_t  band;
    uint8_t  space;
    uint8_t  hilo;
    uint16_t freq;
};

struct fm_softmute_tune_t {
    int32_t  rssi;
    uint16_t freq;   /*  units of 10 kHz */
    int32_t  valid;
};

#define FM_IOC_POWERUP   _IOWR(FM_IOC_MAGIC, 0,  struct fm_tune_parm *)
#define FM_IOC_POWERDOWN _IOW (FM_IOC_MAGIC, 1,  int32_t *)
#define FM_IOC_TUNE      _IOWR(FM_IOC_MAGIC, 2,  struct fm_tune_parm *)
#define FM_IOC_MUTE      _IOWR(FM_IOC_MAGIC, 6,  uint32_t *)
#define FM_IOC_GETRSSI   _IOWR(FM_IOC_MAGIC, 7,  int32_t *)
#define FM_IOC_GETCHIPID _IOWR(FM_IOC_MAGIC, 10, uint16_t *)
#define FM_IOC_SETMONOSTEREO   _IOWR(FM_IOC_MAGIC, 37, int32_t *)
#define FM_IOC_ANT_SWITCH      _IOWR(FM_IOC_MAGIC, 30, int32_t *)
#define FM_IOC_PRE_SEARCH      _IOWR(FM_IOC_MAGIC, 45, int32_t *)
#define FM_IOC_RESTORE_SEARCH  _IOWR(FM_IOC_MAGIC, 46, int32_t *)
#define FM_IOC_SOFTMUTE_TUNE   _IOWR(FM_IOC_MAGIC, 63, struct fm_softmute_tune_t *)

#define FM_DEV          "/dev/fm"
#define FM_BAND_UE      1
#define FM_BAND_JAPAN   2
#define FM_SPACE_100K   1
#define FM_SPACE_200K   2
#define FM_SPACE_50K    5
#define FM_HILO_OFF     0
#define FM_PWR_RX       0

/* RSSI in dBm (used for signal bar) */
#define Y1_RSSI_MIN (-100)
#define Y1_RSSI_MAX (-30)
/* a channel counts as tuned above this level */
#define Y1_TUNED_RSSI_DBM (-90)

static int fm_fd = -1;
static bool fm_present = false;
static bool fm_powered = false;
static int cur_freq = 87500000;
static int cur_region = REGION_EUROPE;
static bool force_mono = false;
static bool last_valid = false;
static int last_rssi = Y1_RSSI_MIN;
static struct mutex fmr_mutex;

/* JNI handles for org.rockbox.Helper.FMRadio */
static jclass fm_jclass = NULL;
static jmethodID fm_enable_audio_method = NULL;
static jmethodID fm_airplane_mode_on_method = NULL;
static jmethodID fm_set_airplane_mode_method = NULL;
static jmethodID fm_wired_headset_on_method = NULL;

static void jni_cache_methods(void)
{
    JNIEnv *env = env_ptr;
    if (env == NULL)
        return;
    jclass local = (*env)->FindClass(env, "org/rockbox/Helper/FMRadio");
    if (local == NULL)
    {
        (*env)->ExceptionClear(env);
        return;
    }
    fm_jclass = (*env)->NewGlobalRef(env, local);
    (*env)->DeleteLocalRef(env, local);
    if (fm_jclass == NULL)
        return;
    fm_enable_audio_method = (*env)->GetStaticMethodID(env, fm_jclass,
                                    "enableFmAudio", "(Z)V");
    fm_airplane_mode_on_method = (*env)->GetStaticMethodID(env, fm_jclass,
                                    "isAirplaneModeOn", "()Z");
    fm_set_airplane_mode_method = (*env)->GetStaticMethodID(env, fm_jclass,
                                    "setAirplaneMode", "(Z)V");
    fm_wired_headset_on_method = (*env)->GetStaticMethodID(env, fm_jclass,
                                    "isWiredHeadsetOn", "()Z");
}

static void fm_set_audio_path(bool enable)
{
    if (fm_jclass == NULL || fm_enable_audio_method == NULL)
        return;
    JNIEnv *env = getJavaEnvironment();
    if (env == NULL)
        return;
    (*env)->CallStaticVoidMethod(env, fm_jclass, fm_enable_audio_method,
                                 (jboolean)enable);
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
}

/* FM radio doens't work in airplane mode, check first, change if needed */
bool y1_fmradio_airplane_mode_on(void)
{
    if (fm_jclass == NULL || fm_airplane_mode_on_method == NULL)
        return false;
    JNIEnv *env = getJavaEnvironment();
    if (env == NULL)
        return false;
    jboolean ret = (*env)->CallStaticBooleanMethod(env, fm_jclass,
                                                   fm_airplane_mode_on_method);
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
    return ret == JNI_TRUE;
}

void y1_fmradio_set_airplane_mode(bool on)
{
    if (fm_jclass == NULL || fm_set_airplane_mode_method == NULL)
        return;
    JNIEnv *env = getJavaEnvironment();
    if (env == NULL)
        return;
    (*env)->CallStaticVoidMethod(env, fm_jclass, fm_set_airplane_mode_method,
                                 (jboolean)on);
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
}

/* check if headphones are plugged in (acting as antenna) */
static bool fm_wired_headset_on(void)
{
    if (fm_jclass == NULL || fm_wired_headset_on_method == NULL)
        return true; /* assume headset so we don't touch the antenna switch */
    JNIEnv *env = getJavaEnvironment();
    if (env == NULL)
        return true;
    jboolean ret = (*env)->CallStaticBooleanMethod(env, fm_jclass,
                                                   fm_wired_headset_on_method);
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
    return ret == JNI_TRUE;
}

static bool fm_open(void)
{
    if (fm_fd < 0)
        fm_fd = open(FM_DEV, O_RDWR);
    return fm_fd >= 0;
}

static int fm_tune(int freq_hz);

static void fm_fill_tune_parm(struct fm_tune_parm *p, int freq_hz)
{
    const struct fm_region_data *rd = &fm_region_data[cur_region];

    memset(p, 0, sizeof(*p));
    p->band = (rd->freq_min <= 76000000 && rd->freq_max <= 90000000)
              ? FM_BAND_JAPAN : FM_BAND_UE;
    switch (rd->freq_step)
    {
        case 50000:  p->space = FM_SPACE_50K;  break;
        case 200000: p->space = FM_SPACE_200K; break;
        default:     p->space = FM_SPACE_100K; break;
    }
    p->hilo = FM_HILO_OFF;
    p->freq = (uint16_t)(freq_hz / 10000);
}

static int fm_power_on(void)
{
    struct fm_tune_parm p;
    uint32_t mute = 1;
    int32_t mono = 1;

    if (!fm_open())
        return -1;

    fm_fill_tune_parm(&p, cur_freq);
    if (ioctl(fm_fd, FM_IOC_POWERUP, &p) < 0 || p.err)
        return -1;

    fm_powered = true;
    /* keep receiver muted until radio.c explicitly unmutes */
    ioctl(fm_fd, FM_IOC_MUTE, &mute);
    if (force_mono)
        ioctl(fm_fd, FM_IOC_SETMONOSTEREO, &mono);

    /* no antenna switch to the (bad) internal antenna and retune */
    if (!fm_wired_headset_on())
    {
        int32_t antenna = 1;
        if (ioctl(fm_fd, FM_IOC_ANT_SWITCH, &antenna) == 0)
            fm_tune(cur_freq);
    }

    /* route the analog FM output through the Android audio HAL */
    fm_set_audio_path(true);
    return 0;
}

static void fm_power_off(void)
{
    uint32_t mute = 1;
    int32_t type = FM_PWR_RX;

    if (fm_fd >= 0)
    {
        ioctl(fm_fd, FM_IOC_MUTE, &mute);
        ioctl(fm_fd, FM_IOC_POWERDOWN, &type);
        close(fm_fd);
        fm_fd = -1;
    }
    fm_powered = false;
    fm_set_audio_path(false);
}

static int fm_tune(int freq_hz)
{
    struct fm_tune_parm p;

    fm_fill_tune_parm(&p, freq_hz);
    if (ioctl(fm_fd, FM_IOC_TUNE, &p) < 0 || p.err)
        return -1;

    cur_freq = freq_hz;
    return 0;
}

static bool fm_probe_channel(int freq_hz)
{
    struct fm_softmute_tune_t probe;
    int32_t unused = 0;
    bool valid = false;

    ioctl(fm_fd, FM_IOC_PRE_SEARCH, &unused);
    memset(&probe, 0, sizeof(probe));
    probe.freq = (uint16_t)(freq_hz / 10000);
    if (ioctl(fm_fd, FM_IOC_SOFTMUTE_TUNE, &probe) == 0)
    {
        valid = probe.valid != 0;
        last_rssi = probe.rssi;
    }
    ioctl(fm_fd, FM_IOC_RESTORE_SEARCH, &unused);
    return valid;
}

static int fm_get_rssi(void)
{
    int32_t rssi = 0;
    if (ioctl(fm_fd, FM_IOC_GETRSSI, &rssi) == 0)
        return rssi;
    return last_rssi;
}

void y1_fmradio_init(void)
{
    mutex_init(&fmr_mutex);
    jni_cache_methods();

    if (fm_open())
    {
        uint16_t chip_id = 0;
        if (ioctl(fm_fd, FM_IOC_GETCHIPID, &chip_id) == 0)
            DEBUGF("radio-innioasis: /dev/fm present, chip id 0x%04x\n", chip_id);
        else
            DEBUGF("radio-innioasis: /dev/fm present, chip id unknown (airplane mode?)\n");
        fm_present = true;
        close(fm_fd);
        fm_fd = -1;
    }
    else
    {
        DEBUGF("radio-innioasis: no /dev/fm, FM radio not present\n");
    }
}

int y1_fmradio_set(int setting, int value)
{
    int val = 1;

    if (!fm_powered && setting != RADIO_SLEEP)
        return -1;

    mutex_lock(&fmr_mutex);

    switch (setting)
    {
        case RADIO_SLEEP:
            if (value == 0)
                val = fm_power_on();
            else
            {
                fm_power_off();
                val = 0;
            }
            break;

        case RADIO_FREQUENCY:
            val = fm_tune(value);
            break;

        case RADIO_SCAN_FREQUENCY:
            /* report whether the channel holds a valid station then stop there */
            last_valid = fm_probe_channel(value);
            fm_tune(value);
            val = last_valid ? 1 : 0;
            break;

        case RADIO_MUTE:
        {
            uint32_t mute = value ? 1 : 0;
            val = ioctl(fm_fd, FM_IOC_MUTE, &mute);
            break;
        }

        case RADIO_REGION:
            cur_region = value;
            /* applied on next tune/powerup */
            break;

        case RADIO_FORCE_MONO:
        {
            int32_t mono = value ? 1 : 0;
            force_mono = value != 0;
            val = ioctl(fm_fd, FM_IOC_SETMONOSTEREO, &mono);
            break;
        }

        default:
            val = -1;
            break;
    }

    mutex_unlock(&fmr_mutex);

    return val;
}

int y1_fmradio_get(int setting)
{
    int val = -1;

    if (!fm_powered && setting != RADIO_PRESENT)
        return -1;

    mutex_lock(&fmr_mutex);

    switch (setting)
    {
        case RADIO_PRESENT:
            val = fm_present ? 1 : 0;
            break;

        case RADIO_TUNED:
            val = (fm_get_rssi() >= Y1_TUNED_RSSI_DBM) ? 1 : 0;
            break;

        case RADIO_STEREO:
            /* the driver does not report stereo reception, let's just assume stereo whenever we are tuned */
            val = (fm_get_rssi() >= Y1_TUNED_RSSI_DBM) ? 1 : 0;
            break;

        case RADIO_RSSI:
        {
            int rssi = fm_get_rssi();
            if (rssi < Y1_RSSI_MIN) rssi = Y1_RSSI_MIN;
            if (rssi > Y1_RSSI_MAX) rssi = Y1_RSSI_MAX;
            val = rssi;
            break;
        }

        case RADIO_RSSI_MIN:
            val = Y1_RSSI_MIN;
            break;

        case RADIO_RSSI_MAX:
            val = Y1_RSSI_MAX;
            break;
    }

    mutex_unlock(&fmr_mutex);

    return val;
}

void y1_fmradio_dbg_info(struct y1_fmradio_dbg_info *nfo)
{
    memset(nfo, 0, sizeof(*nfo));
    mutex_lock(&fmr_mutex);
    nfo->powered = fm_powered;
    if (fm_powered)
    {
        uint16_t chip_id = 0;
        if (fm_open() && ioctl(fm_fd, FM_IOC_GETCHIPID, &chip_id) == 0)
            nfo->chip_id = chip_id;
        nfo->rssi = fm_get_rssi();
    }
    mutex_unlock(&fmr_mutex);
}

#endif /* INNIOASIS_Y1 && CONFIG_TUNER */
