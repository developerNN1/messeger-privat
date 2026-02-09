import Foundation
import LocalAuthentication

class SecurityManager {
    static let shared = SecurityManager()
    private init() {}
    
    // Biometric authentication
    func authenticateWithBiometrics(reason: String, completion: @escaping (Bool, Error?) -> Void) {
        let context = LAContext()
        var error: NSError?
        
        // Check if biometric authentication is available
        if !context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, error: &error) {
            completion(false, error)
            return
        }
        
        // Authenticate
        context.evaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, localizedReason: reason) { success, authError in
            DispatchQueue.main.async {
                if success {
                    completion(true, nil)
                } else {
                    completion(false, authError)
                }
            }
        }
    }
    
    // Device passcode authentication
    func authenticateWithPasscode(reason: String, completion: @escaping (Bool, Error?) -> Void) {
        let context = LAContext()
        var error: NSError?
        
        // Check if device passcode authentication is available
        if !context.canEvaluatePolicy(.deviceOwnerAuthentication, error: &error) {
            completion(false, error)
            return
        }
        
        // Authenticate
        context.evaluatePolicy(.deviceOwnerAuthentication, localizedReason: reason) { success, authError in
            DispatchQueue.main.async {
                if success {
                    completion(true, nil)
                } else {
                    completion(false, authError)
                }
            }
        }
    }
    
    // Securely store sensitive data in Keychain
    func storeInKeychain(key: String, data: Data) -> Bool {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key,
            kSecValueData as String: data,
            kSecAttrAccessible as String: kSecAttrAccessibleWhenUnlockedThisDeviceOnly
        ]
        
        // Delete any existing item
        SecItemDelete(query as CFDictionary)
        
        // Add new item
        let status = SecItemAdd(query as CFDictionary, nil)
        return status == errSecSuccess
    }
    
    // Retrieve sensitive data from Keychain
    func retrieveFromKeychain(key: String) -> Data? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key,
            kSecReturnData as String: true,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]
        
        var dataTypeRef: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &dataTypeRef)
        
        if status == errSecSuccess {
            return dataTypeRef as? Data
        }
        
        return nil
    }
    
    // Delete sensitive data from Keychain
    func deleteFromKeychain(key: String) -> Bool {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key
        ]
        
        let status = SecItemDelete(query as CFDictionary)
        return status == errSecSuccess
    }
    
    // Securely hash data with salt
    func secureHash(data: String, salt: Data? = nil) -> (hash: String, salt: Data) {
        let saltData = salt ?? generateSalt()
        let inputData = (data + String(data: saltData, encoding: .utf8)!).data(using: .utf8)!
        
        // Using SHA256
        let hashedData = SHA256.hash(data: inputData)
        let hashString = hashedData.compactMap { String(format: "%02x", $0) }.joined()
        
        return (hash: hashString, salt: saltData)
    }
    
    // Generate a random salt
    private func generateSalt() -> Data {
        var salt = Data(count: 32) // 256-bit salt
        let result = salt.withUnsafeMutableBytes { bytes in
            return SecRandomCopyBytes(kSecRandomDefault, 32, bytes.baseAddress!)
        }
        
        if result == errSecSuccess {
            return salt
        } else {
            fatalError("Failed to generate salt")
        }
    }
    
    // Verify if the provided data matches the stored hash
    func verifyHash(data: String, hash: String, salt: Data) -> Bool {
        let (computedHash, _) = secureHash(data: data, salt: salt)
        return computedHash == hash
    }
    
    // Encrypt data using AES-GCM with a key stored in Keychain
    func encryptAndStoreInKeychain(key: String, data: Data, forKeychainKey: String) throws -> Bool {
        // Generate a random key for this encryption
        let encryptionKey = EncryptionManager.shared.generateAESKey()
        
        // Encrypt the data
        let encryptedData = try EncryptionManager.shared.encrypt(data: data, withKey: encryptionKey)
        
        // Store the encryption key in Keychain
        let keyStored = storeInKeychain(key: forKeychainKey, data: encryptionKey)
        
        if keyStored {
            // Store the encrypted data in UserDefaults (or wherever appropriate)
            UserDefaults.standard.set(encryptedData, forKey: key)
            return true
        }
        
        return false
    }
    
    // Decrypt data that was encrypted and stored using encryptAndStoreInKeychain
    func decryptFromKeychain(key: String, forKeychainKey: String) throws -> Data? {
        // Retrieve the encryption key from Keychain
        guard let encryptionKey = retrieveFromKeychain(key: forKeychainKey) else {
            return nil
        }
        
        // Retrieve the encrypted data
        guard let encryptedData = UserDefaults.standard.data(forKey: key) else {
            return nil
        }
        
        // Decrypt the data
        let decryptedData = try EncryptionManager.shared.decrypt(data: encryptedData, withKey: encryptionKey)
        
        return decryptedData
    }
    
    // Check if the device is jailbroken (basic check)
    func isDeviceJailbroken() -> Bool {
        // Check for common jailbreak indicators
        let paths = [
            "/Applications/Cydia.app",
            "/Library/MobileSubstrate/MobileSubstrate.dylib",
            "/bin/bash",
            "/usr/sbin/sshd",
            "/etc/apt",
            "/private/var/lib/apt/",
            "/private/var/mobile/Library/SBSettings/Themes",
            "/Library/MobileSubstrate/CydiaSubstrate.dylib"
        ]
        
        for path in paths {
            if FileManager.default.fileExists(atPath: path) {
                return true
            }
        }
        
        // Check if we can write to system directories
        let testPath = "/private/jailbreak_test"
        do {
            try "test".write(toFile: testPath, atomically: true, encoding: .utf8)
            // If we can write, try to clean up
            try? FileManager.default.removeItem(atPath: testPath)
            return true
        } catch {
            // If we can't write, that's good
        }
        
        // Check if debugging is allowed (another indicator)
        var name: [Int32] = [CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()]
        var info: kinfo_proc = kinfo_proc()
        var info_size = MemoryLayout<kinfo_proc>.size
        
        let result = sysctl(&name, 4, &info, &info_size, nil, 0)
        if result == 0 {
            // Check the process flags for PT_TRACE_ME
            if (info.kp_proc.p_flag & P_TRACED) != 0 {
                return true
            }
        }
        
        return false
    }
    
    // Monitor app for tampering
    func isAppTampered() -> Bool {
        // Check if the app is running in simulator
        #if targetEnvironment(simulator)
            return true
        #endif
        
        // Check if debugger is attached
        if isDebuggerAttached() {
            return true
        }
        
        // Check if device is jailbroken
        if isDeviceJailbroken() {
            return true
        }
        
        // Check for hooking frameworks
        if isHookingDetected() {
            return true
        }
        
        return false
    }
    
    // Check if debugger is attached
    private func isDebuggerAttached() -> Bool {
        var info = kinfo_proc()
        var mib: [Int32] = [CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()]
        var size = MemoryLayout<kinfo_proc>.stride
        
        let junk = sysctl(&mib, 4, &info, &size, nil, 0)
        assert(junk == 0, "sysctl failed")
        
        return (info.kp_proc.p_flag & P_TRACED) != 0
    }
    
    // Check for hooking frameworks
    private func isHookingDetected() -> Bool {
        // Check for dynamic library injection
        let suspiciousLibraries = [
            "CydiaSubstrate",
            "MobileSubstrate",
            "SubstrateInserter",
            "libsubstrate",
            "Frida",
            "frida",
            "Xposed"
        ]
        
        // This is a simplified check - in a real implementation, 
        // you would iterate through loaded libraries
        for library in suspiciousLibraries {
            if let _ = NSClassFromString(library) {
                return true
            }
        }
        
        return false
    }
    
    // Securely wipe sensitive data
    func securelyWipeData(data: inout Data) {
        // Fill with random data multiple times
        for _ in 0..<5 {
            let randomData = Data(count: data.count)
            data = randomData
        }
        
        // Finally, reset to zero
        data.removeAll()
        data.shrinkToFit()
    }
}