#include "library/extensions/filters/http/harness/config.h"
#include "library/extensions/filters/http/harness/filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Harness {

Http::FilterFactoryCb HarnessFilterFactory::createFilterFactoryFromProtoTyped(
    const envoymobile::extensions::filters::http::harness::Harness& proto_config,
    const std::string&, Server::Configuration::FactoryContext&) {

  HarnessFilterConfigSharedPtr filter_config = std::make_shared<HarnessFilterConfig>(
      proto_config);
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamFilter(std::make_shared<HarnessFilter>(filter_config));
  };
}

/**
 * Static registration for the harness filter. @see NamedHttpFilterConfigFactory.
 */
REGISTER_FACTORY(HarnessFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace Harness
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
