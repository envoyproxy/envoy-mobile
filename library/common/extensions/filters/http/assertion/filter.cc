#include "library/common/extensions/filters/http/assertion/filter.h"

#include "envoy/http/codes.h"
#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Assertion {

AssertionFilterConfig::AssertionFilterConfig(
    const envoymobile::extensions::filters::http::assertion::Assertion& proto_config) {
  Common::Tap::buildMatcher(proto_config.match_config(), matchers_);
  statuses_ = Extensions::Common::Tap::Matcher::MatchStatusVector(matchers_.size());

  rootMatcher().onNewStream(statuses_);
}

Extensions::Common::Tap::Matcher& AssertionFilterConfig::rootMatcher() const {
  ASSERT(!matchers_.empty());
  return *matchers_[0];
}

AssertionFilter::AssertionFilter(AssertionFilterConfigSharedPtr config) : config_(config) {}

Http::FilterHeadersStatus AssertionFilter::decodeHeaders(Http::RequestHeaderMap& headers,
                                                         bool end_stream) {
  config_->rootMatcher().onHttpRequestHeaders(headers, config_->statuses());
  if (config_->rootMatcher().matchStatus(config_->statuses()).matches_) {
    if (end_stream) {
      decoder_callbacks_->sendLocalReply(Http::Code::OK,
                                         "Request Headers match configured expectations", nullptr,
                                         absl::nullopt, "");
      return Http::FilterHeadersStatus::StopIteration;
    }
    return Http::FilterHeadersStatus::Continue;
  }

  decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                     "Request Headers do not match configured expectations",
                                     nullptr, absl::nullopt, "");
  return Http::FilterHeadersStatus::StopIteration;
}

Http::FilterDataStatus AssertionFilter::decodeData(Buffer::Instance& data, bool end_stream) {
  config_->rootMatcher().onRequestBody(data, config_->statuses());
  if (config_->rootMatcher().matchStatus(config_->statuses()).matches_) {
    if (end_stream) {
      decoder_callbacks_->sendLocalReply(
          Http::Code::OK, "Request Body match configured expectations", nullptr, absl::nullopt, "");
      return Http::FilterDataStatus::StopIterationNoBuffer;
    }
    return Http::FilterDataStatus::Continue;
  }

  decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                     "Request Body does not match configured expectations", nullptr,
                                     absl::nullopt, "");
  return Http::FilterDataStatus::StopIterationNoBuffer;
}

Http::FilterTrailersStatus AssertionFilter::decodeTrailers(Http::RequestTrailerMap& trailers) {
  config_->rootMatcher().onHttpRequestTrailers(trailers, config_->statuses());
  if (config_->rootMatcher().matchStatus(config_->statuses()).matches_) {
    decoder_callbacks_->sendLocalReply(Http::Code::OK,
                                       "Request Trailers match configured expectations", nullptr,
                                       absl::nullopt, "");
    return Http::FilterTrailersStatus::StopIteration;
  }

  decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                     "Request Trailers do not match configured expectations",
                                     nullptr, absl::nullopt, "");
  return Http::FilterTrailersStatus::StopIteration;
}

} // namespace Assertion
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
