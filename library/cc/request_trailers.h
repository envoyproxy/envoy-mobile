#pragma once

#include "request_trailers_builder.h"
#include "trailers.h"

class RequestTrailers : public Trailers {
public:
  RequestTrailers(const RawHeaders& headers) : Trailers(headers) {}

  RequestTrailersBuilder to_request_trailers_builder() const;
};
