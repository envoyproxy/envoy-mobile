#pragma once

#include <string>
#include <vector>

#include "envoy/network/socket.h"
#include "envoy/singleton/manager.h"
#include "source/extensions/common/dynamic_forward_proxy/dns_cache_impl.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace Network {

using DnsCacheSharedPtr = Extensions::Common::DynamicForwardProxy::DnsCacheSharedPtr;

/**
 * Network utility routines related to mobile clients.
 */
class Configurator : public Singleton::Instance {
public:
  Configurator(absl::optional<DnsCacheSharedPtr> dns_cache) : dns_cache_(dns_cache) {}

  /**
   * @returns a list of local network interfaces supporting IPv4.
   */
  std::vector<std::string> enumerateV4Interfaces();

  /**
   * @returns a list of local network interfaces supporting IPv6.
   */
  std::vector<std::string> enumerateV6Interfaces();

  /**
   * @returns the current OS default/preferred network class.
   */
  envoy_network_t getPreferredNetwork();

  /**
   * Sets the current OS default/preferred network class.
   * @param network, the network preference.
   */
  static void setPreferredNetwork(envoy_network_t network);

  /**
   * @returns the current socket options that should be used for connections.
   */
  Socket::OptionsSharedPtr getUpstreamSocketOptions(envoy_network_t network);

private:
  std::vector<std::string> enumerateInterfaces(unsigned short family);
  absl::optional<DnsCacheSharedPtr> dns_cache_;
  static std::atomic<envoy_network_t> preferred_network_;
};

using ConfiguratorSharedPtr = std::shared_ptr<Configurator>;

/**
 * Provides access to the singleton Configurator.
 */
class ConfiguratorHandle {
public:
  ConfiguratorHandle(Server::Configuration::FactoryContextBase& context)
      : context_(context) {}

  /**
   * @returns singleton Configurator instance.
   */
  ConfiguratorSharedPtr get();

private:
  Server::Configuration::FactoryContextBase& context_;
};

} // namespace Network
} // namespace Envoy
