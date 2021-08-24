#pragma once

#include <string>

#include "source/common/common/logger.h"

#include "library/common/api/external.h"
#include <iostream>
#include "absl/strings/string_view.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Logger {

class EventTrackingDelegate : public SinkDelegate {
public:
  EventTrackingDelegate(DelegatingLogSinkSharedPtr log_sink)
      : SinkDelegate(log_sink), event_tracker_(*static_cast<envoy_event_tracker*>(
                                    Api::External::retrieveApi(envoy_event_tracker_api_name))) {}

  // void logWithStableName(absl::string_view stable_name, absl::string_view level,
  // absl::string_view component, absl::string_view msg) override {
  //   if (event_tracker_.track == nullptr) {
  //     return;
  //   }

  //   if (stable_name == whatever) {
  //     event_tracker_.track(makeEnvoyMap({{"name", stable_name}, {"msg", msg}}));
  //   }
  // }

private:
  envoy_event_tracker& event_tracker_;
};

using EventTrackingDelegatePtr = std::unique_ptr<EventTrackingDelegate>;
class LambdaDelegate : public EventTrackingDelegate {
public:
  LambdaDelegate(envoy_logger logger, DelegatingLogSinkSharedPtr log_sink);
  ~LambdaDelegate() override;

  // SinkDelegate
  void log(absl::string_view msg) override;
  // Currently unexposed. May be desired in the future.
  void flush() override{};

private:
  envoy_logger logger_;
};

// A default log delegate that logs to stderr, mimicing the default used by Envoy
// when no logger has been installed. Using this default delegate allows us to
// intercept the named log lines (used for analytic events) even if no platform
// logger has been installed.
class DefaultDelegate : public EventTrackingDelegate {
public:
  DefaultDelegate(absl::Mutex& mutex, DelegatingLogSinkSharedPtr log_sink);
  ~DefaultDelegate() override;

  // SinkDelegate
  void log(absl::string_view msg) override {
    absl::MutexLock l(&mutex_);
    std::cerr << msg;
  }
  void flush() override {
    absl::MutexLock l(&mutex_);
    std::cerr << std::flush;
  };

private:
  absl::Mutex& mutex_;
};

} // namespace Logger
} // namespace Envoy
