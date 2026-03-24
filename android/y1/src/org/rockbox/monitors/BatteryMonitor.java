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

import java.util.Timer;
import java.util.TimerTask;
import java.util.LinkedList;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.PowerManager;
import android.util.Log;

public class BatteryMonitor extends BroadcastReceiver
{
    private final IntentFilter mBattFilter;
    private final Context mContext;
    @SuppressWarnings("unused")
    /* read by native code */
    private int mBattLevel;
    private int mPlugged;
    private int mCharging;
    public int mFullMinutes = 1200; /* let's guess we have 20h active battery life */
    public int mEstimatedMinutes = -1;
    private Timer mTimer;
    private TimerTask mTask;
    private PowerManager pm;
    
    void startTimer() {
        /*
        * We get literally spammed with battery status updates
        * Therefore we actually unregister after each onReceive() and
        * setup a timer to re-register in 2s */
        stopTimer();
        mTimer = new Timer();
        mTask = new TimerTask() {
            public void run() {
                if (pm.isScreenOn()){
                    attach();
                }
            }
        };
        mTimer.schedule(mTask, 5000, 2000);
    }

    void stopTimer() {
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
        if (mTask != null) {
            mTask.cancel();
            mTask = null;
        }
    }

    public BatteryMonitor(Context c)
    {
        pm = (PowerManager) c.getSystemService(Context.POWER_SERVICE);
        mBattFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        mContext = c;

        startTimer();
    }

    @Override
    public void onReceive(Context arg0, Intent intent)
    {
        int rawlevel = intent.getIntExtra("level", -1);
        int scale = intent.getIntExtra("scale", -1);
        if (rawlevel >= 0 && scale > 0)
            mBattLevel = (rawlevel * 100) / scale;
        else
            mBattLevel = -1;

        // Charging status
        int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING;
        if (isCharging) {
            mCharging = 1;
        } else {
            mCharging = 0;
        }

        // Plugged type
        int chargePlug = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        boolean usbCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_USB;
        boolean acCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_AC;
        if (usbCharge){
           mPlugged = 1;
        } else if (acCharge){
           mPlugged = 2;
        } else {
           mPlugged = 0;
        }

        /* Battery life estimation */
        double drainRate = mFullMinutes / 100; 
        mEstimatedMinutes = (int) Math.round(mBattLevel * drainRate); 

        mContext.unregisterReceiver(this);
    }
    
    void attach()
    {
        mContext.registerReceiver(this, mBattFilter);
    }
}
