#pragma once

#include <string>
#include <vector>

#include "envoy/network/socket.h"
#include "envoy/singleton/manager.h"

#include "source/extensions/common/dynamic_forward_proxy/dns_cache_impl.h"

#include "library/common/types/c_types.h"

/**
 * This type identifies a snapshot of network configuration state. It's returned from calls that
 * may alter current state, and passed back as a parameter to this API to determine if calls
 * remain valid/relevant at time of execution.
 */
typedef uint16_t envoy_netconf_t;

/**
 * These values specificy the behavior of the network configurator with respect to the upstream
 * socket options it supplies.
 */
typedef enum {
  // In this mode, the configurator will provide socket options that result in the creation of a
  // distinct connection pool for a given value of preferred network.
  DefaultPreferredNetworkMode = 0,
  // In this mode, the configurator will provide socket options that intentionally attempt to
  // override the current preferred network type with an alternative, via interface-binding socket
  // options. Note this mode is experimental, and it will not be enabled at all unless
  // enable_interface_binding_ is set to true.
  AlternateBoundInterfaceMode = 1,
} envoy_socket_mode_t;

namespace Envoy {
namespace Network {

using DnsCacheManagerSharedPtr = Extensions::Common::DynamicForwardProxy::DnsCacheManagerSharedPtr;

/**
 * Object responsible for tracking network state, especially with respect to multiple interfaces,
 * and providing auxiliary configuration to network connections, in the form of upstream socket
 * options.
 */
class Configurator : public Logger::Loggable<Logger::Id::upstream>, public Singleton::Instance {
public:
  Configurator(DnsCacheManagerSharedPtr dns_cache_manager)
      : enable_interface_binding_{false}, dns_cache_manager_(dns_cache_manager) {}

  /**
   * @returns a list of local network interfaces supporting IPv4.
   */
  std::vector<std::string> enumerateV4Interfaces();

  /**
   * @returns a list of local network interfaces supporting IPv6.
   */
  std::vector<std::string> enumerateV6Interfaces();

  /**
   * @param family, network family of the interface.
   * @param select_flags, flags which MUST be set for each returned interface.
   * @param reject_flags, flags which MUST NOT be set for any returned interface.
   * @returns a list of local network interfaces filtered by the providered flags.
   */
  std::vector<std::string> enumerateInterfaces(unsigned short family, unsigned int select_flags,
                                               unsigned int reject_flags);

  /**
   * @returns the current OS default/preferred network class.
   */
  envoy_network_t getPreferredNetwork();

  /**
   * @returns the current mode used to determine upstream socket options.
   */
  envoy_socket_mode_t getSocketMode();

  /**
   * @returns configuration key representing current network state.
   */
  envoy_netconf_t getConfigurationKey();

  /**
   * Call to report on the current viability of the passed network configuration after an attempt
   * at transmission (e.g., an HTTP request).
   * @param network_fault, whether a transmission attempt terminated w/o receiving upstream bytes.
   */
  void reportNetworkUsage(envoy_netconf_t configuration_key, bool network_fault);

  /**
   * Sets the current OS default/preferred network class.
   * @param network, the OS-preferred network.
   * @returns configuration key to associate with any related calls.
   */
  static envoy_netconf_t setPreferredNetwork(envoy_network_t network);

  /**
   * Sets whether subsequent calls for upstream socket options may leverage options that bind
   * to specific network interfaces.
   * @param enabled, whether to enable interface binding.
   */
  void setInterfaceBindingEnabled(bool enabled);

  /**
   * Refresh DNS in response to preferred network update. May be no-op.
   * @param configuration_key, key provided by this class representing the current configuration.
   */
  void refreshDns(envoy_netconf_t configuration_key);

  /**
   * @returns the current socket options that should be used for connections.
   */
  Socket::OptionsSharedPtr getUpstreamSocketOptions(envoy_network_t network,
                                                    envoy_socket_mode_t socket_mode);

  /**
   * @param options, upstream connection options to which additional options should be appended.
   * @returns configuration key to associate with any related calls.
   */
  envoy_netconf_t addUpstreamSocketOptions(Socket::OptionsSharedPtr options);

private:
  struct NetworkState {
    // The configuration key is passed through calls dispatched on the run loop to determine if
    // they're still valid/relevant at time of execution.
    envoy_netconf_t configuration_key_ ABSL_GUARDED_BY(mutex_);
    envoy_network_t network_ ABSL_GUARDED_BY(mutex_);
    uint8_t remaining_faults_ ABSL_GUARDED_BY(mutex_);
    envoy_socket_mode_t socket_mode_ ABSL_GUARDED_BY(mutex_);
    Thread::MutexBasicLockable mutex_;
  };
  Socket::OptionsSharedPtr getAlternateInterfaceSocketOptions(envoy_network_t network);
  const std::string getActiveAlternateInterface(envoy_network_t network, unsigned short family);

  bool enable_interface_binding_;
  DnsCacheManagerSharedPtr dns_cache_manager_;
  static NetworkState network_state_;
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
