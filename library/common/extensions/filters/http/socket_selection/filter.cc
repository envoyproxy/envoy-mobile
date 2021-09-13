#include "library/common/extensions/filters/http/socket_selection/filter.h"

#include "envoy/server/filter_config.h"
#include "source/common/network/upstream_socket_options_filter_state.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketSelection {

Http::FilterHeadersStatus SocketSelectionFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  ASSERT(decoder_callbacks_);
  ENVOY_LOG(debug, "SocketSelectionFilter::decodeHeaders");

  auto network_selection_options = std::make_shared<Network::Socket::Options>();
  decoder_callbacks_->addUpstreamSocketOptions(network_selection_options);

  return Http::FilterHeadersStatus::Continue;
}

Http::LocalErrorStatus SocketSelectionFilter::onLocalReply(const LocalReplyData& reply) {
  ENVOY_LOG(debug, "SocketSelectionFilter::onLocalReply({}, {})", reply.code_, reply.details_);
  ASSERT(decoder_callbacks_);
  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace SocketSelection
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
