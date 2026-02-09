package com.anonymousemessage.service;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Base64;
import android.util.Log;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;

public class SessionManager {
    private static final String PREF_NAME = "AnonymousMessage_Session";
    private static final String KEY_USERNAME = "username";
    private static final String KEY_EMAIL = "email";
    private static final String KEY_PRIVATE_KEY = "private_key";
    private static final String ENCRYPTION_KEY = "session_encryption_key";
    
    private static SessionManager instance;
    private SharedPreferences sharedPreferences;
    private Context context;
    
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 16;
    
    private SessionManager(Context context) {
        this.context = context.getApplicationContext();
        this.sharedPreferences = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
    }
    
    public static synchronized SessionManager getInstance() {
        if (instance == null) {
            throw new IllegalStateException("SessionManager not initialized. Call initialize(Context) first.");
        }
        return instance;
    }
    
    public static synchronized void initialize(Context context) {
        if (instance == null) {
            instance = new SessionManager(context);
        }
    }
    
    public void saveCredentials(String username, String email, String privateKey) {
        try {
            // Encrypt the private key before storing
            String encryptedPrivateKey = encrypt(privateKey);
            
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString(KEY_USERNAME, username);
            editor.putString(KEY_EMAIL, email);
            editor.putString(KEY_PRIVATE_KEY, encryptedPrivateKey);
            editor.apply();
        } catch (Exception e) {
            Log.e("SessionManager", "Error saving credentials", e);
        }
    }
    
    public String getUsername() {
        return sharedPreferences.getString(KEY_USERNAME, null);
    }
    
    public String getEmail() {
        return sharedPreferences.getString(KEY_EMAIL, null);
    }
    
    public String getPrivateKey() {
        String encryptedPrivateKey = sharedPreferences.getString(KEY_PRIVATE_KEY, null);
        if (encryptedPrivateKey != null) {
            try {
                return decrypt(encryptedPrivateKey);
            } catch (Exception e) {
                Log.e("SessionManager", "Error decrypting private key", e);
                return null;
            }
        }
        return null;
    }
    
    public boolean isLoggedIn() {
        return getUsername() != null && getPrivateKey() != null;
    }
    
    public void logout() {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.clear();
        editor.apply();
    }
    
    private String encrypt(String data) throws Exception {
        // Retrieve or generate a master key for session encryption
        String masterKeyStr = sharedPreferences.getString(ENCRYPTION_KEY, null);
        SecretKey masterKey;
        
        if (masterKeyStr == null) {
            // Generate a new master key
            KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
            keyGen.init(256);
            masterKey = keyGen.generateKey();
            
            // Store the encoded key
            String encodedKey = Base64.encodeToString(masterKey.getEncoded(), Base64.DEFAULT);
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString(ENCRYPTION_KEY, encodedKey);
            editor.apply();
        } else {
            // Decode existing master key
            byte[] decodedKey = Base64.decode(masterKeyStr, Base64.DEFAULT);
            masterKey = new SecretKeySpec(decodedKey, ALGORITHM);
        }
        
        // Generate a random IV
        byte[] iv = new byte[GCM_IV_LENGTH];
        new SecureRandom().nextBytes(iv);
        
        // Initialize cipher
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, masterKey, gcmSpec);
        
        // Encrypt data
        byte[] encryptedData = cipher.doFinal(data.getBytes("UTF-8"));
        
        // Combine IV + encrypted data for storage
        byte[] result = new byte[iv.length + encryptedData.length];
        System.arraycopy(iv, 0, result, 0, iv.length);
        System.arraycopy(encryptedData, 0, result, iv.length, encryptedData.length);
        
        return Base64.encodeToString(result, Base64.NO_WRAP);
    }
    
    private String decrypt(String encryptedDataStr) throws Exception {
        // Retrieve the master key
        String masterKeyStr = sharedPreferences.getString(ENCRYPTION_KEY, null);
        if (masterKeyStr == null) {
            throw new IllegalStateException("Master key not found");
        }
        
        byte[] decodedMasterKey = Base64.decode(masterKeyStr, Base64.DEFAULT);
        SecretKey masterKey = new SecretKeySpec(decodedMasterKey, ALGORITHM);
        
        // Decode the encrypted data
        byte[] encryptedData = Base64.decode(encryptedDataStr, Base64.NO_WRAP);
        
        // Extract IV from the beginning
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedData, 0, iv, 0, GCM_IV_LENGTH);
        
        // Extract encrypted content
        byte[] encryptedContent = new byte[encryptedData.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedData, GCM_IV_LENGTH, encryptedContent, 0, encryptedContent.length);
        
        // Initialize cipher for decryption
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, masterKey, gcmSpec);
        
        // Decrypt data
        byte[] decryptedBytes = cipher.doFinal(encryptedContent);
        return new String(decryptedBytes, "UTF-8");
    }
}