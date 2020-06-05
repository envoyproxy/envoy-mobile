#include "library/common/extensions/filter/platform_filter.h"

#include "common/common/assert.h"
#include "common/common/utility.h"

#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Http {

FilterHeadersStatus mapStatus(envoy_filter_headers_status_t status) {
  switch (status) {
  case ENVOY_FILTER_HEADERS_STATUS_CONTINUE:
    return FilterHeadersStatus::Continue;
  case ENVOY_FILTER_HEADERS_STATUS_STOP_ITERATION:
    return FilterHeadersStatus::StopIteration;
  case ENVOY_FILTER_HEADERS_STATUS_CONTINUE_AND_END_STREAM:
    return FilterHeadersStatus::ContinueAndEndStream;
  case ENVOY_FILTER_HEADERS_STATUS_STOP_ALL_ITERATION_AND_BUFFER:
    return FilterHeadersStatus::StopAllIterationAndBuffer;
  default:
    ASSERT(false, "unrecognized filter status from platform: {}");
    return FilterHeadersStatus::Continue;
  }
}

FilterHeadersStatus PlatformExtensionFilter::decodeHeaders(RequestHeaderMap& headers,
                                                           bool end_stream) {
  return mapStatus(platform_filter_.on_request_headers(Utility::toBridgeHeaders(headers),
                                                       end_stream, platform_filter_.context));
}

FilterDataStatus PlatformExtensionFilter::decodeData(Buffer::Instance& /*data*/,
                                                     bool /*end_stream*/) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus PlatformExtensionFilter::decodeTrailers(RequestTrailerMap& /*trailers*/) {
  return FilterTrailersStatus::Continue;
}

FilterMetadataStatus PlatformExtensionFilter::decodeMetadata(MetadataMap& /*metadata*/) {
  return FilterMetadataStatus::Continue;
}

FilterHeadersStatus
PlatformExtensionFilter::encode100ContinueHeaders(ResponseHeaderMap& /*headers*/) {
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus PlatformExtensionFilter::encodeHeaders(ResponseHeaderMap& /*headers*/,
                                                           bool /*end_stream*/) {
  return FilterHeadersStatus::Continue;
}

FilterDataStatus PlatformExtensionFilter::encodeData(Buffer::Instance& /*data*/,
                                                     bool /*end_stream*/) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus PlatformExtensionFilter::encodeTrailers(ResponseTrailerMap& /*trailers*/) {
  return FilterTrailersStatus::Continue;
}

FilterMetadataStatus PlatformExtensionFilter::encodeMetadata(MetadataMap& /*metadata*/) {
  return FilterMetadataStatus::Continue;
}

} // namespace Http
} // namespace Envoy
