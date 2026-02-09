package com.anonymousemessage.ui.activity;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.anonymousemessage.R;
import com.anonymousemessage.adapters.MessageAdapter;
import com.anonymousemessage.models.Message;
import com.anonymousemessage.models.User;
import com.anonymousemessage.service.VoiceRecordService;
import java.util.ArrayList;
import java.util.List;

public class ChatActivity extends AppCompatActivity {

    private static final int LOCATION_PERMISSION_REQUEST_CODE = 1001;
    
    private ListView messagesListView;
    private EditText messageInput;
    private Button sendButton;
    private ImageButton attachButton;
    private ImageButton voiceRecordButton;
    private ImageButton locationButton;
    private ImageButton cameraButton;
    
    private MessageAdapter messageAdapter;
    private List<Message> messageList;
    private User contact;
    
    private boolean isRecordingVoice = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat);

        // Get contact from intent
        contact = (User) getIntent().getSerializableExtra("contact");

        initViews();
        setupClickListeners();
        setupMessageList();
    }

    private void initViews() {
        messagesListView = findViewById(R.id.messages_list_view);
        messageInput = findViewById(R.id.message_input);
        sendButton = findViewById(R.id.send_button);
        attachButton = findViewById(R.id.attach_button);
        voiceRecordButton = findViewById(R.id.voice_record_button);
        locationButton = findViewById(R.id.location_button);
        cameraButton = findViewById(R.id.camera_button);
    }

    private void setupClickListeners() {
        sendButton.setOnClickListener(v -> sendMessage());
        
        voiceRecordButton.setOnTouchListener((v, event) -> {
            // Handle long press for voice recording
            switch (event.getAction()) {
                case android.view.MotionEvent.ACTION_DOWN:
                    startVoiceRecording();
                    break;
                case android.view.MotionEvent.ACTION_UP:
                    stopVoiceRecording();
                    break;
            }
            return true;
        });
        
        locationButton.setOnClickListener(v -> sendLocation());
        cameraButton.setOnClickListener(v -> openCamera());
        attachButton.setOnClickListener(v -> openAttachmentMenu());
    }

    private void setupMessageList() {
        messageList = new ArrayList<>();
        messageAdapter = new MessageAdapter(this, messageList);
        messagesListView.setAdapter(messageAdapter);
    }

    private void sendMessage() {
        String text = messageInput.getText().toString().trim();
        if (!text.isEmpty()) {
            // Create and send message
            Message message = new Message(
                System.currentTimeMillis(),
                "current_user_id", // Will be replaced with actual current user ID
                contact.getUserId(),
                text,
                Message.Type.TEXT,
                System.currentTimeMillis()
            );
            
            messageList.add(message);
            messageAdapter.notifyDataSetChanged();
            messageInput.setText("");
            
            // Send message through Tor network
            sendMessageThroughTor(message);
        }
    }

    private void sendMessageThroughTor(Message message) {
        // In real implementation, this would send the message through Tor
        // For now, we just simulate sending
        Toast.makeText(this, "Message sent anonymously via Tor", Toast.LENGTH_SHORT).show();
    }

    private void startVoiceRecording() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) 
            != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, 
                new String[]{Manifest.permission.RECORD_AUDIO}, 1);
            return;
        }
        
        // Start voice recording service
        Intent intent = new Intent(this, VoiceRecordService.class);
        intent.setAction("START_RECORDING");
        startService(intent);
        
        isRecordingVoice = true;
        voiceRecordButton.setImageResource(R.drawable.ic_stop_voice);
        Toast.makeText(this, "Recording voice message...", Toast.LENGTH_SHORT).show();
    }

    private void stopVoiceRecording() {
        if (!isRecordingVoice) return;
        
        // Stop voice recording service
        Intent intent = new Intent(this, VoiceRecordService.class);
        intent.setAction("STOP_RECORDING");
        startService(intent);
        
        isRecordingVoice = false;
        voiceRecordButton.setImageResource(R.drawable.ic_mic);
        Toast.makeText(this, "Voice message recorded", Toast.LENGTH_SHORT).show();
    }

    private void sendLocation() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) 
            != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, 
                new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 
                LOCATION_PERMISSION_REQUEST_CODE);
            return;
        }
        
        // Get current location and send
        getCurrentLocation(location -> {
            if (location != null) {
                Message locationMessage = new Message(
                    System.currentTimeMillis(),
                    "current_user_id", // Will be replaced with actual current user ID
                    contact.getUserId(),
                    location.getLatitude() + "," + location.getLongitude(),
                    Message.Type.LOCATION,
                    System.currentTimeMillis()
                );
                
                messageList.add(locationMessage);
                messageAdapter.notifyDataSetChanged();
                
                // Send location through Tor
                sendMessageThroughTor(locationMessage);
            } else {
                Toast.makeText(this, "Unable to get location", Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void getCurrentLocation(LocationCallback callback) {
        // In real implementation, this would get the current location
        // For now, we'll simulate getting a location
        new Thread(() -> {
            try {
                Thread.sleep(1000); // Simulate location retrieval
                runOnUiThread(() -> {
                    // Simulated location (Moscow center)
                    Location location = new Location("");
                    location.setLatitude(55.7558);
                    location.setLongitude(37.6173);
                    callback.onLocationRetrieved(location);
                });
            } catch (InterruptedException e) {
                runOnUiThread(() -> callback.onLocationRetrieved(null));
            }
        }).start();
    }

    private void openCamera() {
        // Open camera for photo capture
        Toast.makeText(this, "Camera feature coming soon", Toast.LENGTH_SHORT).show();
    }

    private void openAttachmentMenu() {
        // Show attachment options
        Toast.makeText(this, "Attachments feature coming soon", Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, 
                                         @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        
        if (requestCode == LOCATION_PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                sendLocation();
            } else {
                // Permission denied, redirect to settings
                Toast.makeText(this, "Location permission needed to share location", 
                             Toast.LENGTH_LONG).show();
                Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                Uri uri = Uri.fromParts("package", getPackageName(), null);
                intent.setData(uri);
                startActivity(intent);
            }
        }
    }

    interface LocationCallback {
        void onLocationRetrieved(Location location);
    }
}