#import "EnvoyEngine.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Wrapper layer for calling into Envoy's C/++ API.
@interface EnvoyEngineImpl : NSObject <EnvoyEngine>

/// Performs necessary setup after Envoy has initialized and started running.
/// TODO: create a post-initialization callback from Envoy to handle this automatically.
+ (void)setupEnvoy;

@end

NS_ASSUME_NONNULL_END
