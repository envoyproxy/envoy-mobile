#import "library/objective-c/EnvoyEngineImpl.h"

#import "library/common/main_interface.h"

@implementation EnvoyEngineImpl

#pragma mark - class methods
+ (EnvoyStatus)runWithConfig:(NSString *)config {
  return [self runWithConfig:config logLevel:@"info"];
}

+ (EnvoyStatus)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel {
  // Envoy exceptions will only be caught here when compiled for 64-bit arches.
  // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Articles/Exceptions64Bit.html
  @try {
    return (EnvoyStatus)run_engine(config.UTF8String, logLevel.UTF8String);
  } @catch (...) {
    NSLog(@"Envoy exception caught.");
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException" object:self];
    return EnvoyStatusFailure;
  }
}

+ (void)setupEnvoy {
  setup_envoy();
}

@end
