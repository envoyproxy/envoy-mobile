#import "library/objective-c/EnvoyEngine.h"

#import "library/common/main_interface.h"

#import <UIKit/UIKit.h>

@implementation EnvoyDeviceLifecycleMonitor

+ (void)startFlushingStatsOnLifecycleChanges {
  static dispatch_once_t lifecycleMonitoringStarted;
  dispatch_once(&lifecycleMonitoringStarted, ^{
    [self startObservingLifecycleNotifications];
  });
}

#pragma mark - Private

+ (void)startObservingLifecycleNotifications {
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

+ (void)lifecycleDidChangeWithNotification:(NSNotification *)notification {
  NSLog(@"[Envoy] triggering stats flush (%@)", notification.name);
  flush_stats();
}

@end
