#pragma once

#include <string>

#include "common/common/logger.h"
#include "common/common/thread.h"

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Logger {

class LambdaDelegate : public SinkDelegate {
public:
  using LogCb = std::function<void(absl::string_view)>;
  using FlushCb = std::function<void()>;

  LambdaDelegate(LogCb log_callback, FlushCb flush_callback, DelegatingLogSinkSharedPtr log_sink);
  ~LambdaDelegate() override;

  // SinkDelegate
  void log(absl::string_view msg) override;
  void flush() override;

private:
  Thread::MutexBasicLockable mutex_;
  LogCb log_callback_ ABSL_GUARDED_BY(mutex_);
  FlushCb flush_callback_ ABSL_GUARDED_BY(mutex_);
};

using LambdaDelegatePtr = std::unique_ptr<LambdaDelegate>;

} // namespace Logger
} // namespace Envoy
