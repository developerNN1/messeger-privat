package com.anonymousmessage.android.ui.activity;

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

import com.anonymousmessage.android.R;
import com.anonymousmessage.android.network.ApiClient;
import com.anonymousmessage.android.model.User;
import com.anonymousmessage.android.utils.ValidationUtils;

public class RegistrationActivity extends AppCompatActivity {

    private EditText etUsername, etEmail, etPassword;
    private Button btnRegister;
    private TextView tvLoginRedirect;
    private ProgressBar progressBar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_registration);

        initializeViews();
        setupClickListeners();
    }

    private void initializeViews() {
        etUsername = findViewById(R.id.et_username);
        etEmail = findViewById(R.id.et_email);
        etPassword = findViewById(R.id.et_password);
        btnRegister = findViewById(R.id.btn_register);
        tvLoginRedirect = findViewById(R.id.tv_login_redirect);
        progressBar = findViewById(R.id.progress_bar);
    }

    private void setupClickListeners() {
        btnRegister.setOnClickListener(v -> performRegistration());
        tvLoginRedirect.setOnClickListener(v -> navigateToLogin());
    }

    private void performRegistration() {
        String username = etUsername.getText().toString().trim();
        String email = etEmail.getText().toString().trim();
        String password = etPassword.getText().toString().trim();

        // Validate inputs
        if (!validateInputs(username, email, password)) {
            return;
        }

        // Show progress bar
        progressBar.setVisibility(View.VISIBLE);
        btnRegister.setEnabled(false);

        // Check if email is already registered
        ApiClient.checkEmailExists(email, exists -> {
            runOnUiThread(() -> {
                if (exists) {
                    Toast.makeText(this, R.string.email_already_registered, Toast.LENGTH_LONG).show();
                    progressBar.setVisibility(View.GONE);
                    btnRegister.setEnabled(true);
                } else {
                    // Check if username is taken
                    ApiClient.checkUsernameExists(username, usernameTaken -> {
                        runOnUiThread(() -> {
                            if (usernameTaken) {
                                Toast.makeText(this, R.string.username_taken, Toast.LENGTH_LONG).show();
                                progressBar.setVisibility(View.GONE);
                                btnRegister.setEnabled(true);
                            } else {
                                // Register user
                                registerUser(username, email, password);
                            }
                        });
                    });
                }
            });
        });
    }

    private boolean validateInputs(String username, String email, String password) {
        if (TextUtils.isEmpty(username)) {
            etUsername.setError("Username is required");
            etUsername.requestFocus();
            return false;
        }

        if (!username.startsWith("@")) {
            etUsername.setError("Username must start with @");
            etUsername.requestFocus();
            return false;
        }

        if (TextUtils.isEmpty(email)) {
            etEmail.setError("Email is required");
            etEmail.requestFocus();
            return false;
        }

        if (!Patterns.EMAIL_ADDRESS.matcher(email).matches()) {
            etEmail.setError(getString(R.string.invalid_email_format));
            etEmail.requestFocus();
            return false;
        }

        if (TextUtils.isEmpty(password)) {
            etPassword.setError("Password is required");
            etPassword.requestFocus();
            return false;
        }

        if (!ValidationUtils.isPasswordStrong(password)) {
            etPassword.setError(getString(R.string.password_too_weak));
            etPassword.requestFocus();
            return false;
        }

        return true;
    }

    private void registerUser(String username, String email, String password) {
        User user = new User(username, email, password);
        ApiClient.registerUser(user, success -> {
            runOnUiThread(() -> {
                progressBar.setVisibility(View.GONE);
                btnRegister.setEnabled(true);
                
                if (success) {
                    Toast.makeText(this, "Registration successful! Verification code sent to your email.", Toast.LENGTH_LONG).show();
                    // Navigate to verification activity
                    Intent intent = new Intent(this, VerificationActivity.class);
                    intent.putExtra("email", email);
                    startActivity(intent);
                    finish();
                } else {
                    Toast.makeText(this, "Registration failed. Please try again.", Toast.LENGTH_SHORT).show();
                }
            });
        });
    }

    private void navigateToLogin() {
        Intent intent = new Intent(this, LoginActivity.class);
        startActivity(intent);
        finish();
    }
}