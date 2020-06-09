#include <string>

#include "library/extensions/filters/http/harness/filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Harness {

HarnessFilterConfig::HarnessFilterConfig(
    const envoymobile::extensions::filters::http::harness::Harness& proto_config)
    : name_(proto_config.name()) {}

HarnessFilter::HarnessFilter(HarnessFilterConfigSharedPtr config)
    : config_(config) {}

Http::FilterHeadersStatus HarnessFilter::decodeHeaders(Http::RequestHeaderMap& headers, bool) {
  headers.addCopy(Http::LowerCaseString("x-envoy-mobile-harness"), config_->name());

  return Http::FilterHeadersStatus::Continue;
}

}
}
}
}
