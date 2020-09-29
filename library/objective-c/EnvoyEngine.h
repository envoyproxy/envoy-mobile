#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Aliases

/// A set of headers that may be passed to/from an Envoy stream.
typedef NSDictionary<NSString *, NSArray<NSString *> *> EnvoyHeaders;

#pragma mark - EnvoyHTTPCallbacks

/// Interface that can handle callbacks from an HTTP stream.
@interface EnvoyHTTPCallbacks : NSObject

/**
 * Dispatch queue provided to handle callbacks.
 */
@property (nonatomic, assign) dispatch_queue_t dispatchQueue;

/**
 * Called when all headers get received on the async HTTP stream.
 * @param headers the headers received.
 * @param endStream whether the response is headers-only.
 */
@property (nonatomic, copy) void (^onHeaders)(EnvoyHeaders *headers, BOOL endStream);

/**
 * Called when a data frame gets received on the async HTTP stream.
 * This callback can be invoked multiple times if the data gets streamed.
 * @param data the data received.
 * @param endStream whether the data is the last data frame.
 */
@property (nonatomic, copy) void (^onData)(NSData *data, BOOL endStream);

/**
 * Called when all trailers get received on the async HTTP stream.
 * Note that end stream is implied when on_trailers is called.
 * @param trailers the trailers received.
 */
@property (nonatomic, copy) void (^onTrailers)(EnvoyHeaders *trailers);

/**
 * Called when the async HTTP stream has an error.
 */
@property (nonatomic, copy) void (^onError)
    (uint64_t errorCode, NSString *message, int32_t attemptCount);

/**
 * Called when the async HTTP stream is canceled.
 * Note this callback will ALWAYS be fired if a stream is canceled, even if the request and/or
 * response is already complete. It will fire no more than once, and no other callbacks for the
 * stream will be issued afterwards.
 */
@property (nonatomic, copy) void (^onCancel)(void);

@end

#pragma mark - EnvoyHTTPFilter

/// Return codes for on-headers filter invocations. @see envoy/http/filter.h
extern const int kEnvoyFilterHeadersStatusContinue;
extern const int kEnvoyFilterHeadersStatusStopIteration;
extern const int kEnvoyFilterHeadersStatusContinueAndEndStream;
extern const int kEnvoyFilterHeadersStatusStopAllIterationAndBuffer;

/// Return codes for on-data filter invocations. @see envoy/http/filter.h
extern const int kEnvoyFilterDataStatusContinue;
extern const int kEnvoyFilterDataStatusStopIterationAndBuffer;
extern const int kEnvoyFilterDataStatusStopIterationNoBuffer;
extern const int kEnvoyFilterDataStatusResumeIteration;

/// Return codes for on-trailers filter invocations. @see envoy/http/filter.h
extern const int kEnvoyFilterTrailersStatusContinue;
extern const int kEnvoyFilterTrailersStatusStopIteration;
extern const int kEnvoyFilterTrailersStatusResumeIteration;

@interface EnvoyHTTPFilter : NSObject

@property (nonatomic, copy) NSArray * (^onRequestHeaders)(EnvoyHeaders *headers, BOOL endStream);

@property (nonatomic, copy) NSArray * (^onRequestData)(NSData *data, BOOL endStream);

@property (nonatomic, copy) NSArray * (^onRequestTrailers)(EnvoyHeaders *trailers);

@property (nonatomic, copy) NSArray * (^onResponseHeaders)(EnvoyHeaders *headers, BOOL endStream);

@property (nonatomic, copy) NSArray * (^onResponseData)(NSData *data, BOOL endStream);

@property (nonatomic, copy) NSArray * (^onResponseTrailers)(EnvoyHeaders *trailers);

@end

#pragma mark - EnvoyHTTPFilterFactory

@interface EnvoyHTTPFilterFactory : NSObject

@property (nonatomic, strong) NSString *filterName;

@property (nonatomic, copy) EnvoyHTTPFilter * (^create)();

@end

#pragma mark - EnvoyHTTPStream

@protocol EnvoyHTTPStream

/**
 Open an underlying HTTP stream.

 @param handle Underlying handle of the HTTP stream owned by an Envoy engine.
 @param callbacks The callbacks for the stream.
 */
- (instancetype)initWithHandle:(intptr_t)handle callbacks:(EnvoyHTTPCallbacks *)callbacks;

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
 Send trailers over the provided stream.

 @param trailers Trailers to send over the stream.
 */
- (void)sendTrailers:(EnvoyHeaders *)trailers;

/**
 Cancel the stream. This functions as an interrupt, and aborts further callbacks and handling of the
 stream.

 @return Success unless the stream has already been canceled.
 */
