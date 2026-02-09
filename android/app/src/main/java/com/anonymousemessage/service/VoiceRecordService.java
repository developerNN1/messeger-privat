package com.anonymousemessage.service;

import android.app.Service;
import android.content.Intent;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class VoiceRecordService extends Service {
    
    private static final String TAG = "VoiceRecordService";
    private MediaRecorder mediaRecorder;
    private String recordedFilePath;
    private boolean isRecording = false;
    
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();
        
        if (action != null) {
            switch (action) {
                case "START_RECORDING":
                    startRecording();
                    break;
                case "STOP_RECORDING":
                    stopRecording();
                    break;
                case "CANCEL_RECORDING":
                    cancelRecording();
                    break;
            }
        }
        
        return START_STICKY;
    }
    
    private void startRecording() {
        if (isRecording) {
            Log.w(TAG, "Already recording, ignoring start request");
            return;
        }
        
        try {
            mediaRecorder = new MediaRecorder();
            mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
            
            // Create file for recording
            File recordDir = new File(getExternalFilesDir(Environment.DIRECTORY_MUSIC), "voice_messages");
            if (!recordDir.exists()) {
                recordDir.mkdirs();
            }
            
            SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault());
            String fileName = "voice_" + sdf.format(new Date()) + ".m4a";
            recordedFilePath = new File(recordDir, fileName).getAbsolutePath();
            
            mediaRecorder.setOutputFile(recordedFilePath);
            mediaRecorder.prepare();
            mediaRecorder.start();
            
            isRecording = true;
            Log.d(TAG, "Started recording: " + recordedFilePath);
        } catch (IOException e) {
            Log.e(TAG, "Failed to start recording", e);
        } catch (IllegalStateException e) {
            Log.e(TAG, "MediaRecorder error", e);
        }
    }
    
    private void stopRecording() {
        if (!isRecording) {
            Log.w(TAG, "Not recording, ignoring stop request");
            return;
        }
        
        try {
            if (mediaRecorder != null) {
                mediaRecorder.stop();
                mediaRecorder.release();
                mediaRecorder = null;
                
                // Notify that recording is complete
                Log.d(TAG, "Stopped recording: " + recordedFilePath);
                
                isRecording = false;
                
                // In a real implementation, we would send this file to the intended recipient
                // through the Tor network
                
                // For now, just log that we have a recording ready to send
                Log.d(TAG, "Voice message ready to send: " + recordedFilePath);
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "Error stopping recorder", e);
        }
    }
    
    private void cancelRecording() {
        if (!isRecording) {
            Log.w(TAG, "Not recording, ignoring cancel request");
            return;
        }
        
        try {
            if (mediaRecorder != null) {
                mediaRecorder.stop();
                mediaRecorder.release();
                mediaRecorder = null;
                
                // Delete the recorded file since it was cancelled
                if (recordedFilePath != null) {
                    File file = new File(recordedFilePath);
                    if (file.exists()) {
                        file.delete();
                        Log.d(TAG, "Cancelled and deleted recording: " + recordedFilePath);
                    }
                }
                
                isRecording = false;
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "Error cancelling recorder", e);
        }
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        
        if (isRecording && mediaRecorder != null) {
            try {
                mediaRecorder.stop();
                mediaRecorder.release();
            } catch (RuntimeException e) {
                Log.e(TAG, "Error stopping recorder in onDestroy", e);
            }
        }
    }
}