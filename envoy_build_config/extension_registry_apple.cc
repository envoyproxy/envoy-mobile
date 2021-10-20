#include "extension_registry_platform_additions.h"

#include "source/extensions/network/dns_resolver/apple/apple_dns_impl.h"

namespace Envoy {

void ExtensionRegistryPlatformAdditions::registerFactories() {
  Envoy::Network::forceRegisterAppleDnsResolverFactory();
}

} // namespace Envoy
