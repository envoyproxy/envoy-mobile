#include "library/common/common/lambda_logger_delegate.h"

#include <iostream>

#include "common/common/lock_guard.h"

namespace Envoy {
namespace Logger {

LambdaDelegate::LambdaDelegate(FlushCb flush_callback, DelegatingLogSinkSharedPtr log_sink)
    : SinkDelegate(log_sink), flush_callback_(flush_callback) {
  setDelegate();
}

LambdaDelegate::~LambdaDelegate() { restoreDelegate(); }

void LambdaDelegate::log(absl::string_view msg) {
  Thread::LockGuard lock(mutex_);
  // std::cerr << "log" << std::endl;
  // absl::StrAppend(&to_flush_, msg);
  flush_callback_(std::string(msg));
}

void LambdaDelegate::flush() {
  // Thread::LockGuard lock(mutex_);
  // std::cerr << "flush" << std::endl;
  // std::cerr << std::flush;
  // flush_callback_(to_flush_);
  // to_flush_ = "";
}

} // namespace Logger
} // namespace Envoy
