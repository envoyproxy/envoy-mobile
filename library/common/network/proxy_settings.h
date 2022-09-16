#include "source/common/network/utility.h"

namespace Envoy {
namespace Network {

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
      : address_(Envoy::Network::Utility::parseInternetAddressNoThrow(host, port)) {}

  /**
   * @brief Returns an address of a proxy. This method returns nullptr for proxy settings
   *        that are initialized with anything other than an IP address.
   *
   * @return Address of a proxy or nullptr if proxy address is incorrect or host is
   *         defined using a hostname and not an IP address.
   */
  const Envoy::Network::Address::InstanceConstSharedPtr& address() const { return address_; }

  /**
   * @brief Returns a human readable representation of the proxy settings represented
   *        by the receiver
   *
   * @return const A human readable representation of the receiver.
   */
  const std::string& asString() const { return address_->asString(); }

  bool operator==(ProxySettings const& rhs) const {
    if (this->address() == nullptr || rhs.address() == nullptr) {
      return this->address() == nullptr && rhs.address() == nullptr;
    }

    return this->address()->asString() == rhs.address()->asString();
  }

  bool operator!=(ProxySettings const& rhs) const { return !(*this == rhs); }

private:
  Envoy::Network::Address::InstanceConstSharedPtr address_;
};

} // namespace Network
} // namespace Envoy
