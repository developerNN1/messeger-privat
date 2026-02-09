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

public class LoginActivity extends AppCompatActivity {
    
    private EditText emailEditText;
    private EditText passwordEditText;
    private Button signInButton;
    private ProgressBar progressBar;
    private TextView registerTextView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        
        initViews();
        setupClickListeners();
    }
    
    private void initViews() {
        emailEditText = findViewById(R.id.email_input);
        passwordEditText = findViewById(R.id.password_input);
        signInButton = findViewById(R.id.sign_in_button);
        progressBar = findViewById(R.id.progress_bar);
        registerTextView = findViewById(R.id.register_text_view);
    }
    
    private void setupClickListeners() {
        signInButton.setOnClickListener(v -> attemptLogin());
        registerTextView.setOnClickListener(v -> {
            Intent intent = new Intent(LoginActivity.this, RegisterActivity.class);
            startActivity(intent);
            finish();
        });
    }
    
    private void attemptLogin() {
        // Reset errors
        emailEditText.setError(null);
        passwordEditText.setError(null);
        
        String email = emailEditText.getText().toString().trim();
        String password = passwordEditText.getText().toString().trim();
        
        boolean cancel = false;
        View focusView = null;
        
        // Validate inputs
        if (TextUtils.isEmpty(password)) {
            passwordEditText.setError("Password is required");
            focusView = passwordEditText;
            cancel = true;
        }
        
        if (!isEmailValid(email)) {
            emailEditText.setError("Enter a valid email address");
            focusView = emailEditText;
            cancel = true;
        }
        
        if (cancel) {
            focusView.requestFocus();
        } else {
            performLogin(email, password);
        }
    }
    
    private boolean isEmailValid(String email) {
        return !TextUtils.isEmpty(email) && Patterns.EMAIL_ADDRESS.matcher(email).matches();
    }
    
    private void performLogin(String email, String password) {
        showProgress(true);
        
        // In real implementation, this would securely authenticate through Tor
        // For now, we'll simulate the login process
        new Thread(() -> {
            try {
                Thread.sleep(1500); // Simulate network delay
                runOnUiThread(() -> {
                    showProgress(false);
                    
                    // For demo purposes, if email is valid, go to main activity
                    if (Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
                        Toast.makeText(LoginActivity.this, "Login successful!", Toast.LENGTH_SHORT).show();
                        
                        Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                        startActivity(intent);
                        finish();
                    } else {
                        Toast.makeText(LoginActivity.this, "Login failed. Please try again.", Toast.LENGTH_SHORT).show();
                    }
                });
            } catch (InterruptedException e) {
                runOnUiThread(() -> {
                    showProgress(false);
                    Toast.makeText(LoginActivity.this, "Login failed. Please try again.", Toast.LENGTH_SHORT).show();
                });
            }
        }).start();
    }
    
    private void showProgress(boolean show) {
        progressBar.setVisibility(show ? View.VISIBLE : View.GONE);
        signInButton.setEnabled(!show);
    }
}