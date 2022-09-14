#include "library/common/extensions/stat_sinks/dynamic/sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {
namespace {
// Verifies that we throw an exception if we fail to resolve the dynamic symbols. We need to do this
// test in a different binary than sink_test.cc, as we define the em_record_counter symbol within
// the other test binary.
TEST(SinkTest, InvalidLibrary) {
  EXPECT_THROW_WITH_MESSAGE(Sink("doesntexist"), EnvoyException, "blah");
}
} // namespace
} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
