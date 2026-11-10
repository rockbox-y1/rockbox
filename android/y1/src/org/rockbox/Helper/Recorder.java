package org.rockbox.Helper;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

public class Recorder {
    private static final String TAG = "RockboxRecorder";

    private static AudioRecord audioRecord;
    private static Thread captureThread;
    private static volatile boolean capturing;

    private static native int nativeRecordData(byte[] data, int offset, int length);

    private static AudioRecord createRecord(int sampleRate) {
        int minBuf = AudioRecord.getMinBufferSize(sampleRate,
                AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
        if (minBuf <= 0)
            minBuf = 4096;
        int bufSize = Math.max(minBuf * 2, 8192);
        try {
            return new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate,
                    AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, bufSize);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "sample rate " + sampleRate + " not supported, using 44100");
            return new AudioRecord(MediaRecorder.AudioSource.MIC, 44100,
                    AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, bufSize);
        }
    }

    public static synchronized void start(final int sampleRate) {
        stop();
        audioRecord = createRecord(sampleRate);
        if (audioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
            Log.e(TAG, "AudioRecord init failed");
            audioRecord.release();
            audioRecord = null;
            return;
        }
        audioRecord.startRecording();
        capturing = true;
        captureThread = new Thread(new Runnable() {
            public void run() {
                byte[] buf = new byte[4096];
                while (capturing) {
                    int n = audioRecord.read(buf, 0, buf.length);
                    if (n <= 0)
                        continue;
                    int off = 0;
                    while (off < n && capturing) {
                        int used = nativeRecordData(buf, off, n - off);
                        if (used > 0)
                            off += used;
                        else
                            try { Thread.sleep(5); } catch (InterruptedException e) { }
                    }
                }
            }
        }, "RockboxRecorder");
        captureThread.start();
        Log.i(TAG, "recording started at " + sampleRate + " Hz");
    }

    public static synchronized void stop() {
        capturing = false;
        if (audioRecord != null) {
            try {
                audioRecord.stop();
            } catch (IllegalStateException e) { }
            audioRecord.release();
            audioRecord = null;
        }
    }
}
