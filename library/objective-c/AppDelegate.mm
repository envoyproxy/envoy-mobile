#import "AppDelegate.h"
#include "exe/main_common.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [NSThread detachNewThreadSelector:@selector(envoyRunner) toTarget:self withObject:nil];
    NSLog(@"Finished launching!");
    return YES;
}

- (void)envoyRunner {
    std::unique_ptr<Envoy::MainCommon> main_common;

    // Initialize the server's main context under a try/catch loop and simply return EXIT_FAILURE
    // as needed. Whatever code in the initialization path that fails is expected to log an error
    // message so the user can diagnose.

    const char* argv[] = { "envoy", "--config-yaml", "*" };
    try {
      main_common = std::make_unique<Envoy::MainCommon>(3, argv);
    } catch (const Envoy::NoServingException& e) {
      exit(EXIT_SUCCESS);
      return;
    } catch (const Envoy::MalformedArgvException& e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (const Envoy::EnvoyException& e) {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }

    // Run the server listener loop outside try/catch blocks, so that unexpected exceptions
    // show up as a core-dumps for easier diagnostics.
    main_common->run();
}

@end
