#import "AppDelegate.h"
#include "absl/debugging/symbolize.h"


int main(int argc, char * argv[]) {
  @autoreleasepool {
    absl::InitializeSymbolizer(argv[0]);

    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
  }
}
