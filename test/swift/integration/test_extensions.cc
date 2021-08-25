#include "test/swift/integration/test_extensions.h"

#include "library/common/extensions/filters/http/test_logger/config.h"

void register_test_extensions() {
  Envoy::Extensions::HttpFilters::TestLogger::forceRegisterFactory();
}
