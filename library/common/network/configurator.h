#pragma once

#include <string>
#include <vector>

#include "envoy/network/socket.h"
#include "envoy/singleton/manager.h"

#include "source/extensions/common/dynamic_forward_proxy/dns_cache_impl.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace Network {

using DnsCacheManagerSharedPtr = Extensions::Common::DynamicForwardProxy::DnsCacheManagerSharedPtr;

/**
 * Object responsible for tracking network state, especially with respect to multiple interfaces,
 * and providing auxiliary configuration to network conditions.
 *
 * The current version of the Network::Configurator can be configured in 2 modes with these
 * general properties:
 *
 * Standard
 * - Use (effectively) per-network pools of connections.
 * - Trigger a refresh of cached DNS addresses on network change.
 *
 * Bound Interface Overrides
 * - Maintain a separate network_override boolean.
 * - For a given state, after 1+ successes and 3 consecutive faults or 0 successes and 1 fault,
 *   toggle the boolean. For the purpose of this heuristic, a fault is a request that terminates
 *   having received 0 transmitted bytes, and a success is anything else.
 * - When the boolean is true, supply additional socket options that bind new connections to
 *   a different active network interface from the likely OS default, in a best effort fashion.
 * - Trigger a refresh of cached DNS on network state change.
 *
 * Note the latter configuration is experimental. The heuristic and the behavior are subject to
 * change.
 */
class Configurator : public Logger::Loggable<Logger::Id::upstream>, public Singleton::Instance {
public:
  Configurator(DnsCacheManagerSharedPtr dns_cache_manager)
      : dns_cache_manager_(dns_cache_manager) {}

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
   * Call to report on the current viability of the passed network configuration after an attempt
   * at transmission (e.g., an HTTP request).
   * @param network_fault, whether a transmission attempt terminated w/o receiving upstream bytes.
   */
  void reportNetworkUsage(uint64_t configuration_key, bool network_fault);

  /**
   * @returns whether to override the specified network interface based on internal heuristics.
   */
  bool overrideInterface(envoy_network_t network);

  /**
   * Sets the current OS default/preferred network class.
   * @param network, the OS-preferred network.
   * @returns configuration key to associate with any related calls.
   */
  static uint64_t setPreferredNetwork(envoy_network_t network);

  /**
   * Refresh DNS in response to preferred network update. May be no-op.
   * @param configuration_key, key provided by this class representing the current configuration.
   */
  void refreshDns(uint64_t configuration_key);

  /**
   * @returns the current socket options that should be used for connections.
   */
  Socket::OptionsSharedPtr getUpstreamSocketOptions(envoy_network_t network,
                                                    bool override_interface);

  /**
   * @param options, upstream connection options to which additional options should be appended.
   * @returns configuration key to associate with any related calls.
   */
  uint64_t addUpstreamSocketOptions(Socket::OptionsSharedPtr options);

private:
  struct NetworkState {
    // The configuration key is passed through calls dispatched on the run loop to determine if
    // they're still valid/relevant at time of execution.
    uint64_t configuration_key_;
    envoy_network_t network_;
    uint_least8_t remaining_faults_;
    bool overridden_;
  };
  Socket::OptionsSharedPtr getAlternateInterfaceSocketOptions(envoy_network_t network);
  const std::string getActiveAlternateInterface(envoy_network_t network, unsigned short family);
  std::vector<std::string> enumerateInterfaces(unsigned short family, unsigned int select_flags,
                                               unsigned int reject_flags);
  DnsCacheManagerSharedPtr dns_cache_manager_;
  static std::atomic<NetworkState> network_state_;
};

using ConfiguratorSharedPtr = std::shared_ptr<Configurator>;

/**
 * Provides access to the singleton Configurator.
 */
class ConfiguratorFactory {
public:
  ConfiguratorFactory(Server::Configuration::FactoryContextBase& context) : context_(context) {}

  /**
   * @returns singleton Configurator instance.
   */
  ConfiguratorSharedPtr get();

private:
  Server::Configuration::FactoryContextBase& context_;
};

/**
 * Provides nullable access to the singleton Configurator.
 */
class ConfiguratorHandle {
public:
  ConfiguratorHandle(Singleton::Manager& singleton_manager)
      : singleton_manager_(singleton_manager) {}

  /**
   * @returns singleton Configurator instance. Can be nullptr if it hasn't been created.
   */
  ConfiguratorSharedPtr get();

private:
  Singleton::Manager& singleton_manager_;
};

} // namespace Network
} // namespace Envoy
