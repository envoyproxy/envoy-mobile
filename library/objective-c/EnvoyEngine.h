#import <Foundation/Foundation.h>

/// Wrapper layer to simplify calling into Envoy's C++ API.
@interface EnvoyEngine : NSObject

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (int)runWithConfig:(NSString *)config;

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (int)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

typedef NSArray<NSDictionary<NSString *, NSString *> *> EnvoyHeaders;

typedef void (^OnHeadersClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnDataClosure)(NSData *, BOOL);

typedef void (^OnTrailersClosure)(EnvoyHeaders *, BOOL);

@end



typedef enum {} EnvoyErrorCode;

@interface EnvoyError: NSError

@property (nonatomic, strong) NSString *message;

@property (nonatomic, assign) EnvoyErrorCode error_code;

@end
