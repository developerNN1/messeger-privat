package com.anonymousmessage.android.utils;

import java.util.regex.Pattern;

public class ValidationUtils {
    
    private static final String PASSWORD_PATTERN = 
        "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{8,}$";
    
    private static final Pattern pattern = Pattern.compile(PASSWORD_PATTERN);

    /**
     * Validates password strength
     * Password must contain:
     * - At least 8 characters
     * - At least one digit
     * - At least one lowercase letter
     * - At least one uppercase letter
     * - At least one special character (@#$%^&+=!)
     * - No whitespace
     */
    public static boolean isPasswordStrong(String password) {
        if (password == null) {
            return false;
        }
        return pattern.matcher(password).matches();
    }
    
    /**
     * Validates username format
     * Username must start with @ and contain only alphanumeric characters and underscores
     */
    public static boolean isValidUsername(String username) {
        if (username == null || !username.startsWith("@")) {
            return false;
        }
        
        String cleanUsername = username.substring(1); // Remove @
        if (cleanUsername.length() < 3 || cleanUsername.length() > 30) {
            return false;
        }
        
        return cleanUsername.matches("[a-zA-Z0-9_]+");
    }
    
    /**
     * Sanitizes username by removing invalid characters
     */
    public static String sanitizeUsername(String username) {
        if (username == null) {
            return null;
        }
        
        // Remove @ if present at the beginning
        if (username.startsWith("@")) {
            username = username.substring(1);
        }
        
        // Keep only alphanumeric and underscore
        username = username.replaceAll("[^a-zA-Z0-9_]", "");
        
        // Ensure length requirements
        if (username.length() < 3) {
            // Pad with underscores if too short
            while (username.length() < 3) {
                username += "_";
            }
        } else if (username.length() > 30) {
            // Truncate if too long
            username = username.substring(0, 30);
        }
        
        return "@" + username;
    }
}