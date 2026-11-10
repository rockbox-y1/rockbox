/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
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
 ****************************************************************************/

package org.rockbox;

import java.nio.ByteBuffer;

import org.rockbox.RockboxService;
import org.rockbox.Helper.Connectivity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewConfiguration;
import android.os.Vibrator;
import android.util.Log;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemClock;
import android.graphics.Paint;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class RockboxFramebuffer extends SurfaceView 
                                 implements SurfaceHolder.Callback
{
    private final DisplayMetrics metrics;
    private final ViewConfiguration view_config;
    private Bitmap btm;
    private final Paint sharpPaint = new Paint();

    /* surface lifecycle states/objects */
    private final Object surfaceLock = new Object();
    private boolean surfaceReady = false;
    private boolean surfaceEnabled = false;

    /* watchdog for detecting freezes */
    private final Handler watchdogHandler = new Handler(Looper.getMainLooper());
    private volatile long lastUpdateTime = 0;
    private static final long WATCHDOG_TIMEOUT_MS = 5000;

    private static final int[] duration_mapping = {
        0, 1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50
    };

    private static final int PLAY_KEYCODE = KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE;
    private static final int SCROLL_BACK_KEYCODE = KeyEvent.KEYCODE_MEDIA_PLAY;
    private static final int SCROLL_FWD_KEYCODE = KeyEvent.KEYCODE_MEDIA_PAUSE;
    private static final int SKIP_NEXT_KEYCODE = KeyEvent.KEYCODE_MEDIA_NEXT;
    private static final int SKIP_PREV_KEYCODE = KeyEvent.KEYCODE_MEDIA_PREVIOUS;

    private static final int BACK_KEYCODE = 4;

    /* BACK is handled like MENU in Rockbox, but in the main
     * menu rockbox lets android handle BACK so we can exit the app. When BACK is pressed
     * together with PLAY we send MENU instead so the MENU+PLAY keylock combo still triggers
     * All cases this covers:
     * - Press BACK+PLAY in any menu -> send MENU+PLAY (triggers keylock)
     * - Press BACK in main menu -> android handles this, exits rockbox back to launcher
     * - Press BACK anywhere else -> send BACK, which rockbox then translates to MENU */
    private static boolean playHeld = false;
    private static boolean backHeld = false;
    private static boolean backSentAsMenu = false;

    static boolean handleKey(int keyCode, boolean down) {
        if (keyCode == PLAY_KEYCODE) {
            playHeld = down;
            if (down && backHeld && !backSentAsMenu) {
                backSentAsMenu = true;
                buttonHandler(KeyEvent.KEYCODE_MENU, true);
            }
            return buttonHandler(keyCode, down);
        }
        if (keyCode == BACK_KEYCODE) {
            backHeld = down;
            if (down) {
                if (playHeld) {
                    backSentAsMenu = true;
                    return buttonHandler(KeyEvent.KEYCODE_MENU, true);
                }
                /* solo back press in main menu, exit rockbox */
                return buttonHandler(keyCode, true);
            } else {
                if (backSentAsMenu) {
                    backSentAsMenu = false;
                    return buttonHandler(KeyEvent.KEYCODE_MENU, false);
                }
                return buttonHandler(keyCode, false);
            }
        }
        return buttonHandler(keyCode, down);
    }

    private static boolean handleKeyRepeat(int keyCode) {
        if (keyCode == BACK_KEYCODE && (backSentAsMenu || isBackButtonIgnored())) {
            /* long-press back: in the main menu the initial down was ignored by
             * rockbox (so the host can exit on a short press). Send MENU down
             * event first then forward the repeat as MENU. */
            if (!backSentAsMenu) {
                backSentAsMenu = true;
                buttonHandler(KeyEvent.KEYCODE_MENU, true);
            }
            return buttonHandlerRepeat(KeyEvent.KEYCODE_MENU);
        }
        return buttonHandlerRepeat(keyCode);
    }
    private static long lastScrollPressTime = 0;
    private static final long SCROLL_REPEAT_THRESHOLD_MS = 100;

    public static void screenOff() {
        try {
            PowerManager pm = (PowerManager) RockboxService.getInstance()
                    .getSystemService(Context.POWER_SERVICE);
            pm.goToSleep(SystemClock.uptimeMillis());
            Log.d("RockboxButton", "Device put to sleep");
        } catch (Exception e) {
            Log.e("RockboxButton", "Failed to put device to sleep: " + e.getMessage());
        }
    }

    public static void shutdown() {
        try {
            RockboxService.getInstance().shutdownDevice(0);
            Log.d("RockboxButton", "Shutdown device...");
        } catch (Exception e) {
            Log.e("RockboxButton", "Shutdown failed: " + e.getMessage());
        }
    }

    /* first stage init; needs to run from a thread that has a Looper 
     * setup stuff that needs a Context */
    public RockboxFramebuffer(Context c)
    {
        super(c);
        metrics = c.getResources().getDisplayMetrics();
        view_config = ViewConfiguration.get(c);
        getHolder().addCallback(this);
        /* Needed so we can catch KeyEvents */
        setFocusable(true);
        setFocusableInTouchMode(true);
        setClickable(true);
        /* don't draw until native is ready (2nd stage) */
        setEnabled(false);
        sharpPaint.setFilterBitmap(false);
    }

    private void update(ByteBuffer framebuffer)
    {
        /* check if surface ready before drawing */
        synchronized (surfaceLock) {
            if (!surfaceReady) {
                return;
            }
        }

        SurfaceHolder holder = getHolder();                            
        Canvas c = holder.lockCanvas();
        if (c == null) {
            Log.w("RockboxFramebuffer", "update: lockCanvas returned null");
            return;
        }

        btm.copyPixelsFromBuffer(framebuffer);
        synchronized (holder)
        { /* draw */
            c.drawBitmap(btm, 0.0f, 0.0f, null);
        }
        holder.unlockCanvasAndPost(c);

        /* last drawing (used by watchdog) */
        lastUpdateTime = System.currentTimeMillis();
    }
    
    private void update(ByteBuffer framebuffer, Rect dirty)
    {
        /* Check if surface is ready before attempting to draw */
        synchronized (surfaceLock) {
            if (!surfaceReady) {
                return;
            }
        }

        SurfaceHolder holder = getHolder();                            
        Canvas c = holder.lockCanvas(dirty);

        if (c == null) {
            Log.w("RockboxFramebuffer", "update(dirty): lockCanvas returned null");
            return;
        }

        /* can't copy a partial buffer, but it doesn't make a noticeable difference anyway */
        btm.copyPixelsFromBuffer(framebuffer);
        synchronized (holder)
        {   /* draw */
            c.drawBitmap(btm, dirty, dirty, null);
        }
        holder.unlockCanvasAndPost(c);

        /* last drawing (used by watchdog) */
        lastUpdateTime = System.currentTimeMillis();
    }

    public boolean onTouchEvent(MotionEvent me)
    {        
        int x = (int) me.getX();
        int y = (int) me.getY();

        switch (me.getAction())
        {
        case MotionEvent.ACTION_CANCEL:
        case MotionEvent.ACTION_UP:
            touchHandler(false, x, y);
            return true;
        case MotionEvent.ACTION_MOVE:
        case MotionEvent.ACTION_DOWN:
            touchHandler(true, x, y);
            return true;
        }

        return false;
    }

    public boolean onKeyDown(final int keyCode, KeyEvent event) {
        long currentTime = System.currentTimeMillis();
        
        if (event.getRepeatCount() > 0) {
            return handleKeyRepeat(keyCode);
        } else {
            if ((keyCode == SCROLL_BACK_KEYCODE || keyCode == SCROLL_FWD_KEYCODE) && 
                (currentTime - lastScrollPressTime <= SCROLL_REPEAT_THRESHOLD_MS)){
                lastScrollPressTime = currentTime;

                return buttonHandlerRepeat(keyCode);
            } else {
                if (keyCode == SCROLL_BACK_KEYCODE || keyCode == SCROLL_FWD_KEYCODE){
                    buttonHandler(keyCode, true);
                    new Thread(new Runnable() {
                        public void run() {
                            try {
                                Thread.sleep(50);
                                buttonHandler(keyCode, false);
                            } catch (Exception e) {
                                Log.e("RockboxButton", "Failed to send buttonHandler(keyCode, false): " + e.getMessage());
                                e.printStackTrace();
                            }
                        }
                    }).start();
                    return true;
                }
                return handleKey(keyCode, true);
            }
        }
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        long currentTime = System.currentTimeMillis();
        /* Other keys */

        if ((keyCode == SCROLL_BACK_KEYCODE || keyCode == SCROLL_FWD_KEYCODE) && 
            (currentTime - lastScrollPressTime <= SCROLL_REPEAT_THRESHOLD_MS)){
            lastScrollPressTime = currentTime;
            return true;
        } else {
            if (keyCode == SCROLL_BACK_KEYCODE || keyCode == SCROLL_FWD_KEYCODE) {
                lastScrollPressTime = currentTime;
                return true;
            }
            return handleKey(keyCode, false);
        }
    }

    private int getDpi()
    {
        return metrics.densityDpi;
    }

    private int getScrollThreshold()
    {
        return view_config.getScaledTouchSlop();
    }

    private native void touchHandler(boolean down, int x, int y);
    public native static boolean buttonHandler(int keycode, boolean state);
    public native static boolean buttonHandlerRepeat(int keycode);
    private static native boolean isBackButtonIgnored();
    public native static void triggerVibrationNative(int baseDuration, int boostDuration, boolean immediate);

    public native void surfaceCreated(SurfaceHolder holder);
    
    /* Add native method to force a full redraw from native code */
    public native void forceFullRedraw();
      
    /* Add native method to get virtual framebuffer dimensions */
    public native void getVirtualFramebufferDimensions(int[] dimensions);

    /* Trigger vibration for button feedback */
    public static void triggerVibration(Context context, int baseDuration, int boostDuration, boolean immediate) {
        try {
            Vibrator vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
            if (vibrator != null) {
                int base_ms = duration_mapping[baseDuration];
                int boost_ms = boostDuration;
                int total_ms;
                if (immediate){
                    total_ms = base_ms;
                } else {
                    total_ms = base_ms + boost_ms;
                }
                vibrator.vibrate(total_ms);
            } else {
                android.util.Log.e("RockboxFramebuffer", "Vibrator is null");
            }
        } catch (Exception e) {
            android.util.Log.e("RockboxFramebuffer", "Vibration error: " + e.getMessage());
        }

    }
    private final Runnable watchdogRunnable = new Runnable() {
        @Override
        public void run() {
            long now = System.currentTimeMillis();
            long timeSinceLastUpdate = now - lastUpdateTime;

            if (timeSinceLastUpdate > WATCHDOG_TIMEOUT_MS) {
                Log.e("RockboxFramebuffer", "WATCHDOG: No framebuffer update for " +
                      (timeSinceLastUpdate / 1000) + "s, forcing redraw");
                forceFullRedraw();
            }

            /* Schedule next check */
            watchdogHandler.postDelayed(this, WATCHDOG_TIMEOUT_MS);
        }
    };

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        /* Create bitmap with the appropriate dimensions */
        Log.d("RockboxFramebuffer", "surfaceChanged: w=" + width + " h=" + height);

        /* recycle old btm before creating new one to prevent mem leaks */
        if (btm != null && !btm.isRecycled()) {
            btm.recycle();
        }
        btm = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        
        /* mark surface as ready for drawing */
        synchronized (surfaceLock) {
            surfaceReady = true;
            surfaceEnabled = true;
        }

        /* start watchdog */
        lastUpdateTime = System.currentTimeMillis();
        watchdogHandler.postDelayed(watchdogRunnable, WATCHDOG_TIMEOUT_MS);

        setEnabled(true);
        /* Trigger a full framebuffer redraw from native code */
        forceFullRedraw();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("RockboxFramebuffer", "surfaceDestroyed");

        /* stop watchdog */
        watchdogHandler.removeCallbacks(watchdogRunnable);

        /* mark surface as not ready (prevent drawing while surface is being destroyed) */
        synchronized (surfaceLock) {
            surfaceReady = false;
            surfaceEnabled = false;
        }

        setEnabled(false);
        if (btm != null) {
            btm.recycle();
            btm = null;
        }
    }

}
