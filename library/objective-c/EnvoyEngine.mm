#import "library/objective-c/EnvoyEngine.h"

#import "library/common/include/c_types.h"
#import "library/common/main_interface.h"

@implementation EnvoyEngine

#pragma mark - utility
static envoy_data EnvoyString(NSString *s) { return {s.length, strdup(s.UTF8String)}; }

#pragma mark - class methods
+ (EnvoyStatus)runWithConfig:(NSString *)config {
  return [self runWithConfig:config logLevel:@"info"];
}

+ (EnvoyStatus)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel {
  try {
    return (EnvoyStatus)run_engine(config.UTF8String, logLevel.UTF8String);
  } catch (NSException *e) {
    NSLog(@"Envoy exception: %@", e);
    NSDictionary *userInfo = @{@"exception" : e};
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException"
                                                      object:self
                                                    userInfo:userInfo];
    return Failure;
  }
}

+ (void)setupEnvoy {
  setup_envoy();
}

+ (EnvoyStatus)sendHeaders:(EnvoyHeaders *)headers to:(EnvoyStream *)stream close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

+ (EnvoyStatus)sendData:(NSData *)data to:(EnvoyStream *)stream close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

+ (EnvoyStatus)sendMetadata:(EnvoyHeaders *)metadata to:(EnvoyStream *)stream close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

+ (EnvoyStatus)sendTrailers:(EnvoyHeaders *)trailers to:(EnvoyStream *)stream close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

+ (EnvoyStatus)locallyCloseStream:(EnvoyStream *)stream {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

+ (EnvoyStatus)resetStream:(EnvoyStream *)stream {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

@end
