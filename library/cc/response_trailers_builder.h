#pragma once

#include "headers_builder.h"
#include "response_trailers.h"

class ResponseTrailers;

class ResponseTrailersBuilder : public HeadersBuilder {
public:
  ResponseTrailersBuilder() {}

  ResponseTrailers build() const;
};
