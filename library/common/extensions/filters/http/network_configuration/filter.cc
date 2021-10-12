#include "library/common/extensions/filters/http/network_configuration/filter.h"

#include "envoy/server/filter_config.h"

#include "library/common/stream_info/aux_stream_info.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace NetworkConfiguration {

Http::FilterHeadersStatus NetworkConfigurationFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::decodeHeaders");

  auto& aux_stream_info = StreamInfo::AuxProvider::get(decoder_callbacks_->streamInfo());
  auto options = std::make_shared<Network::Socket::Options>();

  network_configurator_->setInterfaceBindingEnabled(enable_interface_binding_);
  aux_stream_info.configuration_key_ = network_configurator_->addUpstreamSocketOptions(options);
  decoder_callbacks_->addUpstreamSocketOptions(options);

  return Http::FilterHeadersStatus::Continue;
}

Http::FilterHeadersStatus NetworkConfigurationFilter::encodeHeaders(Http::ResponseHeaderMap&,
                                                                    bool) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::encodeHeaders");

  auto& aux_stream_info = StreamInfo::AuxProvider::get(decoder_callbacks_->streamInfo());
  network_configurator_->reportNetworkUsage(aux_stream_info.configuration_key_.value(), false);

  return Http::FilterHeadersStatus::Continue;
}

Http::LocalErrorStatus NetworkConfigurationFilter::onLocalReply(const LocalReplyData& reply) {
  ENVOY_LOG(debug, "NetworkConfigurationFilter::onLocalReply");

  auto& stream_info = decoder_callbacks_->streamInfo();
  auto& aux_stream_info = StreamInfo::AuxProvider::get(stream_info);

  bool success_status = static_cast<int>(reply.code_) < 400;
  bool fault = !success_status && !stream_info.firstUpstreamRxByteReceived().has_value();
  network_configurator_->reportNetworkUsage(aux_stream_info.configuration_key_.value(), fault);

  return Http::LocalErrorStatus::ContinueAndResetStream;
}

} // namespace NetworkConfiguration
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
