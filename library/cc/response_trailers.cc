#include "response_trailers.h"

// NOLINT(namespace-envoy)

ResponseTrailersBuilder ResponseTrailers::to_response_trailers_builder() {
  ResponseTrailersBuilder builder;
  for (const auto& pair : this->all_headers()) {
    builder.set(pair.first, pair.second);
  }
  return builder;
}
