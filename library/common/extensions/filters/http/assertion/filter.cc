#include "library/common/extensions/filters/http/assertion/filter.h"

#include "envoy/http/codes.h"
#include "envoy/server/filter_config.h"

#include "common/http/header_map_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Assertion {

AssertionFilterConfig::AssertionFilterConfig(
    const envoymobile::extensions::filters::http::assertion::Assertion& proto_config) {
  Common::Matcher::buildMatcher(proto_config.match_config(), matchers_);
}

Extensions::Common::Matcher::Matcher& AssertionFilterConfig::rootMatcher() const {
  ASSERT(!matchers_.empty());
  return *matchers_[0];
}

AssertionFilter::AssertionFilter(AssertionFilterConfigSharedPtr config) : config_(config) {
  statuses_ = Extensions::Common::Matcher::Matcher::MatchStatusVector(config_->matchersSize());
  config_->rootMatcher().onNewStream(statuses_);
}

Http::FilterHeadersStatus AssertionFilter::decodeHeaders(Http::RequestHeaderMap& headers,
                                                         bool end_stream) {
  config_->rootMatcher().onHttpRequestHeaders(headers, statuses_);
  auto& matchStatus = config_->rootMatcher().matchStatus(statuses_);
  if (!matchStatus.matches_ && !matchStatus.might_change_status_) {
    decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                       "Request Headers do not match configured expectations",
                                       nullptr, absl::nullopt, "");
    return Http::FilterHeadersStatus::StopIteration;
  }

  if (end_stream) {
    Buffer::OwnedImpl empty_buffer;
    config_->rootMatcher().onRequestBody(empty_buffer, statuses_);
    auto& matchStatus = config_->rootMatcher().matchStatus(statuses_);
    if (!matchStatus.matches_ && !matchStatus.might_change_status_) {
      decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                         "Request Body does not match configured expectations",
                                         nullptr, absl::nullopt, "");
      return Http::FilterHeadersStatus::StopIteration;
    }

    auto empty_trailers = Http::RequestTrailerMapImpl::create();
    config_->rootMatcher().onHttpRequestTrailers(*empty_trailers, statuses_);
    auto& finalMatchStatus = config_->rootMatcher().matchStatus(statuses_);
    if (!finalMatchStatus.matches_ && !finalMatchStatus.might_change_status_) {
      decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                         "Request Trailers do not match configured expectations",
                                         nullptr, absl::nullopt, "");
      return Http::FilterHeadersStatus::StopIteration;
    }
    if (!finalMatchStatus.matches_) {
      decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                         "Request Body does not match configured expectations",
                                         nullptr, absl::nullopt, "");
      return Http::FilterHeadersStatus::StopIteration;
    }
  }

  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus AssertionFilter::decodeData(Buffer::Instance& data, bool end_stream) {
  config_->rootMatcher().onRequestBody(data, statuses_);
  auto& matchStatus = config_->rootMatcher().matchStatus(statuses_);
  if (!matchStatus.matches_ && !matchStatus.might_change_status_) {
    decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                       "Request Body does not match configured expectations",
                                       nullptr, absl::nullopt, "");
    return Http::FilterDataStatus::StopIterationNoBuffer;
  }

  if (end_stream) {
    auto empty_trailers = Http::RequestTrailerMapImpl::create();
    config_->rootMatcher().onHttpRequestTrailers(*empty_trailers, statuses_);
    auto& matchStatus = config_->rootMatcher().matchStatus(statuses_);
    if (!matchStatus.matches_ && !matchStatus.might_change_status_) {
      decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                         "Request Trailers do not match configured expectations",
                                         nullptr, absl::nullopt, "");
      return Http::FilterDataStatus::StopIterationNoBuffer;
    }
    if (!matchStatus.matches_) {
      decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                         "Request Body does not match configured expectations",
                                         nullptr, absl::nullopt, "");
      return Http::FilterDataStatus::StopIterationNoBuffer;
    }
  }
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus AssertionFilter::decodeTrailers(Http::RequestTrailerMap& trailers) {
  config_->rootMatcher().onHttpRequestTrailers(trailers, statuses_);
  auto& matchStatus = config_->rootMatcher().matchStatus(statuses_);
  if (!matchStatus.matches_ && !matchStatus.might_change_status_) {
    decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                       "Request Trailers do not match configured expectations",
                                       nullptr, absl::nullopt, "");
    return Http::FilterTrailersStatus::StopIteration;
  }
  if (!matchStatus.matches_) {
    decoder_callbacks_->sendLocalReply(Http::Code::BadRequest,
                                       "Request Body does not match configured expectations",
                                       nullptr, absl::nullopt, "");
    return Http::FilterTrailersStatus::StopIteration;
  }
  return Http::FilterTrailersStatus::Continue;
}

} // namespace Assertion
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
