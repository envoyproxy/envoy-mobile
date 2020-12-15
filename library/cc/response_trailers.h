#pragma once

// NOLINT(namespace-envoy)

#include "response_trailers_builder.h"
#include "trailers.h"

class ResponseTrailersBuilder;

class ResponseTrailers : public Trailers {
public:
  ResponseTrailers(RawHeaders trailers) : Trailers(std::move(trailers)) {}

  ResponseTrailersBuilder to_response_trailers_builder();
};

using ResponseTrailersSharedPtr = std::shared_ptr<ResponseTrailers>;