- (int)cancel;

/**
 Clean up the stream after it's closed (by completion, cancellation, or error).
 */
- (void)cleanUp;

@end

#pragma mark - EnvoyHTTPStreamImpl

// Concrete implementation of the `EnvoyHTTPStream` protocol.
@interface EnvoyHTTPStreamImpl : NSObject <EnvoyHTTPStream>

@end

#pragma mark - EnvoyConfiguration

/// Typed configuration that may be used for starting Envoy.
@interface EnvoyConfiguration : NSObject

@property (nonatomic, strong) NSString *statsDomain;
@property (nonatomic, assign) UInt32 connectTimeoutSeconds;
@property (nonatomic, assign) UInt32 dnsRefreshSeconds;
@property (nonatomic, assign) UInt32 dnsFailureRefreshSecondsBase;
@property (nonatomic, assign) UInt32 dnsFailureRefreshSecondsMax;
@property (nonatomic, strong) NSArray<EnvoyHTTPFilterFactory *> *httpFilterFactories;
@property (nonatomic, assign) UInt32 statsFlushSeconds;
@property (nonatomic, strong) NSString *appVersion;
@property (nonatomic, strong) NSString *appId;
@property (nonatomic, strong) NSString *virtualClusters;

/**
 Create a new instance of the configuration.
 */
- (instancetype)initWithStatsDomain:(NSString *)statsDomain
              connectTimeoutSeconds:(UInt32)connectTimeoutSeconds
                  dnsRefreshSeconds:(UInt32)dnsRefreshSeconds
       dnsFailureRefreshSecondsBase:(UInt32)dnsFailureRefreshSecondsBase
        dnsFailureRefreshSecondsMax:(UInt32)dnsFailureRefreshSecondsMax
                        filterChain:(NSArray<EnvoyHTTPFilterFactory *> *)httpFilterFactories
                  statsFlushSeconds:(UInt32)statsFlushSeconds
                         appVersion:(NSString *)appVersion
                              appId:(NSString *)appId
                    virtualClusters:(NSString *)virtualClusters;

/**
 Resolves the provided configuration template using properties on this configuration.

 @param templateYAML The template configuration to resolve.
 @return The resolved template. Nil if the template fails to fully resolve.
 */
- (nullable NSString *)resolveTemplate:(NSString *)templateYAML;

@end

#pragma mark - EnvoyEngine

/// Return codes for Engine interface. @see /library/common/types/c_types.h
extern const int kEnvoySuccess;
extern const int kEnvoyFailure;

/// Wrapper layer for calling into Envoy's C/++ API.
@protocol EnvoyEngine

/**
 Create a new instance of the engine.
 */
- (instancetype)init;

/**
 Run the Envoy engine with the provided configuration and log level.

 @param config The EnvoyConfiguration used to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @param onEngineRunning Closure called when the engine finishes its async startup and begins
 running.
 @return A status indicating if the action was successful.
 */
- (int)runWithConfig:(EnvoyConfiguration *)config
            logLevel:(NSString *)logLevel
     onEngineRunning:(nullable void (^)())onEngineRunning;

/**
 Run the Envoy engine with the provided yaml string and log level.

 @param configYAML The configuration yaml with which to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @param onEngineRunning Closure called when the engine finishes its async startup and begins
 running.
 @return A status indicating if the action was successful.
 */
- (int)runWithConfigYAML:(NSString *)configYAML
                logLevel:(NSString *)logLevel
         onEngineRunning:(nullable void (^)())onEngineRunning;

/**
 Opens a new HTTP stream attached to this engine.

 @param callbacks Handler for observing stream events.
 */
- (id<EnvoyHTTPStream>)startStreamWithCallbacks:(EnvoyHTTPCallbacks *)callbacks;

/**
 Increments a counter with the given count.

 @param elements Elements of the counter stat.
 @param count Amount to add to the counter.
 @return A status indicating if the action was successful.
 */
- (int)recordCounter:(NSString *)elements count:(NSUInteger)count;

@end

#pragma mark - EnvoyEngineImpl

// Concrete implementation of the `EnvoyEngine` interface.
@interface EnvoyEngineImpl : NSObject <EnvoyEngine>

@property (nonatomic, copy, nullable) void (^onEngineRunning)();

@end

#pragma mark - EnvoyNetworkMonitor

// Monitors network changes in order to update Envoy network cluster preferences.
@interface EnvoyNetworkMonitor : NSObject

// Start monitoring reachability, updating the preferred Envoy network cluster on changes.
// This is typically called by `EnvoyEngine` automatically on startup.
+ (void)startReachabilityIfNeeded;

@end

NS_ASSUME_NONNULL_END
