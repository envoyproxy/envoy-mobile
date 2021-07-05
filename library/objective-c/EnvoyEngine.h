#import <Foundation/Foundation.h>

#import "library/common/types/c_types.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Aliases

/// A set of headers that may be passed to/from an Envoy stream.
typedef NSDictionary<NSString *, NSArray<NSString *> *> EnvoyHeaders;

typedef NSDictionary<NSString *, NSString *> EnvoyTags;

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

/// Return codes for on-resume filter invocations. These are unique to platform filters,
/// and used exclusively after an asynchronous request to resume iteration via callbacks.
extern const int kEnvoyFilterResumeStatusStopIteration;
extern const int kEnvoyFilterResumeStatusResumeIteration;

/// Callbacks for asynchronous interaction with the filter.
@protocol EnvoyHTTPFilterCallbacks

/// Resume filter iteration asynchronously. This will result in an on-resume invocation of the
/// filter.
- (void)resumeIteration;

@end

@interface EnvoyHTTPFilter : NSObject

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, forward headers
@property (nonatomic, copy) NSArray * (^onRequestHeaders)(EnvoyHeaders *headers, BOOL endStream);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - NSData *, forward data
/// 2 - EnvoyHeaders *, optional pending headers
@property (nonatomic, copy) NSArray * (^onRequestData)(NSData *data, BOOL endStream);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, forward trailers
/// 2 - EnvoyHeaders *, optional pending headers
/// 3 - NSData *, optional pending data
@property (nonatomic, copy) NSArray * (^onRequestTrailers)(EnvoyHeaders *trailers);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, forward headers
@property (nonatomic, copy) NSArray * (^onResponseHeaders)(EnvoyHeaders *headers, BOOL endStream);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - NSData *, forward data
/// 2 - EnvoyHeaders *, optional pending headers
@property (nonatomic, copy) NSArray * (^onResponseData)(NSData *data, BOOL endStream);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, forward trailers
/// 2 - EnvoyHeaders *, optional pending headers
/// 3 - NSData *, optional pending data
@property (nonatomic, copy) NSArray * (^onResponseTrailers)(EnvoyHeaders *trailers);

@property (nonatomic, copy) void (^onCancel)(void);

@property (nonatomic, copy) void (^onError)
    (uint64_t errorCode, NSString *message, int32_t attemptCount);

@property (nonatomic, copy) void (^setRequestFilterCallbacks)
    (id<EnvoyHTTPFilterCallbacks> callbacks);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, optional pending headers
/// 2 - NSData *, optional pending data
/// 3 - EnvoyHeaders *, optional pending trailers
@property (nonatomic, copy) NSArray * (^onResumeRequest)
    (EnvoyHeaders *_Nullable headers, NSData *_Nullable data, EnvoyHeaders *_Nullable trailers,
     BOOL endStream);

@property (nonatomic, copy) void (^setResponseFilterCallbacks)
    (id<EnvoyHTTPFilterCallbacks> callbacks);

/// Returns tuple of:
/// 0 - NSNumber *,filter status
/// 1 - EnvoyHeaders *, optional pending headers
/// 2 - NSData *, optional pending data
/// 3 - EnvoyHeaders *, optional pending trailers
@property (nonatomic, copy) NSArray * (^onResumeResponse)
    (EnvoyHeaders *_Nullable headers, NSData *_Nullable data, EnvoyHeaders *_Nullable trailers,
     BOOL endStream);

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

#pragma mark - EnvoyStringAccessor

@interface EnvoyStringAccessor : NSObject

@property (nonatomic, copy) NSString * (^getEnvoyString)();

- (instancetype)initWithBlock:(NSString * (^)())block;

@end

#pragma mark - EnvoyNativeFilterConfig

@interface EnvoyNativeFilterConfig : NSObject

@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) NSString *typedConfig;

- (instancetype)initWithName:(NSString *)name typedConfig:(NSString *)typedConfig;

@end

#pragma mark - EnvoyConfiguration

/// Typed configuration that may be used for starting Envoy.
@interface EnvoyConfiguration : NSObject

@property (nonatomic, strong, nullable) NSString *grpcStatsDomain;
@property (nonatomic, assign) UInt32 connectTimeoutSeconds;
@property (nonatomic, assign) UInt32 dnsRefreshSeconds;
@property (nonatomic, assign) UInt32 dnsFailureRefreshSecondsBase;
@property (nonatomic, assign) UInt32 dnsFailureRefreshSecondsMax;
@property (nonatomic, strong) NSString *dnsPreresolveHostnames;
@property (nonatomic, assign) UInt32 statsFlushSeconds;
@property (nonatomic, assign) UInt32 streamIdleTimeoutSeconds;
@property (nonatomic, strong) NSString *appVersion;
@property (nonatomic, strong) NSString *appId;
@property (nonatomic, strong) NSString *virtualClusters;
@property (nonatomic, strong) NSString *directResponseMatchers;
@property (nonatomic, strong) NSString *directResponses;
@property (nonatomic, strong) NSArray<EnvoyNativeFilterConfig *> *nativeFilterChain;
@property (nonatomic, strong) NSArray<EnvoyHTTPFilterFactory *> *httpPlatformFilterFactories;
@property (nonatomic, strong) NSDictionary<NSString *, EnvoyStringAccessor *> *stringAccessors;

