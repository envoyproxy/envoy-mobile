#pragma once

#include "envoy/http/filter.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filter/c_types.h"

namespace Envoy {
namespace Http {
/**
 * Harness to bridge Envoy filter invocations up to the platform layer.
 */
class PlatformExtensionFilter final : public PassThroughFilter {
public:
  PlatformExtensionFilter(envoy_http_filter platform_filter) : platform_filter_(platform_filter) {}

  // StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(RequestHeaderMap& headers, bool end_stream) override;
  FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus decodeTrailers(RequestTrailerMap& trailers) override;
  FilterMetadataStatus decodeMetadata(MetadataMap& metadata) override;

  // StreamEncoderFilter
  FilterHeadersStatus encode100ContinueHeaders(ResponseHeaderMap& headers) override;
  FilterHeadersStatus encodeHeaders(ResponseHeaderMap& headers, bool end_stream) override;
  FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus encodeTrailers(ResponseTrailerMap& trailers) override;
  FilterMetadataStatus encodeMetadata(MetadataMap& metadata) override;

private:
  const envoy_http_filter platform_filter_;
};

} // namespace Http
} // namespace Envoy
