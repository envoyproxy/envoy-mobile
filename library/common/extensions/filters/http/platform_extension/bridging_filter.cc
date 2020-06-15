#include "library/common/extensions/filter/platform_filter.h"

#include "envoy/server/filter_config.h"

#include "common/common/assert.h"
#include "common/common/utility.h"

#include "library/common/api/external.h"
#include "library/common/buffer/utility.h"
#include "library/common/http/header_utility.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace PlatformExtension {

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

BridgingFilterConfig::BridgingFilterConfig(
    const envoymobile::extensions::filters::http::platform_extension::Bridging& proto_config)
    : name_(proto_config.name()) {}

BridgingFilter::BridgingFilter(BridgingFilterConfigSharedPtr config)
    platform_filter_(Api::External::retrieveApi(config.name()) {}

FilterHeadersStatus BridgingFilter::decodeHeaders(RequestHeaderMap& headers,
                                                           bool end_stream) {
  // This is a hack that results in a full copy of the header map every time.
  // Viable optimizations include:
  // 1) Only tracking modifications to the map at the bridging/platform layers and performing those modifications.
  // 2) Implementing a fully-bridged header map so that modifications actually occur on the underlying Envoy map.
  envoy_headers incoming_headers = Utility::toBridgeHeaders(headeres);
  envoy_headers* new_headers = &incoming_headers;
  FilterHeadersStatus status = mapStatus(platform_filter_.on_request_headers(&new_headers, end_stream, platform_filter_.context));
  if (&incoming_headers != new_headers) {
    headers.removePrefix(LowerCaseString()); // Remove all headers
    for (envoy_header_size_t i = 0; i < new_headers.length; i++) {
      headers->addCopy(LowerCaseString(convertToString(new_headers.headers[i].key)),
                       convertToString(new_headers.headers[i].value));
    }
  // The C envoy_headers struct can be released now because the headers have been copied.
  release_envoy_headers(new_headers);

    // Add modified headers

}

FilterDataStatus BridgingFilter::decodeData(Buffer::Instance& /*data*/,
                                                     bool /*end_stream*/) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus BridgingFilter::decodeTrailers(RequestTrailerMap& /*trailers*/) {
  return FilterTrailersStatus::Continue;
}

FilterMetadataStatus BridgingFilter::decodeMetadata(MetadataMap& /*metadata*/) {
  return FilterMetadataStatus::Continue;
}

FilterHeadersStatus
BridgingFilter::encode100ContinueHeaders(ResponseHeaderMap& /*headers*/) {
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus BridgingFilter::encodeHeaders(ResponseHeaderMap& /*headers*/,
                                                           bool /*end_stream*/) {
  return FilterHeadersStatus::Continue;
}

FilterDataStatus BridgingFilter::encodeData(Buffer::Instance& /*data*/,
                                                     bool /*end_stream*/) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus BridgingFilter::encodeTrailers(ResponseTrailerMap& /*trailers*/) {
  return FilterTrailersStatus::Continue;
}

FilterMetadataStatus BridgingFilter::encodeMetadata(MetadataMap& /*metadata*/) {
  return FilterMetadataStatus::Continue;
}

} // namespace Http
} // namespace Envoy
