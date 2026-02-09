package com.anonymousmessage.android.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import org.torproject.android.binary.TorServiceConstants;

import java.io.File;

/**
 * Service that manages the TOR daemon for anonymous communication
 * This service handles starting, stopping, and managing the TOR process
 */
public class TorService extends Service {
    private static final String TAG = "TorService";
    
    private final IBinder binder = new TorBinder();
    
    // TOR configuration constants
    private static final String TOR_DATA_DIR = "tor_data";
    private static final String TOR_BINARY_NAME = "tor";
    
    private boolean isTorRunning = false;
    private Process torProcess;
    
    public class TorBinder extends Binder {
        public TorService getService() {
            return TorService.this;
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "TorService bound");
        return binder;
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "TorService created");
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Starting TorService");
        
        // Start TOR in a separate thread to avoid blocking main thread
        new Thread(this::startTor).start();
        
        return START_STICKY; // Restart service if killed
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        stopTor();
        Log.d(TAG, "TorService destroyed");
    }
    
    /**
     * Starts the TOR daemon
     */
    private void startTor() {
        if (isTorRunning) {
            Log.d(TAG, "TOR is already running");
            return;
        }
        
        try {
            Log.d(TAG, "Attempting to start TOR...");
            
            // Create data directory for TOR
            File dataDir = new File(getFilesDir(), TOR_DATA_DIR);
            if (!dataDir.exists()) {
                dataDir.mkdirs();
            }
            
            // Build TOR command
            String torBinaryPath = getApplicationInfo().nativeLibraryDir + "/" + TOR_BINARY_NAME;
            String[] torCommand = {
                torBinaryPath,
                "--DataDirectory", dataDir.getAbsolutePath(),
                "--GeoIPFile", getFilesDir() + "/geoip",
                "--GeoIPv6File", getFilesDir() + "/geoip6",
                "--SocksPort", "9050",
                "--ControlPort", "9051",
                "--Log", "notice stdout"
            };
            
            // Start TOR process
            torProcess = new ProcessBuilder(torCommand).start();
            
            isTorRunning = true;
            Log.d(TAG, "TOR started successfully");
            
            // Monitor TOR process
            monitorTorProcess();
            
        } catch (Exception e) {
            Log.e(TAG, "Error starting TOR: " + e.getMessage(), e);
            isTorRunning = false;
        }
    }
    
    /**
     * Monitors the TOR process and restarts if needed
     */
    private void monitorTorProcess() {
        new Thread(() -> {
            try {
                if (torProcess != null) {
                    int exitCode = torProcess.waitFor();
                    Log.w(TAG, "TOR process exited with code: " + exitCode);
                    
                    // Attempt to restart TOR if it stopped unexpectedly
                    if (isTorRunning) {
                        Log.i(TAG, "Restarting TOR...");
                        startTor();
                    }
                }
            } catch (InterruptedException e) {
                Log.i(TAG, "TOR monitor thread interrupted");
            }
        }).start();
    }
    
    /**
     * Stops the TOR daemon
     */
    private void stopTor() {
        Log.d(TAG, "Stopping TOR...");
        
        isTorRunning = false;
        
        if (torProcess != null) {
            torProcess.destroy();
            try {
                torProcess.waitFor(); // Wait for process to finish
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted while waiting for TOR process to finish", e);
            }
            torProcess = null;
        }
        
        Log.d(TAG, "TOR stopped");
    }
    
    /**
     * Checks if TOR is currently running
     * @return true if TOR is running, false otherwise
     */
    public boolean isTorRunning() {
        return isTorRunning;
    }
    
    /**
     * Gets the SOCKS proxy port used by TOR
     * @return the SOCKS proxy port number
     */
    public int getSocksProxyPort() {
        return TorServiceConstants.SOCKS_PROXY_PORT_DEFAULT;
    }
    
    /**
     * Gets the control port used by TOR
     * @return the control port number
     */
    public int getControlPort() {
        return TorServiceConstants.CONTROL_SOCKET_PORT_DEFAULT;
    }
}