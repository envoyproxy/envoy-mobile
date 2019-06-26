#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"

@implementation EnvoyEngine

+ (int)runWithConfig:(NSString *config) {
  return [self runWithConfig:config logLevel:@"info"];
}

+ (void)runWithConfig:(NSString *config) logLevel:(NSString *logLevel) {
  try {
    return run_envoy(config.UTF8String, logLevel.UTF8String);
  } catch (NSException *e) {
    NSLog(@"Envoy exception: %@", e);
    NSDictionary *userInfo = @{ @"exception": e};
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException"
                                        object:self
                                        userInfo:userInfo];
  }
}

@end
