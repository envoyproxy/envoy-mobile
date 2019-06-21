#import "library/objective-c/Envoy.h"

#import "library/common/main_interface.h"


@interface Envoy ()
@property (nonatomic, strong) NSThread *runner;
@end

@implementation Envoy

@synthesize runner;

- (id)initWithConfig:(NSString *)config {
  if (self = [super init]) {
    self.runner = [[NSThread alloc] initWithTarget:self selector:@selector(run:) object:config];
    [self.runner start];
  } else {
    return nil;
  }
}

- (BOOL)isRunning {
  return self.runner.isExecuting;
}

- (BOOL)isTerminated {
  return self.runner.isFinished;
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
