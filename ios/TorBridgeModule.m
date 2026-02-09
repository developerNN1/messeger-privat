#import <React/RCTBridgeModule.h>
#import <React/RCTEventEmitter.h>

@interface RCT_EXTERN_MODULE(TorBridge, NSObject)

RCT_EXTERN_METHOD(isTorConnected:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(startTor:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(sendThroughTor:(NSString *)data endpoint:(NSString *)endpoint callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(hashPassword:(NSString *)password callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(generateUserKeys:(NSString *)username callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(encryptData:(NSDictionary *)data callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(decryptData:(NSString *)encryptedDataBase64 callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(storeInKeychain:(NSString *)key data:(NSString *)data callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(retrieveFromKeychain:(NSString *)key callback:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(isDeviceJailbroken:(RCTResponseSenderBlock)callback)
RCT_EXTERN_METHOD(isAppTampered:(RCTResponseSenderBlock)callback)

@end