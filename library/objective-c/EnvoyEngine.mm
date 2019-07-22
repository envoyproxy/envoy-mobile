#import "library/objective-c/EnvoyEngine.h"

#import "library/common/include/c_types.h"
#import "library/common/main_interface.h"

@implementation EnvoyEngine

#pragma mark - utility
static envoy_string EnvoyString(NSString *s) { return {s.length, strdup(s.UTF8String)}; }

static void printHeaders(envoy_headers headers, bool sent) {
  for (int i = 0; i < headers.length; i++) {
    NSString *name = @(headers.headers[i].name.data);
    NSString *value = @(headers.headers[i].value.data);
    NSString *action = sent ? @"SENT" : @"RECEIVED";
    NSLog(@"[%@ HEADER] %@: %@", action, name, value);
  }
}

+ (void)makeRequest {
  NSLog(@"Inside makeRequest");
  void (^onHeaders)(envoy_headers) = ^(envoy_headers headers) {
    printHeaders(headers, false);
  };

  envoy_observer observer = {
      platform_on_headers,
      nullptr,
      nullptr,
      nullptr,
  };

  NSLog(@"Calling start_stream");
  envoy_stream stream_pair = start_stream(observer);
  NSLog(@"Checking start_stream result");
  if (stream_pair.status == ENVOY_SUCCESS) {
    NSLog(@"[STREAM OPEN: %@]", @(stream_pair.stream));
    _callbacks[@(stream_pair.stream)] = onHeaders;
  } else {
    NSLog(@"[STREAM OPEN FAILED]");
  }

  envoy_header *header_array = new envoy_header[4];

  header_array[0] = {EnvoyString(@":method"), EnvoyString(@"GET")};
  header_array[1] = {EnvoyString(@":scheme"), EnvoyString(@"https")};
  header_array[2] = {EnvoyString(@":authority"), EnvoyString(@"api.lyft.com")};
  header_array[3] = {EnvoyString(@":path"), EnvoyString(@"/ping")};

  envoy_headers request_headers = {4, header_array};

  envoy_status_t status = send_headers(stream_pair.stream, request_headers, true);
  if (status == ENVOY_SUCCESS) {
    printHeaders(request_headers, true);
  } else {
    NSLog(@"[FAILED TO SEND HEADERS]");
  }
}

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

+ (void)setupEnvoy {
  setup_envoy();
}

@end
