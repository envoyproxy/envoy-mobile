#pragma once

#include "envoy/http/filter.h"

#include "common/common/logger.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/local_error/filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

class LocalErrorFilterConfig {
public:
  LocalErrorFilterConfig(
      const envoymobile::extensions::filters::http::local_error::LocalError& proto_config);

private:
  const bool enabled_;
};

typedef std::shared_ptr<LocalErrorFilterConfig> LocalErrorFilterConfigSharedPtr;

/**
 * Filter to assert expectations on HTTP requests.
 */
class LocalErrorFilter final : public Http::PassThroughEncoderFilter,
                               public Logger::Loggable<Logger::Id::filter> {
public:
  LocalErrorFilter(LocalErrorFilterConfigSharedPtr config);

  // StreamEncoderFilter
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;

private:
  void mapLocalResponseToError(Http::ResponseHeaderMap& headers);

  const LocalErrorFilterConfigSharedPtr config_;
  Http::ResponseHeaderMap* headers_{};
  bool processingError_{};
};

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
