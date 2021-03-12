#pragma once

#include "envoy/http/filter.h"

#include "common/common/logger.h"

#include "extensions/filters/http/common/pass_through_filter.h"

#include "library/common/extensions/filters/http/route_cache_reset/filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace RouteCacheReset {

/**
 * Filter that has the sole purpose of clearing the route cache for a given
 * stream on the request path. This forces the router filter to recompute
 * routes for outbound requests (taking into account mutations from platform
 * and other filters on the request/headers).
 */
class RouteCacheResetFilter final : public Http::StreamDecoderFilter,
                               public Logger::Loggable<Logger::Id::filter> {
public:
  RouteCacheResetFilter();

  // StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;

private:
  Http::StreamDecoderFilterCallbacks* callbacks_{};
};

} // namespace RouteCacheReset
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
