
#import <Foundation/Foundation.h>
#import "test/swift/integration/test_extensions.h"

OBJC_EXPORT void force_register() {
	register_test_extensions();
}
