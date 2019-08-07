#import "library/objective-c/EnvoyTypes.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Wrapper layer for calling into Envoy's C/++ API.
@protocol EnvoyEngine

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

@end

NS_ASSUME_NONNULL_END
