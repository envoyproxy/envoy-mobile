#import <Foundation/Foundation.h>

#import "library/objective-c/EnvoyObserver.h"
#import "library/objective-c/EnvoyStream.h"

NS_ASSUME_NONNULL_BEGIN

/// Wrapper layer for calling into Envoy's C/++ API.
@protocol EnvoyEngine

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @return A status indicating if the action was successful.
 */
- (int)runWithConfig:(NSString *)config;

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @return A status indicating if the action was successful.
 */
- (int)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

/// Performs necessary setup after Envoy has initialized and started running.
/// TODO: create a post-initialization callback from Envoy to handle this automatically.
- (void)setupEnvoy;

/**
 Creates a new stream with the provided observer.

 @param observer The observer for receiving callbacks from the stream.
 @return A stream that may be used for sending data.
 */
- (EnvoyStream *)startStreamWithObserver:(EnvoyObserver *)observer;

@end

NS_ASSUME_NONNULL_END
