#pragma once

#include "response_trailers_builder.h"
#include "trailers.h"

namespace Envoy {
namespace Platform {

class ResponseTrailersBuilder;

class ResponseTrailers : public Trailers {
public:
  ResponseTrailers(RawHeaders trailers) : Trailers(std::move(trailers)) {}

  ResponseTrailersBuilder to_response_trailers_builder();
};

using ResponseTrailersSharedPtr = std::shared_ptr<ResponseTrailers>;

} // namespace Platform
} // namespace Envoy
