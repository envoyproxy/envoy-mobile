#include "library/common/extensions/filters/http/local_error/filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

LocalErrorFilter::LocalErrorFilter() {}

Http::LocalErrorStatus LocalErrorFilter::onLocalReply(const LocalReplyData& reply) {
  auto& info = decoder_callbacks_->streamInfo();
  ASSERT(static_cast<uint32_t>(reply.code_) == info.responseCode());
  ASSERT(reply.details_ == info.responseCodeDetails());
  info.setResponseCode(static_cast<uint32_t>(reply.code_));

  // Charge failures to the upstream cluster to allow for passive healthchecking.
  if (auto upstream = info.upstreamHost()) {
    upstream->outlierDetector().putHttpResponseCode(static_cast<uint64_t>(reply.code_));
  }

  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
