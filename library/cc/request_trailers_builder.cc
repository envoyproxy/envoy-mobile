#include "request_trailers_builder.h"

// NOLINT(namespace-envoy)

RequestTrailers RequestTrailersBuilder::build() const {
  return RequestTrailers(this->all_headers());
}
