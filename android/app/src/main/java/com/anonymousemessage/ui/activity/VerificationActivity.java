package com.anonymousemessage.ui.activity;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import com.anonymousemessage.R;

public class VerificationActivity extends AppCompatActivity {

    private EditText verificationCodeInput;
    private Button verifyButton;
    private ProgressBar progressBar;
    private TextView resendCodeTextView;
    
    private String email, username, password;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_verification);

        // Get data from previous activity
        email = getIntent().getStringExtra("email");
        username = getIntent().getStringExtra("username");
        password = getIntent().getStringExtra("password");

        initViews();
        setupClickListeners();
    }

    private void initViews() {
        verificationCodeInput = findViewById(R.id.verification_code_input);
        verifyButton = findViewById(R.id.verify_button);
        progressBar = findViewById(R.id.progress_bar);
        resendCodeTextView = findViewById(R.id.resend_code_text_view);
    }

    private void setupClickListeners() {
        verifyButton.setOnClickListener(v -> attemptVerification());
        resendCodeTextView.setOnClickListener(v -> resendVerificationCode());
    }

    private void attemptVerification() {
        String code = verificationCodeInput.getText().toString().trim();

        if (TextUtils.isEmpty(code)) {
            verificationCodeInput.setError("Verification code is required");
            verificationCodeInput.requestFocus();
            return;
        }

        if (code.length() != 6) {
            verificationCodeInput.setError("Invalid verification code");
            verificationCodeInput.requestFocus();
            return;
        }

        performVerification(code);
    }

    private void performVerification(String code) {
        showProgress(true);

        // In real implementation, this would verify the code through Tor
        // For now, we'll simulate the verification process
        new Thread(() -> {
            try {
                Thread.sleep(1500); // Simulate network delay
                runOnUiThread(() -> {
                    showProgress(false);
                    
                    // For demo purposes, accept any 6-digit code
                    if (code.matches("\\d{6}")) {
                        Toast.makeText(VerificationActivity.this, "Verification successful!", Toast.LENGTH_SHORT).show();
                        
                        // Complete registration
                        completeRegistration();
                    } else {
                        Toast.makeText(VerificationActivity.this, "Invalid verification code. Please try again.", Toast.LENGTH_SHORT).show();
                    }
                });
            } catch (InterruptedException e) {
                runOnUiThread(() -> {
                    showProgress(false);
                    Toast.makeText(VerificationActivity.this, "Verification failed. Please try again.", Toast.LENGTH_SHORT).show();
                });
            }
        }).start();
    }

    private void completeRegistration() {
        // Registration completed successfully
        // Navigate to login screen
        Intent intent = new Intent(VerificationActivity.this, LoginActivity.class);
        intent.putExtra("email", email);
        startActivity(intent);
        finish();
    }

    private void resendVerificationCode() {
        // Resend verification code
        // In real implementation, this would be a network call through Tor
        new Thread(() -> {
            try {
                Thread.sleep(1000); // Simulate network delay
                runOnUiThread(() -> {
                    Toast.makeText(VerificationActivity.this, "Verification code resent to your email.", Toast.LENGTH_SHORT).show();
                });
            } catch (InterruptedException e) {
                runOnUiThread(() -> {
                    Toast.makeText(VerificationActivity.this, "Failed to resend code. Please try again.", Toast.LENGTH_SHORT).show();
                });
            }
        }).start();
    }

    private void showProgress(boolean show) {
        progressBar.setVisibility(show ? View.VISIBLE : View.GONE);
        verifyButton.setEnabled(!show);
    }
}