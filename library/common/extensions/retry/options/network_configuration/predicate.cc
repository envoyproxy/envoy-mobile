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

  const auto& stream_info = parameters.retriable_request_stream_info_;
  auto& extra_stream_info = const_cast<StreamInfo::ExtraStreamInfo&>(
      stream_info.filterState().getDataReadOnly<StreamInfo::ExtraStreamInfo>(
          StreamInfo::ExtraStreamInfo::key()));

  bool fault = !stream_info.firstUpstreamRxByteReceived().has_value();
  RELEASE_ASSERT(extra_stream_info.configuration_key_.has_value(), "configuration key missing");
  network_configurator_->reportNetworkUsage(extra_stream_info.configuration_key_.value(), fault);

  auto options = std::make_shared<Network::Socket::Options>();
  extra_stream_info.configuration_key_ = network_configurator_->addUpstreamSocketOptions(options);
  Upstream::RetryOptionsPredicate::UpdateOptionsReturn ret{options};
  return ret;
}

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
