#include "library/common/extensions/filters/http/platform_extension/config.h"
#include "library/common/extensions/filters/http/platform_extension/bridging_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace PlatformExtension {

Http::FilterFactoryCb BridgingFilterFactory::createFilterFactoryFromProtoTyped(
    const envoymobile::extensions::filters::http::platform_extension::Bridging& proto_config,
    const std::string&, Server::Configuration::FactoryContext&) {

  BridgingFilterConfigSharedPtr filter_config = std::make_shared<BridgingFilterConfig>(
      proto_config);
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamFilter(std::make_shared<BridgingFilter>(filter_config));
  };
}

/**
 * Static registration for the Bridging filter. @see NamedHttpFilterConfigFactory.
 */
REGISTER_FACTORY(BridgingFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace PlatformExtension
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
