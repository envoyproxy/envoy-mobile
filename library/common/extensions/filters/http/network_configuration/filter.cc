#include "library/common/extensions/filters/http/network_configuration/filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace NetworkConfiguration {

void NetworkConfigurationFilter::setDecoderFilterCallbacks(
    Http::StreamDecoderFilterCallbacks& callbacks) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::setDecoderFilterCallbacks");

  auto new_extra_stream_info = std::make_unique<StreamInfo::ExtraStreamInfo>();
  extra_stream_info_ = new_extra_stream_info.get();

  decoder_callbacks_ = &callbacks;
  decoder_callbacks_->streamInfo().filterState()->setData(
      StreamInfo::ExtraStreamInfo::key(), std::move(new_extra_stream_info),
      StreamInfo::FilterState::StateType::Mutable, StreamInfo::FilterState::LifeSpan::Request);

  auto options = std::make_shared<Network::Socket::Options>();
  network_configurator_->setInterfaceBindingEnabled(enable_interface_binding_);
  extra_stream_info_->configuration_key_ = network_configurator_->addUpstreamSocketOptions(options);
  decoder_callbacks_->addUpstreamSocketOptions(options);
}

Http::FilterHeadersStatus NetworkConfigurationFilter::encodeHeaders(Http::ResponseHeaderMap&,
                                                                    bool) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::encodeHeaders");
  // Report request status to network configurator, so that socket configuration may be adapted
  // to current network conditions.
  network_configurator_->reportNetworkUsage(extra_stream_info_->configuration_key_.value(), false);

  return Http::FilterHeadersStatus::Continue;
}

Http::LocalErrorStatus NetworkConfigurationFilter::onLocalReply(const LocalReplyData& reply) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::onLocalReply");

  bool success_status = static_cast<int>(reply.code_) < 400;
  bool fault = !success_status &&
               !decoder_callbacks_->streamInfo().firstUpstreamRxByteReceived().has_value();
  // Report request status to network configurator, so that socket configuration may be adapted
  // to current network conditions.
  network_configurator_->reportNetworkUsage(extra_stream_info_->configuration_key_.value(), fault);

  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace NetworkConfiguration
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
