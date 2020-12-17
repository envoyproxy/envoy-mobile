#include "response_trailers_builder.h"

namespace Envoy {
namespace Platform {

ResponseTrailers ResponseTrailersBuilder::build() const {
  return ResponseTrailers(this->all_headers());
}

} // namespace Platform
} // namespace Envoy
