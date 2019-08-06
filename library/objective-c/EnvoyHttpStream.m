#import "library/objective-c/EnvoyHttpStream.h"

#import "library/common/include/c_types.h"
#import "library/common/main_interface.h"

#import <stdatomic.h>

#pragma mark - utility functions to move elsewhere
typedef struct {
  atomic_bool *canceled;
  EnvoyObserver *observer;
} ios_context;

// static envoy_data toUnmanagedNativeData(NSData *data) {
//   // TODO: implement me
//   return envoy_nodata;
// }

static void ios_free_native_data(void *context) {
  free(context);
}

// static void ios_release_native_data(void *context) {
//   // TODO: implement me
// }

static envoy_data toManagedNativeString(NSString *s) {
  size_t length = s.length;
  uint8_t *native_string = (uint8_t *)malloc(sizeof(uint8_t) * length);
  memcpy(native_string, s.UTF8String, length);
  envoy_data ret = { length, native_string, ios_free_native_data, native_string };
  return ret;
}

static envoy_headers toNativeHeaders(EnvoyHeaders *headers) {
  int length = 0;
  for (id headerList in headers) {
    length += [headerList count];
  }
  envoy_header *header_array = (envoy_header *)malloc(sizeof(envoy_header) * length);
  int header_index = 0;
  for (id headerKey in headers) {
    NSArray *headerList = headers[headerKey];
    for (id headerValue in headerList) {
      envoy_header new_header = {
        toManagedNativeString(headerKey),
        toManagedNativeString(headerValue)
      };
      header_array[header_index++] = new_header;
    }
  }
  // ASSERT(header_index == length);
  envoy_headers ret = { length, header_array };
  return ret;
}

static NSData * to_ios_data(envoy_data data) {
  // TODO: investigate buffer ownership
  // Possibly extend/subclass NSData to call envoy_data.release on dealloc and have release drain the underlying Envoy buffer instance
  return [NSData dataWithBytes:(void *)data.bytes length:data.length];
}

static EnvoyHeaders * to_ios_headers(envoy_headers headers) {
  NSMutableDictionary *headerDict = [NSMutableDictionary new];
  for (uint_fast32_t i = 0; i < headers.length; i++) {
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
  return headerDict;
}

#pragma mark - c callbacks
static void ios_on_headers(envoy_headers headers, bool end_stream, void* context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    if (atomic_load(c->canceled)) {
      return;
    }
    observer.onHeaders(to_ios_headers(headers), end_stream);
  });
}

static void ios_on_data(envoy_data data, bool end_stream, void* context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    if (atomic_load(c->canceled)) {
      return;
    }
    // TODO: retain data
    observer.onData(to_ios_data(data), end_stream);
  });
}

static void ios_on_metadata(envoy_headers metadata, void* context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    if (atomic_load(c->canceled)) {
      return;
    }
    observer.onMetadata(to_ios_headers(metadata));
  });
}

static void ios_on_trailers(envoy_headers trailers, void *context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    if (atomic_load(c->canceled)) {
      return;
    }
    observer.onTrailers(to_ios_headers(trailers));
  });
}

static void ios_on_complete(void *context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    // TODO: release stream
    if (atomic_load(c->canceled)) {
      return;
    }
  });
}

static void ios_on_cancel(void *context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  // TODO: release stream
  dispatch_async(observer.dispatchQueue, ^{
    // This call is atomically gated at the call-site and will only happen once.
    observer.onCancel();
  });
}

static void ios_on_error(envoy_error error, void *context) {
  ios_context *c = (ios_context *)context;
  EnvoyObserver *observer = c->observer;
  dispatch_async(observer.dispatchQueue, ^{
    // TODO: release stream
    if (atomic_load(c->canceled)) {
      return;
    }
    // FIXME transform error and pass up
    observer.onError([EnvoyEngineError alloc]);
  });
}


@implementation EnvoyHttpStream {
  EnvoyHttpStream *_strongSelf;
  EnvoyObserver *_platformObserver;
  envoy_observer *_nativeObserver;
  envoy_stream_t _nativeStream;
}

- (instancetype)initWithObserver:(EnvoyObserver *)observer {
  self = [super init];
  if (!self) {
    return nil;
  }

  // Retain platform observer
  _platformObserver = observer;

  // Create callback context
  ios_context *context = (ios_context *)malloc(sizeof(ios_context));
  context->observer = observer;
  context->canceled = (atomic_bool *)(malloc(sizeof(atomic_bool)));
  atomic_store(context->canceled, NO);

  // Create native observer
  envoy_observer *native_obs = (envoy_observer *)malloc(sizeof(envoy_observer));
  envoy_observer native_init = {
    ios_on_headers,
    ios_on_data,
    ios_on_trailers,
    ios_on_metadata,
    ios_on_complete,
    ios_on_error,
    context
  };
  memcpy(native_obs, &native_init, sizeof(envoy_observer));
  _nativeObserver = native_obs;

  envoy_stream result = start_stream(*native_obs);
  if (result.status != ENVOY_SUCCESS) {
    return nil;
  }

  _nativeStream = result.stream;
  _strongSelf = self;
  return self;
}

- (void)dealloc {
  envoy_observer *native_obs = _nativeObserver;
  _nativeObserver = nil;
  ios_context *context = native_obs->context;
  free(context->canceled);
  free(context);
  free(native_obs);
}

- (void)sendHeaders:(EnvoyHeaders *)headers close:(BOOL)close {
  send_headers(_nativeStream, toNativeHeaders(headers), close);
}

- (void)sendData:(NSData *)data close:(BOOL)close {
  // TODO: implement
  //send_data(_nativeStream, toNativeData(data), close);
}

- (void)sendMetadata:(EnvoyHeaders *)metadata {
  send_metadata(_nativeStream, toNativeHeaders(metadata));
}

- (void)sendTrailers:(EnvoyHeaders *)trailers {
  send_trailers(_nativeStream, toNativeHeaders(trailers));
}

- (EnvoyStatus)cancel {
  ios_context *context = _nativeObserver->context;
  // Step 1: atomically and synchronously prevent the execution of further callbacks other than on_cancel.
  if (!atomic_exchange(context->canceled, YES)) {
    // Step 2: directly fire the cancel callback.
    ios_on_cancel(context);
    // Step 3: propagate the reset into native code.
    reset_stream(_nativeStream);
    return EnvoyStatusSuccess;
  } else {
    return EnvoyStatusFailure;
  }
}

@end
