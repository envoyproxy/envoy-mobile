#import <Foundation/Foundation.h>

// MARK: - EnvoyErrorCode

typedef NS_ENUM(NSUInteger, EnvoyErrorCode) {
    StreamReset = 0,
};

// MARK: - EnvoyError

@interface EnvoyError: NSError

@property (nonatomic, copy) NSString *message;

@property (nonatomic, assign) EnvoyErrorCode errorCode;

@end

// MARK: - Aliases

typedef UInt64 EnvoyStreamID;

typedef NSArray<NSDictionary<NSString *, NSString *> *> EnvoyHeaders;

typedef void (^OnHeadersClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnDataClosure)(NSData *, BOOL);

typedef void (^OnMetadataClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnTrailersClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnErrorClosure)(EnvoyError *);

// MARK: - EnvoyStatus

typedef NS_ENUM(NSUInteger, EnvoyStatus) {
    Success = 0,
    Failure = 1,
};

// MARK: - EnvoyStream

typedef struct {
    EnvoyStatus status;
    EnvoyStreamID streamID;
} EnvoyStream;

// MARK: - EnvoyEngine

/// Wrapper layer to simplify calling into Envoy's C/++ API.
@interface EnvoyEngine : NSObject

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (EnvoyStatus)runWithConfig:(NSString *)config;

/// Run the Envoy engine with the provided config and log level. This call is synchronous
/// and will not yield.
+ (EnvoyStatus)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel;

@end

// MARK: - EnvoyObserver

@interface EnvoyObserver : NSObject

@property (nonatomic, strong) OnHeadersClosure onHeaders;

@property (nonatomic, strong) OnDataClosure onData;

@property (nonatomic, strong) OnMetadataClosure onMetadata;

@property (nonatomic, strong) OnTrailersClosure onTrailers;

@property (nonatomic, strong) OnErrorClosure onError;

@end
