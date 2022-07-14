#pragma once

#include "envoy/config/core/v3/base.pb.h"
#include "envoy/network/address.h"
#include "envoy/network/listen_socket.h"

namespace Envoy {
namespace Network {

class SocketTag {};

/**
 * This is a "synthetic" socket option implementation, which sets the source IP/port of a socket
 * using a provided IP address (and maybe port) during bind.
 *
 * Based on the OriginalSrcSocketOption extension.
 */
class SocketTagSocketOptionImpl : public Network::Socket::Option {
public:
  SocketTagSocketOptionImpl(uid_t uid, uint32_t traffic_stats_tag);

  // Socket::Option
  bool setOption(Network::Socket& socket,
                 envoy::config::core::v3::SocketOption::SocketState state) const override;
  void hashKey(std::vector<uint8_t>& hash_key) const override;
  absl::optional<Details>
  getOptionDetails(const Network::Socket& socket,
                   envoy::config::core::v3::SocketOption::SocketState state) const override;
  bool isSupported() const override;

private:
  const Network::SocketOptionName optname_;

  uid_t uid_;
  uint32_t traffic_stats_tag_;
};

} // namespace Network
} // namespace Envoy
