#include "library/common/network/socket_tag_socket_option_impl.h"

#include "envoy/config/core/v3/base.pb.h"
#include "library/common/jni/android_jni_utility.h"
#include "source/common/common/assert.h"
#include "source/common/common/scalar_to_byte_vector.h"

namespace Envoy {
namespace Network {

SocketTagSocketOptionImpl::SocketTagSocketOptionImpl(uid_t uid, uint32_t traffic_stats_tag)
      : optname_(0, 0, "socket_tag"), uid_(uid), traffic_stats_tag_(traffic_stats_tag) {}

bool SocketTagSocketOptionImpl::setOption(
    Socket& socket, envoy::config::core::v3::SocketOption::SocketState state) const {
  if (state != envoy::config::core::v3::SocketOption::STATE_PREBIND) {
    return true;
  }

  if (!isSupported()) {
    //ENVOY_LOG(warn, "Failed to set unsupported option on socket");
    return false;
  }

  int fd = socket.ioHandle().fdDoNotUse();
  tag_socket(fd, 0, 0);
  tag_socket(fd, uid_, traffic_stats_tag_);
  return true;
}

void SocketTagSocketOptionImpl::hashKey(std::vector<uint8_t>& hash_key) const {
  pushScalarToByteVector(uid_, hash_key);
  pushScalarToByteVector(traffic_stats_tag_, hash_key);
}

absl::optional<Socket::Option::Details> SocketTagSocketOptionImpl::getOptionDetails(
    const Socket&, envoy::config::core::v3::SocketOption::SocketState /*state*/) const {
  if (!isSupported()) {
    return absl::nullopt;
  }

  static std::string name = "socket_tag";
  Socket::Option::Details details;
  details.name_ = optname_;
  //  details.value_ = tag_->dataForLogging();
  return absl::make_optional(std::move(details));
}

bool SocketTagSocketOptionImpl::isSupported() const { return optname_.hasValue(); }

} // namespace Network
} // namespace Envoy
