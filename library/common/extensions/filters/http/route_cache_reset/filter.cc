#include "library/common/extensions/filters/http/route_cache_reset/filter.h"

#include "envoy/http/codes.h"
#include "envoy/server/filter_config.h"

#include "common/grpc/common.h"
#include "common/grpc/status.h"
#include "common/http/codes.h"
#include "common/http/header_map_impl.h"
#include "common/http/utility.h"

#include "library/common/http/headers.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace RouteCacheReset {

RouteCacheResetFilter::RouteCacheResetFilter() {}

void RouteCacheResetFilter::onDestroy() {}

Http::FilterHeadersStatus RouteCacheResetFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  callbacks_->clearRouteCache();
  return Http::FilterHeadersStatus::Continue;
}

void RouteCacheResetFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  callbacks_ = &callbacks;
}

Http::FilterDataStatus RouteCacheResetFilter::decodeData(Buffer::Instance&, bool) {
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus RouteCacheResetFilter::decodeTrailers(Http::RequestTrailerMap&) {
  return Http::FilterTrailersStatus::Continue;
}

} // namespace RouteCacheReset
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
