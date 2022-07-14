#include "library/common/extensions/filters/http/socket_tag/filter.h"

#include "envoy/server/filter_config.h"
#include "source/common/common/scalar_to_byte_vector.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketTag {

namespace {

class SocketTagSocketOptionImpl : public Network::Socket::Option, Logger::Loggable<Logger::Id::connection> {
public:
  SocketTagSocketOptionImpl(envoy::config::core::v3::SocketOption::SocketState in_state,
                            uid_t uid, uint32_t traffic_stats_tag)
      : in_state_(in_state), optname_(0, 0, "socket_tag"), uid_(uid), traffic_stats_tag_(traffic_stats_tag) {}

  // Socket::Option
  bool setOption(Network::Socket& socket,
                 envoy::config::core::v3::SocketOption::SocketState state) const override;
  void hashKey(std::vector<uint8_t>& hash_key) const override;
  absl::optional<Details>
  getOptionDetails(const Network::Socket& socket,
                   envoy::config::core::v3::SocketOption::SocketState state) const override;
  bool isSupported() const override;

private:
  const envoy::config::core::v3::SocketOption::SocketState in_state_;
  const Network::SocketOptionName optname_;

  // UID to tag with.
  uid_t uid_;
  // TrafficStats tag to tag with.
  int32_t traffic_stats_tag_;
};

bool SocketTagSocketOptionImpl::setOption(
    Network::Socket& /*socket*/, envoy::config::core::v3::SocketOption::SocketState state) const {
  if (state != in_state_) {
    return true;
  }

  if (!isSupported()) {
    ENVOY_LOG(warn, "Failed to set unsupported option on socket");
    return false;
  }

  //tag_->apply(socket.ioHandle());
  return true;
}

void SocketTagSocketOptionImpl::hashKey(std::vector<uint8_t>& hash_key) const {
  pushScalarToByteVector(uid_, hash_key);
  pushScalarToByteVector(traffic_stats_tag_, hash_key);
}

absl::optional<Network::Socket::Option::Details> SocketTagSocketOptionImpl::getOptionDetails(
    const Network::Socket&, envoy::config::core::v3::SocketOption::SocketState state) const {
  if (state != in_state_ || !isSupported()) {
    return absl::nullopt;
  }

  static std::string name = "socket_tag";
  Network::Socket::Option::Details details;
  details.name_ = optname_;
  //  details.value_ = tag_->dataForLogging();
  return absl::make_optional(std::move(details));
}

bool SocketTagSocketOptionImpl::isSupported() const { return optname_.hasValue(); }

} // namespace

Http::FilterHeadersStatus SocketTagFilter::decodeHeaders(Http::RequestHeaderMap& request_headers, bool) {
  auto socket_tag = Http::LowerCaseString("socket-tag");
  if (!request_headers.get(socket_tag).empty()) {
    std::string tag_string(request_headers.get(socket_tag)[0]->value().getStringView());
    /*
    Network::MockSocketTag* tag = new Network::MockSocketTag;
    EXPECT_CALL(*tag, hashKey(testing::_))
            .WillOnce(testing::Invoke([=](std::vector<uint8_t>& key) {
              pushScalarToByteVector(StringUtil::CaseInsensitiveHash()(tag_string), key);
            }));
    EXPECT_CALL(*tag, apply(testing::_));
    Network::SocketTagSharedPtr st(tag);
    callbacks_->addUpstreamSocketOptions(Network::SocketOptionFactory::buildSocketTagOptions(st));
    */
    request_headers.remove(socket_tag);
  }
  return Http::FilterHeadersStatus::Continue;
}

void SocketTagFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

} // namespace SocketTag
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
