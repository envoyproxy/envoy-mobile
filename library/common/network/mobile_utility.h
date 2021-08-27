#pragma once

#include <string>
#include <vector>

namespace Envoy {
namespace Network {

/**
 * Network utility routines related to mobile clients.
 */
class MobileUtility {
public:
  static std::vector<std::string> enumerateInterfaces();
};

} // namespace Network
} // namespace Envoy
