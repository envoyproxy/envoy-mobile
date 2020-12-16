#pragma once

#include "envoy/http/filter.h"

#include "common/common/logger.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/local_error/filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace LocalError {

/**
 * Filter to assert expectations on HTTP requests.
 */
class LocalErrorFilter final : public Http::PassThroughEncoderFilter,
                               public Logger::Loggable<Logger::Id::filter> {
public:
  LocalErrorFilter();

  // StreamEncoderFilter
  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;

private:
  void mapLocalResponseToError(Http::ResponseHeaderMap& headers);

  Http::ResponseHeaderMap* headers_{};
  bool processingError_{};
};

} // namespace LocalError
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
