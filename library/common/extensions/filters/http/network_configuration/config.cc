#include "library/common/extensions/filters/http/network_configuration/config.h"

#include "library/common/extensions/filters/http/network_configuration/filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace NetworkConfiguration {

Http::FilterFactoryCb NetworkConfigurationFilterFactory::createFilterFactoryFromProtoTyped(
    const envoymobile::extensions::filters::http::network_configuration::NetworkConfiguration&,
    const std::string&, Server::Configuration::FactoryContext& context) {

  Network::ConfiguratorHandle network_configurator_handle{context.singletonManager()};
  auto network_configurator = network_configurator_handle.get();

  return [network_configurator](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamFilter(std::make_shared<NetworkConfigurationFilter>(network_configurator));
  };
}

/**
 * Static registration for the NetworkConfiguration filter. @see NamedHttpFilterConfigFactory.
 */
REGISTER_FACTORY(NetworkConfigurationFilterFactory,
                 Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace NetworkConfiguration
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
