#include "library/common/extensions/filters/http/test_read/filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TestRead {

Http::FilterHeadersStatus TestReadFilter::decodeHeaders(Http::RequestHeaderMap& request_headers,
                                                        bool) {
  // sample path is /failed?start=0x10000
  std::basic_string_view<char> str = request_headers.Path()->value().getStringView();
  std::vector<std::string> query = absl::StrSplit(str, absl::ByAnyChar("?="));
  ASSERT(query.size() == 3,
         "url path should be in the format: /{status}?{phase}={errorcode} eg /failed?start=0x1000");
  uint64_t response_flag = std::stoul(query[2], nullptr, 16);
  decoder_callbacks_->streamInfo().setResponseFlag(
      TestReadFilter::mapErrorToResponseFlag(response_flag));
  decoder_callbacks_->sendLocalReply(Http::Code::BadRequest, "test_read filter threw: ", nullptr,
                                     absl::nullopt, "");
  return Http::FilterHeadersStatus::StopIteration;
}

StreamInfo::ResponseFlag TestReadFilter::mapErrorToResponseFlag(uint64_t errorCode) {
  switch (errorCode) {
  case 0x4000000:
    return StreamInfo::DnsResolutionFailed;
  case 0x40:
    return StreamInfo::UpstreamConnectionTermination;
  case 0x20:
    return StreamInfo::UpstreamConnectionFailure;
  case 0x10:
    return StreamInfo::UpstreamRemoteReset;
  case 0x10000:
    return StreamInfo::StreamIdleTimeout;
  default:
    // Any other error that we arent interested in, picked a random error
    return StreamInfo::RateLimitServiceError;
  }
}

} // namespace TestRead
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
