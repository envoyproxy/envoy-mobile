#import <Foundation/Foundation.h>

/// A set of headers that may be passed to/from an Envoy stream.
typedef NSDictionary<NSString *, NSArray<NSString *> *> EnvoyHeaders;
