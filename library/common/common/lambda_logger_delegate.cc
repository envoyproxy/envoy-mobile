#include "library/common/common/lambda_logger_delegate.h"
#include <iostream>

#include "common/common/lock_guard.h"

namespace Envoy {
namespace Logger {

LambdaDelegate::LambdaDelegate(FlushCb flush_callback, DelegatingLogSinkSharedPtr log_sink): SinkDelegate(log_sink), flush_callback_(flush_callback) {
  // std::cerr << "Setting Lambda Delegate" << std::endl;
  setDelegate();
}

LambdaDelegate::~LambdaDelegate() { restoreDelegate(); }

void LambdaDelegate::log(absl::string_view msg) {
  // Thread::LockGuard lock(mutex_);
  // absl::StrAppend(&to_flush_, msg);
  previousDelegate()->log(msg);
}

void LambdaDelegate::flush() {
  // Thread::LockGuard lock(mutex_);
  // std::cerr << "Flushing:: " << to_flush_ << std::endl;
  // flush_callback_(to_flush_);
  // to_flush_ = "";
  previousDelegate()->flush();
}

} // namespace Logger
} // namespace Envoy
