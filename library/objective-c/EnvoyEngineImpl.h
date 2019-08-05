#import "EnvoyEngine.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Wrapper layer for calling into Envoy's C/++ API.
@interface EnvoyEngineImpl : NSObject <EnvoyEngine>

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @return A status indicating if the action was successful.
 */
+ (EnvoyStatus)runWithConfig:(NSString *)config;

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @return A status indicating if the action was successful.
 */
+ (EnvoyStatus)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

/// Performs necessary setup after Envoy has initialized and started running.
/// TODO: create a post-initialization callback from Envoy to handle this automatically.
+ (void)setupEnvoy;

@end

NS_ASSUME_NONNULL_END
