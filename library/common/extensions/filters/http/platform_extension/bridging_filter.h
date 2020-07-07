#pragma once

#include "envoy/http/filter.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/platform_extension/c_types.h"
#include "library/common/extensions/filters/http/platform_extension/filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace PlatformExtension {

class BridgingFilterConfig {
public:
  BridgingFilterConfig(
      const envoymobile::extensions::filters::http::platform_extension::Bridging& proto_config);

  const std::string& name() const { return name_; }

private:
  const std::string name_;
};

typedef std::shared_ptr<BridgingFilterConfig> BridgingFilterConfigSharedPtr;

/**
 * Harness to bridge Envoy filter invocations up to the platform layer.
 */
class BridgingFilter final : public Http::PassThroughFilter {
public:
  BridgingFilter(BridgingFilterConfigSharedPtr config);

  // StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap& trailers) override;
  Http::FilterMetadataStatus decodeMetadata(Http::MetadataMap& metadata) override;

  // StreamEncoderFilter
  Http::FilterHeadersStatus encode100ContinueHeaders(Http::ResponseHeaderMap& headers) override;
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus encodeTrailers(Http::ResponseTrailerMap& trailers) override;
  Http::FilterMetadataStatus encodeMetadata(Http::MetadataMap& metadata) override;

private:
  // FIXME: this leaks
  const envoy_http_filter* platform_filter_;
};

} // namespace PlatformExtension
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
