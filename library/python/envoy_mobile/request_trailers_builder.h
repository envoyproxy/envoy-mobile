#pragma once

#include "headers_builder.h"
#include "request_trailers.h"


class RequestTrailers;

class RequestTrailersBuilder : public HeadersBuilder {
public:
  RequestTrailersBuilder() : HeadersBuilder() { }

  using HeadersBuilder::add;
  using HeadersBuilder::set;
  using HeadersBuilder::remove;

  RequestTrailers build() const;

private:
  using HeadersBuilder::internal_set;
};
