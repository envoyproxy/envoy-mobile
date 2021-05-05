#import <XCTest/XCTest.h>

typedef NSDictionary<NSString *, NSArray<NSString *> *> EnvoyHeaders;

typedef NSDictionary<NSString *, NSString *> EnvoyTags;

#import "library/objective-c/EnvoyBridgeUtility.h"

@interface EnvoyBridgeUtilityTest : XCTestCase
@end

@implementation EnvoyBridgeUtilityTest

- (void)setUp {
  [super setUp];
}

- (void)tearDown {
  [super tearDown];
}

- (void)testToNativeData {
  NSString *testString = @"abc";
  NSData *testData = [testString dataUsingEncoding:NSUTF8StringEncoding];
  envoy_data nativeData = toNativeData(testData);
  XCTAssertTrue(memcmp(nativeData.bytes, testData.bytes, 3) == 0);
}

@end
