#import "library/objective-c/EnvoyEngine.h"

#import "library/common/include/c_types.h"
#import "library/common/main_interface.h"

@implementation EnvoyEngine

#pragma mark - utility
static envoy_string EnvoyString(NSString *s) { return {s.length, strdup(s.UTF8String)}; }

#pragma mark - class methods
+ (int)runWithConfig:(NSString *)config {
  return [self runWithConfig:config logLevel:@"info"];
}

+ (int)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel {
  try {
    return run_engine(config.UTF8String, logLevel.UTF8String);
  } catch (NSException *e) {
    NSLog(@"Envoy exception: %@", e);
    NSDictionary *userInfo = @{@"exception" : e};
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException"
                                                      object:self
                                                    userInfo:userInfo];
    return 1;
  }
}

+ (void)setupEnvoy {
  setup_envoy();
}

@end
