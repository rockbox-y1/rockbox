package org.rockbox.Helper;

import android.app.Activity;
import android.provider.Settings;
import android.content.ContentResolver;
import org.rockbox.RockboxService;
import org.rockbox.Helper.Logger;

/**
 * Android screen timeout controller for Rockbox
 * Controls the system screen timeout setting
 */
public class ScreenTimeoutController {
    
    /**
     * Set screen timeout to a specific value in seconds
     * @param timeoutSeconds Screen timeout in seconds (0=never, -1=system default)
     * @return 0 on success, -1 on error
     */
    public int setScreenTimeout(int timeoutSeconds) {
        try {
            RockboxService service = RockboxService.getInstance();
            if (service != null) {
                final Activity activity = service.getActivity();
                if (activity != null) {
                    final int targetTimeout = timeoutSeconds;
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            ContentResolver resolver = activity.getContentResolver();
                            if (targetTimeout == 0) {
                                // Never timeout
                                Settings.System.putInt(resolver, Settings.System.SCREEN_OFF_TIMEOUT, Integer.MAX_VALUE);
                            } else if (targetTimeout == -1) {
                                // Use system default (30 seconds)
                                Settings.System.putInt(resolver, Settings.System.SCREEN_OFF_TIMEOUT, 30000);
                            } else {
                                // Set specific timeout in milliseconds
                                Settings.System.putInt(resolver, Settings.System.SCREEN_OFF_TIMEOUT, targetTimeout * 1000);
                            }
                        }
                    });
                    return 0; // Success
                }
            }
        } catch (Exception e) {
            Logger.d("Failed to set screen timeout: " + e.getMessage());
            return -1;
        }
        
        return -1; // Failed to get service or activity
    }
    
    /**
     * Get current screen timeout in seconds
     * @return Current screen timeout in seconds (0=never, -1=system default)
     */
    public int getScreenTimeout() {
        try {
            RockboxService service = RockboxService.getInstance();
            if (service != null) {
                Activity activity = service.getActivity();
                if (activity != null) {
                    ContentResolver resolver = activity.getContentResolver();
                    int timeoutMs = Settings.System.getInt(resolver, Settings.System.SCREEN_OFF_TIMEOUT, 30000);
                    
                    if (timeoutMs == Integer.MAX_VALUE) {
                        return 0; // Never timeout
                    } else if (timeoutMs == 30000) {
                        return -1; // System default
                    } else {
                        return timeoutMs / 1000; // Convert to seconds
                    }
                }
            }
        } catch (Exception e) {
            Logger.d("Failed to get screen timeout: " + e.getMessage());
        }
        
        // Fallback to system default
        return -1;
    }
} 