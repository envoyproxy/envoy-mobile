#pragma once

#include "headers_builder.h"
#include "request_trailers.h"

class RequestTrailers;

class RequestTrailersBuilder : public HeadersBuilder {
public:
  RequestTrailersBuilder() : HeadersBuilder() {}

  using HeadersBuilder::add;
  using HeadersBuilder::remove;
  using HeadersBuilder::set;

  RequestTrailers build() const;

private:
  using HeadersBuilder::internal_set;
};
