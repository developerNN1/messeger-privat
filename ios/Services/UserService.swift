import Foundation

class UserService {
    static let shared = UserService()
    private init() {}
    
    struct User: Codable {
        let id: String
        let username: String
        let email: String
        let publicKey: String
        let avatar: String?
        let createdAt: Date
        
        enum CodingKeys: String, CodingKey {
            case id, username, email, publicKey, avatar
            case createdAt = "created_at"
        }
    }
    
    private var currentUser: User?
    
    func getCurrentUser() -> User? {
        return currentUser
    }
    
    func setCurrentUser(_ user: User) {
        currentUser = user
    }
    
    func isLoggedIn() -> Bool {
        return currentUser != nil
    }
    
    func logout() {
        currentUser = nil
    }
    
    // Register a new user
    func register(username: String, email: String, password: String, completion: @escaping (Result<User, Error>) -> Void) {
        // Generate user keys
        guard let keyPair = EncryptionManager.shared.generateRSAKeyPair() else {
            completion(.failure(NSError(domain: "UserService", code: -1, userInfo: [NSLocalizedDescriptionKey: "Failed to generate keys"])))
            return
        }
        
        // Hash the password
        let (hashedPassword, salt) = EncryptionManager.shared.hashPassword(password)
        
        // Prepare registration data
        let registrationData = [
            "username": username,
            "email": email,
            "password_hash": hashedPassword.base64EncodedString(),
            "salt": salt.base64EncodedString(),
            "public_key": keyPair.publicKey.base64EncodedString(),
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        // Encrypt the registration data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: registrationData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/register", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "UserService", code: -2, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let userData = responseJson?["user"] as? [String: Any],
                       let userId = userData["id"] as? String {
                        
                        let user = User(
                            id: userId,
                            username: userData["username"] as? String ?? username,
                            email: userData["email"] as? String ?? email,
                            publicKey: userData["public_key"] as? String ?? keyPair.publicKey.base64EncodedString(),
                            avatar: userData["avatar"] as? String,
                            createdAt: Date()
                        )
                        
                        self.setCurrentUser(user)
                        completion(.success(user))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Registration failed"
                        completion(.failure(NSError(domain: "UserService", code: -3, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Login user
    func login(email: String, password: String, completion: @escaping (Result<User, Error>) -> Void) {
        // Hash the password
        let (hashedPassword, salt) = EncryptionManager.shared.hashPassword(password)
        
        // Prepare login data
        let loginData = [
            "email": email,
            "password_hash": hashedPassword.base64EncodedString(),
            "salt": salt.base64EncodedString(),
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        // Encrypt the login data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: loginData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/login", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "UserService", code: -2, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let userData = responseJson?["user"] as? [String: Any],
                       let userId = userData["id"] as? String {
                        
                        let user = User(
                            id: userId,
                            username: userData["username"] as? String ?? "",
                            email: userData["email"] as? String ?? email,
                            publicKey: userData["public_key"] as? String ?? "",
                            avatar: userData["avatar"] as? String,
                            createdAt: Date()
                        )
                        
                        self.setCurrentUser(user)
                        completion(.success(user))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Login failed"
                        completion(.failure(NSError(domain: "UserService", code: -3, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Update user profile
    func updateProfile(avatarData: Data?, completion: @escaping (Result<User, Error>) -> Void) {
        guard let currentUser = getCurrentUser() else {
            completion(.failure(NSError(domain: "UserService", code: -1, userInfo: [NSLocalizedDescriptionKey: "User not logged in"])))
            return
        }
        
        // Prepare update data
        var updateData: [String: Any] = [
            "user_id": currentUser.id,
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        if let avatarData = avatarData {
            updateData["avatar"] = avatarData.base64EncodedString()
        }
        
        // Encrypt the update data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: updateData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/update_profile", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "UserService", code: -2, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let userData = responseJson?["user"] as? [String: Any],
                       let userId = userData["id"] as? String {
                        
                        let updatedUser = User(
                            id: userId,
                            username: userData["username"] as? String ?? currentUser.username,
                            email: userData["email"] as? String ?? currentUser.email,
                            publicKey: userData["public_key"] as? String ?? currentUser.publicKey,
                            avatar: userData["avatar"] as? String ?? currentUser.avatar,
                            createdAt: currentUser.createdAt
                        )
                        
                        self.setCurrentUser(updatedUser)
                        completion(.success(updatedUser))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Update profile failed"
                        completion(.failure(NSError(domain: "UserService", code: -3, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
}