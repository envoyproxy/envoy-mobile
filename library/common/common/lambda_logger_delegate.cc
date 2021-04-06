#include "library/common/common/lambda_logger_delegate.h"

#include <iostream>

namespace Envoy {
namespace Logger {

LambdaDelegate::LambdaDelegate(LogCb log_callback, FlushCb flush_callback,
                               DelegatingLogSinkSharedPtr log_sink)
    : SinkDelegate(log_sink), log_callback_(log_callback), flush_callback_(flush_callback) {
  setDelegate();
}

LambdaDelegate::~LambdaDelegate() { restoreDelegate(); }

void LambdaDelegate::log(absl::string_view msg) { log_callback_(msg); }

void LambdaDelegate::flush() { flush_callback_(); }

} // namespace Logger
} // namespace Envoy
