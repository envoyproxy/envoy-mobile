#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"
#import "library/common/types/c_types.h"

@implementation EnvoyEngineImpl {
  envoy_engine_t _engineHandle;
}

- (instancetype)init {
  self = [super init];
  if (!self) {
    return nil;
  }

  _engineHandle = init_engine();
  [EnvoyNetworkMonitor startReachabilityIfNeeded];
  return self;
}

- (int)runWithConfig:(EnvoyConfiguration *)config logLevel:(NSString *)logLevel {
  NSString *templateYAML = [[NSString alloc] initWithUTF8String:config_template];
  NSString *resolvedYAML = [config resolveTemplate:templateYAML];
  if (resolvedYAML == nil) {
    return 1;
  }

  return [self runWithConfigYAML:resolvedYAML logLevel:logLevel];
}

- (int)runWithConfigYAML:(NSString *)configYAML logLevel:(NSString *)logLevel {
  // Envoy exceptions will only be caught here when compiled for 64-bit arches.
  // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Articles/Exceptions64Bit.html
  @try {
    return (int)run_engine(configYAML.UTF8String, logLevel.UTF8String);
  } @catch (...) {
    NSLog(@"Envoy exception caught.");
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyException" object:self];
    return 1;
  }
}

- (id<EnvoyHTTPStream>)startStreamWithCallbacks:(EnvoyHTTPCallbacks *)callbacks {
  return [[EnvoyHTTPStreamImpl alloc] initWithHandle:init_stream(_engineHandle)
                                           callbacks:callbacks];
}

@end
