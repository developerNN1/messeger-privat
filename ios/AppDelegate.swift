import UIKit
import React
import UserNotifications

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
    
    var window: UIWindow?
    var bridge: RCTBridge?

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Initialize security manager
        if SecurityManager.shared.isAppTampered() {
            // App has been tampered with, don't proceed
            let alert = UIAlertController(title: "Security Alert", message: "This application has been modified and is not secure.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "Exit", style: .default) { _ in
                exit(0)
            })
            
            window = UIWindow(frame: UIScreen.main.bounds)
            window?.rootViewController = UIViewController()
            window?.makeKeyAndVisible()
            window?.rootViewController?.present(alert, animated: true)
            
            return false
        }
        
        // Start Tor in the background
        DispatchQueue.global(qos: .background).async {
            TorManager.shared.startTor { success, error in
                if !success {
                    DispatchQueue.main.async {
                        let alert = UIAlertController(
                            title: "Tor Connection Failed",
                            message: error?.localizedDescription ?? "Could not connect to Tor network",
                            preferredStyle: .alert
                        )
                        alert.addAction(UIAlertAction(title: "OK", style: .default))
                        self.window?.rootViewController?.present(alert, animated: true)
                    }
                }
            }
        }
        
        // Initialize React Native
        initializeReactNative()
        
        return true
    }
    
    func applicationWillTerminate(_ application: UIApplication) {
        // Stop Tor when app terminates
        TorManager.shared.stopTor()
    }

    func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }

    private func initializeReactNative() {
        let jsCodeLocation = RCTBundleURLProvider.sharedSettings().jsBundleURL(forBundleRoot: "index", fallbackResource: nil)
        
        let rootView = RCTRootView(
            bundleURL: jsCodeLocation,
            moduleName: "AnonymousMessage",
            initialProperties: [:],
            launchOptions: [:]
        )
        
        rootView.backgroundColor = UIColor.systemBackground
        
        let viewController = UIViewController()
        viewController.view = rootView
        
        window = UIWindow(frame: UIScreen.main.bounds)
        window?.rootViewController = viewController
        window?.makeKeyAndVisible()
    }
    
    // Handle URL schemes for deep linking
    func application(_ app: UIApplication, open url: URL, options: [UIApplication.OpenURLOptionsKey : Any] = [:]) -> Bool {
        // Handle custom URL schemes
        return true
    }
    
    // Handle universal links
    func application(_ application: UIApplication, continue userActivity: NSUserActivity, restorationHandler: @escaping ([UIUserActivityRestoring]?) -> Void) -> Bool {
        // Handle universal links
        return true
    }
    
    // Remote notification handling
    func application(_ application: UIApplication, didRegisterForRemoteNotificationsWithDeviceToken deviceToken: Data) {
        // Handle device token registration
    }
    
    func application(_ application: UIApplication, didFailToRegisterForRemoteNotificationsWithError error: Error) {
        // Handle registration failure
    }
    
    func application(_ application: UIApplication, didReceiveRemoteNotification userInfo: [AnyHashable : Any], fetchCompletionHandler completionHandler: @escaping (UIBackgroundFetchResult) -> Void) {
        // Handle remote notifications
        completionHandler(.newData)
    }
}

// MARK: - Scene Delegate for iOS 13+
@available(iOS 13.0, *)
class SceneDelegate: UIResponder, UIWindowSceneDelegate {
    var window: UIWindow?

    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options connectionOptions: UIScene.ConnectionOptions) {
        guard let _ = (scene as? UIWindowScene) else { return }
    }

    func sceneDidDisconnect(_ scene: UIScene) {}

    func sceneDidBecomeActive(_ scene: UIScene) {}

    func sceneWillResignActive(_ scene: UIScene) {}

    func sceneWillEnterForeground(_ scene: UIScene) {}

    func sceneDidEnterBackground(_ scene: UIScene) {}
}