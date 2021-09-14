#include "library/common/extensions/filters/http/socket_selection/filter.h"

#include "envoy/server/filter_config.h"

#include "library/common/network/mobile_utility.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketSelection {

std::atomic<int> network_override{0};

Http::FilterHeadersStatus SocketSelectionFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  ASSERT(decoder_callbacks_);
  ENVOY_LOG(debug, "SocketSelectionFilter::decodeHeaders");

  network_override ^= 1;
  envoy_network_t network = static_cast<envoy_network_t>(network_override.load());
  ENVOY_LOG(debug, "SocketSelectionFilter NETWORK OVERRIDE: {}", network);

  auto connection_options = Network::MobileUtility::getUpstreamSocketOptions(network);
  decoder_callbacks_->addUpstreamSocketOptions(connection_options);

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
