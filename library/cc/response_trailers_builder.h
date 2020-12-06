#pragma once

#include "headers_builder.h"
#include "response_trailers.h"

class ResponseTrailers;

class ResponseTrailersBuilder : HeadersBuilder {
public:
  ResponseTrailersBuilder() {}

  using HeadersBuilder::add;
  using HeadersBuilder::remove;
  using HeadersBuilder::set;

  ResponseTrailers build() const;

private:
  using HeadersBuilder::internal_set;
};
