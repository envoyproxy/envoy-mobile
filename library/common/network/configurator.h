#pragma once

#include <string>
#include <vector>

#include "envoy/network/socket.h"
#include "envoy/singleton/manager.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace Network {

/**
 * Network utility routines related to mobile clients.
 */
class Configurator : public Singleton::Instance {
public:
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
  static std::atomic<envoy_network_t> preferred_network_;
};

using ConfiguratorSharedPtr = std::shared_ptr<Configurator>;

/**
 * Provides access to the singleton Configurator.
 */
class ConfiguratorHandle {
public:
  ConfiguratorHandle(Singleton::Manager& singleton_manager)
    : singleton_manager_(singleton_manager) {}

  /**
   * @returns singleton Configurator instance.
   */
  ConfiguratorSharedPtr get();

private:
  Singleton::Manager& singleton_manager_;
};

} // namespace Network
} // namespace Envoy
