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
  // TODO(goaway): The predicate has no inherent way to know the prior configuration key so we need
  // some way to retrieve it. Options:
  // 1. store the configuration key directly in stream info (but it would need to be non-const)
  // 2. store a unique identifier in stream info that can be keyed in a singleton "extra info" map
  // 3. store a metadata map in stream info (also needs to be non-const here)
  // 4. store in an extension point on stream info for adding extra fields
  // Here we use option 2), but 4) seems like a cleaner long-term strategy.
  RELEASE_ASSERT(extra_stream_info.configuration_key_.has_value(), "extra stream info missing");
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
