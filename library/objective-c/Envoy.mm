#import "library/objective-c/Envoy.h"

#import "library/common/main_interface.h"


@interface Envoy ()
@property (nonatomic, strong) NSThread *runner;
@end

@implementation Envoy

@synthesize runner;

// Create a new Envoy instance. The Envoy runner NSThread is started as part of instance
// initialization with the configuration provided.
- (id)initWithConfig:(NSString *)config {
  self.runner = [[NSThread alloc] initWithTarget:self selector:@selector(run:) object:config];
  [self.runner start];
}

// Returns whether this Envoy instance is currently active and running.
- (BOOL)isRunning {
  return self.runner != NULL && self.runner.isExecuting;
}

// Returns whether the Envoy instance is terminated.
- (BOOL)isTerminated {
  return self.runner != NULL && self.runner.isFinished;
}

#pragma mark private

- (void)run:(NSString *)config {
  try {
    run_envoy(config.UTF8String);
  } catch (NSException *e) {
    NSLog(@"Envoy exception: %@", e);
    NSDictionary *userInfo = @{ @"exception": e};
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException"
                                        object:self
                                        userInfo:userInfo];
  }
}

@end
