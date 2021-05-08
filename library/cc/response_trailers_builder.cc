#include "response_trailers_builder.h"

namespace Envoy {
namespace Platform {

ResponseTrailersSharedPtr ResponseTrailersBuilder::build() const {
  ResponseTrailers* trailers = new ResponseTrailers(this->allHeaders());
  return ResponseTrailersSharedPtr(trailers);
}

} // namespace Platform
} // namespace Envoy
