#import "library/objective-c/EnvoyEngineImpl.h"

#import "library/common/main_interface.h"

@implementation EnvoyEngineImpl

- (int)runWithConfig:(NSString *)config {
  return [self runWithConfig:config logLevel:@"info"];
}

- (int)runWithConfig:(NSString *)config logLevel:(NSString *)logLevel {
  // Envoy exceptions will only be caught here when compiled for 64-bit arches.
  // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Articles/Exceptions64Bit.html
  @try {
    return (int)run_engine(config.UTF8String, logLevel.UTF8String);
  } @catch (...) {
    NSLog(@"Envoy exception caught.");
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException" object:self];
    return 1;
  }
}

- (void)setupEnvoy {
  setup_envoy();
}

- (EnvoyStream *)startStreamWithObserver:(EnvoyObserver *)observer {
  // TODO: Use a envoy_engine_t to select an instance for the stream
  return [[EnvoyStream alloc] initWithObserver:observer];
}

@end
