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
    ASSERT(false, fmt::format("unrecognized filter status from platform: {}", status));
    return Http::FilterHeadersStatus::Continue;
  }
}

BridgingFilterConfig::BridgingFilterConfig(
    const envoymobile::extensions::filters::http::platform_extension::Bridging& proto_config)
    : name_(proto_config.name()) {}

BridgingFilter::BridgingFilter(BridgingFilterConfigSharedPtr config)
    : platform_filter_(
          static_cast<envoy_http_filter*>(Api::External::retrieveApi(config->name()))) {}

Http::FilterHeadersStatus BridgingFilter::onHeaders(Http::HeaderMap& headers, bool end_stream, envoy_filter_on_headers_f on_headers) {
  // Allow nullptr to act as (optimized) no-op.
  if (on_headers == nullptr) {
    return Http::FilterHeadersStatus::Continue;
  }

  envoy_headers in_headers = Http::Utility::toBridgeHeaders(headers);
  envoy_filter_headers_status result = on_headers(in_headers, end_stream, platform_filter_->context);
  Http::FilterHeadersStatus status = mapStatus(result.status);
  // Current platform implementations expose immutable headers, thus any modification necessitates a full copy.
  // If the returned pointer is identical, we assume no modification was made and elide the copy here.
  if (in_headers.headers != result.headers.headers) {
    headers.clear();
    for (envoy_header_size_t i = 0; i < result.headers.length; i++) {
      headers.addCopy(
          Http::LowerCaseString(Http::Utility::convertToString(result.headers.headers[i].key)),
          Http::Utility::convertToString(result.headers.headers[i].value));
    }
  }
  // The C envoy_headers struct can be released now because the headers have been copied.
  release_envoy_headers(result.headers);
  return status;
}

Http::FilterHeadersStatus BridgingFilter::decodeHeaders(Http::RequestHeaderMap& headers,
                                                        bool end_stream) {
  // Delegate to shared implementation for request and response path.
  return onHeaders(headers, end_stream, platform_filter_->on_request_headers);
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

Http::FilterHeadersStatus BridgingFilter::encodeHeaders(Http::ResponseHeaderMap& headers,
                                                        bool end_stream) {
  // Delegate to shared implementation for request and response path.
  return onHeaders(headers, end_stream, platform_filter_->on_response_headers);
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
