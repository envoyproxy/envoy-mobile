#import "AppDelegate.h"
#import <UIKit/UIKit.h>
#import "ViewController.h"
#include "library/objective-c/EnvoyEngine.h"

@interface AppDelegate () <NSURLConnectionDelegate>
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  [NSThread detachNewThreadSelector:@selector(envoyRunner) toTarget:self withObject:nil];
  [NSThread detachNewThreadSelector:@selector(envoyHarness) toTarget:self withObject:nil];

  UIViewController *controller = [UITableViewController new];
  self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
  [self.window setRootViewController:controller];
  [self.window makeKeyAndVisible];
  NSLog(@"Finished launching!");
  return YES;
}

- (void)envoyRunner {
  // Read in and store as string
  NSString *configFile = [[NSBundle mainBundle] pathForResource:@"config" ofType:@"yaml"];
  NSString *configYaml = [NSString stringWithContentsOfFile:configFile
                                                   encoding:NSUTF8StringEncoding
                                                      error:NULL];
  NSLog(@"Loading config: %@", configYaml);

  [EnvoyEngine runWithConfig:configYaml logLevel:@"debug"];
}

- (void)envoyHarness {
  sleep(5);
  [EnvoyEngine setupEnvoy];
  sleep(1);
  while (true) {
    [EnvoyEngine makeRequest];
    sleep(1);
  }
}

#pragma mark NSURLConnectionDelegate

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
  NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
  NSLog(@"status: %ld", httpResponse.statusCode);
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
}

- (NSCachedURLResponse *)connection:(NSURLConnection *)connection
                  willCacheResponse:(NSCachedURLResponse *)cachedResponse {
  return nil;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
  NSLog(@"response complete");
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
  NSLog(@"ut oh");
}

@end