/**
 Create a new instance of the configuration.
 */
- (instancetype)
         initWithGrpcStatsDomain:(nullable NSString *)grpcStatsDomain
           connectTimeoutSeconds:(UInt32)connectTimeoutSeconds
               dnsRefreshSeconds:(UInt32)dnsRefreshSeconds
    dnsFailureRefreshSecondsBase:(UInt32)dnsFailureRefreshSecondsBase
     dnsFailureRefreshSecondsMax:(UInt32)dnsFailureRefreshSecondsMax
          dnsPreresolveHostnames:(NSString *)dnsPreresolveHostnames
               statsFlushSeconds:(UInt32)statsFlushSeconds
        streamIdleTimeoutSeconds:(UInt32)streamIdleTimeoutSeconds
                      appVersion:(NSString *)appVersion
                           appId:(NSString *)appId
                 virtualClusters:(NSString *)virtualClusters
          directResponseMatchers:(NSString *)directResponseMatchers
                 directResponses:(NSString *)directResponses
               nativeFilterChain:(NSArray<EnvoyNativeFilterConfig *> *)nativeFilterChain
             platformFilterChain:(NSArray<EnvoyHTTPFilterFactory *> *)httpPlatformFilterFactories
                 stringAccessors:(NSDictionary<NSString *, EnvoyStringAccessor *> *)stringAccessors;

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

 @param onEngineRunning Closure called when the engine finishes its async startup and begins
 running.
 @param logger Logging interface.
 */
- (instancetype)initWithRunningCallback:(nullable void (^)())onEngineRunning
                                 logger:(nullable void (^)(NSString *))logger;
/**
 Run the Envoy engine with the provided configuration and log level.

 @param config The EnvoyConfiguration used to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @return A status indicating if the action was successful.
 */
- (int)runWithConfig:(EnvoyConfiguration *)config logLevel:(NSString *)logLevel;

/**
 Run the Envoy engine with the provided yaml string and log level.

 @param yaml The configuration template with which to start Envoy.
 @param config The EnvoyConfiguration used to start Envoy.
 @param logLevel The log level to use when starting Envoy.
 @return A status indicating if the action was successful.
 */
- (int)runWithTemplate:(NSString *)yaml
                config:(EnvoyConfiguration *)config
              logLevel:(NSString *)logLevel;

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
- (int)recordCounterInc:(NSString *)elements tags:(EnvoyTags *)tags count:(NSUInteger)count;

/**
 Set a gauge of a given string of elements with the given value.

 @param elements Elements of the gauge stat.
 @param value Value to set to the gauge.
 @return A status indicating if the action was successful.
 */
- (int)recordGaugeSet:(NSString *)elements tags:(EnvoyTags *)tags value:(NSUInteger)value;

/**
 Add the gauge with the given string of elements and by the given amount.

 @param elements Elements of the counter stat.
 @param amount Amount to add to the gauge.
 @return A status indicating if the action was successful.
 */
- (int)recordGaugeAdd:(NSString *)elements tags:(EnvoyTags *)tags amount:(NSUInteger)amount;

/**
 Subtract from the gauge with the given string of elements and by the given amount.

 @param elements Elements of the gauge stat.
 @param amount Amount to subtract from the gauge.
 @return A status indicating if the action was successful.
 */
- (int)recordGaugeSub:(NSString *)elements tags:(EnvoyTags *)tags amount:(NSUInteger)amount;

/**
 Add another recorded duration to the timer histogram with the given string of elements.
 @param elements Elements of the histogram stat.
 @param durationMs The duration in milliseconds to record in the histogram distribution
 @return A status indicating if the action was successful.
 */
- (int)recordHistogramDuration:(NSString *)elements
                          tags:(EnvoyTags *)tags
                    durationMs:(NSUInteger)durationMs;

/**
 Add another recorded value to the histogram with the given string of elements.
 @param elements Elements of the histogram stat.
 @param value Amount to record as a new value for the histogram distribution.
 @return A status indicating if the action was successful.
 */
- (int)recordHistogramValue:(NSString *)elements tags:(EnvoyTags *)tags value:(NSUInteger)value;

- (void)flushStats;

@end

#pragma mark - EnvoyLogger

// Logging interface.
@interface EnvoyLogger : NSObject

@property (nonatomic, copy) void (^log)(NSString *);

/**
 Create a new instance of the logger.
 */
- (instancetype)initWithLogClosure:(void (^)(NSString *))log;

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
