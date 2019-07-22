#import <Foundation/Foundation.h>

/// Wrapper layer to simplify calling into Envoy's C++ API.
@interface EnvoyEngine : NSObject

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (int)runWithConfig:(NSString *)config;

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (int)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

/// Performs necessary setup after Envoy has initialized and started running.
/// TODO: create a post-initialization callback from Envoy to handle this automatically.
+ (void)setupEnvoy;

@end
