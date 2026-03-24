package org.rockbox.Helper;

import android.view.WindowManager;
import android.app.Activity;
import org.rockbox.RockboxService;
import org.rockbox.Helper.Logger;

/**
 * Simple Android brightness controller for Rockbox
 * Controls brightness from 0% to 100%
 */
public class BrightnessController {
    
    /**
     * Set brightness to a specific percentage
     * @param percent Brightness percentage (0-100)
     * @return 0 on success, -1 on error
     */
    public int setBrightnessPercent(int percent) {
        if (percent < 0 || percent > 100) {
            return -1; // Invalid input
        }

        float brightness = percent / 100.0f;

        try {
            RockboxService service = RockboxService.getInstance();
            if (service != null) {
                final Activity activity = service.getActivity();
                if (activity != null) {
                    final float targetBrightness = brightness;
                    activity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            WindowManager.LayoutParams lp = activity.getWindow().getAttributes();
                            lp.screenBrightness = targetBrightness;
                            activity.getWindow().setAttributes(lp);
                        }
                    });
                    return 0; // Success
                }
            }
        } catch (Exception e) {
            return -1;
        }
        
        return -1; // Failed to get service or activity
    }
    
    /**
     * Get current brightness as percentage
     * @return Current brightness percentage (0-100)
     */
    public int getBrightnessPercent() {
        try {
            RockboxService service = RockboxService.getInstance();
            if (service != null) {
                Activity activity = service.getActivity();
                if (activity != null) {
                    WindowManager.LayoutParams lp = activity.getWindow().getAttributes();
                    float brightness = lp.screenBrightness;
                    if (brightness < 0) {
                        // If window brightness is not set, default to 100%
                        return 100;
                    }
                    return (int) (brightness * 100);
                }
            }
        } catch (Exception e) {
            Logger.d("Failed to get brightness: " + e.getMessage());
        }
        
        // Fallback to 100%
        return 100;
    }
} 