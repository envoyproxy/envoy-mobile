#import "library/objective-c/EnvoyEngine.h"

#pragma mark - EnvoyHTTPFilterCallbacksImpl

@implementation EnvoyHTTPFilterCallbacks {
}

- (instancetype)init {
  self = [super init];
  if (!self) {
    return nil
  }
  return self;
}

- (void)resumeIteration {
  NSLog(@"async resume");
}

@end
