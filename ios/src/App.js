import React, { useState, useEffect } from 'react';
import {
  SafeAreaView,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
  Image,
  FlatList,
  PermissionsAndroid,
  Platform,
  Alert,
  ActivityIndicator,
} from 'react-native';

import messaging from '@react-native-firebase/messaging';
import Geolocation from 'react-native-geolocation-service';
import { launchImageLibrary, launchCamera } from 'react-native-image-picker';
import { request, PERMISSIONS } from 'react-native-permissions';

const App = () => {
  const [currentUser, setCurrentUser] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [username, setUsername] = useState('');
  const [screen, setScreen] = useState('login'); // login, register, chat
  const [messages, setMessages] = useState([]);
  const [newMessage, setNewMessage] = useState('');
  const [contacts, setContacts] = useState([]);

  useEffect(() => {
    // Check if user is already logged in
    checkLoggedInUser();
  }, []);

  const checkLoggedInUser = async () => {
    // In a real implementation, this would check stored session
    // For now, we'll just simulate checking
  };

  const handleLogin = async () => {
    setIsLoading(true);
    
    // Validate inputs
    if (!email || !password) {
      Alert.alert('Error', 'Please enter both email and password');
      setIsLoading(false);
      return;
    }

    // Simulate login process
    setTimeout(() => {
      // In real implementation, this would authenticate via Tor
      setCurrentUser({
        id: 'user123',
        username: 'testuser',
        email: email,
        avatar: '',
      });
      setScreen('chat');
      setIsLoading(false);
    }, 1500);
  };

  const handleRegister = async () => {
    setIsLoading(true);
    
    // Validate inputs
    if (!username || !email || !password) {
      Alert.alert('Error', 'Please fill in all fields');
      setIsLoading(false);
      return;
    }

    if (password.length < 6) {
      Alert.alert('Error', 'Password must be at least 6 characters');
      setIsLoading(false);
      return;
    }

    // Simulate registration process
    setTimeout(() => {
      // In real implementation, this would register via Tor
      setCurrentUser({
        id: 'user123',
        username: username,
        email: email,
        avatar: '',
      });
      setScreen('chat');
      setIsLoading(false);
    }, 2000);
  };

  const sendMessage = () => {
    if (!newMessage.trim()) return;

    const message = {
      id: Date.now().toString(),
      text: newMessage,
      senderId: currentUser?.id,
      timestamp: new Date().toISOString(),
      type: 'text',
    };

    setMessages([...messages, message]);
    setNewMessage('');

    // In real implementation, this would send through Tor
    console.log('Sending message:', message);
  };

  const sendLocation = async () => {
    // Request location permission
    if (Platform.OS === 'android') {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        {
          title: 'Location Permission',
          message: 'This app needs access to your location to share it.',
          buttonNeutral: 'Ask Me Later',
          buttonNegative: 'Cancel',
          buttonPositive: 'OK',
        }
      );
      
      if (granted !== PermissionsAndroid.RESULTS.GRANTED) {
        Alert.alert('Permission Denied', 'Location permission is required to share location.');
        return;
      }
    }

    // Get current location
    Geolocation.getCurrentPosition(
      (position) => {
        const { latitude, longitude } = position.coords;
        const locationMessage = {
          id: Date.now().toString(),
          text: `Location: ${latitude}, ${longitude}`,
          senderId: currentUser?.id,
          timestamp: new Date().toISOString(),
          type: 'location',
          location: { latitude, longitude },
        };

        setMessages([...messages, locationMessage]);

        // In real implementation, this would send through Tor
        console.log('Sending location:', locationMessage);
      },
      (error) => {
        Alert.alert('Error', 'Unable to retrieve location: ' + error.message);
      },
      { enableHighAccuracy: true, timeout: 15000, maximumAge: 10000 }
    );
  };

  const selectImage = () => {
    const options = {
      mediaType: 'photo',
      quality: 0.8,
    };

    launchImageLibrary(options, (response) => {
      if (response.didCancel || response.error) {
        return;
      }

      const imageMessage = {
        id: Date.now().toString(),
        text: 'Photo',
        senderId: currentUser?.id,
        timestamp: new Date().toISOString(),
        type: 'image',
        imageUrl: response.assets?.[0]?.uri,
      };

      setMessages([...messages, imageMessage]);

      // In real implementation, this would send through Tor
      console.log('Sending image:', imageMessage);
    });
  };

  const renderMessage = ({ item }) => (
    <View style={[
      styles.messageContainer,
      item.senderId === currentUser?.id ? styles.myMessage : styles.theirMessage
    ]}>
      <Text style={styles.messageText}>{item.text}</Text>
      <Text style={styles.messageTime}>
        {new Date(item.timestamp).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
      </Text>
    </View>
  );

  if (screen === 'login') {
    return (
      <SafeAreaView style={styles.container}>
        <StatusBar barStyle="dark-content" backgroundColor="#f5f5f7" />
        
        <ScrollView contentInsetAdjustmentBehavior="automatic">
          <View style={styles.loginContainer}>
            <Image 
              source={{ uri: 'https://via.placeholder.com/100x100' }} 
              style={styles.logo} 
            />
            
            <Text style={styles.title}>AnonymousMessage</Text>
            <Text style={styles.subtitle}>Secure, Anonymous Messaging via Tor</Text>
            
            <TextInput
              style={styles.input}
              placeholder="Email"
              value={email}
              onChangeText={setEmail}
              keyboardType="email-address"
              autoCapitalize="none"
            />
            
            <TextInput
              style={styles.input}
              placeholder="Password"
              value={password}
              onChangeText={setPassword}
              secureTextEntry
            />
            
            <TouchableOpacity 
              style={styles.button} 
              onPress={handleLogin}
              disabled={isLoading}
            >
              {isLoading ? (
                <ActivityIndicator color="#fff" />
              ) : (
                <Text style={styles.buttonText}>Sign In</Text>
              )}
            </TouchableOpacity>
            
            <TouchableOpacity 
              style={[styles.button, styles.secondaryButton]} 
              onPress={() => setScreen('register')}
            >
              <Text style={styles.secondaryButtonText}>Create New Account</Text>
            </TouchableOpacity>
          </View>
        </ScrollView>
      </SafeAreaView>
    );
  }

  if (screen === 'register') {
    return (
      <SafeAreaView style={styles.container}>
        <StatusBar barStyle="dark-content" backgroundColor="#f5f5f7" />
        
        <ScrollView contentInsetAdjustmentBehavior="automatic">
          <View style={styles.loginContainer}>
            <Text style={styles.title}>Create Account</Text>
            
            <TextInput
              style={styles.input}
              placeholder="Username"
              value={username}
              onChangeText={setUsername}
              autoCapitalize="none"
            />
            
            <TextInput
              style={styles.input}
              placeholder="Email"
              value={email}
              onChangeText={setEmail}
              keyboardType="email-address"
              autoCapitalize="none"
            />
            
            <TextInput
              style={styles.input}
              placeholder="Password"
              value={password}
              onChangeText={setPassword}
              secureTextEntry
            />
            
            <TouchableOpacity 
              style={styles.button} 
              onPress={handleRegister}
              disabled={isLoading}
            >
              {isLoading ? (
                <ActivityIndicator color="#fff" />
              ) : (
                <Text style={styles.buttonText}>Register</Text>
              )}
            </TouchableOpacity>
            
            <TouchableOpacity 
              style={[styles.button, styles.secondaryButton]} 
              onPress={() => setScreen('login')}
            >
              <Text style={styles.secondaryButtonText}>Already have an account? Sign In</Text>
            </TouchableOpacity>
          </View>
        </ScrollView>
      </SafeAreaView>
    );
  }

  if (screen === 'chat') {
    return (
      <SafeAreaView style={styles.container}>
        <StatusBar barStyle="dark-content" backgroundColor="#fff" />
        
        {/* Chat Header */}
        <View style={styles.chatHeader}>
          <Text style={styles.chatTitle}>Anonymous Chat</Text>
        </View>
        
        {/* Messages List */}
        <FlatList
          style={styles.messagesList}
          data={messages}
          renderItem={renderMessage}
          keyExtractor={(item) => item.id}
          inverted
        />
        
        {/* Message Input Area */}
        <View style={styles.inputContainer}>
          <TextInput
            style={styles.messageInput}
            placeholder="Type a message..."
            value={newMessage}
            onChangeText={setNewMessage}
            multiline
          />
          
          <View style={styles.actionButtons}>
            <TouchableOpacity style={styles.actionButton} onPress={selectImage}>
              <Text style={styles.actionButtonText}>📷</Text>
            </TouchableOpacity>
            
            <TouchableOpacity style={styles.actionButton} onPress={sendLocation}>
              <Text style={styles.actionButtonText}>📍</Text>
            </TouchableOpacity>
            
            <TouchableOpacity style={styles.sendButton} onPress={sendMessage}>
              <Text style={styles.sendButtonText}>➤</Text>
            </TouchableOpacity>
          </View>
        </View>
      </SafeAreaView>
    );
  }

  return null;
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f7',
  },
  loginContainer: {
    flex: 1,
    padding: 24,
    justifyContent: 'center',
  },
  logo: {
    width: 100,
    height: 100,
    alignSelf: 'center',
    marginBottom: 24,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 8,
    color: '#000',
  },
  subtitle: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 24,
    color: '#8e8e93',
  },
  input: {
    backgroundColor: '#fff',
    borderWidth: 1,
    borderColor: '#ddd',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    fontSize: 16,
  },
  button: {
    backgroundColor: '#007aff',
    borderRadius: 12,
    padding: 16,
    alignItems: 'center',
    marginBottom: 12,
  },
  buttonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
  secondaryButton: {
    backgroundColor: '#fff',
    borderWidth: 1,
    borderColor: '#007aff',
  },
  secondaryButtonText: {
    color: '#007aff',
    fontSize: 16,
    fontWeight: '600',
  },
  chatHeader: {
    backgroundColor: '#fff',
    padding: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#eee',
  },
  chatTitle: {
    fontSize: 18,
    fontWeight: '600',
    color: '#000',
  },
  messagesList: {
    flex: 1,
    paddingHorizontal: 16,
  },
  messageContainer: {
    maxWidth: '80%',
    padding: 12,
    borderRadius: 18,
    marginVertical: 4,
    position: 'relative',
  },
  myMessage: {
    backgroundColor: '#007aff',
    alignSelf: 'flex-end',
    borderBottomRightRadius: 4,
  },
  theirMessage: {
    backgroundColor: '#e5e5ea',
    alignSelf: 'flex-start',
    borderBottomLeftRadius: 4,
  },
  messageText: {
    color: '#000',
    fontSize: 16,
  },
  myMessageText: {
    color: '#fff',
  },
  messageTime: {
    position: 'absolute',
    bottom: 4,
    right: 8,
    fontSize: 11,
    color: 'rgba(255,255,255,0.7)',
  },
  inputContainer: {
    flexDirection: 'row',
    padding: 12,
    backgroundColor: '#fff',
    alignItems: 'flex-end',
  },
  messageInput: {
    flex: 1,
    backgroundColor: '#e5e5ea',
    borderRadius: 24,
    paddingVertical: 12,
    paddingHorizontal: 16,
    marginRight: 12,
    fontSize: 16,
    maxHeight: 100,
  },
  actionButtons: {
    flexDirection: 'row',
  },
  actionButton: {
    width: 48,
    height: 48,
    borderRadius: 24,
    backgroundColor: '#e5e5ea',
    justifyContent: 'center',
    alignItems: 'center',
    marginRight: 8,
  },
  actionButtonText: {
    fontSize: 20,
  },
  sendButton: {
    width: 48,
    height: 48,
    borderRadius: 24,
    backgroundColor: '#007aff',
    justifyContent: 'center',
    alignItems: 'center',
  },
  sendButtonText: {
    fontSize: 20,
    color: '#fff',
  },
});

export default App;