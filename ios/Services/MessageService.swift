import Foundation

class MessageService {
    static let shared = MessageService()
    private init() {}
    
    struct Message: Codable {
        let id: String
        let text: String
        let senderId: String
        let recipientId: String?
        let groupId: String?
        let timestamp: Date
        let type: MessageType
        let encryptedContent: String
        let signature: String?
        
        enum MessageType: String, Codable {
            case text
            case image
            case voice
            case location
            case file
        }
        
        enum CodingKeys: String, CodingKey {
            case id, text, senderId, recipientId, groupId, timestamp, type
            case encryptedContent = "encrypted_content"
            case signature
        }
    }
    
    struct Group: Codable {
        let id: String
        let name: String
        let members: [String] // Array of user IDs
        let createdAt: Date
        let createdBy: String
    }
    
    // Send a message through Tor
    func sendMessage(_ message: Message, completion: @escaping (Result<Message, Error>) -> Void) {
        // Prepare message data
        let messageData = [
            "id": message.id,
            "text": message.text,
            "sender_id": message.senderId,
            "recipient_id": message.recipientId as Any,
            "group_id": message.groupId as Any,
            "timestamp": Int64(message.timestamp.timeIntervalSince1970 * 1000),
            "type": message.type.rawValue,
            "encrypted_content": message.encryptedContent,
            "signature": message.signature as Any
        ]
        
        // Encrypt the message data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: messageData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/send_message", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let messageData = responseJson?["message"] as? [String: Any],
                       let messageId = messageData["id"] as? String {
                        
                        let sentMessage = Message(
                            id: messageId,
                            text: messageData["text"] as? String ?? message.text,
                            senderId: messageData["sender_id"] as? String ?? message.senderId,
                            recipientId: messageData["recipient_id"] as? String,
                            groupId: messageData["group_id"] as? String,
                            timestamp: Date(timeIntervalSince1970: TimeInterval((messageData["timestamp"] as? Int64 ?? Int64(Date().timeIntervalSince1970 * 1000)) / 1000)),
                            type: MessageType(rawValue: messageData["type"] as? String ?? "text") ?? .text,
                            encryptedContent: messageData["encrypted_content"] as? String ?? message.encryptedContent,
                            signature: messageData["signature"] as? String ?? message.signature
                        )
                        
                        completion(.success(sentMessage))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Send message failed"
                        completion(.failure(NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Fetch messages for a user through Tor
    func fetchMessages(forUserId userId: String, completion: @escaping (Result<[Message], Error>) -> Void) {
        // Prepare request data
        let requestData = [
            "user_id": userId,
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        // Encrypt the request data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: requestData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/fetch_messages", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let messagesData = responseJson?["messages"] as? [[String: Any]] {
                        
                        var messages: [Message] = []
                        
                        for messageData in messagesData {
                            if let messageId = messageData["id"] as? String,
                               let senderId = messageData["sender_id"] as? String,
                               let text = messageData["text"] as? String,
                               let encryptedContent = messageData["encrypted_content"] as? String {
                                
                                let message = Message(
                                    id: messageId,
                                    text: text,
                                    senderId: senderId,
                                    recipientId: messageData["recipient_id"] as? String,
                                    groupId: messageData["group_id"] as? String,
                                    timestamp: Date(timeIntervalSince1970: TimeInterval((messageData["timestamp"] as? Int64 ?? Int64(Date().timeIntervalSince1970 * 1000)) / 1000)),
                                    type: MessageType(rawValue: messageData["type"] as? String ?? "text") ?? .text,
                                    encryptedContent: encryptedContent,
                                    signature: messageData["signature"] as? String
                                )
                                
                                messages.append(message)
                            }
                        }
                        
                        completion(.success(messages))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Fetch messages failed"
                        completion(.failure(NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Create a group through Tor
    func createGroup(name: String, members: [String], completion: @escaping (Result<Group, Error>) -> Void) {
        // Prepare group data
        let groupData = [
            "name": name,
            "members": members,
            "created_by": UserService.shared.getCurrentUser()?.id ?? "",
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        // Encrypt the group data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: groupData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/create_group", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success,
                       let groupInfo = responseJson?["group"] as? [String: Any],
                       let groupId = groupInfo["id"] as? String,
                       let groupName = groupInfo["name"] as? String,
                       let groupMembers = groupInfo["members"] as? [String],
                       let createdBy = groupInfo["created_by"] as? String {
                        
                        let group = Group(
                            id: groupId,
                            name: groupName,
                            members: groupMembers,
                            createdAt: Date(timeIntervalSince1970: TimeInterval((groupInfo["created_at"] as? Int64 ?? Int64(Date().timeIntervalSince1970 * 1000)) / 1000)),
                            createdBy: createdBy
                        )
                        
                        completion(.success(group))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Create group failed"
                        completion(.failure(NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Invite user to group through Tor
    func inviteToGroup(groupId: String, userId: String, completion: @escaping (Result<Bool, Error>) -> Void) {
        // Prepare invitation data
        let invitationData = [
            "group_id": groupId,
            "user_id": userId,
            "inviter_id": UserService.shared.getCurrentUser()?.id ?? "",
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]
        
        // Encrypt the invitation data
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: invitationData)
            let encryptedData = try EncryptionManager.shared.encrypt(data: jsonData, withKey: EncryptionManager.shared.generateAESKey())
            
            // Send through Tor
            TorManager.shared.sendRequest(method: "POST", url: "http://anonymousmsg.onion/api/invite_to_group", body: encryptedData, headers: [
                "Content-Type": "application/octet-stream"
            ]) { data, response, error in
                if let error = error {
                    completion(.failure(error))
                    return
                }
                
                guard let responseData = data else {
                    completion(.failure(NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "No response data"])))
                    return
                }
                
                do {
                    // Decrypt the response
                    let decryptedData = try EncryptionManager.shared.decrypt(data: responseData, withKey: EncryptionManager.shared.generateAESKey())
                    
                    // Parse the response
                    let responseJson = try JSONSerialization.jsonObject(with: decryptedData, options: []) as? [String: Any]
                    
                    if let success = responseJson?["success"] as? Bool, success {
                        completion(.success(true))
                    } else {
                        let errorMessage = responseJson?["message"] as? String ?? "Invite to group failed"
                        completion(.failure(NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: errorMessage])))
                    }
                } catch {
                    completion(.failure(error))
                }
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    // Encrypt message content using recipient's public key
    func encryptMessageContent(_ content: String, forRecipientPublicKey recipientPublicKey: String) throws -> String {
        guard let publicKeyData = Data(base64Encoded: recipientPublicKey) else {
            throw NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "Invalid public key"])
        }
        
        guard let encryptedData = EncryptionManager.shared.rsaEncrypt(data: content.data(using: .utf8)!, withPublicKey: publicKeyData) else {
            throw NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: "Failed to encrypt message"])
        }
        
        return encryptedData.base64EncodedString()
    }
    
    // Decrypt message content using user's private key
    func decryptMessageContent(_ encryptedContent: String, withPrivateKey privateKey: String) throws -> String {
        guard let encryptedData = Data(base64Encoded: encryptedContent) else {
            throw NSError(domain: "MessageService", code: -1, userInfo: [NSLocalizedDescriptionKey: "Invalid encrypted content"])
        }
        
        guard let privateKeyData = Data(base64Encoded: privateKey) else {
            throw NSError(domain: "MessageService", code: -2, userInfo: [NSLocalizedDescriptionKey: "Invalid private key"])
        }
        
        guard let decryptedData = EncryptionManager.shared.rsaDecrypt(data: encryptedData, withPrivateKey: privateKeyData) else {
            throw NSError(domain: "MessageService", code: -3, userInfo: [NSLocalizedDescriptionKey: "Failed to decrypt message"])
        }
        
        return String(data: decryptedData, encoding: .utf8) ?? ""
    }
}