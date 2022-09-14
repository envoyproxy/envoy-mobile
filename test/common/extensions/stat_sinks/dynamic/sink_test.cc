#include "test/mocks/stats/mocks.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"
#include "library/common/extensions/stat_sinks/dynamic/sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {
namespace {

using LoopRecordCounterFunc = std::function<void(em_string_view, em_metric_tag*, size_t, size_t)>;
LoopRecordCounterFunc* record_counter_handle() {
  static LoopRecordCounterFunc handle = nullptr;

  return &handle;
}

__attribute__((__used__)) extern "C" void
em_record_counter(em_string_view name, em_metric_tag* tags, size_t tag_size, size_t value) {
  (*record_counter_handle())(name, tags, tag_size, value);
}

// Verifies that we can load the symbol from a configured shared library path.
TEST(SinkTest, SharedLibrary) {
  Sink sink(TestEnvironment::runfilesPath(
      "test/common/extensions/stat_sinks/dynamic/libshared_library.dylib", "envoy_mobile"));

  Stats::MockMetricSnapshot snapshot;

  Stats::MockCounter counter;
  counter.name_ = "no_tags";
  snapshot.counters_.emplace_back(Stats::MetricSnapshot::CounterSnapshot{5, std::ref(counter)});

  // Trying to assert that we called the function defined in the shared library is a bit involved,
  // so instead we verify that we do not call the function defined in the main binary. This, coupled
  // with the fact that we throw an exception if we fail to resolve a function, seems like a
  // reasonable test that we use the configured binary.
  bool called = false;
  *record_counter_handle() = [&](em_string_view, em_metric_tag*, size_t, size_t) { called = true; };
  EXPECT_CALL(snapshot, counters());
  sink.flush(snapshot);

  EXPECT_FALSE(called);
}

// Verifies the behavior of the conversion to string views and the filtering logic when shared
// library fails and the symbol is resolved from the main binary.
TEST(SinkTest, SinkFlushesToLoop) {
  Sink sink("doesntexist");

  // List of lambdas that will be invoked for counters passed to loop-sdk, in order.
  auto counter_assertions = std::list<LoopRecordCounterFunc>{
      {[&](em_string_view name, em_metric_tag* tags, size_t tag_count, size_t value) {
         ASSERT_EQ(absl::string_view(name.data, name.size), "counter");
         ASSERT_EQ(tag_count, 1);
         EXPECT_EQ(absl::string_view(tags[0].key.data, tags[0].key.size), "foo");
         EXPECT_EQ(absl::string_view(tags[0].value.data, tags[0].value.size), "bar");
         EXPECT_EQ(value, 1);
       },
       {[&](em_string_view name, em_metric_tag* tags, size_t tag_count, size_t value) {
         EXPECT_EQ(absl::string_view(name.data, name.size), "no_tags");
         ASSERT_EQ(tag_count, 0);
         EXPECT_NE(tags, nullptr);
         EXPECT_EQ(value, 5);
       }}}};

  *record_counter_handle() = [&](em_string_view name, em_metric_tag* tags, size_t tag_count,
                                 size_t value) {
    ASSERT_FALSE(counter_assertions.empty());
    counter_assertions.front()(name, tags, tag_count, value);
    counter_assertions.pop_front();
  };

  Stats::MockMetricSnapshot snapshot;

  // Standard counter with tags.
  Stats::MockCounter counter1;
  counter1.name_ = "counter";
  counter1.setTags({{"foo", "bar"}});
  snapshot.counters_.emplace_back(Stats::MetricSnapshot::CounterSnapshot{1, std::ref(counter1)});

  // Standard counter with a zero delta. This should be ignored.
  Stats::MockCounter counter2;
  counter2.name_ = "zero_delta";
  counter2.setTags({{"foo", "bar"}});
  snapshot.counters_.emplace_back(Stats::MetricSnapshot::CounterSnapshot{0, std::ref(counter2)});

  // Standard counter with no tags.
  Stats::MockCounter counter3;
  counter3.name_ = "no_tags";
  snapshot.counters_.emplace_back(Stats::MetricSnapshot::CounterSnapshot{5, std::ref(counter3)});

  EXPECT_CALL(snapshot, counters());
  sink.flush(snapshot);

  // Ensure that all assertions were used.
  EXPECT_TRUE(counter_assertions.empty());
}

} // namespace
} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
