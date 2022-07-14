#include "source/common/network/utility.h"

struct ProxySettings;

using ProxySettingsConstSharedPtr = std::shared_ptr<const ProxySettings>;

struct ProxySettings {
  ProxySettings(const std::string& hostname, const std::string& address)
      : hostname_(hostname),
        address_(Envoy::Network::Utility::parseInternetAddressNoThrow(address)) {}

  bool isValid() const { return address_ != NULL; }

  std::string hostname_;
  Envoy::Network::Address::InstanceConstSharedPtr address_;
};
