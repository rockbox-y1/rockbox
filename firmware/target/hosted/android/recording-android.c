/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   |  | _\_ |__   _______  ___
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

#include <jni.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define _SYSTEM_WITH_JNI
#include <pthread.h>
#include "system.h"
#include "kernel.h"
#include "debug.h"
#include "logf.h"
#include "pcm.h"
#include "pcm-internal.h"
#include "audiohw.h"

extern JNIEnv *env_ptr;

/* protects rec_addr/rec_size/rec_filled/rec_active */
static pthread_mutex_t rec_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int rec_suspended = 1;

static int      rec_active = 0;
static uint8_t *rec_addr   = NULL;
static size_t   rec_size   = 0;
static size_t   rec_filled = 0;

static jclass   Recorder_class;
static jmethodID rec_start_method;
static jmethodID rec_stop_method;

void pcm_rec_lock(void)
{
    rec_suspended = 1;
}

void pcm_rec_unlock(void)
{
    rec_suspended = 0;
}

void pcm_rec_dma_init(void)
{
    JNIEnv *env = env_ptr;
    jclass cls = (*env)->FindClass(env, "org/rockbox/Helper/Recorder");
    if (!cls)
    {
        logf("rec: Recorder class not found");
        return;
    }
    Recorder_class  = (*env)->NewGlobalRef(env, cls);
    rec_start_method = (*env)->GetStaticMethodID(env, Recorder_class, "start", "(I)V");
    rec_stop_method  = (*env)->GetStaticMethodID(env, Recorder_class, "stop", "()V");
    (*env)->DeleteLocalRef(env, cls);
}

void pcm_rec_dma_close(void)
{
    if (Recorder_class)
    {
        JNIEnv *env = env_ptr;
        (*env)->DeleteGlobalRef(env, Recorder_class);
        Recorder_class = NULL;
    }
}

void pcm_rec_dma_start(void *addr, size_t size)
{
    logf("rec: dma start %p %d", addr, (int)size);

    pthread_mutex_lock(&rec_mutex);
    rec_addr   = addr;
    rec_size   = size;
    rec_filled = 0;
    rec_active = 1;
    pthread_mutex_unlock(&rec_mutex);

    /* pcm_apply_settings() ran just before, so this is the recording rate */
    unsigned int rate = pcm_get_frequency();
    JNIEnv *env = getJavaEnvironment();
    (*env)->CallStaticVoidMethod(env, Recorder_class, rec_start_method, (jint)rate);
}

void pcm_rec_dma_stop(void)
{
    logf("rec: dma stop");

    pthread_mutex_lock(&rec_mutex);
    rec_active = 0;
    rec_addr   = NULL;
    pthread_mutex_unlock(&rec_mutex);

    if (Recorder_class)
    {
        JNIEnv *env = getJavaEnvironment();
        (*env)->CallStaticVoidMethod(env, Recorder_class, rec_stop_method);
    }
}

const void * pcm_rec_dma_get_peak_buffer(void)
{
    pthread_mutex_lock(&rec_mutex);
    const uint8_t *p = rec_addr ? rec_addr + rec_filled : NULL;
    pthread_mutex_unlock(&rec_mutex);
    /* align */
    return p ? (const void *)(((uintptr_t)p + 3) & ~(uintptr_t)3) : NULL;
}

/* (data: mono 16-bit PCM from the microphone)
 * converts mono to stereo, appends that to the recording buffer */
 JNIEXPORT jint JNICALL
Java_org_rockbox_Helper_Recorder_nativeRecordData(JNIEnv *env, jclass cls,
                                                  jbyteArray data,
                                                  jint offset, jint length)
{
    (void)cls;

    if (rec_suspended || length <= 0)
        return 0;

    jbyte *src = (*env)->GetByteArrayElements(env, data, NULL);
    if (!src)
        return 0;

    jint consumed = 0;
    const uint8_t *s = (const uint8_t *)src + offset;

    while (consumed < length)
    {
        pthread_mutex_lock(&rec_mutex);
        if (!rec_active || !rec_addr)
        {
            pthread_mutex_unlock(&rec_mutex);
            break;
        }

        size_t space  = rec_size - rec_filled;   /* bytes left in chunk */
        size_t frames = (length - consumed) / 2; /* mono frames left */
        size_t fit    = space / 4;               /* stereo frames that fit */
        size_t c      = MIN(frames, fit);

        int16_t *dst = (int16_t *)(rec_addr + rec_filled);
        const int16_t *in = (const int16_t *)s;
        for (size_t i = 0; i < c; i++)
        {
            int16_t v = in[i];
            dst[2*i]     = v;
            dst[2*i + 1] = v;
        }
        rec_filled += c * 4;
        consumed   += c * 2;
        s          += c * 2;

        bool full = (rec_filled >= rec_size);
        void  *naddr = rec_addr;
        size_t nsize = rec_size;
        pthread_mutex_unlock(&rec_mutex);

        if (full)
        {
            /* must not hold rec_mutex here: the callbacks may call pcm_rec_lock()/pcm_rec_unlock() */
            if (!pcm_rec_dma_complete_callback(PCM_DMAST_OK, &naddr, &nsize))
            {
                pthread_mutex_lock(&rec_mutex);
                rec_active = 0;
                rec_addr   = NULL;
                pthread_mutex_unlock(&rec_mutex);
                break;
            }
            pthread_mutex_lock(&rec_mutex);
            if (!rec_active) /* stopped while fetching the next chunk */
            {
                pthread_mutex_unlock(&rec_mutex);
                break;
            }
            rec_addr   = naddr;
            rec_size   = nsize;
            rec_filled = 0;
            pthread_mutex_unlock(&rec_mutex);
        }
    }

    (*env)->ReleaseByteArrayElements(env, data, src, JNI_ABORT);
    return consumed;
}

/* API 16 has no gain control, just ignore the setting */
void audiohw_set_recvol(int left, int right, int type)
{
    (void)left;
    (void)right;
    (void)type;
}
