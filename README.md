# AnonymousMessage

A secure, anonymous messaging application that works through Tor without requiring hosting.

## Features
- End-to-end encrypted messaging
- Voice calls with microphone mute/unmute
- Screen sharing (Android + PC)
- Group chats with @username invitations
- Voice messages recording and playback
- Location sharing
- Profile avatars
- macOS-style glass UI design
- Works without hosting via Tor network
- Secure authentication with email verification

## Architecture
- Android client (Java)
- PC client (C++)
- iOS client (React Native)

## Security Design
- All communications routed through Tor
- End-to-end encryption
- No central server dependency
- Anonymous usernames