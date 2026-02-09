import Foundation
import React

@objc(TorBridge)
class TorBridge: NSObject {
    
    @objc static func requiresMainQueueSetup() -> Bool {
        return false
    }
    
    // Method to check if Tor is connected
    @objc func isTorConnected(_ callback: @escaping RCTResponseSenderBlock) {
        let isConnected = TorManager.shared.isConnected()
        callback([isConnected])
    }
    
    // Method to start Tor connection
    @objc func startTor(_ callback: @escaping RCTResponseSenderBlock) {
        TorManager.shared.startTor { success, error in
            if success {
                callback([NSNull(), true])
            } else {
                callback([error?.localizedDescription ?? "Unknown error", false])
            }
        }
    }
    
    // Method to send data through Tor
    @objc func sendThroughTor(_ data: String, endpoint: String, callback: @escaping RCTResponseSenderBlock) {
        guard let dataObj = data.data(using: .utf8) else {
            callback(["Invalid data", NSNull()])
            return
        }
        
        TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion\(endpoint)", body: dataObj) { responseData, response, error in
            if let error = error {
                callback([error.localizedDescription, NSNull()])
                return
            }
            
            guard let responseData = responseData else {
                callback(["No response data", NSNull()])
                return
            }
            
            if let responseString = String(data: responseData, encoding: .utf8) {
                callback([NSNull(), responseString])
            } else {
                callback(["Could not parse response", NSNull()])
            }
        }
    }
    
    // Method to hash password securely
    @objc func hashPassword(_ password: String, callback: @escaping RCTResponseSenderBlock) {
        let (hashedPassword, salt) = EncryptionManager.shared.hashPassword(password)
        let result = [
            "hash": hashedPassword.base64EncodedString(),
            "salt": salt.base64EncodedString()
        ]
        callback([NSNull(), result])
    }
    
    // Method to generate user keys
    @objc func generateUserKeys(_ username: String, callback: @escaping RCTResponseSenderBlock) {
        guard let keyPair = EncryptionManager.shared.generateRSAKeyPair() else {
            callback(["Failed to generate keys", NSNull()])
            return
        }
        
        let result = [
            "publicKey": keyPair.publicKey.base64EncodedString(),
            "privateKey": keyPair.privateKey.base64EncodedString()
        ]
        callback([NSNull(), result])
    }
    
    // Method to encrypt data
    @objc func encryptData(_ data: NSDictionary, callback: @escaping RCTResponseSenderBlock) {
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: data)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            callback([NSNull(), encryptedData.base64EncodedString()])
        } catch {
            callback([error.localizedDescription, NSNull()])
        }
    }
    
    // Method to decrypt data
    @objc func decryptData(_ encryptedDataBase64: String, callback: @escaping RCTResponseSenderBlock) {
        guard let encryptedData = Data(base64Encoded: encryptedDataBase64) else {
            callback(["Invalid encrypted data", NSNull()])
            return
        }
        
        do {
            // For demo purposes, using a random key - in practice, you'd need the original key
            let decryptedData = try EncryptionManager.shared.decrypt(data: encryptedData, withKey: EncryptionManager.shared.generateAESKey())
            
            if let jsonString = String(data: decryptedData, encoding: .utf8),
               let jsonObject = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? NSDictionary {
                callback([NSNull(), jsonObject])
            } else {
                callback(["Could not parse decrypted data", NSNull()])
            }
        } catch {
            callback([error.localizedDescription, NSNull()])
        }
    }
    
    // Method to securely store data in keychain
    @objc func storeInKeychain(_ key: String, data: String, callback: @escaping RCTResponseSenderBlock) {
        let dataObj = Data(data.utf8)
        let success = SecurityManager.shared.storeInKeychain(key: key, data: dataObj)
        callback([NSNull(), success])
    }
    
    // Method to retrieve data from keychain
    @objc func retrieveFromKeychain(_ key: String, callback: @escaping RCTResponseSenderBlock) {
        if let dataObj = SecurityManager.shared.retrieveFromKeychain(key: key) {
            let dataString = String(decoding: dataObj, as: UTF8.self)
            callback([NSNull(), dataString])
        } else {
            callback(["Key not found", NSNull()])
        }
    }
    
    // Method to check if device is jailbroken
    @objc func isDeviceJailbroken(_ callback: @escaping RCTResponseSenderBlock) {
        let isJailbroken = SecurityManager.shared.isDeviceJailbroken()
        callback([isJailbroken])
    }
    
    // Method to check if app is tampered
    @objc func isAppTampered(_ callback: @escaping RCTResponseSenderBlock) {
        let isTampered = SecurityManager.shared.isAppTampered()
        callback([isTampered])
    }
}