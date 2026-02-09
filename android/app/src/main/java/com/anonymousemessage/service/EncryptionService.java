package com.anonymousemessage.service;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class EncryptionService {
    
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 16;
    
    /**
     * Encrypts data using AES-GCM algorithm with a random IV
     */
    public static byte[] encryptData(byte[] data) throws Exception {
        // Generate a random secret key
        KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
        keyGen.init(256);
        SecretKey secretKey = keyGen.generateKey();
        
        // Generate a random IV
        byte[] iv = new byte[GCM_IV_LENGTH];
        new SecureRandom().nextBytes(iv);
        
        // Initialize cipher
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmSpec);
        
        // Encrypt data
        byte[] encryptedData = cipher.doFinal(data);
        
        // Combine IV + encrypted data for transmission
        byte[] result = new byte[iv.length + encryptedData.length];
        System.arraycopy(iv, 0, result, 0, iv.length);
        System.arraycopy(encryptedData, 0, result, iv.length, encryptedData.length);
        
        return result;
    }
    
    /**
     * Decrypts data using AES-GCM algorithm
     */
    public static byte[] decryptData(byte[] encryptedData) throws Exception {
        // Extract IV from the beginning
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedData, 0, iv, 0, GCM_IV_LENGTH);
        
        // Extract encrypted content
        byte[] encryptedContent = new byte[encryptedData.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedData, GCM_IV_LENGTH, encryptedContent, 0, encryptedContent.length);
        
        // Generate a key (in real implementation, this would come from session)
        KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
        keyGen.init(256);
        SecretKey secretKey = keyGen.generateKey();
        
        // Initialize cipher for decryption
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmSpec);
        
        // Decrypt data
        return cipher.doFinal(encryptedContent);
    }
    
    /**
     * Hashes a password using SHA-256
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes("UTF-8"));
            StringBuilder hexString = new StringBuilder();
            
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            
            return hexString.toString();
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }
    
    /**
     * Generates user-specific keys for secure communication
     */
    public static UserKeys generateUserKeys(String username) {
        try {
            // Generate AES key for symmetric encryption
            KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
            keyGen.init(256);
            SecretKey aesKey = keyGen.generateKey();
            
            // For asymmetric key exchange, we'd normally use RSA or EC
            // Here we're simulating public/private key generation
            // In production, use proper RSA key pair generation
            
            // Generate a pseudo-public key (in real implementation, this would be actual public key)
            String publicKey = Base64.getEncoder().encodeToString(aesKey.getEncoded()).substring(0, 32);
            
            // Return both keys
            return new UserKeys(publicKey, Base64.getEncoder().encodeToString(aesKey.getEncoded()));
        } catch (Exception e) {
            throw new RuntimeException("Failed to generate user keys", e);
        }
    }
}