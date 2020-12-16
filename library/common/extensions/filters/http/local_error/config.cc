#include "library/common/extensions/filters/http/local_error/config.h"

#include "library/common/extensions/filters/http/local_error/filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

Http::FilterFactoryCb LocalErrorFilterFactory::createFilterFactoryFromProtoTyped(
    const envoymobile::extensions::filters::http::local_error::LocalError& proto_config,
    const std::string&, Server::Configuration::FactoryContext&) {

  LocalErrorFilterConfigSharedPtr filter_config =
      std::make_shared<LocalErrorFilterConfig>(proto_config);
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamEncoderFilter(std::make_shared<LocalErrorFilter>(filter_config));
  };
}

/**
 * Static registration for the LocalError filter. @see NamedHttpFilterConfigFactory.
 */
REGISTER_FACTORY(LocalErrorFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
