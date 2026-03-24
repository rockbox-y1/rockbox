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

package org.rockbox.Helper;

import org.rockbox.RockboxFramebuffer;
import org.rockbox.RockboxService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.view.KeyEvent;
import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.input.InputManager;
import android.os.SystemClock;
import android.view.InputEvent;
import java.lang.reflect.Method;

import android.util.Log;

public class MediaButtonReceiver
{
    /* A note on the API being used. 2.2 introduces a new and sane API
     * for handling multimedia button presses
     * http://android-developers.blogspot.com/2010/06/allowing-applications-to-play-nicer.html
     * 
     * the old API is flawed. It doesn't have management for
     * concurrent media apps
     * 
     * if multiple media apps are running 
     * probably all of them want to respond to media keys
     * 
     * it's not clear which app wins, it depends on the
     * priority set for the IntentFilter (see below)
     * 
     * so this all might or might not work on < 2.2 */

    IMultiMediaReceiver api;

    private static Context mContext;
    public static SharedPreferences prefs;
    public MediaButtonReceiver(Context c)
    {
        mContext = c.getApplicationContext();
        prefs = mContext.getSharedPreferences("app_state", Context.MODE_PRIVATE);

        try {
            api = new NewApi(c);
        } catch (Throwable t) {
            /* Throwable includes Exception and the expected
             * NoClassDefFoundError */
            api = new OldApi(c);
            Logger.i("MediaButtonReceiver: Falling back to compatibility API");
        }
    }

    public static void setDpadMode(boolean enabled) {
        prefs.edit().putBoolean("dpad_mode", enabled).apply();
        Log.d("MediaButtonReceiver", "dpad_mode set to: " + enabled);
    }

    public void register()
    {
        api.register();
    }
    
    public void unregister()
    {
        api.unregister();
    }

    /* helper class for the manifest */
    public static class MediaReceiver extends BroadcastReceiver
    {
        private void startService(Context c, Intent baseIntent)
        {
            baseIntent.setClass(c, RockboxService.class);
            c.startService(baseIntent);
        }

        private static int mediaToDpad(int keyCode) {
            switch (keyCode) {
                case KeyEvent.KEYCODE_MEDIA_NEXT: return KeyEvent.KEYCODE_DPAD_RIGHT;
                case KeyEvent.KEYCODE_MEDIA_PREVIOUS: return KeyEvent.KEYCODE_DPAD_LEFT;
                case KeyEvent.KEYCODE_MEDIA_PLAY: return KeyEvent.KEYCODE_DPAD_UP;
                case KeyEvent.KEYCODE_MEDIA_PAUSE: return KeyEvent.KEYCODE_DPAD_DOWN;
                case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE: return KeyEvent.KEYCODE_DPAD_CENTER;
                default: return KeyEvent.KEYCODE_DPAD_RIGHT;
            }
        }

        private static void injectKeyEvent(Context context, KeyEvent event) {
            try {
                InputManager im = (InputManager) context.getSystemService(Context.INPUT_SERVICE);
                Method injectInputEvent = InputManager.class.getMethod(
                        "injectInputEvent", InputEvent.class, int.class);
                final int INJECT_INPUT_EVENT_MODE_ASYNC = 0;
                injectInputEvent.invoke(im, event, INJECT_INPUT_EVENT_MODE_ASYNC);
            } catch (Exception e) {
                Log.e("MediaButtonReceiver", "Failed to inject key event", e);
            }
        }

        @Override
        public void onReceive(Context context, Intent intent)
        {
            boolean dpad_mode = prefs.getBoolean("dpad_mode", false);
            if (dpad_mode){
                if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction())) {
                    KeyEvent key = (KeyEvent) intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                    if (key == null) return;

                    int action = key.getAction();
                    int mediaCode = key.getKeyCode();
                    int dpadCode = mediaToDpad(mediaCode);

                    long now = SystemClock.uptimeMillis();

                    if (action == KeyEvent.ACTION_UP) {
                        KeyEvent down = new KeyEvent(now, now, KeyEvent.ACTION_DOWN, dpadCode, 0);
                        KeyEvent up   = new KeyEvent(now+20, now+20, KeyEvent.ACTION_UP,   dpadCode, 0);

                        Log.d("MediaButtonReceiver", "simulating dpad: " + dpadCode);
                        injectKeyEvent(context, down);
                        injectKeyEvent(context, up);

                        // Prevent other media apps from also handling this
                        abortBroadcast();
                    }
                }

            } else{
                if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction()))
                {
                    KeyEvent key = (KeyEvent)intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                    if (key.getAction() == KeyEvent.ACTION_UP)
                    {   /* pass the pressed key to Rockbox, starting it if needed */
                        RockboxService s = RockboxService.getInstance();
                        if (s == null || !s.isRockboxRunning())
                            startService(context, intent);
                        else if (RockboxFramebuffer.buttonHandler(key.getKeyCode(), false))
                            abortBroadcast();
                    }
                    else if (key.getAction() == KeyEvent.ACTION_DOWN)
                    {   
                        if (key.getRepeatCount() > 0){
                            /* handle repeat events */
                            RockboxService s = RockboxService.getInstance();
                            if (s != null && s.isRockboxRunning())
                            {
                                if (RockboxFramebuffer.buttonHandlerRepeat(key.getKeyCode()))
                                    abortBroadcast();
                            }
                        } else if (RockboxFramebuffer.buttonHandler(key.getKeyCode(), true)) {
                            abortBroadcast();
                        }
                    }
                }
            }
        }
    }
    
    private interface IMultiMediaReceiver 
    {
        void register();
        void unregister();
    }

    private static class NewApi 
                    implements IMultiMediaReceiver, AudioManager.OnAudioFocusChangeListener
    {
        private AudioManager audio_manager;
        private ComponentName receiver_name;
        private boolean running = false;
        
        NewApi(Context c)
        {
            audio_manager = (AudioManager)c.getSystemService(Context.AUDIO_SERVICE);
            receiver_name = new ComponentName(c, MediaReceiver.class);
        }
        
        public void register()
        {
            try {
                audio_manager.registerMediaButtonEventReceiver(receiver_name);
                audio_manager.requestAudioFocus(this, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
                running = true;
            } catch (Exception e) {
                // Nothing
                e.printStackTrace();
            }
        }

        public void unregister()
        {
            try
            {
                audio_manager.unregisterMediaButtonEventReceiver(receiver_name);
                audio_manager.abandonAudioFocus(this);
                running = false;
            } catch (Exception e) {
                // Nothing
                e.printStackTrace();
            }
        }

        public void onAudioFocusChange(int focusChange)
        {
            Logger.d("Audio focus" + ((focusChange>0)?"gained":"lost")+
                                         ": "+ focusChange);
            if (running)
            {   /* Play nice and stop for the the other app */
                if (focusChange == AudioManager.AUDIOFOCUS_LOSS)
                    RockboxFramebuffer.buttonHandler(KeyEvent.KEYCODE_MEDIA_STOP, false);
            }
        }
        
    }

    private static class OldApi implements IMultiMediaReceiver
    {
        private static final IntentFilter filter = new IntentFilter(Intent.ACTION_MEDIA_BUTTON);
        private MediaReceiver receiver;
        private Context context;
        OldApi(Context c)
        {
            filter.setPriority(1); /* 1 higher than the built-in media player */
            receiver = new MediaReceiver();
            context = c;
        }
        
        public void register()
        {
            context.registerReceiver(receiver, filter);            
        }

        public void unregister()
        {
            context.unregisterReceiver(receiver);
        }
        
    }
}
