#include "source/common/network/utility.h"

struct ProxySettings;

using ProxySettingsConstSharedPtr = std::shared_ptr<const ProxySettings>;

/**
 * Proxy settings coming from platform specific APIs, i.e. ConnectivityManager in
 * the case of Android platform.
 *
 */
struct ProxySettings {
  /**
   * @brief Construct a new Proxy Settings object.
   *
   * @param host The proxy host defined as a hostname or an IP address. Some platforms
   *             (i.e., Android) allow users to specify proxy using either one of these.
   * @param port The proxy port.
   */
  ProxySettings(const std::string& host, const uint16_t port)
      : host_(host), port_(port) {}

  /**
   * @brief Returns an address of a proxy. This method returns nullptr for proxy settings
   *        that are initialized with a host represted using a hostname.
   *
   * @return Address of a proxy or nullptr if proxy address is incorrect or host is
   *         defined using a hostname and not an IP address.
   */
  Envoy::Network::Address::InstanceConstSharedPtr address() const {
    return Envoy::Network::Utility::parseInternetAddressNoThrow(host_, port_);
  }

private:
  std::string host_;
  uint16_t port_;
};
