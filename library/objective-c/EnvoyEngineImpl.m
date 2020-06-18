#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"
#import "library/common/types/c_types.h"

#import <UIKit/UIKit.h>

static void ios_on_exit() {
  // Currently nothing needs to happen in iOS on exit. Just log.
  NSLog(@"[Envoy] library is exiting");
}

typedef struct {
  __unsafe_unretained EnvoyHTTPFilter *filter;
} ios_http_filter_context;

// TODO: factor out conversion
// TODO: move impl
static envoy_data toManagedNativeString(NSString *s) {
  size_t length = s.length;
  uint8_t *native_string = (uint8_t *)safe_malloc(sizeof(uint8_t) * length);
  memcpy(native_string, s.UTF8String, length);
  envoy_data ret = {length, native_string, free, native_string};
  return ret;
}

static EnvoyMutableHeaders *to_ios_mutable_headers(envoy_headers headers) {
  NSMutableDictionary *headerDict = [NSMutableDictionary new];
  for (envoy_header_size_t i = 0; i < headers.length; i++) {
    envoy_header header = headers.headers[i];
    NSString *headerKey = [[NSString alloc] initWithBytes:header.key.bytes
                                                   length:header.key.length
                                                 encoding:NSUTF8StringEncoding];
    NSString *headerValue = [[NSString alloc] initWithBytes:header.value.bytes
                                                     length:header.value.length
                                                   encoding:NSUTF8StringEncoding];
    NSMutableArray *headerValueList = headerDict[headerKey];
    if (headerValueList == nil) {
      headerValueList = [NSMutableArray new];
      headerDict[headerKey] = headerValueList;
    }
    [headerValueList addObject:headerValue];
  }
  // The C envoy_headers struct can be released now because the headers have been copied.
  release_envoy_headers(headers);
  return headerDict;
}

static envoy_headers toNativeHeaders(EnvoyHeaders *headers) {
  envoy_header_size_t length = 0;
  for (id headerKey in headers) {
    length += [headers[headerKey] count];
  }
  envoy_header *header_array = (envoy_header *)safe_malloc(sizeof(envoy_header) * length);
  envoy_header_size_t header_index = 0;
  for (id headerKey in headers) {
    NSArray *headerList = headers[headerKey];
    for (id headerValue in headerList) {
      envoy_header new_header = {toManagedNativeString(headerKey),
                                 toManagedNativeString(headerValue)};
      header_array[header_index++] = new_header;
    }
  }
  // TODO: ASSERT(header_index == length);
  envoy_headers ret = {length, header_array};
  return ret;
}

static uint32_t ios_http_filter_on_request_headers(envoy_headers *headers, bool end_stream,
                                                   void *context) {
  ios_http_filter_context *c = (ios_http_filter_context *)context;
  EnvoyMutableHeaders *platformHeaders = to_ios_mutable_headers(*headers);
  release_envoy_headers(*headers);
  c->filter.onRequestHeaders(platformHeaders, end_stream);
  *headers = toNativeHeaders(platformHeaders);
  return 0; // Continue
}

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

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
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
  // re-enable lifecycle-based stat flushing when https://github.com/lyft/envoy-mobile/issues/748
  // gets fixed. [self startObservingLifecycleNotifications];

  // Envoy exceptions will only be caught here when compiled for 64-bit arches.
  // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Articles/Exceptions64Bit.html
  @try {
    envoy_engine_callbacks native_callbacks = {ios_on_exit};
    return (int)run_engine(_engineHandle, native_callbacks, configYAML.UTF8String,
                           logLevel.UTF8String);
  } @catch (NSException *exception) {
    NSLog(@"[Envoy] exception caught: %@", exception);
    [NSNotificationCenter.defaultCenter postNotificationName:@"EnvoyError" object:self];
    return 1;
  }
}

- (id<EnvoyHTTPStream>)startStreamWithCallbacks:(EnvoyHTTPCallbacks *)callbacks {
  return [[EnvoyHTTPStreamImpl alloc] initWithHandle:init_stream(_engineHandle)
                                           callbacks:callbacks];
}

#pragma mark - Private

- (void)startObservingLifecycleNotifications {
  NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
  [notificationCenter addObserver:self
                         selector:@selector(lifecycleDidChangeWithNotification:)
                             name:UIApplicationWillResignActiveNotification
                           object:nil];
  [notificationCenter addObserver:self
                         selector:@selector(lifecycleDidChangeWithNotification:)
                             name:UIApplicationWillTerminateNotification
                           object:nil];
}

- (void)lifecycleDidChangeWithNotification:(NSNotification *)notification {
  NSLog(@"[Envoy] triggering stats flush (%@)", notification.name);
  flush_stats();
}

@end
