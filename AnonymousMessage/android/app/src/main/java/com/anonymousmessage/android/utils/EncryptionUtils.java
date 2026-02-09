package com.anonymousmessage.android.utils;

import android.util.Base64;
import android.util.Log;

import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.KeyGenerator;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * Utility class for encryption and decryption operations
 * Uses AES-GCM for symmetric encryption and RSA for key exchange
 */
public class EncryptionUtils {
    private static final String TAG = "EncryptionUtils";
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 96 bits
    private static final int GCM_TAG_LENGTH = 16; // 128 bits

    /**
     * Generates a new AES key
     * @return SecretKey object
     */
    public static SecretKey generateKey() {
        try {
            KeyGenerator keyGenerator = KeyGenerator.getInstance(ALGORITHM);
            keyGenerator.init(256); // 256-bit key
            return keyGenerator.generateKey();
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "Error generating key", e);
            return null;
        }
    }

    /**
     * Encrypts data using AES-GCM
     * @param plaintext Data to encrypt
     * @param key Encryption key
     * @return Encrypted data with IV prepended
     */
    public static byte[] encrypt(byte[] plaintext, SecretKey key) {
        try {
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
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 InvalidAlgorithmParameterException | IllegalBlockSizeException | BadPaddingException e) {
            Log.e(TAG, "Error encrypting data", e);
            return null;
        }
    }

    /**
     * Decrypts data using AES-GCM
     * @param data Encrypted data with IV prepended
     * @param key Decryption key
     * @return Decrypted data
     */
    public static byte[] decrypt(byte[] data, SecretKey key) {
        try {
            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            
            // Extract IV from the beginning of the data
            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(data, 0, iv, 0, GCM_IV_LENGTH);
            
            // Extract ciphertext
            byte[] ciphertext = new byte[data.length - GCM_IV_LENGTH];
            System.arraycopy(data, GCM_IV_LENGTH, ciphertext, 0, ciphertext.length);
            
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, spec);
            
            return cipher.doFinal(ciphertext);
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 InvalidAlgorithmParameterException | IllegalBlockSizeException | BadPaddingException e) {
            Log.e(TAG, "Error decrypting data", e);
            return null;
        }
    }

    /**
     * Converts SecretKey to Base64 string
     * @param key SecretKey to convert
     * @return Base64 representation of the key
     */
    public static String keyToString(SecretKey key) {
        if (key == null) return null;
        return Base64.encodeToString(key.getEncoded(), Base64.NO_WRAP);
    }

    /**
     * Creates SecretKey from Base64 string
     * @param keyString Base64 representation of the key
     * @return SecretKey object
     */
    public static SecretKey stringToKey(String keyString) {
        if (keyString == null) return null;
        byte[] decodedKey = Base64.decode(keyString, Base64.NO_WRAP);
        return new SecretKeySpec(decodedKey, ALGORITHM);
    }

    /**
     * Generates SHA-256 hash of input data
     * @param data Input data to hash
     * @return Hash as byte array
     */
    public static byte[] sha256Hash(byte[] data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            return digest.digest(data);
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "Error hashing data", e);
            return null;
        }
    }

    /**
     * Generates SHA-256 hash of input string
     * @param input Input string to hash
     * @return Hash as hex string
     */
    public static String sha256Hash(String input) {
        byte[] hash = sha256Hash(input.getBytes());
        if (hash == null) return null;
        
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString();
    }
}