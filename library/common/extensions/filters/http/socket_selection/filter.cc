#include "library/common/extensions/filters/http/socket_selection/filter.h"

#include "envoy/server/filter_config.h"

#include "library/common/network/mobile_utility.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketSelection {

Http::FilterHeadersStatus SocketSelectionFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  ASSERT(decoder_callbacks_);
  ENVOY_LOG(debug, "SocketSelectionFilter::decodeHeaders");

  envoy_network_t network = Network::MobileUtility::getPreferredNetwork();
  ENVOY_LOG(debug, "current preferred network: {}", network);

  auto connection_options = Network::MobileUtility::getUpstreamSocketOptions(network);
  decoder_callbacks_->addUpstreamSocketOptions(connection_options);

  return Http::FilterHeadersStatus::Continue;
}

Http::LocalErrorStatus SocketSelectionFilter::onLocalReply(const LocalReplyData&) {
  ASSERT(decoder_callbacks_);
  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace SocketSelection
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
