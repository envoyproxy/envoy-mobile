#import "AppDelegate.h"
#import <Envoy/Envoy.h>
#import <UIKit/UIKit.h>
#import "ViewController.h"

@interface AppDelegate ()
@property (nonatomic, strong) Envoy *envoy;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    UIViewController *controller = [ViewController new];
    UINavigationController *navigation = [[UINavigationController alloc] initWithRootViewController:controller];
    self.window = [[UIWindow alloc] initWithFrame: [UIScreen mainScreen].bounds];
    [self.window setRootViewController:navigation];
    [self.window makeKeyAndVisible];

    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [NSThread detachNewThreadSelector:@selector(startEnvoy) toTarget:self withObject:nil];
    NSLog(@"Finished launching!");
    return YES;
}

- (void)startEnvoy {
    NSString *configFile = [[NSBundle mainBundle] pathForResource:@"config" ofType:@"yaml"];
    NSString *configYaml = [NSString stringWithContentsOfFile:configFile encoding:NSUTF8StringEncoding error:NULL];
    NSLog(@"Loading config:\n%@", configYaml);

    // Initialize the server's main context under a try/catch loop and simply return EXIT_FAILURE
    // as needed. Whatever code in the initialization path that fails is expected to log an error
    // message so the user can diagnose.
    try {
        self.envoy = [[Envoy alloc] initWithConfig:configYaml logLevel:EnvoyLogLevelTrace];
    } catch (NSException *e) {
        NSLog(@"Error starting Envoy: %@", e);
        exit(EXIT_FAILURE);
    }
}

@end
