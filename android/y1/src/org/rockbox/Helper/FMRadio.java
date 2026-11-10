package org.rockbox.Helper;

import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.provider.Settings;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

/* The FM receiver's audio is analog and never passes through Rockbox's PCM
 * path, so the audio path is opened on the MediaTek audio HAL with a
 * MediaPlayer of playertype FM on the (hidden) STREAM_FM stream, the same
 * mechanism the stock MTK FM app uses. Volume follows Rockbox's volume
 * setting while the radio is on. */

public class FMRadio {
    private static Context context = null;
    private static AudioManager audioManager = null;

    private static MediaPlayer fmPlayer = null;
    private static boolean fmAudioEnabled = false;

    private static int lastRockboxVolume = 0;

    /* AudioManager.STREAM_FM is hidden in the SDK, resolved via reflection */
    private static int streamFm = 10;
    private static final int FORCE_NONE = 0;
    private static int forFm = 5;
    private static Class<?> audioSystemClass = null;
    private static Method setForceUseMethod = null;
    private static Method setStreamVolumeIndexMethod = null;
    private static boolean constantsResolved = false;

    /* MTK MediaPlayer data source that routes the analog input to the output */
    private static final String FM_PLAYER_SOURCE = "MEDIATEK://MEDIAPLAYER_PLAYERTYPE_FM";

    private static final AudioManager.OnAudioFocusChangeListener focusListener =
            new AudioManager.OnAudioFocusChangeListener() {
                public void onAudioFocusChange(int focusChange) { }
            };

    public static void setContext(Context ctx) {
        context = ctx;
        audioManager = (AudioManager) ctx.getSystemService(Context.AUDIO_SERVICE);
        resolveConstants();
    }

    private static void resolveConstants() {
        if (constantsResolved)
            return;
        constantsResolved = true;
        try {
            Field f = AudioManager.class.getField("STREAM_FM");
            streamFm = f.getInt(null);
        } catch (Exception e) {
            Logger.d("FMRadio: no STREAM_FM field, using fallback " + streamFm);
        }
        try {
            audioSystemClass = Class.forName("android.media.AudioSystem");
            Field f = audioSystemClass.getField("FOR_FM");
            forFm = f.getInt(null);
        } catch (Exception e) {
            Logger.d("FMRadio: no AudioSystem.FOR_FM field, using fallback " + forFm);
        }
        if (audioSystemClass != null) {
            try {
                setForceUseMethod = audioSystemClass.getMethod("setForceUse",
                        int.class, int.class);
            } catch (Exception e) {
                Logger.e("FMRadio: no AudioSystem.setForceUse: " + e);
            }
            try {
                setStreamVolumeIndexMethod = audioSystemClass.getMethod("setStreamVolumeIndex",
                        int.class, int.class, int.class);
            } catch (Exception e) {
                /* just in case  */
            }
        }
    }

    /* tell the MTK audio policy that FM is in use and how to route it
       otherwise the output screams in ~1kHz for some reason */
    private static void setFmForceUse(int config) {
        if (setForceUseMethod == null)
            return;
        try {
            setForceUseMethod.invoke(null, forFm, config);
        } catch (Exception e) {
            Logger.e("FMRadio: setForceUse failed: " + e);
        }
    }

    public static synchronized void enableFmAudio(boolean enable) {
        if (audioManager == null) {
            Logger.d("FMRadio: no audio manager, cannot " + (enable ? "enable" : "disable") + " FM audio");
            return;
        }
        if (enable == fmAudioEnabled)
            return;
        fmAudioEnabled = enable;
        if (enable) {
            try {
                setFmForceUse(FORCE_NONE);
                audioManager.requestAudioFocus(focusListener, streamFm,
                        AudioManager.AUDIOFOCUS_GAIN);
                fmPlayer = new MediaPlayer();
                fmPlayer.setDataSource(FM_PLAYER_SOURCE);
                fmPlayer.setAudioStreamType(streamFm);
                fmPlayer.setLooping(true);
                fmPlayer.prepare();
                fmPlayer.start();
                applyVolume();
                Logger.d("FMRadio: FM audio path enabled on STREAM_FM=" + streamFm
                        + " FOR_FM=" + forFm);
            } catch (Exception e) {
                Logger.e("FMRadio: failed to enable FM audio: " + e);
                releasePlayer();
            }
        } else {
            releasePlayer();
            audioManager.abandonAudioFocus(focusListener);
            Logger.d("FMRadio: FM audio path disabled");
        }
    }

    public static boolean isWiredHeadsetOn() {
        return audioManager != null && audioManager.isWiredHeadsetOn();
    }

    public static synchronized void setFmVolume(int rockboxVolume) {
        lastRockboxVolume = rockboxVolume;
        applyVolume();
    }

    public static synchronized boolean isFmAudioEnabled() {
        return fmAudioEnabled;
    }

    public static synchronized void shutdown() {
        if (fmAudioEnabled)
            enableFmAudio(false);
    }

    public static boolean isAirplaneModeOn() {
        if (context == null)
            return false;
        try {
            return Settings.Global.getInt(context.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        } catch (Throwable t) {
            /* just in case */
            try {
                return Settings.System.getInt(context.getContentResolver(),
                        Settings.System.AIRPLANE_MODE_ON, 0) != 0;
            } catch (Throwable t2) {
                return false;
            }
        }
    }

    public static void setAirplaneMode(boolean on) {
        if (context == null)
            return;
        int value = on ? 1 : 0;
        try {
            Settings.Global.putInt(context.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, value);
        } catch (Throwable t) {
            try {
                Settings.System.putInt(context.getContentResolver(),
                        Settings.System.AIRPLANE_MODE_ON, value);
            } catch (Throwable t2) {
                Logger.e("FMRadio: failed to set airplane mode: " + t2);
                return;
            }
        }
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", on);
        context.sendBroadcast(intent);
        Logger.d("FMRadio: airplane mode set to " + on);
    }

    private static void applyVolume() {
        if (!fmAudioEnabled || audioManager == null)
            return;
        int max = audioManager.getStreamMaxVolume(streamFm);
        if (max <= 0)
            return;
        float fraction = 1 - (lastRockboxVolume / -990.0f);
        int index = (int) Math.ceil(max * fraction);
        if (index < 0)
            index = 0;
        if (index > max)
            index = max;
        audioManager.setStreamVolume(streamFm, index, 0);
        /* mtk fm radio specific magic for setting volume */
        if (setStreamVolumeIndexMethod != null) {
            try {
                setStreamVolumeIndexMethod.invoke(null, 3, index, 0x80);
                setStreamVolumeIndexMethod.invoke(null, 9, index, 0x80);
            } catch (Exception e) {
                /* just in case */
            }
        }
    }

    private static void releasePlayer() {
        if (fmPlayer != null) {
            try {
                fmPlayer.stop();
            } catch (Exception e) {
                /* not started or already stopped */
            }
            fmPlayer.release();
            fmPlayer = null;
        }
    }
}
