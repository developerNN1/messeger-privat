package com.anonymousemessage.utils;

import android.util.Base64;
import android.util.Log;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;

public class EncryptionUtil {
    
    private static final String TAG = "EncryptionUtil";
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 96 bits
    private static final int GCM_TAG_LENGTH = 16; // 128 bits
    
    /**
     * Generates a new AES key
     */
    public static SecretKey generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance(ALGORITHM);
        keyGenerator.init(256); // 256-bit key
        return keyGenerator.generateKey();
    }
    
    /**
     * Encrypts data using AES-GCM
     */
    public static byte[] encrypt(byte[] plaintext, SecretKey key) throws Exception {
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        
        // Generate random IV
        byte[] iv = new byte[GCM_IV_LENGTH];
        new SecureRandom().nextBytes(iv);
        
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        byte[] ciphertext = cipher.doFinal(plaintext);
        
        // Prepend IV to ciphertext
        byte[] result = new byte[iv.length + ciphertext.length];
        System.arraycopy(iv, 0, result, 0, iv.length);
        System.arraycopy(ciphertext, 0, result, iv.length, ciphertext.length);
        
        return result;
    }
    
    /**
     * Decrypts data using AES-GCM
     */
    public static byte[] decrypt(byte[] encryptedData, SecretKey key) throws Exception {
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        
        // Extract IV from the beginning
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedData, 0, iv, 0, GCM_IV_LENGTH);
        
        // Extract the actual ciphertext
        byte[] ciphertext = new byte[encryptedData.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedData, GCM_IV_LENGTH, ciphertext, 0, ciphertext.length);
        
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        
        return cipher.doFinal(ciphertext);
    }
    
    /**
     * Converts a key to Base64 string representation
     */
    public static String keyToString(SecretKey key) {
        return Base64.encodeToString(key.getEncoded(), Base64.NO_WRAP);
    }
    
    /**
     * Creates a key from Base64 string representation
     */
    public static SecretKey stringToKey(String keyString) {
        byte[] decodedKey = Base64.decode(keyString, Base64.NO_WRAP);
        return new SecretKeySpec(decodedKey, 0, decodedKey.length, ALGORITHM);
    }
    
    /**
     * Encrypts a string
     */
    public static String encryptString(String plaintext, SecretKey key) throws Exception {
        byte[] encrypted = encrypt(plaintext.getBytes("UTF-8"), key);
        return Base64.encodeToString(encrypted, Base64.NO_WRAP);
    }
    
    /**
     * Decrypts a string
     */
    public static String decryptString(String encryptedText, SecretKey key) throws Exception {
        byte[] decrypted = decrypt(Base64.decode(encryptedText, Base64.NO_WRAP), key);
        return new String(decrypted, "UTF-8");
    }
}