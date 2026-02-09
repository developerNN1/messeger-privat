package com.anonymousemessage.ui.activity;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Patterns;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import com.anonymousemessage.R;

public class RegisterActivity extends AppCompatActivity {
    
    private EditText usernameEditText;
    private EditText emailEditText;
    private EditText passwordEditText;
    private EditText confirmPasswordEditText;
    private Button registerButton;
    private ProgressBar progressBar;
    private TextView signInTextView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);
        
        initViews();
        setupClickListeners();
    }
    
    private void initViews() {
        usernameEditText = findViewById(R.id.username_input);
        emailEditText = findViewById(R.id.email_input);
        passwordEditText = findViewById(R.id.password_input);
        confirmPasswordEditText = findViewById(R.id.confirm_password_input);
        registerButton = findViewById(R.id.register_button);
        progressBar = findViewById(R.id.progress_bar);
        signInTextView = findViewById(R.id.sign_in_text_view);
    }
    
    private void setupClickListeners() {
        registerButton.setOnClickListener(v -> attemptRegistration());
        signInTextView.setOnClickListener(v -> {
            Intent intent = new Intent(RegisterActivity.this, LoginActivity.class);
            startActivity(intent);
            finish();
        });
    }
    
    private void attemptRegistration() {
        // Reset errors
        usernameEditText.setError(null);
        emailEditText.setError(null);
        passwordEditText.setError(null);
        confirmPasswordEditText.setError(null);
        
        String username = usernameEditText.getText().toString().trim();
        String email = emailEditText.getText().toString().trim();
        String password = passwordEditText.getText().toString().trim();
        String confirmPassword = confirmPasswordEditText.getText().toString().trim();
        
        boolean cancel = false;
        View focusView = null;
        
        // Validate inputs
        if (!TextUtils.isEmpty(password) && !isPasswordValid(password)) {
            passwordEditText.setError(getString(R.string.invalid_password));
            focusView = passwordEditText;
            cancel = true;
        }
        
        if (!password.equals(confirmPassword)) {
            confirmPasswordEditText.setError("Passwords do not match");
            focusView = confirmPasswordEditText;
            cancel = true;
        }
        
        if (TextUtils.isEmpty(confirmPassword)) {
            confirmPasswordEditText.setError("Please confirm password");
            focusView = confirmPasswordEditText;
            cancel = true;
        }
        
        if (!isEmailValid(email)) {
            emailEditText.setError("Enter a valid email address");
            focusView = emailEditText;
            cancel = true;
        }
        
        if (TextUtils.isEmpty(username) || !isUsernameValid(username)) {
            usernameEditText.setError("Enter a valid username");
            focusView = usernameEditText;
            cancel = true;
        }
        
        if (cancel) {
            focusView.requestFocus();
        } else {
            performRegistration(username, email, password);
        }
    }
    
    private boolean isEmailValid(String email) {
        return !TextUtils.isEmpty(email) && Patterns.EMAIL_ADDRESS.matcher(email).matches();
    }
    
    private boolean isPasswordValid(String password) {
        return password.length() >= 6;
    }
    
    private boolean isUsernameValid(String username) {
        return username.length() >= 3 && username.matches("^[a-zA-Z0-9_]+$");
    }
    
    private void performRegistration(String username, String email, String password) {
        showProgress(true);
        
        // Check if email is already registered
        checkEmailExists(email, exists -> {
            if (exists) {
                runOnUiThread(() -> {
                    showProgress(false);
                    Toast.makeText(RegisterActivity.this, R.string.email_already_registered, Toast.LENGTH_LONG).show();
                    emailEditText.setError(getString(R.string.email_already_registered));
                    emailEditText.requestFocus();
                });
            } else {
                // Proceed with registration
                registerUser(username, email, password);
            }
        });
    }
    
    private void checkEmailExists(String email, EmailCheckCallback callback) {
        // Real implementation to check if email exists through Tor
        new Thread(() -> {
            try {
                // Create unique identifier for this operation
                String operationId = java.util.UUID.randomUUID().toString();
                
                // Prepare encrypted request to check email existence
                // Using our secure communication protocol over Tor
                java.util.Map<String, Object> requestParams = new java.util.HashMap<>();
                requestParams.put("operation", "check_email");
                requestParams.put("email", email);
                requestParams.put("operation_id", operationId);
                
                // Send request through Tor network
                byte[] requestData = new com.fasterxml.jackson.databind.ObjectMapper()
                    .writeValueAsBytes(requestParams);
                
                // Encrypt request data using session keys
                byte[] encryptedRequest = com.anonymousemessage.service.EncryptionService
                    .encryptData(requestData);
                
                // Send through our secure Tor channel
                byte[] response = com.anonymousemessage.service.TorService
                    .sendSecureRequest(encryptedRequest);
                
                if (response != null) {
                    // Decrypt response
                    byte[] decryptedResponse = com.anonymousemessage.service.EncryptionService
                        .decryptData(response);
                    
                    java.util.Map<String, Object> responseMap = new com.fasterxml.jackson.databind.ObjectMapper()
                        .readValue(decryptedResponse, java.util.Map.class);
                    
                    boolean exists = (Boolean) responseMap.getOrDefault("exists", false);
                    callback.onResult(exists);
                } else {
                    callback.onResult(false);
                }
            } catch (Exception e) {
                // Log security-relevant error but don't expose details to UI
                android.util.Log.e("RegisterActivity", "Email check failed: " + e.getMessage());
                callback.onResult(false);
            }
        }).start();
    }
    
    private void registerUser(String username, String email, String password) {
        new Thread(() -> {
            try {
                // Generate secure keys for this user
                com.anonymousemessage.models.UserKeys userKeys = 
                    com.anonymousemessage.service.EncryptionService.generateUserKeys(username);
                
                // Create registration request with encrypted credentials
                java.util.Map<String, Object> registrationData = new java.util.HashMap<>();
                registrationData.put("username", username);
                registrationData.put("email", email);
                registrationData.put("password_hash", 
                    com.anonymousemessage.service.EncryptionService.hashPassword(password));
                registrationData.put("public_key", userKeys.getPublicKey());
                registrationData.put("timestamp", System.currentTimeMillis());
                
                // Encrypt the entire registration package
                byte[] registrationBytes = new com.fasterxml.jackson.databind.ObjectMapper()
                    .writeValueAsBytes(registrationData);
                
                byte[] encryptedRegistration = com.anonymousemessage.service.EncryptionService
                    .encryptData(registrationBytes);
                
                // Send registration through Tor
                byte[] response = com.anonymousemessage.service.TorService
                    .sendSecureRequest(encryptedRegistration);
                
                if (response != null) {
                    // Parse registration response
                    byte[] decryptedResponse = com.anonymousemessage.service.EncryptionService
                        .decryptData(response);
                    
                    java.util.Map<String, Object> responseMap = new com.fasterxml.jackson.databind.ObjectMapper()
                        .readValue(decryptedResponse, java.util.Map.class);
                    
                    boolean success = (Boolean) responseMap.getOrDefault("success", false);
                    
                    if (success) {
                        // Save user credentials locally (encrypted)
                        com.anonymousemessage.service.SessionManager.getInstance()
                            .saveCredentials(username, email, userKeys.getPrivateKey());
                        
                        runOnUiThread(() -> {
                            showProgress(false);
                            Toast.makeText(RegisterActivity.this, 
                                "Registration successful! Verification code sent to your email.", 
                                Toast.LENGTH_LONG).show();
                            
                            // Navigate to verification screen
                            Intent intent = new Intent(RegisterActivity.this, VerificationActivity.class);
                            intent.putExtra("email", email);
                            intent.putExtra("username", username);
                            intent.putExtra("password", password);
                            startActivity(intent);
                            finish();
                        });
                    } else {
                        runOnUiThread(() -> {
                            showProgress(false);
                            String errorMessage = (String) responseMap.getOrDefault("error", 
                                "Registration failed. Please try again.");
                            Toast.makeText(RegisterActivity.this, errorMessage, 
                                Toast.LENGTH_SHORT).show();
                        });
                    }
                } else {
                    runOnUiThread(() -> {
                        showProgress(false);
                        Toast.makeText(RegisterActivity.this, 
                            "Network error during registration. Please try again.", 
                            Toast.LENGTH_SHORT).show();
                    });
                }
            } catch (Exception e) {
                android.util.Log.e("RegisterActivity", "Registration error: " + e.getMessage(), e);
                runOnUiThread(() -> {
                    showProgress(false);
                    Toast.makeText(RegisterActivity.this, 
                        "Registration failed due to technical error. Please try again.", 
                        Toast.LENGTH_SHORT).show();
                });
            }
        }).start();
    }
    
    private void showProgress(boolean show) {
        progressBar.setVisibility(show ? View.VISIBLE : View.GONE);
        registerButton.setEnabled(!show);
    }
    
    interface EmailCheckCallback {
        void onResult(boolean exists);
    }
}