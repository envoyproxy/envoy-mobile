uimport <Foundation/Foundation.h>

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
- (instancetype)initWithHandle:(uint64_t)handle observer:(EnvoyObserver *)observer;

/**
 Send headers over the provided stream.

 @param headers Headers to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendHeaders:(EnvoyHeaders *)headers close:(BOOL)close;

/**
 Send data over the provided stream.

 @param data Data to send over the stream.
 @param close True if the stream should be closed after sending.
 */
- (void)sendData:(NSData *)data close:(BOOL)close;

/**
 Send metadata over the provided stream.

 @param metadata Metadata to send over the stream.
 */
- (void)sendMetadata:(EnvoyHeaders *)metadata;

/**
 Send trailers over the provided stream.

 @param trailers Trailers to send over the stream.
 */
- (void)sendTrailers:(EnvoyHeaders *)trailers;

/**
 Cancel the stream. This functions as an interrupt, and aborts further callbacks and handling of the
 stream.
 @return Success, unless the stream has already been canceled.
>>>>>>> master
 */
- (EnvoyStream *)startStreamWithObserver:(EnvoyObserver *)observer;

@end

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
- (void)setup;

/**
 Opens a new HTTP stream attached to this engine.

 @param observer Handler for stream events.
 */
- (EnvoyHttpStream *)openHttpStreamWithObserver:(EnvoyObserver *)observer;

@end

// Concrete implementation of the `EnvoyEngine` protocol.
@interface EnvoyEngineImpl : NSObject <EnvoyEngine>

@end

NS_ASSUME_NONNULL_END
