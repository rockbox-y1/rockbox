/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2011 Thomas Martitz
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

package org.rockbox.monitors;

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.util.Log;

public class HeadphoneMonitor extends BroadcastReceiver
{
    private static int hp_state = -1;
    private static final String TAG = "Rockbox.HeadphoneMonitor";

    public HeadphoneMonitor(Context c)
    {
        if (isBluetoothA2dpConnected())
        {
            postHpStateChanged(1);
        }

        AudioManager audioManager =
            (AudioManager) c.getSystemService(Context.AUDIO_SERVICE);
        if (audioManager != null && audioManager.isWiredHeadsetOn())
        {
            postHpStateChanged(1);
        }

        IntentFilter hpFilter = new IntentFilter(Intent.ACTION_HEADSET_PLUG);
        IntentFilter btFilter = new IntentFilter(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);

        c.registerReceiver(this, hpFilter);
        c.registerReceiver(new BtStateMonitor(), btFilter);
    }

    @Override
    public void onReceive(Context arg0, Intent intent)
    {
        int state = intent.getIntExtra("state", -1);
        /* hp_state is sometimes -1 here even if BT was connected on startup, catch this */
        if (hp_state == -1 && isBluetoothA2dpConnected())
            hp_state = 1;
        /* ignore unplug events when we know BT is connected */
        if (state == 0 && hp_state == 1)
            return;

        postHpStateChanged(state);
    }

    /*
     * Tracks BT connection state:
     * - on connect signal plugged
     * - on disconnect signal unplugged
     */
    private class BtStateMonitor extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context arg0, Intent arg1)
        {
            int state = arg1.getIntExtra(
                "android.bluetooth.profile.extra.STATE", BluetoothProfile.STATE_DISCONNECTED);

            if (state == BluetoothProfile.STATE_CONNECTED)
                postHpStateChanged(1);
            else if (state == BluetoothProfile.STATE_DISCONNECTED)
                postHpStateChanged(0);
        }
    }

    private synchronized native void postHpStateChanged(int state);

    private boolean isBluetoothA2dpConnected()
    {
        try
        {
            Process process = Runtime.getRuntime().exec("dumpsys bluetooth_a2dp");
            java.io.BufferedReader reader =
                new java.io.BufferedReader(
                    new java.io.InputStreamReader(process.getInputStream()));
            String line;
            boolean connected = false;
            while ((line = reader.readLine()) != null)
            {
                if (line.contains("connected"))
                {
                    connected = true;
                    break;
                }
            }
            reader.close();
            process.waitFor();
            Log.d(TAG, "isBluetoothA2dpConnected (dumpsys): " + connected);
            return connected;
        }
        catch (Exception e)
        {
            Log.e(TAG, "isBluetoothA2dpConnected (dumpsys) failed", e);
            return false;
        }
    }
}
