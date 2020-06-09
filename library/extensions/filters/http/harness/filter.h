#pragma once

#include <string>

#include "envoy/server/filter_config.h"
#include "library/extensions/filters/http/harness/filter.pb.h"
#include "extensions/filters/http/common/pass_through_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Harness {

class HarnessFilterConfig {
public:
  HarnessFilterConfig(const envoymobile::extensions::filters::http::harness::Harness& proto_config);

  const std::string& name() const { return name_; }

private:
  const std::string name_;
};

typedef std::shared_ptr<HarnessFilterConfig> HarnessFilterConfigSharedPtr;

class HarnessFilter : public Http::PassThroughFilter {
public:
  HarnessFilter(HarnessFilterConfigSharedPtr config);

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap&, bool) override;

private:
HarnessFilterConfigSharedPtr config_;
};

}
}
} // namespace Http
} // namespace Envoy
