#pragma once

#include "trailers.h"
#include "response_trailers_builder.h"

class ResponseTrailers : public Trailers {
public:
  ResponseTrailersBuilder to_response_trailers_builder();

private:
  ResponseTrailers(RawHeaders trailers) : Trailers(std::move(trailers)) {}

  friend class ResponseTrailersBuilder;
};

using ResponseTrailersSharedPtr = std::shared_ptr<ResponseTrailers>;
