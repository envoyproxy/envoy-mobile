#pragma once

#include "envoy/http/filter.h"

#include "source/common/common/logger.h"
#include "source/extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/network_configuration/filter.pb.h"
#include "library/common/network/configurator.h"
#include "library/common/types/c_types.h"

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
  NetworkConfigurationFilter(Network::ConfiguratorSharedPtr network_configurator)
      : network_configurator_(network_configurator) {}

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;

private:
  Network::ConfiguratorSharedPtr network_configurator_;
};

} // namespace NetworkConfiguration
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
