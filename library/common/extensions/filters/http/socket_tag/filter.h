#pragma once

#include "envoy/http/filter.h"

#include "source/common/common/logger.h"
#include "source/extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/socket_tag/filter.pb.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace SocketTag {

/**
 * Filter to set upstream socket options based on network conditions.
 */
class SocketTagFilter final : public Http::PassThroughFilter,
                              public Logger::Loggable<Logger::Id::filter> {
public:
  // Http::StreamDecoderFilter
  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& request_headers, bool) override;

private:
  Http::StreamDecoderFilterCallbacks* callbacks_{};
};

} // namespace SocketTag
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
