#import "AppDelegate.h"
#import <UIKit/UIKit.h>
#import "ViewController.h"

@interface AppDelegate ()
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    UIViewController *controller = [ViewController new];
    UINavigationController *navigation = [[UINavigationController alloc] initWithRootViewController:controller];
    self.window = [[UIWindow alloc] initWithFrame: [UIScreen mainScreen].bounds];
    [self.window setRootViewController:navigation];
    [self.window makeKeyAndVisible];

    [UIApplication sharedApplication].idleTimerDisabled = YES;
    NSLog(@"Finished launching!");
    return YES;
}

@end
