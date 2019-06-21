#import <Foundation/Foundation.h>

@interface Envoy : NSObject
@property (nonatomic, readonly, getter=isRunning) BOOL running;
@property (nonatomic, readonly, getter=isTerminated) BOOL terminated;

- (id)initWithConfig:(NSString*)config;
@end
