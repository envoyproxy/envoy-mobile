#include "library/common/extensions/filters/http/platform_extension/bridging_filter.h"

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

Http::FilterHeadersStatus mapStatus(envoy_filter_headers_status_t status) {
  switch (status) {
  case ENVOY_FILTER_HEADERS_STATUS_CONTINUE:
    return Http::FilterHeadersStatus::Continue;
  case ENVOY_FILTER_HEADERS_STATUS_STOP_ITERATION:
    return Http::FilterHeadersStatus::StopIteration;
  case ENVOY_FILTER_HEADERS_STATUS_CONTINUE_AND_END_STREAM:
    return Http::FilterHeadersStatus::ContinueAndEndStream;
  case ENVOY_FILTER_HEADERS_STATUS_STOP_ALL_ITERATION_AND_BUFFER:
    return Http::FilterHeadersStatus::StopAllIterationAndBuffer;
  default:
    ASSERT(false, "unrecognized filter status from platform: {}");
    return Http::FilterHeadersStatus::Continue;
  }
}

BridgingFilterConfig::BridgingFilterConfig(
    const envoymobile::extensions::filters::http::platform_extension::Bridging& proto_config)
    : name_(proto_config.name()) {}

BridgingFilter::BridgingFilter(BridgingFilterConfigSharedPtr config)
    : platform_filter_(
          static_cast<envoy_http_filter*>(Api::External::retrieveApi(config->name()))) {}

Http::FilterHeadersStatus BridgingFilter::decodeHeaders(Http::RequestHeaderMap& headers,
                                                        bool end_stream) {
  // This is a hack that results in a full copy of the header map every time.
  // Viable optimizations include:
  // 1) Only tracking modifications to the map at the bridging/platform layers and performing those
  // modifications. 2) Implementing a fully-bridged header map so that modifications actually occur
  // on the underlying Envoy map.
  envoy_headers incoming_headers = Http::Utility::toBridgeHeaders(headers);
  envoy_headers* new_headers = &incoming_headers;
  Http::FilterHeadersStatus status = mapStatus(
      platform_filter_->on_request_headers(new_headers, end_stream, platform_filter_->context));
  if (&incoming_headers != new_headers) {
    headers.removePrefix(Http::LowerCaseString()); // Remove all headers
    for (envoy_header_size_t i = 0; i < new_headers->length; i++) {
      headers.addCopy(
          Http::LowerCaseString(Http::Utility::convertToString(new_headers->headers[i].key)),
          Http::Utility::convertToString(new_headers->headers[i].value));
    }
  }
  // The C envoy_headers struct can be released now because the headers have been copied.
  release_envoy_headers(*new_headers);
}

Http::FilterDataStatus BridgingFilter::decodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus BridgingFilter::decodeTrailers(Http::RequestTrailerMap& /*trailers*/) {
  return Http::FilterTrailersStatus::Continue;
}

Http::FilterMetadataStatus BridgingFilter::decodeMetadata(Http::MetadataMap& /*metadata*/) {
  return Http::FilterMetadataStatus::Continue;
}

Http::FilterHeadersStatus
BridgingFilter::encode100ContinueHeaders(Http::ResponseHeaderMap& /*headers*/) {
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterHeadersStatus BridgingFilter::encodeHeaders(Http::ResponseHeaderMap& /*headers*/,
                                                        bool /*end_stream*/) {
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus BridgingFilter::encodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus BridgingFilter::encodeTrailers(Http::ResponseTrailerMap& /*trailers*/) {
  return Http::FilterTrailersStatus::Continue;
}

Http::FilterMetadataStatus BridgingFilter::encodeMetadata(Http::MetadataMap& /*metadata*/) {
  return Http::FilterMetadataStatus::Continue;
}

} // namespace PlatformExtension
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
