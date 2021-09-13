#include "library/common/extensions/filters/http/socket_selection/filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketSelection {

Http::FilterHeadersStatus SocketSelectionFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  ASSERT(decoder_callbacks_);
  ENVOY_LOG(debug, "SocketSelectionFilter::decodeHeaders");
  //auto& info = decoder_callbacks_->streamInfo();

  return Http::FilterHeadersStatus::Continue;
}

Http::LocalErrorStatus SocketSelectionFilter::onLocalReply(const LocalReplyData& reply) {
  ENVOY_LOG(debug, "SocketSelectionFilter::onLocalReply({}, {})", reply.code_, reply.details_);
  ASSERT(decoder_callbacks_);
  auto& info = decoder_callbacks_->streamInfo();
  // TODO(goaway): set responseCode in upstream Envoy when responseCodDetails are set.
  // ASSERT(static_cast<uint32_t>(reply.code_) == info.responseCode());
  // TODO(goaway): follow up on the underscore discrepancy between these values.
  // ASSERT(reply.details_ == info.responseCodeDetails().value());
  info.setResponseCode(static_cast<uint32_t>(reply.code_));

  // Charge failures to the upstream cluster to allow for passive healthchecking.
  if (auto upstream = info.upstreamHost()) {
    ENVOY_LOG(trace, "SocketSelectionFilter::onLocalReply charging failure to upstream host");
    upstream->outlierDetector().putHttpResponseCode(static_cast<uint64_t>(reply.code_));
  }
  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace SocketSelection
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
