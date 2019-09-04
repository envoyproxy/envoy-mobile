#import "library/objective-c/EnvoyEngine.h"

@implementation EnvoyError {
  uint64_t _errorCode;
  NSString *_message;
}

- (instancetype)initWithErrorCode:(uint64_t)errorCode message:(NSString *)message {
  _errorCode = errorCode;
  _message = message;
}

@end
