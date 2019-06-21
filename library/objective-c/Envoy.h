#import <Foundation/Foundation.h>

@interface Envoy : NSObject

/// Indicates whether this Envoy instance is currently active and running.
@property (nonatomic, readonly, getter=isRunning) BOOL running;

/// Returns whether the Envoy instance is terminated.
@property (nonatomic, readonly, getter=isTerminated) BOOL terminated;

/// Create a new Envoy instance. The Envoy runner NSThread is started as part of instance
/// initialization with the configuration provided.
- (instancetype)initWithConfig:(NSString*)config;
@end
