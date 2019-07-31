#import "library/objective-c/EnvoyEngine.h"
#import "library/objective-c/EnvoyTypes.h"

#import "library/common/include/c_types.h"
#import "library/common/main_interface.h"


#pragma mark - callbacks
static void ios_on_headers(envoy_headers headers, bool end_stream, void* context) {
  EnvoyObserver *observer = (EnvoyObserver *)context;
  dispatch_async(context.dispatchQueue, ^{
    if (atomic_load(canceled)) {
      return;
    }
    observer.onHeaders(translate_headers(headers), end_stream);
  });
}

static void ios_on_data(envoy_data data, bool end_stream, void* context) {
EnvoyObserver *observer = (EnvoyObserver *)context;
  dispatch_async(context.dispatchQueue, ^{
    if (atomic_load(canceled)) {
      return;
    }
    // TODO: retain data
    observer.onData(translate_data(data), end_stream);
  });
}

static void ios_on_metadata(envoy_headers metadata, void* context) {
  EnvoyObserver *observer = (EnvoyObserver *)context;
  dispatch_async(context.dispatchQueue, ^{
    if (atomic_load(canceled)) {
      return;
    }
    observer.onMetadata(translate_headers(metadata));
  });
}

static void ios_on_trailers(envoy_headers trailers, bool end_stream, void *context) {
  EnvoyObserver *observer = (EnvoyObserver *)context;
  dispatch_async(context.dispatchQueue, ^{
    if (atomic_load(canceled)) {
      return;
    }
    observer.onTrailers(translate_headers(trailers), end_stream);
  });
}

static void ios_on_complete(void *context) {
  EnvoyObserver *observer = (EnvoyObserver *)context;
  dispatch_async(context.dispatchQueue, ^{
    // TODO: release stream
    if (atomic_load(canceled)) {
      return;
    }
  });
}

static void ios_on_error(envoy_error error, void *context) {
  // TODO: implement me
}

static envoy_data EnvoyString(NSString *s) { return {s.length, strdup(s.UTF8String)}; }

@implementation EnvoyEngine

#pragma mark - class methods
+ (EnvoyStatus)runWithConfig:(NSString *)config {
  return [self runWithConfig:config logLevel:@"debug"];
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

// TODO: move to Envoy
+ (EnvoyStream *)startHttpStreamForRequest:(EnvoyRequest *)request handler:(EnvoyObserver *)handler {
  EnvoyStream *stream = [[EnvoyStream alloc] initWithEngine:engine,
                                                    request:request,
                                                   observer:handler];
}

@end

@implementation EnvoyHttpStream

@property (nonatomic, strong) __typeof__(self) strongSelf;
@property (nonatomic, strong) EnvoyObserver *platformObserver;
@property (nonatomic, assign) envoy_observer *nativeObserver;
@property (nonatomic, assign) envoy_stream_t nativeStream;

- (instancetype)initWithObserver:(EnvoyObserver *)observer {
  self = [super init];
  if (!self) {
    return nil;
  }

  self.platformObserver = observer;
  envoy_observer *native_obs = (envoy_observer *)malloc(sizeof(envoy_observer));  

  atomic_bool *canceled = (atomic_bool *)malloc(sizeof(atomic_bool));
  atomic_store(canceled, false);

  envoy_observer native_init = {
    ios_on_headers,
    ios_on_data,
    ios_on_trailers,
    ios_on_metadata,
    ios_on_complete,
    ios_on_error,
    canceled,
    observer
  };
  memcpy(native_obs, &native_init, sizeof(envoy_observer));

  self.nativeObserver = native_obs;
  envoy_stream result = start_stream(native_obs);
  if (result.status != ENVOY_SUCCESS) {
    return nil;
  }

  self.nativeStream = result.stream;
  self.strongSelf = self;
  return self;
}

- (void)dealloc {
  free(self.nativeObserver);
}

- (EnvoyStatus)sendHeaders:(EnvoyHeaders *)headers close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

- (EnvoyStatus)sendData:(NSData *)data close:(BOOL)close {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

- (EnvoyStatus)sendMetadata:(EnvoyHeaders *)metadata {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

- (EnvoyStatus)sendTrailers:(EnvoyHeaders *)trailers {
  NSLog(@"%@ not implemented, returning failure", NSStringFromSelector((SEL) __func__));
  return Failure;
}

- (EnvoyStatus)cancel {
  if (!atomic_exchange(self.nativeObserver.canceled, YES)) {
    ios_on_error({0, nullptr, 0}, self.nativeObserver.observer); // TODO: "canceled"
    return Success;
  } else {
    return Failure;
  }
}

@end
