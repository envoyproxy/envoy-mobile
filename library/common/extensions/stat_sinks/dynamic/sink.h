#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "envoy/stats/sink.h"

#include "library/common/extensions/stat_sinks/dynamic/config.pb.h"
#include "library/common/extensions/stat_sinks/dynamic/dynamic_sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {

// A stats sink which flushes metrics via a configured shared library using dynamic symbol loading.
class Sink : public Stats::Sink {
public:
  Sink(const std::string& library_name);

  // Stats::Sink;
  void flush(Stats::MetricSnapshot& snapshot) override;
  void onHistogramComplete(const Stats::Histogram&, uint64_t) override {}

private:
  em_record_counter_t record_counter_ = nullptr;
};

} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
