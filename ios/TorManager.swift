import Foundation
import Tor

class TorManager: NSObject {
    private var torController: TorController?
    private var torThread: TorThread?
    
    static let shared = TorManager()
    private override init() {}
    
    enum TorStatus {
        case disconnected
        case connecting
        case connected
        case failed
    }
    
    var status: TorStatus = .disconnected
    
    func startTor(completion: @escaping (Bool, String?) -> Void) {
        guard status == .disconnected else {
            completion(false, "Tor is already running or connecting")
            return
        }
        
        status = .connecting
        
        // Configure Tor
        let configuration = TorConfiguration()
        configuration.cookieAuthentication = false
        configuration.arguments = [
            "--allow-missing-torrc",
            "--ignore-missing-torrc",
            "--client-only", "1",
            "--socksport", "39050",
            "--controlport", "39051",
            "--dnsport", "39052",
            "--transocksport", "39053",
            "--ClientTransportPlugin", "obfs4 exec ./obfs4proxy",
            "--UseBridges", "1",
            "--Bridge", "obfs4 192.0.2.1:443 2C8017E8EBF4B0640AE4B4F2AEA8E7E8ECDDCD1F cert=NfvQfMkW7LXhHFOtvUq9VS8tKUjynlJCKG0qzE2ieTz0QwN57dOwz8QoQcHqpdB0D9xOCQ iat-mode=0"
        ]
        
        torThread = TorThread(configuration: configuration)
        torThread?.delegate = self
        torThread?.start()
        
        // Initialize controller
        torController = TorController(socketHost: "127.0.0.1", port: 39051)
        
        // Wait for Tor to boot
        DispatchQueue.global(qos: .background).async {
            var attempts = 0
            let maxAttempts = 60  // Wait up to 60 seconds
            
            while attempts < maxAttempts {
                if self.torController?.isConnected ?? false {
                    do {
                        try self.torController?.authenticate()
                        
                        // Wait for bootstrap to complete
                        _ = self.torController?.bootstrap(completionHandler: { (status, error) in
                            if status == .done {
                                DispatchQueue.main.async {
                                    self.status = .connected
                                    completion(true, nil)
                                }
                            } else if let error = error {
                                DispatchQueue.main.async {
                                    self.status = .failed
                                    completion(false, error.localizedDescription)
                                }
                            }
                        })
                        
                        break
                    } catch {
                        DispatchQueue.main.async {
                            self.status = .failed
                            completion(false, error.localizedDescription)
                        }
                        break
                    }
                }
                
                usleep(1000000)  // Sleep for 1 second
                attempts += 1
            }
            
            if attempts >= maxAttempts {
                DispatchQueue.main.async {
                    self.status = .failed
                    completion(false, "Tor connection timeout")
                }
            }
        }
    }
    
    func stopTor() {
        torController?.disconnect()
        torThread?.stop()
        status = .disconnected
    }
    
    func isConnected() -> Bool {
        return status == .connected
    }
    
    func getSOCKSHost() -> String {
        return "127.0.0.1"
    }
    
    func getSOCKSPort() -> UInt16 {
        return 39050
    }
    
    func sendRequest(method: String, url: String, body: Data? = nil, headers: [String: String]? = nil, completion: @escaping (Data?, URLResponse?, Error?) -> Void) {
        guard isConnected() else {
            completion(nil, nil, NSError(domain: "TorManager", code: -1, userInfo: [NSLocalizedDescriptionKey: "Tor is not connected"]))
            return
        }
        
        // Create a URLSessionConfiguration with SOCKS proxy
        let config = URLSessionConfiguration.default
        config.connectionProxyDictionary = [
            "SOCKSProxy": getSOCKSHost(),
            "SOCKSPort": getSOCKSPort()
        ]
        
        let session = URLSession(configuration: config)
        
        guard let requestUrl = URL(string: url) else {
            completion(nil, nil, NSError(domain: "TorManager", code: -2, userInfo: [NSLocalizedDescriptionKey: "Invalid URL"]))
            return
        }
        
        var request = URLRequest(url: requestUrl)
        request.httpMethod = method
        request.httpBody = body
        
        // Add headers
        if let headers = headers {
            for (key, value) in headers {
                request.setValue(value, forHTTPHeaderField: key)
            }
        }
        
        // Add custom headers for anonymity
        request.setValue("AnonymousMessage iOS", forHTTPHeaderField: "User-Agent")
        request.setValue("no-referrer", forHTTPHeaderField: "Referrer-Policy")
        
        let task = session.dataTask(with: request) { data, response, error in
            completion(data, response, error)
        }
        
        task.resume()
    }
}

extension TorManager: TorThreadDelegate {
    func torDidFinishStarting(_ torProcess: TorProcess) {
        print("Tor did finish starting")
    }
    
    func torThreadDidInitialize(thread: TorThread!) {
        print("Tor thread initialized")
    }
    
    func torThread(thread: TorThread!, didFailWithError error: Error!) {
        print("Tor thread failed with error: \(error?.localizedDescription ?? "Unknown error")")
        status = .failed
    }
}