import Foundation
import CommonCrypto
import CryptoKit

class EncryptionManager {
    static let shared = EncryptionManager()
    private init() {}
    
    // Generate a random key for AES encryption
    func generateAESKey() -> Data {
        var keyData = Data(count: 32) // 256-bit key
        let result = keyData.withUnsafeMutableBytes { bytes in
            return SecRandomCopyBytes(kSecRandomDefault, 32, bytes.baseAddress!)
        }
        
        if result == errSecSuccess {
            return keyData
        } else {
            fatalError("Failed to generate random key")
        }
    }
    
    // Generate RSA key pair
    func generateRSAKeyPair() -> (publicKey: Data, privateKey: Data)? {
        let tag = "com.anonymousmessage.rsa"
        
        // Remove existing keys
        let removeQuery: [String: Any] = [
            kSecClass as String: kSecClassKey,
            kSecAttrApplicationTag as String: tag
        ]
        SecItemDelete(removeQuery as CFDictionary)
        
        let attributes: [String: Any] = [
            kSecAttrKeyType as String: kSecAttrKeyTypeRSA,
            kSecAttrKeySizeInBits as String: 2048,
            kSecPrivateKeyAttrs as String: [
                kSecAttrIsPermanent as String: true,
                kSecAttrApplicationTag as String: "\(tag).private"
            ],
            kSecPublicKeyAttrs as String: [
                kSecAttrIsPermanent as String: false,
                kSecAttrApplicationTag as String: "\(tag).public"
            ]
        ]
        
        var error: Unmanaged<CFError>?
        guard let privateKey = SecKeyCreateRandomKey(attributes as CFDictionary, &error),
              let publicKey = SecKeyCopyPublicKey(privateKey) else {
            print("Error generating RSA key pair: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return nil
        }
        
        guard let publicKeyData = SecKeyCopyExternalRepresentation(publicKey, &error),
              let privateKeyData = SecKeyCopyExternalRepresentation(privateKey, &error) else {
            print("Error getting key data: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return nil
        }
        
        return (publicKey: publicKeyData as Data, privateKey: privateKeyData as Data)
    }
    
    // Hash password using PBKDF2
    func hashPassword(_ password: String, salt: Data? = nil) -> (hash: Data, salt: Data) {
        let saltData = salt ?? generateSalt()
        let passwordData = password.data(using: .utf8)!
        
        var hashedData = Data(repeating: 0, count: 32) // SHA256 produces 32-byte output
        
        let status = hashedData.withUnsafeMutableBytes { hashedBytes in
            saltData.withUnsafeBytes { saltBytes in
                CCKeyDerivationPBKDF(
                    CCPBKDFAlgorithm(kCCPBKDF2),
                    password,
                    password.count,
                    saltBytes.baseAddress!.assumingMemoryBound(to: UInt8.self),
                    saltData.count,
                    CCPseudoRandomAlgorithm(kCCPRFHmacAlgSHA256),
                    10000, // iterations
                    hashedBytes.baseAddress!.assumingMemoryBound(to: UInt8.self),
                    hashedData.count
                )
            }
        }
        
        if status == kCCSuccess {
            return (hash: hashedData, salt: saltData)
        } else {
            fatalError("Failed to hash password")
        }
    }
    
    // Generate a random salt
    private func generateSalt() -> Data {
        var salt = Data(count: 16) // 128-bit salt
        let result = salt.withUnsafeMutableBytes { bytes in
            return SecRandomCopyBytes(kSecRandomDefault, 16, bytes.baseAddress!)
        }
        
        if result == errSecSuccess {
            return salt
        } else {
            fatalError("Failed to generate salt")
        }
    }
    
    // AES-GCM encryption
    func encrypt(data: Data, withKey key: Data) throws -> Data {
        let symmetricKey = SymmetricKey(data: key)
        let sealedBox = try AES.GCM.seal(data, using: symmetricKey)
        return Data(sealedBox.combined)
    }
    
    // AES-GCM decryption
    func decrypt(data: Data, withKey key: Data) throws -> Data {
        let symmetricKey = SymmetricKey(data: key)
        let sealedBox = try AES.GCM.SealedBox(combined: data)
        return try AES.GCM.open(sealedBox, using: symmetricKey)
    }
    
    // RSA encryption
    func rsaEncrypt(data: Data, withPublicKey publicKeyData: Data) -> Data? {
        var error: Unmanaged<CFError>?
        let publicKey = SecKeyCreateWithData(publicKeyData as CFData, [
            kSecAttrKeyType: kSecAttrKeyTypeRSA,
            kSecAttrKeyClass: kSecAttrKeyClassPublic,
            kSecAttrKeySizeInBits: 2048
        ] as CFDictionary, &error)
        
        guard let key = publicKey else {
            print("Error creating public key: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return nil
        }
        
        let blockSize = SecKeyGetBlockSize(key) - 11 // PKCS1 padding overhead
        let dataBytes = Array(data)
        var encryptedData = Data()
        
        for i in stride(from: 0, to: dataBytes.count, by: blockSize) {
            let endIndex = min(i + blockSize, dataBytes.count)
            let chunk = Array(dataBytes[i..<endIndex])
            var encryptedChunk = [UInt8](repeating: 0, count: SecKeyGetBlockSize(key))
            var encryptedLength = SecKeyGetBlockSize(key)
            
            let result = SecKeyRawVerify(key, .PKCS1SHA256, chunk, chunk.count, &encryptedChunk, &encryptedLength)
            
            if result == errSecSuccess {
                encryptedData.append(contentsOf: encryptedChunk)
            } else {
                print("Error during RSA encryption: \(result)")
                return nil
            }
        }
        
        return encryptedData
    }
    
    // RSA decryption
    func rsaDecrypt(data: Data, withPrivateKey privateKeyData: Data) -> Data? {
        var error: Unmanaged<CFError>?
        let privateKey = SecKeyCreateWithData(privateKeyData as CFData, [
            kSecAttrKeyType: kSecAttrKeyTypeRSA,
            kSecAttrKeyClass: kSecAttrKeyClassPrivate,
            kSecAttrKeySizeInBits: 2048
        ] as CFDictionary, &error)
        
        guard let key = privateKey else {
            print("Error creating private key: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return nil
        }
        
        let blockSize = SecKeyGetBlockSize(key)
        let dataBytes = Array(data)
        var decryptedData = Data()
        
        for i in stride(from: 0, to: dataBytes.count, by: blockSize) {
            let endIndex = min(i + blockSize, dataBytes.count)
            let chunk = Array(dataBytes[i..<endIndex])
            var decryptedChunk = [UInt8](repeating: 0, count: blockSize)
            var decryptedLength = blockSize
            
            let result = SecKeyRawSign(key, .PKCS1SHA256, chunk, chunk.count, &decryptedChunk, &decryptedLength)
            
            if result == errSecSuccess {
                decryptedData.append(contentsOf: decryptedChunk[0..<decryptedLength])
            } else {
                print("Error during RSA decryption: \(result)")
                return nil
            }
        }
        
        return decryptedData
    }
    
    // Sign data with RSA private key
    func sign(data: Data, withPrivateKey privateKeyData: Data) -> Data? {
        var error: Unmanaged<CFError>?
        let privateKey = SecKeyCreateWithData(privateKeyData as CFData, [
            kSecAttrKeyType: kSecAttrKeyTypeRSA,
            kSecAttrKeyClass: kSecAttrKeyClassPrivate,
            kSecAttrKeySizeInBits: 2048
        ] as CFDictionary, &error)
        
        guard let key = privateKey else {
            print("Error creating private key: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return nil
        }
        
        var signedData = [UInt8](repeating: 0, count: SecKeyGetBlockSize(key))
        var signedLength = SecKeyGetBlockSize(key)
        
        let digest = SHA256.hash(data: data)
        let digestBytes = Array(digest)
        
        let result = SecKeyRawSign(key, .PKCS1SHA256, digestBytes, digestBytes.count, &signedData, &signedLength)
        
        if result == errSecSuccess {
            return Data(signedData[0..<signedLength])
        } else {
            print("Error signing data: \(result)")
            return nil
        }
    }
    
    // Verify signature with RSA public key
    func verifySignature(signature: Data, forData data: Data, withPublicKey publicKeyData: Data) -> Bool {
        var error: Unmanaged<CFError>?
        let publicKey = SecKeyCreateWithData(publicKeyData as CFData, [
            kSecAttrKeyType: kSecAttrKeyTypeRSA,
            kSecAttrKeyClass: kSecAttrKeyClassPublic,
            kSecAttrKeySizeInBits: 2048
        ] as CFDictionary, &error)
        
        guard let key = publicKey else {
            print("Error creating public key: \(error?.takeRetainedValue() as Error? ?? NSError(domain: "EncryptionManager", code: -1, userInfo: nil))")
            return false
        }
        
        let digest = SHA256.hash(data: data)
        let digestBytes = Array(digest)
        let signatureBytes = Array(signature)
        
        let result = SecKeyRawVerify(key, .PKCS1SHA256, digestBytes, digestBytes.count, signatureBytes, signatureBytes.count)
        
        return result == errSecSuccess
    }
    
    // Generate a secure random string
    func generateSecureRandomString(length: Int) -> String {
        let letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        var randomString = ""
        
        for _ in 0..<length {
            let randomIndex = Int.random(in: 0..<letters.count)
            let index = letters.index(letters.startIndex, offsetBy: randomIndex)
            randomString.append(letters[index])
        }
        
        return randomString
    }
}