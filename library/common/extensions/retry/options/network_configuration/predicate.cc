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

  const auto& stream_info = parameters.retriable_request_stream_info_;
  auto& filter_state = const_cast<StreamInfo::FilterState&>(stream_info.filterState());
  if (!filter_state.hasData<StreamInfo::ExtraStreamInfo>(StreamInfo::ExtraStreamInfo::key())) {
    return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{absl::nullopt};
  }

  auto& extra_stream_info =
      filter_state.getDataMutable<StreamInfo::ExtraStreamInfo>(StreamInfo::ExtraStreamInfo::key());
  if (!extra_stream_info.configuration_key_.has_value()) {
    return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{absl::nullopt};
  }

  bool fault = !stream_info.firstUpstreamRxByteReceived().has_value();
  network_configurator_->reportNetworkUsage(extra_stream_info.configuration_key_.value(), fault);
  extra_stream_info.configuration_key_ = network_configurator_->addUpstreamSocketOptions(options);

  return Upstream::RetryOptionsPredicate::UpdateOptionsReturn{options};
}

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
