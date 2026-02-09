package com.anonymousmessage.android.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import com.anonymousmessage.android.service.TorService;

/**
 * Receiver that monitors network changes and manages TOR connections accordingly
 */
public class NetworkChangeReceiver extends BroadcastReceiver {
    private static final String TAG = "NetworkChangeReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        
        if (ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
            boolean isConnected = isNetworkAvailable(context);
            
            Log.d(TAG, "Network connectivity changed. Connected: " + isConnected);
            
            // Handle network changes for TOR service
            handleNetworkChange(context, isConnected);
        }
    }

    /**
     * Checks if network is available
     * @param context Application context
     * @return true if network is available, false otherwise
     */
    private boolean isNetworkAvailable(Context context) {
        ConnectivityManager cm = (ConnectivityManager) 
            context.getSystemService(Context.CONNECTIVITY_SERVICE);
        
        if (cm != null) {
            NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
            return activeNetwork != null && activeNetwork.isConnectedOrConnecting();
        }
        
        return false;
    }

    /**
     * Handles network connectivity changes for TOR service
     * @param context Application context
     * @param isConnected Whether network is connected
     */
    private void handleNetworkChange(Context context, boolean isConnected) {
        if (isConnected) {
            // Network is available, ensure TOR service is running
            Intent torIntent = new Intent(context, TorService.class);
            context.startService(torIntent);
            
            Log.d(TAG, "Started TOR service due to network availability");
        } else {
            // Network is not available, TOR won't work anyway
            Log.d(TAG, "Network unavailable, TOR service will pause until connection restored");
        }
    }
}