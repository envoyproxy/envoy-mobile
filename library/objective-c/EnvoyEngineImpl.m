#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"
#import "library/common/types/c_types.h"
#import "library/common/extensions/filters/http/platform_bridge/c_types.h"

#import <UIKit/UIKit.h>

static void ios_on_exit() {
  // Currently nothing needs to happen in iOS on exit. Just log.
  NSLog(@"[Envoy] library is exiting");
}

// TODO(goaway): The mapping code below contains a great deal of duplication from
// EnvoyHTTPStreamImpl.m, however retain/release semantics are slightly different and need to be
// reconciled before this can be factored into a generic set of utility functions.
static envoy_data toManagedNativeString(NSString *s) {
  size_t length = s.length;
  uint8_t *native_string = (uint8_t *)safe_malloc(sizeof(uint8_t) * length);
  memcpy(native_string, s.UTF8String, length);
  return (envoy_data){length, native_string, free, native_string};
}

static EnvoyHeaders *to_ios_headers(envoy_headers headers) {
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
  // TODO(goaway): consider solution that doesn't violate release convention
  // Note: We don't call release_envoy_headers because they may not be modified by the filter
  return headerDict;
}

static envoy_headers toNativeHeaders(EnvoyHeaders *headers) {
  envoy_header_size_t length = 0;
  for (NSString *headerKey in headers) {
    length += [headers[headerKey] count];
  }
  envoy_header *header_array = (envoy_header *)safe_malloc(sizeof(envoy_header) * length);
  envoy_header_size_t header_index = 0;
  for (NSString *headerKey in headers) {
    NSArray *headerList = headers[headerKey];
    for (NSString *headerValue in headerList) {
      envoy_header new_header = {toManagedNativeString(headerKey),
                                 toManagedNativeString(headerValue)};
      header_array[header_index++] = new_header;
    }
  }
  // TODO: ASSERT(header_index == length);
  return (envoy_headers){length, header_array};
}

static const void* ios_http_filter_init(const void *context) {
  EnvoyHTTPFilterFactory *filterFactory = (__bridge EnvoyHTTPFilterFactory *)context;
  EnvoyHTTPFilter *filter = filterFactory.create();
  return CFBridgingRetain(filter);
}

static envoy_filter_headers_status
ios_http_filter_on_request_headers(envoy_headers headers, bool end_stream, const void *context) {
  // TODO(goaway): optimize unmodified case
  EnvoyHTTPFilter *filter = (__bridge EnvoyHTTPFilter *)context;
  if (filter.onRequestHeaders == nil) {
    return (envoy_filter_headers_status){/*status*/ kEnvoyFilterHeadersStatusContinue,
                                         /*headers*/ headers};
  }

  EnvoyHeaders *platformHeaders = to_ios_headers(headers);
  release_envoy_headers(headers);
  // TODO(goaway): consider better solution for compound return
  NSArray *result = filter.onRequestHeaders(platformHeaders, end_stream);
  return (envoy_filter_headers_status){/*status*/ [result[0] intValue],
                                       /*headers*/ toNativeHeaders(result[1])};
}

static envoy_filter_headers_status
ios_http_filter_on_response_headers(envoy_headers headers, bool end_stream, const void *context) {
  // TODO(goaway): optimize unmodified case
  EnvoyHTTPFilter *filter = (__bridge EnvoyHTTPFilter *)context;
  if (filter.onResponseHeaders == nil) {
    return (envoy_filter_headers_status){/*status*/ kEnvoyFilterHeadersStatusContinue,
                                         /*headers*/ headers};
  }

  EnvoyHeaders *platformHeaders = to_ios_headers(headers);
  release_envoy_headers(headers);
  // TODO(goaway): consider better solution for compound return
  NSArray *result = filter.onResponseHeaders(platformHeaders, end_stream);
  return (envoy_filter_headers_status){/*status*/ [result[0] intValue],
                                       /*headers*/ toNativeHeaders(result[1])};
}

static void ios_http_filter_release(const void *context) {
  CFRelease(context);
  return;
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

- (int)registerFilterFactory:(EnvoyHTTPFilterFactory *)filterFactory {
  // TODO(goaway): Everything here leaks, but it's all be tied to the life of the engine.
  // This will need to be updated for https://github.com/lyft/envoy-mobile/issues/332
  envoy_http_filter *api = safe_malloc(sizeof(envoy_http_filter));
  api->init_filter = ios_http_filter_init;
  api->on_request_headers = ios_http_filter_on_request_headers;
  api->on_request_data = NULL;
  api->on_response_headers = ios_http_filter_on_response_headers;
  api->on_response_data = NULL;
  api->release_filter = ios_http_filter_release;
  api->static_context = CFBridgingRetain(filterFactory);
  api->instance_context = NULL;

  register_platform_api(filterFactory.filterName.UTF8String, api);
  return kEnvoySuccess;
}

- (int)runWithConfig:(EnvoyConfiguration *)config logLevel:(NSString *)logLevel {
  NSString *templateYAML = [[NSString alloc] initWithUTF8String:config_template];
  NSString *resolvedYAML = [config resolveTemplate:templateYAML];
  if (resolvedYAML == nil) {
    return kEnvoyFailure;
  }

  for (EnvoyHTTPFilterFactory *filterFactory in config.httpFilterFactories) {
    [self registerFilterFactory:filterFactory];
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
    return kEnvoyFailure;
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
