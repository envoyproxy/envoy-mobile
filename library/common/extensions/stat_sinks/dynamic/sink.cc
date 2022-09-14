#include "library/common/extensions/stat_sinks/dynamic/sink.h"

#include <dlfcn.h>

#include "envoy/common/exception.h"

#include "library/common/extensions/stat_sinks/dynamic/dynamic_sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {

Sink::Sink(const std::string& library_name) {
  // We support loading the dynamic symbol in two ways: either via a dynamically loaded library
  // named `library_name`, or from the main process (in the case of static linking).
  // To do this we first attempt to load the shared library, falling back to loading using the
  // default algorithm should this lookup fail.
  void* handle = dlopen(library_name.c_str(), 0);

  // Clear the error first to ensure that dlerror() called below is not a previous error.
  dlerror();

  // Using RTLD_DEFAULT permits dlsym to look up the symbol from the main process if the handle is
  // nullptr.
  record_counter_ = reinterpret_cast<em_record_counter_t>(
      dlsym(handle ? handle : RTLD_DEFAULT, "em_record_counter"));

  // Fail gracefully should symbol resoulution fail.
  if (record_counter_ == nullptr) {
    throw EnvoyException(
        fmt::format("failed to load stats dynamic symbol from {} {}", library_name, dlerror()));
  }
}

void Sink::flush(Stats::MetricSnapshot& snapshot) {
  // No-op if the symbol failed to resolve.
  if (!record_counter_) {
    return;
  }

  // For each counter, construct a set of C types that can be used to pass the counter information
  // over to the dynamic library.
  for (auto counter : snapshot.counters()) {
    // Ignore counters that haven't changed since last flush.
    if (counter.delta_ == 0) {
      continue;
    }

    // To keep things simple, we stringify the counters on each flush. This is not the most
    // efficient way of doing things, and should this become a problem we should look into
    // supporting stats handles via the dynamic API.
    const std::string name = counter.counter_.get().tagExtractedName();
    const em_string_view name_view{name.c_str(), name.size()};

    const Stats::TagVector tags = counter.counter_.get().tags();
    const auto tags_view = std::unique_ptr<em_metric_tag[]>(new em_metric_tag[tags.size()]);

    size_t index = 0;
    for (const auto& t : tags) {
      tags_view[index].key = em_string_view{t.name_.c_str(), t.name_.size()};
      tags_view[index].value = em_string_view{t.value_.c_str(), t.value_.size()};
      index++;
    }

    record_counter_(name_view, tags_view.get(), tags.size(), counter.delta_);
  }
}

} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
