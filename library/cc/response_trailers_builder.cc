#include "response_trailers_builder.h"

// NOLINT(namespace-envoy)

ResponseTrailers ResponseTrailersBuilder::build() const {
  return ResponseTrailers(this->all_headers());
}
