#include "library/common/network/mobile_utility.h"

#include "envoy/common/platform.h"

#include "source/common/common/assert.h"

#ifdef SUPPORTS_GETIFADDRS
#include <ifaddrs.h>
#endif

namespace Envoy {
namespace Network {

std::vector<std::string> MobileUtility::enumerateInterfaces() {
  std::vector<std::string> names{};

#ifdef SUPPORTS_GETIFADDRS
  struct ifaddrs* interfaces = nullptr;
  struct ifaddrs* ifa = nullptr;

  const int rc = getifaddrs(&interfaces);
  RELEASE_ASSERT(!rc, "getifaddrs failed");

  for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) {
      continue;
    }

    // TODO(goaway): determine if additional special handling is needed for IPv6
    if (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6) {
      names.push_back(std::string{ifa->ifa_name});
    }
  }

  freeifaddrs(interfaces);
#endif

  return names;
}

} // namespace Network
} // namespace Envoy
