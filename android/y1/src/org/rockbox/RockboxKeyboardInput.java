/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 Jonathan Gordon
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

import org.rockbox.Helper.MediaButtonReceiver;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.text.Editable;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.view.KeyEvent;
import android.util.Log;

public class RockboxKeyboardInput
{
    public void kbd_input(final String text, final String ok, final String cancel)
    {
        final Activity c = RockboxService.getInstance().getActivity();
        MediaButtonReceiver.setDpadMode(true);

        c.runOnUiThread(new Runnable() {
            public void run()
            {
                LayoutInflater inflater = LayoutInflater.from(c);
                View addView = inflater.inflate(R.layout.keyboardinput, null);
                EditText input = (EditText) addView.findViewById(R.id.KbdInput);
                input.setText(text);
                AlertDialog.Builder builder = new AlertDialog.Builder(c)
                    .setTitle(R.string.KbdInputTitle)
                    .setView(addView)
                    .setIcon(R.drawable.icon)
                    .setCancelable(false)
                    .setPositiveButton(ok, new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            EditText input = (EditText)((Dialog)dialog)
                                                    .findViewById(R.id.KbdInput);
                            Editable s = input.getText();
                            put_result(true, s.toString());
                            MediaButtonReceiver.setDpadMode(false);
                        }
                    })
                    .setNegativeButton(cancel, new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            put_result(false, "");
                            MediaButtonReceiver.setDpadMode(false);
                        }
                    });
                final AlertDialog dialog = builder.create();
                dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialogInterface) {
                        MediaButtonReceiver.setDpadMode(false);
                    }
                });
                
                dialog.show();

                new Thread(new Runnable() {
                    public void run() {
                        try {
                            Log.d("RockboxKeyboard", "Attempting to launch keyboard...");
                            java.lang.Process proc = Runtime.getRuntime().exec(new String[]{"su", "-c", "input trackball press"});
                            proc.waitFor();
                        } catch (Exception e) {
                            Log.e("RockboxKeyboard", "Failed Launching Keyboard: " + e.getMessage());
                            e.printStackTrace();
                        }
                    }
                }).start();

            }
        });
    }

    private native void put_result(boolean accepted, String new_string);

    public boolean is_usable()
    {
        return RockboxService.getInstance().getActivity() != null;
    }
}
