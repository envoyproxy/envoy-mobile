#import <Foundation/Foundation.h>

// MARK: - Aliases

typedef UInt64 EnvoyStreamID;

typedef NSArray<NSDictionary<NSString *, NSString *> *> EnvoyHeaders;

// MARK: - EnvoyErrorCode

typedef NS_ENUM(NSUInteger, EnvoyErrorCode) {
  StreamReset = 0,
};

// MARK: - EnvoyError

@interface EnvoyError : NSError

@property (nonatomic, copy) NSString *message;

@property (nonatomic, assign) EnvoyErrorCode errorCode;

@end

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

// MARK: - Callbacks

typedef void (^OnHeadersClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnDataClosure)(NSData *, BOOL);

typedef void (^OnMetadataClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnTrailersClosure)(EnvoyHeaders *, BOOL);

typedef void (^OnErrorClosure)(EnvoyError *);

// MARK: - EnvoyObserver

@interface EnvoyObserver : NSObject

@property (nonatomic, strong) OnHeadersClosure onHeaders;

@property (nonatomic, strong) OnDataClosure onData;

@property (nonatomic, strong) OnMetadataClosure onMetadata;

@property (nonatomic, strong) OnTrailersClosure onTrailers;

@property (nonatomic, strong) OnErrorClosure onError;

@end
