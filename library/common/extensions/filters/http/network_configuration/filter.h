#pragma once

#include "envoy/http/filter.h"

#include "source/common/common/logger.h"
#include "source/extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/network_configuration/filter.pb.h"
#include "library/common/network/configurator.h"
#include "library/common/types/c_types.h"
#include "library/common/stream_info/extra_stream_info.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace NetworkConfiguration {

/**
 * Filter to set upstream socket options based on network conditions.
 */
class NetworkConfigurationFilter final : public Http::PassThroughFilter,
                                         public Logger::Loggable<Logger::Id::filter> {
public:
  NetworkConfigurationFilter(Network::ConfiguratorSharedPtr network_configurator,
                             bool enable_interface_binding)
      : network_configurator_(network_configurator), tmp_extra_stream_info_(std::make_unique<StreamInfo::ExtraStreamInfo>()),
        extra_stream_info_(*tmp_extra_stream_info_),
        enable_interface_binding_(enable_interface_binding) {}

  // Http::StreamDecoderFilter
  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;
  // Http::StreamEncoderFilter
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap&, bool) override;
  // Http::StreamFilterBase
  Http::LocalErrorStatus onLocalReply(const LocalReplyData&) override;

private:
  Network::ConfiguratorSharedPtr network_configurator_;
  // Don't use this; it will be moved into FilterState.
  StreamInfo::ExtraStreamInfoPtr tmp_extra_stream_info_;
  // Use this instead.
  StreamInfo::ExtraStreamInfo& extra_stream_info_;
  bool enable_interface_binding_;
};

} // namespace NetworkConfiguration
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
