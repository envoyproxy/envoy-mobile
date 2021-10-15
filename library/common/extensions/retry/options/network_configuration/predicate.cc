#include "library/common/extensions/retry/options/network_configuration/predicate.h"

#include "library/common/stream_info/extra_stream_info.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

NetworkConfigurationRetryOptionsPredicate::NetworkConfigurationRetryOptionsPredicate(
    const envoymobile::extensions::retry::options::network_configuration::
        NetworkConfigurationOptionsPredicate&,
    Upstream::RetryExtensionFactoryContext& context) {
  network_configurator_ = Network::ConfiguratorHandle{context.singletonManager()}.get();
  RELEASE_ASSERT(network_configurator_ != nullptr, "unexpected nullptr network configurator");
}

Upstream::RetryOptionsPredicate::UpdateOptionsReturn
NetworkConfigurationRetryOptionsPredicate::updateOptions(
    const Upstream::RetryOptionsPredicate::UpdateOptionsParameters& parameters) const {

  auto options = std::make_shared<Network::Socket::Options>();

  auto& stream_info = parameters.retriable_request_stream_info_;
  auto filter_state = stream_info.filterState();
  // ExtraStreamInfo is added by the NetworkConfigurationFilter and should normally always be
  // present - this check is mostly defensive.
  if (!filter_state->hasData<StreamInfo::ExtraStreamInfo>(StreamInfo::ExtraStreamInfo::key())) {
    ENVOY_LOG(warn, "extra stream info is missing");
    return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{absl::nullopt};
  }

  auto& extra_stream_info =
      filter_state->getDataMutable<StreamInfo::ExtraStreamInfo>(StreamInfo::ExtraStreamInfo::key());
  if (!extra_stream_info.configuration_key_.has_value()) {
    return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{absl::nullopt};
  }

  bool fault = !stream_info.firstUpstreamRxByteReceived().has_value();
  // Report request status to network configurator, so that socket configuration may be adapted
  // to current network conditions.
  network_configurator_->reportNetworkUsage(extra_stream_info.configuration_key_.value(), fault);
  // Update socket configuration for next retry attempt.
  extra_stream_info.configuration_key_ = network_configurator_->addUpstreamSocketOptions(options);

  return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{options};
}

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
