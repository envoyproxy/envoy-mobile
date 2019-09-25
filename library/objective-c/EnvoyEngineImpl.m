#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"
#import "library/common/types/c_types.h"

#import <SystemConfiguration/SystemConfiguration.h>

@implementation EnvoyEngineImpl {
  envoy_engine_t _engineHandle;
}

#pragma mark - Public interface

- (instancetype)init {
  self = [super init];
  if (!self) {
    return nil;
  }

  _engineHandle = init_engine();
  _ensure_reachability_running();
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

#pragma mark - Reachability

static SCNetworkReachabilityRef _reachability_ref;

static void _reachability_callback(SCNetworkReachabilityRef target,
                                   SCNetworkReachabilityFlags flags, void *info) {
  if (flags == 0) {
    return;
  }

  BOOL isUsingWWAN = flags & kSCNetworkReachabilityFlagsIsWWAN;
  set_preferred_network(isUsingWWAN ? ENVOY_NET_WWAN : ENVOY_NET_WLAN);
}

static void _ensure_reachability_running() {
  if (_reachability_ref) {
    return;
  }

  NSString *name = @"io.envoymobile.reachability";
  SCNetworkReachabilityRef reachability =
      SCNetworkReachabilityCreateWithName(nil, [name UTF8String]);
  if (!reachability) {
    return;
  }

  _reachability_ref = reachability;

  SCNetworkReachabilityContext context = {0, NULL, NULL, NULL, NULL};
  if (!SCNetworkReachabilitySetCallback(_reachability_ref, _reachability_callback, &context)) {
    return;
  }

  dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  if (!SCNetworkReachabilitySetDispatchQueue(_reachability_ref, queue)) {
    SCNetworkReachabilitySetCallback(_reachability_ref, NULL, NULL);
    return;
  }

  return;
}

@end
