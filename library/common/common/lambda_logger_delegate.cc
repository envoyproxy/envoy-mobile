#include "library/common/common/lambda_logger_delegate.h"

#include <iostream>

#include "library/common/bridge/utility.h"
#include "library/common/data/utility.h"

namespace Envoy {
namespace Logger {

void EventTrackingDelegate::logWithStableName(absl::string_view stable_name, absl::string_view,
                                              absl::string_view, absl::string_view msg) {
  std::cout << "logging stable name " << stable_name << std::endl;
  if (event_tracker_.track == nullptr) {
    return;
  }

  event_tracker_.track(Bridge::makeEnvoyMap({{"name", "envoy_event_log"},
                                             {"log_name", std::string(stable_name)},
                                             {"msg", std::string(msg)}}),
                       event_tracker_.context);
}

LambdaDelegate::LambdaDelegate(envoy_logger logger, DelegatingLogSinkSharedPtr log_sink)
    : EventTrackingDelegate(log_sink), logger_(logger) {
  setDelegate();
}

LambdaDelegate::~LambdaDelegate() {
  restoreDelegate();
  logger_.release(logger_.context);
}

void LambdaDelegate::log(absl::string_view msg) {
  logger_.log(Data::Utility::copyToBridgeData(msg), logger_.context);
}

DefaultDelegate::DefaultDelegate(absl::Mutex& mutex, DelegatingLogSinkSharedPtr log_sink)
    : EventTrackingDelegate(log_sink), mutex_(mutex) {
  setDelegate();
}

DefaultDelegate::~DefaultDelegate() { restoreDelegate(); }

} // namespace Logger
} // namespace Envoy
