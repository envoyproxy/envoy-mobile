#pragma once

#include <string>

#include "common/common/logger.h"
#include "common/common/thread.h"

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Logger {

class LambdaDelegate : public SinkDelegate {
public:
  using FlushCb = std::function<void(std::string)>;
  LambdaDelegate(FlushCb flush_callback, DelegatingLogSinkSharedPtr log_sink);
  ~LambdaDelegate() override;

  // SinkDelegate
  void log(absl::string_view msg) override;
  void flush() override;

private:
  Thread::MutexBasicLockable mutex_;
  FlushCb flush_callback_;
  std::string to_flush_ ABSL_GUARDED_BY(mutex_);
};

} // namespace Logger
} // namespace Envoy
