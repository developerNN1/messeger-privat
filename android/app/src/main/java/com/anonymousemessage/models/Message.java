package com.anonymousemessage.models;

import java.io.Serializable;

public class Message implements Serializable {
    public enum Type {
        TEXT, IMAGE, VIDEO, VOICE, LOCATION, CONTACT, SYSTEM
    }

    private String messageId;
    private String senderId;
    private String recipientId;
    private String content;
    private Type type;
    private long timestamp;
    private boolean isDelivered;
    private boolean isRead;
    private String fileName; // For media files
    private long fileSize;   // For media files
    private String mimeType; // For media files

    public Message() {
        // Default constructor for serialization
    }

    public Message(String messageId, String senderId, String recipientId, String content, Type type, long timestamp) {
        this.messageId = messageId;
        this.senderId = senderId;
        this.recipientId = recipientId;
        this.content = content;
        this.type = type;
        this.timestamp = timestamp;
        this.isDelivered = false;
        this.isRead = false;
    }

    public Message(String senderId, String recipientId, String content, Type type, long timestamp) {
        this(java.util.UUID.randomUUID().toString(), senderId, recipientId, content, type, timestamp);
    }

    // Getters
    public String getMessageId() { return messageId; }
    public String getSenderId() { return senderId; }
    public String getRecipientId() { return recipientId; }
    public String getContent() { return content; }
    public Type getType() { return type; }
    public long getTimestamp() { return timestamp; }
    public boolean isDelivered() { return isDelivered; }
    public boolean isRead() { return isRead; }
    public String getFileName() { return fileName; }
    public long getFileSize() { return fileSize; }
    public String getMimeType() { return mimeType; }

    // Setters
    public void setMessageId(String messageId) { this.messageId = messageId; }
    public void setSenderId(String senderId) { this.senderId = senderId; }
    public void setRecipientId(String recipientId) { this.recipientId = recipientId; }
    public void setContent(String content) { this.content = content; }
    public void setType(Type type) { this.type = type; }
    public void setTimestamp(long timestamp) { this.timestamp = timestamp; }
    public void setDelivered(boolean delivered) { isDelivered = delivered; }
    public void setRead(boolean read) { isRead = read; }
    public void setFileName(String fileName) { this.fileName = fileName; }
    public void setFileSize(long fileSize) { this.fileSize = fileSize; }
    public void setMimeType(String mimeType) { this.mimeType = mimeType; }

    @Override
    public String toString() {
        return "Message{" +
                "messageId='" + messageId + '\'' +
                ", senderId='" + senderId + '\'' +
                ", recipientId='" + recipientId + '\'' +
                ", content='" + content + '\'' +
                ", type=" + type +
                ", timestamp=" + timestamp +
                ", isDelivered=" + isDelivered +
                ", isRead=" + isRead +
                '}';
    }
}