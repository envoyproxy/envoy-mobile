#import "EnvoyTypes.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface EnvoyHttpStream

/**
 Open an underlying HTTP stream.

 @param observer the observer that will run the stream callbacks.
 */
- (EnvoyStream)startStreamWithObserver:(EnvoyObserver *)observer;

/**
 Send headers over the provided stream.

 @param metadata Headers to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendHeaders:(EnvoyHeaders *)headers close:(BOOL)close;

/**
 Send data over the provided stream.

 @param metadata Data to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendData:(NSData *)data close:(BOOL)close;

/**
 Send metadata over the provided stream.

 @param metadata Metadata to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendMetadata:(EnvoyHeaders *)metadata close:(BOOL)close;

/**
 Send trailers over the provided stream.

 @param trailers Trailers to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendTrailers:(EnvoyHeaders *)trailers close:(BOOL)close;

/**
 Cancel the stream. This functions as an interrupt, and aborts further callbacks and handling of the stream.
 @return Success, unless the stream has already been canceled.
 */
- (EnvoyStatus)cancel;

@end

/// Wrapper layer for calling into Envoy's C/++ API.
@interface EnvoyEngine : NSObject <EnvoyEngineStreamInterface>

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @return A status indicating if the action was successful.
 */
+ (envoy_engine_t)runWithConfig:(NSString *)config;

/**
 Run the Envoy engine with the provided config and log level.

 @param config The configuration file with which to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @return A status indicating if the action was successful.
 */
+ (envoy_engine_t)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

/// Performs necessary setup after Envoy has initialized and started running.
/// TODO: create a post-initialization callback from Envoy to handle this automatically.
+ (void)setupEnvoy;

@end

NS_ASSUME_NONNULL_END
