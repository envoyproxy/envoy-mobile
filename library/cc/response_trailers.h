#pragma once

#include "trailers.h"
#include "response_trailers_builder.h"

class ResponseTrailers : public Trailers {
public:
  ResponseTrailers(RawHeaders trailers) : Trailers(trailers) {}

  ResponseTrailersBuilder to_response_trailers_builder();
};

using ResponseTrailersSharedPtr = std::shared_ptr<ResponseTrailers>;
